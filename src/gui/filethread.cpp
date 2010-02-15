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
//

#include "filethread.h"

#include <QtGui>
#include "crc32.h"
#include "settings.h"
//#include "types.h"

#if 0
struct RomIdentifier
{
	const uint32 crc32;
	const char name[128];
	const char file[128];
	const uint32 flags;
};
#endif

// Should have another flag for whether or not it requires DSP, BIOS,
// whether it's a .rom, it's a BIOS, etc...

RomIdentifier romList[] = {
	{ 0x0509C85E, "Raiden (World)", "raiden.jpg", 0 },
	{ 0x08F15576, "Iron Soldier (World) (v1.04)", "iron-soldier.jpg", 0 },
	{ 0x0957A072, "Kasumi Ninja (World)", "kasumi-ninja.jpg", 0 },
	{ 0x0AC83D77, "NBA Jam T.E. (World)", "nba-jam.jpg", 0 },
	{ 0x0EC5369D, "Evolution - Dino Dudes (World)", "dino-dudes.jpg", 0 },
	{ 0x0F6A1C2C, "Ultra Vortek (World)", "ultra-vortek.jpg", 0 },
	{ 0x14915F20, "White Men Can't Jump (World)", "white-men-can't-jump.jpg", 0 },
	{ 0x1660F070, "Power Drive Rally (World)", "power-drive-rally.jpg", 0 },
	{ 0x1E451446, "Trevor McFur in the Crescent Galaxy (World)", "trevor-mcfur.jpg", 0 },
	{ 0x27594C6A, "Defender 2000 (World)", "defender-2000.jpg", 0 },
	{ 0x2E17D5DA, "Bubsy in Fractured Furry Tales (World)", "bubsy.jpg", 0 },
	{ 0x348E6449, "Double Dragon V - The Shadow Falls (World)", "double-dragon.jpg", 0 },
	{ 0x3615AF6A, "Fever Pitch Soccer (World) (En,Fr,De,Es,It)", "fever-pitch-soccer.jpg", 0 },
	{ 0x38A130ED, "Troy Aikman NFL Football (World)", "troy-aikman-football.jpg", 0 },
	{ 0x3C044941, "Skyhammer (World)", "skyhammer.jpg", 0 },
	{ 0x42A13EC5, "Soccer Kid (World)", "soccer-kid.jpg", 0 },
	{ 0x47EBC158, "Theme Park (World)", "theme-park.jpg", 0 },
	{ 0x4899628F, "Hover Strike (World)", "hover-strike.jpg", 0 },
	{ 0x53DF6440, "Space War 2000 (World)", "", 0 },
	{ 0x55A0669C, "[BIOS] Atari Jaguar Developer CD (World)", "", 0 },
	{ 0x58272540, "Syndicate (World)", "syndicate.jpg", 0 },
	{ 0x5A101212, "Sensible Soccer - International Edition (World)", "sensible-soccer.jpg", 0 },
	{ 0x5B6BB205, "Ruiner Pinball (World)", "ruiner-pinball.jpg", 0 },
	{ 0x5CFF14AB, "Pinball Fantasies (World)", "pinball-fantasies.jpg", 0 },
	{ 0x5E2CDBC0, "Doom (World)", "doom.jpg", 0 },
	{ 0x61C7EEC0, "Zero 5 (World)", "zero-5.jpg", 0 },
	{ 0x67F9AB3A, "Battle Sphere Gold (World)", "", 0 },
	{ 0x687068D5, "[BIOS] Atari Jaguar CD (World)", "", 0 },
	{ 0x6B2B95AD, "Tempest 2000 (World)", "tempest-2000.jpg", 0 },
	{ 0x6EB774EB, "Worms (World)", "worms.jpg", 0 },
	{ 0x6F8B2547, "Super Burnout (World)", "super-burnout.jpg", 0 },
	{ 0x817A2273, "Pitfall - The Mayan Adventure (World)", "pitfall.jpg", 0 },
	{ 0x8975F48B, "Zool 2 (World)", "zool-2.jpg", 0 },
	{ 0x8D15DBC6, "[BIOS] Atari Jaguar Stubulator '94 (World)", "", 0 },
	{ 0x8FEA5AB0, "Dragon - The Bruce Lee Story (World)", "dragon.jpg", 0 },
	{ 0x97EB4651, "I-War (World)", "i-war.jpg", 0 },
	{ 0xA27823D8, "Ultra Vortek (World) (v0.94) (Beta)", "", 0 },
	{ 0xA56D0798, "Protector - Special Edition (World)", "protector-se.jpg", 0 },
	{ 0xA9F8A00E, "Rayman (World)", "rayman.jpg", 0 },
	{ 0xB14C4753, "Fight for Life (World)", "fight-for-life.jpg", 0 },
	{ 0xBCB1A4BF, "Brutal Sports Football (World)", "brutal-sports-football.jpg", 0 },
	{ 0xBDA405C6, "Cannon Fodder (World)", "cannon-fodder.jpg", 0 },
	{ 0xBDE67498, "Cybermorph (World) (Rev 1)", "cybermorph-2mb.jpg", 0 },
	{ 0xC5562581, "Zoop! (World)", "zoop.jpg", 0 },
	{ 0xC654681B, "Total Carnage (World)", "total-carnage.jpg", 0 },
	{ 0xC6C7BA62, "Fight for Life (World) (Alt) [BOGUS DUMP]", "", 0 },
	{ 0xC9608717, "Val d'Isere Skiing and Snowboarding (World)", "val-d'isere-skiing.jpg", 0 },
	{ 0xCBFD822A, "Air Cars (World)", "air-cars.jpg", 0 },
	{ 0xCD5BF827, "Attack of the Mutant Penguins (World)", "attack-of-the-mutant-pengiuns.jpg", 0 },
	{ 0xD6C19E34, "Iron Soldier 2 (World)", "iron-soldier-2.jpg", 0 },
	{ 0xDA9C4162, "Missile Command 3D (World)", "missile-command-3d.jpg", 0 },
	{ 0xDC187F82, "Alien vs Predator (World)", "alien-vs-predator.jpg", 0 },
	{ 0xDE55DCC7, "Flashback - The Quest for Identity (World) (En,Fr)", "flashback.jpg", 0 },
	{ 0xE28756DE, "Atari Karts (World)", "atari-karts.jpg", 0 },
	{ 0xE60277BB, "[BIOS] Atari Jaguar Stubulator '93 (World)", "", 0 },
	{ 0xE91BD644, "Wolfenstein 3D (World)", "wolfenstein-3d.jpg", 0 },
	{ 0xEC22F572, "SuperCross 3D (World)", "supercross-3d.jpg", 0 },
	{ 0xECF854E7, "Cybermorph (World) (Rev 2)", "cybermorph.jpg", 0 },
	{ 0xEEE8D61D, "Club Drive (World)", "club-drive.jpg", 0 },
	{ 0xF0360DB3, "Hyper Force (World)", "hyper-force.jpg", 0 },
	{ 0xFA7775AE, "Checkered Flag (World)", "checkered-flag.jpg", 0 },
	{ 0xFAE31DD0, "Flip Out! (World)", "flip-out.jpg", 0 },
	{ 0xFB731AAA, "[BIOS] Atari Jaguar (World)", "", 0 },
	{ 0xFFFFFFFF, "***END***", "", 0 }
};
/*
breakout-2000.jpg
memory-track.jpg
protector.jpg
towers-2.jpg
*/

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

FileThread::FileThread(QObject * parent/*= 0*/): QThread(parent), /*listWidget(NULL),*/ abort(false)
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

//void FileThread::Go(QListWidget * lw)
void FileThread::Go(void)
{
	QMutexLocker locker(&mutex);
//	this->listWidget = lw;
	start();
}

//
// Here's the thread's actual execution path...
//
void FileThread::run(void)
{
	QDir romDir(vjs.ROMPath);
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
//printf("FilePickerWindow: File crc == %08X...\n", crc);

			uint32 index = FindCRCIndexInFileList(crc);

			if (index != 0xFFFFFFFF)
			{
//printf("FileThread: Found match [%s]...\n", romList[index].name);
//				new QListWidgetItem(QIcon(":/res/generic.png"), romList[index].name, listWidget);
//				emit FoundAFile(romList[index].crc32);
				emit FoundAFile(index);
			}
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

