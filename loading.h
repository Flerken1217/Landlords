#ifndef LOADING_H
#define LOADING_H

#include <QWidget>
#include <QMediaPlayer>
#include "bgmcontrol.h"

class Loading : public QWidget
{
    Q_OBJECT
public:
    explicit Loading(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);


private:
    QPixmap m_bk;
    QPixmap m_progress;
    BGMControl m_openVoice;
    int m_dist = 15;
};

#endif // LOADING_H
