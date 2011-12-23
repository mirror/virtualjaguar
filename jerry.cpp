//
// JERRY Core
//
// Originally by David Raingeard
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Carwin Jones (BeOS)
// Cleanups/rewrites/fixes by James L. Hammons
//
//	------------------------------------------------------------
//	JERRY REGISTERS (Mapped by Aaron Giles)
//	------------------------------------------------------------
//	F10000-F13FFF   R/W   xxxxxxxx xxxxxxxx   Jerry
//	F10000            W   xxxxxxxx xxxxxxxx   JPIT1 - timer 1 pre-scaler
//	F10002            W   xxxxxxxx xxxxxxxx   JPIT2 - timer 1 divider
//	F10004            W   xxxxxxxx xxxxxxxx   JPIT3 - timer 2 pre-scaler
//	F10008            W   xxxxxxxx xxxxxxxx   JPIT4 - timer 2 divider
//	F10010            W   ------xx xxxxxxxx   CLK1 - processor clock divider
//	F10012            W   ------xx xxxxxxxx   CLK2 - video clock divider
//	F10014            W   -------- --xxxxxx   CLK3 - chroma clock divider
//	F10020          R/W   ---xxxxx ---xxxxx   JINTCTRL - interrupt control register
//	                  W   ---x---- --------      (J_SYNCLR - clear synchronous serial intf ints)
//	                  W   ----x--- --------      (J_ASYNCLR - clear asynchronous serial intf ints)
//	                  W   -----x-- --------      (J_TIM2CLR - clear timer 2 [tempo] interrupts)
//	                  W   ------x- --------      (J_TIM1CLR - clear timer 1 [sample] interrupts)
//	                  W   -------x --------      (J_EXTCLR - clear external interrupts)
//	                R/W   -------- ---x----      (J_SYNENA - enable synchronous serial intf ints)
//	                R/W   -------- ----x---      (J_ASYNENA - enable asynchronous serial intf ints)
//	                R/W   -------- -----x--      (J_TIM2ENA - enable timer 2 [tempo] interrupts)
//	                R/W   -------- ------x-      (J_TIM1ENA - enable timer 1 [sample] interrupts)
//	                R/W   -------- -------x      (J_EXTENA - enable external interrupts)
//	F10030          R/W   -------- xxxxxxxx   ASIDATA - asynchronous serial data
//	F10032            W   -x------ -xxxxxxx   ASICTRL - asynchronous serial control
//	                  W   -x------ --------      (TXBRK - transmit break)
//	                  W   -------- -x------      (CLRERR - clear error)
//	                  W   -------- --x-----      (RINTEN - enable receiver interrupts)
//	                  W   -------- ---x----      (TINTEN - enable transmitter interrupts)
//	                  W   -------- ----x---      (RXIPOL - receiver input polarity)
//	                  W   -------- -----x--      (TXOPOL - transmitter output polarity)
//	                  W   -------- ------x-      (PAREN - parity enable)
//	                  W   -------- -------x      (ODD - odd parity select)
//	F10032          R     xxx-xxxx x-xxxxxx   ASISTAT - asynchronous serial status
//	                R     x------- --------      (ERROR - OR of PE,FE,OE)
//	                R     -x------ --------      (TXBRK - transmit break)
//	                R     --x----- --------      (SERIN - serial input)
//	                R     ----x--- --------      (OE - overrun error)
//	                R     -----x-- --------      (FE - framing error)
//	                R     ------x- --------      (PE - parity error)
//	                R     -------x --------      (TBE - transmit buffer empty)
//	                R     -------- x-------      (RBF - receive buffer full)
//	                R     -------- ---x----      (TINTEN - enable transmitter interrupts)
//	                R     -------- ----x---      (RXIPOL - receiver input polarity)
//	                R     -------- -----x--      (TXOPOL - transmitter output polarity)
//	                R     -------- ------x-      (PAREN - parity enable)
//	                R     -------- -------x      (ODD - odd parity)
//	F10034          R/W   xxxxxxxx xxxxxxxx   ASICLK - asynchronous serial interface clock
//	F10036          R     xxxxxxxx xxxxxxxx   JPIT1 - timer 1 pre-scaler
//	F10038          R     xxxxxxxx xxxxxxxx   JPIT2 - timer 1 divider
//	F1003A          R     xxxxxxxx xxxxxxxx   JPIT3 - timer 2 pre-scaler
//	F1003C          R     xxxxxxxx xxxxxxxx   JPIT4 - timer 2 divider
//	------------------------------------------------------------
//	F14000-F17FFF   R/W   xxxxxxxx xxxxxxxx   Joysticks and GPIO0-5
//	F14000          R     xxxxxxxx xxxxxxxx   JOYSTICK - read joystick state
//	F14000            W   x------- xxxxxxxx   JOYSTICK - latch joystick output
//	                  W   x------- --------      (enable joystick outputs)
//	                  W   -------- xxxxxxxx      (joystick output data)
//	F14002          R     xxxxxxxx xxxxxxxx   JOYBUTS - button register
//	F14800-F14FFF   R/W   xxxxxxxx xxxxxxxx   GPI00 - reserved (CD-ROM?)
//	F15000-F15FFF   R/W   xxxxxxxx xxxxxxxx   GPI01 - reserved
//	F16000-F16FFF   R/W   xxxxxxxx xxxxxxxx   GPI02 - reserved
//	F17000-F177FF   R/W   xxxxxxxx xxxxxxxx   GPI03 - reserved
//	F17800-F17BFF   R/W   xxxxxxxx xxxxxxxx   GPI04 - reserved
//	F17C00-F17FFF   R/W   xxxxxxxx xxxxxxxx   GPI05 - reserved
//	------------------------------------------------------------
//	F18000-F1FFFF   R/W   xxxxxxxx xxxxxxxx   Jerry DSP
//	F1A100          R/W   xxxxxxxx xxxxxxxx   D_FLAGS - DSP flags register
//	                R/W   x------- --------      (DMAEN - DMA enable)
//	                R/W   -x------ --------      (REGPAGE - register page)
//	                  W   --x----- --------      (D_EXT0CLR - clear external interrupt 0)
//	                  W   ---x---- --------      (D_TIM2CLR - clear timer 2 interrupt)
//	                  W   ----x--- --------      (D_TIM1CLR - clear timer 1 interrupt)
//	                  W   -----x-- --------      (D_I2SCLR - clear I2S interrupt)
//	                  W   ------x- --------      (D_CPUCLR - clear CPU interrupt)
//	                R/W   -------x --------      (D_EXT0ENA - enable external interrupt 0)
//	                R/W   -------- x-------      (D_TIM2ENA - enable timer 2 interrupt)
//	                R/W   -------- -x------      (D_TIM1ENA - enable timer 1 interrupt)
//	                R/W   -------- --x-----      (D_I2SENA - enable I2S interrupt)
//	                R/W   -------- ---x----      (D_CPUENA - enable CPU interrupt)
//	                R/W   -------- ----x---      (IMASK - interrupt mask)
//	                R/W   -------- -----x--      (NEGA_FLAG - ALU negative)
//	                R/W   -------- ------x-      (CARRY_FLAG - ALU carry)
//	                R/W   -------- -------x      (ZERO_FLAG - ALU zero)
//	F1A102          R/W   -------- ------xx   D_FLAGS - upper DSP flags
//	                R/W   -------- ------x-      (D_EXT1ENA - enable external interrupt 1)
//	                R/W   -------- -------x      (D_EXT1CLR - clear external interrupt 1)
//	F1A104            W   -------- ----xxxx   D_MTXC - matrix control register
//	                  W   -------- ----x---      (MATCOL - column/row major)
//	                  W   -------- -----xxx      (MATRIX3-15 - matrix width)
//	F1A108            W   ----xxxx xxxxxx--   D_MTXA - matrix address register
//	F1A10C            W   -------- -----x-x   D_END - data organization register
//	                  W   -------- -----x--      (BIG_INST - big endian instruction fetch)
//	                  W   -------- -------x      (BIG_IO - big endian I/O)
//	F1A110          R/W   xxxxxxxx xxxxxxxx   D_PC - DSP program counter
//	F1A114          R/W   xxxxxxxx xx-xxxxx   D_CTRL - DSP control/status register
//	                R     xxxx---- --------      (VERSION - DSP version code)
//	                R/W   ----x--- --------      (BUS_HOG - hog the bus!)
//	                R/W   -----x-- --------      (D_EXT0LAT - external interrupt 0 latch)
//	                R/W   ------x- --------      (D_TIM2LAT - timer 2 interrupt latch)
//	                R/W   -------x --------      (D_TIM1LAT - timer 1 interrupt latch)
//	                R/W   -------- x-------      (D_I2SLAT - I2S interrupt latch)
//	                R/W   -------- -x------      (D_CPULAT - CPU interrupt latch)
//	                R/W   -------- ---x----      (SINGLE_GO - single step one instruction)
//	                R/W   -------- ----x---      (SINGLE_STEP - single step mode)
//	                R/W   -------- -----x--      (FORCEINT0 - cause interrupt 0 on GPU)
//	                R/W   -------- ------x-      (CPUINT - send GPU interrupt to CPU)
//	                R/W   -------- -------x      (DSPGO - enable DSP execution)
//	F1A116          R/W   -------- -------x   D_CTRL - upper DSP control/status register
//	                R/W   -------- -------x      (D_EXT1LAT - external interrupt 1 latch)
//	F1A118-F1A11B     W   xxxxxxxx xxxxxxxx   D_MOD - modulo instruction mask
//	F1A11C-F1A11F   R     xxxxxxxx xxxxxxxx   D_REMAIN - divide unit remainder
//	F1A11C            W   -------- -------x   D_DIVCTRL - divide unit control
//	                  W   -------- -------x      (DIV_OFFSET - 1=16.16 divide, 0=32-bit divide)
//	F1A120-F1A123   R     xxxxxxxx xxxxxxxx   D_MACHI - multiply & accumulate high bits
//	F1A148            W   xxxxxxxx xxxxxxxx   R_DAC - right transmit data
//	F1A14C            W   xxxxxxxx xxxxxxxx   L_DAC - left transmit data
//	F1A150            W   -------- xxxxxxxx   SCLK - serial clock frequency
//	F1A150          R     -------- ------xx   SSTAT
//	                R     -------- ------x-      (left - no description)
//	                R     -------- -------x      (WS - word strobe status)
//	F1A154            W   -------- --xxxx-x   SMODE - serial mode
//	                  W   -------- --x-----      (EVERYWORD - interrupt on MSB of every word)
//	                  W   -------- ---x----      (FALLING - interrupt on falling edge)
//	                  W   -------- ----x---      (RISING - interrupt of rising edge)
//	                  W   -------- -----x--      (WSEN - enable word strobes)
//	                  W   -------- -------x      (INTERNAL - enables serial clock)
//	------------------------------------------------------------
//	F1B000-F1CFFF   R/W   xxxxxxxx xxxxxxxx   Local DSP RAM
//	------------------------------------------------------------
//	F1D000          R     xxxxxxxx xxxxxxxx   ROM_TRI - triangle wave
//	F1D200          R     xxxxxxxx xxxxxxxx   ROM_SINE - full sine wave
//	F1D400          R     xxxxxxxx xxxxxxxx   ROM_AMSINE - amplitude modulated sine wave
//	F1D600          R     xxxxxxxx xxxxxxxx   ROM_12W - sine wave and second order harmonic
//	F1D800          R     xxxxxxxx xxxxxxxx   ROM_CHIRP16 - chirp
//	F1DA00          R     xxxxxxxx xxxxxxxx   ROM_NTRI - traingle wave with noise
//	F1DC00          R     xxxxxxxx xxxxxxxx   ROM_DELTA - spike
//	F1DE00          R     xxxxxxxx xxxxxxxx   ROM_NOISE - white noise
//	------------------------------------------------------------

//#include <math.h>
#include "jaguar.h"
#include "wavetable.h"
#include "jerry.h"
#include "clock.h"
#include "dac.h"

//Note that 44100 Hz requires samples every 22.675737 usec.
#define NEW_TIMER_SYSTEM
//#define JERRY_DEBUG
//#define I2S_MASTER_IRQ				// Log I2S irq if Jerry is I2S master
//#define I2S_MASTER_DATA				// Log I2S transmit data if Jerry is I2S master
//#define I2S_SLAVE_IRQ					// Log I2S irq if Jerry is I2S slave
//#define I2S_SLAVE_DATA				// Log I2S receive data if Jerry is I2S slave

#define JERRY_INT0_ENABLE	0x0001
#define JERRY_INT1_ENABLE	0x0002
#define JERRY_INT2_ENABLE	0x0004
#define JERRY_INT3_ENABLE	0x0008
#define JERRY_INT4_ENABLE	0x0010
#define JERRY_INT5_ENABLE	0x0020
#define JERRY_INT0_CLEAR	0x0100
#define JERRY_INT1_CLEAR	0x0200
#define JERRY_INT2_CLEAR	0x0400
#define JERRY_INT3_CLEAR	0x0800
#define JERRY_INT4_CLEAR	0x1000
#define JERRY_INT5_CLEAR	0x2000
#define JERRY_INT0_PENDING	0x0001
#define JERRY_INT1_PENDING	0x0002
#define JERRY_INT2_PENDING	0x0004
#define JERRY_INT3_PENDING	0x0008
#define JERRY_INT4_PENDING	0x0010
#define JERRY_INT5_PENDING	0x0020

#define JERRY_INT05_ENABLE_FLAGS	(JERRY_INT0_ENABLE | JERRY_INT1_ENABLE | JERRY_INT2_ENABLE | JERRY_INT3_ENABLE | JERRY_INT4_ENABLE | JERRY_INT5_ENABLE)
#define JERRY_INT05_CLEAR_FLAGS		(JERRY_INT0_CLEAR | JERRY_INT1_CLEAR | JERRY_INT2_CLEAR | JERRY_INT3_CLEAR | JERRY_INT4_CLEAR | JERRY_INT5_CLEAR)

static uint8 * jerry_ram_8;

static uint16 jerryTimer1Prescaler;
static uint16 jerryTimer1Divider;
static uint16 jerryTimer2Prescaler;
static uint16 jerryTimer2Divider;
static uint16 jerryTimer1PrescalerCounter;
static uint16 jerryTimer1DividerCounter;
static uint16 jerryTimer2PrescalerCounter;
static uint16 jerryTimer2DividerCounter;
//static int16 jerry_timer_1_counter;
//static int32 jerry_timer_2_counter;

//static uint32 jerry_i2s_interrupt_divide = 8;
//static int32 jerry_i2s_interrupt_timer = -1;
//uint32 jerryI2SCycles;
uint16 jerryIntControl;
uint16 jerryIntPending;

extern uint16 * DACBuffer;							// Left and right audio data
extern uint16 DACBufferIndex;

void SDLAdjustAudioFrequency(void);

// Private function prototypes

//void JERRYResetPIT1(void);
//void JERRYResetPIT2(void);
//void JERRYResetI2S(void);

//void JERRYPIT1Callback(void);
//void JERRYPIT2Callback(void);
//void JERRYI2SCallback(void);

#define LTXD			0xF1A148
#define RTXD			0xF1A14C
#define LRXD			0xF1A148
#define RRXD			0xF1A14C
#define SCLK			0xF1A150
#define SMODE			0xF1A154
#define SSTAT			0xF1A150

// Used for I2S interface of Jerry and Butch
enum ClkState { STATE_LOW, STATE_HIGH, STATE_RISING, STATE_FALLING };

// I2S interface
uint16	I2S_SerialClock;
uint8	I2S_SerialMode;
uint8	I2S_SerialStatus;
uint16	I2SClkWaitCycle;
uint8	I2SClkState;
uint8	I2SWSBitCount;		// Word Strobe bit counter
bool	I2SWSHigh;			// Word Strobe status (high/low)

//uint16 JerryShiftTXD;			// Transmits data from Jerry I2S to DAC I2S
uint16 ButchShiftTXD;			// Transmits data from Butch I2S to Jerry I2S
//uint16 JerryLatchedLTXD;		// Transmits left data to DAC I2S
//uint16 JerryLatchedRTXD;		// Transmits right data to DAC I2S
uint16 JerryLatchedLRXD;		// Receives left data from Butch I2S
uint16 JerryLatchedRRXD;		// Receives right data from Butch I2S

/*
//This approach is probably wrong, since the timer is continuously counting down, though
//it might only be a problem if the # of interrupts generated is greater than 1--the M68K's
//timeslice should be running during that phase... (The DSP needs to be aware of this!)

//This is only used by the old system, so once the new timer system is working this
//should be safe to nuke.
void jerry_i2s_exec(uint32 cycles)
{
#ifndef NEW_TIMER_SYSTEM
	extern uint16 serialMode;						// From DAC.CPP
	if (serialMode & 0x01)							// INTERNAL flag (JERRY is master)
	{

	// Why is it called this? Instead of SCLK? Shouldn't this be read from DAC.CPP???
//Yes, it should. !!! FIX !!!
		jerry_i2s_interrupt_divide &= 0xFF;

		if (jerry_i2s_interrupt_timer == -1)
		{
		// We don't have to divide the RISC clock rate by this--the reason is a bit
		// convoluted. Will put explanation here later...
// What's needed here is to find the ratio of the frequency to the number of clock cycles
// in one second. For example, if the sample rate is 44100, we divide the clock rate by
// this: 26590906 / 44100 = 602 cycles.
// Which means, every 602 cycles that go by we have to generate an interrupt.
			jerryI2SCycles = 32 * (2 * (jerry_i2s_interrupt_divide + 1));
		}

		jerry_i2s_interrupt_timer -= cycles;
		if (jerry_i2s_interrupt_timer <= 0)
		{
			// Generate I2S interrupts, does "IRQ enabled" checking
			DSPSetIRQLine(DSP_IRQ_I2S);
			JERRYSetIRQLine(JERRY_IRQ_I2S);
			jerry_i2s_interrupt_timer += jerryI2SCycles;
#ifdef JERRY_DEBUG
			if (jerry_i2s_interrupt_timer < 0)
				WriteLog("JERRY: Missed generating an interrupt (missed %u)!\n", (-jerry_i2s_interrupt_timer / jerryI2SCycles) + 1);
#endif
		}
	}
	else											// JERRY is slave to external word clock
	{
		// This is just a temporary kludge to see if the CD bus mastering works
		// I.e., this is totally faked...!
// The whole interrupt system is pretty much borked and is need of an overhaul.
// What we need is a way of handling these interrupts when they happen instead of
// scanline boundaries the way it is now.
		jerry_i2s_interrupt_timer -= cycles;
		if (jerry_i2s_interrupt_timer <= 0)
		{
//This is probably wrong as well (i.e., need to check enable lines)... !!! FIX !!! [DONE]
			if (ButchIsReadyToSend())//Not sure this is right spot to check...
			{
//	return GetWordFromButchSSI(offset, who);
				SetSSIWordsXmittedFromButch();
				// Generate I2S interrupts, does "IRQ enabled" checking
				DSPSetIRQLine(DSP_IRQ_I2S);//, ASSERT_LINE);
				JERRYSetIRQLine(JERRY_IRQ_I2S);
			}
			jerry_i2s_interrupt_timer += 602;
		}
	}
#else
	RemoveCallback(JERRYI2SCallback);
	JERRYI2SCallback();
#endif
}
*/

//NOTE: This is only used by the old execution core. Safe to nuke once it's stable.
void JERRYExecPIT(void)	//uint32 cycles)
{
	// Decrement prescaler counter
	jerryTimer1PrescalerCounter--;
	if (jerryTimer1PrescalerCounter == 0)
	{
		// Reinitialize prescaler counter
		jerryTimer1PrescalerCounter = jerryTimer1Prescaler + 1;
		// Decrement divider counter
		jerryTimer1DividerCounter--;
		if (jerryTimer1DividerCounter == 0)
		{
			// Reinitialize divider counter
			jerryTimer1DividerCounter = jerryTimer1Divider + 1;

			// Generate TIMER 1 interrupts, does "IRQ enabled" checking
			DSPSetIRQLine(DSP_IRQ_TIMER1);
			JERRYSetIRQLine(DSP_IRQ_TIMER1);
		}
	}
	// Decrement prescaler counter
	jerryTimer2PrescalerCounter--;
	if (jerryTimer2PrescalerCounter == 0)
	{
		// Reinitialize prescaler counter
		jerryTimer2PrescalerCounter = jerryTimer2Prescaler + 1;
		// Decrement divider counter
		jerryTimer2DividerCounter--;
		if (jerryTimer2DividerCounter == 0)
		{
			// Reinitialize divider counter
			jerryTimer2DividerCounter = jerryTimer2Divider + 1;

			// Generate TIMER 1 interrupts, does "IRQ enabled" checking
			DSPSetIRQLine(DSP_IRQ_TIMER2);
			JERRYSetIRQLine(DSP_IRQ_TIMER2);
		}
	}
}
/*		//This is wrong too: Counters are *always* spinning! !!! FIX !!! [DONE]
//	if (jerry_timer_1_counter)
		jerry_timer_1_counter -= cycles;

	if (jerry_timer_1_counter <= 0)
	{
		// Generate TIMER 1 interrupts, does "IRQ enabled" checking
		DSPSetIRQLine(DSP_IRQ_TIMER1);
		JERRYSetIRQLine(DSP_IRQ_TIMER1);
//		JERRYResetPIT1();
		jerry_timer_1_counter += (JERRYPIT1Prescaler + 1) * (JERRYPIT1Divider + 1);
	}

//This is wrong too: Counters are *always* spinning! !!! FIX !!! [DONE]
//	if (jerry_timer_2_counter)
		jerry_timer_2_counter -= cycles;

	if (jerry_timer_2_counter <= 0)
	{
		// Generate TIMER 2 interrupts, does "IRQ enabled" checking
		DSPSetIRQLine(DSP_IRQ_TIMER2);
		JERRYSetIRQLine(JERRY_IRQ_TIMER2);
//		JERRYResetPIT2();
		jerry_timer_2_counter += (JERRYPIT2Prescaler + 1) * (JERRYPIT2Divider + 1);
	}
}*/

/*void JERRYResetI2S(void)
{
	//WriteLog("i2s: reseting\n");
//This is really SCLK... !!! FIX !!!
	jerry_i2s_interrupt_divide = 8;
	jerry_i2s_interrupt_timer = -1;
}*/

/*void JERRYResetPIT1(void)
{
#ifndef NEW_TIMER_SYSTEM
//	if (!JERRYPIT1Prescaler || !JERRYPIT1Divider)
//		jerry_timer_1_counter = 0;
//	else
//Small problem with this approach: Overflow if both are = $FFFF. !!! FIX !!!
		jerry_timer_1_counter = (JERRYPIT1Prescaler + 1) * (JERRYPIT1Divider + 1);

//	if (jerry_timer_1_counter)
//		WriteLog("jerry: reseting timer 1 to 0x%.8x (%i)\n",jerry_timer_1_counter,jerry_timer_1_counter);

#else
	RemoveCallback(JERRYPIT1Callback);

	if (JERRYPIT1Prescaler | JERRYPIT1Divider)
	{
		double usecs = (float)(JERRYPIT1Prescaler + 1) * (float)(JERRYPIT1Divider + 1) * RISC_CYCLE_IN_USEC;
		SetCallbackTime(JERRYPIT1Callback, usecs);
	}
#endif
}*/

/*void JERRYResetPIT2(void)
{
#ifndef NEW_TIMER_SYSTEM
//	if (!JERRYPIT2Prescaler || !JERRYPIT2Divider)
//	{
//		jerry_timer_2_counter = 0;
//		return;
//	}
//	else//
		jerry_timer_2_counter = (JERRYPIT2Prescaler + 1) * (JERRYPIT2Divider + 1);

//	if (jerry_timer_2_counter)
//		WriteLog("jerry: reseting timer 2 to 0x%.8x (%i)\n",jerry_timer_2_counter,jerry_timer_2_counter);

#else
	RemoveCallback(JERRYPIT2Callback);

	if (JERRYPIT1Prescaler | JERRYPIT1Divider)
	{
		double usecs = (float)(JERRYPIT2Prescaler + 1) * (float)(JERRYPIT2Divider + 1) * RISC_CYCLE_IN_USEC;
		SetCallbackTime(JERRYPIT2Callback, usecs);
	}
#endif
}*/

/*void JERRYPIT1Callback(void)
{
	// Generate TIMER 1 interrupts, does "IRQ enabled" checking
	DSPSetIRQLine(DSP_IRQ_TIMER1);
	JERRYSetIRQLine(JERRY_IRQ_TIMER1);
	JERRYResetPIT1();
}*/

/*void JERRYPIT2Callback(void)
{
	// Generate TIMER 2 interrupts, does "IRQ enabled" checking
	DSPSetIRQLine(DSP_IRQ_TIMER2);
	JERRYSetIRQLine(JERRY_IRQ_TIMER2);
	JERRYResetPIT2();
}*/

//
// Generate rising and falling clock edges for I2S
//
void GenerateI2SClock(uint16 I2SFrequency)
{
	// Generate I2S master clock
	I2SClkWaitCycle++;
	// Have we reached the wanted "frequency"
	if (I2SClkWaitCycle >= I2SFrequency)
	{
		I2SClkWaitCycle = 0;

		// Toggle the clock edge
		if (I2SClkState == STATE_LOW)
		{
			I2SClkState = STATE_RISING;
			//WriteLog("I2S: Clock edge rising.\n");
		}	
		if (I2SClkState == STATE_HIGH)
		{
			I2SClkState = STATE_FALLING;
			//WriteLog("I2S: Clock edge falling.\n");
		}
	}
	else
	{
		// Set H or L state between toggling edges
		if (I2SClkState == STATE_FALLING)
			I2SClkState = STATE_LOW;
		if (I2SClkState == STATE_RISING)
			I2SClkState = STATE_HIGH;
	}
}

//
// The I2S interface is coupled to the serial DAC.
// The I2S interface clocks 16 bit left and 16 bit right digital audio data into the DAC
// and the DAC puts the analog signal on the "speakers".
// The I2S interface can generate an interupt after each 16 bits transfered.
// I2S interrupts trigger the DSP I2S vector routine.
// This game specific routine writes the audio data to the I2S registers LTXD and RTXD.
// Mostly the I2S vector routine puts left and right data together into LTXD and RTXD
// so one interrupt after 32 bits (left and right data) can be used to trigger the transfer.
// With SDL audio this timing gets tricky.
// The audio buffer which is transferred in the SDLSoundCallback function
// to the sound card has to be filled faster.
// Therefore the 16 bit transfer cycles are "skipped" and
// the I2S interrupts are generated immediately if Jerry is the I2S master.
// This approach pushes more load on the I2S vector routine.
//
void JERRYExecI2S(void)
{
	if (I2S_SerialMode & 0x01)				// INTERNAL flag set -> Jerry is I2S master
	{
		GenerateI2SClock(I2S_SerialClock);

		// Only if falling clock edge
		if (I2SClkState == STATE_FALLING)
		{
			I2SClkState = STATE_LOW;		// Set static

			// Handle WS
// Disabled due to memo above
			//if (I2SWSBitCount >= 16)		// Toggle WS after 16 data bits
			{
				I2SWSBitCount = 0;			

				I2SWSHigh = !I2SWSHigh;
				// Handle serial status of Jerry as I2S master
				{
					if (I2SWSHigh == true)
					{
						// Left channel data
						// Set WS flag (bit 0) in status register
						// Set LEFT flag (bit 1) in status register
						I2S_SerialStatus = I2S_SerialStatus | 0x01 | 0x02;

						if (I2S_SerialMode & 0x04)		// WSEN flag set
						{
							// Interrupt on rising edge or every WS 
							if (I2S_SerialMode & (0x08 | 0x20))
							{
								// Generate I2S interrupts, does "IRQ enabled" checking
								DSPSetIRQLine(DSP_IRQ_I2S);
								JERRYSetIRQLine(JERRY_IRQ_I2S);
#ifdef I2S_MASTER_IRQ
								WriteLog("I2S_JERRY: Rising WS IRQ generated\n");
#endif
							}
						}
						else
						{
							// Disable WS if Jerry is master and WSEN flag not set
							I2SWSHigh = false;
							// Clear WS flag in status register
							I2S_SerialStatus = I2S_SerialStatus & ~0x01;
						}
					}
					else	// WS is falling
					{
						// Right channel data
						// Clear WS flag (bit 0) in status register
						// Clear LEFT flag (bit 1) in status register
						I2S_SerialStatus = I2S_SerialStatus & ~(0x01 | 0x02);

						// Interrupt on falling edge or every WS 
						if (I2S_SerialMode & (0x10 | 0x20))
						{
							// Generate I2S interrupts, does "IRQ enabled" checking
							DSPSetIRQLine(DSP_IRQ_I2S);
							JERRYSetIRQLine(JERRY_IRQ_I2S);
#ifdef I2S_MASTER_IRQ
							WriteLog("I2S_JERRY: Falling WS IRQ generated\n");
#endif
						}
					}
				}
/*#ifdef I2S_MASTER_IRQ
				if (I2SWSHigh == true)
					WriteLog("I2S_JERRY: Master WS is rising.\n");
				else
					WriteLog("I2S_JERRY: Master WS is falling.\n");
#endif*/
			}

// Audio data is already written into DACBuffer with JERRYWriteWord() function
/*			// Handle transmitting data
			if (I2SWSBitCount == 1)		// Skip first data bit 0
			{
				if (I2SWSHigh == true)		// Transfer left channel data
				{
					// Transmit left data
					JerryShiftTXD = JerryLatchedLTXD;
#ifdef I2S_MASTER_DATA
					WriteLog("I2S_JERRY: RTXD to shift register: %04X\n", JerryShiftTXD);
#endif
					// Receive data from I2S
					L_Audio = JerryShiftTXD;
					WriteLog("I2S_DAC: Data of shift register to L_Audio: %04X\n", L_Audio);
				}
				else	// WS is low, transfer right channel data
				{
					// Transmit right data with I2S
					JerryShiftTXD = JerryLatchedRTXD;
#ifdef I2S_MASTER_DATA
					WriteLog("I2S_JERRY: RTXD to shift register: %04X\n", JerryShiftTXD);
#endif
					// Receive data from I2S
					R_Audio = JerryShiftTXD;
					WriteLog("I2S_JERRY: Data of shift register to R_Audio: %04X\n", R_Audio);
				}
			}*/
			I2SWSBitCount++;	// Next data bit
			//WriteLog("I2S: WS bit count: %02X\n", I2SWSBitCount);
		}
	}
	else	// Jerry is I2S slave
	{	
		// Only if rising clock edge
		if (I2SClkState == STATE_RISING)
		{
			I2SClkState = STATE_HIGH;		// Set static

			//WriteLog("I2S_JERRY: Slave, WS bit count: %02X\n", I2SWSBitCount);

			// Handle serial status of Jerry if Jerry is slave
			if (I2SWSBitCount == 1)			// Generate interrupts only if WS toggled
			{
				if (I2SWSHigh == true)
				{
					// Left channel data
					// Set WS flag (bit 0) in status register
					// Set LEFT flag (bit 1) in status register
					I2S_SerialStatus = I2S_SerialStatus | 0x01 | 0x02;

					// WSEN not active if Jerry is slave and therefore not generating WS
					//if (I2S_SerialMode & 0x04))		// WSEN flag set
					//{
						// Interrupt on rising edge or every WS 
						if (I2S_SerialMode & (0x08 | 0x20))
						{
							// Generate I2S interrupts, does "IRQ enabled" checking
							DSPSetIRQLine(DSP_IRQ_I2S);
							JERRYSetIRQLine(JERRY_IRQ_I2S);
#ifdef I2S_SLAVE_IRQ
							WriteLog("I2S_JERRY: Rising WS IRQ generated\n");
#endif
						}
					//}
					//else
					//{
					//	// Jerry is slave and does not change WS
					//	WSHigh = false;
					//	// Clear WS flag in status register
					//	I2S_SerialStatus = I2S_SerialStatus & ~0x01;
					//}
				}
				else	// WS is falling
				{
					// Right channel data
					// Clear WS flag (bit 0) in status register
					// Clear LEFT flag (bit 1) in status register
					I2S_SerialStatus = I2S_SerialStatus & ~(0x01 | 0x02);

					// Interrupt on falling edge or every WS 
					if (I2S_SerialMode & (0x10 | 0x20))
					{
						// Generate I2S interrupts, does "IRQ enabled" checking
						DSPSetIRQLine(DSP_IRQ_I2S);
						JERRYSetIRQLine(JERRY_IRQ_I2S);
#ifdef I2S_SLAVE_IRQ
						WriteLog("I2S_JERRY: Falling WS IRQ generated\n");
#endif
					}
				}
			}
#ifdef I2S_SLAVE_IRQ
			if (I2SWSHigh == true)
				WriteLog("I2S_JERRY: Slave WS is rising.\n");
			else
				WriteLog("I2S_JERRY: Slave WS is falling.\n");*/
#endif
			// Handle receiving data
			if (I2SWSBitCount == 1)		// Skip first data bit 0
			{
				if (I2SWSHigh == false)	// WS just toggled one clock edge before
				{
					// Receive left channel data
					JerryLatchedLRXD = ButchShiftTXD;
#ifdef I2S_SLAVE_DATA
					WriteLog("I2S_JERRY: Data of shift register to LRXD: %04X\n", JerryLatchedLRXD);
#endif
				}
				else
				{
					// Receive right channel data
					JerryLatchedRRXD = ButchShiftTXD;
#ifdef I2S_SLAVE_DATA
					WriteLog("I2S_JERRY: Data of shift register to RRXD: %04X\n", JerryLatchedRRXD);
#endif
				}
			}
		}
	}

	/*		// Receive data
			if (I2SJerryDataBitCount > 16)	// 16 bits received
			{
				// Interrupt on every word
				if (I2S_SerialMode & 0x20)
				{
					// Generate I2S interrupts, does "IRQ enabled" checking
					DSPSetIRQLine(DSP_IRQ_I2S);
					JERRYSetIRQLine(JERRY_IRQ_I2S);
				}
			}*/
}

/*void JERRYI2SCallback(void)
{
	WriteLog("JERRY: I2SCallback\n");

	// Why is it called this? Instead of SCLK? Shouldn't this be read from DAC.CPP???
//Yes, it should. !!! FIX !!!
	jerry_i2s_interrupt_divide &= 0xFF;
	// We don't have to divide the RISC clock rate by this--the reason is a bit
	// convoluted. Will put explanation here later...
// What's needed here is to find the ratio of the frequency to the number of clock cycles
// in one second. For example, if the sample rate is 44100, we divide the clock rate by
// this: 26590906 / 44100 = 602 cycles.
// Which means, every 602 cycles that go by we have to generate an interrupt.
	jerryI2SCycles = 32 * (2 * (jerry_i2s_interrupt_divide + 1));
	WriteLog("JERRY: jerryI2SCycles: %d\n", jerryI2SCycles);

//This should be in this file with an extern reference in the header file so that
//DAC.CPP can see it... !!! FIX !!!
	extern uint16 serialMode;						// From DAC.CPP

	if (serialMode & 0x01)							// INTERNAL flag (JERRY is master)
	{
		// Generate I2S interrupts, does "IRQ enabled" checking
		DSPSetIRQLine(DSP_IRQ_I2S);
		JERRYSetIRQLine(JERRY_IRQ_I2S);
		double usecs = (float)jerryI2SCycles * RISC_CYCLE_IN_USEC;
		SetCallbackTime(JERRYI2SCallback, usecs);
	}
	else											// JERRY is slave to external word clock
	{
//Note that 44100 Hz requires samples every 22.675737 usec.
//When JERRY is slave to the word clock, we need to do interrupts either at 44.1K
//sample rate or at a 88.2K sample rate (11.332... usec).
/*		// This is just a temporary kludge to see if the CD bus mastering works
		// I.e., this is totally faked...!
// The whole interrupt system is pretty much borked and is need of an overhaul.
// What we need is a way of handling these interrupts when they happen instead of
// scanline boundaries the way it is now.
		jerry_i2s_interrupt_timer -= cycles;
		if (jerry_i2s_interrupt_timer <= 0)
		{
//This is probably wrong as well (i.e., need to check enable lines)... !!! FIX !!! [DONE]
			if (ButchIsReadyToSend())//Not sure this is right spot to check...
			{
//	return GetWordFromButchSSI(offset, who);
				SetSSIWordsXmittedFromButch();
				DSPSetIRQLine(DSPIRQ_SSI, ASSERT_LINE);
			}
			jerry_i2s_interrupt_timer += 602;
		}*/

/*		if (ButchIsReadyToSend())//Not sure this is right spot to check...
		{
//	return GetWordFromButchSSI(offset, who);
			SetSSIWordsXmittedFromButch();

			WriteLog("JERRY: Writing serial data\n");
			//JaguarWriteLong(0xF1A148, 0x01234567, UNKNOWN);
			//JaguarWriteLong(0xF1A14C, 0x89ABCDEF, UNKNOWN);

			//DSPSetIRQLine(DSPIRQ_SSI, ASSERT_LINE);
			//SetCallbackTime(JERRYI2SCallback, 1);
			//return;
		}
		SetCallbackTime(JERRYI2SCallback, 22.675737);
		//double usecs = (float)jerryI2SCycles * RISC_CYCLE_IN_USEC;
		//SetCallbackTime(JERRYI2SCallback, 1);//usecs);
*//*	}
}*/


void jerry_init(void)
{
	// I2S interface
	I2S_SerialClock = 19;	// Default is roughly 22 KHz (20774 Hz in NTSC mode)
	I2S_SerialMode = 0;
	I2S_SerialStatus = 0;
	I2SClkWaitCycle = 0;
	I2SClkState = STATE_LOW;
	I2SWSBitCount = 0;		// Word Strobe bit counter
	I2SWSHigh = false;		// Word Strobe status

//	clock_init();
//	anajoy_init();
	joystick_init();
	DACInit();
//This should be handled with the cart initialization...
//	eeprom_init();
	memory_malloc_secure((void **)&jerry_ram_8, 0x10000, "JERRY RAM/ROM");
	memcpy(&jerry_ram_8[0xD000], wave_table, 0x1000);
	jerry_reset();
	//JERRYPIT1Prescaler = 0xFFFF;
	//JERRYPIT2Prescaler = 0xFFFF;
	//JERRYPIT1Divider = 0xFFFF;
	//JERRYPIT2Divider = 0xFFFF;
}

void jerry_reset(void)
{
//	clock_reset();
//	anajoy_reset();
	joystick_reset();
	eeprom_reset();
	//JERRYResetI2S();
	DACReset();

	memset(jerry_ram_8, 0x00, 0xD000);		// Don't clear out the Wavetable ROM...!
	jerryTimer1Prescaler = 0xFFFF;
	jerryTimer2Prescaler = 0xFFFF;
	jerryTimer1Divider = 0xFFFF;
	jerryTimer2Divider = 0xFFFF;
	jerryTimer1PrescalerCounter = jerryTimer1Prescaler;
	jerryTimer2PrescalerCounter = jerryTimer2Prescaler;
	jerryTimer1DividerCounter = jerryTimer1Divider;
	jerryTimer2DividerCounter = jerryTimer2Divider;
	//jerry_timer_1_counter = 0;
	//jerry_timer_2_counter = 0;
	jerryIntControl = 0;
	jerryIntPending = 0;
}

void jerry_done(void)
{
	WriteLog("JERRY: M68K Interrupt control ($F10020) = %04X\n", GET16(jerry_ram_8, 0x20));
	memory_free(jerry_ram_8);
//	clock_done();
//	anajoy_done();
	joystick_done();
	DACDone();
	eeprom_done();
}

void JERRYSetIRQLine(int irqline)
{
	// Jerry's interrupt enable and pending bit are the same
	// so just optimizing the shifts
	uint16 bit = JERRY_INT0_ENABLE << irqline;

	//WriteLog("jerryIntControl: %04X, irqline: %04X, bit: %04X\n", jerryIntControl, irqline, bit);

	// Is interrupt enabled ?
	if (jerryIntControl & bit)
	{
		// Set the latch bit
		//jerryIntPending |= (JERRY_INT0_PENDING << irqline);
		jerryIntPending |= bit;

		//	JERRYHandleIRQs();

		// Generate JERRY interrupts, does "IRQ enable" checking
		GPUSetIRQLine(GPU_IRQ_JERRY);
		TOMSetIRQLine(TOM_IRQ_JERRY);	// Generates 68k interrupt
	}
}

/*bool JERRYIRQEnabled(int irq)
{
	// Read the word @ $F10020 
	return jerry_ram_8[0x21] & (1 << irq);
}*/

/*void JERRYSetPendingIRQ(int irq)
{
	// This is the shadow of INT (it's a split RO/WO register)
	jerryIntPending |= (1 << irq);
}*/

//
// JERRY byte access (read)
//
uint8 JERRYReadByte(uint32 address, uint32 who/*=UNKNOWN*/)
{
#ifdef JERRY_DEBUG
	WriteLog("JERRY: Reading byte at %06X\n", address);
#endif
	if ((address >= DSP_CONTROL_RAM_BASE) && (address < (DSP_CONTROL_RAM_BASE + 0x20)))
		return DSPReadByte(address, who);
	else if ((address >= DSP_WORK_RAM_BASE) && (address < (DSP_WORK_RAM_BASE + 0x2000)))
		return DSPReadByte(address, who);
	// LRXD/RRXD/SSTAT $F1A148/4C/50 (really 16-bit registers...)
	else if ((address >= 0xF1A148) && (address <= 0xF1A153))
		return DACReadByte(address, who);
//	F10036          R     xxxxxxxx xxxxxxxx   JPIT1 - timer 1 pre-scaler
//	F10038          R     xxxxxxxx xxxxxxxx   JPIT2 - timer 1 divider
//	F1003A          R     xxxxxxxx xxxxxxxx   JPIT3 - timer 2 pre-scaler
//	F1003C          R     xxxxxxxx xxxxxxxx   JPIT4 - timer 2 divider
//This is WRONG!
//	else if (offset >= 0xF10000 && offset <= 0xF10007)
//This is still wrong. What needs to be returned here are the values being counted down
//in the jerry_timer_n_counter variables... !!! FIX !!! [DONE]

//This is probably the problem with the new timer code... This is invalid
//under the new system... !!! FIX !!!
	else if ((address >= 0xF10036) && (address <= 0xF1003D))
	{
		switch(address & 0x0F)
		{
		case 0x06:
			return jerryTimer1PrescalerCounter >> 8;
		case 0x07:
			return jerryTimer1PrescalerCounter & 0xFF;
		case 0x08:
			return jerryTimer1DividerCounter >> 8;
		case 0x09:
			return jerryTimer1DividerCounter & 0xFF;
		case 0x0A:
			return jerryTimer2PrescalerCounter >> 8;
		case 0x0B:
			return jerryTimer2PrescalerCounter & 0xFF;
		case 0x0C:
			return jerryTimer2DividerCounter >> 8;
		case 0X0D:
			return jerryTimer2DividerCounter & 0xFF;
		}
	}
/*#ifndef NEW_TIMER_SYSTEM
//		jerry_timer_1_counter = (JERRYPIT1Prescaler + 1) * (JERRYPIT1Divider + 1);
		uint32 counter1Hi = (jerry_timer_1_counter / (JERRYPIT1Divider + 1)) - 1;
		uint32 counter1Lo = (jerry_timer_1_counter % (JERRYPIT1Divider + 1)) - 1;
		uint32 counter2Hi = (jerry_timer_2_counter / (JERRYPIT2Divider + 1)) - 1;
		uint32 counter2Lo = (jerry_timer_2_counter % (JERRYPIT2Divider + 1)) - 1;

		switch(address & 0x0F)
		{
		case 6:
//			return JERRYPIT1Prescaler >> 8;
			return counter1Hi >> 8;
		case 7:
//			return JERRYPIT1Prescaler & 0xFF;
			return counter1Hi & 0xFF;
		case 8:
//			return JERRYPIT1Divider >> 8;
			return counter1Lo >> 8;
		case 9:
//			return JERRYPIT1Divider & 0xFF;
			return counter1Lo & 0xFF;
		case 10:
//			return JERRYPIT2Prescaler >> 8;
			return counter2Hi >> 8;
		case 11:
//			return JERRYPIT2Prescaler & 0xFF;
			return counter2Hi & 0xFF;
		case 12:
//			return JERRYPIT2Divider >> 8;
			return counter2Lo >> 8;
		case 13:
//			return JERRYPIT2Divider & 0xFF;
			return counter2Lo & 0xFF;
		}
#else
WriteLog("JERRY: Unhandled timer read (BYTE) at %08X...\n", address);
#endif
	}*/
//	else if ((address >= 0xF10010) && (address <= 0xF10015))
//		return clock_byte_read(address);
//	else if ((address >= 0xF17C00) && (address <= 0xF17C01))
//		return anajoy_byte_read(address);
	else if ((address >= 0xF14000) && (address <= 0xF14003))
		return joystick_byte_read(address) | eeprom_byte_read(address);
	else if ((address >= 0xF14000) && (address <= 0xF1A0FF))
		return eeprom_byte_read(address);
	
	return jerry_ram_8[address & 0xFFFF];
}

void GPUDumpRegisters(void);
void DSPDumpRegisters(void);
void DSPDumpDisassembly(void);
//
// JERRY word access (read)
//
uint16 JERRYReadWord(uint32 address, uint32 who/*=UNKNOWN*/)
{
#ifdef JERRY_DEBUG
	WriteLog("JERRY: Reading word at %06X\n", address);
#endif

	if ((address >= DSP_CONTROL_RAM_BASE) && (address < DSP_CONTROL_RAM_BASE+0x20))
		return DSPReadWord(address, who);
	else if ((address >= DSP_WORK_RAM_BASE) && (address <= (DSP_WORK_RAM_BASE + 0x1FFF)))
		return DSPReadWord(address, who);
	else if ((address == LRXD) || (address == RRXD))
	{
		return 0x0000;
	}
	else if (address == (LRXD + 2))
	{
		/*if (JerryLatchedLRXD != 0x0)//== 0x5249)
		{
			DSPDumpRegisters();
	//			WriteLog("ATRI\n");
			//DSPDumpDisassembly();
		}*/
		//WriteLog("JERRY: Read LRXD + 2: %04X\n", JerryLatchedLRXD);
		return JerryLatchedLRXD;
	}
	else if (address == (RRXD + 2))
	{
		/*if (JerryLatchedRRXD != 0x0)//== 0x5249)
		{
			DSPDumpRegisters();
	//			WriteLog("ATRI\n");
			//DSPDumpDisassembly();
		}*/
		//WriteLog("JERRY: Read RRXD + 2: %04X\n", JerryLatchedRRXD);
		return JerryLatchedRRXD;
	}
	else if (address == SSTAT)
	{
		WriteLog("JERRY: Read SSTAT: 0x0000\n");
		return 0x0000;
	}
	else if (address == (SSTAT + 2))
	{
		WriteLog("JERRY: Read SSTAT + 2: %02X\n", I2S_SerialStatus);
		return I2S_SerialStatus;
	}
	else if (address == SMODE)
	{
		WriteLog("JERRY: Read SMODE: 0xFFFF\n");
		return 0xFFFF;
	}
	else if (address == (SMODE + 2))
	{
		WriteLog("JERRY: Read SMODE + 2: 0xFFFF\n");
		return 0xFFFF;
	}
	// LRXD/RRXD/SSTAT $F1A148/4C/50 (really 16-bit registers...)
	//else if ((address >= 0xF1A148) && (address <= 0xF1A153))
	//	return DACReadWord(address, who);
//	F10036          R     xxxxxxxx xxxxxxxx   JPIT1 - timer 1 pre-scaler
//	F10038          R     xxxxxxxx xxxxxxxx   JPIT2 - timer 1 divider
//	F1003A          R     xxxxxxxx xxxxxxxx   JPIT3 - timer 2 pre-scaler
//	F1003C          R     xxxxxxxx xxxxxxxx   JPIT4 - timer 2 divider
//This is WRONG!
//	else if ((offset >= 0xF10000) && (offset <= 0xF10007))
//This is still wrong. What needs to be returned here are the values being counted down
//in the jerry_timer_n_counter variables... !!! FIX !!! [DONE]
	else if ((address >= 0xF10036) && (address <= 0xF1003D))
	{
		switch (address & 0x0E)
		{
		case 0x06:
			return jerryTimer1PrescalerCounter;
		case 0x08:
			return jerryTimer1DividerCounter;
		case 0x0A:
			return jerryTimer2PrescalerCounter;
		case 0x0C:
			return jerryTimer2DividerCounter;
		}
	}
/*#ifndef NEW_TIMER_SYSTEM
//		jerry_timer_1_counter = (JERRYPIT1Prescaler + 1) * (JERRYPIT1Divider + 1);
		uint32 counter1Hi = (jerry_timer_1_counter / (JERRYPIT1Divider + 1)) - 1;
		uint32 counter1Lo = (jerry_timer_1_counter % (JERRYPIT1Divider + 1)) - 1;
		uint32 counter2Hi = (jerry_timer_2_counter / (JERRYPIT2Divider + 1)) - 1;
		uint32 counter2Lo = (jerry_timer_2_counter % (JERRYPIT2Divider + 1)) - 1;

		switch(address & 0x0F)
		{
		case 6:
//			return JERRYPIT1Prescaler;
			return counter1Hi;
		case 8:
//			return JERRYPIT1Divider;
			return counter1Lo;
		case 10:
//			return JERRYPIT2Prescaler;
			return counter2Hi;
		case 12:
//			return JERRYPIT2Divider;
			return counter2Lo;
		}
		// Unaligned word reads???
#else
WriteLog("JERRY: Unhandled timer read (WORD) at %08X...\n", address);
#endif
	}*/
//	else if ((address >= 0xF10010) && (address <= 0xF10015))
//		return clock_word_read(offset);
	else if (address == 0xF10020)
		return jerryIntPending;
	else if ((address >= 0xF10030) && (address <= 0xF10034))
	{
		return 0x0000;
	}
//	else if ((address >= 0xF17C00) && (address <= 0xF17C01))
//		return anajoy_word_read(offset);
	else if (address == 0xF14000)
		return (joystick_word_read(address) & 0xFFFE) | eeprom_word_read(address);
	else if ((address >= 0xF14002) && (address < 0xF14003))
		return joystick_word_read(address);
	else if ((address >= 0xF14000) && (address <= 0xF1A0FF))
		return eeprom_word_read(address);

//if (address >= 0xF1B000)
//	WriteLog("JERRY: Reading word at %08X [%04X]...\n", address, ((uint16)jerry_ram_8[(address+0)&0xFFFF] << 8) | jerry_ram_8[(address+1)&0xFFFF]);//*/

	uint32 offset = address & 0xFFFF;				// Prevent crashing...!
	return ((uint16)jerry_ram_8[offset + 0] << 8) | jerry_ram_8[offset + 1];
}

//
// JERRY byte access (write)
//
void JERRYWriteByte(uint32 address, uint8 data, uint32 who/*=UNKNOWN*/)
{
#ifdef JERRY_DEBUG
	WriteLog("jerry: writing byte %.2x at 0x%.6x\n", data, address);
#endif
	if ((address >= DSP_CONTROL_RAM_BASE) && (address < (DSP_CONTROL_RAM_BASE + 0x20)))
	{
		DSPWriteByte(address, data, who);
		return;
	}
	else if ((address >= DSP_WORK_RAM_BASE) && (address < (DSP_WORK_RAM_BASE + 0x2000)))
	{
		DSPWriteByte(address, data, who);
		return;
	}
	// SCLK ($F1A150--8 bits wide)
//NOTE: This should be taken care of in DAC...
/*	else if ((address >= 0xF1A152) && (address <= 0xF1A153))
	{
//		WriteLog("JERRY: Writing %02X to SCLK...\n", data);
		if ((address & 0x03) == 2)
			jerry_i2s_interrupt_divide = (jerry_i2s_interrupt_divide & 0x00FF) | ((uint32)data << 8);
		else
			jerry_i2s_interrupt_divide = (jerry_i2s_interrupt_divide & 0xFF00) | (uint32)data;

		jerry_i2s_interrupt_timer = -1;
#ifndef NEW_TIMER_SYSTEM
		jerry_i2s_exec(0);
#else
		RemoveCallback(JERRYI2SCallback);
		JERRYI2SCallback();
#endif
//		return;
	}*/
	// LTXD/RTXD/SCLK/SMODE $F1A148/4C/50/54 (really 16-bit registers...)
	else if ((address >= 0xF1A148) && (address <= 0xF1A157))
	{ 
		//DACWriteByte(address, data, who);
		return; 
	}
	else if ((address >= 0xF10000) && (address <= 0xF10007))
	{
		switch (address & 0x07)
		{
		case 0:
			jerryTimer1Prescaler = (jerryTimer1Prescaler & 0x00FF) | (data << 8);
			jerryTimer1PrescalerCounter = jerryTimer1Prescaler;
			break;
		case 1:
			jerryTimer1Prescaler = (jerryTimer1Prescaler & 0xFF00) | data;
			jerryTimer1PrescalerCounter = jerryTimer1Prescaler;
			break;
		case 2:
			jerryTimer1Divider = (jerryTimer1Divider & 0x00FF) | (data << 8);
			jerryTimer1DividerCounter = jerryTimer1Divider;
			break;
		case 3:
			jerryTimer1Divider = (jerryTimer1Divider & 0xFF00) | data;
			jerryTimer1DividerCounter = jerryTimer1Divider;
			break;
		case 4:
			jerryTimer2Prescaler = (jerryTimer2Prescaler & 0x00FF) | (data << 8);
			jerryTimer2PrescalerCounter = jerryTimer2Prescaler;
			break;
		case 5:
			jerryTimer2Prescaler = (jerryTimer2Prescaler & 0xFF00) | data;
			jerryTimer2PrescalerCounter = jerryTimer2Prescaler;
			break;
		case 6:
			jerryTimer2Divider = (jerryTimer2Divider & 0x00FF) | (data << 8);
			jerryTimer2DividerCounter = jerryTimer2Divider;
			break;
		case 7:
			jerryTimer2Divider = (jerryTimer2Divider & 0xFF00) | data;
			jerryTimer2DividerCounter = jerryTimer2Divider;
		}
		return;
	}
/*#ifndef NEW_TIMER_SYSTEM
		switch (address & 0x07)
		{
		case 0:
			JERRYPIT1Prescaler = (JERRYPIT1Prescaler & 0x00FF) | (data << 8);
			JERRYResetPIT1();
			break;
		case 1:
			JERRYPIT1Prescaler = (JERRYPIT1Prescaler & 0xFF00) | data;
			JERRYResetPIT1();
			break;
		case 2:
			JERRYPIT1Divider = (JERRYPIT1Divider & 0x00FF) | (data << 8);
			JERRYResetPIT1();
			break;
		case 3:
			JERRYPIT1Divider = (JERRYPIT1Divider & 0xFF00) | data;
			JERRYResetPIT1();
			break;
		case 4:
			JERRYPIT2Prescaler = (JERRYPIT2Prescaler & 0x00FF) | (data << 8);
			JERRYResetPIT2();
			break;
		case 5:
			JERRYPIT2Prescaler = (JERRYPIT2Prescaler & 0xFF00) | data;
			JERRYResetPIT2();
			break;
		case 6:
			JERRYPIT2Divider = (JERRYPIT2Divider & 0x00FF) | (data << 8);
			JERRYResetPIT2();
			break;
		case 7:
			JERRYPIT2Divider = (JERRYPIT2Divider & 0xFF00) | data;
			JERRYResetPIT2();
		}
#else
WriteLog("JERRY: Unhandled timer write (BYTE) at %08X...\n", address);
#endif
		return;
	}*/
/*	else if ((address >= 0xF10010) && (address <= 0xF10015))
	{
		clock_byte_write(address, data);
		return;
	}//*/
	// JERRY -> 68K interrupt enables/latches (need to be handled!)
	else if ((address >= 0xF10020) && (address <= 0xF10023))
	{
WriteLog("JERRY: (68K int en/lat - Unhandled!) Tried to write $%02X to $%08X!\n", data, address);
	}
/*	else if ((address >= 0xF17C00) && (address <= 0xF17C01))
	{
		anajoy_byte_write(address, data);
		return;
	}*/
	else if ((address >= 0xF14000) && (address <= 0xF14003))
	{
		joystick_byte_write(address, data);
		eeprom_byte_write(address, data);
		return;
	}
	else if ((address >= 0xF14000) && (address <= 0xF1A0FF))
	{
		eeprom_byte_write(address, data);
		return;
	}

//Need to protect write attempts to Wavetable ROM (F1D000-FFF)
	if ((address >= 0xF1D000) && (address <= 0xF1DFFF))
		return;

	jerry_ram_8[address & 0xFFFF] = data;
}

//
// JERRY word access (write)
//
void JERRYWriteWord(uint32 address, uint16 data, uint32 who/*=UNKNOWN*/)
{
#ifdef JERRY_DEBUG
	WriteLog( "JERRY: Writing word %04X at %06X\n", data, address);
#endif

	if ((address >= DSP_CONTROL_RAM_BASE) && (address < (DSP_CONTROL_RAM_BASE + 0x20)))
	{
		DSPWriteWord(address, data, who);
		return;
	}
	else if ((address >= DSP_WORK_RAM_BASE) && (address < (DSP_WORK_RAM_BASE + 0x2000)))
	{
		DSPWriteWord(address, data, who);
		return;
	}
	else if (address == (SCLK + 2))
	{
		I2S_SerialClock = data;
		WriteLog("JERRY: Serial clock frequency register: %04X\n", data);

		// Calculate and set SDL audio clock
		SDLAdjustAudioFrequency();
	}
	else if (address == (SMODE + 2))
	{
		I2S_SerialMode = data;
		WriteLog("JERRY: %s writing to SMODE: %04X. Bits: %s%s%s%s%s%s [68K PC=%08X]\n", whoName[who], data,
			(data & 0x01 ? "INTERNAL " : ""), (data & 0x02 ? "MODE " : ""),
			(data & 0x04 ? "WSEN " : ""), (data & 0x08 ? "RISING " : ""),
			(data & 0x10 ? "FALLING " : ""), (data & 0x20 ? "EVERYWORD" : ""),
			m68k_get_reg(NULL, M68K_REG_PC));
	}
	else if (address == LTXD)
	{
		//WriteLog("JERRY: Write LTXD: %04X\n", data);
	}
	else if (address == (LTXD + 2))
	{
		// Left channel data
		//WriteLog("JERRY: Write LTXD + 2: %04X\n", data);
		//JerryLatchedLTXD = data;
		//SDL_LockAudio();
		DACBuffer[DACBufferIndex] = data;
		DACBufferIndex++;
		//SDL_UnlockAudio();
	}
	else if (address == RTXD)
	{
		//WriteLog("JERRY: Write RTXD: %04X\n", data);
	}
	else if (address == (RTXD + 2))
	{
		// Right channel data
		//WriteLog("JERRY: Write RTXD + 2: %04X\n", data);
		//JerryLatchedRTXD = data;
		//SDL_LockAudio();
		DACBuffer[DACBufferIndex] = data;
		DACBufferIndex++;
		//SDL_UnlockAudio();
	}
	//NOTE: This should be taken care of in DAC...
/*	else if (address == (SCLK + 2))
	{
		WriteLog("JERRY: Writing %04X to SCLK (by %s)...\n", data, whoName[who]);
//This should *only* be enabled when SMODE has its INTERNAL bit set! !!! FIX !!!
		jerry_i2s_interrupt_divide = (uint8)data;
		jerry_i2s_interrupt_timer = -1;
#ifndef NEW_TIMER_SYSTEM
		jerry_i2s_exec(0);
#else
		RemoveCallback(JERRYI2SCallback);
		JERRYI2SCallback();
#endif

		DACWriteWord(address, data, who);
		return; 
	}*/
	else if ((address >= 0xF10000) && (address <= 0xF10007))
	{
		switch (address & 0x06)
		{
		case 0:
			jerryTimer1Prescaler = data; 
			jerryTimer1PrescalerCounter = data;
			break;
		case 2:
			jerryTimer1Divider = data; 
			jerryTimer1DividerCounter = data;
			break;
		case 4:
			jerryTimer2Prescaler = data; 
			jerryTimer2PrescalerCounter = data;
			break;
		case 6:
			jerryTimer2Divider = data; 
			jerryTimer2DividerCounter = data;
			break;
		}
		return;
	}
/*//#ifndef NEW_TIMER_SYSTEM
#if 1
		switch(address & 0x07)
		{
		case 0:
			JERRYPIT1Prescaler = data;
			JERRYResetPIT1();
			break;
		case 2:
			JERRYPIT1Divider = data;
			JERRYResetPIT1();
			break;
		case 4:
			JERRYPIT2Prescaler = data;
			JERRYResetPIT2();
			break;
		case 6:
			JERRYPIT2Divider = data;
			JERRYResetPIT2();
		}
		// Need to handle (unaligned) cases???
#else
WriteLog("JERRY: Unhandled timer write %04X (WORD) at %08X by %s...\n", data, address, whoName[who]);
#endif
		return;
	}*/
	/*	else if ((address >= 0xF10010) && (address < 0xF10016))
	{
		clock_word_write(address, data);
		return;
	}//*/
	// JERRY -> 68K interrupt enables/latches (need to be handled!) Done
	else if (address == 0xF10020)
	{
		// Enable interrupts, use only 'enable' bits
		jerryIntControl = jerryIntControl | (data & JERRY_INT05_ENABLE_FLAGS);
		// Clear interrupts, use only 'clear' bits
		jerryIntPending = jerryIntPending & ~((data & JERRY_INT05_CLEAR_FLAGS) >> 8);

#ifdef JERRY_DEBUG
		if (data & JERRY_INT0_ENABLE)
			WriteLog("JERRY: EINT0 interrupt enabled.\n");
		if (data & JERRY_INT1_ENABLE)
			WriteLog("JERRY: DSP interrupt enabled.\n");
		if (data & JERRY_INT2_ENABLE)
			WriteLog("JERRY: PIT1 interrupt enabled.\n");
		if (data & JERRY_INT3_ENABLE)
			WriteLog("JERRY: PIT2 interrupt enabled.\n");
		if (data & JERRY_INT4_ENABLE)
			WriteLog("JERRY: ASI interrupt enabled.\n");
		if (data & JERRY_INT5_ENABLE)
			WriteLog("JERRY: I2S interrupt enabled.\n");

		if (data & JERRY_INT0_CLEAR)
			WriteLog("JERRY: EINT0 interrupt cleared.\n");
		if (data & JERRY_INT1_CLEAR)
			WriteLog("JERRY: DSP interrupt cleared.\n");
		if (data & JERRY_INT1_CLEAR)
			WriteLog("JERRY: PIT1 interrupt cleared.\n");
		if (data & JERRY_INT3_CLEAR)
			WriteLog("JERRY: PIT2 interrupt cleared.\n");
		if (data & JERRY_INT4_CLEAR)
			WriteLog("JERRY: ASI interrupt cleared.\n");
		if (data & JERRY_INT5_CLEAR)
			WriteLog("JERRY: I2S interrupt cleared.\n");
#endif	// JERRY_DEBUG
		return;
	}
	/*	else if ((address >= 0xF17C00) && (address < 0xF17C02))
	{
//I think this was removed from the Jaguar. If so, then we don't need this...!
		anajoy_word_write(address, data);
		return;
	}*/
	else if ((address >= 0xF14000) && (address < 0xF14003))
	{
		joystick_word_write(address, data);
		eeprom_word_write(address, data);
		return;
	}
	else if ((address >= 0xF14000) && (address <= 0xF1A0FF))
	{
		eeprom_word_write(address, data);
		return;
	}

//Need to protect write attempts to Wavetable ROM (F1D000-FFF)
	if ((address >= 0xF1D000) && (address <= 0xF1DFFF))
		return;

	jerry_ram_8[(address+0) & 0xFFFF] = (data >> 8) & 0xFF;
	jerry_ram_8[(address+1) & 0xFFFF] = data & 0xFF;
}
