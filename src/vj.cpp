//
// Virtual Jaguar Emulator
//
// Original codebase by Cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/fixes/enhancements by James L. Hammons and Adam Green
//

#ifdef __GCCUNIX__
#include <unistd.h>
#endif

#include <dirent.h>									// POSIX, but should compile with linux & mingw...
#include <time.h>
#include <SDL.h>
#include "jaguar.h"
#include "crc32.h"
#include "zlib.h"
#include "unzip.h"
#include "video.h"
#include "gui.h"
#include "sdlemu_opengl.h"
#include "settings.h"								// Pull in "vjs" struct

// Uncomment this for speed control (?)
//#define SPEED_CONTROL

// Private function prototypes

uint32 JaguarLoadROM(uint8 *, char *);
void JaguarLoadCart(uint8 *, char *);
int gzfilelength(gzFile gd);

// External variables

extern uint8 * jaguar_mainRam;
extern uint8 * jaguar_bootRom;
extern uint8 * jaguar_mainRom;

// Various paths

//static char * jaguar_bootRom_path = "./bios/jagboot.rom";
//static char  *jaguar_bootRom_path="c:/jaguarEmu/newload.img";
//static char  *jaguar_bootRom_path="./bios/JagOS.bin";
//char * jaguar_eeproms_path = "./eeproms/";
//char jaguar_boot_dir[MAX_PATH];

//These should go into video.cpp...
SDL_Surface * surface, * mainSurface;
int16 * backbuffer = NULL;
SDL_Joystick * joystick;
Uint32 mainSurfaceFlags = SDL_SWSURFACE;

bool finished = false;
bool showGUI = false;
bool showMessage = false;
uint32 showMessageTimeout;
char messageBuffer[200];

//
// The main emulator loop (what else?)
//
//Maybe we should move the video stuff to TOM? Makes more sense to put it there...
//Actually, it would probably be better served in VIDEO.CPP... !!! FIX !!! [DONE]
uint32 totalFrames;//temp, so we can grab this from elsewhere...
int main(int argc, char * argv[])
{
	uint32 startTime;//, totalFrames;//, endTime;//, w, h;
	uint32 nNormalLast = 0;
	int32 nNormalFrac = 0; 
    int32 nFrameskip = 0;							// Default: Show every frame
    int32 nFrame = 0;								// No. of Frame

	printf("Virtual Jaguar GCC/SDL Portable Jaguar Emulator v1.0.6\n");
	printf("Based upon Virtual Jaguar core v1.0.0 by Cal2 of Potato emulation.\n");
	printf("Written by Niels Wagenaar (Linux/WIN32), Caz (BeOS),\n");
	printf("James L. Hammons (WIN32) and Adam Green (MacOS)\n");
	printf("Contact: http://sdlemu.ngemu.com/ | sdlemu@ngemu.com\n");

	bool haveCart = false;							// Assume there is no cartridge...!

	log_init("vj.log");
	LoadVJSettings();								// Get config file settings...

	// Check the switches... ;-)
	// NOTE: Command line switches can override any config file settings, thus the
	//       proliferation of the noXXX switches. ;-)

	for(int i=1; i<argc || argv[i]!=NULL; i++)
	{
		// This would be the most likely place to do the cart loading...
		if (argv[i][0] != '-')
			haveCart = true;						// It looks like we have a cartridge!

		if (!strcmp(argv[i], "-joystick")) 
			vjs.useJoystick = true;

		if (!strcmp(argv[i], "-joyport"))
		{
			vjs.joyport = atoi(argv[++i]) + 1;
			if (vjs.joyport > 3)
				vjs.joyport = 3;
		}

		if (!strcmp(argv[i], "-frameskip"))
		{
			nFrameskip = atoi(argv[++i]) + 1;
			if (nFrameskip > 10)
				nFrameskip = 10;
#ifdef SPEED_CONTROL
			nFrameskip = 0;
#endif
		}

		if (!strcmp(argv[i], "-bios"))
			vjs.useJaguarBIOS = true;

		if (!strcmp(argv[i], "-nobios"))
			vjs.useJaguarBIOS = false;

		if (!strcmp(argv[i], "-dsp"))
			vjs.DSPEnabled = true;

		if (!strcmp(argv[i], "-nodsp"))
			vjs.DSPEnabled = false;

		if (!strcmp(argv[i], "-gl"))
			vjs.useOpenGL = true;

		if (!strcmp(argv[i], "-nogl"))
			vjs.useOpenGL = false;

		if (!strcmp(argv[i], "-fullscreen")) 
			vjs.fullscreen = true;

		if (!strcmp(argv[i], "-window")) 
			vjs.fullscreen = false;

		if (!strcmp(argv[i], "-pal"))
			vjs.hardwareTypeNTSC = false;

		if (!strcmp(argv[i], "-ntsc"))
			vjs.hardwareTypeNTSC = true;

		if (!strcmp(argv[i], "-help") || !strcmp(argv[i], "-?"))
		{
		    printf("Usage: \n\n");
			printf("vj [romfile] [switches]\n");
			printf("  -? or -help     : Display usage and switches                \n");
			printf("  -frameskip 1-10 : Enable frameskip 1 - 10 (default: none)   \n");
			printf("  -joystick       : Enable joystick/gamepad                   \n");
			printf("  -joyport 0-3    : Select desired joystick port              \n");
			printf("  -bios           : Boot cart using Jaguar BIOS ROM           \n");
			printf("  -nobios         : Boot cart without using Jaguar BIOS ROM   \n");
			printf("  -dsp            : Force VJ to use the DSP                   \n");
			printf("  -nodsp          : Force VJ to run without the DSP           \n");
			printf("  -gl             : Use OpenGL rendering                      \n");
			printf("  -nogl           : Use old non-OpenGL rendering              \n");
			printf("  -fullscreen     : Enable fullscreen mode (default: windowed)\n");
			printf("  -window         : Enable windowed mode                      \n");
			printf("  -pal            : Force VJ to PAL mode (default: NTSC)      \n");
			printf("  -ntsc           : Force VJ to NTSC mode                     \n");
			printf("\nInvoking Virtual Jagaur with no ROM file will cause it to boot up\n");
			printf("with the Jaguar BIOS.\n");
 			return 1;
		}
    }

//	getcwd(jaguar_boot_dir, 1024);
	memory_init();
	version_init();
	version_display(log_get());
	jaguar_init();

	// Get the BIOS ROM
	if (vjs.useJaguarBIOS)
		JaguarLoadROM(jaguar_bootRom, vjs.jagBootPath);

	SET32(jaguar_mainRam, 0, 0x00200000);			// Set top of stack...

	// Set up the backbuffer
//To be safe, this should be 1280 * 625 * 2...
//	backbuffer = (int16 *)malloc(845 * 525 * sizeof(int16));
	backbuffer = (int16 *)malloc(1280 * 625 * sizeof(int16));
	memset(backbuffer, 0x44, VIRTUAL_SCREEN_WIDTH * VIRTUAL_SCREEN_HEIGHT * sizeof(int16));

	InitVideo();
	InitGUI();

	// Get the cartridge ROM (if passed in)
	// Now with crunchy GUI goodness!
//	JaguarLoadCart(jaguar_mainRom, (haveCart ? argv[1] : (char *)""));
	JaguarLoadCart(jaguar_mainRom, (haveCart ? argv[1] : vjs.ROMPath));

	jaguar_reset();
	
	totalFrames = 0;
	startTime = clock();
	nNormalLast = 0;									// Last value of timeGetTime()
	nNormalFrac = 0;									// Extra fraction we did
	nNormalLast = SDL_GetTicks();						//timeGetTime();

	while (!finished)
	{
#ifdef SPEED_CONTROL
		nTime = SDL_GetTicks() - nNormalLast;			// calcule le temps écoulé depuis le dernier affichage
														// nTime est en mili-secondes.
		// détermine le nombre de trames à passer + 1
		nCount = (nTime * 600 - nNormalFrac) / 10000;

		// si le nombre de trames à passer + 1 est nul ou négatif,
		// ne rien faire pendant 2 ms
		if (nCount <= 0) 
		{ 
			//Sleep(2); 
			//SDL_Delay(1);
		} // No need to do anything for a bit
		else
		{
			nNormalFrac += nCount * 10000;				// 
			nNormalLast += nNormalFrac / 600;			// add the duration of nNormalFrac frames
			nNormalFrac %= 600;							// 

			// Pas plus de 9 (10-1) trames non affichées 
			if (nCount > 10)
				nCount = 10;
			for(int i=0; i<nCount-1; i++)
				jaguar_exec(backbuffer, false);
#endif
            // Set up new backbuffer with new pixels and data
			JaguarExecute(backbuffer, true);
			totalFrames++;

			// Some QnD GUI stuff here...
			if (showGUI)
			{
				extern uint32 gpu_pc, dsp_pc;
				DrawString(backbuffer, 8, 8, false, "GPU PC: %08X", gpu_pc);
				DrawString(backbuffer, 8, 16, false, "DSP PC: %08X", dsp_pc);
			}

			// Simple frameskip
			if (nFrame == nFrameskip)
			{
				RenderBackbuffer();
				nFrame = 0;
			}
			else
				nFrame++;

			joystick_exec();

#ifdef SPEED_CONTROL
		}
#endif
	}

	int elapsedTime = clock() - startTime;
	int fps = (1000 * totalFrames) / elapsedTime;
	WriteLog("VJ: Ran at an average of %i FPS.\n", fps);

	jaguar_done();
	version_done();
	memory_done();
	VideoDone();									// Free SDL components last...!
	log_done();	

    return 0;
}

//
// Generic ROM loading
//
uint32 JaguarLoadROM(uint8 * rom, char * path)
{
	uint32 romSize = 0;

	char * ext = strrchr(path, '.');
	if (ext != NULL)
	{
		WriteLog("VJ: Loading %s...", path);

		if (stricmp(ext, ".zip") == 0)
		{
			// Handle ZIP file loading here...
			WriteLog("(ZIPped)...");

			if (load_zipped_file(0, 0, path, NULL, &rom, &romSize) == -1)
			{
				WriteLog("Failed!\n");
				return 0;
			}
		}
		else
		{
/*			FILE * fp = fopen(path, "rb");

			if (fp == NULL)
			{
				WriteLog("Failed!\n");
				return 0;
			}

			fseek(fp, 0, SEEK_END);
			romSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			fread(rom, 1, romSize, fp);
			fclose(fp);*/

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
	}

	return romSize;
}

//
// Jaguar cartridge ROM loading
//
void JaguarLoadCart(uint8 * mem, char * path)
{
	uint32 romSize = JaguarLoadROM(mem, path);

	if (romSize == 0)
	{
		char newPath[2048];
		WriteLog("VJ: Trying GUI...\n");

//This is not *nix friendly for some reason...
//		if (!UserSelectFile(path, newPath))
		if (!UserSelectFile((strlen(path) == 0 ? (char *)"." : path), newPath))
		{
			WriteLog("VJ: Could not find valid ROM in directory \"%s\"...\nAborting!\n", path);
			log_done();
			exit(0);
		}

		romSize = JaguarLoadROM(mem, newPath);

		if (romSize == 0)
		{
			WriteLog("VJ: Could not load ROM from file \"%s\"...\nAborting!\n", newPath);
			log_done();
			exit(0);
		}
	}

	jaguar_mainRom_crc32 = crc32_calcCheckSum(jaguar_mainRom, romSize);
	WriteLog("CRC: %08X\n", (unsigned int)jaguar_mainRom_crc32);
	eeprom_init();
}

//
// Get the length of a (possibly) gzipped file
//
int gzfilelength(gzFile gd)
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
