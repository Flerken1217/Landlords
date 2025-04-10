#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include <QLabel>
#include <QObject>
#include "robot.h"
#include "userplayer.h"

struct BetRecrod
{
    BetRecrod()
    {
        reset();
    }
    void reset()
    {
        player = nullptr;
        bet = 0;
        times = 0;
    }
    Player* player;
    int bet;
    int times;              // 记录当前是第几次叫地主
};

class GameControl : public QObject
{
    Q_OBJECT
public:
    // 游戏状态
    enum GameStatus
    {
        ExtractTianLaiZi,   //抽取天癞子状态
        DispatchCard,   // 发牌状态
        ExtractDiLaiZi,     //抽取地癞子状态
        CallingLord,        // 叫地主状态
        PlayingHand,  // 出牌状态
    };

    // 玩家状态
    enum PlayerStatus
    {
        ThinkingForCallLord,  // 考虑叫地主状态
        ThinkingForPlayHand,  // 考虑出牌
        Winning               // 获胜状态
    };



    explicit GameControl(QObject *parent = nullptr);

    // 初始化整副扑克牌信息到容器中
    void initAllCards();
    // 初始化玩家
    void playerInit();

    // 得到玩家的实例对象
    Robot* getLeftRobot();
    Robot* getRightRobot();
    UserPlayer* getUserPlayer();

    //春天状态
    void setSpring(bool bl=false);
    bool isSpring();

    //反春状态
    void setAntiSpring(bool bl=false);
    bool isAntiSpirng();

    static int TianLaiZi;            //记录天癞子的点数
    static int DiLaiZi;              //地癞子的点数
    //初始化天癞子
   void initTianLaiZi();

    //初始化地癞子
   void initDiLaiZi();


    // 得到/设置当前玩家对象
    Player* getCurrentPlayer();
    void setCurrentPlayer(Player* player);

    // 得到刚出完牌的玩家对象
    Player* getPendPlayer();
    // 得到玩家刚打出的牌
    Cards getPendCards();

    // 清空所有玩家的得分
    void clearPlayerScore();
    // 重置玩家卡牌数据
    void resetCardsData();

    // 随机发一张牌
    Card takeOneCard();
    // 得到剩余的牌
    Cards getSurplusCards();

    // 开始叫地主
    void startLordCard();

    // 玩家抢地主成功, 成为地主
    void becomeLord(Player* player, int bet);

    // 得到玩家叫地主过程中最高下注分数
    int getPlayerMaxBet();

    //处理卡牌的倍数叠加
    void computeMultipleForCard(Player& pendPlayer,Cards& pendCards);

public slots:
    // 处理叫地主
    void onGrabLordBet(Player* player, int score);
    // 处理出牌
    void onPlayHand(Player* player, Cards& card);

signals:
    // 通知抢地主下注
    void notifyGrabLordBet(Player* player, int score, bool isfirst);
    // 通知出牌
    void notifyPlayHand(Player* player, Cards& card);
    // 发牌
    void notifyPickCards(Player* player, Cards& card);


    // 传递出牌的玩家对象已经发出的牌
    void pendingInfo(Player* player, Cards& card);
    // 通知玩家状态变化了
    void playerStatusChanged(Player* player, PlayerStatus status);
    // 通知游戏状态变化了
    void gameStatusChanged(GameStatus status);

    //void hideLordCard();

private:
    int m_currBet;              // 当前这一局赌注的底分
    Robot* m_leftRobot;         // 左侧机器人玩家
    Robot* m_rightRobot;        // 右侧机器人玩家
    UserPlayer* m_user;         // 玩家自己
    Player* m_curPlayer;        // 当前玩家
    Cards m_allCards;           // 存储整副扑克牌
    BetRecrod m_betRecord;      // 记录叫地主信息
    Player* m_pendPlayer;       // 出牌的玩家
    Cards m_pendCards;          // 出牌的玩家打出的牌
    bool m_isSpring;            //判断该局是否春天
    bool m_isAntiSpring;        //判断该局是否反春


};

#endif // GAMECONTROL_H
