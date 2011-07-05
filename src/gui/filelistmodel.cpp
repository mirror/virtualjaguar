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
//	return pixList.size();
//	return dbIndex.size();
	return list.size();
}

QVariant FileListModel::data(const QModelIndex & index, int role) const
{
//	return QVariant();
//	return pixList.at(index.row());
//	return (uint)dbIndex.at(index.row());
//It could be that this is fucking things up...
#if 0
	if (role == Qt::DecorationRole)
		return list.at(index.row()).label;
	else if (role == Qt::DisplayRole)
		return (uint)list.at(index.row()).dbIndex;
	else if (role == Qt::EditRole)
		return list.at(index.row()).filename;
	else if (role == Qt::WhatsThisRole)
		return (uint)list.at(index.row()).fileSize;
	else
		return QVariant();
#else
	if (role == FLM_LABEL)
		return list.at(index.row()).label;
	else if (role == FLM_INDEX)
		return (uint)list.at(index.row()).dbIndex;
	else if (role == FLM_FILENAME)
		return list.at(index.row()).filename;
	else if (role == FLM_FILESIZE)
		return (uint)list.at(index.row()).fileSize;
	else if (role == FLM_UNIVERSALHDR)
		return (uint)list.at(index.row()).hasUniversalHeader;
	else if (role == FLM_FILETYPE)
		return (uint)list.at(index.row()).fileType;
	else if (role == FLM_CRC)
		return (uint)list.at(index.row()).crc;

	return QVariant();
#endif
}

QVariant FileListModel::headerData(int/* section*/, Qt::Orientation/* orientation*/, int role/*= Qt::DisplayRole*/) const
{
#if 0
	// Not sure that this is necessary for our purposes...
	// Especially since this model would never make use of this info...
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
		return QString("Column %1").arg(section);
	else
		return QString("Row %1").arg(section);
#else
	// This seems more like what we want...
	if (role == Qt::SizeHintRole)
		return QSize(1, 1);

	return QVariant();
#endif
}

/*
void FileListModel::AddData(QIcon pix)
{
	pixList.push_back(pix);
	reset();
}

void FileListModel::AddData(unsigned long index)
{
	dbIndex.push_back(index);
	reset();
}
*/

void FileListModel::AddData(unsigned long index, QString str, QImage img, unsigned long size)
{
	// Assuming that both QString and QImage have copy constructors, this should work.
	FileListData data;

	data.dbIndex = index;
	data.fileSize = size;
	data.filename = str;
	data.label = img;

	list.push_back(data);
	reset();
}

void FileListModel::AddData(unsigned long index, QString str, QImage img, unsigned long size, bool univHdr, uint32_t type, uint32_t fileCrc)
{
	// Assuming that both QString and QImage have copy constructors, this should work.
	FileListData data;

	data.dbIndex = index;
	data.fileSize = size;
	data.filename = str;
	data.label = img;
	data.hasUniversalHeader = univHdr;
	data.fileType = type;
	data.crc = fileCrc;

	list.push_back(data);
//This is probably not the best way to do this, it prevents the user from using the
//list while it populates.
#warning "!!! AddData calls reset() for every addition, this is bad !!!"
	reset();
}

void FileListModel::ClearData(void)
{
	if (list.size() == 0)
		return;

	beginResetModel();
	list.clear();
	endResetModel();
}

//FileListData FileListModel::GetData(const QModelIndex & index) const
//{
//	return list.at(index.row());
//}

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
