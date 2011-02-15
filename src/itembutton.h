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
    ItemButton(QString nTitle, QString nDescription, QString nPrice, QIcon nIcon, int slot, QWidget *parent = 0);
    void setDescription(QString nDescription);
    QString getDescription();
    void setPrice(QString nPrice);
    QString getPrice();
    int getSlot();

signals:
    void clicked(ItemButton *);

private:
    void paintEvent(QPaintEvent *event);
    int getMinSize();
    int slot;

    QString description;
    QString price;

private slots:
    void handleClick();
};

#endif /*ITEMBUTTON_H_*/
