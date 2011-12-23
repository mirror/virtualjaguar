//
// SETTINGS.H: Header file
//

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

// MAX_PATH isn't defined in stdlib.h on *nix, so we do it here...
#ifdef __GCCUNIX__
#include <limits.h>
#define MAX_PATH		_POSIX_PATH_MAX
#else
#ifdef _MSC_VER									// for Microsoft Visual Studio
#pragma warning(disable:4996)					// Avoid deprecated warnings
#define MAX_PATH _MAX_PATH
#define strcasecmp stricmp
//#define __USE_W32_SOCKETS
#else
#include <stdlib.h>								// for MAX_PATH on MinGW/Darwin
#endif
#endif
#include "types.h"

// Settings struct

struct VJSettings
{
	bool useJoystick;
	int32 joyport;								// Joystick port
	bool hardwareTypeNTSC;						// Set to false for PAL
	//bool useJaguarBIOS;
	bool JaguarBIOSLoaded;
	bool CDBIOSLoaded;
	int16 CDDrive;
	int16 ErrorRetry;
	bool DSPEnabled;
	bool usePipelinedDSP;
	bool fullscreen;
	bool useOpenGL;
	uint32 glFilter;
	bool hardwareTypeAlpine;
	uint32 frameSkip;
	uint32 renderType;
	
	// Keybindings in order of U, D, L, R, C, B, A, Op, Pa, 0-9, #, *

	uint16 p1KeyBindings[21];
	uint16 p2KeyBindings[21];

	// Paths

	char ROMPath[MAX_PATH];
	char jagBootPath[MAX_PATH];
	char CDBootPath[MAX_PATH];
	char EEPROMPath[MAX_PATH];
};

// Render types

enum { RT_NORMAL = 0, RT_TV = 1 };

// Exported functions

void LoadVJSettings(void);
void SaveVJSettings(void);

// Exported variables

extern VJSettings vjs;

#endif	// __SETTINGS_H__
