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
#include "about.h"
#include "settings.h"
#include "filepicker.h"

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

	ReadSettings();
	setUnifiedTitleAndToolBarOnMac(true);

	aboutWin = new AboutWindow(this);
	filePickWin = new FilePickerWindow(this);

    videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// Create actions

	quitAppAct = new QAction(tr("E&xit"), this);
	quitAppAct->setShortcuts(QKeySequence::Quit);
	quitAppAct->setStatusTip(tr("Quit Virtual Jaguar"));
	connect(quitAppAct, SIGNAL(triggered()), this, SLOT(close()));

	powerAct = new QAction(QIcon(":/res/power.png"), tr("&Power"), this);
	powerAct->setStatusTip(tr("Toggle running state"));
	powerAct->setCheckable(true);
	connect(powerAct, SIGNAL(triggered()), this, SLOT(ToggleRunState()));

	zoomActs = new QActionGroup(this);

	x1Act = new QAction(QIcon(":/res/zoom100.png"), tr("Zoom 100%"), zoomActs);
	x1Act->setStatusTip(tr("Set window zoom to 100%"));
	x1Act->setCheckable(true);
	connect(x1Act, SIGNAL(triggered()), this, SLOT(SetZoom100()));

	x2Act = new QAction(QIcon(":/res/zoom200.png"), tr("Zoom 200%"), zoomActs);
	x2Act->setStatusTip(tr("Set window zoom to 200%"));
	x2Act->setCheckable(true);
	connect(x2Act, SIGNAL(triggered()), this, SLOT(SetZoom200()));

	x3Act = new QAction(QIcon(":/res/zoom300.png"), tr("Zoom 300%"), zoomActs);
	x3Act->setStatusTip(tr("Set window zoom to 300%"));
	x3Act->setCheckable(true);
	connect(x3Act, SIGNAL(triggered()), this, SLOT(SetZoom300()));

	tvTypeActs = new QActionGroup(this);

	ntscAct = new QAction(QIcon(":/res/generic.png"), tr("NTSC"), tvTypeActs);
	ntscAct->setStatusTip(tr("Sets OpenGL rendering to GL_NEAREST"));
	ntscAct->setCheckable(true);
	connect(ntscAct, SIGNAL(triggered()), this, SLOT(SetNTSC()));

	palAct = new QAction(QIcon(":/res/generic.png"), tr("PAL"), tvTypeActs);
	palAct->setStatusTip(tr("Sets OpenGL rendering to GL_NEAREST"));
	palAct->setCheckable(true);
	connect(palAct, SIGNAL(triggered()), this, SLOT(SetPAL()));

	blurAct = new QAction(QIcon(":/res/generic.png"), tr("Blur"), this);
	blurAct->setStatusTip(tr("Sets OpenGL rendering to GL_NEAREST"));
	blurAct->setCheckable(true);
	connect(blurAct, SIGNAL(triggered()), this, SLOT(ToggleBlur()));

	aboutAct = new QAction(QIcon(":/res/generic.png"), tr("&About..."), this);
	aboutAct->setStatusTip(tr("Blatant self-promotion"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(ShowAboutWin()));

	filePickAct = new QAction(QIcon(":/res/generic.png"), tr("Insert Cartridge..."), this);
	filePickAct->setStatusTip(tr("Insert a cartridge into Virtual Jaguar"));
	connect(filePickAct, SIGNAL(triggered()), this, SLOT(InsertCart()));

	// Create menus & toolbars

	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(filePickAct);
	fileMenu->addAction(powerAct);
	fileMenu->addAction(quitAppAct);

	fileMenu = menuBar()->addMenu(tr("&Help"));
	fileMenu->addAction(aboutAct);

	QToolBar * toolbar = addToolBar(tr("Stuff"));
	toolbar->addAction(powerAct);
	toolbar->addAction(x1Act);
	toolbar->addAction(x2Act);
	toolbar->addAction(x3Act);
	toolbar->addAction(ntscAct);
	toolbar->addAction(palAct);
	toolbar->addAction(blurAct);

	//	Create status bar
	statusBar()->showMessage(tr("Ready"));

	// Set toolbar button based on setting read in (sync the UI)...
	blurAct->setChecked(vjs.glFilter);
	x1Act->setChecked(zoomLevel == 1);
	x2Act->setChecked(zoomLevel == 2);
	x3Act->setChecked(zoomLevel == 3);
	running = powerAct->isChecked();
	ntscAct->setChecked(vjs.hardwareTypeNTSC);
	palAct->setChecked(!vjs.hardwareTypeNTSC);

	// Do this in case original size isn't correct (mostly for the first-run case)
	ResizeMainWindow();

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
	zoomLevel = 1;
	ResizeMainWindow();
}

void MainWin::SetZoom200(void)
{
	zoomLevel = 2;
	ResizeMainWindow();
}

void MainWin::SetZoom300(void)
{
	zoomLevel = 3;
	ResizeMainWindow();
}

void MainWin::SetNTSC(void)
{
	vjs.hardwareTypeNTSC = true;
	ResizeMainWindow();
}

void MainWin::SetPAL(void)
{
	vjs.hardwareTypeNTSC = false;
	ResizeMainWindow();
}

void MainWin::ToggleBlur(void)
{
	vjs.glFilter = !vjs.glFilter;
}

void MainWin::ShowAboutWin(void)
{
	aboutWin->show();
}

void MainWin::InsertCart(void)
{
	filePickWin->show();
}

void MainWin::ResizeMainWindow(void)
{
	videoWidget->setFixedSize(zoomLevel * 320, zoomLevel * (vjs.hardwareTypeNTSC ? 240 : 256));
	show();

	for(int i=0; i<2; i++)
	{
		resize(0, 0);
		usleep(2000);
		QApplication::processEvents();
	}
}

void MainWin::ReadSettings(void)
{
	QSettings settings("Underground Software", "Virtual Jaguar");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
	resize(size);
	move(pos);

	zoomLevel = settings.value("zoom", 1).toInt();

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

	// Hardcoded, !!! FIX !!!
	strcpy(vjs.ROMPath, "./roms");
}

void MainWin::WriteSettings(void)
{
	QSettings settings("Underground Software", "Virtual Jaguar");
	settings.setValue("pos", pos());
	settings.setValue("size", size());

	settings.setValue("zoom", zoomLevel);

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

// Here's how Byuu does it...
// I think I have it working now... :-)
#if 0
void Utility::resizeMainWindow()
{
  unsigned region = config().video.context->region;
  unsigned multiplier = config().video.context->multiplier;
  unsigned width = 256 * multiplier;
  unsigned height = (region == 0 ? 224 : 239) * multiplier;

  if(config().video.context->correctAspectRatio)
  {
    if(region == 0)
	{
      width = (double)width * config().video.ntscAspectRatio + 0.5;  //NTSC adjust
    }
	else
	{
      width = (double)width * config().video.palAspectRatio  + 0.5;  //PAL adjust
    }
  }

  if(config().video.isFullscreen == false)
  {
    //get effective desktop work area region (ignore Windows taskbar, OS X dock, etc.)
    QRect deskRect = QApplication::desktop()->availableGeometry(mainWindow);

    //ensure window size will not be larger than viewable desktop area
    constrainSize(height, width, deskRect.height()); //- frameHeight);
    constrainSize(width, height, deskRect.width());  //- frameWidth );

    mainWindow->canvas->setFixedSize(width, height);
    mainWindow->show();
  }
  else
  {
    for(unsigned i = 0; i < 2; i++)
	{
      unsigned iWidth = width, iHeight = height;

      constrainSize(iHeight, iWidth, mainWindow->canvasContainer->size().height());
      constrainSize(iWidth, iHeight, mainWindow->canvasContainer->size().width());

      //center canvas onscreen; ensure it is not larger than viewable area
      mainWindow->canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      mainWindow->canvas->setFixedSize(iWidth, iHeight);
      mainWindow->canvas->setMinimumSize(0, 0);

      usleep(2000);
      QApplication::processEvents();
    }
  }

  //workaround for Qt/Xlib bug:
  //if window resize occurs with cursor over it, Qt shows Qt::Size*DiagCursor;
  //so force it to show Qt::ArrowCursor, as expected
  mainWindow->setCursor(Qt::ArrowCursor);
  mainWindow->canvasContainer->setCursor(Qt::ArrowCursor);
  mainWindow->canvas->setCursor(Qt::ArrowCursor);

  //workaround for DirectSound(?) bug:
  //window resizing sometimes breaks audio sync, this call re-initializes it
  updateAvSync();
}

void Utility::setScale(unsigned scale)
{
  config().video.context->multiplier = scale;
  resizeMainWindow();
  mainWindow->shrink();
  mainWindow->syncUi();
}

void QbWindow::shrink()
{
  if(config().video.isFullscreen == false)
  {
    for(unsigned i = 0; i < 2; i++)
	{
      resize(0, 0);
      usleep(2000);
      QApplication::processEvents();
    }
  }
}
#endif
