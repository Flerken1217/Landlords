#include "cards.h"
#include <QRandomGenerator>
#include <QVector>

Cards::Cards()
{
}

//返回当前所有牌
QSet<Card> Cards::getCards(){
    return m_cards;
}

void Cards::add(const Card &card)
{
    m_cards.insert(card);
}

//已经有了一些牌，再给一些新的牌
void Cards::add(const Cards &cards)
{
    m_cards.unite(cards.m_cards);
}

void Cards::add(const QVector<Cards> &array)
{
    for(int i=0; i<array.size(); ++i)
    {
        add(array.at(i));
    }
}

Cards &Cards::operator <<(const Cards &cards)
{
    add(cards);
    return *this;
}

Cards &Cards::operator<<(const Card &card)
{
    add(card);
    return *this;
}

Cards &Cards::operator<<(const QVector<Cards>& array)
{
    add(array);
    return *this;
}

void Cards::remove(const Card &card)
{
    m_cards.remove(card);
}

void Cards::remove(const Cards &cards)
{
    m_cards.subtract(cards.m_cards);
}

void Cards::remove(const QVector<Cards> &array)
{
    for(int i=0; i<array.size(); ++i)
    {
        remove(array.at(i));
    }
}

bool Cards::contains(const Card &card)
{
    return m_cards.contains(card);
}

bool Cards::contains(const Cards &cards)
{
    return m_cards.contains(cards.m_cards);
}

Card Cards::takeRandomCard()
{
    // 得到一个有效的随机数: 0 ~ m_cards.size()-1
    int num = QRandomGenerator::global()->bounded(m_cards.size());
    QSet<Card>::const_iterator it = m_cards.constBegin();
    for(int i=0; i<num; ++i, it++);
    Card card = *it;
    m_cards.erase(it);
    return card;
}

bool Cards::isEmpty()
{
    return m_cards.isEmpty();
}

void Cards::clear()
{
    m_cards.clear();
}

int Cards::cardCount()
{
    return m_cards.count();
}

int Cards::pointCount(Card::CardPoint pt)
{
    int count = 0;
    for(auto it = m_cards.begin(); it!=m_cards.end(); ++it)
    {
        if(it->m_point == pt)
        {
            count ++;
        }
    }
    return count;
}

Card::CardPoint Cards::minPoint()
{
    Card::CardPoint min = Card::Card_End;
    if(m_cards.isEmpty())
    {
        return min;
    }
    for(auto it = m_cards.begin(); it!=m_cards.end(); ++it)
    {
        if(it->m_point < min)
        {
            min = it->m_point;
        }
    }
    return min;
}

Card::CardPoint Cards::maxPoint()
{
    Card::CardPoint max = Card::Card_Begin;
    if(m_cards.isEmpty())
    {
        return max;
    }
    for(auto it = m_cards.begin(); it!=m_cards.end(); ++it)
    {
        if(it->m_point > max)
        {
            max = it->m_point;
        }
    }
    return max;
}

CardList Cards::toCardList(SortType type)
{
    CardList list;
    // 默认是数据是无序的
    for(auto it = m_cards.begin(); it != m_cards.end(); ++it)
    {
        list << *it;
    }
    if(type == Desc)
    {
        std::sort(list.begin(), list.end(), greaterSort);
    }
    else if(type == Asc)
    {
        std::sort(list.begin(), list.end(), lessSort);
    }
    return list;
}
