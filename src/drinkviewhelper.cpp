#include "drinkviewhelper.h"

#define DEBUG 0

DrinkViewHelper::DrinkViewHelper(DrinkView *parent) : QThread(0) {
	this->parent = parent;	

    socket = new QSslSocket(this);
	moveToThread(this);
}

void DrinkViewHelper::run() {
	exec();
}

void DrinkViewHelper::refresh() {
#if DEBUG
	qDebug() << "Sending: '" << MSG_STAT << "'";
#endif
    socket->write(MSG_STAT);
    if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
		buffer = socket->readAll();
		qDebug() << "Received:" << buffer;
		emit hasRefreshed();
	} else {
        qDebug() << "Timed out while waiting for stats";
    }
}

void DrinkViewHelper::authenticate(QString id) {
    if (!reconnectSocket())
        return;

    //Login to the drink server
#if DEBUG
	qDebug() << "Sending: '" << QString("ibutton %1\n").arg(id).toAscii().data() << "'";
#endif
    socket->write(QString("ibutton %1\n").arg(id).toAscii().data());
    if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
        //Parse the user's drink credits
        QString res = socket->readAll().trimmed();
        if (res.left(2) == MSG_OK) {
            parent->credits = res.mid(res.indexOf(": ") + 2).trimmed().toInt();
        }

        qDebug() << "Received: " << res;
    } else {
        qDebug() << "Timed out while logging in";
        parent->emit error("Network timeout");
        return;
    }

#if DEBUG
	qDebug() << "Sending: '" << MSG_WHOAMI << "'";
#endif
    socket->write(MSG_WHOAMI);
    QString res;
    if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
        res = socket->readAll().trimmed();
        qDebug() << "Received: " << res;
    } else {
        qDebug() << "Timed out while getting username";
        parent->emit error("Network timeout");
        return;
    }


    if (res.left(strlen(MSG_OK)) ==  MSG_OK) {
        parent->username = res.split(": ").at(1).trimmed();
        qDebug() << parent->username;

        parent->emit hasUsername(parent->username);
    } else {
        parent->emit error(MSG_INVALID_ID);
    }

	//refresh();
    //parent->emit hasUsername(parent->username);
}

bool DrinkViewHelper::reconnectSocket() {
    socket->connectToHost(parent->host, parent->port);
    if (socket->waitForConnected(SOCKET_TIMEOUT)) {
        if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
            qDebug() << "Received: " << socket->readAll().trimmed();
            return true;
        } else {
            qDebug() << "Timed out while waiting for response after reconnect";
            parent->emit error(QString("Could not reconnect to '%1'").arg(parent->host));
            return false;
        }
    } else {
        qDebug() << "Timed out while waiting for reconnect";
        parent->emit error(QString("Could not reconnect to '%1'").arg(parent->host));
        return false;
    }
}

void DrinkViewHelper::dropItem(int slot) {
#if DEBUG
	qDebug() << "Sending: '" << QString("DROP %1 0\n").arg(slot).toAscii() << "'";
#endif
	socket->write(QString("DROP %1 0\n").arg(slot).toAscii());

	if (socket->waitForReadyRead(DROP_TIMEOUT)) {
		buffer = socket->readAll();
		qDebug() << "Received:" << buffer;

        if (buffer.left(2) == MSG_OK) {
		    emit dropReturned(true, "");
		} else {
	        emit dropReturned(false, buffer.trimmed());
		}
	} else {
        qDebug() << "Timed out while waiting for drop";
		emit dropReturned(false, "Timeout");
    }
}

void DrinkViewHelper::disconnect() {
    socket->disconnectFromHost();
}

