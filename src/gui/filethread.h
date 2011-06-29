//
// filethread.h: File discovery thread class definition
//

#ifndef __FILETHREAD_H__
#define __FILETHREAD_H__

#include <QtCore>
#include "types.h"

class FileThread: public QThread
{
	Q_OBJECT

	public:
		FileThread(QObject * parent = 0);
		~FileThread();
		void Go(void);

	signals:
		void FoundAFile(unsigned long index);
		void FoundAFile2(unsigned long index, QString filename, QImage * label, unsigned long);

	protected:
		void run(void);
		void HandleFile(QFileInfo);
		uint32 FindCRCIndexInFileList(uint32);

	private:
		QMutex mutex;
		QWaitCondition condition;
		bool abort;
};

#endif	// __FILETHREAD_H__
