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

#include "jaguar.h"
#include "video.h"
#include "tom.h"
#include "log.h"
#include "file.h"

// Uncomment this to use built-in BIOS/CD-ROM BIOS
// You'll need a copy of jagboot.h & jagcd.h for this to work...!
//#define USE_BUILT_IN_BIOS

// Uncomment this for an official Virtual Jaguar release
//#define VJ_RELEASE_VERSION "2.0.0"
#warning !!! FIX !!! Figure out how to use this in GUI.CPP as well!

#ifdef USE_BUILT_IN_BIOS
#include "jagboot.h"
#include "jagcd.h"
#endif

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
	ntscAct->setStatusTip(tr("Sets Jaguar to NTSC mode"));
	ntscAct->setCheckable(true);
	connect(ntscAct, SIGNAL(triggered()), this, SLOT(SetNTSC()));

	palAct = new QAction(QIcon(":/res/generic.png"), tr("PAL"), tvTypeActs);
	palAct->setStatusTip(tr("Sets Jaguar to PAL mode"));
	palAct->setCheckable(true);
	connect(palAct, SIGNAL(triggered()), this, SLOT(SetPAL()));

	blurAct = new QAction(QIcon(":/res/generic.png"), tr("Blur"), this);
	blurAct->setStatusTip(tr("Sets OpenGL rendering to GL_NEAREST"));
	blurAct->setCheckable(true);
	connect(blurAct, SIGNAL(triggered()), this, SLOT(ToggleBlur()));

	aboutAct = new QAction(QIcon(":/res/generic.png"), tr("&About..."), this);
	aboutAct->setStatusTip(tr("Blatant self-promotion"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(ShowAboutWin()));

	filePickAct = new QAction(QIcon(":/res/generic.png"), tr("&Insert Cartridge..."), this);
	filePickAct->setStatusTip(tr("Insert a cartridge into Virtual Jaguar"));
	connect(filePickAct, SIGNAL(triggered()), this, SLOT(InsertCart()));

	// Create menus & toolbars

	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(filePickAct);
	fileMenu->addAction(powerAct);
	fileMenu->addAction(quitAppAct);

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);

	toolbar = addToolBar(tr("Stuff"));
	toolbar->addAction(powerAct);
	toolbar->addSeparator();
	toolbar->addAction(x1Act);
	toolbar->addAction(x2Act);
	toolbar->addAction(x3Act);
	toolbar->addSeparator();
	toolbar->addAction(ntscAct);
	toolbar->addAction(palAct);
	toolbar->addSeparator();
	toolbar->addAction(blurAct);

	//	Create status bar
	statusBar()->showMessage(tr("Ready"));

	// Set toolbar buttons/menus based on settings read in (sync the UI)...
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

#ifdef VJ_RELEASE_VERSION
	WriteLog("Virtual Jaguar %s (Last full build was on %s %s)\n", VJ_RELEASE_VERSION, __DATE__, __TIME__);
#else
	WriteLog("Virtual Jaguar SVN %s (Last full build was on %s %s)\n", __DATE__, __DATE__, __TIME__);
#endif
	WriteLog("Initializing jaguar subsystem...\n");
	JaguarInit();

	// Get the BIOS ROM
#ifdef USE_BUILT_IN_BIOS
	WriteLog("VJ: Using built in BIOS/CD BIOS...\n");
	memcpy(jaguarBootROM, jagBootROM, 0x20000);
	memcpy(jaguarCDBootROM, jagCDROM, 0x40000);
	BIOSLoaded = CDBIOSLoaded = true;
#else
// What would be nice here would be a way to check if the BIOS was loaded so that we
// could disable the pushbutton on the Misc Options menu... !!! FIX !!! [DONE here, but needs to be fixed in GUI as well!]
WriteLog("About to attempt to load BIOSes...\n");
#if 1
//This is short-circuiting the file finding thread... ??? WHY ???
	BIOSLoaded = (JaguarLoadROM(jaguarBootROM, vjs.jagBootPath) == 0x20000 ? true : false);
#else
	BIOSLoaded = false;
#endif
	WriteLog("VJ: BIOS is %savailable...\n", (BIOSLoaded ? "" : "not "));
	CDBIOSLoaded = (JaguarLoadROM(jaguarCDBootROM, vjs.CDBootPath) == 0x40000 ? true : false);
	WriteLog("VJ: CD BIOS is %savailable...\n", (CDBIOSLoaded ? "" : "not "));
#endif

	SET32(jaguarMainRAM, 0, 0x00200000);			// Set top of stack...

//This is crappy!!! !!! FIX !!!
//Is this even needed any more? Hmm. Maybe. Dunno.
//Seems like it is... But then again, maybe not. Have to test it to see.
WriteLog("GUI: Resetting Jaguar...\n");
	JaguarReset();

}

void MainWin::closeEvent(QCloseEvent * event)
{
	WriteSettings();
	event->accept(); // ignore() if can't close for some reason
}

void MainWin::Open(void)
{
}

//
// Here's the main emulator loop
//
void MainWin::Timer(void)
{
	if (!running)
		return;

#if 0
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
#else
	JaguarExecuteNew();
//	memcpy(videoWidget->buffer, backbuffer, videoWidget->rasterHeight * videoWidget->rasterWidth);
	memcpy(videoWidget->buffer, backbuffer, videoWidget->rasterHeight * videoWidget->textureWidth);
//	memcpy(surface->pixels, backbuffer, TOMGetVideoModeWidth() * TOMGetVideoModeHeight() * 4);
#endif

	videoWidget->updateGL();
}

#if 0
Window * RunEmu(void)
{
//	extern uint32 * backbuffer;
	uint32 * overlayPixels = (uint32 *)sdlemuGetOverlayPixels();
	memset(overlayPixels, 0x00, 640 * 480 * 4);			// Clear out overlay...

//This is crappy... !!! FIX !!!
//	extern bool finished, showGUI;

	sdlemuDisableOverlay();

//	uint32 nFrame = 0, nFrameskip = 0;
	uint32 totalFrames = 0;
	finished = false;
	bool showMessage = true;
	uint32 showMsgFrames = 120;
	uint8 transparency = 0xFF;
	// Pass a message to the "joystick" code to debounce the ESC key...
	debounceRunKey = true;

	uint32 cartType = 4;
	if (jaguarRomSize == 0x200000)
		cartType = 0;
	else if (jaguarRomSize == 0x400000)
		cartType = 1;
	else if (jaguarMainRomCRC32 == 0x687068D5)
		cartType = 2;
	else if (jaguarMainRomCRC32 == 0x55A0669C)
		cartType = 3;

	const char * cartTypeName[5] = { "2M Cartridge", "4M Cartridge", "CD BIOS", "CD Dev BIOS", "Homebrew" };
	uint32 elapsedTicks = SDL_GetTicks(), frameCount = 0, framesPerSecond = 0;

	while (!finished)
	{
		// Set up new backbuffer with new pixels and data
		JaguarExecuteNew();
		totalFrames++;
//WriteLog("Frame #%u...\n", totalFrames);
//extern bool doDSPDis;
//if (totalFrames == 373)
//	doDSPDis = true;

//Problem: Need to do this *only* when the state changes from visible to not...
//Also, need to clear out the GUI when not on (when showMessage is active...)
if (showGUI || showMessage)
	sdlemuEnableOverlay();
else
	sdlemuDisableOverlay();

//Add in a new function for clearing patches of screen (ClearOverlayRect)

// Also: Take frame rate into account when calculating fade time...

		// Some QnD GUI stuff here...
		if (showGUI)
		{
			FillScreenRectangle(overlayPixels, 8, 1*FONT_HEIGHT, 128, 4*FONT_HEIGHT, 0x00000000);
			extern uint32 gpu_pc, dsp_pc;
			DrawString(overlayPixels, 8, 1*FONT_HEIGHT, false, "GPU PC: %08X", gpu_pc);
			DrawString(overlayPixels, 8, 2*FONT_HEIGHT, false, "DSP PC: %08X", dsp_pc);
			DrawString(overlayPixels, 8, 4*FONT_HEIGHT, false, "%u FPS", framesPerSecond);
		}

		if (showMessage)
		{
			DrawString2(overlayPixels, 8, 24*FONT_HEIGHT, 0x007F63FF, transparency, "Running...");
			DrawString2(overlayPixels, 8, 26*FONT_HEIGHT, 0x001FFF3F, transparency, "%s, run address: %06X", cartTypeName[cartType], jaguarRunAddress);
			DrawString2(overlayPixels, 8, 27*FONT_HEIGHT, 0x001FFF3F, transparency, "CRC: %08X", jaguarMainRomCRC32);

			if (showMsgFrames == 0)
			{
				transparency--;

				if (transparency == 0)
{
					showMessage = false;
/*extern bool doGPUDis;
doGPUDis = true;//*/
}

			}
			else
				showMsgFrames--;
		}

		frameCount++;

		if (SDL_GetTicks() - elapsedTicks > 250)
			elapsedTicks += 250, framesPerSecond = frameCount * 4, frameCount = 0;
	}

	// Save the background for the GUI...
	// In this case, we squash the color to monochrome, then force it to blue + green...
	for(uint32 i=0; i<TOMGetVideoModeWidth() * 256; i++)
	{
		uint32 pixel = backbuffer[i];
		uint8 b = (pixel >> 16) & 0xFF, g = (pixel >> 8) & 0xFF, r = pixel & 0xFF;
		pixel = ((r + g + b) / 3) & 0x00FF;
		backbuffer[i] = 0xFF000000 | (pixel << 16) | (pixel << 8);
	}

	sdlemuEnableOverlay();

	return NULL;
}
#endif

void MainWin::ToggleRunState(void)
{
	running = !running;

	if (!running)
	{
#if 0
		for(uint32_t x=0; x<videoWidget->rasterWidth; x++)
			for(uint32_t y=0; y<videoWidget->rasterHeight; y++)
				videoWidget->buffer[(y * videoWidget->textureWidth) + x] = 0x00000000;
#else
//		for(uint32_t i=0; i<TOMGetVideoModeWidth() * 256; i++)
		for(uint32_t i=0; i<videoWidget->textureWidth * 256; i++)
		{
			uint32_t pixel = backbuffer[i];
//			uint8_t b = (pixel >> 16) & 0xFF, g = (pixel >> 8) & 0xFF, r = pixel & 0xFF;
			uint8_t r = (pixel >> 24) & 0xFF, g = (pixel >> 16) & 0xFF, b = (pixel >> 8) & 0xFF;
			pixel = ((r + g + b) / 3) & 0x00FF;
//			backbuffer[i] = 0xFF000000 | (pixel << 16) | (pixel << 8);
			backbuffer[i] = 0x000000FF | (pixel << 16) | (pixel << 8);
		}

//		memcpy(videoWidget->buffer, backbuffer, videoWidget->rasterHeight * videoWidget->rasterWidth);
		memcpy(videoWidget->buffer, backbuffer, videoWidget->rasterHeight * videoWidget->textureWidth);
#endif

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
	strcpy(vjs.jagBootPath, settings.value("JagBootROM", "./bios/jagboot.rom").toString().toAscii().data());
	strcpy(vjs.CDBootPath, settings.value("CDBootROM", "./bios/jagcd.rom").toString().toAscii().data());
	strcpy(vjs.EEPROMPath, settings.value("EEPROMs", "./eeproms").toString().toAscii().data());
	strcpy(vjs.ROMPath, settings.value("ROMs", "./software").toString().toAscii().data());
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
	settings.setValue("JagBootROM", vjs.jagBootPath);
	settings.setValue("CDBootROM", vjs.CDBootPath);
	settings.setValue("EEPROMs", vjs.EEPROMPath);
	settings.setValue("ROMs", vjs.ROMPath);
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
