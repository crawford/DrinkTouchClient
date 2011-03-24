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
#include <QLabel>
#include "ibuttonhelper.h"

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
		QLabel *lblSplashStatus;
		QLabel *lblSplashError;

		QMap<QString, QWidget *> *panels;

		IButtonHelper *ibutton;
		QString currentUser;
		bool authenticating;

		void setupUi();
		void buildTabs(QSettings *);

	private slots:
		void handleSslErrors(QNetworkReply *);
		void handleNewIButton(QString);
		void handleError(QString);
		void authenticated(QString);
		void logout();
};

#endif // MAINWINDOW_H

