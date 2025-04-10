#ifndef CARDS_H
#define CARDS_H

#include <QObject>
#include <QSet>
#include "card.h"


// 该类用于处理所有的多张扑克牌
class Cards
{
public:
    // 扑克牌的排序方式: 升序, 降序, 不排序
    enum SortType{Asc, Desc, NoSort};
    explicit Cards();

    // 添加扑克牌
    void add(const Card& card);
    void add(const Cards& cards);
    void add(const QVector<Cards>& array);

    // 一次性添加多个对象
    Cards& operator <<(const Cards& cards);
    Cards& operator <<(const Card& card);
    Cards& operator <<(const QVector<Cards>& array);

    // 移除扑克牌
    void remove(const Card& card);
    void remove(const Cards& cards);
    void remove(const QVector<Cards>& array);
    // 判断手里是否有这张牌
    bool contains(const Card& card);
    bool contains(const Cards& cards);
    // 随机发一张牌
    Card takeRandomCard();

    // 判断玩家手里是否还有牌
    bool isEmpty();
    // 清空玩家手里的牌
    void clear();

    // 获得扑克牌的张数
    int cardCount();

    // 计算指定点数的扑克牌的张数
    int pointCount(Card::CardPoint pt);

    //返回当前所有牌
    QSet<Card> getCards();
    // 返回手里这副牌的最小点数
    Card::CardPoint minPoint();
    // 返回手里这副牌的最大点数
    Card::CardPoint maxPoint();

    // 把无序的QSet转换为有序的QVector，实现排序
    CardList toCardList(SortType type = Asc);


private:
    QSet<Card> m_cards;     // 存储玩家手里的扑克牌/整副扑克牌
};

#endif // CARDS_H
