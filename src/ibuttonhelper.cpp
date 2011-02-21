#include "ibuttonhelper.h"
#include <QDebug>
#include <QApplication>
#include <sys/inotify.h>
#include <errno.h>

IButtonHelper::IButtonHelper(QString filename, QObject *parent) : QThread(parent) {
    ibuttonFile = new QFile(filename, this);
    running = false;
    curId = "";
}

void IButtonHelper::run() {
    if (!ibuttonFile->exists()) {
        ibuttonFile->open(QFile::WriteOnly);
        ibuttonFile->write("");
        ibuttonFile->close();
    }

    //Clear out any old ibuttons
    ibuttonFile->open(QFile::WriteOnly);
    ibuttonFile->write("");
    ibuttonFile->close();

    ibuttonFile->open(QFile::ReadOnly);
    running = true;

    /*int fd = inotify_init();
    if (fd < 0) {
        qDebug() << "inotify_init error";
    }

    int wd = inotify_add_watch(fd, ibuttonFile->fileName().toAscii().data(), IN_MODIFY);
    if (wd < 0) {
        qDebug() << "inotify_add_watch error";
    }*/

    while (running) {
        ibuttonFile->seek(0);
        QString id = ibuttonFile->readLine().trimmed();

        if (!id.isEmpty()) {
            //Clear the value in the file
            ibuttonFile->close();
            ibuttonFile->open(QFile::WriteOnly);
            ibuttonFile->write("");
            ibuttonFile->close();
            ibuttonFile->open(QFile::ReadOnly);

            if (curId.isEmpty()) {
                qDebug() << "Read iButton" << id;
                curId = id;

                emit newIButton(id);
            }
        }

        sleep(1);

        /*
        qDebug() << "Looping";

        struct timeval time;
        fd_set rfds;
        int ret;

        // timeout after five seconds
        time.tv_sec = 5;
        time.tv_usec = 0;

        // zero-out the fd_set
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        ret = select(fd + 1, &rfds, NULL, NULL, &time);

        if (FD_ISSET (fd, &rfds)) {
            //Clear the pending inotify events
            printf("READING %d (error %d)\n", read(fd, NULL, 100), errno);

            QString id = ibuttonFile->readLine().trimmed();

            if (!id.isEmpty()) {
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
        */
    }

    //close(fd);
    ibuttonFile->close();
}

void IButtonHelper::stop() {
    running = false;
}

void IButtonHelper::clearIButton() {
    curId = "";
    qDebug() << "Clearing";
}
