#include <QApplication>
#include <QMessageBox>
#include <QBoxLayout>
#include <QWebView>
#include <QLabel>
#include <QDebug>
#include <QStatusBar>
#include <QSslConfiguration>
#include <QFile>
#include "mainwindow.h"

#define CONFIG_WEB_TAG     "web"
#define CONFIG_DRINK_TAG   "drink"
#define CONFIG_TYPE_SUB    "/Type"
#define CONFIG_ADDRESS_SUB "/Address"
#define CONFIG_PORT_SUB    "/Port"
#define CONFIG_DEFAULT_PORT 4242

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	QSettings *config = new QSettings(qApp->arguments().at(1), QSettings::IniFormat, this);

	setupUi();
	buildTabs(config);
	//createConnections(config);
}

MainWindow::~MainWindow() {

}

void MainWindow::setupUi() {
	tabCentral = new QTabWidget(this);
	sbrStatus = new QStatusBar(this);
	prgLoading = new QProgressBar(this);

	sbrStatus->addPermanentWidget(prgLoading);

	setCentralWidget(tabCentral);
	setStatusBar(sbrStatus);

	resize(1024, 768);
}

void MainWindow::buildTabs(QSettings *settings) {
	QStringList tabs = settings->childGroups();

	foreach(QString tab, tabs) {
		qDebug() << "Creating " << tab;
		if (settings->value(tab + CONFIG_TYPE_SUB, "").toString().toLower() == CONFIG_WEB_TAG) {
			QWebView *webview = new QWebView(this);

                        webview->connect(webview, SIGNAL(loadProgress(int)), prgLoading, SLOT(setValue(int)));
                        webview->connect(webview->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(handleSslErrors(QNetworkReply*)));

			webview->load(QUrl(settings->value(tab + CONFIG_ADDRESS_SUB, "http://csh.rit.edu").toString()));
			sbrStatus->showMessage(webview->url().toString());

                        tabCentral->addTab(webview, tab);
		} else if(settings->value(tab + CONFIG_TYPE_SUB, "").toString().toLower() == CONFIG_DRINK_TAG) {
			tabCentral->addTab(new QLabel("DRINK", this), tab);
		}
	}
}

void MainWindow::handleSslErrors(QNetworkReply *reply) {
    reply->ignoreSslErrors();
}

void MainWindow::createConnections(QSettings *settings) {
	connections = new QMap<QString, QTcpSocket *>();

	QStringList tabs = settings->childGroups();

	foreach(QString tab, tabs) {
		if (settings->value(tab + CONFIG_TYPE_SUB, "").toString().toLower() == CONFIG_DRINK_TAG) {
			QTcpSocket *socket = new QTcpSocket(this);
			QString host = settings->value(tab + CONFIG_ADDRESS_SUB, "<No Host>").toString();
			int port = settings->value(tab + CONFIG_PORT_SUB, CONFIG_DEFAULT_PORT).toInt();

			socket->connectToHost(host, port);
			if(socket->waitForConnected(2000)) {
				//waitForResponse(socket);
				connections->insert(tab, socket);
			} else {
				QMessageBox::critical(this, "Communication Error", "Could not establish a connection to " + host + ".");
			}
		}
	}
}
