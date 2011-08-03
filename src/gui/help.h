//
// help.h: Built-in help system
//
// by James L. Hammons
// (C) 2011 Underground Software
//

#ifndef __HELP_H__
#define __HELP_H__

#include <QtGui>

class HelpWindow: public QWidget
{
	public:
		HelpWindow(QWidget * parent = 0);

	private:
		QVBoxLayout * layout;
		QTextBrowser * text;
//		QLabel * text;
//		QLabel * image;
};

#endif	// __HELP_H__
