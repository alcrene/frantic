#include <QApplication>
#include "brown_stats.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    StticLangevin mainUI;
    mainUI.show();

    mainUI.run();

    return app.exec();
}

