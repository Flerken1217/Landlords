#include "cardpanel.h"

#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

CardPanel::CardPanel(QWidget *parent) : QWidget(parent)
{
    m_isFront = true;
    m_isSelect = false;
    m_owner = nullptr;
}

// 委托构造函数
CardPanel::CardPanel(QPixmap &front, QPixmap &back, Card& c, QWidget *parent) :
    CardPanel(parent)
{
    setImage(front, back);
    setCard(c);
}
//复制构造函数
CardPanel::CardPanel(const CardPanel& other){
    m_front=other.m_front;    // 保存正面图片
    m_back=other.m_back;     // 保存背面图片
    // m_isFront=other.m_isFront;     // 记录当前是正面还是反面
    // m_isSelect=other.m_isSelect;    // 记录当前是否被选中
    // m_owner=other.m_owner;    // 记录当前这张牌的所有者
    m_card=other.m_card;        // 存储当前这张牌的属性信息

    setImage(m_front, m_back);
}

CardPanel& CardPanel::operator=(const CardPanel& other)
{
    if (this == &other)return *this;
    m_front=other.m_front;    // 保存正面图片
    m_back=other.m_back;     // 保存背面图片
    m_isFront=other.m_isFront;     // 记录当前是正面还是反面
    m_isSelect=other.m_isSelect;    // 记录当前是否被选中
    m_owner=other.m_owner;    // 记录当前这张牌的所有者
    m_card=other.m_card;        // 存储当前这张牌的属性信息
    return *this;

}

void CardPanel::setImage(QPixmap &front, QPixmap &back)
{
    m_front = front;
    m_back = back;
    // 根据图片重置窗口大小
    resize(front.size());
    // 窗口重绘
    update();
}

QPixmap CardPanel::getImage()
{
    return m_front;
}

void CardPanel::setOwner(Player *player)
{
    m_owner = player;
}

Player *CardPanel::getOwner()
{
    return m_owner;
}

void CardPanel::setCard(Card &card)
{
    m_card = card;
}

Card CardPanel::getCard()
{
    return m_card;
}

void CardPanel::setFrontSide(bool flag)
{
    m_isFront = flag;
}

bool CardPanel::isFrontSide()
{
    return m_isFront;
}

void CardPanel::setSelected(bool flag)
{
    m_isSelect = flag;
}

bool CardPanel::isSelected()
{
    return m_isSelect;
}

void CardPanel::clicked()
{
    emit cardSelected(Qt::LeftButton);
}

void CardPanel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if(m_isFront)
    {
        p.drawPixmap(rect(), m_front);
    }
    else
    {
        p.drawPixmap(rect(), m_back);
    }
}

void CardPanel::mousePressEvent(QMouseEvent *ev)
{
    emit cardSelected(ev->button());
}
