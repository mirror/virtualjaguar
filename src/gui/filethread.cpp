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
// JLH  03/02/2010  Added .ZIP file fishing
//

#include "filethread.h"

#include <QtGui>
#include "crc32.h"
#include "file.h"
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

		// ZIP files are special: They contain more than just the software now... ;-)
		// So now we fish around inside them to pull out the stuff we want.
		// Probably also need more stringent error checking as well... :-O
		if (fileInfo.suffix().compare("zip", Qt::CaseInsensitive) == 0)
		{
			uint8 * buffer = NULL;
			uint32 size = GetFileFromZIP(fileInfo.canonicalFilePath().toAscii(), FT_SOFTWARE, buffer);

			if (size > 0)
			{
				uint32_t fileSize = size;
//printf("FileThread: About to calc checksum on file with size %u... (buffer=%08X)\n", size, buffer);
				uint32 crc = crc32_calcCheckSum(buffer, size);
				uint32 index = FindCRCIndexInFileList(crc);
// These two are NOT interchangeable!
//				delete[] buffer;
				free(buffer);

// Mebbe we should pass a index AND a QImage here???
/*
Let's think about this... What *do* we need to send out?
we need the filename for sure. image file if it exists.
do we need the index? I think we're only using it to pull the label from the subdir...
we might need it if we want to pull ROM flags from the fileDB...
*/
				if (index != 0xFFFFFFFF && !(romList[index].flags & FF_BIOS))
				{
//Here's a little problem. When we create the image here and pass it off to FilePicker,
//we can clobber this image before we have a chance to copy it out in the FilePicker function
//because we can be back here before FilePicker can respond.
//So we need to fix this so that this does not happen. :-/
//And now it is. :-)
/*
So I guess we can create an image on the heap and pass *that* to FilePicker. But then, would
it be worthwhile to just pass the pointer into the FileListModel instead of a copy of an object?
Maybe. We'd do like so:
QImage * imageCopy = new QImage();
*/
					QImage * img = NULL;
					size = GetFileFromZIP(fileInfo.canonicalFilePath().toAscii(), FT_LABEL, buffer);
//printf("FT: Label size = %u bytes.\n", size);

					if (size > 0)
					{
//#warning "!!!"
//Not sure if this will work properly... Seems to.
						QImage label;
						bool success = label.loadFromData(buffer, size);
						img = new QImage();
						*img = label.scaled(373, 172, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//printf("FT: Label %s: %ux%u.\n", (success ? "succeeded" : "did not succeed"), img->width(), img->height());
// These two are NOT interchangeable!
//						delete[] buffer;
						free(buffer);
					}
//printf("FileThread: Attempted to load image. Size: %u x %u...\n", img.width(), img.height());

//					emit FoundAFile(index);
					emit FoundAFile2(index, fileInfo.canonicalFilePath(), img, fileSize);
				}
			}
		}
		else
		{
			QFile file(romDir.filePath(fileInfo.fileName()));

			if (file.open(QIODevice::ReadOnly))
			{
				uint8 * buffer = new uint8[fileInfo.size()];
				file.read((char *)buffer, fileInfo.size());
				file.close();
				uint32 crc = crc32_calcCheckSum(buffer, fileInfo.size());
				uint32 index = FindCRCIndexInFileList(crc);
				delete[] buffer;

// Mebbe we should pass a index AND a QImage here???
				if (index != 0xFFFFFFFF && !(romList[index].flags & FF_BIOS))
//					emit FoundAFile(index);
					emit FoundAFile2(index, fileInfo.canonicalFilePath(), 0, fileInfo.size());
			}
		}
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

