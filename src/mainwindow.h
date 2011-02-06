#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QSettings>
#include <QProgressBar>

namespace Ui {
	class mainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	QTabWidget *tabCentral;
	QStatusBar *sbrStatus;
	QProgressBar *prgLoading;

	void setupUi();
	void buildTabs(QSettings *);
};

#endif // MAINWINDOW_H
