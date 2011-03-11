#include "drinkview.h"
#include <QGridLayout>
#include <QTimer>
#include <math.h>
#include <limits>

#define MSG_STAT           "STAT\n"
#define MSG_BALANCE        "GETBALANCE\n"
#define MSG_OK             "OK"
#define MSG_WHOAMI         "WHOAMI\n"
#define MSG_INVALID_ID     "Invalid iButton. Try a different one."
#define PERFERRED_ROWS     6.0
#define FONT_SIZE          20
#define SOCKET_TIMEOUT     1000
#define DROP_TIMEOUT       10000

DrinkView::DrinkView(QString host, int port, QWidget *parent) : QWidget(parent) {
    init(host, port);
}

DrinkView::DrinkView(QString host, int port, int height, int width, QList<int> sizes, QWidget *parent) : QWidget(parent) {
    init(host, port);

    slotsHeight = height;
    slotsWidth = width;
    slotSizes = sizes;
}

void DrinkView::init(QString host, int port) {
    setLayout(new QGridLayout(this));
    this->host = host;
    this->port = port;

    msgbox = NULL;
    buttons = new QList<ItemButton *>();

    credits = 0;
    username = "";
    slotsHeight = 0;
    slotsWidth = 0;

    socket = new QSslSocket(this);
}

void DrinkView::authenticate(QString id) {
    if (!reconnectSocket())
        return;

    //Login to the drink server
    socket->write(QString("ibutton %1\n").arg(id).toAscii().data());
    if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
        //Parse the user's drink credits
        QString res = socket->readAll().trimmed();
        if (res.left(2) == MSG_OK) {
            credits = res.mid(res.indexOf(": ") + 2).trimmed().toInt();
        }

        qDebug() << "Received: " << res;
    } else {
        qDebug() << "Timed out while logging in";
        emit error("Network timeout");
        return;
    }

    socket->write(MSG_WHOAMI);
    QString res;
    if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
        res = socket->readAll().trimmed();
        qDebug() << "Received: " << res;
    } else {
        qDebug() << "Timed out while getting username";
        emit error("Network timeout");
        return;
    }


    if (res.left(strlen(MSG_OK)) ==  MSG_OK) {
        username = res.split(": ").at(1).trimmed();
        qDebug() << username;

        emit hasUsername(username);
    } else {
        emit error(MSG_INVALID_ID);
    }
}

void DrinkView::logout() {
    socket->disconnectFromHost();
    username = "";
}

void DrinkView::refresh() {
    if (msgbox) {
        msgbox->close();
    }

    socket->write(MSG_STAT);
    if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
        //Delete all of the buttons
        while (!buttons->isEmpty()) {
            delete buttons->takeFirst();
        }

        parseStats();
    } else {
        qDebug() << "Timed out while waiting for stats";
    }
}

void DrinkView::parseStats() {
    QString line;
    QString item;
    QString price;
    QString count;
    QPixmap icon;
    int slot = 1;

    while(!socket->atEnd()) {
        line = socket->readLine();
        if (line.mid(0, 2) == MSG_OK) {
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

        ItemButton *button = new ItemButton(item, count + " Remaining", price + " credits", QIcon(icon), slot, this);

        QFont font = button->font();
        font.setPixelSize(FONT_SIZE);
        button->setFont(font);
        if (count == "0" || price.toInt() > credits) {
            button->setEnabled(false);
        }

        //connect(button, SIGNAL(clicked()), button, SLOT(mark()));
        connect(button, SIGNAL(clicked(ItemButton*)), this, SLOT(handleClick(ItemButton*)));

        buttons->append(button);

        slot++;
    }

    if (slotsWidth == 0) {
        int numCols = ceil(buttons->size() / PERFERRED_ROWS);

        for (int i = 0; i < buttons->size(); i++) {
            ((QGridLayout *)layout())->addWidget(buttons->at(i), i / numCols, i % numCols);
        }
    } else {
        int row = 0;
        int col = 0;

        for (int i = 0; i < buttons->size(); i++) {
            ((QGridLayout *)layout())->addWidget(buttons->at(i), row, col, 1, slotSizes.at(i));
            col += slotSizes.at(i);
            if (col >= slotsWidth) {
                row++;
                col = 0;
            }
        }
    }
}

bool DrinkView::reconnectSocket() {

    socket->connectToHost(host, port);
    if (socket->waitForConnected(SOCKET_TIMEOUT)) {
        if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
            qDebug() << "Received: " << socket->readAll().trimmed();
            return true;
        } else {
            qDebug() << "Timed out while waiting for response after reconnect";
            emit error(QString("Could not reconnect to '%1'").arg(host));
            return false;
        }
    } else {
        qDebug() << "Timed out while waiting for reconnect";
        emit error(QString("Could not reconnect to '%1'").arg(host));
        return false;
    }
}

int DrinkView::getCredits() {
    return credits;
}

bool DrinkView::isAuthed() {
    return !username.isEmpty();
}

void DrinkView::handleClick(ItemButton *button) {
    socket->write(QString("DROP %1 0\n").arg(button->getSlot()).toAscii());

    if (msgbox) {
        delete msgbox;
    }
    msgbox = new QMessageBox(QMessageBox::Information, "Dropping", "Dropping your item!", QMessageBox::NoButton, this);
    msgbox->setStandardButtons(0);
    msgbox->show();

    QString res;
    if (socket->waitForReadyRead(DROP_TIMEOUT)) {
        res = socket->readAll().trimmed();
        qDebug() << "Received: " << res;
    } else {
        qDebug() << "Drop timmed out";
    }

    if (res != MSG_OK) {
        msgbox->close();
        delete msgbox;

        msgbox = new QMessageBox(QMessageBox::Critical, "Drop Error", QString("The item could not be dropped (%1).").arg(res));
        msgbox->show();
    } else {
        handleDropTimeout();
    }
}

void DrinkView::handleDropTimeout() {
    msgbox->close();
    delete msgbox;
    msgbox = NULL;
    emit dropped();
}
