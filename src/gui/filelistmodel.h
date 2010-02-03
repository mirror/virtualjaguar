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

class FileListModel: public QAbstractListModel
{
	public:
		FileListModel(QObject * parent = 0);

		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		QVariant data(const QModelIndex & index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

		void AddData(QIcon pix);

	private:
		std::vector<QIcon> pixList;
};

#endif	// __FILELISTMODEL_H__
