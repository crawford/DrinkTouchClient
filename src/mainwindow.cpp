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
#define CONFIG_IBUTTON     "IButtonFile"
#define CONFIG_SLOT_WIDTH  "/Layout/Width"
#define CONFIG_SLOT_HEIGHT "/Layout/Height"
#define CONFIG_SLOT_SIZES  "/Layout/Slots"
#define SPLASH_INDEX       0
#define SERVICES_INDEX     1
#define PROP_TYPE          "dr_type"
#define PROP_DEFAULT_URL   "dr_url"
#define MSG_TOUCH_IBUTTON  "Touch iButton to continue..."
#define MSG_AUTHENTICATING "Authenticating iButton..."

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QSettings *config = new QSettings(qApp->arguments().at(1), QSettings::IniFormat, this);

    ibutton = new IButtonHelper(config->value(CONFIG_IBUTTON).toString(), this);
    connect(ibutton, SIGNAL(newIButton(QString)), this, SLOT(handleNewIButton()));

    setupUi();
    buildTabs(config);

    delete config;

    ibutton->start();
}

MainWindow::~MainWindow() {
    ibutton->stop();
    qDebug() << "Waiting for ibutton thread to terminate";
    ibutton->wait(6000);
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

    //resize(1024, 768);
    showFullScreen();
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

            DrinkView *view;

            if (settings->contains(tab + CONFIG_SLOT_WIDTH) &&
                settings->contains(tab + CONFIG_SLOT_HEIGHT) &&
                settings->contains(tab + CONFIG_SLOT_SIZES)) {

                int width = settings->value(tab + CONFIG_SLOT_WIDTH).toInt();
                int height = settings->value(tab + CONFIG_SLOT_HEIGHT).toInt();
                QStringList strSizes = settings->value(tab + CONFIG_SLOT_SIZES).toString().split(',');
                QList<int> sizes;

                foreach (QString size, strSizes) {
                    sizes.append(size.toInt());
                }

                view = new DrinkView(host, port, height, width, sizes, tabServices);
            } else {
                view = new DrinkView(host, port, tabServices);
            }

            view->setProperty(PROP_TYPE, CONFIG_DRINK_TAG);
            connect(ibutton, SIGNAL(newIButton(QString)), view, SLOT(authenticate(QString)));
            connect(view, SIGNAL(hasUsername(QString)), this, SLOT(authenticated(QString)));
            connect(view, SIGNAL(error(QString)), this, SLOT(handleError(QString)));
            connect(view, SIGNAL(dropped()), this, SLOT(logout()));

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

void MainWindow::handleNewIButton() {
    lblSplashStatus->setText(MSG_AUTHENTICATING);
}

void MainWindow::handleError(QString error) {
    lblSplashStatus->setText(MSG_TOUCH_IBUTTON);
    lblSplashError->setText(error);

    logout();
}

void MainWindow::authenticated(QString username) {
    if (!currentUser.isEmpty()) {
        return;
    }

    //Make sure all of the drinkviews have auth'd
    foreach (QWidget *panel, panels->values()) {
        if (panel->property(PROP_TYPE) == CONFIG_DRINK_TAG) {
            if (!((DrinkView *)panel)->isAuthed()) {
                return;
            }
        }
    }

    currentUser = username;

    if (currentUser.isEmpty()){
        //Something went horribly wrong
        qDebug() << "CURRENT USER IS EMPTY";
        return;
    }

    foreach (QWidget *panel, panels->values()) {
        if (panel->property(PROP_TYPE) == CONFIG_DRINK_TAG) {
            DrinkView *view = (DrinkView *)panel;
            view->refresh();
            sbrStatus->clearMessage();
            sbrStatus->showMessage(QString("%1 (Credits: %2)").arg(currentUser).arg(view->getCredits()));
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
    lblSplashError->clear();
}

void MainWindow::logout() {
    sbrStatus->hide();
    ((QStackedLayout *)(wgtCentral->layout()))->setCurrentIndex(SPLASH_INDEX);
    currentUser.clear();

    foreach (QString key, panels->keys()) {
        if (panels->value(key)->property(PROP_TYPE) == CONFIG_WEB_TAG) {
            //Re-create the web instances
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
        } else if (panels->value(key)->property(PROP_TYPE) == CONFIG_DRINK_TAG) {
            //Logout each of the drink views
            ((DrinkView *)panels->value(key))->logout();
        }
    }

    ibutton->clearIButton();
}
