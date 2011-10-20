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
class HelpWindow;
class FilePickerWindow;

class MainWin: public QMainWindow
{
	// All Qt apps require this macro for signal/slot functionality to work
	Q_OBJECT

	public:
		MainWin();

	protected:
		void closeEvent(QCloseEvent *);
		void keyPressEvent(QKeyEvent *);
		void keyReleaseEvent(QKeyEvent *);

	private slots:
		void Open(void);
 		void Configure(void);
		void Timer(void);
		void TogglePowerState(void);
		void ToggleRunState(void);
		void SetZoom100(void);
		void SetZoom200(void);
		void SetZoom300(void);
		void SetNTSC(void);
		void SetPAL(void);
		void ToggleBlur(void);
		void ShowAboutWin(void);
		void ShowHelpWin(void);
		void InsertCart(void);
		void Unpause(void);
		void LoadSoftware(QString);
		void ToggleCDUsage(void);

	private:
		void HandleKeys(QKeyEvent *, bool);
		void ResizeMainWindow(void);
		void ReadSettings(void);
		void WriteSettings(void);

//	public:
		GLWidget * videoWidget;
		AboutWindow * aboutWin;
		HelpWindow * helpWin;
		FilePickerWindow * filePickWin;
		QTimer * timer;
		bool running;
		int zoomLevel;
		bool powerButtonOn;
		bool showUntunedTankCircuit;
		bool cartridgeLoaded;
		bool allowUnknownSoftware;
		bool CDActive;
//		bool alpineLoadSuccessful;
		bool pauseForFileSelector;

		QMenu * fileMenu;
		QMenu * helpMenu;
		QToolBar * toolbar;

		QActionGroup * zoomActs;
		QActionGroup * tvTypeActs;

		QAction * quitAppAct;
		QAction * powerAct;
		QAction * pauseAct;
		QAction * x1Act;
		QAction * x2Act;
		QAction * x3Act;
		QAction * ntscAct;
		QAction * palAct;
		QAction * blurAct;
		QAction * aboutAct;
		QAction * helpAct;
		QAction * filePickAct;
		QAction * configAct;
		QAction * useCDAct;
		
		QIcon powerGreen;
		QIcon powerRed;
};

#endif	// __MAINWIN_H__
