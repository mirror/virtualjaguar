//
// filelistmodel.h: Class definition
//
// by James L. Hammons
// (C) 2010 Underground Software
//

#ifndef __FILELISTMODEL_H__
#define __FILELISTMODEL_H__

#include <QtGui>
#include <vector>

struct FileListData
{
//	FileListData(unsigned long ul=0, QString str="", QImage img=QImage()): dbIndex(ul), filename(str), label(img) {}
//	FileListData(unsigned long ul=0, QString str, QImage img): dbIndex(ul), filename(str), label(img) {}

	unsigned long dbIndex;
	QString filename;
	QImage label;
};

class FileListModel: public QAbstractListModel
{
	public:
		FileListModel(QObject * parent = 0);

		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		QVariant data(const QModelIndex & index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

		void AddData(QIcon pix);
		void AddData(unsigned long);
		void AddData(unsigned long, QString, QImage);

//		FileListData GetData(const QModelIndex & index) const;

	private:
		std::vector<QIcon> pixList;
		std::vector<unsigned long> dbIndex;
		std::vector<FileListData> list;
};

#endif	// __FILELISTMODEL_H__
