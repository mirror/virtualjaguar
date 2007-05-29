////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef __PORT__
#include "stdafx.h"
#include <mmsystem.h>
#include <direct.h>
#endif	// #ifndef __PORT__
#include <time.h>

/* Added by SDLEMU (http://sdlemu.ngemu.com) */
/* Added for GCC UNIX compatibility          */
#ifdef __GCCUNIX__
#include <unistd.h>
#endif	// #ifdef __GCCUNIX__

#include <SDL.h>
#include "SDLptc.h"
#include "jaguar.h"

/* Enable this (uncomment) for speed control */
//#define SPEED_CONTROL

Surface * surface;
Format format(16, 0x007C00, 0x00003E0, 0x0000001F);
uint8 finished = 0;
Console console;

int fullscreen = 0;
char * jaguar_boot_dir;

void main_screen_switch(void)
{
	fullscreen = !fullscreen;
	if (fullscreen)
		console.option("fullscreen output");
	else
		console.option("windowed output");
	console.close();
	console.open("WIP VERSION", tom_width, tom_height, format);
}

#ifdef __PORT__

/* Added/changed by SDLEMU http://sdlemu.ngemu.com */

int main(int argc, char * argv[])
{

	uint32		 startTime, totalFrames, endTime, w, h;
	int32        *vs;
	unsigned int nNormalLast = 0;
	int			 nNormalFrac = 0; 
	int			 i           = 0;
    int          nTime       = 0;
    int          nCount      = 0;
    int          nFrameskip  = 1; /* Frameskip */
    int          nFrame      = 0; /* No. of Frame */
    int          nJoyport    = 0; /* Joystick port */

	printf("Virtual Jaguar/SDL v1.0.1 (GCC/SDL Port)\n");
	printf("Based upon Virtual Jaguar core v1.0.0 by Potato emulation.\n");
	printf("Written by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)\n");
	printf("Portions massaged by James L. Hammons (WIN32)\n");
	printf("Contact : http://sdlemu.ngemu.com/ | sdlemu@ngemu.com\n");

	if (argc <= 1)
	{
	    printf("Usage : \n\n");
		printf("jag_em <romfile> [switches]  ]\n");
		printf("                  -fullscreen     : Enable fullscreen mode           \n");
		printf("                  -window         : Enable windowed   mode (default) \n");
		printf("                  -frameskip 1-10 : Enable frameskip 1 (default) - 10\n");
		printf("                  -joystick       : Enable joystick/gamepad          \n");
		printf("                  -joyport   0-3  : Select desired joystick port     \n");
 		return true;
	}

	jaguar_boot_dir=(char*)malloc(1024);
	getcwd(jaguar_boot_dir,1024);

	log_init("jag_em.log");

	memory_init();
	version_init();
	version_display(log_get());
	jaguar_init(argv[1]);
	jaguar_reset();
	
    /* Setting up the backbuffer */
	int16 *backbuffer=(int16*)malloc(845*525*sizeof(int16));
	memset(backbuffer,0xaa,tom_getVideoModeWidth()*tom_getVideoModeHeight()*sizeof(int16));

	/* Setting up the primary SDL display */
	Format format(16,0x007c00,0x00003e0,0x0000001f);
	surface=new Surface(tom_getVideoModeWidth(),tom_getVideoModeHeight(), format);
    console.option("windowed output");

    /* Checking the switches ;) */
	for(i = 0; (i < argc || argv[i] != NULL); i++)
	{
	   if(!strcmp(argv[i], "-fullscreen")) 
	       console.option("fullscreen output");

	   if(!strcmp(argv[i], "-window")) 
	       console.option("windowed output");

	   if(!strcmp(argv[i], "-joystick")) 
	       console.option("joystick enabled");

	   if(!strcmp(argv[i], "-joyport"))
	   {
	    	nJoyport = atoi(argv[++i]) + 1;
			if(nJoyport > 3) nJoyport = 3;
	   }

	   if(!strcmp(argv[i], "-frameskip"))
	   {
	    	nFrameskip = atoi(argv[++i]) + 1;
			if(nFrameskip > 10) nFrameskip = 10;
			#ifdef SPEED_CONTROL
			   nFrameskip = 0;
            #endif
	   }


	       
    }
    

	/* Initialize Joystick support under SDL */
	if ( console.JoyEnabled() == 1 ) {
	
	if(SDL_NumJoysticks() <= 0)
	{
        console.option("joystick disabled");
		printf("No joystick(s) or joypad(s) detected on your system. Using keyboards....\n");
	}
	else {
	  if((console.joystick = SDL_JoystickOpen(nJoyport)) == 0)
	  {
        console.option("joystick disabled");
		printf("Unable to open a Joystick on port : %d\n", nJoyport);
	  }
	  else
	   printf("Using: %s\n", SDL_JoystickName(nJoyport));
	}
	}

	/* Open the display and start emulating some l337 Atari Jaguar games :P */
	console.open("Virtual Jaguar",tom_getVideoModeWidth(),tom_getVideoModeHeight(),format);

	
	totalFrames=0;
	startTime=clock();
	nNormalLast=0;// Last value of timeGetTime()
	nNormalFrac=0; // Extra fraction we did

	nNormalLast=SDL_GetTicks();//timeGetTime();
	while (!finished)
	{
#ifdef SPEED_CONTROL
		nTime=SDL_GetTicks()-nNormalLast;					  // calcule le temps écoulé depuis le dernier affichage
															  // nTime est en mili-secondes.
		// détermine le nombre de trames à passer + 1
		nCount=(nTime*600 - nNormalFrac) /10000; 	

		// si le nombre de trames à passer + 1 est nul ou négatif,
		// ne rien faire pendant 2 ms
		if (nCount<=0) 
		{ 
			//Sleep(2); 
			//SDL_Delay(1);
		} // No need to do anything for a bit
		else
		{
			nNormalFrac+=nCount*10000;				// 
			nNormalLast+=nNormalFrac/600;				// add the duration of nNormalFrac frames
			nNormalFrac%=600;							// 

			// Pas plus de 9 (10-1) trames non affichées 
			if (nCount>10) 
			  nCount=10; 
			for (i=0;i<nCount-1;i++) 
				jaguar_exec(backbuffer,0);
#endif
            /* Setting up new backbuffer with new pixels and data */
			jaguar_exec(backbuffer,1);
			totalFrames++;

			/* Simple frameskip */
			if ( nFrame == nFrameskip ) {
               w=tom_width;
	           h=tom_height;
			   vs = (int32 *)surface->lock();
			   memcpy(vs,backbuffer,w*h*2);
			   surface->unlock();
			   surface->copy(console);
			   console.update();
			   nFrame = 0;
			}
            else
               nFrame++;
			joystick_exec();
			
#ifdef SPEED_CONTROL
		}
#endif
	}
	int elapsedTime=clock()-startTime;
	int fps=(1000*totalFrames)/elapsedTime;
	printf("statistics: %i fps\n",fps);
	
	if ( console.JoyEnabled() == 1 ) {}
 
	jaguar_done();
	version_done();
	memory_done();
	log_done();	
	console.close(); // Close SDL items as last!
    return 0;
}

#else

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	uint32		 startTime, endTime, totalFrames;
	unsigned int nNormalLast=0;
	int			 nNormalFrac=0; 
	int			 nTime=0,nCount=0; int i=0;

	jaguar_boot_dir=(char*)malloc(1024);
	_getcwd(jaguar_boot_dir,1024);

	log_init("jagem.log");
	// 15 bits RGB555
	SDL_Event	app_input_event;

	memory_init();
	version_init();
	version_display(log_get());
	jaguar_init();
	jaguar_reset();
	

	int16 * backbuffer = (int16 *)malloc(845 * 525 * sizeof(int16));
	memset(backbuffer, 0xAA, tom_getVideoModeWidth() * tom_getVideoModeHeight() * sizeof(int16));
	surface=new Surface(tom_getVideoModeWidth(),tom_getVideoModeHeight(),format);
	console.option("DirectX");
	console.option("windowed output");
//	console.option("fullscreen output");
	console.option("center window");
	console.open("WIP VERSION",tom_getVideoModeWidth(),tom_getVideoModeHeight(),format);
	uint32 surfacePitch=(surface->pitch()>>1);

	totalFrames=0;
	startTime=clock();
	nNormalLast=0;// Last value of timeGetTime()
	nNormalFrac=0; // Extra fraction we did
	nNormalLast=timeGetTime();
	while (!finished)
	{
		while ( SDL_PollEvent(&app_input_event) )
		{
			if ( app_input_event.type == SDL_QUIT )
			{
				finished = 1;
			}
		}
		joystick_exec();
#define SPEED_CONTROL
#ifdef SPEED_CONTROL
		nTime=timeGetTime()-nNormalLast;					  // calcule le temps écoulé depuis le dernier affichage
															  // nTime est en mili-secondes.
		// détermine le nombre de trames à passer + 1
		nCount=(nTime*600 - nNormalFrac) /10000; 	

		// si le nombre de trames à passer + 1 est nul ou négatif,
		// ne rien faire pendant 2 ms
		if (nCount<=0) 
		{ 
			Sleep(2); 
		} // No need to do anything for a bit
		else
		{
			nNormalFrac+=nCount*10000;				// 
			nNormalLast+=nNormalFrac/600;				// add the duration of nNormalFrac frames
			nNormalFrac%=600;							// 

			// Pas plus de 9 (10-1) trames non affichées 
			if (nCount>10) 
			  nCount=10; 
			for (i=0;i<nCount-1;i++) 
				jaguar_exec(backbuffer,0);
#endif
			jaguar_exec(backbuffer,1);
			totalFrames++;

			int32 *vs = (int32 *)surface->lock();
			uint32 w,h;
			w=tom_width;
			h=tom_height;

			memcpy(vs,backbuffer,w*h*2);
			surface->unlock();
			surface->copy(console);

			console.update();
#ifdef SPEED_CONTROL
		}
#endif
	}
	int elapsedTime=clock()-startTime;
	int fps=(1000*totalFrames)/elapsedTime;
	fprintf(log_get(),"statistics: %i fps\n",fps);
	
	console.close();

	jaguar_done();
	version_done();
	memory_done();
	log_done();	
    return 0;
    
}

#endif

