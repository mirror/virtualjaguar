//
// app.h: Header file
//
// by James L. Hammons
// (C) 2009 Underground Software
//

#ifndef __APP_H__
#define __APP_H__

//Hrm. uh??? I thought this wasn't the way to do this stuff...???
#include <QtGui>

// Forward declarations
class MainWin;

class App: public QApplication
{
	public:
		App(int argc, char * argv[]);

	private:
		MainWin * mainWindow;

	// Globally accessible stuff goes here...
	// Although... Globally accessible stuff should go into settings.cpp...
//	public:
};

#endif	// __APP_H__
