#include "SRDemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SRDemo w;
	w.setWindowState(Qt::WindowMaximized);
    w.showMaximized();
    return a.exec();
}
