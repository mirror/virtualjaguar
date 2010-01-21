//
// mainwin.h: Header file
//
// by James L. Hammons
// (C) 2010 Underground Software
//

#ifndef __MAINWIN_H__
#define __MAINWIN_H__

//Hrm. uh??? I thought this wasn't the way to do this stuff...???
#include <QtGui>

// Forward declarations

class GLWidget;
//class EditWindow;
//class CharWindow;

class MainWin: public QMainWindow
{
	// All Qt apps require this macro for signal/slot functionality to work
	Q_OBJECT

	public:
		MainWin();

	protected:
		void closeEvent(QCloseEvent * event);

	private slots:
		void Open(void);
		void Timer(void);
		void ToggleRunState(void);
		void SetZoom100(void);
		void SetZoom200(void);
		void SetZoom300(void);
		void ToggleBlur(void);

	private:
		void ReadSettings(void);
		void WriteSettings(void);

//	public:
		GLWidget * videoWidget;
		QTimer * timer;
		bool running;
//		EditWindow * editWnd;
//		CharWindow * charWnd;
		QActionGroup * zoomActs;

		QAction * action;
		QAction * quitAppAct;
		QAction * x1Act;
		QAction * x2Act;
		QAction * x3Act;
		QAction * blurAct;
};

#endif	// __MAINWIN_H__
