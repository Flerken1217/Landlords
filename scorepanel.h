#ifndef SCOREPANEL_H
#define SCOREPANEL_H

#include <QLabel>
#include <QWidget>
#include<QTableWidget>

namespace Ui {
class ScorePanel;
}

class ScorePanel : public QWidget
{
    Q_OBJECT

public:
    enum FontColor{Black, Red};
    explicit ScorePanel(QWidget *parent = nullptr);
    ~ScorePanel();

    // 设置分数
    void setScores(int left, int me, int right);

    // 设置字体颜色
    void setMyFontColor(FontColor color);
    // 设置字体大小
    void setMyFontSize(int point);

private:
    Ui::ScorePanel *ui;
    QVector<int> m_scores; // 存储每个分数的值
    QTableWidget *tableWidget; // 用于显示玩家信息和分数的表格
};

#endif // SCOREPANEL_H
