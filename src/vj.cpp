//
// Virtual Jaguar Emulator
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/fixes by James L. Hammons
//

// Added by SDLEMU (http://sdlemu.ngemu.com)
// Added for GCC UNIX compatibility
#ifdef __GCCUNIX__
#include <unistd.h>
#endif	// __GCCUNIX__

#include <dirent.h>					// POSIX, but should compile with linux & mingw...
#include <time.h>
#include <SDL.h>
#include "jaguar.h"
#include "crc32.h"
#include "unzip.h"
#include "gui.h"

// Uncomment this for speed control
//#define SPEED_CONTROL

// Private function prototypes

uint32 JaguarLoadROM(uint8 *, char *);
void JaguarLoadCart(uint8 *, char *);

// External variables

//These two should be local!
extern bool jaguar_use_bios;
extern bool dsp_enabled;

extern uint8 * jaguar_mainRam;
extern uint8 * jaguar_bootRom;
extern uint8 * jaguar_mainRom;

// Various paths

static char * jaguar_bootRom_path = "./bios/jagboot.rom";
//static char  *jaguar_bootRom_path="c:/jaguarEmu/newload.img";
//static char  *jaguar_bootRom_path="./bios/JagOS.bin";
char * jaguar_eeproms_path = "./eeproms/";
char jaguar_boot_dir[1024];

SDL_Surface * surface, * mainSurface;
int16 * backbuffer = NULL;
SDL_Joystick * joystick;
Uint32 mainSurfaceFlags = SDL_SWSURFACE;

bool finished = false;
bool fullscreen = false;
bool hardwareTypeNTSC = true;			// Set to false for PAL

bool useJoystick = false;
bool showGUI = false;

// Added/changed by SDLEMU http://sdlemu.ngemu.com

uint32 totalFrames;//temp, so we can grab this from elsewhere...
int main(int argc, char * argv[])
{
	uint32 startTime;//, totalFrames;//, endTime;//, w, h;
	uint32 nNormalLast = 0;
	int32 nNormalFrac = 0; 
    int32 nFrameskip = 0;								// Default: Show every frame
    int32 nFrame = 0;									// No. of Frame
    int32 nJoyport = 0;									// Joystick port

	printf("Virtual Jaguar/SDL v1.0.5 (GCC/SDL Port)\n");
	printf("Based upon Virtual Jaguar core v1.0.0 by cal2 of Potato emulation.\n");
	printf("Written by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)\n");
	printf("Portions massaged by James L. Hammons (WIN32)\n");
	printf("Contact: http://sdlemu.ngemu.com/ | sdlemu@ngemu.com\n");

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
			fullscreen = true;

//We *don't* need this option!
/*		if (!strcmp(argv[i], "-window")) 
//			console.option("windowed output");
			fullscreen = false;*/

		if (!strcmp(argv[i], "-joystick")) 
			useJoystick = true;

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
			printf("  -? or -help     : Display usage and switches               \n");
			printf("  -fullscreen     : Enable fullscreen mode (windowed default)\n");
			printf("  -frameskip 1-10 : Enable frameskip 1 (default) - 10        \n");
			printf("  -joystick       : Enable joystick/gamepad                  \n");
			printf("  -joyport   0-3  : Select desired joystick port             \n");
			printf("  -nobios         : Boot cart without using Jaguar BIOS ROM  \n");
			printf("  -dspon          : Force VJ to use the DSP                  \n");
			printf("  -pal            : Force VJ to PAL mode (default is NTSC)   \n");
			printf("\nInvoking Virtual Jagaur with no ROM file will cause it to boot up\n");
			printf("with the Jaguar BIOS.\n");
 			return 1;
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

	jaguar_reset();

	// Set up the backbuffer
//	int16 * backbuffer = (int16 *)malloc(845 * 525 * sizeof(int16));
	backbuffer = (int16 *)malloc(845 * 525 * sizeof(int16));
	memset(backbuffer, 0x22, tom_getVideoModeWidth() * tom_getVideoModeHeight() * sizeof(int16));

	// Set up SDL library
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) < 0)
	{
		WriteLog("VJ: Could not initialize the SDL library: %s", SDL_GetError());
		exit(1);
	}

	// Let's get proper info about the platform we're running on...
	const SDL_VideoInfo * info = SDL_GetVideoInfo();

	if (!info)
	{
		WriteLog("VJ: SDL is unable to get the video info: %s\n", SDL_GetError());
		exit(1);
	}

	if (info->hw_available)
		mainSurfaceFlags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;

	if (info->blit_hw)
		mainSurfaceFlags |= SDL_HWACCEL;

	if (fullscreen)
		mainSurfaceFlags |= SDL_FULLSCREEN;

	// Note: mainSurface is *never* used again!
	//Not true--had to look at what's what here... It's the primary surface...
	mainSurface = SDL_SetVideoMode(tom_getVideoModeWidth(), tom_getVideoModeHeight(), 16, mainSurfaceFlags);

	if (mainSurface == NULL)
	{
		WriteLog("VJ: SDL is unable to set the video mode: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("Virtual Jaguar", "Virtual Jaguar");

	// Create the primary SDL display (16 BPP, 5/5/5 RGB format)
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, tom_getVideoModeWidth(),
		tom_getVideoModeHeight(), 16, 0x7C00, 0x03E0, 0x001F, 0);

	if (surface == NULL)
	{
		WriteLog("VJ: Could not create primary SDL surface: %s\n", SDL_GetError());
		exit(1);
	}

	// Initialize Joystick support under SDL
	if (useJoystick)
	{
		if (SDL_NumJoysticks() <= 0)
		{
			useJoystick = false;
			printf("VJ: No joystick(s) or joypad(s) detected on your system. Using keyboard...\n");
		}
		else
		{
			if ((joystick = SDL_JoystickOpen(nJoyport)) == 0)
			{
				useJoystick = false;
				printf("VJ: Unable to open a Joystick on port: %d\n", (int)nJoyport);
			}
			else
				printf("VJ: Using: %s\n", SDL_JoystickName(nJoyport));
		}
	}

	// Get the cartridge ROM (if passed in)
//	if (haveCart)
//		JaguarLoadCart(jaguar_mainRom, argv[1]);
	// Now with crunchy GUI goodness!
	JaguarLoadCart(jaguar_mainRom, (haveCart ? argv[1] : (char *)"."));

//Do this again??? Hmm... This is not very nice.
//Maybe it's not necessary??? Seems to be, at least for PD ROMs... !!! FIX !!!
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
            // Setting up new backbuffer with new pixels and data
			JaguarExecute(backbuffer, true);
			totalFrames++;

			// GUI stuff here...
			if (showGUI)
			{
				extern uint32 gpu_pc, dsp_pc;
				DrawText(backbuffer, 8, 8, false, "GPU PC: %08X", gpu_pc);
				DrawText(backbuffer, 8, 16, false, "DSP PC: %08X", dsp_pc);
			}

			// Simple frameskip
			if (nFrame == nFrameskip)
			{
				BlitBackbuffer();
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
	fprintf(log_get(), "VJ: Ran at an average of %i FPS.\n", fps);

	jaguar_done();
	version_done();
	memory_done();
	log_done();	

	SDL_JoystickClose(joystick);
	SDL_FreeSurface(surface);
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	SDL_Quit();

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

		if (strcmpi(ext, ".zip") == 0)
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
			FILE * fp = fopen(path, "rb");

			if (fp == NULL)
			{
				WriteLog("Failed!\n");
				return 0;
			}

			fseek(fp, 0, SEEK_END);
			romSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			fread(rom, 1, romSize, fp);
			fclose(fp);
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

		if (!UserSelectFile(path, newPath))
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
