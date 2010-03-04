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
		void AddFileToList2(unsigned long index, QString, QImage *);

	protected:
//		void PopulateList(void);

	private:
		QListWidget * fileList2;
		FileThread * fileThread;
//		QAbstractItemModel * model;
		FileListModel * model;
		QListView * fileList;
};
