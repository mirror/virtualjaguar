//
// filepicker.h - A ROM chooser
//

#include <QtGui>
#include "types.h"

// Forward declarations
class QListWidget;
class FileThread;
class FileListModel;
class QListView;

class FilePickerWindow: public QWidget
{
	// Once we have signals/slots, we need this...
	Q_OBJECT

	public:
		FilePickerWindow(QWidget * parent = 0);

	public slots:
		void AddFileToList(unsigned long index);
		void AddFileToList2(unsigned long index, QString, QImage *, unsigned long size);
		void UpdateSelection(const QModelIndex &, const QModelIndex &);
		void LoadButtonPressed(void);

	signals:
		void RequestLoad(QString);

	protected:
//		void PopulateList(void);

	private:
		QString currentFile;
		QListWidget * fileList2;
		FileThread * fileThread;
		FileListModel * model;
		QListView * fileList;
		QLabel * cartImage;
		QLabel * title;
		QLabel * data;
		QPushButton * insertCart;
};
