//
// Virtual Jaguar Emulator
//
// Original codebase by David Raingeard (Cal2)
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/fixes/enhancements by James L. Hammons and Adam Green
//

#ifdef __GCCUNIX__
#include <unistd.h>									// Is this necessary anymore?
#endif

#include <time.h>
#include <SDL.h>
#include "jaguar.h"
#include "video.h"
#include "gui.h"
#include "sdlemu_opengl.h"
#include "settings.h"								// Pull in "vjs" struct

// Uncomment this to use built-in BIOS/CD-ROM BIOS
// You'll need a copy of jagboot.h & jagcd.h for this to work...!
//#define USE_BUILT_IN_BIOS

#ifdef USE_BUILT_IN_BIOS
#include "jagboot.h"
#include "jagcd.h"
#endif

// Private function prototypes

// External variables

extern uint8 * jaguar_mainRam;
extern uint8 * jaguar_mainRom;
extern uint8 * jaguar_bootRom;
extern uint8 * jaguar_CDBootROM;

// Global variables (export capable)
//should these even be here anymore?

bool finished = false;
bool showGUI = false;
bool showMessage = false;
uint32 showMessageTimeout;
char messageBuffer[200];
//bool BIOSLoaded = false;
//bool CDBIOSLoaded = false;

//
// The main emulator loop (what else?)
//
//Maybe we should move the video stuff to TOM? Makes more sense to put it there...
//Actually, it would probably be better served in VIDEO.CPP... !!! FIX !!! [DONE]
uint32 totalFrames;//temp, so we can grab this from elsewhere...
int main(int argc, char * argv[])
{
//NOTE: This isn't actually used anywhere... !!! FIX !!!
	int32 nFrameskip = 0;							// Default: Show every frame

	printf("Virtual Jaguar GCC/SDL Portable Jaguar Emulator v1.1.0\n");
	printf("Based upon Virtual Jaguar core v1.0.0 by David Raingeard.\n");
	printf("Written by Niels Wagenaar (Linux/WIN32), Carwin Jones (BeOS),\n");
	printf("James L. Hammons (WIN32) and Adam Green (MacOS)\n");
	printf("Stephan Kapfer (WIN32)\n");
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
		}

		//if (!strcmp(argv[i], "-bios"))
		//	vjs.useJaguarBIOS = true;

		//if (!strcmp(argv[i], "-nobios"))
		//	vjs.useJaguarBIOS = false;

		//if (!strcmp(argv[i], "-dsp"))
		//	vjs.DSPEnabled = true;

		//if (!strcmp(argv[i], "-nodsp"))
		//	vjs.DSPEnabled = false;

		//if (!strcmp(argv[i], "-pipeline"))
		//	vjs.usePipelinedDSP = true;

		//if (!strcmp(argv[i], "-nopipeline"))
		//	vjs.usePipelinedDSP = false;

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
			printf("  -pipeline       : Use the DSP pipelined core                \n");
			printf("  -nopipeline     : Use the DSP non-pipelined core            \n");
			printf("  -gl             : Use OpenGL rendering                      \n");
			printf("  -nogl           : Use old non-OpenGL rendering              \n");
			printf("  -fullscreen     : Enable fullscreen mode (default: windowed)\n");
			printf("  -window         : Enable windowed mode                      \n");
			printf("  -pal            : Force VJ to PAL mode (default: NTSC)      \n");
			printf("  -ntsc           : Force VJ to NTSC mode                     \n");
			printf("\nInvoking Virtual Jagaur with no ROM file will cause it to boot up\n");
			printf("with the VJ GUI.\n");
 			return 1;
		}
    }

	// Set up SDL library
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
//		| SDL_INIT_CDROM) < 0)
//		| SDL_INIT_CDROM | SDL_INIT_NOPARACHUTE) < 0)
	{
		WriteLog("VJ: Could not initialize the SDL library: %s\n", SDL_GetError());
		return -1;
	}

	WriteLog("VJ: SDL successfully initialized.\n");

WriteLog("Initializing memory subsystem...\n");
	InitMemory();
WriteLog("Initializing version...\n");
	InitVersion();
	version_display(log_get());
WriteLog("Initializing jaguar subsystem...\n");
	jaguar_init();

	// Get the BIOS ROM
/*#ifdef USE_BUILT_IN_BIOS
	WriteLog("VJ: Using built in BIOS/CD BIOS...\n");
	memcpy(jaguar_bootRom, jagBootROM, 0x20000);
	memcpy(jaguar_CDBootROM, jagCDROM, 0x40000);
	BIOSLoaded = CDBIOSLoaded = true;
#else
//	if (vjs.useJaguarBIOS)
// What would be nice here would be a way to check if the BIOS was loaded so that we
// could disable the pushbutton on the Misc Options menu... !!! FIX !!! [DONE here, but needs to be fixed in GUI as well!]
WriteLog("About to attempt to load BIOSes...\n");
	BIOSLoaded = (JaguarLoadROM(jaguar_bootRom, vjs.jagBootPath) == 0x20000 ? true : false);
	WriteLog("VJ: BIOS is %savailable...\n", (BIOSLoaded ? "" : "not "));
	CDBIOSLoaded = (JaguarLoadROM(jaguar_CDBootROM, vjs.CDBootPath) == 0x40000 ? true : false);
	WriteLog("VJ: CD BIOS is %savailable...\n", (CDBIOSLoaded ? "" : "not "));
#endif*/
	WriteLog("About to attempt to load BIOSes...\n");
	vjs.JaguarBIOSLoaded = (JaguarLoadROM(jaguar_bootRom, vjs.jagBootPath) == 0x20000 ? true : false);
	WriteLog("VJ: BIOS is %savailable...\n", (vjs.JaguarBIOSLoaded ? "" : "not "));
	vjs.CDBIOSLoaded = (JaguarLoadROM(jaguar_CDBootROM, vjs.CDBootPath) == 0x40000 ? true : false);
	WriteLog("VJ: CD BIOS is %savailable...\n", (vjs.CDBIOSLoaded ? "" : "not "));

	SET32(jaguar_mainRam, 0, 0x00200000);			// Set top of stack...

	WriteLog("Initializing video subsystem...\n");
	InitVideo();


	// Check for passed cartridge/program file
/*	if (argv[1])	//FileExists(argv[1]))
	{
		WriteLog("Loading file: %s\n", argv[1]);
		JaguarLoadFile(argv[1]);

		memcpy(jaguar_mainRam + 0x4000, jaguar_mainRom, 600000);
		jaguarRunAddress = 0x4000;
	}
	else*/
	{
		// Use CD bios ?
		if (vjs.CDBIOSLoaded)
		{
			// Copy CD bios to rom address
			memcpy(jaguar_mainRom, jaguar_CDBootROM, 0x40000);
			// Get run address of CD bios
			jaguarRunAddress = GET32(jaguar_mainRom, 0x404);
/*SET16(jaguar_mainRom,0x2214,0x4E71);
SET16(jaguar_mainRom,0x2216,0x4E71);
SET16(jaguar_mainRom,0x2218,0x4E71);
SET16(jaguar_mainRom,0x221A,0x4E71);
SET16(jaguar_mainRom,0x221C,0x4E71);
SET16(jaguar_mainRom,0x221E,0x4E71);
SET16(jaguar_mainRom,0x2220,0x4E71);

SET16(jaguar_mainRom,0x2222,0x4E71);
SET16(jaguar_mainRom,0x2224,0x4E71);
SET16(jaguar_mainRom,0x2226,0x4E71);

SET16(jaguar_mainRom,0x2228,0x4E71);
SET16(jaguar_mainRom,0x222A,0x4E71);
SET16(jaguar_mainRom,0x222C,0x4E71);

SET16(jaguar_mainRom,0x222E,0x4E71);

SET16(jaguar_mainRom,0x2230,0x4E71);
SET16(jaguar_mainRom,0x2232,0x4E71);
SET16(jaguar_mainRom,0x2234,0x4E71);
SET16(jaguar_mainRom,0x2236,0x6000);
*/

// move.l #$12345678,d1
//SET16(jaguar_mainRom,0x2222,0x223C);
//SET16(jaguar_mainRom,0x2224,0x1234);
//SET16(jaguar_mainRom,0x2226,0x5678);

//
// Something is missing during CD bios run.
// The encryption check routine returns $3D0DEAD.
// This means encryption is OK, right ?
// But the bios is looping to do the encryption check again and again.
// Setting the bus width in the bios (rom address $80240A) leads to the following:
// Bus width -> Encryption check result:
//     1   ->   $3D0DEAD -> jump back to encryption check -> loop
//     2   ->   $12345678 -> jump to $8023AA ($503AA)
//     4   ->   skipped -> jump to $8023AA ($503AA)
//
// For now patching the CD bios bus width
SET16(jaguar_mainRom,0x40A,0x0002);

//jaguar_mainRom_crc32 = crc32_calcCheckSum(jaguar_mainRom, 0x40000);
		}
		else
		{
			WriteLog("Initializing GUI subsystem...\n");
			InitGUI();
		}
	}

	// Now with crunchy GUI goodness!
//WriteLog("About to start GUI...\n");
//	GUIMain(haveCart ? argv[1] : NULL);

InitGUI();

//GUIMain(argv[1]);
GUIMain(NULL);	// No passed filename


//This is no longer accurate...!
//	int elapsedTime = clock() - startTime;
//	int fps = (1000 * totalFrames) / elapsedTime;
//	WriteLog("VJ: Ran at an average of %i FPS.\n", fps);

	jaguar_done();
	VersionDone();
	MemoryDone();
	VideoDone();
	log_done();	

	// Free SDL components last...!
//	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_CDROM);
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	SDL_Quit();

    return 0;
}
