#include <QApplication>
#include <QMessageBox>
#include <QStackedLayout>
#include <QWebView>
#include <QLabel>
#include <QDebug>
#include <QStatusBar>
#include <QSslConfiguration>
#include <QFile>
#include "mainwindow.h"
#include "itembutton.h"
#include "drinkview.h"

#include <QWebHistory>

#define CONFIG_WEB_TAG     "web"
#define CONFIG_DRINK_TAG   "drink"
#define CONFIG_TYPE_SUB    "/Type"
#define CONFIG_ADDRESS_SUB "/Address"
#define CONFIG_PORT_SUB    "/Port"
#define CONFIG_CREDS       "CredentialsFile"
#define CONFIG_LDAP_URI    "LdapURI"
#define CONFIG_IBUTTON     "IButtonFile"
#define CREDS_USER_DN      "UserDN"
#define CREDS_PASSWORD     "Password"
#define SPLASH_INDEX       0
#define SERVICES_INDEX     1
#define PROP_TYPE          "dr_type"
#define PROP_DEFAULT_URL   "dr_url"
#define MSG_TOUCH_IBUTTON  "Touch iButton to continue..."
#define MSG_AUTHENTICATING "Authenticating iButton..."
#define MSG_LDAP_FAILURE   "Cannot connect to LDAP. Try again later."
#define MSG_INVALID_ID     "Invalid iButton. Try a different one."

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QSettings *config = new QSettings(qApp->arguments().at(1), QSettings::IniFormat, this);

    setupUi();
    buildTabs(config);

    QSettings creds(config->value(CONFIG_CREDS).toString(), QSettings::IniFormat, this);
    ldap = new LdapHelper(creds.value(CREDS_USER_DN).toString(), creds.value(CREDS_PASSWORD).toString(), config->value(CONFIG_LDAP_URI).toString());

    ibutton = new IButtonHelper(config->value(CONFIG_IBUTTON).toString(), this);
    connect(ibutton, SIGNAL(newIButton(QString)), this, SLOT(handleIButton(QString)));
    ibutton->start();

    delete config;
}

MainWindow::~MainWindow() {
    ibutton->stop();
    ibutton->wait(2000);
}

void MainWindow::setupUi() {
    wgtCentral = new QWidget(this);
    wgtSplash = new QWidget(wgtCentral);
    tabServices = new QTabWidget(wgtCentral);
    sbrStatus = new QStatusBar(wgtCentral);
    prgLoading = new QProgressBar(wgtCentral);
    btnLogout = new QPushButton("Logout", tabServices);
    lblSplashStatus = new QLabel(MSG_TOUCH_IBUTTON, this);
    lblSplashError = new QLabel(this);

    setObjectName("mainWindow");
    wgtCentral->setObjectName("wgtCentral");
    wgtSplash->setObjectName("wgtSplash");
    tabServices->setObjectName("tabCentral");
    sbrStatus->setObjectName("sbrStatus");
    prgLoading->setObjectName("prgLoading");
    btnLogout->setObjectName("btnLogout");
    lblSplashStatus->setObjectName("lblSplashStatus");
    lblSplashError->setObjectName("lblSplashError");

    lblSplashStatus->setAlignment(Qt::AlignCenter);
    lblSplashError->setAlignment(Qt::AlignCenter);

    sbrStatus->addWidget(prgLoading);
    sbrStatus->hide();
    prgLoading->hide();

    QBoxLayout *splashLayout = new QBoxLayout(QBoxLayout::TopToBottom, wgtSplash);
    wgtSplash->setLayout(splashLayout);
    splashLayout->addStretch();
    splashLayout->addWidget(lblSplashStatus);
    splashLayout->addWidget(lblSplashError);
    splashLayout->addStretch();

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
        if (settings->value(tab + CONFIG_TYPE_SUB).toString().toLower() == CONFIG_DRINK_TAG) {
            qDebug() << "Creating " << tab;

            QString host = settings->value(tab + CONFIG_ADDRESS_SUB).toString();
            int port = settings->value(tab + CONFIG_PORT_SUB).toInt();

            DrinkView *view = new DrinkView(host, port, tabServices);
            view->setProperty(PROP_TYPE, CONFIG_DRINK_TAG);

            panels->insert(tab, view);
            tabServices->addTab(view, tab);
        }
    }

    //Add the web services last
    foreach (QString tab, tabs) {
        if (settings->value(tab + CONFIG_TYPE_SUB).toString().toLower() == CONFIG_WEB_TAG) {
            qDebug() << "Creating " << tab;

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

void MainWindow::handleIButton(QString id) {
    if (!currentUser.isEmpty()) {
        return;
    }

    lblSplashStatus->setText(MSG_AUTHENTICATING);
    lblSplashStatus->repaint();

    if (ldap->connect()) {
        currentUser = ldap->getUserFromIButton(id);
        ldap->disconnect();
    } else {
        lblSplashStatus->setText(MSG_LDAP_FAILURE);
        lblSplashError->setText(ldap->getLastError());
        return;
    }

    if (currentUser.isEmpty()){
        lblSplashStatus->setText(MSG_INVALID_ID);
        lblSplashError->setText(ldap->getLastError());
        return;
    }

    foreach (QWidget *panel, panels->values()) {
        if (panel->property(PROP_TYPE) == CONFIG_DRINK_TAG) {
            ((DrinkView *)panel)->refresh();
        } else if (panel->property(PROP_TYPE) == CONFIG_WEB_TAG) {
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

    lblSplashStatus->setText(MSG_TOUCH_IBUTTON);
}

void MainWindow::logout() {
    sbrStatus->hide();
    ((QStackedLayout *)(wgtCentral->layout()))->setCurrentIndex(SPLASH_INDEX);
    currentUser.clear();

    //Re-create the web instances
    foreach (QString key, panels->keys()) {
        if (panels->value(key)->property(PROP_TYPE) == CONFIG_WEB_TAG) {
            QUrl url = panels->value(key)->property(PROP_DEFAULT_URL).toUrl();
            QWebView *old = (QWebView *)(panels->value(key));

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
