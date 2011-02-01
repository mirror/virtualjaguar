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
	unsigned long fileSize;
	QString filename;
	QImage label;
};

//hm.
#define FLM_INDEX		(Qt::UserRole + 1)
#define FLM_FILESIZE	(Qt::UserRole + 2)
#define FLM_FILENAME	(Qt::UserRole + 3)
#define FLM_LABEL		(Qt::UserRole + 4)

class FileListModel: public QAbstractListModel
{
	public:
		FileListModel(QObject * parent = 0);

		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		QVariant data(const QModelIndex & index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

//		void AddData(QIcon pix);
//		void AddData(unsigned long);
		void AddData(unsigned long, QString, QImage, unsigned long);

//		FileListData GetData(const QModelIndex & index) const;

	private:
//		std::vector<QIcon> pixList;
//		std::vector<unsigned long> dbIndex;
		std::vector<FileListData> list;
//		std::vector<unsigned long> size;
};

#endif	// __FILELISTMODEL_H__
