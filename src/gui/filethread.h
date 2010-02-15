//
// filethread.h: File discovery thread class definition
//

#ifndef __FILETHREAD_H__
#define __FILETHREAD_H__

#include <QtCore>
#include "types.h"

// Forward declarations
//class QListWidget;

class FileThread: public QThread
{
	Q_OBJECT

	public:
		FileThread(QObject * parent = 0);
		~FileThread();
//		void Go(QListWidget * lw);
		void Go(void);

	signals:
		void FoundAFile(unsigned long index);

	protected:
		void run(void);
		uint32 FindCRCIndexInFileList(uint32);

	private:
//		QListWidget * listWidget;
		QMutex mutex;
		QWaitCondition condition;
		bool abort;
};

enum FileFlags { FF_ROM=1, FF_ALPINE=2, FF_BIOS=4, FF_REQ_DSP=8, FF_REQ_BIOS=16, FF_NON_WORKING=32 };

struct RomIdentifier
{
	const uint32 crc32;
	const char name[128];
	const char file[128];
	const uint32 flags;
};

extern RomIdentifier romList[];

#endif	// __FILETHREAD_H__
