#ifndef __PORT__
#include "include/stdafx.h"
#include <mmsystem.h>
#endif
#include <time.h>
#include <SDL.h>
#include "include/SDLptc.h"
#include "include/jaguar.h"

void main_screen_switch(void);


#define BUTTON_U		0
#define BUTTON_D		1
#define BUTTON_L		2
#define BUTTON_R		3
#define BUTTON_1		4
#define BUTTON_4		5
#define BUTTON_7		6
#define BUTTON_s		7
#define BUTTON_0		8
#define BUTTON_8		9
#define BUTTON_5		10
#define BUTTON_2		11
#define BUTTON_d		12
#define BUTTON_9		13
#define BUTTON_6		14
#define BUTTON_3		15


#define BUTTON_A		16
#define BUTTON_B		17
#define BUTTON_C		18
#define BUTTON_OPTION	19
#define BUTTON_PAUSE	20

static uint8 joystick_ram[4];
static uint8 joypad_0_buttons[21];
static uint8 joypad_1_buttons[21];
extern uint8 finished;
extern int start_logging;

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void joystick_init(void)
{
	joystick_reset();
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void joystick_exec(void)
{

	uint8    *keystate = SDL_GetKeyState(NULL);
	Sint16    x = 0, y = 0;
	extern   Console console;
  	
	memset(joypad_0_buttons,0,21);
	memset(joypad_1_buttons,0,21);

	if ((keystate[SDLK_LALT])&(keystate[SDLK_RETURN]))	main_screen_switch();

	/* Added/Changed by SDLEMU (http://sdlemu.ngemu.com) */

	if (keystate[SDLK_UP])		joypad_0_buttons[BUTTON_U]=0x01;
	if (keystate[SDLK_DOWN])	joypad_0_buttons[BUTTON_D]=0x01;
	if (keystate[SDLK_LEFT])	joypad_0_buttons[BUTTON_L]=0x01;
	if (keystate[SDLK_RIGHT])	joypad_0_buttons[BUTTON_R]=0x01;
	if (keystate[SDLK_z])		joypad_0_buttons[BUTTON_A]=0x01;
	if (keystate[SDLK_x])		joypad_0_buttons[BUTTON_B]=0x01;
	if (keystate[SDLK_c])		joypad_0_buttons[BUTTON_C]=0x01;
	if (keystate[SDLK_TAB])		joypad_0_buttons[BUTTON_OPTION]=0x01;
	if (keystate[SDLK_RETURN])	joypad_0_buttons[BUTTON_PAUSE]=0x01;
	if (keystate[SDLK_q])		start_logging=1;
	if (keystate[SDLK_w])		gpu_reset_stats();
//f (keystate[SDLK_u])		jaguar_long_write(0xf1c384,jaguar_long_read(0xf1c384)+1);

	if (keystate[SDLK_KP0])		joypad_0_buttons[BUTTON_0]=0x01;
	if (keystate[SDLK_KP1])		joypad_0_buttons[BUTTON_1]=0x01;
	if (keystate[SDLK_KP2])		joypad_0_buttons[BUTTON_2]=0x01;
	if (keystate[SDLK_KP3])		joypad_0_buttons[BUTTON_3]=0x01;
	if (keystate[SDLK_KP4])		joypad_0_buttons[BUTTON_4]=0x01;
	if (keystate[SDLK_KP5])		joypad_0_buttons[BUTTON_5]=0x01;
	if (keystate[SDLK_KP6])		joypad_0_buttons[BUTTON_6]=0x01;
	if (keystate[SDLK_KP7])		joypad_0_buttons[BUTTON_7]=0x01;
	if (keystate[SDLK_KP8])		joypad_0_buttons[BUTTON_8]=0x01;
	if (keystate[SDLK_KP9])		joypad_0_buttons[BUTTON_9]=0x01;
	
    if (keystate[SDLK_ESCAPE])	finished=1;
    
    /* Added/Changed by SDLEMU (http://sdlemu.ngemu.com */
    /* Joystick support                                 */
    
    if ( console.JoyEnabled() == 1 ) {
    
	x = SDL_JoystickGetAxis(console.joystick, 0);
	y = SDL_JoystickGetAxis(console.joystick, 1);
	
	if ( x > 16384  ) joypad_0_buttons[BUTTON_R]=0x01;
	if ( x < -16384 ) joypad_0_buttons[BUTTON_L]=0x01;
	if ( y > 16384  ) joypad_0_buttons[BUTTON_D]=0x01;
	if ( y < -16384 ) joypad_0_buttons[BUTTON_U]=0x01;
	
	if (SDL_JoystickGetButton(console.joystick, 0) == SDL_PRESSED) joypad_0_buttons[BUTTON_A]=0x01;
	if (SDL_JoystickGetButton(console.joystick, 1) == SDL_PRESSED) joypad_0_buttons[BUTTON_B]=0x01;
	if (SDL_JoystickGetButton(console.joystick, 2) == SDL_PRESSED) joypad_0_buttons[BUTTON_C]=0x01;

	}
	
	/* ADDED by SDLEMU (http://sdlemu.ngemu.com */
	/* Needed to make sure that the events queue is empty */
    SDL_PumpEvents();            

}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void joystick_reset(void)
{
	memset(joystick_ram,0x00,4);
	memset(joypad_0_buttons,0,21);
	memset(joypad_1_buttons,0,21);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void joystick_done(void)
{
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void joystick_byte_write(uint32 offset, uint8 data)
{
	joystick_ram[offset&0x03]=data;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void joystick_word_write(uint32 offset, uint16 data)
{
	offset&=0x03;
	joystick_ram[offset+0]=(data>>8)&0xff;
	joystick_ram[offset+1]=data&0xff;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint8 joystick_byte_read(uint32 offset)
{
	offset&=0x03;

	if (offset==0)
	{
		uint8 data=0x00;
		int pad0Index=joystick_ram[1]&0x0f;
		int pad1Index=(joystick_ram[1]>>4)&0x0f;
		
		if (!(pad0Index&0x01)) 
			pad0Index=0;
		else
		if (!(pad0Index&0x02)) 
			pad0Index=1;
		else
		if (!(pad0Index&0x04)) 
			pad0Index=2;
		else
			pad0Index=3;
		
		if (!(pad1Index&0x01)) 
			pad1Index=0;
		else
		if (!(pad1Index&0x02)) 
			pad1Index=1;
		else
		if (!(pad1Index&0x04)) 
			pad1Index=2;
		else
			pad1Index=3;

		if (joypad_0_buttons[(pad0Index<<2)+0]) data|=0x01;
		if (joypad_0_buttons[(pad0Index<<2)+1]) data|=0x02;
		if (joypad_0_buttons[(pad0Index<<2)+2]) data|=0x04;
		if (joypad_0_buttons[(pad0Index<<2)+3]) data|=0x08;
		if (joypad_1_buttons[(pad1Index<<2)+0]) data|=0x10;
		if (joypad_1_buttons[(pad1Index<<2)+1]) data|=0x20;
		if (joypad_1_buttons[(pad1Index<<2)+2]) data|=0x40;
		if (joypad_1_buttons[(pad1Index<<2)+3]) data|=0x80;
		return(~data);
	}
	else
	if (offset==3)
	{
		uint8 data=((1<<5)|(1<<4)|0x0f) ;
		int pad0Index=joystick_ram[1]&0x0f;
		int pad1Index=(joystick_ram[1]>>4)&0x0f;
		
		if (!(pad0Index&0x01)) 
		{
			if (joypad_0_buttons[BUTTON_PAUSE]) data^=0x01;
			if (joypad_0_buttons[BUTTON_A]) data^=0x02;
		}
		else
		if (!(pad0Index&0x02)) 
		{
			if (joypad_0_buttons[BUTTON_B])		 data^=0x02;
		}
		else
		if (!(pad0Index&0x04)) 
		{
			if (joypad_0_buttons[BUTTON_C]) data^=0x02;
		}
		else
		{
			if (joypad_0_buttons[BUTTON_OPTION]) data^=0x02;
		}		
		return(data);
	}
	return(joystick_ram[offset]);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint16 joystick_word_read(uint32 offset)
{
	uint16 data=joystick_byte_read((offset+0)&0x03);
	data<<=8;
	data|=joystick_byte_read((offset+1)&0x03);
	return(data);
}
