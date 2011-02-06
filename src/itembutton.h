#ifndef ITEMBUTTON_H_
#define ITEMBUTTON_H_

#include <QAbstractButton>
#include <QSpinBox>

class ItemButton : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(QString Price READ getPrice WRITE setPrice)
    Q_PROPERTY(QString Description READ getDescription WRITE setDescription)

public:
    ItemButton(QWidget *parent = 0);
    ItemButton(QString nTitle, QString nDescription, QString nPrice, QIcon nIcon, QWidget *parent = 0);
    void setDescription(QString nDescription);
    QString getDescription();
    void setPrice(QString nPrice);
    QString getPrice();
    bool isMarked();

public slots:
    void mark();
    void unmark();

private:
    void paintEvent(QPaintEvent *event);
    int getMinSize();

    QString description;
    QString price;
    bool marked;
};

#endif /*ITEMBUTTON_H_*/
