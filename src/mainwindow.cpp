#include <QApplication>
#include <QMessageBox>
#include <QStackedLayout>
#include <QWebView>
#include <QLabel>
#include <QDebug>
#include <QStatusBar>
#include <QSslConfiguration>
#include <QFile>
#include <math.h>
#include "mainwindow.h"
#include "itembutton.h"

#include <QWebHistory>

#define CONFIG_WEB_TAG     "web"
#define CONFIG_DRINK_TAG   "drink"
#define CONFIG_TYPE_SUB    "/Type"
#define CONFIG_ADDRESS_SUB "/Address"
#define CONFIG_PORT_SUB    "/Port"
#define CONFIG_CREDS       "CredentialsFile"
#define CONFIG_LDAP_URI    "LdapURI"
#define CREDS_USER_DN      "UserDN"
#define CREDS_PASSWORD     "Password"
#define STAT_MESSAGE       "STAT\n"
#define PERFERRED_ROWS     6.0
#define FONT_SIZE          20
#define SPLASH_INDEX       0
#define SERVICES_INDEX     1
#define PROP_TYPE          "dr_type"
#define PROP_DEFAULT_URL   "dr_url"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QSettings *config = new QSettings(qApp->arguments().at(1), QSettings::IniFormat, this);

    setupUi();
    buildTabs(config);
    createConnections(config);

    QSettings creds(config->value(CONFIG_CREDS).toString(), QSettings::IniFormat, this);
    ldap = new LdapHelper(creds.value(CREDS_USER_DN).toString(), creds.value(CREDS_PASSWORD).toString(), config->value(CONFIG_LDAP_URI).toString());

    delete config;
}

MainWindow::~MainWindow() {

}

void MainWindow::setupUi() {
    wgtCentral = new QWidget(this);
    wgtSplash = new QWidget(wgtCentral);
    tabServices = new QTabWidget(wgtCentral);
    sbrStatus = new QStatusBar(wgtCentral);
    prgLoading = new QProgressBar(wgtCentral);
    btnLogout = new QPushButton("Logout", tabServices);

    setObjectName("mainWindow");
    wgtCentral->setObjectName("wgtCentral");
    wgtSplash->setObjectName("wgtSplash");
    tabServices->setObjectName("tabCentral");
    sbrStatus->setObjectName("sbrStatus");
    prgLoading->setObjectName("prgLoading");
    btnLogout->setObjectName("btnLogout");

    sbrStatus->addWidget(prgLoading);
    prgLoading->hide();

    wgtSplash->setLayout(new QBoxLayout(QBoxLayout::LeftToRight, wgtSplash));
    wgtSplash->layout()->addWidget(new QLabel("Touch iButton to continue...", wgtSplash));
    QPushButton *button = new QPushButton("iButton", wgtSplash);
    connect(button, SIGNAL(clicked()), this, SLOT(handleIButton()));
    wgtSplash->layout()->addWidget(button);

    tabServices->setCornerWidget(btnLogout);
    connect(btnLogout, SIGNAL(clicked()), this, SLOT(logout()));

    wgtCentral->setLayout(new QStackedLayout(wgtCentral));
    ((QStackedLayout *)(wgtCentral->layout()))->insertWidget(SPLASH_INDEX, wgtSplash);
    ((QStackedLayout *)(wgtCentral->layout()))->insertWidget(SERVICES_INDEX, tabServices);

    setCentralWidget(wgtCentral);
    setStatusBar(sbrStatus);

    resize(1024, 768);
}

void MainWindow::buildTabs(QSettings *settings) {
    QStringList tabs = settings->childGroups();
    panels = new QMap<QString, QWidget *>();

    //Add the drink services first
    foreach (QString tab, tabs) {
        qDebug() << "Creating " << tab;
        if (settings->value(tab + CONFIG_TYPE_SUB).toString().toLower() == CONFIG_DRINK_TAG) {
            QWidget *panel = new QWidget(tabServices);
            panel->setProperty(PROP_TYPE, CONFIG_DRINK_TAG);

            panel->setLayout(new QGridLayout(panel));
            panels->insert(tab, panel);
            tabServices->addTab(panel, tab);
        }
    }

    //Add the web services last
    foreach (QString tab, tabs) {
        qDebug() << "Creating " << tab;
        if (settings->value(tab + CONFIG_TYPE_SUB).toString().toLower() == CONFIG_WEB_TAG) {
            QWebView *webview = new QWebView(tabServices);
            webview->setProperty(PROP_TYPE, CONFIG_WEB_TAG);

            connect(webview, SIGNAL(loadProgress(int)), prgLoading, SLOT(setValue(int)));
            connect(webview->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(handleSslErrors(QNetworkReply*)));
            connect(webview, SIGNAL(loadFinished(bool)), prgLoading, SLOT(setHidden(bool)));
            connect(webview, SIGNAL(loadStarted()), prgLoading, SLOT(show()));

            webview->setProperty(PROP_DEFAULT_URL, QUrl(settings->value(tab + CONFIG_ADDRESS_SUB).toString()));

            panels->insert(tab, webview);
            tabServices->addTab(webview, tab);
        }
    }
}

void MainWindow::handleSslErrors(QNetworkReply *reply) {
    reply->ignoreSslErrors();
}

void MainWindow::handleIButton() {
    if (ldap->connect()) {
        ldap->disconnect();
        currentUser = "abcrawf";
    }

    refreshStats();

    qDebug() << "Refreshed";

    //Reload all of the WebKit instances to their default page
    foreach (QWidget *panel, panels->values()) {
        if (panel->property(PROP_TYPE) == CONFIG_WEB_TAG) {
            //Build the POST request
            QByteArray postData = ("username=" + currentUser).toAscii();
            QNetworkRequest request(panel->property(PROP_DEFAULT_URL).toUrl());

            //Load the site
            ((QWebView *)(panel))->load(request, QNetworkAccessManager::PostOperation, postData);
        }
    }

    //Set focus to the first panel
    tabServices->setCurrentIndex(0);

    //Show the services view
    sbrStatus->show();
    ((QStackedLayout *)(wgtCentral->layout()))->setCurrentIndex(SERVICES_INDEX);
}

void MainWindow::logout() {
    sbrStatus->hide();
    ((QStackedLayout *)(wgtCentral->layout()))->setCurrentIndex(SPLASH_INDEX);

    //Re-create the web instances
    foreach (QString key, panels->keys()) {
        if (panels->value(key)->property(PROP_TYPE) == CONFIG_WEB_TAG) {
            QUrl url = panels->value(key)->property(PROP_DEFAULT_URL).toUrl();
            QWebView *old = (QWebView *)(panels->value(key));
            //delete panels->value(key);

            QWebView *webview = new QWebView(tabServices);
            webview->setProperty(PROP_TYPE, CONFIG_WEB_TAG);
            webview->setProperty(PROP_DEFAULT_URL, url);
            tabServices->removeTab(tabServices->indexOf(old));
            tabServices->addTab(webview, key);


            disconnect(old, SIGNAL(loadProgress(int)), prgLoading, SLOT(setValue(int)));
            disconnect(old->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(handleSslErrors(QNetworkReply*)));
            disconnect(old, SIGNAL(loadFinished(bool)), prgLoading, SLOT(setHidden(bool)));
            disconnect(old, SIGNAL(loadStarted()), prgLoading, SLOT(show()));

            connect(webview, SIGNAL(loadProgress(int)), prgLoading, SLOT(setValue(int)));
            connect(webview->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(handleSslErrors(QNetworkReply*)));
            connect(webview, SIGNAL(loadFinished(bool)), prgLoading, SLOT(setHidden(bool)));
            connect(webview, SIGNAL(loadStarted()), prgLoading, SLOT(show()));

            (*panels)[key] = webview;
            delete old;
        }
    }
}

void MainWindow::createConnections(QSettings *settings) {
    connections = new QMap<QString, QSslSocket *>();

    QStringList tabs = settings->childGroups();

    foreach (QString tab, tabs) {
        if (settings->value(tab + CONFIG_TYPE_SUB).toString().toLower() == CONFIG_DRINK_TAG) {
            QSslSocket *socket = new QSslSocket(this);
            QString host = settings->value(tab + CONFIG_ADDRESS_SUB, "<No Host>").toString();
            int port = settings->value(tab + CONFIG_PORT_SUB).toInt();

            socket->connectToHost(host, port);
            if (socket->waitForConnected(2000)) {
                qDebug() << waitForResponse(socket);
                connections->insert(tab, socket);
            } else {
                QMessageBox::critical(this, "Communication Error", "Could not establish a connection to " + host + ".");
            }
        }
    }
}

QByteArray MainWindow::waitForResponse(QSslSocket *socket) {
    int bytes = -1;
    do {
        socket->waitForReadyRead(5000);
        if (socket->bytesAvailable() == -1) {
            QMessageBox::critical(this, "Communication Error", "No response from host.");
            break;
        } else {
            bytes = socket->bytesAvailable();
        }
    } while(socket->peek(100).right(1) != "\n");

    return socket->readAll();
}

void MainWindow::refreshStats() {
    foreach (QString key, connections->keys()) {
        QSslSocket *socket = connections->value(key);
        QWidget *panel = panels->value(key);
        QObjectList children = panel->layout()->children();

        socket->write(STAT_MESSAGE);
        if (socket->waitForReadyRead(1000)) {
            while(!children.isEmpty()) {
                delete children.front();
                children.removeFirst();
            }
            parseStats(panel, socket);
        }
    }
}

void MainWindow::parseStats(QWidget *panel, QSslSocket *socket) {
    QString line;
    QString item;
    QString price;
    QString count;
    QPixmap icon;
    QList<ItemButton *> buttons;
    int slot = 0;

    while(!socket->atEnd()) {
        line = socket->readLine();
        if (line.mid(0, 2) == "OK") {
            break;
        }
        line.remove(0, line.indexOf('"') + 1);
        item = line.mid(0, line.indexOf('"'));
        line.remove(0, line.indexOf('"') + 2);
        price = line.mid(0, line.indexOf(' '));
        line.remove(0, line.indexOf(' ') + 1);
        count = line.mid(0, line.indexOf(' '));

        icon = QPixmap("logos/" + item.toLower().replace(".", "").replace(" ", "") + ".png");
        if (icon.isNull()) {
            icon = QPixmap("logos/default.png");
        }

        ItemButton *button = new ItemButton(item, count + " Remaining", price + " credits", QIcon(icon), this);
        QFont font = button->font();
        font.setPixelSize(FONT_SIZE);
        button->setFont(font);
        if (count == "0" || price.toInt() > 100) {
            button->setEnabled(false);
        }

        connect(button, SIGNAL(clicked()), button, SLOT(mark()));
        //connect(button, SIGNAL(clicked()), this, SLOT(processClick()));

        buttons.append(button);

        slot++;
    }

    double numCols = ceil(buttons.size() / PERFERRED_ROWS);
    int numRows = (int)ceil(buttons.size() / numCols);

    for (int i = 0; i < buttons.size(); i++) {
        ((QGridLayout *)panel->layout())->addWidget(buttons.at(i), i % numRows, i / numRows);
    }
}
