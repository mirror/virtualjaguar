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
// JLH  12/20/2010  Added settings, menus & toolbars
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

#include "mainwin.h"

//#include <QtGui>
//#include <QtOpenGL>
#include "glwidget.h"
#include "settings.h"

// The way BSNES controls things is by setting a timer with a zero
// timeout, sleeping if not emulating anything. Seems there has to be a
// better way.

// It has a novel approach to plugging-in/using different video/audio/input
// methods, can we do something similar or should we just use the built-in
// QOpenGL?

// We're going to try to use the built-in OpenGL support and see how it goes.
// We'll make the VJ core modular so that it doesn't matter what GUI is in
// use, we can drop it in anywhere and use it as-is.

MainWin::MainWin()
{
	videoWidget = new GLWidget(this);
	setCentralWidget(videoWidget);
	setWindowIcon(QIcon(":/res/vj.xpm"));
	setWindowTitle("Virtual Jaguar v2.0.0");

	// Create actions

	quitAppAct = new QAction(tr("E&xit"), this);
	quitAppAct->setShortcuts(QKeySequence::Quit);
	quitAppAct->setStatusTip(tr("Quit Virtual Jaguar"));
	connect(quitAppAct, SIGNAL(triggered()), this, SLOT(close()));

	action = new QAction(QIcon(":/res/power.png"), tr("&Power"), this);
	action->setStatusTip(tr("Toggle running state"));
	action->setCheckable(true);
	connect(action, SIGNAL(triggered()), this, SLOT(ToggleRunState()));

	zoomActs = new QActionGroup(this);

	x1Act = new QAction(QIcon(":/res/zoom100.png"), tr("Zoom 100%"), zoomActs);
	x1Act->setStatusTip(tr("Set window zoom to 100%"));
	x1Act->setCheckable(true);
//	connect(x1Act, SIGNAL(triggered()), this, SLOT(???()));

	x2Act = new QAction(QIcon(":/res/zoom200.png"), tr("Zoom 200%"), zoomActs);
	x2Act->setStatusTip(tr("Set window zoom to 200%"));
	x2Act->setCheckable(true);
//	connect(x2Act, SIGNAL(triggered()), this, SLOT(???()));

	x3Act = new QAction(QIcon(":/res/zoom300.png"), tr("Zoom 300%"), zoomActs);
	x3Act->setStatusTip(tr("Set window zoom to 300%"));
	x3Act->setCheckable(true);
//	connect(x3Act, SIGNAL(triggered()), this, SLOT(???()));

	blurAct = new QAction(QIcon(":/res/generic.png"), tr("Blur"), this);
	blurAct->setStatusTip(tr("Sets OpenGL rendering to GL_NEAREST"));
	blurAct->setCheckable(true);
	connect(blurAct, SIGNAL(triggered()), this, SLOT(ToggleBlur()));

	// Create menus & toolbars

	QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(action);
	fileMenu->addAction(quitAppAct);

	QToolBar * toolbar = addToolBar(tr("Stuff"));
	toolbar->addAction(action);
	toolbar->addAction(x1Act);
	toolbar->addAction(x2Act);
	toolbar->addAction(x3Act);
	toolbar->addAction(blurAct);
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
	setUnifiedTitleAndToolBarOnMac(true);

	// Set toolbar button based on setting read in (sync the UI)...
	blurAct->setChecked(vjs.glFilter);
	x1Act->setChecked(true);
	running = action->isChecked();

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
	if (!running)
		return;

	// Random hash & trash
	// We try to simulate an untuned tank circuit here... :-)
	for(uint32_t x=0; x<videoWidget->rasterWidth; x++)
	{
		for(uint32_t y=0; y<videoWidget->rasterHeight; y++)
		{
			videoWidget->buffer[(y * videoWidget->textureWidth) + x] = (rand() & 0xFF) << 8 | (rand() & 0xFF) << 16 | (rand() & 0xFF) << 24;// | (rand() & 0xFF);//0x000000FF;
//			buffer[(y * textureWidth) + x] = x*y;
		}
	}

	videoWidget->updateGL();
}

void MainWin::ToggleRunState(void)
{
	running = !running;

	if (!running)
	{
		for(uint32_t x=0; x<videoWidget->rasterWidth; x++)
			for(uint32_t y=0; y<videoWidget->rasterHeight; y++)
				videoWidget->buffer[(y * videoWidget->textureWidth) + x] = 0x00000000;

		videoWidget->updateGL();
	}
}

void MainWin::SetZoom100(void)
{
}

void MainWin::SetZoom200(void)
{
}

void MainWin::SetZoom300(void)
{
}

void MainWin::ToggleBlur(void)
{
	vjs.glFilter = !vjs.glFilter;
}

void MainWin::ReadSettings(void)
{
	QSettings settings("Underground Software", "Virtual Jaguar");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
	resize(size);
	move(pos);

	vjs.useJoystick      = settings.value("useJoystick", false).toBool();
	vjs.joyport          = settings.value("joyport", 0).toInt();
	vjs.hardwareTypeNTSC = settings.value("hardwareTypeNTSC", true).toBool();
	vjs.frameSkip        = settings.value("frameSkip", 0).toInt();
	vjs.useJaguarBIOS    = settings.value("useJaguarBIOS", false).toBool();
	vjs.DSPEnabled       = settings.value("DSPEnabled", false).toBool();
	vjs.usePipelinedDSP  = settings.value("usePipelinedDSP", false).toBool();
	vjs.fullscreen       = settings.value("fullscreen", false).toBool();
	vjs.useOpenGL        = settings.value("useOpenGL", true).toBool();
	vjs.glFilter         = settings.value("glFilterType", 0).toInt();
	vjs.renderType       = settings.value("renderType", 0).toInt();
}

void MainWin::WriteSettings(void)
{
	QSettings settings("Underground Software", "Virtual Jaguar");
	settings.setValue("pos", pos());
	settings.setValue("size", size());

	settings.setValue("useJoystick", vjs.useJoystick);
	settings.setValue("joyport", vjs.joyport);
	settings.setValue("hardwareTypeNTSC", vjs.hardwareTypeNTSC);
	settings.setValue("frameSkip", vjs.frameSkip);
	settings.setValue("useJaguarBIOS", vjs.useJaguarBIOS);
	settings.setValue("DSPEnabled", vjs.DSPEnabled);
	settings.setValue("usePipelinedDSP", vjs.usePipelinedDSP);
	settings.setValue("fullscreen", vjs.fullscreen);
	settings.setValue("useOpenGL", vjs.useOpenGL);
	settings.setValue("glFilterType", vjs.glFilter);
	settings.setValue("renderType", vjs.renderType);
}

