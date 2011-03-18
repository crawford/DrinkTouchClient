#include <QtGui/QApplication>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s config_file\n", argv[0]);
		exit(2);
	}

	QApplication a(argc, argv);
	MainWindow w;
	w.setCursor(Qt::BlankCursor);

	QSettings *settings = new QSettings(qApp->arguments().at(1), QSettings::IniFormat);
	if (settings->contains("StyleSheet")) {
		qDebug() << "Loading custom stylesheet";
		qApp->setStyleSheet(settings->value("StyleSheet").toString());
	}

	w.show();
	return a.exec();
}

