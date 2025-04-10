#include "player.h"
#include "character.h"
#include <QRandomGenerator>

QSet<Character>Player::characters;
Character* Character::ikki;
Character* Character::fuji;
Character* Character::kakuya;
Character* Character::man1;
Character* Character::man2;
Character* Character::man3;
Character* Character::man4;

Player::Player(QObject *parent) : QObject(parent)
{
    m_score = 0;
    m_isWin = false;
    m_timesforSend=0;
    //m_sex = (Sex)QRandomGenerator::global()->bounded(2);

    // 得到一个有效的随机数: 0 ~ m_cards.size()-1
    int num = QRandomGenerator::global()->bounded(characters.size());
    QSet<Character>::const_iterator it = characters.constBegin();
    for(int i=0; i<num; ++i, it++);
    m_character = *it;

}

// 委托构造函数
Player::Player(QString name, QObject *parent) : Player(parent)
{
    setName(name);
}

void Player::initCharaters()
{
    characters.insert(*Character::ikki);
    characters.insert(*Character::fuji);
    characters.insert(*Character::kakuya);
    characters.insert(*Character::man1);
    characters.insert(*Character::man2);
    characters.insert(*Character::man3);
    characters.insert(*Character::man4);
}

void Player::setName(QString& name)
{
    m_name = name;
}

QString Player::getName()
{
    return m_name;
}

void Player::setPrevPlayer(Player *player)
{
    m_prevPlayer = player;
}

void Player::setNextPlayer(Player *player)
{
    m_nextPlayer = player;
}

Player *Player::getPrevPlayer()
{
    return m_prevPlayer;
}

Player *Player::getNextPlayer()
{
    return m_nextPlayer;
}

void Player::setScore(int score)
{
    m_score = score;
}

int Player::getScore()
{
    return m_score;
}

void Player::setRole(Role role)
{
    m_role = role;
}

Player::Role Player::getRole()
{
    return m_role;
}

void Player::setType(Type t)
{
    m_type = t;
}

Player::Type Player::getType()
{
    return m_type;
}

void Player::grabLordBet(int point)
{
    // 处理动作在GameControl类里边
    emit notifyGrabLordBet(this, point);
}

void Player::playHand(Cards &cards)
{
    m_cards.remove(cards);
    // 处理动作在GameControl类里边
    emit notifyPlayHand(this, cards);
}

void Player::clearCards()
{
    m_cards.clear();
}

//增加玩家出牌的次数
void Player::addTimesForSend(){
    m_timesforSend++;
}

//获取玩家出牌的次数
int Player::getTimesForSend(){
     return m_timesforSend;
}

//设置玩家当前的底分
void Player::setBottomScore(int bottomscore){
    m_bottomScore=bottomscore;
}
//获取当前玩家的底分
int Player::getBottomScore(){
    return m_bottomScore;
}

void Player::storeDispatchCard(Card &card)
{
    // 添加到set容器里边
    Cards cards;
    cards.add(card);
    storeDispatchCards(cards);
}

void Player::storeDispatchCards(Cards &cs)
{
    m_cards.add(cs);
    emit notifyPickCards(this, cs);
}

Cards Player::getCards()
{
    return m_cards;
}

void Player::thinkCallLord()
{

}

void Player::thinkPlayHand()
{

}

void Player::prepareCallLord()
{

}

void Player::preparePlayHand()
{

}

Player *Player::getPendPlayer()
{
    return m_pendPlayer;
}

Cards Player::getPendCards()
{
    return m_pendCards;
}

void Player::storePendingInfo(Player *player, Cards &card)
{
     m_pendPlayer = player;
     m_pendCards = card;
}

//Player::Sex Player::getSex()
//{
//    return m_sex;
//}

Character Player::getCharacter()
{
    return m_character;
}

void Player::setCharacter(const Character& other)
{
    m_character=other;
}

void Player::setDirection(Direction direct)
{
    m_direct = direct;
}

Player::Direction Player::getDirection()
{
    return m_direct;
}

bool Player::isWin()
{
    return m_isWin;
}

void Player::setWin(bool bl)
{
    m_isWin = bl;
}

//bool Player::isSpring()
//{
//    return m_isSpring;
//}

//void Player::setSpirng(bool bl)
//{
//    m_isSpring = bl;
//}

//获取当前倍数
int Player::getMultiple()const
{
    return m_multiple;
}
//设置当前倍数
void Player::setMultiple(int mul)
{
    m_multiple=mul;
}
