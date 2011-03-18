#ifndef DRINKVIEWHELPER_H
#define DRINKVIEWHELPER_H

#include "drinkview.h"
#include <QThread>

#define MSG_STAT           "STAT\n"
#define MSG_BALANCE        "GETBALANCE\n"
#define MSG_OK             "OK"
#define MSG_WHOAMI         "WHOAMI\n"
#define MSG_INVALID_ID     "Invalid iButton. Try a different one."
#define PERFERRED_ROWS     6.0
#define FONT_SIZE          20
#define SOCKET_TIMEOUT     15000
#define DROP_TIMEOUT       30000

class DrinkView;

class DrinkViewHelper : public QThread {
	Q_OBJECT

	public:
		DrinkViewHelper(DrinkView *);
		bool reconnectSocket();
		void disconnect();

		QString buffer;

	public slots:
		void authenticate(QString);
		void dropItem(int);
		void refresh();

	signals:
		void hasRefreshed();
		void dropReturned(bool, QString);

	protected:
		void run();

	private:
		DrinkView *parent;
		QSslSocket *socket;
};

#endif // DRINKVIEWHELPER_H

