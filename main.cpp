#include "gamepanel.h"
#include "cards.h"
#include "loading.h"
#include <QApplication>
#include <QResource>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<Cards>("Cards&");
    QResource::registerResource("./resource.rcc");

    Character::ikki = new Character(QPixmap(":/images/character/Ikki_portrait.png"),QPixmap(":/images/character/Kayoki_activation.png"),Character::Woman);
    Character::fuji = new Character(QPixmap(":/images/character/Fuji_portrait.png"), QPixmap(":/images/character/Kayoki_activation.png"),Character::Woman);
    Character::kakuya = new Character(QPixmap(":/images/character/Kayoki_portrait.png"),QPixmap(":/images/character/Kayoki_activation.png"),Character::Woman);
    Character::man1 = new Character(QPixmap(":/images/character/man1_portrait.png"), QPixmap(":/images/character/man1_activation.png"),Character::Man);
    Character::man2 = new Character(QPixmap(":/images/character/man2_portrait.png"),QPixmap(":/images/character/man2_activation.png"),Character::Man);
    Character::man3 = new Character(QPixmap(":/images/character/man3_portrait.png"), QPixmap(":/images/character/man3_activation.png"),Character::Man);
    Character::man4 = new Character(QPixmap(":/images/character/man4_portrait.png"),QPixmap(":/images/character/man4_activation.png"),Character::Man);

    Player::initCharaters();

    Loading window;
    window.show();

    return a.exec();
}
