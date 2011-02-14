#ifndef DRINKVIEW_H
#define DRINKVIEW_H

#include <QWidget>
#include <QSslSocket>

class DrinkView : public QWidget {
    Q_OBJECT
public:
    explicit DrinkView(QString, int, QWidget *parent = 0);
    int getCredits();
    bool isAuthed();

signals:
    void hasUsername(QString);
    void error(QString);

public slots:
    void refresh();
    void authenticate(QString);

private:
    QSslSocket *socket;
    QString host;
    int port;
    int credits;
    QString username;

    void parseStats();
    QByteArray waitForResponse();
    void reconnectSocket();
};

#endif // DRINKVIEW_H
