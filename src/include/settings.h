//
// SETTINGS.H: Header file
//

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

// MAX_PATH isn't defined in stdlib.h on *nix, so we do it here...
#ifdef __GCCUNIX__
#include <limits.h>
#define MAX_PATH		_POSIX_PATH_MAX
#endif

// Settings struct

struct VJSettings
{
	bool useJoystick;
	int32 joyport;									// Joystick port
	bool hardwareTypeNTSC;							// Set to false for PAL
	bool useJaguarBIOS;
	bool DSPEnabled;
	bool usePipelinedDSP;
	bool fullscreen;
	bool useOpenGL;
	uint32 glFilter;
	bool hardwareTypeAlpine;

	// Keybindings in order of U, D, L, R, C, B, A, Op, Pa, 0-9, #, *
	uint16 p1KeyBindings[21];
	uint16 p2KeyBindings[21];

	// Paths
	char ROMPath[MAX_PATH];
	char jagBootPath[MAX_PATH];
	char CDBootPath[MAX_PATH];
	char EEPROMPath[MAX_PATH];

	// Internal global stuff
//	uint32 ROMType;
};

// ROM Types
//enum { RT_CARTRIDGE, RT_

// Exported functions

void LoadVJSettings(void);
void SaveVJSettings(void);

// Exported variables

extern VJSettings vjs;

#endif	// __SETTINGS_H__
