#include "drinkviewhelper.h"

DrinkViewHelper::DrinkViewHelper(DrinkView *parent) : QThread(0) {
	this->parent = parent;	

    socket = new QSslSocket(this);
	moveToThread(this);
}

void DrinkViewHelper::run() {
	exec();
}

void DrinkViewHelper::refresh() {
	qDebug() << "Sending: '" << MSG_STAT << "'";
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
	qDebug() << "Sending: '" << QString("ibutton %1\n").arg(id).toAscii().data() << "'";
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

	qDebug() << "Sending: '" << MSG_WHOAMI << "'";
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
	qDebug() << "Sending: '" << QString("DROP %1 0\n").arg(slot).toAscii() << "'";
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

