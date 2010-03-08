//
// imagedelegate.cpp - Qt Model/View rendering class
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  02/04/2010  Created this file
//

// This class takes care of rendering items in our custom model in the ListView
// class utilized in FilePicker.

#include "imagedelegate.h"

#include "filedb.h"


//ImageDelegate::ImageDelegate(QObject * parent): QAbstractItemDelegate(parent)//, pixelSize(12)
//{
//}

/*
Each item is rendered by the delegate's paint() function. The view calls this function with a ready-to-use QPainter object, style information that the delegate should use to correctly draw the item, and an index to the item in the model:
*/

void ImageDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	if (option.state & QStyle::State_Selected)
		painter->fillRect(option.rect, option.palette.highlight());

/*
The first task the delegate has to perform is to draw the item's background correctly. Usually, selected items appear differently to non-selected items, so we begin by testing the state passed in the style option and filling the background if necessary.

The radius of each circle is calculated in the following lines of code:
*/

#if 0
	int size = qMin(option.rect.width(), option.rect.height());
	int brightness = index.model()->data(index, Qt::DisplayRole).toInt();
	double radius = (size/2.0) - (brightness/255.0 * size/2.0);
	if (radius == 0.0)
		return;
#endif

/*
First, the largest possible radius of the circle is determined by taking the smallest dimension of the style option's rect attribute. Using the model index supplied, we obtain a value for the brightness of the relevant pixel in the image. The radius of the circle is calculated by scaling the brightness to fit within the item and subtracting it from the largest possible radius.
*/

	painter->save();
#if 0
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(Qt::NoPen);

/*
We save the painter's state, turn on antialiasing (to obtain smoother curves), and turn off the pen.
*/

	if (option.state & QStyle::State_Selected)
		painter->setBrush(option.palette.highlightedText());
	else
		painter->setBrush(QBrush(Qt::black));

/*
The foreground of the item (the circle representing a pixel) must be rendered using an appropriate brush. For unselected items, we will use a solid black brush; selected items are drawn using a predefined brush from the style option's palette.
*/

	painter->drawEllipse(QRectF(option.rect.x() + option.rect.width()/2 - radius,
		option.rect.y() + option.rect.height()/2 - radius, 2*radius, 2*radius));
#else
//	painter->drawPixmap(option.rect.x()+8, option.rect.y()+8, 200, 94, QPixmap(":/res/labels/rayman.jpg"));
//	painter->drawPixmap(option.rect.x()+13, option.rect.y()+51, 433/2, 203/2, QPixmap(":/res/labels/rayman.jpg"));
//	painter->drawPixmap(option.rect.x(), option.rect.y(), 488/2, 395/2, QPixmap(":/res/cart-blank.png"));
	painter->drawPixmap(option.rect.x(), option.rect.y(), 488/4, 395/4, QPixmap(":/res/cart-blank.png"));
//	unsigned long i = index.model()->data(index, Qt::DisplayRole).toUInt();
	unsigned long i = index.model()->data(index, Qt::DisplayRole).toUInt();
	QString filename = index.model()->data(index, Qt::EditRole).toString();
	QImage label = index.model()->data(index, Qt::DecorationRole).value<QImage>();

#if 0
	if (role == Qt::DecorationRole)
		return list.at(index.row()).label;
	else if (role == Qt::DisplayRole)
		return (uint)list.at(index.row()).dbIndex;
	else if (role == Qt::EditRole)
		return list.at(index.row()).filename;
#endif

//	if (romList[i].file[0] == 0)
	if (label.isNull())
	{
//	painter->drawPixmap(option.rect.x()+14, option.rect.y()+50, 433/2, 203/2, QPixmap(":/res/label-blank.png"));
		painter->drawPixmap(option.rect.x()+7, option.rect.y()+25, 433/4, 203/4, QPixmap(":/res/label-blank.png"));
//Need to query the model for the data we're supposed to draw here...
//	painter->drawText(17, 73, QString(romList[i].name));
//	painter->setPen(Qt::white);
		painter->setPen(QColor(255, 128, 0, 255));
//	painter->drawText(QRect(option.rect.x()+20, option.rect.y()+73, 196, 70), Qt::TextWordWrap | Qt::AlignHCenter, QString(romList[i].name));
		painter->drawText(QRect(option.rect.x()+10, option.rect.y()+36, 196/2, 70/2), Qt::TextWordWrap | Qt::AlignHCenter, QString(romList[i].name));
	}
	else
	{
#if 0
		QString filename(romList[i].file);
		filename.prepend("./label/");
		QImage img(filename);
		painter->drawImage(QRect(option.rect.x()+7, option.rect.y()+25, 433/4, 203/4), img);
#else
		painter->drawImage(QRect(option.rect.x()+7, option.rect.y()+25, 433/4, 203/4), label);
#endif
	}
//26x100
#endif
	painter->restore();
}

/*
Finally, we paint the circle within the rectangle specified by the style option and we call restore() on the painter.

The paint() function does not have to be particularly complicated; it is only necessary to ensure that the state of the painter when the function returns is the same as it was when it was called. This usually means that any transformations applied to the painter must be preceded by a call to QPainter::save() and followed by a call to QPainter::restore().

The delegate's sizeHint() function returns a size for the item based on the predefined pixel size, initially set up in the constructor:
*/

QSize ImageDelegate::sizeHint(const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const
{
	// 488x395 --> blank cart (full size)
	// 400x188 --> label (full size) 433x203 <-- (actually, it's this)

	// 200x94 is shrunk dimension...
//	return QSize(100, 47);
//	return QSize(216, 110);
//	return QSize(488/2, 395/2);
	return QSize(488/4, 395/4);
}

/*
The delegate's size is updated whenever the pixel size is changed. We provide a custom slot to do this:
*/

//void ImageDelegate::setPixelSize(int size)
//{
//	pixelSize = size;
//}
