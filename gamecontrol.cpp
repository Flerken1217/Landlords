#include "gamecontrol.h"
#include "playhand.h"

#include <QTimer>
#include <QDebug>
#include<QRandomGenerator>

int GameControl::TianLaiZi;            //记录天癞子的点数
int GameControl::DiLaiZi;              //地癞子的点数

GameControl::GameControl(QObject *parent) : QObject(parent)
{
    m_pendPlayer = nullptr;
    m_pendCards = Cards();
    //m_currBet = 0;
    m_isSpring=false;
    m_isAntiSpring=false;
}

void GameControl::initAllCards()
{
    m_allCards.clear();
    for(int p=Card::Card_Begin+1; p<Card::Card_SJ; ++p)
    {
        for(int s=Card::Suit_Begin+1; s<Card::Suit_End; ++s)
        {
            Card c((Card::CardPoint)p, (Card::CardSuit)s);
            m_allCards.add(c);
        }
    }
    // 添加大小王
    m_allCards.add(Card(Card::Card_SJ, Card::Suit_Begin));
    m_allCards.add(Card(Card::Card_BJ, Card::Suit_Begin));
}

void GameControl::playerInit()
{
    // 实例化玩家对象 && 设置玩家名字
    m_leftRobot = new Robot("机器人A", this);
    m_rightRobot = new Robot("机器人B", this);
    m_user = new UserPlayer("玩家", this);

    // 设置玩家头像显示的位置: 左侧 or 右侧
    m_leftRobot->setDirection(Player::Left);
    m_rightRobot->setDirection(Player::Right);
    m_user->setDirection(Player::Right);

    // 设置出牌顺序 --> 逆时针, 左侧是上家, 右侧是下家
    // 左侧机器人
    m_leftRobot->setPrevPlayer(m_rightRobot);
    m_leftRobot->setNextPlayer(m_user);
    // 非机器人玩家
    m_user->setPrevPlayer(m_leftRobot);
    m_user->setNextPlayer(m_rightRobot);
    // 右侧机器人
    m_rightRobot->setPrevPlayer(m_user);
    m_rightRobot->setNextPlayer(m_leftRobot);

    // 三个玩家进行对接
    // 抢地主
    connect(m_leftRobot, &UserPlayer::notifyGrabLordBet, this, &GameControl::onGrabLordBet);
    connect(m_rightRobot, &UserPlayer::notifyGrabLordBet, this, &GameControl::onGrabLordBet);
    connect(m_user, &UserPlayer::notifyGrabLordBet, this, &GameControl::onGrabLordBet);

    // 出牌
    connect(m_leftRobot, &UserPlayer::notifyPlayHand, this, &GameControl::onPlayHand);
    connect(m_rightRobot, &UserPlayer::notifyPlayHand, this, &GameControl::onPlayHand);
    connect(m_user, &UserPlayer::notifyPlayHand, this, &GameControl::onPlayHand);

    // 发牌
    connect(m_leftRobot, &UserPlayer::notifyPickCards, this, &GameControl::notifyPickCards);
    connect(m_rightRobot, &UserPlayer::notifyPickCards, this, &GameControl::notifyPickCards);
    connect(m_user, &UserPlayer::notifyPickCards, this, &GameControl::notifyPickCards);

    // 传递待处理的玩家和扑克牌数据
    connect(this, &GameControl::pendingInfo, m_leftRobot, &Robot::storePendingInfo);
    connect(this, &GameControl::pendingInfo, m_rightRobot, &Robot::storePendingInfo);
    connect(this, &GameControl::pendingInfo, m_user, &UserPlayer::storePendingInfo);

    // 指定默认的当前玩家
    m_curPlayer = m_user;
}


//设置春天状态
void GameControl::setSpring(bool bl){
    m_isSpring=bl;
}
bool GameControl::isSpring(){
    return m_isSpring;
}

//设置为反春状态
void GameControl::setAntiSpring(bool bl){
    m_isAntiSpring=bl;
}
bool GameControl::isAntiSpirng(){
    return m_isAntiSpring;
}

//设置天癞子
void GameControl::initTianLaiZi()
{
    //生成从Card_3到Card_2的随机数，只存储int即可，因为癞子没有花色限制
    TianLaiZi = QRandomGenerator::global()->bounded(Card::Card_2)+Card::Card_3;
}


//设置地癞子
void GameControl::initDiLaiZi(){
    while(true){
        DiLaiZi = QRandomGenerator::global()->bounded(Card::Card_2)+Card::Card_3;
        if(DiLaiZi!=TianLaiZi)break;//确保天地癞子不一样
    }
}

Robot *GameControl::getLeftRobot()
{
    return m_leftRobot;
}

Robot *GameControl::getRightRobot()
{
    return m_rightRobot;
}

UserPlayer *GameControl::getUserPlayer()
{
    return m_user;
}

Player *GameControl::getCurrentPlayer()
{
    return m_curPlayer;
}

void GameControl::setCurrentPlayer(Player *player)
{
    m_curPlayer = player;
}

Player *GameControl::getPendPlayer()
{
    return m_pendPlayer;
}

Cards GameControl::getPendCards()
{
    return m_pendCards;
}

void GameControl::clearPlayerScore()
{
    m_leftRobot->setScore(0);
    m_rightRobot->setScore(0);
    m_user->setScore(0);
}

void GameControl::resetCardsData()
{
    initAllCards();

    m_leftRobot->clearCards();
    m_rightRobot->clearCards();
    m_user->clearCards();

    // 数据重置, 还没有出牌的玩家, 更没有打出任何扑克牌
    m_pendPlayer = nullptr;
    m_pendCards.clear();
    // 传递信息, 用于其它对象的初始化
    emit pendingInfo(m_pendPlayer, m_pendCards);
}

Card GameControl::takeOneCard()
{
    return m_allCards.takeRandomCard();
}

Cards GameControl::getSurplusCards()
{
    return m_allCards;
}

void GameControl::startLordCard()
{
    emit playerStatusChanged(m_curPlayer, ThinkingForCallLord);
    m_curPlayer->prepareCallLord();
}


void GameControl::becomeLord(Player *player, int bet)
{
    m_currBet = bet;
    player->setRole(Player::Lord);
    player->getPrevPlayer()->setRole(Player::Farmer);
    player->getNextPlayer()->setRole(Player::Farmer);

    // 设置地主为当前玩家
    m_curPlayer = player;
    // 拿到剩下的三张底牌
    player->storeDispatchCards(m_allCards);
    m_allCards.clear();

    //emit hideLordCard();

    // 延时等待一下再开始出牌的过程 --> 使用定时器
    QTimer::singleShot(1000, this, [=](){
        // 游戏状态发生变化 -> 出牌状态
        //emit gameStatusChanged(PlayingHand);
        emit gameStatusChanged(ExtractDiLaiZi);
        // 玩家状态发生变化 -> 考虑出牌状态
        //emit playerStatusChanged(m_curPlayer, ThinkingForPlayHand);
        // 玩家出牌
        m_curPlayer->preparePlayHand();

    });
}

int GameControl::getPlayerMaxBet()
{
    return m_betRecord.bet;
}

void GameControl::onGrabLordBet(Player *player, int score)
{    
    //qDebug() << "current score: " << score;
    //qDebug() << "m_betRecord.bet: " << m_betRecord.bet;

    player->setBottomScore(score);

    // 如果分数为0, 或者 分数不比上家大则视为无效分数, 不抢地主
    if(score == 0 || m_betRecord.bet >= score)
    {
        emit notifyGrabLordBet(player, 0, false);
        //qDebug() << "emit notifyGrabLordBet(player, 0, false)";
    }
    else if(score > 0 && m_betRecord.bet == 0)
    {
        // 第一次叫地主
        emit notifyGrabLordBet(player, score, true);
        //qDebug() << "emit notifyGrabLordBet(player, score, true)";
    }
    else
    {
        // 不是第一次, 开始抢地主
        emit notifyGrabLordBet(player, score, false);
        //qDebug() << "emit notifyGrabLordBet(player, score, false)";
    }

    // 抢地主下注为3分，抢地主结束
    if(score == 3)
    {
        // 直接当地主
        becomeLord(player, score);
        // 本轮数据清空
        m_betRecord.reset();
        return;
    }


    // 没有人叫3分, 比较分数, 如果分数高, 将该玩家和分数存储起来
    if(m_betRecord.bet < score)
    {
        m_betRecord.bet = score;        // 更新分数
        m_betRecord.player = player;    // 更新玩家
    }
    m_betRecord.times++;

    // 每个人都叫过一次地主, 做最后的处理
    if(m_betRecord.times == 3)
    {
        if(m_betRecord.bet == 0)
        {
            // 都放弃叫地主, 重新发牌
            emit gameStatusChanged(DispatchCard);
        }
        else
        {
            becomeLord(m_betRecord.player, m_betRecord.bet);
        }
        m_betRecord.reset();
        return;
    }

    // 让下一个玩家叫分
    Player* nextPlayer = player->getNextPlayer();
    m_curPlayer = nextPlayer;
    // 发信号 -> 玩家状态发送了变化(考虑叫地主)
    emit playerStatusChanged(m_curPlayer, ThinkingForCallLord);
    // 玩家叫地主
    m_curPlayer->prepareCallLord();
}

// 所有player玩家出牌
void GameControl::onPlayHand(Player *player, Cards &cards)
{
    // 该信号的处理者是 GamePanel 类的对象
    //qDebug() << m_curPlayer << player;
    emit notifyPlayHand(m_curPlayer, cards);

    if(!cards.isEmpty())
    {
        // 如果打出的牌不为空, 记录出牌人和打出的牌
        m_pendCards = cards;
        m_pendPlayer = player;
        player->addTimesForSend();//记录当前玩家的出牌次数
        //计算成绩倍数
        computeMultipleForCard(*player,cards);
        emit pendingInfo(player, cards);
    }

    // player已经把牌出完了, 计算得分
    if(player->getCards().isEmpty())
    {
        Player* prev = player->getPrevPlayer();
        Player* next = player->getNextPlayer();

        //判断是否春天和反春天
        //当前玩家是地主
        if(player->getRole() == Player::Lord){
            int allRest=(54-3)/3;//总数为54，减去三张地主的牌每人应该有多少张牌
            //两个农民均是一张牌都没出
            if(prev->getCards().cardCount()==allRest&&next->getCards().cardCount()==allRest)
            {
                setSpring(true);
            }
        }else//当前玩家是农民
        {
            //如果前一个玩家是地主且出牌次数为1 or 后一个玩家为地主且出牌次数为1
            if((prev->getRole()== Player::Lord&&prev->getTimesForSend()==1)
                    ||(next->getRole()== Player::Lord&&next->getTimesForSend()==1))
                setAntiSpring(true);//设置为反春状态
        }


        //依据不同的设置计分规则
        //当前player是地主且胜利，按照地主的胜利的计分方式来算
        if(player->getRole() == Player::Lord)
        {
            player->setWin(true);
            prev->setWin(false);
            prev->setWin(false);
            player->setScore(player->getScore() + 2 * m_currBet);
            prev->setScore(prev->getScore() - m_currBet);
            next->setScore(next->getScore() - m_currBet);
            if(isSpring())
                player->setScore(player->getScore()*2);
        }
        else //player是农民且胜利
        {
            player->setWin(true);
            player->setScore(player->getScore() + m_currBet);
            if(prev->getRole() == Player::Lord)
            {
                prev->setScore(prev->getScore() - 2 * m_currBet);
                next->setScore(next->getScore() -  m_currBet);
                if(isAntiSpirng()){//是反春，所有农民分数翻两倍
                    player->setScore(player->getScore()*2);
                    next->setScore(next->getScore()*2);
                }
                prev->setWin(false);
                next->setWin(true);
            }
            else
            {
                prev->setScore(prev->getScore() -  m_currBet);
                next->setScore(next->getScore() - 2 * m_currBet);
                if(isAntiSpirng()){//是反春，所有农民分数翻两倍
                    player->setScore(player->getScore()*2);
                    prev->setScore(prev->getScore()*2);
                }
                prev->setWin(true);
                next->setWin(false);
            }

        }

        // 玩家状态变化了
        emit playerStatusChanged(player, GameControl::Winning);
        return;
    }
    // 出完牌, 轮到下一个玩家
    m_curPlayer = player->getNextPlayer();
    emit playerStatusChanged(m_curPlayer, GameControl::ThinkingForPlayHand);
    m_curPlayer->preparePlayHand();
}

void GameControl::computeMultipleForCard(Player& pendPlayer,Cards& pendCards)
{
    //把刚刚打出的牌传进来
    PlayHand hand(pendCards);
    //判断牌的类型
    PlayHand::HandType type = hand.getType();
    // 判断牌型
    //4张牌的炸翻两倍，5-8张牌的炸翻6倍，王炸翻6倍，王炸带任何东西都翻6倍
    switch(type)
    {
    case PlayHand::Hand_Bomb_Jokers:                // 王炸
    case PlayHand::Hand_Bomb_Pair:                  // 炸弹带一对
    case PlayHand::Hand_Bomb_Two_Single:            // 炸弹带两单
    case PlayHand::Hand_Bomb_Jokers_Pair:           // 王炸带一对
    case PlayHand::Hand_Bomb_Jokers_Two_Single:     // 王炸带两单
    case PlayHand::Hand_Bomb_thriple:              // 炸弹带三连对
    case PlayHand::Hand_Bomb_Two_Bomb:               // 两个炸弹
        pendPlayer.setMultiple(6);
        break;
    case PlayHand::Hand_Bomb:                       // 炸弹
        pendPlayer.setMultiple(4);
        break;
    default:
        break;
    }
}
