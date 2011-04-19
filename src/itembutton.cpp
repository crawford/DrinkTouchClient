#include "itembutton.h"
#include <QStylePainter>
#include <QStyleOptionButton>

ItemButton::ItemButton(QWidget *parent) : QAbstractButton(parent) {
	description = "";
	price = "";
	slot = 0;

	setContentsMargins(9, 9, 9, 9);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);


	setMinimumHeight(getMinSize());

	connect(this, SIGNAL(clicked()), this, SLOT(handleClick()));
}

ItemButton::ItemButton(QString nTitle, QString nDescription, QString nPrice, QIcon nIcon, int slot, QWidget *parent) : QAbstractButton(parent) {
	setText(nTitle);
	description = nDescription;
	price = nPrice;
	setIcon(nIcon);
	this->slot = slot;

	setContentsMargins(13, 13, 13, 13);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setMinimumHeight(getMinSize());

	connect(this, SIGNAL(clicked()), this, SLOT(handleClick()));
}

int ItemButton::getMinSize() {
	//return fontInfo().pixelSize()*4;
	return 48;//iconSize().height();
}

void ItemButton::setDescription(QString nDescription) {
	description = nDescription;
	update();
	updateGeometry();
}

QString ItemButton::getDescription() {
	return description;
}

void ItemButton::setPrice(QString nPrice) {
	price = nPrice;
	update();
	updateGeometry();
}

QString ItemButton::getPrice() {
	return price;
}

void ItemButton::paintEvent(QPaintEvent *event) {
	QStylePainter p(this);
	QStyleOptionButton option;
	QRect margins = contentsRect();
	QFontMetrics fontMetrics(p.font());
	int textHeight = 0;
	int textWidth = 0;
	int priceWidth = fontMetrics.boundingRect(price).width();


	option.initFrom(this);

	if (isDown())
		option.state |= QStyle::State_Sunken;
	else
		option.state |= QStyle::State_Raised;

	if (isChecked())
		option.state |= QStyle::State_On;

	//Draw button
	p.drawControl(QStyle::CE_PushButtonBevel, option);

	int iconWidth = iconSize().height() > iconSize().width() ? iconSize().height() : iconSize().width();
	textWidth = fontMetrics.boundingRect(description).width();
	if (margins.right() - iconWidth < textWidth) {
		iconWidth = margins.right() - textWidth;
	}

	int iconHeight = margins.bottom();
	QSize size = icon().availableSizes().first();
	if (iconWidth < iconHeight) {
		size.scale(iconWidth, iconWidth, Qt::KeepAspectRatio);
	} else {
		size.scale(iconHeight, iconHeight, Qt::KeepAspectRatio);
	}
	setIconSize(size);

	//Draw icon
	if(isEnabled())
		p.drawPixmap(margins.left() + (iconWidth - iconSize().width())/2, (margins.height() - iconSize().height())/2 + margins.top(), icon().pixmap(iconSize(), QIcon::Normal));
	else
		p.drawPixmap(margins.left() + (iconWidth - iconSize().width())/2, (margins.height() - iconSize().height())/2 + margins.top(), icon().pixmap(iconSize(), QIcon::Disabled));

	//Draw title text
	textHeight = fontMetrics.boundingRect(text()).height();
	textWidth = fontMetrics.boundingRect(text()).width();

	if (textWidth > margins.width() - iconWidth - margins.left()) {
		QFont font = p.font();
		font.setPixelSize((double)font.pixelSize() * (margins.width() - iconWidth - margins.left()) / textWidth);
		p.setFont(font);
	}

	p.drawText(QRect(margins.left()*2 + iconWidth, margins.top(), margins.width() - iconWidth, textHeight), Qt::AlignLeft|Qt::TextSingleLine, text());

	p.setFont(this->font());

	//Draw price text
	p.drawText(QRect(margins.right() - priceWidth, margins.top(), priceWidth, margins.height()), Qt::AlignRight|Qt::TextSingleLine|Qt::AlignVCenter, price);

	//Resize font
	QFont font = p.font();
	font.setPixelSize((int)(font.pixelSize() * 0.75));
	//font.setPointSizeF((font.pointSizeF() * 0.75));
	p.setFont(font);

	//Draw description text
	fontMetrics = QFontMetrics(p.font());
	textHeight = fontMetrics.boundingRect(description).height();
	textWidth = fontMetrics.boundingRect(description).width();
	p.drawText(QRect(margins.left()*2 + iconWidth, margins.bottom() - textHeight, margins.width() - iconWidth, textHeight), Qt::AlignLeft|Qt::TextSingleLine, description);
}

void ItemButton::handleClick() {
	emit clicked(this);
}

int ItemButton::getSlot() {
	return slot;
}

