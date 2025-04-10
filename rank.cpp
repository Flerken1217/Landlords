#include "rank.h"
#include "ui_rank.h"

Rank::Rank(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Rank)
{
    ui->setupUi(this);
}

Rank::~Rank()
{
    delete ui;
}
