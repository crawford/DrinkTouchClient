#include "ibuttonhelper.h"
#include <QDebug>

#define UDPATE_INTERVAL 250

IButtonHelper::IButtonHelper(QString filename, QObject *parent) : QThread(parent) {
    ibuttonFile = new QFile(filename, this);
    running = false;
}

void IButtonHelper::run() {
    ibuttonFile->open(QFile::ReadOnly);
    running = true;

    while (running) {
        QString id;

        if (ibuttonFile->isOpen()) {
            id = ibuttonFile->readLine().trimmed();
        } else {
            ibuttonFile->open(QFile::ReadOnly);
        }

        if (id.isEmpty()) {
            usleep(UDPATE_INTERVAL);
            continue;
        }

        //Clear the value in the file
        ibuttonFile->close();
        ibuttonFile->open(QFile::WriteOnly);
        ibuttonFile->write("");
        ibuttonFile->close();
        ibuttonFile->open(QFile::ReadOnly);

        qDebug() << "Read iButton" << id;

        emit newIButton(id);
    }
}

void IButtonHelper::stop() {
    running = false;
}
