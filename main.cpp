#include "mainwindow.h"

#include <QApplication>
#include"citylist.h"

int main(int argc, char *argv[])
{
    qputenv("QT_LOGGING_RULES", "qt.gui.imageio=false");
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    // CityList c;
    // c.show();
    return a.exec();
}
