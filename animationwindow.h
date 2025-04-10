#ifndef ANIMATIONWINDOW_H
#define ANIMATIONWINDOW_H

#include <QWidget>
#include <player.h>
class AnimationWindow : public QWidget
{
    Q_OBJECT
public:
    enum Type{Sequence, Pair};
    explicit AnimationWindow(QWidget *parent = nullptr);

    void showPlane();

    void showBomb();

    void showJokerBomb();

    void showSequence(Type type);

    void showBetScore(int bet);

    void showSpring();

    void showAntiSpring();

    //展示角色技能
    void showCharacterSkill(Player& player);

signals:
    void animationOver();

protected:
    void paintEvent(QPaintEvent *event);

private:
    int m_x;
    int m_index;
    QPixmap m_image;
};

#endif // ANIMATIONWINDOW_H
