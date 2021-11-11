#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "pedalinterface.h"
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PedalInterface w;
    w.show();
    return a.exec();
}
