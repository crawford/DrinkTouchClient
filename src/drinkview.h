#ifndef DRINKVIEW_H
#define DRINKVIEW_H

#include <QWidget>
#include <QSslSocket>
#include <QMessageBox>
#include "itembutton.h"

class DrinkView : public QWidget {
    Q_OBJECT
public:
    explicit DrinkView(QString, int, QWidget *parent = 0);
    explicit DrinkView(QString, int, int, int, QList<int>, QWidget *parent = 0);
    int getCredits();
    bool isAuthed();

signals:
    void hasUsername(QString);
    void error(QString);
    void dropped();

public slots:
    void refresh();
    void authenticate(QString);
    void logout();

private:
    QSslSocket *socket;
    QString host;
    int port;
    int credits;
    QString username;
    int slotsWidth;
    int slotsHeight;
    QList<int> slotSizes;
    QMessageBox *msgbox;
    QList<ItemButton *> *buttons;

    void init(QString, int);
    void parseStats();
    bool reconnectSocket();

private slots:
    void handleClick(ItemButton *);
    void handleDropTimeout();
};

#endif // DRINKVIEW_H
