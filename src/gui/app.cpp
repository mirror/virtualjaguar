//
// app.cpp - Qt-based GUI for Virtual Jaguar
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  12/23/2009  Created this file
// JLH  01/21/2011  Added SDL initialization
// JLH  06/26/2011  Added fix to keep SDL from hijacking main() on win32
//

#include "app.h"

#include <SDL.h>
#include <QApplication>
#include "log.h"
#include "mainwin.h"
#include "settings.h"
#include "types.h"

#ifdef __GCCWIN32__
// Apparently on win32, SDL is hijacking main from Qt. So let's do this:
#undef main
#endif

//hm. :-/
// This is stuff we pass into the mainWindow...
bool noUntunedTankPlease = false;
bool loadAndGo = false;
QString filename;

// Here's the main application loop--short and simple...
int main(int argc, char * argv[])
{
	// Normally, this would be read in from the settings module... :-P
	vjs.hardwareTypeAlpine = false;
	// This is stuff we pass into the mainWindow...
//	noUntunedTankPlease = false;

	if (argc > 1)
	{
		if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)
			|| (strcmp(argv[1], "-?") == 0))
		{
			printf("Virtual Jaguar 2.0.0 help\n");
			printf("\n");
			printf("Command line interface is mostly non-functional ATM, but may return if\n"
				"there is enough demand for it. :-)\n");
			return 0;
		}

		if (strcmp(argv[1], "--yarrr") == 0)
		{
			printf("\n");
			printf("Shiver me timbers!\n");
			printf("\n");
			return 0;
		}

		if ((strcmp(argv[1], "--alpine") == 0) || (strcmp(argv[1], "-a") == 0))
		{
			printf("Alpine Mode enabled.\n");
			vjs.hardwareTypeAlpine = true;
		}

		if (strcmp(argv[1], "--please-dont-kill-my-computer") == 0)
		{
			noUntunedTankPlease = true;
		}

		// Check for filename
		if (argv[1][0] != '-')
		{
			loadAndGo = true;
			filename = argv[1];
		}
	}

	Q_INIT_RESOURCE(virtualjaguar);	// This must the same name as the exe filename
//or is it the .qrc filename???
	// This is so we can pass this stuff using signal/slot mechanism...
//ick	int id = qRegisterMetaType<uint32>();

	bool success = (bool)LogInit("virtualjaguar.log");	// Init logfile
	int retVal = -1;							// Default is failure

	if (!success)
		printf("Failed to open virtualjaguar.log for writing!\n");

	// Set up SDL library
	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
	{
		WriteLog("VJ: Could not initialize the SDL library: %s\n", SDL_GetError());
	}
	else
	{
		WriteLog("VJ: SDL (joystick, audio) successfully initialized.\n");
		App app(argc, argv);					// Declare an instance of the application
		retVal = app.exec();					// And run it!

		// Free SDL components last...!
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
		SDL_Quit();
	}

	LogDone();									// Close logfile
	return retVal;
}

// Main app constructor--we stick globally accessible stuff here...

App::App(int argc, char * argv[]): QApplication(argc, argv)
{
	mainWindow = new MainWin(filename);
	mainWindow->plzDontKillMyComputer = noUntunedTankPlease;
	mainWindow->show();
}
