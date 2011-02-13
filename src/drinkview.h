#ifndef DRINKVIEW_H
#define DRINKVIEW_H

#include <QWidget>
#include <QSslSocket>

class DrinkView : public QWidget {
    Q_OBJECT
public:
    explicit DrinkView(QString, int, QWidget *parent = 0);
    int getCredits();

signals:

public slots:
    void refresh();

private:
    QSslSocket *socket;
    QString host;
    int port;
    int credits;

    void parseStats();
    QByteArray waitForResponse();
    void reconnectSocket();

};

#endif // DRINKVIEW_H
