#include <QMessageBox>
#include <QBoxLayout>
#include <QWebView>
#include <QLabel>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#define CONFIG_WEB_TAG   "web"
#define CONFIG_DRINK_TAG "drink"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	QSettings *config = new QSettings(qApp->arguments().at(1), QSettings::IniFormat, this);

	setupUi();
	buildTabs(config);
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
		if (settings->value(tab + "/Type", "web").toString().toLower() == CONFIG_WEB_TAG) {
			QWebView *webview = new QWebView(this);

			webview->connect(webview, SIGNAL(loadProgress(int)), prgLoading, SLOT(setValue(int)));
			webview->load(QUrl(settings->value(tab + "/Address", "http://csh.rit.edu").toString()));
			sbrStatus->showMessage(webview->url().toString());

			tabCentral->addTab(webview, tab);
		} else if(settings->value(tab + "/Type", "drink").toString().toLower() == CONFIG_DRINK_TAG) {
			tabCentral->addTab(new QLabel("DRINK", this), tab);
		}
	}
}
