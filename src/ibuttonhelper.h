#ifndef IBUTTONHELPER_H
#define IBUTTONHELPER_H

#include <QThread>
#include <QFile>

class IButtonHelper : public QThread
{
    Q_OBJECT
public:
    explicit IButtonHelper(QString, QObject *parent = 0);

signals:
    void newIButton(QString);

public slots:
    void stop();

private:
    QFile *ibuttonFile;
    bool running;

    virtual void run();

};

#endif // IBUTTONHELPER_H
