////////////////////////////////////////////////////////////////////////////////
// Audio
////////////////////////////////////////////////////////////////////////////////

#include "include/jaguar.h"
#include "include/dsnd.h"

int ym2413_enable;

int FREQUENCE=44100;

void dsound_reset(void);

#define LEFT 0
#define RIGHT 1
#define SOUND_BUFFER	(0.16)		//Seconds
#define UNDEFINED		0xFFFFFF

int chipBufferLength;	//Number of bytes to hold SOUND_BUFFER seconds.

int lastChipWrite  =0;
int chipWrite =UNDEFINED;	//Write Cursor

INT16* buf1;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

void ws_write_state(int fp)
{
}


void ws_read_state(int fp)
{
}




////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ws_audio_init(void)
{
#ifdef _EMULATE_SOUND
	dsound_init(GetForegroundWindow(),1,FREQUENCE);
//	ws_audio_reset();
#endif
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ws_audio_reset(void)
{
#ifdef _EMULATE_SOUND
 int i;
 dsound_reset();

	 chipWrite=UNDEFINED;

	for (i=0;i<4;i++)
	{
	 ws_audio_stop_channel(i);
	 ws_audio_play_channel(i);
	 ws_audio_clear_channel(i);
	}

	 IDirectSoundBuffer_SetPan(chipBuffer,0);
	 IDirectSoundBuffer_SetVolume(chipBuffer,0);
#endif
}


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ws_audio_done(void)
{
#ifdef _EMULATE_SOUND
	system_sound_shutdown();
#endif
}


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ws_audio_clear_channel(int Channel)
{
#ifdef _EMULATE_SOUND

 switch (Channel)
 {
 case 0:
 case 1:
	  memset(buf1,0,sizeof(buf1));
	  chipWrite = UNDEFINED;
	  break;

 case 2:
 case 3:
	    break;

 }
#endif
}



////////////////////////////////////////////////////////////////////////////////
// start playing a channel
////////////////////////////////////////////////////////////////////////////////
int ws_audio_play_channel(int Channel)
{
#ifdef _EMULATE_SOUND
	switch (Channel)
	{
	case 0:
	case 1:
//		if (psg_on)
		IDirectSoundBuffer_Play(chipBuffer,0,0, DSBPLAY_LOOPING);
		break;
	case 2:
	case 3:
		break;
	}
#endif
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// stop playing a channel
////////////////////////////////////////////////////////////////////////////////
int ws_audio_stop_channel(int Channel)
{
#ifdef _EMULATE_SOUND
	switch (Channel)
	{
	case 0:
	case 1:
		IDirectSoundBuffer_Stop(chipBuffer);
		chipWrite=UNDEFINED;
		break;

	case 2:
	case 3:
		break;
	}
#endif
	return(0);
}



void dsound_reset(void)
{
#ifdef _EMULATE_SOUND
	BYTE	*ppvAudioPtr1, *ppvAudioPtr2;
	DWORD	pdwAudioBytes1, pdwAudioBytes2;


	 chipWrite = UNDEFINED;
	 if (chipBuffer)
	 {
		IDirectSoundBuffer_Stop(chipBuffer);
		// Fill the sound buffer
		if SUCCEEDED(IDirectSoundBuffer_Lock(chipBuffer,0,0, 
			(LPVOID*)&ppvAudioPtr1, &pdwAudioBytes1, (LPVOID*)&ppvAudioPtr2, &pdwAudioBytes2, DSBLOCK_ENTIREBUFFER))
		{
			if (ppvAudioPtr1 && pdwAudioBytes1)
				memset(ppvAudioPtr1, 0, pdwAudioBytes1);

			if (ppvAudioPtr2 && pdwAudioBytes2)
				memset(ppvAudioPtr2, 0, pdwAudioBytes2);
			
			IDirectSoundBuffer_Unlock(chipBuffer, 
				ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2);
		}

		//Start playing
//		if (psg_on)
			IDirectSoundBuffer_Play(chipBuffer, 0,0, DSBPLAY_LOOPING );
	 }
IDirectSoundBuffer_Play(chipBuffer, 0,0, DSBPLAY_LOOPING );
#endif
}



void system_sound_shutdown(void)
{
#ifdef _EMULATE_SOUND
	 if (chipBuffer)	IDirectSoundBuffer_Stop(chipBuffer);
	 if (chipBuffer)	IDirectSoundBuffer_Release(chipBuffer);

	if (primaryBuffer)	IDirectSoundBuffer_Stop(primaryBuffer);
	if (primaryBuffer)	IDirectSoundBuffer_Release(primaryBuffer);
	if (ds)	IDirectSound_Release(ds);
	if (buf1) free(buf1);
#endif
}


void system_sound_update(void)
{
#ifdef _EMULATE_SOUND
	LPDWORD null_ptr=NULL;
	int		pdwAudioBytes1, pdwAudioBytes2;
	int		Write, LengthSamples;
	UINT16	*chipPtr1, *chipPtr2;

	 // UNDEFINED write cursors
	 IDirectSoundBuffer_GetCurrentPosition(chipBuffer, 0, (LPDWORD)&Write);
	 if ((chipWrite == UNDEFINED) )
	 {
		lastChipWrite = chipWrite= Write - 128;
		return; //Wait a frame to accumulate length.
	 }

	
		//SN76496 Sound Chips

		 if (Write < lastChipWrite)	//Wrap?
			lastChipWrite -= chipBufferLength;

		LengthSamples = (Write - lastChipWrite) / 2; 
		lastChipWrite = Write;

/*		if (psg_on)*/ {
		pcm_updateOne(0,buf1,LengthSamples>>1);

		if SUCCEEDED(IDirectSoundBuffer_Lock(
			chipBuffer, chipWrite, LengthSamples*2, 
			(LPVOID*)&chipPtr1, (LPDWORD)&pdwAudioBytes1,
			(LPVOID*)&chipPtr2, (LPDWORD)&pdwAudioBytes2, 0))
		 {
			 __asm {
					cld
					mov eax,buf1
					mov ebx,buf1
					mov edi,chipPtr1
					mov ecx,pdwAudioBytes1
					shr ecx,2
					
Copie1:				mov esi,eax
					movsw
					add eax,2
					mov esi,ebx
					movsw
					add ebx,2
					loop Copie1
			
					mov edi,chipPtr2
					cmp edi,0
					je End
					mov ecx,pdwAudioBytes2
					shr ecx,2

Copie2:				
					mov esi,eax
					movsw
					add eax,2
					mov esi,ebx
					movsw
					add ebx,2
					loop Copie2

End:				}
				

			 IDirectSoundBuffer_Unlock(chipBuffer,
				chipPtr1, pdwAudioBytes1, chipPtr2, pdwAudioBytes2);

			chipWrite+= (LengthSamples * 2);
			if (chipWrite> chipBufferLength)
				chipWrite-= chipBufferLength;
		}
		else
		{
			DWORD status;
			chipWrite= UNDEFINED;
			IDirectSoundBuffer_GetStatus(chipBuffer, &status);
			if (status & DSBSTATUS_BUFFERLOST)
			{
				if (IDirectSoundBuffer_Restore(chipBuffer) != DS_OK) return;
				/*if (psg_on)*/ IDirectSoundBuffer_Play(chipBuffer, 0, 0, DSBPLAY_LOOPING);
			}
	  }
	 }
#endif
}
