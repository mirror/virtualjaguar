//
// mainwin.cpp - Qt-based GUI for Virtual Jaguar: Main Application Window
// by James L. Hammons
// (C) 2009 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  12/23/2009  Created this file
//

// FIXED:
//
//
// STILL TO BE DONE:
//
//

// Uncomment this for debugging...
//#define DEBUG
//#define DEBUGFOO			// Various tool debugging...
//#define DEBUGTP				// Toolpalette debugging...

//#include <QtGui>
#include "mainwin.h"

//#include <QtOpenGL>
#include "glwidget.h"
//#include "editwindow.h"
//#include "charwindow.h"
//#include "ttedit.h"

MainWin::MainWin()
{
	// The way BSNES controls things is by setting a timer with a zero
	// timeout, sleeping if not emulating anything. Seems there has to be a
	// better way.

	// It has a novel approach to plugging-in/using different video/audio/input
	// methods, can we do something similar or should we just use the built-in
	// QOpenGL?

//	((TTEdit *)qApp)->charWnd = new CharWindow(this);
	videoWidget = new GLWidget(this);
	setCentralWidget(videoWidget);
	setWindowIcon(QIcon(":/res/vj.xpm"));
	setWindowTitle("Virtual Jaguar v2.0.0");

#if 0
//	createActions();
	newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create a new file"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

	openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

//	createMenus();
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(cutAct);
	editMenu->addAction(copyAct);
	editMenu->addAction(pasteAct);

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);

//	createToolBars();
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(newAct);
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(cutAct);
	editToolBar->addAction(copyAct);
	editToolBar->addAction(pasteAct);
#endif

	//	Create status bar
	statusBar()->showMessage(tr("Ready"));

	ReadSettings();

//	connect(textEdit->document(), SIGNAL(contentsChanged()),
//			this, SLOT(documentWasModified()));

//	setCurrentFile("");
	setUnifiedTitleAndToolBarOnMac(true);

	// Set up timer based loop for animation...
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(Timer()));
	timer->start(20);
}

void MainWin::closeEvent(QCloseEvent * event)
{
	WriteSettings();
	event->accept(); // ignore() if can't close for some reason
}

void MainWin::Open(void)
{
}

void MainWin::Timer(void)
{
	// Random hash & trash
	// We try to simulate an untuned tank circuit here... :-)
	for(int x=0; x<videoWidget->rasterWidth; x++)
	{
		for(int y=0; y<videoWidget->rasterHeight; y++)
		{
			videoWidget->buffer[(y * videoWidget->textureWidth) + x] = (rand() & 0xFF) << 8 | (rand() & 0xFF) << 16 | (rand() & 0xFF) << 24;// | (rand() & 0xFF);//0x000000FF;
//			buffer[(y * textureWidth) + x] = x*y;
		}
	}

	videoWidget->updateGL();
}

void MainWin::ReadSettings(void)
{
	QSettings settings("Underground Software", "Virtual Jaguar");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
	resize(size);
	move(pos);
//videoWidget->updateGL();
//	pos = settings.value("charWndPos", QPoint(0, 0)).toPoint();
//	size = settings.value("charWndSize", QSize(200, 200)).toSize();
//	((TTEdit *)qApp)->charWnd->resize(size);
//	((TTEdit *)qApp)->charWnd->move(pos);
}

void MainWin::WriteSettings(void)
{
	QSettings settings("Underground Software", "Virtual Jaguar");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
//	settings.setValue("charWndPos", ((TTEdit *)qApp)->charWnd->pos());
//	settings.setValue("charWndSize", ((TTEdit *)qApp)->charWnd->size());
}

