//
// filethread.h: File discovery thread class definition
//

#ifndef __FILETHREAD_H__
#define __FILETHREAD_H__

#include <QtCore>
#include "types.h"

// Forward declarations
class QListWidget;

class FileThread: public QThread
{
	Q_OBJECT

	public:
		FileThread(QObject * parent = 0);
		~FileThread();
		void Go(QListWidget * lw);

	signals:
		void FoundAFile(unsigned long index);

	protected:
		void run(void);
		uint32 FindCRCIndexInFileList(uint32);

	private:
		QListWidget * listWidget;
		QMutex mutex;
		QWaitCondition condition;
		bool abort;
};

#endif	// __FILETHREAD_H__
