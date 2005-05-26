//
// DAC (really, Synchronous Serial Interface) Handler
//
// Originally by David Raingeard
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Rewritten by James L. Hammons
//

#include "SDL.h"
#include "m68k.h"
#include "jaguar.h"
#include "settings.h"
#include "dac.h"

//#define DEBUG_DAC

#define BUFFER_SIZE		0x10000						// Make the DAC buffers 64K x 16 bits

// Jaguar memory locations

#define LTXD			0xF1A148
#define RTXD			0xF1A14C
#define LRXD			0xF1A148
#define RRXD			0xF1A14C
#define SCLK			0xF1A150
#define SMODE			0xF1A154

// Global variables

uint16 lrxd, rrxd;									// I2S ports (into Jaguar)

// Local variables

static uint32 LeftFIFOHeadPtr, LeftFIFOTailPtr, RightFIFOHeadPtr, RightFIFOTailPtr;
static SDL_AudioSpec desired;
static bool SDLSoundInitialized = false;

// We can get away with using native endian here because we can tell SDL to use the native
// endian when looking at the sample buffer, i.e., no need to worry about it.

static uint16 * DACBuffer;
static uint8 SCLKFrequencyDivider = 19;				// Default is roughly 22 KHz (20774 Hz in NTSC mode)
/*static*/ uint16 serialMode = 0;

// Private function prototypes

void SDLSoundCallback(void * userdata, Uint8 * buffer, int length);
int GetCalculatedFrequency(void);

//
// Initialize the SDL sound system
//
void DACInit(void)
{
	memory_malloc_secure((void **)&DACBuffer, BUFFER_SIZE * sizeof(uint16), "DAC buffer");

	desired.freq = GetCalculatedFrequency();		// SDL will do conversion on the fly, if it can't get the exact rate. Nice!
	desired.format = AUDIO_S16SYS;					// This uses the native endian (for portability)...
	desired.channels = 2;
//	desired.samples = 4096;							// Let's try a 4K buffer (can always go lower)
	desired.samples = 2048;							// Let's try a 2K buffer (can always go lower)
	desired.callback = SDLSoundCallback;

	if (SDL_OpenAudio(&desired, NULL) < 0)			// NULL means SDL guarantees what we want
	{
//		WriteLog("DAC: Failed to initialize SDL sound. Shutting down!\n");
//		log_done();
//		exit(1);
		WriteLog("DAC: Failed to initialize SDL sound...\n");
	}
	else
	{
		SDLSoundInitialized = true;
		DACReset();
		SDL_PauseAudio(false);							// Start playback!
		WriteLog("DAC: Successfully initialized.\n");
	}
}

//
// Reset the sound buffer FIFOs
//
void DACReset(void)
{
	LeftFIFOHeadPtr = LeftFIFOTailPtr = 0, RightFIFOHeadPtr = RightFIFOTailPtr = 1;
}

//
// Close down the SDL sound subsystem
//
void DACDone(void)
{
	if (SDLSoundInitialized)
	{
		SDL_PauseAudio(true);
		SDL_CloseAudio();
	}

	memory_free(DACBuffer);
	WriteLog("DAC: Done.\n");
}

//
// SDL callback routine to fill audio buffer
//
// Note: The samples are packed in the buffer in 16 bit left/16 bit right pairs.
//
void SDLSoundCallback(void * userdata, Uint8 * buffer, int length)
{
	// Clear the buffer to silence, in case the DAC buffer is empty (or short)
//This causes choppy sound... Ick.
	memset(buffer, desired.silence, length);
//WriteLog("DAC: Inside callback...\n");
	if (LeftFIFOHeadPtr != LeftFIFOTailPtr)
	{
//WriteLog("DAC: About to write some data!\n");
		int numLeftSamplesReady
			= (LeftFIFOTailPtr + (LeftFIFOTailPtr < LeftFIFOHeadPtr ? BUFFER_SIZE : 0))
				- LeftFIFOHeadPtr;
		int numRightSamplesReady
			= (RightFIFOTailPtr + (RightFIFOTailPtr < RightFIFOHeadPtr ? BUFFER_SIZE : 0))
				- RightFIFOHeadPtr;
		int numSamplesReady
			= (numLeftSamplesReady < numRightSamplesReady
				? numLeftSamplesReady : numRightSamplesReady);//Hmm. * 2;

//The numbers look good--it's just that the DSP can't get enough samples in the DAC buffer!
//WriteLog("DAC: Left/RightFIFOHeadPtr: %u/%u, Left/RightFIFOTailPtr: %u/%u\n", LeftFIFOHeadPtr, RightFIFOHeadPtr, LeftFIFOTailPtr, RightFIFOTailPtr);
//WriteLog("     numLeft/RightSamplesReady: %i/%i, numSamplesReady: %i, length of buffer: %i\n", numLeftSamplesReady, numRightSamplesReady, numSamplesReady, length);

/*		if (numSamplesReady > length)
			numSamplesReady = length;//*/
		if (numSamplesReady > length / 2)	// length / 2 because we're comparing 16-bit lengths
			numSamplesReady = length / 2;
//else
//	WriteLog("     Not enough samples to fill the buffer (short by %u L/R samples)...\n", (length / 2) - numSamplesReady);
//WriteLog("DAC: %u samples ready.\n", numSamplesReady);

		// Actually, it's a bit more involved than this, but this is the general idea:
//		memcpy(buffer, DACBuffer, length);
		for(int i=0; i<numSamplesReady; i++)
			((uint16 *)buffer)[i] = DACBuffer[(LeftFIFOHeadPtr + i) % BUFFER_SIZE];
			// Could also use (as long as BUFFER_SIZE is a multiple of 2):
//			buffer[i] = DACBuffer[(LeftFIFOHeadPtr + i) & (BUFFER_SIZE - 1)];

		LeftFIFOHeadPtr = (LeftFIFOHeadPtr + numSamplesReady) % BUFFER_SIZE;
		RightFIFOHeadPtr = (RightFIFOHeadPtr + numSamplesReady) % BUFFER_SIZE;
		// Could also use (as long as BUFFER_SIZE is a multiple of 2):
//		LeftFIFOHeadPtr = (LeftFIFOHeadPtr + numSamplesReady) & (BUFFER_SIZE - 1);
//		RightFIFOHeadPtr = (RightFIFOHeadPtr + numSamplesReady) & (BUFFER_SIZE - 1);
//WriteLog("  -> Left/RightFIFOHeadPtr: %u/%u, Left/RightFIFOTailPtr: %u/%u\n", LeftFIFOHeadPtr, RightFIFOHeadPtr, LeftFIFOTailPtr, RightFIFOTailPtr);
	}
//Hmm. Seems that the SDL buffer isn't being starved by the DAC buffer...
//	else
//		WriteLog("DAC: Silence...!\n");
}

//
// Calculate the frequency of SCLK * 32 using the divider
//
int GetCalculatedFrequency(void)
{
	int systemClockFrequency = (vjs.hardwareTypeNTSC ? RISC_CLOCK_RATE_NTSC : RISC_CLOCK_RATE_PAL);

	// We divide by 32 here in order to find the frequency of 32 SCLKs in a row (transferring
	// 16 bits of left data + 16 bits of right data = 32 bits, 1 SCLK = 1 bit transferred).
	return systemClockFrequency / (32 * (2 * (SCLKFrequencyDivider + 1)));
}

//
// LTXD/RTXD/SCLK/SMODE ($F1A148/4C/50/54)
//
void DACWriteByte(uint32 offset, uint8 data, uint32 who/*= UNKNOWN*/)
{
	WriteLog("DAC: %s writing BYTE %02X at %08X\n", whoName[who], data, offset);
	if (offset == SCLK + 3)
		DACWriteWord(offset - 3, (uint16)data);
}

void DACWriteWord(uint32 offset, uint16 data, uint32 who/*= UNKNOWN*/)
{
	if (offset == LTXD + 2)
	{
		// Spin until buffer has been drained (for too fast processors!)...
//Small problem--if Head == 0 and Tail == buffer end, then this will fail... !!! FIX !!!
//[DONE]
		// Also, we're taking advantage of the fact that the buffer is a multiple of two
		// in this check...
		while (((LeftFIFOTailPtr + 2) & (BUFFER_SIZE - 1)) == LeftFIFOHeadPtr);

		SDL_LockAudio();							// Is it necessary to do this? Mebbe.
		// We use a circular buffer 'cause it's easy. Note that the callback function
		// takes care of dumping audio to the soundcard...! Also note that we're writing
		// the samples in the buffer in an interleaved L/R format.
		LeftFIFOTailPtr = (LeftFIFOTailPtr + 2) % BUFFER_SIZE;
		DACBuffer[LeftFIFOTailPtr] = data;
		SDL_UnlockAudio();
	}
	else if (offset == RTXD + 2)
	{
		// Spin until buffer has been drained (for too fast processors!)...
//uint32 spin = 0;
		while (((RightFIFOTailPtr + 2) & (BUFFER_SIZE - 1)) == RightFIFOHeadPtr);
/*		{
spin++;
//if ((spin & 0x0FFFFFFF) == 0)
//	WriteLog("Tail=%X, Head=%X, BUFFER_SIZE-1=%X\n", RightFIFOTailPtr, RightFIFOHeadPtr, BUFFER_SIZE - 1);

if (spin == 0xFFFF0000)
{
uint32 rtail = RightFIFOTailPtr, rhead = RightFIFOHeadPtr;
WriteLog("Tail=%X, Head=%X\n", rtail, rhead);

	WriteLog("\nStuck in right DAC spinlock!\nAborting!\n\n");
	WriteLog("Tail=%X, Head=%X, BUFFER_SIZE-1=%X\n", RightFIFOTailPtr, RightFIFOHeadPtr, BUFFER_SIZE - 1);
	WriteLog("From while: Tail=%X, Head=%X", (RightFIFOTailPtr + 2) & (BUFFER_SIZE - 1), RightFIFOHeadPtr);
	log_done();
	exit(0);
}
		}//*/

//This is wrong		if (RightFIFOTailPtr + 2 != RightFIFOHeadPtr)
//		{
		SDL_LockAudio();
		RightFIFOTailPtr = (RightFIFOTailPtr + 2) % BUFFER_SIZE;
		DACBuffer[RightFIFOTailPtr] = data;
		SDL_UnlockAudio();
//		}
/*#ifdef DEBUG_DAC
		else
			WriteLog("DAC: Ran into FIFO's right tail pointer!\n");
#endif*/
	}
	else if (offset == SCLK + 2)					// Sample rate
	{
		WriteLog("DAC: Writing %u to SCLK...\n", data);
		if ((uint8)data != SCLKFrequencyDivider)
		{
			SCLKFrequencyDivider = (uint8)data;
//Of course a better way would be to query the hardware to find the upper limit...
			if (data > 7)	// Anything less than 8 is too high!
			{
				if (SDLSoundInitialized)
					SDL_CloseAudio();

				desired.freq = GetCalculatedFrequency();// SDL will do conversion on the fly, if it can't get the exact rate. Nice!
				WriteLog("DAC: Changing sample rate to %u Hz!\n", desired.freq);

				if (SDLSoundInitialized)
				{
					if (SDL_OpenAudio(&desired, NULL) < 0)	// NULL means SDL guarantees what we want
					{
						WriteLog("DAC: Failed to initialize SDL sound: %s.\nDesired freq: %u\nShutting down!\n", SDL_GetError(), desired.freq);
						log_done();
						exit(1);
					}
				}

				DACReset();

				if (SDLSoundInitialized)
					SDL_PauseAudio(false);			// Start playback!
			}
		}
	}
	else if (offset == SMODE + 2)
	{
		serialMode = data;
		WriteLog("DAC: %s writing to SMODE. Bits: %s%s%s%s%s%s [68K PC=%08X]\n", whoName[who],
			(data & 0x01 ? "INTERNAL " : ""), (data & 0x02 ? "MODE " : ""),
			(data & 0x04 ? "WSEN " : ""), (data & 0x08 ? "RISING " : ""),
			(data & 0x10 ? "FALLING " : ""), (data & 0x20 ? "EVERYWORD" : ""),
			m68k_get_reg(NULL, M68K_REG_PC));
	}
}

//
// LRXD/RRXD/SSTAT ($F1A148/4C/50)
//
uint8 DACReadByte(uint32 offset, uint32 who/*= UNKNOWN*/)
{
//	WriteLog("DAC: %s reading byte from %08X\n", whoName[who], offset);
	return 0xFF;
}

//static uint16 fakeWord = 0;
uint16 DACReadWord(uint32 offset, uint32 who/*= UNKNOWN*/)
{
//	WriteLog("DAC: %s reading word from %08X\n", whoName[who], offset);
//	return 0xFFFF;
//	WriteLog("DAC: %s reading WORD %04X from %08X\n", whoName[who], fakeWord, offset);
//	return fakeWord++;
//NOTE: This only works if a bunch of things are set in BUTCH which we currently don't
//      check for. !!! FIX !!!
// Partially fixed: We check for I2SCNTRL in the JERRY I2S routine...
//	return GetWordFromButchSSI(offset, who);
	if (offset == LRXD || offset == RRXD)
		return 0x0000;
	else if (offset == LRXD + 2)
		return lrxd;
	else if (offset == RRXD + 2)
		return rrxd;

	return 0xFFFF;	// May need SSTAT as well... (but may be a Jaguar II only feature)		
}
