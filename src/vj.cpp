//
// Virtual Jaguar Emulator
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/fixes by James L. Hammons
//

//
// Important info:
//
//                           NTSC       PAL
// GPU/DSP/video clock rate  26.590906  26.593900
// 68000 clock rate          13.295453  13.296950
// (clock rates in MHz)
//

// Added by SDLEMU (http://sdlemu.ngemu.com)
// Added for GCC UNIX compatibility
#ifdef __GCCUNIX__
#include <unistd.h>
#endif	// __GCCUNIX__

#include <time.h>
#include <SDL.h>
#include "SDLptc.h"
#include "jaguar.h"
#include "crc32.h"
#include "unzip.h"

// Uncomment this for speed control
//#define SPEED_CONTROL

//
// Private function prototypes
//

uint32 JaguarLoadROM(uint8 *, char *);
void JaguarLoadCart(uint8 *, char *);

//
// Various paths
//

//static char  *jaguar_bootRom_path="c:/jaguarEmu/newload.img";
static char * jaguar_bootRom_path = "./bios/jagboot.rom";
//static char  *jaguar_bootRom_path="./bios/JagOS.bin";
char * jaguar_eeproms_path = "./eeproms/";
char jaguar_boot_dir[1024];
//static char romLoadDialog_filePath[1024];


Console console;
Surface * surface;
Format format(16, 0x007C00, 0x00003E0, 0x0000001F);
bool finished = false;
bool fullscreen = false;
bool hardwareTypeNTSC = true;			// Set to false for PAL

//
// External variables
//

extern bool jaguar_use_bios;
extern bool dsp_enabled;
extern uint8 * jaguar_mainRam;
extern uint8 * jaguar_bootRom;
extern uint8 * jaguar_mainRom;


void main_screen_switch(void)
{
	fullscreen = !fullscreen;
	if (fullscreen)
		console.option("fullscreen output");
	else
		console.option("windowed output");
	console.close();
	console.open("Virtual Jaguar", tom_width, tom_height, format);
}

// Added/changed by SDLEMU http://sdlemu.ngemu.com

uint32 totalFrames;//so we can grab this from somewhere else...
int main(int argc, char * argv[])
{
	uint32 startTime;//, totalFrames;//, endTime;//, w, h;
//	int32 * vs;
	uint32 nNormalLast = 0;
	int32 nNormalFrac = 0; 
//	int32 i = 0;
//unused	int32 nTime = 0;
//unused	int32 nCount = 0;
    int32 nFrameskip = 0;								// Default: Show every frame
    int32 nFrame = 0;									// No. of Frame
    int32 nJoyport = 0;									// Joystick port

	printf("Virtual Jaguar/SDL v1.0.5 (GCC/SDL Port)\n");
	printf("Based upon Virtual Jaguar core v1.0.0 by cal2 of Potato emulation.\n");
	printf("Written by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)\n");
	printf("Portions massaged by James L. Hammons (WIN32)\n");
	printf("Contact: http://sdlemu.ngemu.com/ | sdlemu@ngemu.com\n");

    console.option("windowed output");

	// BIOS is now ON by default--use the -nobios switch to turn it off!
	jaguar_use_bios = true;
	bool haveCart = false;									// Assume there is no cartridge...!

	// Checking the switches ;)

	for(int i=1; i<argc || argv[i]!=NULL; i++)
	{
		// This would be the most likely place to do the cart loading...
		if (argv[i][0] != '-')
			haveCart = true;								// It looks like we have a cartridge!

		if (!strcmp(argv[i], "-fullscreen")) 
		{
			fullscreen = true;
			console.option("fullscreen output");
		}

		if (!strcmp(argv[i], "-window")) 
			console.option("windowed output");

		if (!strcmp(argv[i], "-joystick")) 
			console.option("joystick enabled");

		if (!strcmp(argv[i], "-joyport"))
		{
			nJoyport = atoi(argv[++i]) + 1;
			if (nJoyport > 3)
				nJoyport = 3;
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

		if (!strcmp(argv[i], "-nobios"))
			jaguar_use_bios = false;

		if (!strcmp(argv[i], "-dspon"))
			dsp_enabled = 1;

		if (!strcmp(argv[i], "-pal"))
			hardwareTypeNTSC = false;

		if (!strcmp(argv[i], "-help") || !strcmp(argv[i], "-?"))
		{
		    printf("Usage: \n\n");
			printf("vj [romfile] [switches]\n");
			printf("  -? or -help     : Display usage and switches              \n");
			printf("  -fullscreen     : Enable fullscreen mode                  \n");
			printf("  -window         : Enable windowed mode (default)          \n");
			printf("  -frameskip 1-10 : Enable frameskip 1 (default) - 10       \n");
			printf("  -joystick       : Enable joystick/gamepad                 \n");
			printf("  -joyport   0-3  : Select desired joystick port            \n");
			printf("  -nobios         : Boot cart without using Jaguar BIOS ROM \n");
			printf("  -dspon          : Force VJ to use the DSP                 \n");
			printf("  -pal            : Force VJ to PAL mode (default is NTSC)  \n");
			printf("\nInvoking Virtual Jagaur with no ROM file will cause it to boot up\n");
			printf("with the Jaguar BIOS.\n");
 			return true;
		}
    }

	getcwd(jaguar_boot_dir, 1024);
	log_init("vj.log");
	memory_init();
	version_init();
	version_display(log_get());
	jaguar_init();

	// Get the BIOS ROM
	if (jaguar_use_bios)
		JaguarLoadROM(jaguar_bootRom, jaguar_bootRom_path);

	SET32(jaguar_mainRam, 0, 0x00200000);			// Set top of stack...

	// Get the cartridge ROM (if passed in)
	if (haveCart)
	{
		JaguarLoadCart(jaguar_mainRom, argv[1]);
		eeprom_init();
	}

	jaguar_reset();
	
	// Setting up the backbuffer
	int16 * backbuffer = (int16 *)malloc(845 * 525 * sizeof(int16));
	memset(backbuffer, 0xAA, tom_getVideoModeWidth() * tom_getVideoModeHeight() * sizeof(int16));

	// Setting up the primary SDL display
	surface = new Surface(tom_getVideoModeWidth(), tom_getVideoModeHeight(), format);

	// Initialize Joystick support under SDL
	if (console.JoyEnabled() == 1)
	{
		if (SDL_NumJoysticks() <= 0)
		{
	        console.option("joystick disabled");
			printf("No joystick(s) or joypad(s) detected on your system. Using keyboard...\n");
		}
		else
		{
			if ((console.joystick = SDL_JoystickOpen(nJoyport)) == 0)
			{
				console.option("joystick disabled");
				printf("Unable to open a Joystick on port: %d\n", (int)nJoyport);
			}
			else
				printf("Using: %s\n", SDL_JoystickName(nJoyport));
		}
	}

	// Open the display and start emulating some 3l337 Atari Jaguar games :P
	console.open("Virtual Jaguar", tom_getVideoModeWidth(), tom_getVideoModeHeight(), format);
	
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
            // Setting up new backbuffer with new pixels and data
//			jaguar_exec(backbuffer, true);
			JaguarExecute(backbuffer, true);
			totalFrames++;

			// Simple frameskip
			if (nFrame == nFrameskip)
			{
				int32 * vs = (int32 *)surface->lock();
				memcpy(vs, backbuffer, tom_width * tom_height * 2);
				surface->unlock();
				surface->copy(console);
				console.update();
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
	fprintf(log_get(), "Statistics: %i FPS\n", fps);
	
	if (console.JoyEnabled() == 1) {}
 
	jaguar_done();
	version_done();
	memory_done();
	log_done();	
	console.close();									// Close SDL items as last!

    return 0;
}

//
// Generic ROM loading
//
uint32 JaguarLoadROM(uint8 * rom, char * path)
{
	uint32 romSize;

	WriteLog("JagEm: Loading %s...", path);

	char * ext = strrchr(path, '.');
	if (strcmpi(ext, ".zip") == 0)
	{
		// Handle ZIP file loading here...
		WriteLog("(ZIPped)...");

		if (load_zipped_file(0, 0, path, NULL, &rom, &romSize) == -1)
		{
			WriteLog("Failed!\n");
			log_done();
			exit(0);
		}
	}
	else
	{
		FILE * fp = fopen(path, "rb");

		if (fp == NULL)
		{
			WriteLog("Failed!\n");
			log_done();
			exit(0);
		}

		fseek(fp, 0, SEEK_END);
		romSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fread(rom, 1, romSize, fp);
		fclose(fp);
	}

	WriteLog("OK (%i bytes)\n", romSize);
	return romSize;
}

//
// Jaguar cartridge ROM loading
//
void JaguarLoadCart(uint8 * mem, char * path)
{
	uint32 romsize = JaguarLoadROM(mem, path);
	jaguar_mainRom_crc32 = crc32_calcCheckSum(jaguar_mainRom, romsize);
	WriteLog( "CRC: %08X\n", (unsigned int)jaguar_mainRom_crc32);
}
