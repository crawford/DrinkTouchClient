#ifndef DRINKVIEW_H
#define DRINKVIEW_H

#include <QWidget>
#include <QSslSocket>
#include <QMessageBox>
#include "itembutton.h"
#include "drinkviewhelper.h"

class DrinkViewHelper;

class DrinkView : public QWidget {
    Q_OBJECT

friend class DrinkViewHelper;

public:
    explicit DrinkView(QString, int, QWidget *parent = 0);
    explicit DrinkView(QString, int, int, int, QList<int>, QWidget *parent = 0);
    int getCredits();
    bool isAuthed();
	void run();
	void reqRefresh();

signals:
    void hasUsername(QString);
    void error(QString);
    void dropped();
	void initAuth(QString);
	void initRefresh();
	void initDropItem(int);

public slots:
    void refreshDisplay();
    void authenticate(QString);
    void logout();
	void handleDropReturned(bool, QString);

private:
    QString host;
    int port;
    int credits;
    QString username;
    int slotsWidth;
    int slotsHeight;
    QList<int> slotSizes;
    QMessageBox *msgbox;
    QList<ItemButton *> *buttons;
	QThread *thread;
	DrinkViewHelper *helper;

    void init(QString, int);

private slots:
    void handleClick(ItemButton *);
    void parseStats();
};

#endif // DRINKVIEW_H

