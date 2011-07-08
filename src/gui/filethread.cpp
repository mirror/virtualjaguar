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
// JLH  06/28/2011  Cleanup in the file parsing/fishing code, to make it easier
//                  to follow the flow of the logic
//

#include "filethread.h"

#include <QtGui>
#include "crc32.h"
#include "file.h"
#include "filedb.h"
#include "memory.h"
#include "settings.h"

#define VERBOSE_LOGGING

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

void FileThread::Go(bool allowUnknown/*= false*/)
{
	allowUnknownSoftware = allowUnknown;
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
The future is NOW! :-)
*/

//
// Here's the thread's actual execution path...
//
void FileThread::run(void)
{
	QDir romDir(vjs.ROMPath);
	QFileInfoList list = romDir.entryInfoList();

	for(int i=0; i<list.size(); i++)
	{
		if (abort)
#ifdef VERBOSE_LOGGING
{
printf("FileThread: Aborting!!!\n");
#endif
			return;
#ifdef VERBOSE_LOGGING
}
#endif

		HandleFile(list.at(i));
	}
}

//
// This handles file identification and ZIP extraction.
//
void FileThread::HandleFile(QFileInfo fileInfo)
{
	bool haveZIPFile = (fileInfo.suffix().compare("zip", Qt::CaseInsensitive) == 0
		? true : false);
	uint32_t fileSize = 0;
	uint8 * buffer = NULL;

	if (haveZIPFile)
	{
		// ZIP files are special: They contain more than just the software now... ;-)
		// So now we fish around inside them to pull out the stuff we want.
		// Probably also need more stringent error checking as well... :-O
		fileSize = GetFileFromZIP(fileInfo.filePath().toAscii(), FT_SOFTWARE, buffer);

		if (fileSize == 0)
			return;
	}
	else
	{
		QFile file(fileInfo.filePath());

		if (!file.open(QIODevice::ReadOnly))
			return;

		fileSize = fileInfo.size();

		if (fileSize == 0)
			return;

		buffer = new uint8[fileSize];
		file.read((char *)buffer, fileSize);
		file.close();
	}

	// Try to divine the file type by size & header
	int fileType = ParseFileType(buffer[0], buffer[1], fileSize);

	// Check for Alpine ROM w/Universal Header
	bool foundUniversalHeader = HasUniversalHeader(buffer, fileSize);
	uint32 crc;

//printf("FileThread: About to calc checksum on file with size %u... (buffer=%08X)\n", size, buffer);
	if (foundUniversalHeader)
		crc = crc32_calcCheckSum(buffer + 8192, fileSize - 8192);
	else
		crc = crc32_calcCheckSum(buffer, fileSize);

	uint32 index = FindCRCIndexInFileList(crc);

	if ((index != 0xFFFFFFFF) && (romList[index].flags & FF_BIOS))
		HandleBIOSFile(buffer, crc);

	delete[] buffer;

	// Here we filter out files *not* in the DB (if configured that way) and
	// BIOS files.
	if (index == 0xFFFFFFFF)
	{
		// If we allow unknown software, we pass the (-1) index on, otherwise...
		if (!allowUnknownSoftware)
			return;								// CRC wasn't found, so bail...
	}
	else if (romList[index].flags & FF_BIOS)
		return;

//Here's a little problem. When we create the image here and pass it off to FilePicker,
//we can clobber this image before we have a chance to copy it out in the FilePicker function
//because we can be back here before FilePicker can respond.
// So now we create the image on the heap, problem solved. :-)
	QImage * img = NULL;

	// See if we can fish out a label. :-)
	if (haveZIPFile)
	{
		uint32 size = GetFileFromZIP(fileInfo.filePath().toAscii(), FT_LABEL, buffer);
//printf("FT: Label size = %u bytes.\n", size);

		if (size > 0)
		{
			QImage label;
			bool successful = label.loadFromData(buffer, size);
			img = new QImage;
			*img = label.scaled(365, 168, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//printf("FT: Label %s: %ux%u.\n", (successful ? "succeeded" : "did not succeed"), img->width(), img->height());
			delete[] buffer;
		}
//printf("FileThread: Attempted to load image. Size: %u x %u...\n", img.width(), img.height());
	}

//	emit FoundAFile2(index, fileInfo.canonicalFilePath(), img, fileSize);
	emit FoundAFile3(index, fileInfo.canonicalFilePath(), img, fileSize, foundUniversalHeader, fileType, crc);
}

//
// Handle checking/copying BIOS files into Jaguar core memory
//
void FileThread::HandleBIOSFile(uint8 * buffer, uint32 crc)
{
/*
	{ 0x55A0669C, "[BIOS] Atari Jaguar Developer CD (World)", FF_BIOS },
	{ 0x687068D5, "[BIOS] Atari Jaguar CD (World)", FF_BIOS },
	{ 0x8D15DBC6, "[BIOS] Atari Jaguar Stubulator '94 (World)", FF_BIOS },
	{ 0xE60277BB, "[BIOS] Atari Jaguar Stubulator '93 (World)", FF_BIOS },
	{ 0xFB731AAA, "[BIOS] Atari Jaguar (World)", FF_BIOS },

uint8 jaguarBootROM[0x040000];					// 68K CPU BIOS ROM--uses only half of this!
uint8 jaguarCDBootROM[0x040000];				// 68K CPU CD BIOS ROM (256K)
uint8 jaguarDevBootROM1[0x040000];				// 68K CPU Stubulator 1 ROM--uses only half of this!
uint8 jaguarDevBootROM2[0x040000];				// 68K CPU Stubulator 2 ROM--uses only half of this!
uint8 jaguarDevCDBootROM[0x040000];				// 68K CPU Dev CD BIOS ROM (256K)

enum { BIOS_NORMAL=0x01, BIOS_CD=0x02, BIOS_STUB1=0x04, BIOS_STUB2=0x08, BIOS_DEV_CD=0x10 };
extern int biosAvailable;
*/
	if (crc == 0xFB731AAA && !(biosAvailable & BIOS_NORMAL))
	{
		memcpy(jaguarBootROM, buffer, 0x20000);
		biosAvailable |= BIOS_NORMAL;
	}
	else if (crc == 0x687068D5 && !(biosAvailable & BIOS_CD))
	{
		memcpy(jaguarCDBootROM, buffer, 0x40000);
		biosAvailable |= BIOS_CD;
	}
	else if (crc == 0x8D15DBC6 && !(biosAvailable & BIOS_STUB1))
	{
		memcpy(jaguarDevBootROM1, buffer, 0x20000);
		biosAvailable |= BIOS_STUB1;
	}
	else if (crc == 0xE60277BB && !(biosAvailable & BIOS_STUB2))
	{
		memcpy(jaguarDevBootROM2, buffer, 0x20000);
		biosAvailable |= BIOS_STUB2;
	}
	else if (crc == 0x55A0669C && !(biosAvailable & BIOS_DEV_CD))
	{
		memcpy(jaguarDevCDBootROM, buffer, 0x40000);
		biosAvailable |= BIOS_DEV_CD;
	}
}

//
// Find a CRC in the ROM list (simple brute force algorithm).
// If it's there, return the index, otherwise return $FFFFFFFF
//
uint32 FileThread::FindCRCIndexInFileList(uint32 crc)
{
	// Instead of a simple brute-force search, we should probably do a binary
	// partition search instead, since the CRCs are sorted numerically.
#warning "!!! Should do binary partition search here !!!"
	for(int i=0; romList[i].crc32!=0xFFFFFFFF; i++)
	{
		if (romList[i].crc32 == crc)
			return i;
	}

	return 0xFFFFFFFF;
}
