#ifndef CHARACTER_H
#define CHARACTER_H

#include <QWidget>

class Character : public QWidget
{
    Q_OBJECT
public:

    static Character* ikki ;
    static Character* fuji ;
    static Character* kakuya ;
    static Character* man1  ;
    static Character* man2  ;
    static Character* man3 ;
    static Character* man4 ;

    enum Sex{Man, Woman};

    explicit Character(QWidget *parent = nullptr);
    explicit Character(QPixmap portrait,QPixmap activation,Sex sex);
    Character(const Character& other) : m_portrait(other.m_portrait),m_activation(other.m_activation),m_sex(other.m_sex) {}
    void setPortrait(QPixmap& portrait);
    QPixmap getPortrait() const;
    void setActivation(QPixmap& portrait);
    QPixmap getActivation()const;
    void setSex(Sex sex);
    Sex getSex()const;

    // 重载赋值运算符
    Character& operator=(const Character& other) ;

signals:

private:
    QPixmap m_portrait;    // 普通立绘
    QPixmap m_activation;    // 精二立绘
    Sex m_sex;          //该角色的性别
};

inline bool operator ==(const Character& left, const Character& right)
{
    return (left.getPortrait() == right.getPortrait()
            && left.getActivation()== right.getActivation()
            &&left.getSex() == right.getSex());
}

inline uint qHash(const Character& character)
{
    return (character.getPortrait().width() + character.getActivation().width() );
}
#endif // CHARACTER_H
