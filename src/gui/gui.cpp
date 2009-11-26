//
// GUI.CPP
//
// Graphical User Interface support
// by James L. Hammons
//

#include "gui.h"

#include <SDL.h>
#include "crc32.h"
#include "file.h"
#include "jaguar.h"
#include "log.h"
#include "sdlemu_opengl.h"
#include "settings.h"
#include "tom.h"
#include "video.h"

// Once these are split off, these may not be needed anymore...
#include "button.h"
#include "element.h"
#include "filelist.h"
#include "guimisc.h"
#include "image.h"
#include "listbox.h"
#include "menu.h"
#include "pushbutton.h"
#include "slideswitch.h"
#include "text.h"
#include "textedit.h"
#include "window.h"

// Private function prototypes

Window * LoadROM(void);
Window * ResetJaguar(void);
Window * ResetJaguarCD(void);
Window * RunEmu(void);
Window * Quit(void);
Window * About(void);
Window * MiscOptions(void);

// Local global variables

bool showGUI = false;
bool exitGUI = false;								// GUI (emulator) done variable
int mouseX = 0, mouseY = 0;
uint32 background[1280 * 256];						// GUI background buffer
bool showMessage = false;
//uint32 showMessageTimeout;
//char messageBuffer[200];
bool finished = false;

//
// GUI stuff--it's not crunchy, it's GUI! ;-)
//

void GUIInit(void)
{
	SDL_ShowCursor(SDL_DISABLE);
	SDL_GetMouseState(&mouseX, &mouseY);
}

void GUIDone(void)
{
}

//
// GUI main loop
//
//bool GUIMain(void)
bool GUIMain(char * filename)
{
WriteLog("GUI: Inside GUIMain...\n");

	uint32 pointerBGSave[6 * 8 + 2];
	pointerBGSave[0] = 6;
	pointerBGSave[1] = 8;

// Need to set things up so that it loads and runs a file if given on the command line. !!! FIX !!! [DONE]
//	extern uint32 * backbuffer;
//	bool done = false;
	SDL_Event event;
	Window * mainWindow = NULL;

	// Set up the GUI classes...
//	Element::SetScreenAndPitch(backbuffer, GetSDLScreenWidthInPixels());
	Element::SetScreenAndPitch((uint32 *)sdlemuGetOverlayPixels(), sdlemuGetOverlayWidthInPixels());
	sdlemuEnableOverlay();

	Menu mainMenu;
	MenuItems mi;
	mi.title = "Jaguar";
	mi.item.push_back(NameAction("Load...", LoadROM, SDLK_l));
	mi.item.push_back(NameAction("Reset", ResetJaguar, SDLK_r));
	if (CDBIOSLoaded)
		mi.item.push_back(NameAction("Reset CD", ResetJaguarCD, SDLK_c));
	mi.item.push_back(NameAction("Run", RunEmu, SDLK_ESCAPE));
	mi.item.push_back(NameAction(""));
	mi.item.push_back(NameAction("Quit", Quit, SDLK_q));
	mainMenu.Add(mi);
	mi.title = "Settings";
	mi.item.clear();
	mi.item.push_back(NameAction("Video..."));
	mi.item.push_back(NameAction("Audio..."));
	mi.item.push_back(NameAction("Misc...", MiscOptions, SDLK_m));
	mainMenu.Add(mi);
	mi.title = "Info";
	mi.item.clear();
	mi.item.push_back(NameAction("About...", About));
	mainMenu.Add(mi);

	bool showMouse = true;

	// Grab the BG where the mouse will be painted (prime the backstore)

/*
DISNOWOK
Bitmap ptr = { 6, 8, 4,
""//"000011112222333344445555"
//"000011112222333344445555"
//"000011112222333344445555"
//"000011112222333344445555"
//"000011112222333344445555"
//"000011112222333344445555"
//"000011112222333344445555"
//"000011112222333344445555"
};//*/
	uint32 * overlayPixels = (uint32 *)sdlemuGetOverlayPixels();
	uint32 count = 2;

	for(uint32 y=0; y<pointerBGSave[1]; y++)
		for(uint32 x=0; x<pointerBGSave[0]; x++)
			pointerBGSave[count++] = overlayPixels[((mouseY + y) * sdlemuGetOverlayWidthInPixels()) + (mouseX + x)];

	uint32 oldMouseX = mouseX, oldMouseY = mouseY;

//This is crappy!!! !!! FIX !!!
//Is this even needed any more? Hmm. Maybe. Dunno.
WriteLog("GUI: Resetting Jaguar...\n");
	JaguarReset();

WriteLog("GUI: Clearing BG save...\n");
	// Set up our background save...
//	memset(background, 0x11, tom_getVideoModeWidth() * 240 * 2);
//1111 -> 000100 01000 10001 -> 0001 0000 0100 0010 1000 1100 -> 10 42 8C
	for(uint32 i=0; i<TOMGetVideoModeWidth()*240; i++)
//		background[i] = 0xFF8C4210;
		backbuffer[i] = 0xFF8C4210;

/*	uint32 * overlayPix = (uint32 *)sdlemuGetOverlayPixels();
	for(uint32 i=0; i<sdlemuGetOverlayWidthInPixels()*480; i++)
		overlayPix[i] = 0x00000000;*/

	// Handle loading file passed in on the command line...! [DONE]

	if (filename)
	{
		if (JaguarLoadFile(filename))
		{
//			event.type = SDL_USEREVENT, event.user.code = MENU_ITEM_CHOSEN;
//			event.user.data1 = (void *)ResetJaguar;
//	    	SDL_PushEvent(&event);
			// Make it so that if passed in on the command line, we quit right
			// away when pressing ESC
WriteLog("GUI: Bypassing GUI since ROM passed in on command line...\n");
			ResetJaguar();
			return true;
		}
		else
		{
			// Create error dialog...
			char errText[1024];
			sprintf(errText, "The file %40s could not be loaded.", filename);

			mainWindow = new Window(8, 16, 304, 160);
			mainWindow->AddElement(new Text(8, 8, "Error!"));
			mainWindow->AddElement(new Text(8, 24, errText));
		}
	}

WriteLog("GUI: Entering main loop...\n");
	while (!exitGUI)
	{
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_USEREVENT)
			{
				if (event.user.code == WINDOW_CLOSE)
				{
					delete mainWindow;
					mainWindow = NULL;
				}
				else if (event.user.code == MENU_ITEM_CHOSEN)
				{
					// Confused? Let me enlighten... What we're doing here is casting
					// data1 as a pointer to a function which returns a Window pointer and
					// which takes no parameters (the "(Window *(*)(void))" part), then
					// derefencing it (the "*" in front of that) in order to call the
					// function that it points to. Clear as mud? Yeah, I hate function
					// pointers too, but what else are you gonna do?
					mainWindow = (*(Window *(*)(void))event.user.data1)();

					while (SDL_PollEvent(&event));	// Flush the event queue...
					event.type = SDL_MOUSEMOTION;
					int mx, my;
					SDL_GetMouseState(&mx, &my);
					event.motion.x = mx, event.motion.y = my;
				    SDL_PushEvent(&event);			// & update mouse position...!

					oldMouseX = mouseX, oldMouseY = mouseY;
					mouseX = mx, mouseY = my;		// This prevents "mouse flash"...
				}
			}
			else if (event.type == SDL_ACTIVEEVENT)
			{
				if (event.active.state == SDL_APPMOUSEFOCUS)
					showMouse = (event.active.gain ? true : false);
			}
			else if (event.type == SDL_KEYDOWN)
			{
// Ugly kludge for windowed<-->fullscreen switching...
uint8 * keystate = SDL_GetKeyState(NULL);

if ((keystate[SDLK_LALT] || keystate[SDLK_RALT]) & keystate[SDLK_RETURN])
	ToggleFullscreen();

				if (mainWindow)
					mainWindow->HandleKey(event.key.keysym.sym);
				else
					mainMenu.HandleKey(event.key.keysym.sym);
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				oldMouseX = mouseX, oldMouseY = mouseY;
				mouseX = event.motion.x, mouseY = event.motion.y;

				if (mainWindow)
					mainWindow->HandleMouseMove(mouseX, mouseY);
				else
					mainMenu.HandleMouseMove(mouseX, mouseY);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				uint32 mx = event.button.x, my = event.button.y;

				if (mainWindow)
					mainWindow->HandleMouseButton(mx, my, true);
				else
					mainMenu.HandleMouseButton(mx, my, true);
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				uint32 mx = event.button.x, my = event.button.y;

				if (mainWindow)
					mainWindow->HandleMouseButton(mx, my, false);
				else
					mainMenu.HandleMouseButton(mx, my, false);
			}

//PROBLEM: In order to use the dirty rectangle approach here, we need some way of
//         handling it in mainMenu.Draw() and mainWindow->Draw(). !!! FIX !!!
//POSSIBLE SOLUTION:
// When mouse is moving and not on menu or window, can do straight dirty rect.
// When mouse is on menu, need to update screen. Same for buttons on windows...
// What the menu & windows should do is only redraw on a state change. IOW, they
// should call their own/child window's Draw() function instead of doing it top
// level.
//#define NEW_BACKSTORE_METHOD

			// Draw the GUI...
// The way we do things here is kinda stupid (redrawing the screen every frame), but
// it's simple. Perhaps there may be a reason down the road to be more selective with
// our clearing, but for now, this will suffice.
//			memset(backbuffer, 0x11, tom_getVideoModeWidth() * 240 * 2);
//			memcpy(backbuffer, background, tom_getVideoModeWidth() * 256 * 2);
//			memcpy(backbuffer, background, tom_getVideoModeWidth() * 256 * 4);
#ifndef NEW_BACKSTORE_METHOD
			memset(sdlemuGetOverlayPixels(), 0, sdlemuGetOverlayWidthInPixels() * 480 * 4);

			mainMenu.Draw();
//Could do multiple windows here by using a vector + priority info...
//Though the way ZSNES does it seems to be by a bool (i.e., they're always active, just not shown)
			if (mainWindow)
				mainWindow->Draw();
#endif

/*uint32 pBGS[6 * 8 + 3] = { 6, 8, 4,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0
};*/
//This isn't working... Why????
//It's because DrawTransparentBitmap does alpha blending if it detects zero in the alpha channel.
//So why do it that way? Hm.
			overlayPixels = (uint32 *)sdlemuGetOverlayPixels();

#ifdef NEW_BACKSTORE_METHOD
//			DrawTransparentBitmapDeprecated(overlayPixels, oldMouseX, oldMouseY, pointerBGSave);
//			DrawTransparentBitmap(overlayPixels, oldMouseX, oldMouseY, pBGS);
			for(uint32 y=0; y<pointerBGSave[1]; y++)
				for(uint32 x=0; x<pointerBGSave[0]; x++)
					overlayPixels[((oldMouseY + y) * sdlemuGetOverlayWidthInPixels()) + (oldMouseX + x)] = 0x00000000;

			count = 2;

			for(uint32 y=0; y<pointerBGSave[1]; y++)
				for(uint32 x=0; x<pointerBGSave[0]; x++)
					pointerBGSave[count++] = overlayPixels[((mouseY + y) * sdlemuGetOverlayWidthInPixels()) + (mouseX + x)];
#endif

			if (showMouse)
//				DrawTransparentBitmapDeprecated(backbuffer, mouseX, mouseY, mousePic);
				DrawTransparentBitmapDeprecated(overlayPixels, mouseX, mouseY, mousePic);

			RenderBackbuffer();
		}
	}

	return true;
}

//
// GUI "action" functions
//

Window * LoadROM(void)
{
	FileList * fileList = new FileList(20, 20, 600, 440);

	return (Window *)fileList;
}

Window * ResetJaguar(void)
{
	JaguarReset();

	return RunEmu();
}

Window * ResetJaguarCD(void)
{
	memcpy(jaguarMainROM, jaguarCDBootROM, 0x40000);
	jaguarRunAddress = 0x802000;
	jaguarMainROMCRC32 = crc32_calcCheckSum(jaguarMainROM, 0x40000);
	JaguarReset();
//This is a quick kludge to get the CDBIOS to boot properly...
//Wild speculation: It could be that this memory location is wired into the CD unit
//somehow, which lets it know whether or not a cart is present in the unit...
	jaguarMainROM[0x0040B] = 0x03;

	return RunEmu();
}


#if 0

bool debounceRunKey = true;
Window * RunEmu(void)
{
	extern uint32 * backbuffer;
//Temporary, to test the new timer based code...
sdlemuDisableOverlay();
JaguarExecuteNew();
sdlemuEnableOverlay();
	// Save the background for the GUI...
	// In this case, we squash the color to monochrome, then force it to blue + green...
	for(uint32 i=0; i<tom_getVideoModeWidth() * 256; i++)
	{
		uint32 pixel = backbuffer[i];
		uint8 b = (pixel >> 16) & 0xFF, g = (pixel >> 8) & 0xFF, r = pixel & 0xFF;
		pixel = ((r + g + b) / 3) & 0x00FF;
		backbuffer[i] = 0xFF000000 | (pixel << 16) | (pixel << 8);
	}
return NULL;//*/

//This is crappy... !!! FIX !!!
	extern bool finished, showGUI;

//	uint32 nFrame = 0, nFrameskip = 0;
	uint32 totalFrames = 0;
	finished = false;
	bool showMessage = true;
	uint32 showMsgFrames = 120;
	uint8 transparency = 0;
	// Pass a message to the "joystick" code to debounce the ESC key...
	debounceRunKey = true;

	uint32 cartType = 4;
	if (jaguarRomSize == 0x200000)
		cartType = 0;
	else if (jaguarRomSize == 0x400000)
		cartType = 1;
	else if (jaguar_mainRom_crc32 == 0x687068D5)
		cartType = 2;
	else if (jaguar_mainRom_crc32 == 0x55A0669C)
		cartType = 3;

	char * cartTypeName[5] = { "2M Cartridge", "4M Cartridge", "CD BIOS", "CD Dev BIOS", "Homebrew" };
	uint32 elapsedTicks = SDL_GetTicks(), frameCount = 0, framesPerSecond = 0;

	while (true)
	{
		// Set up new backbuffer with new pixels and data
		JaguarExecute(backbuffer, true);
//		JaguarExecuteNew();
		totalFrames++;
//WriteLog("Frame #%u...\n", totalFrames);
//extern bool doDSPDis;
//if (totalFrames == 373)
//	doDSPDis = true;

//This sucks... !!! FIX !!!
		joystick_exec();
//This is done here so that the crud below doesn't get on our GUI background...
		if (finished)
			break;

		// Some QnD GUI stuff here...
		if (showGUI)
		{
			extern uint32 gpu_pc, dsp_pc;
			DrawString(backbuffer, 8, 8, false, "GPU PC: %08X", gpu_pc);
			DrawString(backbuffer, 8, 16, false, "DSP PC: %08X", dsp_pc);
			DrawString(backbuffer, 8, 32, false, "%u FPS", framesPerSecond);
		}

		if (showMessage)
		{
// FF0F -> 1111 11 11  000 0 1111 -> 3F 18 0F
// 3FE3 -> 0011 11 11  111 0 0011 -> 0F 3F 03
/*			DrawStringTrans((uint32 *)backbuffer, 8, 24*8, 0xFF0F, transparency, "Running...");
			DrawStringTrans((uint32 *)backbuffer, 8, 26*8, 0x3FE3, transparency, "%s, run address: %06X", cartTypeName[cartType], jaguarRunAddress);
			DrawStringTrans((uint32 *)backbuffer, 8, 27*8, 0x3FE3, transparency, "CRC: %08X", jaguar_mainRom_crc32);//*/
//first has wrong color. !!! FIX !!!
			DrawStringTrans(backbuffer, 8, 24*8, 0xFF7F63FF, transparency, "Running...");
			DrawStringTrans(backbuffer, 8, 26*8, 0xFF1FFF3F, transparency, "%s, run address: %06X", cartTypeName[cartType], jaguarRunAddress);
			DrawStringTrans(backbuffer, 8, 27*8, 0xFF1FFF3F, transparency, "CRC: %08X", jaguar_mainRom_crc32);

			if (showMsgFrames == 0)
			{
				transparency++;

				if (transparency == 33)
{
					showMessage = false;
/*extern bool doGPUDis;
doGPUDis = true;//*/
}

			}
			else
				showMsgFrames--;
		}

		RenderBackbuffer();
		frameCount++;

		if (SDL_GetTicks() - elapsedTicks > 250)
			elapsedTicks += 250, framesPerSecond = frameCount * 4, frameCount = 0;
	}

	// Reset the pitch, since it may have been changed in-game...
	Element::SetScreenAndPitch((uint32 *)backbuffer, GetSDLScreenWidthInPixels());

	// Save the background for the GUI...
//	memcpy(background, backbuffer, tom_getVideoModeWidth() * 240 * 2);
	// In this case, we squash the color to monochrome, then force it to blue + green...
	for(uint32 i=0; i<tom_getVideoModeWidth() * 256; i++)
	{
		uint32 pixel = backbuffer[i];
		uint8 b = (pixel >> 16) & 0xFF, g = (pixel >> 8) & 0xFF, r = pixel & 0xFF;
		pixel = ((r + g + b) / 3) & 0x00FF;
		background[i] = 0xFF000000 | (pixel << 16) | (pixel << 8);
	}

	return NULL;
}

#else

bool debounceRunKey = true;
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
	if (jaguarROMSize == 0x200000)
		cartType = 0;
	else if (jaguarROMSize == 0x400000)
		cartType = 1;
	else if (jaguarMainROMCRC32 == 0x687068D5)
		cartType = 2;
	else if (jaguarMainROMCRC32 == 0x55A0669C)
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
			DrawString2(overlayPixels, 8, 27*FONT_HEIGHT, 0x001FFF3F, transparency, "CRC: %08X", jaguarMainROMCRC32);

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


Window * Quit(void)
{
	WriteLog("GUI: Quitting due to user request.\n");
	exitGUI = true;

	return NULL;
}

Window * About(void)
{
	char buf[512];
	sprintf(buf, "SVN %s", __DATE__);
//fprintf(fp, "VirtualJaguar v1.0.8 (Last full build was on %s %s)\n", __DATE__, __TIME__);
//VirtualJaguar v1.0.8 (Last full build was on Dec 30 2004 20:01:31)
//Hardwired, bleh... !!! FIX !!!
uint32 width = 55 * FONT_WIDTH, height = 18 * FONT_HEIGHT;
uint32 xpos = (640 - width) / 2, ypos = (480 - height) / 2;
//	Window * window = new Window(8, 16, 50 * FONT_WIDTH, 21 * FONT_HEIGHT);
	Window * window = new Window(xpos, ypos, width, height);
//	window->AddElement(new Text(8, 8, "Virtual Jaguar 1.0.8"));
//	window->AddElement(new Text(8, 8, "Virtual Jaguar CVS 20050110", 0xFF3030FF, 0xFF000000));
//	window->AddElement(new Text(208, 8+0*FONT_HEIGHT, buf, 0xFF3030FF, 0xFF000000));
	window->AddElement(new Text(248, 8+4*FONT_HEIGHT+5, buf, 0xFF3030FF, 0xFF000000));
	window->AddElement(new Text(8, 8+0*FONT_HEIGHT, "Coders:"));
	window->AddElement(new Text(16, 8+1*FONT_HEIGHT, "James L. Hammons (shamus)"));
	window->AddElement(new Text(16, 8+2*FONT_HEIGHT, "Niels Wagenaar (nwagenaar)"));
	window->AddElement(new Text(16, 8+3*FONT_HEIGHT, "Carwin Jones (Caz)"));
	window->AddElement(new Text(16, 8+4*FONT_HEIGHT, "Adam Green"));
	window->AddElement(new Text(8, 8+6*FONT_HEIGHT, "Testers:"));
	window->AddElement(new Text(16, 8+7*FONT_HEIGHT, "Guruma"));
	window->AddElement(new Text(8, 8+9*FONT_HEIGHT, "Thanks go out to:"));
	window->AddElement(new Text(16, 8+10*FONT_HEIGHT, "Aaron Giles for the original CoJag"));
	window->AddElement(new Text(16, 8+11*FONT_HEIGHT, "David Raingeard for the original VJ"));
	window->AddElement(new Text(16, 8+12*FONT_HEIGHT, "Karl Stenerud for his Musashi 68K emu"));
	window->AddElement(new Text(16, 8+13*FONT_HEIGHT, "Sam Lantinga for his amazing SDL libs"));
	window->AddElement(new Text(16, 8+14*FONT_HEIGHT, "Ryan C. Gordon for VJ's web presence"));
	window->AddElement(new Text(16, 8+15*FONT_HEIGHT, "Curt Vendel for various Jaguar goodies"));
	window->AddElement(new Text(16, 8+16*FONT_HEIGHT, "The guys over at Atari Age ;-)"));
//	window->AddElement(new Image(8, 8, &vj_title_small));
//	window->AddElement(new Image(width - (vj_title_small.width + 8), 8, &vj_title_small));
	window->AddElement(new Image(width - (((Bitmap *)vj_title_small)->width + 8), 8, &vj_title_small));

	return window;
}

Window * MiscOptions(void)
{
	Window * window = new Window(8, 16, 304, 192);
	window->AddElement(new PushButton(8, 8, &vjs.useJaguarBIOS, "BIOS"));
	window->AddElement(new SlideSwitch(8, 32, &vjs.hardwareTypeNTSC, "PAL", "NTSC"));
	window->AddElement(new PushButton(8, 64, &vjs.DSPEnabled, "DSP"));
	window->AddElement(new SlideSwitch(24, 88, &vjs.usePipelinedDSP, "Original", "Pipelined"));
	window->AddElement(new SlideSwitch(8, 120, (bool *)&vjs.glFilter, "Sharp", "Blurry"));
	window->AddElement(new SlideSwitch(8, 152, (bool *)&vjs.renderType, "Normal render", "TV style"));

	window->AddElement(new TextEdit(88, 8, vjs.ROMPath, 20, 0xFF8484FF, 0xFF000000));

/*TextEdit(uint32 x, uint32 y, string s, uint32 mss = 10, uint32 fg = 0xFF8484FF,
	uint32 bg = 0xFF84FF4D): Element(x, y, 0, 0), fgColor(fg), bgColor(bg), text(s),
	caretPos(0), maxScreenSize(mss) {}*/
// Missing:
// * BIOS path
// * ROM path
// * EEPROM path
// * joystick
// * joystick port
// * OpenGL?
// * GL Filter type
// * Window/fullscreen
// * Key definitions

	return window;
}

// Function prototype
Window * CrashGracefullyCallback(void);

//NOTE: Probably should set a flag as well telling it to do a full reset
//      of the Jaguar hardware if this happens...
void GUICrashGracefully(const char * reason)
{
	finished = true;							// We're finished for now!

	// Since this is used in the menu code as well, we could create another
	// internal function called "PushWindowOnQueue" or somesuch
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = MENU_ITEM_CHOSEN;
	event.user.data1 = (void *)CrashGracefullyCallback;
	SDL_PushEvent(&event);
}

Window * CrashGracefullyCallback(void)
{
	Window * window = new Window(8, 16, 304, 192);

	window->AddElement(new Text(8, 8+0*FONT_HEIGHT, "We CRASHED!!!"));

	return window;
}
