#ifndef IBUTTONHELPER_H
#define IBUTTONHELPER_H

#include <QThread>
#include <QFile>
#include <QSslSocket>

class IButtonHelper : public QThread
{
    Q_OBJECT
public:
    explicit IButtonHelper(QString, QObject *parent = 0);
    virtual void run();

signals:
    void newIButton(QString);

public slots:
    void stop();
    void clearIButton();

private:
    QFile *ibuttonFile;
    bool running;
    QString curId;
};

#endif // IBUTTONHELPER_H
