#include "drinkview.h"
#include "itembutton.h"
#include <QGridLayout>
#include <math.h>

#define STAT_MESSAGE       "STAT\n"
#define PERFERRED_ROWS     6.0
#define FONT_SIZE          20

DrinkView::DrinkView(QString host, int port, QWidget *parent) : QWidget(parent) {
    setLayout(new QGridLayout(this));
    this->host = host;
    this->port = port;

    socket = new QSslSocket(this);
    reconnectSocket();
}

void DrinkView::refresh() {
    QObjectList children = layout()->children();

    socket->write(STAT_MESSAGE);
    if (socket->waitForReadyRead(1000)) {
        while(!children.isEmpty()) {
            delete children.takeFirst();
        }
        parseStats();
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
        if (line.mid(0, 2) == "OK") {
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
        if (count == "0" || price.toInt() > 100) {
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

QByteArray DrinkView::waitForHello() {
    int bytes = -1;
    do {
        socket->waitForReadyRead(5000);
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
    if (socket->waitForConnected(2000)) {
        waitForHello();
    }
}
