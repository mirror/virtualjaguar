//
// filethread.cpp - File discovery thread
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/28/2010  Created this file
// JLH  02/16/2010  Moved RomIdentifier stuff to its own file
//

#include "filethread.h"

#include <QtGui>
#include "crc32.h"
#include "filedb.h"
#include "settings.h"

FileThread::FileThread(QObject * parent/*= 0*/): QThread(parent), abort(false)
{
}

FileThread::~FileThread()
{
	mutex.lock();
	abort = true;
	condition.wakeOne();
	mutex.unlock();

	wait();
}

void FileThread::Go(void)
{
	QMutexLocker locker(&mutex);
	start();
}

/*
Our strategy here is like so:
Look at the files in the directory pointed to by ROMPath.
For each file in the directory, take the CRC32 of it and compare it to the CRC
in the romList[]. If there's a match, put it in a list and note it's index value
in romList for future reference.

When constructing the list, use the index to pull up an image of the cart and
put that in the list. User picks from a graphical image of the cart.

Ideally, the label will go into the archive along with the ROM image, but that's
for the future...
Maybe box art, screenshots will go as well...
*/

//
// Here's the thread's actual execution path...
//
void FileThread::run(void)
{
	QDir romDir(vjs.ROMPath);
//	QDir romDir("../virtualjaguar/roms/rarities/");
	QFileInfoList list = romDir.entryInfoList();

/*
Another thing we'll probably have to do here is check for compressed files and
decompress/fish around in them to find what we need. :-P
*/

	for(int i=0; i<list.size(); i++)
	{
		if (abort)
#if 1
{
printf("FileThread: Aborting!!!\n");
#endif
			return;
#if 1
}
#endif

		QFileInfo fileInfo = list.at(i);
		QFile file(romDir.filePath(fileInfo.fileName()));
		uint8 * buffer = new uint8[fileInfo.size()];

		if (file.open(QIODevice::ReadOnly))
		{
			file.read((char *)buffer, fileInfo.size());
			uint32 crc = crc32_calcCheckSum(buffer, fileInfo.size());
			file.close();

			uint32 index = FindCRCIndexInFileList(crc);

// Mebbe we should pass a index AND a QImage here???
			if (index != 0xFFFFFFFF && !(romList[index].flags & FF_BIOS))
				emit FoundAFile(index);
		}

		delete[] buffer;
	}
}

//
// Find a CRC in the ROM list (simple brute force algorithm).
// If it's there, return the index, otherwise return $FFFFFFFF
//
uint32 FileThread::FindCRCIndexInFileList(uint32 crc)
{
	for(int i=0; romList[i].crc32!=0xFFFFFFFF; i++)
	{
		if (romList[i].crc32 == crc)
			return i;
	}

	return 0xFFFFFFFF;
}

