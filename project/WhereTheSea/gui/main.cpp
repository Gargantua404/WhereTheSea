#include <QApplication>

#include <player.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Player obj;

    obj.show();

    return app.exec();
}
