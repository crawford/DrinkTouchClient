#include <QApplication>
#include <QMessageBox>
#include <QBoxLayout>
#include <QWebView>
#include <QLabel>
#include <QDebug>
#include <QStatusBar>
#include <QSslConfiguration>
#include <QFile>
#include <math.h>
#include "mainwindow.h"
#include "itembutton.h"

#define CONFIG_WEB_TAG     "web"
#define CONFIG_DRINK_TAG   "drink"
#define CONFIG_TYPE_SUB    "/Type"
#define CONFIG_ADDRESS_SUB "/Address"
#define CONFIG_PORT_SUB    "/Port"
#define CONFIG_DEFAULT_PORT 4242
#define STAT_MESSAGE       "STAT\n"
#define PERFERRED_ROWS     6.0
#define FONT_SIZE          20

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QSettings *config = new QSettings(qApp->arguments().at(1), QSettings::IniFormat, this);

    setupUi();
    buildTabs(config);
    createConnections(config);
    refreshStats();
}

MainWindow::~MainWindow() {

}

void MainWindow::setupUi() {
    tabCentral = new QTabWidget(this);
    sbrStatus = new QStatusBar(this);
    prgLoading = new QProgressBar(this);

    sbrStatus->addWidget(prgLoading);

    setCentralWidget(tabCentral);
    setStatusBar(sbrStatus);

    resize(1024, 768);
}

void MainWindow::buildTabs(QSettings *settings) {
    QStringList tabs = settings->childGroups();
    panels = new QMap<QString, QWidget *>();

    foreach(QString tab, tabs) {
        qDebug() << "Creating " << tab;
        if (settings->value(tab + CONFIG_TYPE_SUB, "").toString().toLower() == CONFIG_WEB_TAG) {
            QWebView *webview = new QWebView(this);

            connect(webview, SIGNAL(loadProgress(int)), prgLoading, SLOT(setValue(int)));
            connect(webview->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(handleSslErrors(QNetworkReply*)));
            connect(webview, SIGNAL(loadFinished(bool)), prgLoading, SLOT(setHidden(bool)));
            connect(webview, SIGNAL(loadStarted()), prgLoading, SLOT(show()));

            webview->load(QUrl(settings->value(tab + CONFIG_ADDRESS_SUB, "http://csh.rit.edu").toString()));
            sbrStatus->showMessage(webview->url().toString());

            tabCentral->addTab(webview, tab);
        } else if(settings->value(tab + CONFIG_TYPE_SUB, "").toString().toLower() == CONFIG_DRINK_TAG) {
            QWidget *panel = new QWidget(this);
            panel->setLayout(new QGridLayout(this));
            panels->insert(tab, panel);
            tabCentral->addTab(panel, tab);
        }
    }
}

void MainWindow::handleSslErrors(QNetworkReply *reply) {
    reply->ignoreSslErrors();
}

void MainWindow::createConnections(QSettings *settings) {
    connections = new QMap<QString, QSslSocket *>();

    QStringList tabs = settings->childGroups();

    foreach(QString tab, tabs) {
        if (settings->value(tab + CONFIG_TYPE_SUB, "").toString().toLower() == CONFIG_DRINK_TAG) {
            QSslSocket *socket = new QSslSocket(this);
            QString host = settings->value(tab + CONFIG_ADDRESS_SUB, "<No Host>").toString();
            int port = settings->value(tab + CONFIG_PORT_SUB, CONFIG_DEFAULT_PORT).toInt();

            socket->connectToHost(host, port);
            if(socket->waitForConnected(2000)) {
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
        if(socket->bytesAvailable() == -1) {
            QMessageBox::critical(this, "Communication Error", "No response from host.");
            break;
        } else {
            bytes = socket->bytesAvailable();
        }
    } while(socket->peek(100).right(1) != "\n");

    return socket->readAll();
}

void MainWindow::refreshStats() {
    foreach(QString key, connections->keys()) {
        QSslSocket *socket = connections->value(key, 0);
        QWidget *panel = panels->value(key, 0);
        QObjectList children = panel->layout()->children();

        socket->write(STAT_MESSAGE);
        if(socket->waitForReadyRead(1000)) {
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
        if(line.mid(0, 2) == "OK") {
            break;
        }
        line.remove(0, line.indexOf('"') + 1);
        item = line.mid(0, line.indexOf('"'));
        line.remove(0, line.indexOf('"') + 2);
        price = line.mid(0, line.indexOf(' '));
        line.remove(0, line.indexOf(' ') + 1);
        count = line.mid(0, line.indexOf(' '));

        icon = QPixmap("logos/" + item.toLower().replace(".", "").replace(" ", "") + ".png");
        if(icon.isNull()) {
            icon = QPixmap("logos/default.png");
        }

        ItemButton *button = new ItemButton(item, count + " Remaining", price + " credits", QIcon(icon), this);
        QFont font = button->font();
        font.setPixelSize(FONT_SIZE);
        button->setFont(font);
        if(count == "0" || price.toInt() > 100) {
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
