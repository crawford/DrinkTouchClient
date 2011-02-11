#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QProgressBar>
#include <QMap>
#include <QSslSocket>
#include <QNetworkReply>
#include <QByteArray>
#include <QPushButton>
#include "ldaphelper.h"

namespace Ui {
    class mainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QTabWidget *tabServices;
    QStatusBar *sbrStatus;
    QProgressBar *prgLoading;
    QWidget *wgtCentral;
    QWidget *wgtSplash;
    QPushButton *btnLogout;

    QMap<QString, QSslSocket *> *connections;
    QMap<QString, QWidget *> *panels;

    LdapHelper *ldap;
    QString currentUser;

    void setupUi();
    void buildTabs(QSettings *);
    void createConnections(QSettings *);
    QByteArray waitForResponse(QSslSocket *);
    void refreshStats();
    void parseStats(QWidget *, QSslSocket *);

private slots:
    void handleSslErrors(QNetworkReply *);
    void handleIButton();
    void logout();
};

#endif // MAINWINDOW_H
