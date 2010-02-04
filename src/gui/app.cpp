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
//

#include "app.h"

#include <QApplication>
#include "mainwin.h"
#include "types.h"

// Here's the main application loop--short and simple...
int main(int argc, char * argv[])
{
	if (argc > 1)
	{
		if (strcmp(argv[1], "--help") == 0)
		{
			printf("Virtual Jaguar 2.0.0 help\n");
			printf("\n");
			printf("This is an experimental branch of Virtual Jaguar, how did you get it?\n");
			return 0;
		}
	}

	Q_INIT_RESOURCE(vj);	// This must the same name as the exe filename
//or is it the .qrc filename???
	// This is so we can pass this stuff using signal/slot mechanism...
//ick	int id = qRegisterMetaType<uint32>();

	App app(argc, argv);						// Declare an instance of the application

	return app.exec();							// And run it!
}

// Main app constructor--we stick globally accessible stuff here...

App::App(int argc, char * argv[]): QApplication(argc, argv)
{
	mainWindow = new MainWin();
	mainWindow->show();
}
