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
class AboutWindow;
class FilePickerWindow;

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
		void SetNTSC(void);
		void SetPAL(void);
		void ToggleBlur(void);
		void ShowAboutWin(void);
		void InsertCart(void);

	private:
		void ResizeMainWindow(void);
		void ReadSettings(void);
		void WriteSettings(void);

//	public:
		GLWidget * videoWidget;
		AboutWindow * aboutWin;
		FilePickerWindow * filePickWin;
		QTimer * timer;
		bool running;
		int zoomLevel;

		QMenu * fileMenu;
		QMenu * helpMenu;
		QToolBar * toolbar;

		QActionGroup * zoomActs;
		QActionGroup * tvTypeActs;

		QAction * quitAppAct;
		QAction * powerAct;
		QAction * x1Act;
		QAction * x2Act;
		QAction * x3Act;
		QAction * ntscAct;
		QAction * palAct;
		QAction * blurAct;
		QAction * aboutAct;
		QAction * filePickAct;
};

#endif	// __MAINWIN_H__
