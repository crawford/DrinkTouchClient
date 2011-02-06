#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QProgressBar>
#include <QMap>
#include <QSslSocket>
#include <QNetworkReply>
#include <QByteArray>

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

        QMap<QString, QSslSocket *> *connections;
        QMap<QString, QWidget *> *panels;

	void setupUi();
	void buildTabs(QSettings *);
	void createConnections(QSettings *);
        QByteArray waitForResponse(QSslSocket *);
        void refreshStats();
        void parseStats(QWidget *, QSslSocket *);

 private slots:
        void handleSslErrors(QNetworkReply *);
};

#endif // MAINWINDOW_H
