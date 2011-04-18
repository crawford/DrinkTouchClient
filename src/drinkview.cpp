#include "drinkview.h"
#include <QGridLayout>
#include <QTimer>
#include <math.h>
#include <limits>

DrinkView::DrinkView(QString host, int port, QWidget *parent) : QWidget(parent) {
	init(host, port);
}

DrinkView::DrinkView(QString host, int port, int height, int width, QList<int> sizes, QWidget *parent) : QWidget(parent) {
	init(host, port);

	slotsHeight = height;
	slotsWidth = width;
	slotSizes = sizes;
}

void DrinkView::init(QString host, int port) {
	setLayout(new QGridLayout(this));
	this->host = host;
	this->port = port;

	msgbox = NULL;
	buttons = new QList<ItemButton *>();

	credits = 0;
	username = "";
	slotsHeight = 0;
	slotsWidth = 0;

	helper = new DrinkViewHelper(this);
	helper->start();

	connect(this, SIGNAL(initAuth(QString)), helper, SLOT(authenticate(QString)));
	connect(this, SIGNAL(initRefresh()), helper, SLOT(refresh()));
	connect(this, SIGNAL(initDropItem(int)), helper, SLOT(dropItem(int)));
	connect(helper, SIGNAL(hasRefreshed()), this, SLOT(refreshDisplay()));
	connect(helper, SIGNAL(dropReturned(bool, QString)), this, SLOT(handleDropReturned(bool, QString)));
}

void DrinkView::reqRefresh() {
	emit initRefresh();
}

void DrinkView::authenticate(QString id) {
	emit initAuth(id);
}

void DrinkView::logout() {
	helper->disconnect();
	username = "";
}

void DrinkView::refreshDisplay() {
	if (msgbox) {
		msgbox->close();
	}

	while (!buttons->isEmpty()) {
		delete buttons->takeFirst();
	}

	parseStats();
}

void DrinkView::parseStats() {
	QStringList lines;
	QString item;
	QString price;
	QString count;
	int slot = 1;

	lines = helper->buffer.split('\n');
	foreach (QString line, lines) {
		if (line.mid(0, 2) == MSG_OK) {
			break;
		}
		line.remove(0, line.indexOf('"') + 1);
		item = line.mid(0, line.indexOf('"'));
		line.remove(0, line.indexOf('"') + 2);
		price = line.mid(0, line.indexOf(' '));
		line.remove(0, line.indexOf(' ') + 1);
		count = line.mid(0, line.indexOf(' '));

		QPixmap icon("config/" + item.toLower().replace("'", "").replace(" ", "_") + ".png");
		if (icon.isNull()) {
			icon = QPixmap("config/default.png");
			if (icon.isNull()) {
				icon = QPixmap(QSize(50, 50));
				icon.fill(Qt::transparent);
			}
		}

		ItemButton *button = new ItemButton(item, count + " Remaining", price + " credits", QIcon(icon), slot, this);

		QFont font = button->font();
		font.setPixelSize(FONT_SIZE);
		button->setFont(font);
		if (count == "0" || price.toInt() > credits) {
			button->setEnabled(false);
		}

		connect(button, SIGNAL(clicked(ItemButton*)), this, SLOT(handleClick(ItemButton*)));

		buttons->append(button);

		slot++;
	}

	if (slotsWidth == 0) {
		int numCols = ceil(buttons->size() / PERFERRED_ROWS);

		for (int i = 0; i < buttons->size(); i++) {
			((QGridLayout *)layout())->addWidget(buttons->at(i), i / numCols, i % numCols);
		}
	} else {
		int row = 0;
		int col = 0;

		for (int i = 0; i < buttons->size(); i++) {
			((QGridLayout *)layout())->addWidget(buttons->at(i), row, col, 1, slotSizes.at(i));
			col += slotSizes.at(i);
			if (col >= slotsWidth) {
				row++;
				col = 0;
			}
		}
	}
}

int DrinkView::getCredits() {
	return credits;
}

bool DrinkView::isAuthed() {
	return !username.isEmpty();
}

void DrinkView::handleClick(ItemButton *button) {
	emit initDropItem(button->getSlot());

	if (msgbox) {
		delete msgbox;
	}
	msgbox = new QMessageBox(QMessageBox::Information, "Dropping", "Dropping your item!", QMessageBox::NoButton, this);
	msgbox->setStandardButtons(0);
	msgbox->show();
}

void DrinkView::handleDropReturned(bool success, QString error) {
	msgbox->close();
	delete msgbox;
	msgbox = NULL;

	if (success) {
		emit dropped();
	} else {
		msgbox = new QMessageBox(QMessageBox::Critical, "Drop Error", QString("The item could not be dropped (%1).").arg(error));
		msgbox->show();
	}
}

