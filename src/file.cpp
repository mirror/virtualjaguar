//
// FILE.CPP
//
// File support
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JLH  02/28/2010  Added functions to look inside .ZIP files and handle contents
//

#include "file.h"

#include <stdarg.h>
#include <string.h>
#include "crc32.h"
#include "eeprom.h"
#include "jaguar.h"
#include "log.h"
#include "memory.h"
#include "unzip.h"
#include "zlib.h"

// Private function prototypes

static int gzfilelength(gzFile gd);
static bool CheckExtension(const char * filename, const char * ext);
static int ParseFileType(uint8 header1, uint8 header2, uint32 size);

// Private variables/enums

// JST = Jaguar Software Type
enum { JST_NONE = 0, JST_ROM, JST_ALPINE, JST_ABS_TYPE1, JST_ABS_TYPE2, JST_JAGSERVER };
//static int softwareType = JST_NONE;

//
// Generic ROM loading
//
uint32 JaguarLoadROM(uint8 * rom, char * path)
{
// We really should have some kind of sanity checking for the ROM size here to prevent
// a buffer overflow... !!! FIX !!!
#warning "!!! FIX !!! Should have sanity checking for ROM size to prevent buffer overflow!"
	uint32 romSize = 0;

	WriteLog("JaguarLoadROM: Attempting to load file '%s'...", path);
	char * ext = strrchr(path, '.');

	// No filename extension == YUO FAIL IT (it is loading the file).
	// This is naive, but it works. But should probably come up with something a little
	// more robust, to prevent problems with dopes trying to exploit this.
	if (ext == NULL)
	{
		WriteLog("FAILED!\n");
		return 0;
	}

	WriteLog("Succeeded in finding extension (%s)!\n", ext);
	WriteLog("VJ: Loading \"%s\"...", path);

	if (strcasecmp(ext, ".zip") == 0)
	{
		// Handle ZIP file loading here...
		WriteLog("(ZIPped)...");

		uint8_t * buffer = NULL;
		romSize = GetFileFromZIP(path, FT_SOFTWARE, buffer);

		if (romSize == 0)
		{
			WriteLog("Failed!\n");
			return 0;
		}

		memcpy(rom, buffer, romSize);
		delete[] buffer;
	}
	else
	{
		// Handle gzipped files transparently [Adam Green]...

		gzFile fp = gzopen(path, "rb");

		if (fp == NULL)
		{
			WriteLog("Failed!\n");
			return 0;
		}

		romSize = gzfilelength(fp);
		gzseek(fp, 0, SEEK_SET);
		gzread(fp, rom, romSize);
		gzclose(fp);
	}

	WriteLog("OK (%i bytes)\n", romSize);

	return romSize;
}

//
// Jaguar file loading
//
bool JaguarLoadFileOld(char * path)
{
	jaguarROMSize = JaguarLoadROM(jaguarMainROM, path);

	if (jaguarROMSize == 0)
	{
//			WriteLog("VJ: Could not load ROM from file \"%s\"...\nAborting!\n", newPath);
		WriteLog("GUI: Could not load ROM from file \"%s\"...\nAborting load!\n", path);
#warning "!!! Need error dialog here !!!"
// Need to do something else here, like throw up an error dialog instead of aborting. !!! FIX !!!
//		log_done();
//		exit(0);
		return false;								// This is a start...
	}

	jaguarMainROMCRC32 = crc32_calcCheckSum(jaguarMainROM, jaguarROMSize);
	WriteLog("CRC: %08X\n", (unsigned int)jaguarMainROMCRC32);
	EepromInit();

	jaguarRunAddress = 0x802000;

	char * ext = strrchr(path, '.');				// Get the file's extension for non-cartridge checking

//NOTE: Should fix JaguarLoadROM() to replace .zip with what's *in* the zip (.abs, .j64, etc.)
#warning "!!! Need more robust file type checking in JaguarLoadROM() !!!"
	if (strcasecmp(ext, ".rom") == 0)
	{
		// File extension ".ROM": Alpine image that loads/runs at $802000
		WriteLog("GUI: Setting up homebrew (ROM)... Run address: 00802000, length: %08X\n", jaguarROMSize);

		for(int i=jaguarROMSize-1; i>=0; i--)
			jaguarMainROM[0x2000 + i] = jaguarMainROM[i];

		memset(jaguarMainROM, 0xFF, 0x2000);
/*		memcpy(jaguar_mainRam, jaguar_mainRom, jaguarRomSize);
		memset(jaguar_mainRom, 0xFF, 0x600000);
		memcpy(jaguar_mainRom + 0x2000, jaguar_mainRam, jaguarRomSize);
		memset(jaguar_mainRam, 0x00, 0x400000);*/

/*
Stubulator ROM vectors...
handler 001 at $00E00008
handler 002 at $00E008DE
handler 003 at $00E008E2
handler 004 at $00E008E6
handler 005 at $00E008EA
handler 006 at $00E008EE
handler 007 at $00E008F2
handler 008 at $00E0054A
handler 009 at $00E008FA
handler 010 at $00000000
handler 011 at $00000000
handler 012 at $00E008FE
handler 013 at $00E00902
handler 014 at $00E00906
handler 015 at $00E0090A
handler 016 at $00E0090E
handler 017 at $00E00912
handler 018 at $00E00916
handler 019 at $00E0091A
handler 020 at $00E0091E
handler 021 at $00E00922
handler 022 at $00E00926
handler 023 at $00E0092A
handler 024 at $00E0092E
handler 025 at $00E0107A
handler 026 at $00E0107A
handler 027 at $00E0107A
handler 028 at $00E008DA
handler 029 at $00E0107A
handler 030 at $00E0107A
handler 031 at $00E0107A
handler 032 at $00000000

Let's try setting up the illegal instruction vector for a stubulated jaguar...
*/
/*		SET32(jaguar_mainRam, 0x08, 0x00E008DE);
		SET32(jaguar_mainRam, 0x0C, 0x00E008E2);
		SET32(jaguar_mainRam, 0x10, 0x00E008E6);	// <-- Should be here (it is)...
		SET32(jaguar_mainRam, 0x14, 0x00E008EA);//*/

		// Try setting the vector to say, $1000 and putting an instruction there that loops forever:
		// This kludge works! Yeah!
		SET32(jaguarMainRAM, 0x10, 0x00001000);
		SET16(jaguarMainRAM, 0x1000, 0x60FE);		// Here: bra Here
	}
	else if (strcasecmp(ext, ".abs") == 0)
	{
		// File extension ".ABS": Atari linker output file with header (w/o is useless to us here)

/*
ABS Format sleuthing (LBUGDEMO.ABS):

000000  60 1B 00 00 05 0C 00 04 62 C0 00 00 04 28 00 00
000010  12 A6 00 00 00 00 00 80 20 00 FF FF 00 80 25 0C
000020  00 00 40 00

DRI-format file detected...
Text segment size = 0x0000050c bytes
Data segment size = 0x000462c0 bytes
BSS Segment size = 0x00000428 bytes
Symbol Table size = 0x000012a6 bytes
Absolute Address for text segment = 0x00802000
Absolute Address for data segment = 0x0080250c
Absolute Address for BSS segment = 0x00004000

(CRZDEMO.ABS):
000000  01 50 00 03 00 00 00 00 00 03 83 10 00 00 05 3b
000010  00 1c 00 03 00 00 01 07 00 00 1d d0 00 03 64 98
000020  00 06 8b 80 00 80 20 00 00 80 20 00 00 80 3d d0

000030  2e 74 78 74 00 00 00 00 00 80 20 00 00 80 20 00 .txt (+36 bytes)
000040  00 00 1d d0 00 00 00 a8 00 00 00 00 00 00 00 00
000050  00 00 00 00 00 00 00 20
000058  2e 64 74 61 00 00 00 00 00 80 3d d0 00 80 3d d0 .dta (+36 bytes)
000068  00 03 64 98 00 00 1e 78 00 00 00 00 00 00 00 00
000078  00 00 00 00 00 00 00 40
000080  2e 62 73 73 00 00 00 00 00 00 50 00 00 00 50 00 .bss (+36 bytes)
000090  00 06 8b 80 00 03 83 10 00 00 00 00 00 00 00 00
0000a0  00 00 00 00 00 00 00 80

Header size is $A8 bytes...

BSD/COFF format file detected...
3 sections specified
Symbol Table offset = 230160				($00038310)
Symbol Table contains 1339 symbol entries	($0000053B)
The additional header size is 28 bytes		($001C)
Magic Number for RUN_HDR = 0x00000107
Text Segment Size = 7632					($00001DD0)
Data Segment Size = 222360					($00036498)
BSS Segment Size = 428928					($00068B80)
Starting Address for executable = 0x00802000
Start of Text Segment = 0x00802000
Start of Data Segment = 0x00803dd0
*/
		if (jaguarMainROM[0] == 0x60 && jaguarMainROM[1] == 0x1B)
		{
			uint32 loadAddress = GET32(jaguarMainROM, 0x16), //runAddress = GET32(jaguar_mainRom, 0x2A),
				codeSize = GET32(jaguarMainROM, 0x02) + GET32(jaguarMainROM, 0x06);
			WriteLog("GUI: Setting up homebrew (ABS-1)... Run address: %08X, length: %08X\n", loadAddress, codeSize);

			if (loadAddress < 0x800000)
				memcpy(jaguarMainRAM + loadAddress, jaguarMainROM + 0x24, codeSize);
			else
			{
				for(int i=codeSize-1; i>=0; i--)
					jaguarMainROM[(loadAddress - 0x800000) + i] = jaguarMainROM[i + 0x24];
/*				memcpy(jaguar_mainRam, jaguar_mainRom + 0x24, codeSize);
				memset(jaguar_mainRom, 0xFF, 0x600000);
				memcpy(jaguar_mainRom + (loadAddress - 0x800000), jaguar_mainRam, codeSize);
				memset(jaguar_mainRam, 0x00, 0x400000);*/
			}

			jaguarRunAddress = loadAddress;
		}
		else if (jaguarMainROM[0] == 0x01 && jaguarMainROM[1] == 0x50)
		{
			uint32 loadAddress = GET32(jaguarMainROM, 0x28), runAddress = GET32(jaguarMainROM, 0x24),
				codeSize = GET32(jaguarMainROM, 0x18) + GET32(jaguarMainROM, 0x1C);
			WriteLog("GUI: Setting up homebrew (ABS-2)... Run address: %08X, length: %08X\n", runAddress, codeSize);

			if (loadAddress < 0x800000)
				memcpy(jaguarMainRAM + loadAddress, jaguarMainROM + 0xA8, codeSize);
			else
			{
				for(int i=codeSize-1; i>=0; i--)
					jaguarMainROM[(loadAddress - 0x800000) + i] = jaguarMainROM[i + 0xA8];
/*				memcpy(jaguar_mainRam, jaguar_mainRom + 0xA8, codeSize);
				memset(jaguar_mainRom, 0xFF, 0x600000);
				memcpy(jaguar_mainRom + (loadAddress - 0x800000), jaguar_mainRam, codeSize);
				memset(jaguar_mainRam, 0x00, 0x400000);*/
			}

			jaguarRunAddress = runAddress;
		}
		else
		{
			WriteLog("GUI: Couldn't find correct ABS/COF format: %02X %02X\n", jaguarMainROM[0], jaguarMainROM[1]);
			return false;
		}
	}
	else if (strcasecmp(ext, ".jag") == 0)
	{
		// File extension ".JAG": Atari server file with header
//NOTE: The bytes 'JAGR' should also be at position $1C...
//      Also, there's *always* a $601A header at position $00...
		if (jaguarMainROM[0] == 0x60 && jaguarMainROM[1] == 0x1A)
		{
			uint32 loadAddress = GET32(jaguarMainROM, 0x22), runAddress = GET32(jaguarMainROM, 0x2A);
//This is not always right! Especially when converted via bin2jag1!!!
//We should have access to the length of the furshlumiger file that was loaded anyway!
//Now, we do! ;-)
//			uint32 progLength = GET32(jaguar_mainRom, 0x02);
//jaguarRomSize
//jaguarRunAddress
//			WriteLog("Jaguar: Setting up PD ROM... Run address: %08X, length: %08X\n", runAddress, progLength);
//			memcpy(jaguar_mainRam + loadAddress, jaguar_mainRom + 0x2E, progLength);
			WriteLog("GUI: Setting up homebrew (JAG)... Run address: %08X, length: %08X\n", runAddress, jaguarROMSize - 0x2E);
			memcpy(jaguarMainRAM + loadAddress, jaguarMainROM + 0x2E, jaguarROMSize - 0x2E);
//		SET32(jaguar_mainRam, 4, runAddress);
			jaguarRunAddress = runAddress;
		}
		else
			return false;
	}
	// .J64 (Jaguar cartridge ROM image) is implied by the FileList object...

	return true;
}

//
// Jaguar file loading (second stab at it...)
// We do a more intelligent file analysis here instead of relying on (possible false)
// file extensions which people don't seem to give two shits about anyway. :-(
//
bool JaguarLoadFile(char * path)
{
// NOTE: We can further clean this up by fixing JaguarLoadROM to load to a buffer
//       instead of assuming it goes into our ROM space.
	jaguarROMSize = JaguarLoadROM(jaguarMainROM, path);

	if (jaguarROMSize == 0)
	{
//			WriteLog("VJ: Could not load ROM from file \"%s\"...\nAborting!\n", newPath);
		WriteLog("GUI: Could not load ROM from file \"%s\"...\nAborting load!\n", path);
#warning "!!! Need error dialog here !!!"
// Need to do something else here, like throw up an error dialog instead of aborting. !!! FIX !!!
		return false;								// This is a start...
	}

	jaguarMainROMCRC32 = crc32_calcCheckSum(jaguarMainROM, jaguarROMSize);
	WriteLog("CRC: %08X\n", (unsigned int)jaguarMainROMCRC32);
// TODO: Check for EEPROM file in ZIP file. If there is no EEPROM in the user's EEPROM
//       directory, copy the one from the ZIP file, if it exists.
	EepromInit();
	jaguarRunAddress = 0x802000;					// For non-BIOS runs, this is true
	int fileType = ParseFileType(jaguarMainROM[0], jaguarMainROM[1], jaguarROMSize);

	if (fileType == JST_ROM)
		return true;
	else if (fileType == JST_ALPINE)
	{
		// File extension ".ROM": Alpine image that loads/runs at $802000
		WriteLog("GUI: Setting up Alpine ROM... Run address: 00802000, length: %08X\n", jaguarROMSize);

		for(int i=jaguarROMSize-1; i>=0; i--)
			jaguarMainROM[0x2000 + i] = jaguarMainROM[i];

		memset(jaguarMainROM, 0xFF, 0x2000);

// Maybe instead of this, we could try requiring the STUBULATOR ROM? Just a thought...
		// Try setting the vector to say, $1000 and putting an instruction there that loops forever:
		// This kludge works! Yeah!
		SET32(jaguarMainRAM, 0x10, 0x00001000);
		SET16(jaguarMainRAM, 0x1000, 0x60FE);		// Here: bra Here
		return true;
	}
	else if (fileType == JST_ABS_TYPE1)
	{
		uint32 loadAddress = GET32(jaguarMainROM, 0x16),
			codeSize = GET32(jaguarMainROM, 0x02) + GET32(jaguarMainROM, 0x06);
		WriteLog("GUI: Setting up homebrew (ABS-1)... Run address: %08X, length: %08X\n", loadAddress, codeSize);

		if (loadAddress < 0x800000)
			memcpy(jaguarMainRAM + loadAddress, jaguarMainROM + 0x24, codeSize);
		else
		{
			for(int i=codeSize-1; i>=0; i--)
				jaguarMainROM[(loadAddress - 0x800000) + i] = jaguarMainROM[i + 0x24];
		}

		jaguarRunAddress = loadAddress;
		return true;
	}
	else if (fileType == JST_ABS_TYPE2)
	{
		uint32 loadAddress = GET32(jaguarMainROM, 0x28), runAddress = GET32(jaguarMainROM, 0x24),
			codeSize = GET32(jaguarMainROM, 0x18) + GET32(jaguarMainROM, 0x1C);
		WriteLog("GUI: Setting up homebrew (ABS-2)... Run address: %08X, length: %08X\n", runAddress, codeSize);

		if (loadAddress < 0x800000)
			memcpy(jaguarMainRAM + loadAddress, jaguarMainROM + 0xA8, codeSize);
		else
		{
			for(int i=codeSize-1; i>=0; i--)
				jaguarMainROM[(loadAddress - 0x800000) + i] = jaguarMainROM[i + 0xA8];
		}

		jaguarRunAddress = runAddress;
		return true;
	}
	else if (fileType == JST_JAGSERVER)
	{
		uint32 loadAddress = GET32(jaguarMainROM, 0x22), runAddress = GET32(jaguarMainROM, 0x2A);
		WriteLog("GUI: Setting up homebrew (Jag Server)... Run address: %08X, length: %08X\n", runAddress, jaguarROMSize - 0x2E);
		memcpy(jaguarMainRAM + loadAddress, jaguarMainROM + 0x2E, jaguarROMSize - 0x2E);
		jaguarRunAddress = runAddress;
		return true;
	}

	// We can assume we have JST_NONE at this point. :-P
	// TODO: Add a dialog box that tells the user that they're trying to feed VJ a bogus file.
	return false;
}

//
// Get the length of a (possibly) gzipped file
//
static int gzfilelength(gzFile gd)
{
   int size = 0, length = 0;
   unsigned char buffer[0x10000];

   gzrewind(gd);

   do
   {
      // Read in chunks until EOF
      size = gzread(gd, buffer, 0x10000);

      if (size <= 0)
      	break;

      length += size;
   }
   while (!gzeof(gd));

   gzrewind(gd);
   return length;
}

//
// Compare extension to passed in filename. If equal, return true; otherwise false.
//
static bool CheckExtension(const char * filename, const char * ext)
{
	const char * filenameExt = strrchr(filename, '.');	// Get the file's extension (if any)
	return (strcasecmp(filenameExt, ext) == 0 ? true : false);
}

//
// Get file from .ZIP
// Returns the size of the file inside the .ZIP file that we're looking at
// NOTE: If the thing we're looking for is found, it allocates it in the passed in buffer.
//       Which means we have to deallocate it later.
//
uint32 GetFileFromZIP(const char * zipFile, FileType type, uint8 * &buffer)
{
// NOTE: We could easily check for this by discarding anything that's larger than the RAM/ROM
//       size of the Jaguar console.
#warning "!!! FIX !!! Should have sanity checking for ROM size to prevent buffer overflow!"
	const char ftStrings[5][32] = { "Software", "EEPROM", "Label", "Box Art", "Controller Overlay" };
	ZIP * zip = openzip(0, 0, zipFile);

	if (zip == NULL)
	{
		WriteLog("FILE: Could not open file '%s'!\n", zipFile);
		return 0;
	}

	zipent * ze;
	bool found = false;

	// The order is here is important: If the file is found, we need to short-circuit the
	// readzip() call because otherwise, 'ze' will be pointing to the wrong file!
	while (!found && readzip(zip))
	{
		ze = &zip->ent;

		// Here we simply rely on the file extension to tell the truth, but we know
		// that extensions lie like sons-a-bitches. So this is naive, we need to do
		// something a little more robust to keep bad things from happening here.
#warning "!!! Checking for image by extension can be fooled !!!"
		if ((type == FT_LABEL) && (CheckExtension(ze->name, ".png") || CheckExtension(ze->name, ".jpg") || CheckExtension(ze->name, ".gif")))
		{
			found = true;
			WriteLog("FILE: Found image file '%s'.\n", ze->name);
		}

		if ((type == FT_SOFTWARE) && (CheckExtension(ze->name, ".j64") || CheckExtension(ze->name, ".rom") || CheckExtension(ze->name, ".abs") || CheckExtension(ze->name, ".cof")))
		{
			found = true;
			WriteLog("FILE: Found software file '%s'.\n", ze->name);
		}

		if ((type == FT_EEPROM) && (CheckExtension(ze->name, ".eep") || CheckExtension(ze->name, ".eeprom")))
		{
			found = true;
			WriteLog("FILE: Found EEPROM file '%s'.\n", ze->name);
		}
	}

	uint32 fileSize = 0;

	if (found)
	{
		WriteLog("FILE: Uncompressing...");
// Insert file size sanity check here...
		buffer = new uint8[ze->uncompressed_size];

		if (readuncompresszip(zip, ze, (char *)buffer) == 0)
		{
			fileSize = ze->uncompressed_size;
			WriteLog("success! (%u bytes)\n", fileSize);
		}
		else
		{
			delete[] buffer;
			buffer = NULL;
			WriteLog("FAILED!\n");
		}
	}
	else
		// Didn't find what we're looking for...
		WriteLog("FILE: Failed to find file of type %s...\n", ftStrings[type]);

	closezip(zip);
	return fileSize;
}

//
// Parse the file type based upon file size and/or headers.
//
static int ParseFileType(uint8 header1, uint8 header2, uint32 size)
{
	// If the file size is divisible by 1M, we probably have an regular ROM.
	// We can also check our CRC32 against the internal ROM database to be sure.
	if ((size % 1048576) == 0)
		return JST_ROM;

	// If the file size + 8192 bytes is divisible by 1M, we probably have an
	// Alpine format ROM.
	if (((size + 8192) % 1048576) == 0)
		return JST_ALPINE;

	// So much for low hanging fruit. Now try some other types.

	// ABS/COFF type 1
	if (header1 == 0x60 && header2 == 0x1B)
		return JST_ABS_TYPE1;

	// ABS/COFF type 2
	if (header1 == 0x01 && header2 == 0x50)
		return JST_ABS_TYPE2;

	// Jag Server
	if (header1 == 0x60 && header2 == 0x1A)
		return JST_JAGSERVER;

	// Headerless crap
	return JST_NONE;
}

#if 0
// Misc. doco

/*
Stubulator ROM vectors...
handler 001 at $00E00008
handler 002 at $00E008DE
handler 003 at $00E008E2
handler 004 at $00E008E6
handler 005 at $00E008EA
handler 006 at $00E008EE
handler 007 at $00E008F2
handler 008 at $00E0054A
handler 009 at $00E008FA
handler 010 at $00000000
handler 011 at $00000000
handler 012 at $00E008FE
handler 013 at $00E00902
handler 014 at $00E00906
handler 015 at $00E0090A
handler 016 at $00E0090E
handler 017 at $00E00912
handler 018 at $00E00916
handler 019 at $00E0091A
handler 020 at $00E0091E
handler 021 at $00E00922
handler 022 at $00E00926
handler 023 at $00E0092A
handler 024 at $00E0092E
handler 025 at $00E0107A
handler 026 at $00E0107A
handler 027 at $00E0107A
handler 028 at $00E008DA
handler 029 at $00E0107A
handler 030 at $00E0107A
handler 031 at $00E0107A
handler 032 at $00000000

Let's try setting up the illegal instruction vector for a stubulated jaguar...

		SET32(jaguar_mainRam, 0x08, 0x00E008DE);
		SET32(jaguar_mainRam, 0x0C, 0x00E008E2);
		SET32(jaguar_mainRam, 0x10, 0x00E008E6);	// <-- Should be here (it is)...
		SET32(jaguar_mainRam, 0x14, 0x00E008EA);//*/

/*
ABS Format sleuthing (LBUGDEMO.ABS):

000000  60 1B 00 00 05 0C 00 04 62 C0 00 00 04 28 00 00
000010  12 A6 00 00 00 00 00 80 20 00 FF FF 00 80 25 0C
000020  00 00 40 00

DRI-format file detected...
Text segment size = 0x0000050c bytes
Data segment size = 0x000462c0 bytes
BSS Segment size = 0x00000428 bytes
Symbol Table size = 0x000012a6 bytes
Absolute Address for text segment = 0x00802000
Absolute Address for data segment = 0x0080250c
Absolute Address for BSS segment = 0x00004000

(CRZDEMO.ABS):
000000  01 50 00 03 00 00 00 00 00 03 83 10 00 00 05 3b
000010  00 1c 00 03 00 00 01 07 00 00 1d d0 00 03 64 98
000020  00 06 8b 80 00 80 20 00 00 80 20 00 00 80 3d d0

000030  2e 74 78 74 00 00 00 00 00 80 20 00 00 80 20 00 .txt (+36 bytes)
000040  00 00 1d d0 00 00 00 a8 00 00 00 00 00 00 00 00
000050  00 00 00 00 00 00 00 20
000058  2e 64 74 61 00 00 00 00 00 80 3d d0 00 80 3d d0 .dta (+36 bytes)
000068  00 03 64 98 00 00 1e 78 00 00 00 00 00 00 00 00
000078  00 00 00 00 00 00 00 40
000080  2e 62 73 73 00 00 00 00 00 00 50 00 00 00 50 00 .bss (+36 bytes)
000090  00 06 8b 80 00 03 83 10 00 00 00 00 00 00 00 00
0000a0  00 00 00 00 00 00 00 80

Header size is $A8 bytes...

BSD/COFF format file detected...
3 sections specified
Symbol Table offset = 230160				($00038310)
Symbol Table contains 1339 symbol entries	($0000053B)
The additional header size is 28 bytes		($001C)
Magic Number for RUN_HDR = 0x00000107
Text Segment Size = 7632					($00001DD0)
Data Segment Size = 222360					($00036498)
BSS Segment Size = 428928					($00068B80)
Starting Address for executable = 0x00802000
Start of Text Segment = 0x00802000
Start of Data Segment = 0x00803dd0
*/
#endif
