#include <QApplication>
#include "langevin.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Langevin mainUI;
    mainUI.show();

    return app.exec();
}

