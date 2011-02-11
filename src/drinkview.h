#ifndef DRINKVIEW_H
#define DRINKVIEW_H

#include <QWidget>
#include <QSslSocket>

class DrinkView : public QWidget {
    Q_OBJECT
public:
    explicit DrinkView(QString, int, QWidget *parent = 0);

signals:

public slots:
    void refresh();

private:
    QSslSocket *socket;
    QString host;
    int port;

    void parseStats();
    QByteArray waitForHello();
    void reconnectSocket();

};

#endif // DRINKVIEW_H
