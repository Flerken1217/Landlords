#include "gamepanel.h"
#include "playhand.h"
#include "ui_gamepanel.h"
#include "endingpanel.h"
#include"gamecontrol.h"
#include <QPainter>
#include <QDebug>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QSet>
#include<QStackedLayout>
#include<QVector>


GamePanel::GamePanel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GamePanel)
{
    ui->setupUi(this);

    setMouseTracking(false);

    setWindowTitle("雀食：二次元下饭斗地主");
    setFixedSize(1000, 650);

    // 实例化游戏控制对象
    m_gameCtl = new GameControl(this);
    // 初始化游戏玩家
    m_gameCtl->playerInit();
    // 取出各个玩家的实例对象
    Robot* leftRobot = m_gameCtl->getLeftRobot();
    Robot* rightRobot = m_gameCtl->getRightRobot();
    UserPlayer* user = m_gameCtl->getUserPlayer();
    // 存储各个玩家的实例对象, 顺序: 左机器人, 有机器人, 当前玩家
    m_playerList << leftRobot << rightRobot << user;

    // 初始化玩家得分
    updatePlayerScore();
    // 切割并存储每张扑克牌
    initCardsMap();
    // 初始化控制按钮区域
    initButtonGroup();
    // 初始化玩家上下文环境：
    initPlayerContext();

    // 发牌时的底牌和移动牌初始化
    m_baseCard = new CardPanel(this);
    m_moveCard = new CardPanel(this);
    m_baseCard->setImage(m_cardBackImg, m_cardBackImg);
    m_moveCard->setImage(m_cardBackImg, m_cardBackImg);
    m_baseCard->hide();
    m_moveCard->hide();

    // 最后剩余3张底牌, 存到到一个容器中
    for(int i=0; i<3; ++i)
    {
        CardPanel* card = new CardPanel(this);
        m_last3Cards.push_back(card);
    }

    // 发牌定时器
    m_pickCardTimer = new QTimer(this);
    connect(m_pickCardTimer, &QTimer::timeout, this, &GamePanel::onDispatchCard);


    // 绘制窗口
    drawGameScene();

    // 处理游戏控制窗口发射的信号
    connect(m_gameCtl, &GameControl::notifyPickCards, this, &GamePanel::onDisposCard);
    connect(m_gameCtl, &GameControl::playerStatusChanged, this, &GamePanel::onPlayerStatusChanged);
    connect(m_gameCtl, &GameControl::gameStatusChanged, this, &GamePanel::gameStatusProcess);
    connect(m_gameCtl, &GameControl::notifyPlayHand, this, &GamePanel::onDisposePlayHand);
    connect(m_gameCtl, &GameControl::notifyGrabLordBet, this, &GamePanel::onGrabLordBet);
    //connect(m_gameCtl, &GameControl::hideLordCard, this, &GamePanel::onHideLordCard);

    // 背景图片，随机从五张里面挑一张
    int index = QRandomGenerator::global()->bounded(5);
    QString fileName = QString(":/images/background-%1.png").arg(index+1);
    m_bkImg.load(fileName);

    // 初始化游戏音乐控制器
    m_bgm = new BGMControl(this);

    //初始化动画对象
    initAnimation();

    // 出牌倒计时
    initCountDown();
}

GamePanel::~GamePanel()
{
    delete ui;
}


void GamePanel::updatePlayerScore()
{
    ui->scorePanel->setScores(m_gameCtl->getLeftRobot()->getScore(),
                              m_gameCtl->getUserPlayer()->getScore(),
                              m_gameCtl->getRightRobot()->getScore());
}

void GamePanel::onHideLordCard(){
    for(int i=0; i<m_last3Cards.size(); ++i)
    {
        m_last3Cards[i]->hide();
    }
}

void GamePanel::initPlayerContext()
{
    //牌的位置
    QRect cardsRect[] =
    {
        // x, y, width, height
        QRect(90, 130, 100, height() - 300),                    // 左侧机器人
        QRect(rect().right() - 190, 130, 100, height() - 300),  // 右侧机器人
        QRect(250, rect().bottom() - 120, width() - 500, 100)   // 当前玩家
    };
    //玩家出牌的区域
    QRect playHandRect[] =
    {
        QRect(260, 150, 100, 100),                              // 左侧机器人
        QRect(rect().right() - 360, 150, 100, 100),             // 右侧机器人
        QRect(150, rect().bottom() - 290, width() - 300, 100)   // 当前玩家
    };
    //角色图片位置
    QPoint roleImgPos[] =
    {
        QPoint(cardsRect[0].left()-80, cardsRect[0].height() / 2 + 80),     // 左侧机器人
        QPoint(cardsRect[1].right()+3, cardsRect[1].height() / 2 + 80),    // 右侧机器人
        QPoint(cardsRect[2].right()-5, cardsRect[2].top() - 10)            // 当前玩家
    };

    // 遍历三个玩家, 初始化相关信息
    int userIndex = m_playerList.indexOf(m_gameCtl->getUserPlayer());
    for(int i=0; i<m_playerList.size(); ++i)
    {
        PlayerContext context;
        // 扑克牌排列方式和区域初始化
        context.align = i==userIndex ? Horizontal : Vertical;
        context.isFrontSide = i==userIndex ? true : false;
        context.cardsRect = cardsRect[i];
        context.playHandRect = playHandRect[i];
        // 显示游戏过程中的提示信息的对象初始化
        context.info = new QLabel(this);
        context.info->resize(160, 98);
        context.info->hide();
        QRect rect = playHandRect[i];
        QPoint pt(rect.left() + (rect.width() - context.info->width()) / 2,
            rect.top() + (rect.height() - context.info->height()) / 2);
        context.info->move(pt);
        // 显示玩家图像的对象的初始化
        context.roleImg = new QLabel(this);
        context.roleImg->resize(84, 120);
        context.roleImg->hide();
        context.roleImg->move(roleImgPos[i]);
        m_contextMap.insert(m_playerList.at(i), context);
    }
}

void GamePanel::drawGameScene()
{
    m_baseCardPos = QPoint((width() - m_cardSize.width()) / 2, height() / 2 - 100);
    // 底牌和移动牌位置初始化
    m_baseCard->move(m_baseCardPos);
    m_moveCard->move(m_baseCardPos);

    // 显示剩下的三张牌
    int base = (width() - 3 * m_cardSize.width() - 2 * 10) / 2;
    for(int i=0; i<m_last3Cards.size(); ++i)
    {

        m_last3Cards[i]->move(base + (m_cardSize.width() + 10) * i, 250);
    }
}

//更新用户区的扑克牌
void GamePanel::updatePlayerCards(Player *player)
{
    //玩家剩余的卡牌组
    Cards restCards = player->getCards();
    //玩家剩余的有序卡牌组（由无序转换过来）
    CardList restCardList = restCards.toCardList(Cards::Desc);

    m_userCards.clear();//清空卡牌
    m_cardsRect = QRect();// 非机器人玩家剩余的扑克牌显示区域
    // 显示剩下的牌
    const int cardSpacing = 20;		// 用户区域的牌间隔
    QRect cardsRect = m_contextMap[player].cardsRect;//获取当前玩家的卡牌所在区域
    auto itRest = restCardList.begin();//设置迭代器
    for (int i = 0; itRest != restCardList.end(); itRest++, i++)
    {
        //逐个获取玩家手中的牌窗口
        CardPanel* panel = m_cardMap[*itRest];
        //这些窗口全部设为展示正面状态
        panel->setFrontSide(m_contextMap[player].isFrontSide);
        //以叠放在高层的形式展示
        panel->show();
        panel->raise();

        if (m_contextMap[player].align == Horizontal)//如果卡牌是水平放置（用户玩家）
        {
            //设置每张牌的位置
            int leftBase = cardsRect.left() + (cardsRect.width() - (restCardList.size() - 1) * cardSpacing - panel->width()) / 2;
            int top = cardsRect.top() + (cardsRect.height() - panel->height()) / 2;
            if (panel->isSelected()) top -= 10;
            panel->move(leftBase + i * cardSpacing, top);
            // 保存每张剩余的牌在窗口中的位置
            QRect cardRect(leftBase + i * cardSpacing, top, cardSpacing, m_cardSize.height());
            m_userCards.insert(panel, cardRect);
            m_cardsRect = QRect(leftBase, top, cardSpacing * i + m_cardSize.width(), m_cardSize.height());
        }
        else//非user玩家
        {
            int left = cardsRect.left() + (cardsRect.width() - panel->width()) / 2;
            if (panel->isSelected()) left += 10;
            int topBase = cardsRect.top() + (cardsRect.height() - (restCardList.size() - 1) * cardSpacing - panel->height()) / 2;
            panel->move(left, topBase + i * cardSpacing);
        }
    }

    // 把卡牌打到发牌区域
    QRect playCardsRect = m_contextMap[player].playHandRect;//获取该玩家的出牌区域
    if (!m_contextMap[player].lastCards.isEmpty())		// 不是空牌
    {
        int playSpacing = 24;//打出的牌之间的间隔是24
        //转换为有序的从大到小的牌组
        CardList lastCardList = m_contextMap[player].lastCards.toCardList();
        //遍历所有牌
        CardList::ConstIterator itPlayed = lastCardList.constBegin();
        for (int i = 0; itPlayed != lastCardList.constEnd(); itPlayed++, i++)
        {
            //获取到每张牌的panel，设置为正面朝上，以堆叠方式实现
            CardPanel* panel = m_cardMap[*itPlayed];
            panel->setFrontSide(true);
            panel->raise();

            //计算不同玩家的要把牌打到什么位置的区域，并且move到那里去
            if (m_contextMap[player].align == Horizontal)
            {
                int leftBase = playCardsRect.left () +
                    (playCardsRect.width() - (lastCardList.size() - 1) * playSpacing - panel->width()) / 2;
                int top = playCardsRect.top() + (playCardsRect.height() - panel->height()) / 2;
                panel->move(leftBase + i * playSpacing, top);
            }
            else
            {
                int left = playCardsRect.left() + (playCardsRect.width() - panel->width()) / 2;
                int topBase = playCardsRect.top();
                panel->move(left, topBase + i * playSpacing);
            }
        }
    }
}

void GamePanel::initButtonGroup()
{
    ui->btnGroup->initButtons();
    ui->btnGroup->selectPanel(ButtonGroup::Start);
    // 开始游戏按钮
    connect(ui->btnGroup, &ButtonGroup::startGame, this, [=](){
        //隐藏所有按钮，将按钮组设置为空
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        //清空所有玩家得分
        m_gameCtl->clearPlayerScore();
        //更新玩家得分
        updatePlayerScore();
        //切换游戏状态到抽取天癞子状态
        emit gameStatusProcess(GameControl::ExtractTianLaiZi);
        m_bgm->startBGM();
    });
    // 抢地主
    connect(ui->btnGroup, &ButtonGroup::betPoint, this, [=](int point){
        m_gameCtl->getUserPlayer()->grabLordBet(point);
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
    });
    // 不要 pass
    connect(ui->btnGroup, &ButtonGroup::pass, this, &GamePanel::onUserPass);
    // 出牌 playhand
    connect(ui->btnGroup, &ButtonGroup::playHand, this, &GamePanel::onUserPlayHand);
}

//抽取天癞子
void GamePanel::extractTian(){
    //抽取天癞子
    m_gameCtl->initTianLaiZi();
    qDebug()<<"天癞子为"<<(GameControl::TianLaiZi+2);

    //获取整幅卡牌
    Cards allCards;
    // 遍历 QMap 并获取每个 Card 对象
    for (auto it = m_cardMap.begin(); it != m_cardMap.end(); ++it) {
        Card card = it.key(); // 获取键，即 Card 对象
        allCards.add(card);
    }

    //设置迭代器，用于循环遍历获取到的牌
    QSet<Card>::const_iterator it = allCards.getCards().constBegin();
    //遍历整个set，把相同点数且不同花色的都加入天癞子卡牌窗口组中
    for(int i=0; i<allCards.getCards().size(); ++i, it++){
         if(it->m_point==GameControl::TianLaiZi){
             //CardPanel* panel=m_cardMap[*it];
             CardPanel* originalPanel = m_cardMap[*it];
             CardPanel* panel = new CardPanel(*originalPanel); // 使用复制构造函数
             panel->setParent(originalPanel->parentWidget()); // 设置相同的父对象
             panel->setFrontSide(true);
              panel->raise();
              //把和天癞子点数相同的卡牌放到卡牌组里
              m_tianLaiZiSet.push_back(panel);
          }
    }
        int playSpace=-50;//卡牌之间的间隔

        //把天癞子移动到左上方区域
        for(int i = 0; i < m_tianLaiZiSet.size(); ++i) {
            int left = 300 + i * (m_tianLaiZiSet[0]->getImage().width() + playSpace);
                m_tianLaiZiSet[i]->move(left, 20);
                m_tianLaiZiSet[i]->show();
        }

//        QTimer* timer = new QTimer(this);
//        timer->setInterval(1000);
//        // 使用singleShot模式，计时器会在一次信号后停止
//        timer->setSingleShot(true);
//        // 连接 timer 的 timeout 信号到一个新的槽函数
//        timer->start();
//        connect(timer, &QTimer::timeout, this, &GamePanel::emitCardSend);

    //QTimer::singleShot(500,this,SLOT(emitCardSend()));

    //转移到发牌状态
    gameStatusProcess(GameControl::DispatchCard);
}

//抽取地癞子
void GamePanel::extractDi(){

    //抽取地癞子
    m_gameCtl->initDiLaiZi();
    qDebug()<<"地癞子为"<<(GameControl::DiLaiZi+2);
    //获取整幅卡牌
    Cards allCards;
    // 遍历 QMap 并获取每个 Card 对象
    for (auto it = m_cardMap.begin(); it != m_cardMap.end(); ++it) {
        Card card = it.key(); // 获取键，即 Card 对象
        allCards.add(card);
    }

    //设置迭代器，用于循环遍历获取到的牌
    QSet<Card>::const_iterator it = allCards.getCards().constBegin();
    //遍历整个set，把相同点数且不同花色的都加入天癞子卡牌窗口组中
    for(int i=0; i<allCards.getCards().size(); ++i, it++){
         if(it->m_point==GameControl::DiLaiZi){
              //CardPanel* panel=m_cardMap[*it];
             CardPanel* originalPanel = m_cardMap[*it];
             CardPanel* panel = new CardPanel(*originalPanel); // 使用复制构造函数
             panel->setParent(originalPanel->parentWidget()); // 设置相同的父对象
             panel->setFrontSide(true);
              panel->raise();
              //把和天癞子点数相同的卡牌放到卡牌组里
              m_diLaiZiSet.push_back(panel);
          }
    }

    int playSpace=-50;//卡牌之间的间隔

    //把地癞子移动到中心区域
        for(int i = 0; i < m_diLaiZiSet.size(); ++i) {
        int left = 500 + i * (m_diLaiZiSet[0]->getImage().width() + playSpace);
            m_diLaiZiSet[i]->move(left, 20);
            m_diLaiZiSet[i]->show();
        }

   //转移到发牌状态
   gameStatusProcess(GameControl::PlayingHand);
   //改变玩家状态到准备出牌
   onPlayerStatusChanged(m_gameCtl->getCurrentPlayer(), GameControl::ThinkingForPlayHand);
}

void GamePanel::gameStatusProcess(GameControl::GameStatus status)
{
    m_gameStatus = status;
    switch(status)
    {
    case GameControl::ExtractTianLaiZi:
       //抽取天癞子
        extractTian();
        break;
    case GameControl::DispatchCard:
        // 开始发牌
        startPickCard();
        break;
    case GameControl::CallingLord:
    {
        CardList last3Cards = m_gameCtl->getSurplusCards().toCardList();

        for (int i = 0; i < m_last3Cards.size(); i++)
        {
            QPixmap front = m_cardMap[last3Cards[i]]->getImage();
            m_last3Cards[i]->setImage(front, m_cardBackImg);
            m_last3Cards[i]->hide();
        }

        // 开始叫地主
        m_gameCtl->startLordCard();
        break;
    }
    case GameControl::ExtractDiLaiZi:
        //抽取地癞子
        extractDi();
        break;
    case GameControl::PlayingHand:
    {
        // 底牌和移动牌隐藏
        m_baseCard->hide();
        m_moveCard->hide();
        // 显示最后剩余的3张底牌
        for(int i=0; i<m_last3Cards.size(); ++i)
        {
            m_last3Cards.at(i)->show();
        }

        QTimer* timer = new QTimer(this);
        timer->setInterval(1000);

         // 使用singleShot模式，计时器会在一次信号后停止
         timer->setSingleShot(true);
         // 连接 timer 的 timeout 信号到一个新的槽函数
         timer->start();
         connect(timer, &QTimer::timeout, this, &GamePanel::onHideLordCard);

        // 遍历玩家
        for(int i=0; i<m_playerList.size(); ++i)
        {
            PlayerContext &contex = m_contextMap[m_playerList.at(i)];
            // 隐藏信息提示窗口
            contex.info->hide();
            Player* player = m_playerList.at(i);
            QPixmap pixmap = loadCharacterImage(player->getCharacter(), player->getDirection());
            contex.roleImg->setPixmap(pixmap);
            contex.roleImg->show();
        }

        break;
    }
    default:
        break;
    }
}

void GamePanel::startPickCard()
{

    // 初始化每张卡牌的属性
    for(auto it = m_cardMap.begin(); it!=m_cardMap.end(); ++it)
    {
        it.value()->setSelected(false); // 设置窗口非选中状态
        it.value()->setFrontSide(true); // 设置窗口图片显示正面
        it.value()->hide();             // 设置窗口隐藏
    }
    // 隐藏最后三张底牌
    for(int i=0; i<m_last3Cards.size(); ++i)
    {
        m_last3Cards.at(i)->hide();
    }

    // 发牌阶段, 不需要出牌, 清空容器(非机器人玩家使用)
    m_selectCards.clear();
    // 重置所有玩家的窗口环境信息
    int index = m_playerList.indexOf(m_gameCtl->getUserPlayer());
    for(int i=0; i<m_playerList.size(); ++i)
    {
        m_contextMap[m_playerList.at(i)].lastCards.clear();//清空上次打出的牌
        m_contextMap[m_playerList.at(i)].info->hide();//隐藏信息
        m_contextMap[m_playerList.at(i)].roleImg->hide();//隐藏头像
        //如果是user，则显示正面，如果是机器人则显示背面，index是用户玩家
        m_contextMap[m_playerList.at(i)].isFrontSide = i==index ? true : false;
    }
    // 重置所有玩家的卡牌数据
    m_gameCtl->resetCardsData();
    // 显示底牌
    //m_baseCard->show();
    // 切换游戏按钮面板为empty，隐藏所有的按钮
    ui->btnGroup->selectPanel(ButtonGroup::Empty);
    // 启动定时器
    m_pickCardTimer->start(10); // 每隔10ms发一张牌
    // 播放发牌音乐
    m_bgm->playAssistMusic(BGMControl::Dispatch, true);
}

//发牌动画
void GamePanel::showDispatchAnimation(Player *player, int step)
{
    QRect cardsRect = m_contextMap[player].cardsRect;
    // 单元步长
    int unit[] =
    {
        (m_baseCardPos.x() - cardsRect.right()) / 100,
        (cardsRect.left() - m_baseCardPos.x()) / 100,
        (cardsRect.top() - m_baseCardPos.y()) / 100
    };
    // 窗口位置
    QPoint pos[] =
    {
        QPoint(m_baseCardPos.x() - step * unit[0], m_baseCardPos.y()),
        QPoint(m_baseCardPos.x() + step * unit[1], m_baseCardPos.y()),
        QPoint(m_baseCardPos.x(), m_baseCardPos.y() + step * unit[2])
    };
    // 设置映射关系
    QHash<Player*, int> indexMap;
    indexMap.insert(m_gameCtl->getLeftRobot(), 0);
    indexMap.insert(m_gameCtl->getRightRobot(), 1);
    indexMap.insert(m_gameCtl->getUserPlayer(), 2);
    // 设置移动牌的位置
    m_moveCard->move(pos[indexMap[player]]);

    if (step == 0)
    {
        m_moveCard->show();
    }

    if (step >= 100)
    {
        m_moveCard->hide();
    }

    update();
}

void GamePanel::hidePlayerDropCard(Player *player)
{
    auto it = m_contextMap.find(player);
    if (it != m_contextMap.end())
    {
        if (it->lastCards.isEmpty())	// 上一次打的空牌，即pass
        {
            it->info->hide();
        }
        else
        {
            CardList list = it->lastCards.toCardList();
            for (auto last = list.begin(); last != list.end(); ++last)
            {
                m_cardMap[*last]->hide();
            }
        }
    }
}

void GamePanel::initAnimation()
{
    m_animation = new AnimationWindow(this);
    connect(m_animation, &AnimationWindow::animationOver, m_animation, &AnimationWindow::hide);
}

void GamePanel::showAnimation(AnimationType type, Player& player,int bet)
{

    switch(type)
    {
    case AnimationType::LianDui:
    case AnimationType::ShunZi:
        m_animation->setFixedSize(250, 150);
        m_animation->move((width()-m_animation->width())/2, 200);
        m_animation->showSequence((AnimationWindow::Type)type);
        break;
    case AnimationType::Plane:
        m_animation->setFixedSize(800, 75);
        m_animation->move((width()-m_animation->width())/2, 200);
        m_animation->showPlane();

        break;
    case AnimationType::Bomb:
        m_animation->setFixedSize(180, 200);
        m_animation->move((width()-m_animation->width())/2, (height()-m_animation->height())/2 - 70);
        m_animation->showBomb();

        break;
    case AnimationType::JokerBomb:
        m_animation->setFixedSize(250, 200);
        m_animation->move((width()-m_animation->width())/2, (height()-m_animation->height())/2 - 70);
        m_animation->showJokerBomb();

        break;
    case AnimationType::Bet:
        m_animation->setFixedSize(160, 98);
        m_animation->move((width()-m_animation->width())/2, (height()-m_animation->height())/2 - 140);
        m_animation->showBetScore(bet);

        break;
    case AnimationType::Spring:
        m_animation->setFixedSize(487, 222);
        m_animation->move((width()-m_animation->width())/2, (height()-m_animation->height())/2 - 50);
        m_animation->showSpring();

        break;
    case AnimationType::AntiSpring:
        m_animation->setFixedSize(268,217);
        m_animation->move((width()-m_animation->width())/2, (height()-m_animation->height())/2 - 50);
        m_animation->showAntiSpring();

        break;
    }
    m_animation->show();
}

QPixmap GamePanel::loadCharacterImage(Character character,Player::Direction direction)
{
    QPixmap pixmap=character.getPortrait();

    QImage* image = new QImage(pixmap.size(), QImage::Format_ARGB32);
    // 初始化图像数据
    image->fill(Qt::transparent);

    // 将QPixmap绘制到QImage
    QPainter painter(image);
    painter.drawPixmap(QPoint(0, 0), pixmap);
    painter.end();

    if(direction== Player::Left)
    {
        pixmap = QPixmap::fromImage(*image);
    }
    else
    {
        //处理镜像图片
        pixmap = QPixmap::fromImage(image->mirrored(true, false));
    }
    return pixmap;
}

void GamePanel::beforEndAnimation()
{
    // 创建并启动一个QTimer，设置时间间隔为1秒
    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);

    // 使用singleShot模式，计时器会在一次信号后停止
    timer->setSingleShot(true);
    // 连接 timer 的 timeout 信号到一个新的槽函数
    connect(timer, &QTimer::timeout, this, &GamePanel::showScorePanelAnimation);

    if(m_gameCtl->isSpring())
    {
        timer->start();
        showAnimation(AnimationType::Spring,*(m_gameCtl->getCurrentPlayer()));
    }
    else if(m_gameCtl->isAntiSpirng()){
        showAnimation(AnimationType::AntiSpring,*(m_gameCtl->getCurrentPlayer()));
        timer->start();
    }
    timer->start();
}
//结算面板
void GamePanel::showScorePanelAnimation()
{
    bool isLord = m_gameCtl->getUserPlayer()->getRole() == Player::Lord ? true : false;
    bool isWin = m_gameCtl->getUserPlayer()->isWin();
    EndingPanel* panel = new EndingPanel(isLord, isWin, this);
    panel->move((width()-panel->width())/2, -panel->height());
    panel->show();
    panel->setPlayScore(m_gameCtl->getLeftRobot()->getScore(),
                        m_gameCtl->getRightRobot()->getScore(),
                        m_gameCtl->getUserPlayer()->getScore());

    if(isWin)
    {
        // 播放胜利音乐
        m_bgm->playEndingMusic(true);
    }
    else
    {
        // 播放失败音乐
        m_bgm->playEndingMusic(false);
    }


    QPropertyAnimation* animation = new QPropertyAnimation(panel, "geometry", this);
    // 动画时间
    animation->setDuration(1000);   // 1000ms == 1s
    // 起始位置
    animation->setStartValue(QRect(panel->x(), panel->y(), panel->width(), panel->height()));
    // 结束位置
    animation->setEndValue(QRect((width()-panel->width())/2, (height()-panel->height())/2,
                                 panel->width(), panel->height()));
    // 设置运动曲线
    animation->setEasingCurve(QEasingCurve::OutBounce);
    // 开始播放
    animation->start();

    connect(panel, &EndingPanel::continueGame, this, [=]()
    {
        panel->close();
        panel->deleteLater();
        animation->deleteLater();
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        gameStatusProcess(GameControl::ExtractTianLaiZi);
        m_bgm->startBGM();
    });
}

void GamePanel::initCountDown()
{
    m_countDown = new CountDown(this);
    m_countDown->move((width() - m_countDown->width()) / 2, (height() - m_countDown->height()) / 2 + 30);
    connect(m_countDown, &CountDown::notMuchTime, this, [=]()
    {
        m_bgm->playAssistMusic(BGMControl::Alert);
    });
    connect(m_countDown, &CountDown::timeout, this, [=]()
    {
        onUserPass();
    });
    UserPlayer* player = m_gameCtl->getUserPlayer();
    connect(player, &UserPlayer::startCountDown, this, [=]()
    {
        if(m_gameCtl->getPendPlayer() != player && m_gameCtl->getPendPlayer() != nullptr)
        {
            m_countDown->showCountDown();
        }
    });
}

void GamePanel::onDispatchCard()
{
    static int curMoveStep = 0;
    Player* curPlayer = m_gameCtl->getCurrentPlayer();
    if(curMoveStep >= 100)
    {
        // 发一张牌
        Card card = m_gameCtl->takeOneCard();
        curPlayer->storeDispatchCard(card);
        // 显示发牌动画
        showDispatchAnimation(curPlayer, curMoveStep);
        // 切换下一个用户
        m_gameCtl->setCurrentPlayer(curPlayer->getNextPlayer());
        curMoveStep = 0;
        if(m_gameCtl->getSurplusCards().cardCount() == 3)
        {
            // 终止定时器
            m_pickCardTimer->stop();
            m_bgm->stopAssistMusic();
            gameStatusProcess(GameControl::CallingLord);
            return;
        }
    }
    // 显示发牌动画
    showDispatchAnimation(curPlayer, curMoveStep);
    // 步长增长
    curMoveStep += 14;
}

void GamePanel::onUserPass()
{
    m_countDown->stopCountDown();

    Player* user = m_gameCtl->getUserPlayer();
    if(m_gameCtl->getCurrentPlayer() != user)
    {
        // 如果当前用户不是非机器人玩家, 不做任何处理
        return;
    }
    Player* pendPlayer = m_gameCtl->getPendPlayer();
    if(pendPlayer == user || pendPlayer == nullptr)
    {
        // 如果上一次就是非机器人玩家出的牌, 不允许pass直接跳出不做任何处理
        return;
    }
    // 如果pass, 打出一个空对象
    Cards empty;
    m_gameCtl->getUserPlayer()->playHand(empty);

    for(auto it = m_selectCards.begin(); it != m_selectCards.end(); ++it)
    {
        (*it)->setSelected(false);
    }
    m_selectCards.clear();
    updatePlayerCards(m_gameCtl->getUserPlayer());
}

// 非机器人玩家出牌
void GamePanel::onUserPlayHand()
{
    if(m_gameStatus != GameControl::PlayingHand)
    {
        // 游戏状态不是出牌阶段, 直接跳过
        return;
    }
    if(m_gameCtl->getCurrentPlayer() != m_gameCtl->getUserPlayer())
    {
        // 当前玩家是机器人玩家, 直接跳过
        return;
    }
    if(m_selectCards.isEmpty())
    {
        // 如果没有出牌, 直接跳过
        return;
    }

    Cards cs;
    auto it = m_selectCards.begin();
    for(; it != m_selectCards.end(); ++it)
    {
        Card card = (*it)->getCard();
        cs.add(card);
    }
    PlayHand hand(cs);
    PlayHand::HandType type = hand.getType();

    if(type == PlayHand::Hand_Unknown)
    {
        // 位置牌型, 不允许出牌
        return;
    }
    // 管不住其他机器人的牌
    if(m_gameCtl->getPendPlayer() != m_gameCtl->getUserPlayer())
    {
        Cards cards = m_gameCtl->getPendCards();
        if(!(hand>cards))
        {
            return;
        }
    }

    m_countDown->stopCountDown();
    // 出牌
    m_gameCtl->getUserPlayer()->playHand(cs);
    // 牌打出之后, 清空容器
    m_selectCards.clear();
}

void GamePanel::onDisposCard(Player *player, Cards &card)
{
    CardList list = card.toCardList();
    for(auto it = list.begin(); it != list.end(); ++it)
    {
        CardPanel* panel = m_cardMap[*it];
        panel->setOwner(player);
    }
    // 更新
    updatePlayerCards(player);
}

void GamePanel::onPlayerStatusChanged(Player *player, GameControl::PlayerStatus s)
{
    UserPlayer* user = m_gameCtl->getUserPlayer();
    switch(s)
    {
    case GameControl::ThinkingForCallLord:
        if(player == user)
        {
            ui->btnGroup->selectPanel(ButtonGroup::CallLord, m_gameCtl->getPlayerMaxBet());
        }
        break;
    case GameControl::ThinkingForPlayHand:
        // 隐藏文件打出的最后一张牌
        hidePlayerDropCard(player);
        if(player == user)
        {
            // 判断刚才出牌的是不是自己
            Player* pendPlayer = m_gameCtl->getPendPlayer();
            if(pendPlayer == user || pendPlayer == nullptr)
            {
                ui->btnGroup->selectPanel(ButtonGroup::PlayCard);
            }
            else
            {
                ui->btnGroup->selectPanel(ButtonGroup::PassOrPlay);
            }
        }
        else
        {
            ui->btnGroup->selectPanel(ButtonGroup::Empty);
        }
        break;
    case GameControl::Winning:
        m_bgm->stopBGM();
        m_contextMap[m_gameCtl->getLeftRobot()].isFrontSide = true;
        m_contextMap[m_gameCtl->getRightRobot()].isFrontSide = true;
        updatePlayerCards(m_gameCtl->getLeftRobot());
        updatePlayerCards(m_gameCtl->getRightRobot());
        // 更新分数面板
        updatePlayerScore();
//        ui->btnGroup->selectPanel(ButtonGroup::Continue);
        // 获胜的玩家下一局先叫分
        m_gameCtl->setCurrentPlayer(player);
        beforEndAnimation();
        //showScorePanelAnimation();
        break;
    default:
        break;
    }
}

void GamePanel::onCardSelected(Qt::MouseButton btn)
{
    if(m_gameStatus == GameControl::DispatchCard)
    {
        // 如果是发牌状态不处理, 直接跳过
        return;
    }
    // 得到信号发出者对象实例
    CardPanel* panel = (CardPanel*)sender();
    if(panel->getOwner() != m_gameCtl->getUserPlayer())
    {
        // 如果点击的扑克牌不是当前玩家的(是机器人的)直接跳过, 不做任何处理
        return;
    }
    m_curSelCard = panel;
    // 如果是鼠标左键
    if(btn == Qt::LeftButton)
    {
        // 设置点击的扑克牌的选择状态
        panel->setSelected(!panel->isSelected());
        // 更新玩家的牌
        updatePlayerCards(panel->getOwner());
        QSet<CardPanel*>::const_iterator it = m_selectCards.find(panel);
        if(it == m_selectCards.constEnd())
        {
            m_selectCards.insert(panel);
        }
        else
        {
            m_selectCards.erase(it);
        }
        // 播放选牌音乐
        m_bgm->playAssistMusic(BGMControl::SelectCard);
    }
    else if(btn == Qt::RightButton)
    {
        // 右击出牌
        onUserPlayHand();
    }
}

void GamePanel::onDisposePlayHand(Player *player, Cards &cards)
{
    // 隐藏玩家扔出的牌
    hidePlayerDropCard(player);
    // 记录最近一次打出的牌
    auto it = m_contextMap.find(player);
    it->lastCards = cards;

    // 判断牌型
    PlayHand hand(cards);
    PlayHand::HandType type = hand.getType();
    //qDebug() << "------card type--------: " << type;

    if(type == PlayHand::Hand_Plane ||                      // 飞机
       type == PlayHand::Hand_Plane_Two_Single ||           // 飞机带单
       type == PlayHand::Hand_Plane_Two_Pair)               // 飞机带双
    {
        showAnimation(Plane,*(m_gameCtl->getCurrentPlayer()));
    }
    else if(type == PlayHand::Hand_Seq_Pair)                // 连对
    {
        showAnimation(LianDui,*(m_gameCtl->getCurrentPlayer()));
    }
    else if(type == PlayHand::Hand_Seq_Single)              // 顺子
    {
        showAnimation(ShunZi,*(m_gameCtl->getCurrentPlayer()));
    }
    if(type == PlayHand::Hand_Bomb)                         // 炸弹
    {
        showAnimation(Bomb,*(m_gameCtl->getCurrentPlayer()));
    }
    else if(type == PlayHand::Hand_Bomb_Jokers)             // 王炸
    {
        showAnimation(JokerBomb,*(m_gameCtl->getCurrentPlayer()));
    }

    // 打空牌, 显示不要
    if(cards.isEmpty())
    {
        it->info->setPixmap(QPixmap(":/images/pass.png"));
        it->info->show();
        m_bgm->playPassMusic((BGMControl::MusicType)player->getCharacter().getSex());
    }
    else
    {
        if(m_gameCtl->getPendPlayer() == player || m_gameCtl->getPendPlayer() == nullptr)
        {
            //qDebug() << "玩家" << player << "本轮第一个出牌...";
            m_bgm->playCardMusic(cards, true, (BGMControl::MusicType)player->getCharacter().getSex());
        }
        else
        {
            //qDebug() << "玩家" << player << "本轮不是第一个出牌...";
            m_bgm->playCardMusic(cards, false, (BGMControl::MusicType)player->getCharacter().getSex());
        }
    }

    updatePlayerCards(player);

    // 播放剩余牌量
    if(player->getCards().cardCount() == 2)
    {
        m_bgm->playCardMusic(BGMControl::Last2, (BGMControl::MusicType)player->getCharacter().getSex());
    }
    else if(player->getCards().cardCount() == 1)
    {
        m_bgm->playCardMusic(BGMControl::Last1, (BGMControl::MusicType)player->getCharacter().getSex());
    }
}

void GamePanel::onGrabLordBet(Player *player, int bet, bool isfirst)
{
    //qDebug() << "bet: " << bet << "isfirst: " << isfirst;
    auto it = m_contextMap.find(player);
    if(bet == 0)
    {
        it->info->setPixmap(QPixmap(":/images/buqinag.png"));
    }
    else
    {
        if(isfirst)
        {
            it->info->setPixmap(QPixmap(":/images/jiaodizhu.png"));
        }
        else
        {
            it->info->setPixmap(QPixmap(":/images/qiangdizhu.png"));
        }
    }
    it->info->show();

    // 显示分数
    if(bet > 0)
    {
        showAnimation(Bet,*player,bet);
    }

    m_bgm->playRobLordMusic(bet, isfirst, (BGMControl::MusicType)player->getCharacter().getSex());
}

//void GamePanel::emitCardSend()
//{
//    qDebug() << "emitCardSend()";
//    gameStatusProcess(GameControl::DispatchCard);
//}

void GamePanel::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev)
    QPainter p(this);
    p.drawPixmap(rect(), m_bkImg);
}

void GamePanel::mouseMoveEvent(QMouseEvent *ev)
{
    if(ev->buttons() & Qt::LeftButton)
    {
        QPoint pt = ev->pos();
        //qDebug() << "cardrect: " << m_cardsRect << ", pt: " << pt;
        if(!m_cardsRect.contains(pt))
        {
            m_curSelCard = nullptr;
        }
        else
        {
            // 遍历非机器人玩家手中的扑克牌
            QList<CardPanel*> list = m_userCards.keys();
            for(int i=0; i<list.size(); ++i)
            {
                CardPanel* panel = list.at(i);
                if(m_curSelCard != panel && m_userCards[panel].contains(pt))
                {
                    panel->clicked();
                    m_curSelCard = panel;
                }
            }
        }
    }
}

void GamePanel::mousePressEvent(QMouseEvent *ev)
{
}

void GamePanel::mouseReleaseEvent(QMouseEvent *ev)
{
    m_curSelCard = nullptr;
}

void GamePanel::cropImage(QPixmap &pix, int x, int y, Card c)
{
    QPixmap img = pix.copy(x, y, m_cardSize.width(), m_cardSize.height());
    CardPanel* panel = new CardPanel(img, m_cardBackImg, c, this);
    panel->hide();
    m_cardMap.insert(c, panel);

    // 添加事件处理
    connect(panel, &CardPanel::cardSelected, this, &GamePanel::onCardSelected);
}

void GamePanel::initCardsMap()
{
    // 初始化扑克牌的大小
    QPixmap pix(":/images/card.png");
    // 图片中扑克牌有5行,13列
    m_cardSize.setWidth(pix.width()/13);
    m_cardSize.setHeight(pix.height()/5);

    // 背景图片 - 第5行, 第3列
    m_cardBackImg = pix.copy(2 * m_cardSize.width(), 4 * m_cardSize.height(),
                             m_cardSize.width(), m_cardSize.height());
    // 遍历花色
    for(int i=0, suit=Card::Suit_Begin+1; suit<Card::Suit_End; ++suit, ++i)
    {
        // 遍历13张扑克牌点数: 3,4,5,6,7,8,9,10,J,Q,K,A,2
        for(int j=0, pt=Card::Card_Begin+1; pt<Card::Card_SJ; ++pt, ++j)
        {
            Card card;
            card.m_point = (Card::CardPoint)pt; // 点数
            card.m_suit = (Card::CardSuit)suit; // 花色
            // 裁剪存储到map容器中
            cropImage(pix, j*m_cardSize.width(), i*m_cardSize.height(), card);
        }
    }
    // 大小王不属于任何一个花色, 单独处理
    Card card;
    // 小王
    card.m_point = Card::Card_SJ;
    card.m_suit = Card::Suit_Begin;
    cropImage(pix, 0, 4 * m_cardSize.height(), card);
    // 大王
    card.m_point = Card::Card_BJ;
    card.m_suit = Card::Suit_Begin;
    cropImage(pix, m_cardSize.width(), 4 * m_cardSize.height(), card);
}
