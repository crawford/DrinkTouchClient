#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		exit(1);
	}

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
