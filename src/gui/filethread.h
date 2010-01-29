//
// filethread.h: File discovery thread class definition
//

#ifndef	// __FILETHREAD_H__
#define	// __FILETHREAD_H__

#include <QtCore>

// Forward declarations
class QListWidget;

class FileThread: public QThread
{
	public:
		FileThread(QObject * parent = 0);
		~FileThread();

	private:
		QListWidget * listWidget;
		QMutex mutex;
		QWaitCondition condition;
		bool abort;
};

#endif	// __FILETHREAD_H__
