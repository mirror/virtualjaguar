//
// filepicker.cpp - A ROM chooser
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  02/01/2010  Created this file
//

// Note that we have to put in convenience functions to the model for adding data
// and calling reset() to tell the view(s) that the model has changed. So that much
// should be simple. According to the docs, we have to reimplement flags() in the
// QAbstractListModel subclass, but in the example given below they don't. Not sure
// if it's necessary or not.

#include "filelistmodel.h"


FileListModel::FileListModel(QObject * parent/*= 0*/): QAbstractListModel(parent)
{
}

int FileListModel::rowCount(const QModelIndex & parent/*= QModelIndex()*/) const
{
	return pixList.size();
}

QVariant FileListModel::data(const QModelIndex & index, int role) const
{
//	return QVariant();
	return pixList.at(index.row());
}

QVariant FileListModel::headerData(int section, Qt::Orientation orientation, int role/*= Qt::DisplayRole*/) const
{
	// Not sure that this is necessary for our purposes...
	// Especially since this model would never make use of this info...
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
		return QString("Column %1").arg(section);
	else
		return QString("Row %1").arg(section);
}

void FileListModel::AddData(QIcon pix)
{
	pixList.push_back(pix);
	reset();
}


#if 0

class StringListModel : public QAbstractListModel
{
	Q_OBJECT

	public:
		StringListModel(const QStringList &strings, QObject *parent = 0)
			: QAbstractListModel(parent), stringList(strings) {}

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation,
							int role = Qt::DisplayRole) const;

	private:
		QStringList stringList;
};

int StringListModel::rowCount(const QModelIndex &parent) const
{
	return stringList.count();
}

QVariant StringListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= stringList.size())
		return QVariant();

	if (role == Qt::DisplayRole)
		return stringList.at(index.row());
	else
		return QVariant();
}


QVariant StringListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
		return QString("Column %1").arg(section);
	else
		return QString("Row %1").arg(section);
}



 void ImageModel::setImage(const QImage &image)
 {
     modelImage = image;
     reset();
 }

The QAbstractItemModel::reset() call tells the view(s) that the model has changed.

#endif
