#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QProgressBar>
#include <QMap>
#include <QTcpSocket>
#include <QNetworkReply>

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

	QMap<QString, QTcpSocket *> *connections;

	void setupUi();
	void buildTabs(QSettings *);
	void createConnections(QSettings *);

 private slots:
        void handleSslErrors(QNetworkReply *);
};

#endif // MAINWINDOW_H
