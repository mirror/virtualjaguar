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
SDL_Joystick * joystick;
Uint32 mainSurfaceFlags = SDL_SWSURFACE;

bool finished = false;
bool fullscreen = false;
bool hardwareTypeNTSC = true;			// Set to false for PAL

bool useJoystick = false;
bool showGUI = false;

int16 * backbuffer = NULL;

// Added/changed by SDLEMU http://sdlemu.ngemu.com

uint32 totalFrames;//so we can grab this from elsewhere...
int main(int argc, char * argv[])
{

/*DIR * dp = opendir("../../ROMs/");
if (dp == NULL)
	printf("Could not open directory!\n");
else
{
	dirent * de;
	while ((de = readdir(dp)) != NULL)
	{
		char * ext = strrchr(de->d_name, '.');
		if (strcmpi(ext, ".zip") == 0 || strcmpi(ext, ".jag") == 0)
			printf("--> %s\n", de->d_name);
	}

	closedir(dp);
}*/

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
				extern uint32 gpu_pc;
				extern uint32 dsp_pc;
				DrawText(backbuffer, 8, 0, true, "Friendly GUI brought to you by JLH ;-)");
				DrawText(backbuffer, 8, 8, false, "GPU PC: %08X", gpu_pc);
				DrawText(backbuffer, 8, 16, false, "DSP PC: %08X", dsp_pc);
			}

			// Simple frameskip
			if (nFrame == nFrameskip)
			{
				if (SDL_MUSTLOCK(surface))
					while (SDL_LockSurface(surface) < 0)
						SDL_Delay(10);

				memcpy(surface->pixels, backbuffer, tom_width * tom_height * 2);

				if (SDL_MUSTLOCK(surface))
					SDL_UnlockSurface(surface);

				SDL_Rect srcrect, dstrect;
				srcrect.x = srcrect.y = 0, srcrect.w = surface->w, srcrect.h = surface->h;
				dstrect.x = dstrect.y = 0, dstrect.w = surface->w, dstrect.h = surface->h;
				SDL_BlitSurface(surface, &srcrect, mainSurface, &dstrect);
			    SDL_Flip(mainSurface);      
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
// Very very crude GUI file selector
//
uint32 DoROMFind(uint8 * mem, char * path)
{
	char cartName[2048];	// Really, should be MAX_PATH or something like it
	uint32 numFiles = 0;
	DIR * dp = opendir(path);

	if (dp == NULL)
	{
		WriteLog("VJ: Could not open directory \"%s\"!\nAborting!\n", path);
		log_done();
		exit(0);
	}
	else
	{
		dirent * de;
		while ((de = readdir(dp)) != NULL)
		{
			char * ext = strrchr(de->d_name, '.');
			if (ext != NULL)
				if (strcmpi(ext, ".zip") == 0 || strcmpi(ext, ".jag") == 0)
					numFiles++;
		}
		closedir(dp);
	}

	if (numFiles == 0)
	{
		WriteLog("Found no ROM files!\nAborting!\n");
		log_done();
		exit(0);
	}

	char * names = (char *)malloc(numFiles * 2048);
	if (names == NULL)
	{
		WriteLog("Could not allocate memory for %u files!\nAborting!\n", numFiles);
		log_done();
		exit(0);
	}

	int i = 0;
	dp = opendir(path);
	dirent * de;
	while ((de = readdir(dp)) != NULL)
	{
		char * ext = strrchr(de->d_name, '.');
		if (ext != NULL)
			if (strcmpi(ext, ".zip") == 0 || strcmpi(ext, ".jag") == 0)
				strcpy(&names[i++ * 2048], de->d_name);
	}
	closedir(dp);

	// Main GUI selection loop

	uint32 cursor = 0;

	if (numFiles > 1)	// Only go GUI if more than one possibility!
	{
	bool done = false;
	uint32 limit = (numFiles > 24 ? 24 : numFiles);
	SDL_Event event;
	while (!done)
	
	while (SDL_PollEvent(&event))
	{
		// Draw the GUI...
		memset(backbuffer, 0x11, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);
		uint32 startFile = (cursor >= limit ? cursor - limit + 1 : 0);
		for(uint32 i=0; i<limit; i++)
		{
			bool invert = (cursor == startFile + i ? true : false);
			char buf[41];
			// Guarantee that we clip our strings to fit in the screen...
			memcpy(buf, &names[(startFile + i) * 2048], 38);
			buf[38] = 0;
			DrawText(backbuffer, 0, i*8, invert, " %s ", buf);
		}


				if (SDL_MUSTLOCK(surface))
					while (SDL_LockSurface(surface) < 0)
						SDL_Delay(10);

//				memcpy(surface->pixels, backbuffer, tom_width * tom_height * 2);
				memcpy(surface->pixels, backbuffer, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);

				if (SDL_MUSTLOCK(surface))
					SDL_UnlockSurface(surface);

				SDL_Rect srcrect, dstrect;
				srcrect.x = srcrect.y = 0, srcrect.w = surface->w, srcrect.h = surface->h;
				dstrect.x = dstrect.y = 0, dstrect.w = surface->w, dstrect.h = surface->h;
				SDL_BlitSurface(surface, &srcrect, mainSurface, &dstrect);
			    SDL_Flip(mainSurface);      


		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_UP:
				if (cursor != 0)
					cursor--;
				break;
			case SDLK_DOWN:
				if (cursor != numFiles - 1)
					cursor++;
				break;
			case SDLK_RETURN:
				done = true;
				break;
			}
		}
	}
	}

	strcpy(cartName, path);
	if (path[strlen(path) - 1] != '/')
		strcat(cartName, "/");
	strcat(cartName, &names[cursor * 2048]);
	free(names);

	uint32 romSize = JaguarLoadROM(mem, cartName);
	if (romSize == 0)
	{
		log_done();
		exit(0);
	}
	return romSize;
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
//				log_done();
//				exit(0);
				return 0;
			}
		}
		else
		{
			FILE * fp = fopen(path, "rb");

			if (fp == NULL)
			{
				WriteLog("Failed!\n");
//				log_done();
//				exit(0);
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
		WriteLog("VJ: Trying GUI...\n");
		romSize = DoROMFind(mem, path);
	}

	jaguar_mainRom_crc32 = crc32_calcCheckSum(jaguar_mainRom, romSize);
	WriteLog( "CRC: %08X\n", (unsigned int)jaguar_mainRom_crc32);
	eeprom_init();
}
