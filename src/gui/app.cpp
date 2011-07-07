//
// app.cpp - Qt-based GUI for Virtual Jaguar
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
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
#include "types.h"

#ifdef __GCCWIN32__
// Apparently on win32, SDL is hijacking main from Qt. So let's do this:
#undef main
#endif

// Here's the main application loop--short and simple...
int main(int argc, char * argv[])
{
	if (argc > 1)
	{
		if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)
			|| (strcmp(argv[1], "-?") == 0))
		{
			printf("Virtual Jaguar 2.0.0 help\n");
			printf("\n");
			printf("Command line interface is non-functional ATM, but may return if there is\n"
				"enough demand for it. :-)\n");
			return 0;
		}
	}

	Q_INIT_RESOURCE(virtualjaguar);	// This must the same name as the exe filename
//or is it the .qrc filename???
	// This is so we can pass this stuff using signal/slot mechanism...
//ick	int id = qRegisterMetaType<uint32>();

	LogInit("virtualjaguar.log");				// Init logfile
	int retVal = -1;							// Default is failure

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
	mainWindow = new MainWin();
	mainWindow->show();
}
