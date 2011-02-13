#include "drinkview.h"
#include "itembutton.h"
#include <QGridLayout>
#include <math.h>

#define MSG_STAT           "STAT\n"
#define MSG_BALANCE        "GETBALANCE\n"
#define MSG_OK             "OK"
#define PERFERRED_ROWS     6.0
#define FONT_SIZE          20
#define SOCKET_TIMEOUT     1000

DrinkView::DrinkView(QString host, int port, QWidget *parent) : QWidget(parent) {
    setLayout(new QGridLayout(this));
    this->host = host;
    this->port = port;

	credits = 0;

    socket = new QSslSocket(this);
    reconnectSocket();
}

void DrinkView::refresh() {
    QObjectList children = layout()->children();

    socket->write(MSG_STAT);
    if (socket->waitForReadyRead(SOCKET_TIMEOUT)) {
        while(!children.isEmpty()) {
            delete children.takeFirst();
        }
        parseStats();
    }

    socket->write(MSG_BALANCE);
    QByteArray res = waitForResponse();
    if (res.mid(0, 2) == MSG_OK) {
        credits = res.mid(res.indexOf(": ") + 2).trimmed().toInt();
    }
}

void DrinkView::parseStats() {
    QString line;
    QString item;
    QString price;
    QString count;
    QPixmap icon;
    QList<ItemButton *> buttons;
    int slot = 0;

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

        ItemButton *button = new ItemButton(item, count + " Remaining", price + " credits", QIcon(icon), this);
        QFont font = button->font();
        font.setPixelSize(FONT_SIZE);
        button->setFont(font);
        if (count == "0" || price.toInt() > credits) {
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
        ((QGridLayout *)layout())->addWidget(buttons.at(i), i % numRows, i / numRows);
    }
}

QByteArray DrinkView::waitForResponse() {
    int bytes = -1;
    do {
        socket->waitForReadyRead(SOCKET_TIMEOUT);
        if (socket->bytesAvailable() == -1) {
            //QMessageBox::critical(this, "Communication Error", "No response from host.");
            break;
        } else {
            bytes = socket->bytesAvailable();
        }
    } while(socket->peek(100).right(1) != "\n");

    return socket->readAll();
}

void DrinkView::reconnectSocket() {
    socket->connectToHost(host, port);
    if (socket->waitForConnected(SOCKET_TIMEOUT)) {
        waitForResponse();
    }
}

int DrinkView::getCredits() {
    return credits;
}
