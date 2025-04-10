#include "character.h"

//Character* ikki;
//Character* fuji ;
//Character* kakuya ;
//Character* man1 ;
//Character* man2 ;
//Character* man3 ;
//Character* man4 ;

Character::Character(QWidget *parent) : QWidget(parent)
{

}

Character::Character(QPixmap portrait,QPixmap activation,Sex sex)
{
    setPortrait(portrait);
    setActivation(activation);
    setSex(sex);
}


void Character::setPortrait(QPixmap& portrait)
{
    m_portrait = portrait;
    // 根据图片重置窗口大小
    resize(portrait.size());
    // 窗口重绘
    update();
}

QPixmap Character::getPortrait()const
{
    return m_portrait;
}

void Character::setActivation(QPixmap& activation)
{
    m_activation = activation;
    // 根据图片重置窗口大小
    resize(activation.size());
    // 窗口重绘
    update();
}

QPixmap Character::getActivation()const
{
    return m_activation;
}

void Character::setSex(Sex sex)
{
    m_sex=sex;
}

Character::Sex Character::getSex()const
{
    return m_sex;
}

Character& Character::operator=(const Character& other)
{
    this->m_portrait=other.getPortrait();
    this->m_activation=other.getActivation();
    this->m_sex=other.getSex();
    return *this;
}


