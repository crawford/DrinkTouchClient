#include "ibuttonhelper.h"
#include <QDebug>

IButtonHelper::IButtonHelper(QString filename, QObject *parent) : QThread(parent) {
    ibuttonFile = new QFile(filename, this);
    running = false;
}

void IButtonHelper::run() {
    ibuttonFile->open(QFile::ReadOnly);
    running = true;

    while (running) {
        QString id = ibuttonFile->readLine().trimmed();

        if (id.isEmpty()) {
            sleep(1);
            continue;
        }

        //Clear the value in the file
        ibuttonFile->close();
        ibuttonFile->open(QFile::WriteOnly);
        ibuttonFile->write("");
        ibuttonFile->close();
        ibuttonFile->open(QFile::ReadOnly);

        emit newIButton(id);
    }
}

void IButtonHelper::stop() {
    running = false;
}
