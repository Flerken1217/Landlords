#ifndef CARD_H
#define CARD_H

#include <QSet>
// 扑克牌的属性结构体
class Card
{
public:
    // 扑克牌的点数
    enum CardPoint
    {
        Card_Begin,
        Card_3,
        Card_4,
        Card_5,
        Card_6,
        Card_7,
        Card_8,
        Card_9,
        Card_10,
        Card_J,
        Card_Q,
        Card_K,
        Card_A,
        Card_2,
        Card_SJ,    // 小王 Small Joker
        Card_BJ,    // 大王 Big Joker
        Card_End
    };

    // 扑克牌的牌面属性
    enum CardSuit
    {
        Suit_Begin,
        Diamond,    // 方块
        Club,       // 梅花
        Heart,      // 红桃
        Spade,      // 黑桃
        Suit_End
    };

    Card(){}
    Card(CardPoint p, CardSuit s);
    CardPoint m_point;    // 扑克牌的点数
    CardSuit m_suit;      // 扑克牌的牌面属性
};

inline bool operator ==(const Card& left, const Card& right)
{
    return (left.m_point == right.m_point&& left.m_suit== right.m_suit);
}

inline uint qHash(const Card& card)
{
    return (card.m_point * 100 + card.m_suit);
}

//比较两张扑克牌的大小
inline bool lessSort(const Card& c1, const Card& c2)
{
    if (c1.m_point == c2.m_point)
    {
        return c1.m_suit < c2.m_suit;
    }
    else
    {
        return c1.m_point < c2.m_point;
    }
}

inline bool greaterSort(const Card& c1, const Card& c2)
{
    if (c1.m_point == c2.m_point)
    {
        return c1.m_suit > c2.m_suit;
    }
    else
    {
        return c1.m_point > c2.m_point;
    }
}

inline bool operator <(const Card& left, const Card& right)
{
    return lessSort(left, right);
}

using CardList = QVector<Card>;

#endif // CARD_H
