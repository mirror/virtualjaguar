//
// JERRY Core
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//
//	------------------------------------------------------------
//	JERRY REGISTERS (Mapped by Aaron Giles)
//	------------------------------------------------------------
//	F10000-F13FFF   R/W   xxxxxxxx xxxxxxxx   Jerry
//	F10000            W   xxxxxxxx xxxxxxxx   JPIT1 - timer 1 pre-scaler
//	F10004            W   xxxxxxxx xxxxxxxx   JPIT2 - timer 1 divider
//	F10008            W   xxxxxxxx xxxxxxxx   JPIT3 - timer 2 pre-scaler
//	F1000C            W   xxxxxxxx xxxxxxxx   JPIT4 - timer 2 divider
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
//	------------------------------------------------------------
//	F14000-F17FFF   R/W   xxxxxxxx xxxxxxxx   Joysticks and GPIO0-5
//	F14000          R     xxxxxxxx xxxxxxxx   JOYSTICK - read joystick state
//	F14000            W   x------- xxxxxxxx   JOYSTICK - latch joystick output
//	                  W   x------- --------      (enable joystick outputs)
//	                  W   -------- xxxxxxxx      (joystick output data)
//	F14002          R     xxxxxxxx xxxxxxxx   JOYBUTS - button register
//	F14800-F14FFF   R/W   xxxxxxxx xxxxxxxx   GPI00 - reserved
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

#include "jerry.h"
#include "wavetable.h"
#include <math.h>

//#define JERRY_DEBUG

static uint8 * jerry_ram_8;

#define JERRY_CONFIG	0x4002

uint8 analog_x, analog_y;

static uint32 jerry_timer_1_prescaler;
static uint32 jerry_timer_2_prescaler;
static uint32 jerry_timer_1_divider;
static uint32 jerry_timer_2_divider;
static int32 jerry_timer_1_counter;
static int32 jerry_timer_2_counter;

static uint32 jerry_i2s_interrupt_divide = 8;
static int32  jerry_i2s_interrupt_timer = -1;
static int32  jerry_i2s_interrupt_cycles_per_scanline = 0;


void jerry_i2s_exec(uint32 cycles)
{	
	jerry_i2s_interrupt_divide &= 0xFF;

	if (jerry_i2s_interrupt_timer == -1)
	{
		uint32 jerry_i2s_int_freq = (26591000 / 64) / (jerry_i2s_interrupt_divide + 1);
		jerry_i2s_interrupt_cycles_per_scanline = 13300000 / jerry_i2s_int_freq;
		jerry_i2s_interrupt_timer = jerry_i2s_interrupt_cycles_per_scanline;
		//WriteLog("jerry: i2s interrupt rate set to %i hz (every %i cpu clock cycles) jerry_i2s_interrupt_divide=%i\n",jerry_i2s_int_freq,jerry_i2s_interrupt_cycles_per_scanline,jerry_i2s_interrupt_divide);
		pcm_set_sample_rate(jerry_i2s_int_freq);
	}
	jerry_i2s_interrupt_timer -= cycles;
	// note : commented since the sound doesn't work properly else
	if (1)//jerry_i2s_interrupt_timer<=0)
	{
		// i2s interrupt
		dsp_check_if_i2s_interrupt_needed();
		//WriteLog("jerry_i2s_interrupt_timer=%i, generating an i2s interrupt\n",jerry_i2s_interrupt_timer);
		jerry_i2s_interrupt_timer += jerry_i2s_interrupt_cycles_per_scanline;
	}
}

void jerry_reset_i2s_timer(void)
{
	//WriteLog("i2s: reseting\n");
	jerry_i2s_interrupt_divide = 8;
	jerry_i2s_interrupt_timer = -1;
}

void jerry_reset_timer_1(void)
{
	if (!jerry_timer_1_prescaler || !jerry_timer_1_divider)
		jerry_timer_1_counter = 0;
	else
		jerry_timer_1_counter = (1 + jerry_timer_1_prescaler) * (1 + jerry_timer_1_divider);

//	if (jerry_timer_1_counter)
//		WriteLog("jerry: reseting timer 1 to 0x%.8x (%i)\n",jerry_timer_1_counter,jerry_timer_1_counter);
}

void jerry_reset_timer_2(void)
{
	if (!jerry_timer_2_prescaler || !jerry_timer_2_divider)
	{
		jerry_timer_2_counter = 0;
		return;
	}
	else
		jerry_timer_2_counter = ((1 + jerry_timer_2_prescaler) * (1 + jerry_timer_2_divider));

//	if (jerry_timer_2_counter)
//		WriteLog("jerry: reseting timer 2 to 0x%.8x (%i)\n",jerry_timer_2_counter,jerry_timer_2_counter);
}

void jerry_pit_exec(uint32 cycles)
{
	if (jerry_timer_1_counter)
		jerry_timer_1_counter -= cycles;

	if (jerry_timer_1_counter <= 0)
	{
		dsp_set_irq_line(2, 1);
		jerry_reset_timer_1();
	}

	if (jerry_timer_2_counter)
		jerry_timer_2_counter -= cycles;

	if (jerry_timer_2_counter <= 0)
	{
		dsp_set_irq_line(3, 1);
		jerry_reset_timer_2();
	}
}

void jerry_init(void)
{
	clock_init();
	anajoy_init();
	joystick_init();
//This should be handled with the cart initialization...
//	eeprom_init();
	memory_malloc_secure((void **)&jerry_ram_8, 0x10000, "JERRY RAM/ROM");
	memcpy(&jerry_ram_8[0xD000], wave_table, 0x1000);

/*for(int i=0; i<0x1000; i++)
	WriteLog("WT byte, JERRY byte: %02X, %02X\n", wave_table[i], jerry_ram_8[0xD000+i]);//*/
}

void jerry_reset(void)
{
	//WriteLog("jerry_reset()\n");
	clock_reset();
	anajoy_reset();
	joystick_reset();
	eeprom_reset();
	jerry_reset_i2s_timer();

	memset(jerry_ram_8, 0x00, 0xD000);		// Don't clear out the Wavetable ROM...!
	jerry_ram_8[JERRY_CONFIG+1] |= 0x10;	// NTSC (bit 4)
	jerry_timer_1_prescaler = 0xFFFF;
	jerry_timer_2_prescaler = 0xFFFF;
	jerry_timer_1_divider = 0xFFFF;
	jerry_timer_2_divider = 0xFFFF;
	jerry_timer_1_counter = 0;
	jerry_timer_2_counter = 0;

}

void jerry_done(void)
{
	//WriteLog("jerry_done()\n");
	memory_free(jerry_ram_8);
	clock_done();
	anajoy_done();
	joystick_done();
	eeprom_done();
}

//
// JERRY byte access (read)
//

unsigned jerry_byte_read(unsigned int offset)
{
#ifdef JERRY_DEBUG
	WriteLog("JERRY: Reading byte at %06X\n", offset);
#endif
	if ((offset >= DSP_CONTROL_RAM_BASE) && (offset < DSP_CONTROL_RAM_BASE+0x20))
		return dsp_byte_read(offset);
	else if ((offset >= DSP_WORK_RAM_BASE) && (offset < DSP_WORK_RAM_BASE+0x2000))
		return dsp_byte_read(offset);
	else if (offset >= 0xF10000 && offset <= 0xF10007)
	{
		switch(offset & 0x07)
		{
		case 0:
			return jerry_timer_1_prescaler >> 8;
		case 1:
			return jerry_timer_1_prescaler & 0xFF;
		case 2:
			return jerry_timer_1_divider >> 8;
		case 3:
			return jerry_timer_1_divider & 0xFF;
		case 4:
			return jerry_timer_2_prescaler >> 8;
		case 5:
			return jerry_timer_2_prescaler & 0xFF;
		case 6:
			return jerry_timer_2_divider >> 8;
		case 7:
			return jerry_timer_2_divider & 0xFF;
		}
	}
	else if (offset >= 0xF10010 && offset <= 0xf10015)
		return clock_byte_read(offset);
	else if (offset >= 0xF17C00 && offset <= 0xF17C01)
		return anajoy_byte_read(offset);
	else if (offset >= 0xF14000 && offset <= 0xF14003)
	{
		return joystick_byte_read(offset) | eeprom_byte_read(offset);
	}
	else if (offset >= 0xF14000 && offset <= 0xF1A0FF)
		return eeprom_byte_read(offset);
	
	return jerry_ram_8[offset & 0xFFFF];
}

//
// JERRY word access (read)
//

unsigned jerry_word_read(unsigned int offset)
{
#ifdef JERRY_DEBUG
	WriteLog("JERRY: Reading word at %06X\n", offset);
#endif

	if ((offset >= DSP_CONTROL_RAM_BASE) && (offset < DSP_CONTROL_RAM_BASE+0x20))
		return dsp_word_read(offset);
	else if ((offset >= DSP_WORK_RAM_BASE) && (offset < DSP_WORK_RAM_BASE+0x2000))
		return dsp_word_read(offset);
	else if ((offset >= 0xF10000) && (offset <= 0xF10007))
	{
		switch(offset & 0x07)
		{
		case 0:
			return jerry_timer_1_prescaler;
		case 2:
			return jerry_timer_1_divider;
		case 4:
			return jerry_timer_2_prescaler;
		case 6:
			return jerry_timer_2_divider;
		}
		// Unaligned word reads???
	}
	else if ((offset >= 0xF10010) && (offset <= 0xF10015))
		return clock_word_read(offset);
	else if (offset == 0xF10020)
		return 0x00;
	else if ((offset >= 0xF17C00) && (offset <= 0xF17C01))
		return anajoy_word_read(offset);
	else if (offset == 0xF14000)
	{
		//WriteLog("reading 0x%.4x from 0xf14000\n");
		return (joystick_word_read(offset) & 0xFFFE) | eeprom_word_read(offset);
	}
	else if ((offset >= 0xF14002) && (offset < 0xF14003))
		return joystick_word_read(offset);
	else if ((offset >= 0xF14000) && (offset <= 0xF1A0FF))
		return eeprom_word_read(offset);

// This is never executed!
/*	offset &= 0xFFFF;
	if (offset==0x4002)
		return(0xffff);*/

/*if (offset >= 0xF1D000)
	WriteLog("JERRY: Reading word at %08X [%04X]...\n", offset, ((uint16)jerry_ram_8[(offset+0)&0xFFFF] << 8) | jerry_ram_8[(offset+1)&0xFFFF]);//*/

	offset &= 0xFFFF;				// Prevent crashing...!
	return ((uint16)jerry_ram_8[offset+0] << 8) | jerry_ram_8[offset+1];
}

//
// JERRY byte access (write)
//

void jerry_byte_write(unsigned offset, unsigned data)
{
#ifdef JERRY_DEBUG
	WriteLog("jerry: writing byte %.2x at 0x%.6x\n",data,offset);
#endif
	if ((offset >= DSP_CONTROL_RAM_BASE) && (offset < DSP_CONTROL_RAM_BASE+0x20))
	{
		dsp_byte_write(offset, data);
		return;
	}
	else if ((offset >= DSP_WORK_RAM_BASE) && (offset < DSP_WORK_RAM_BASE+0x2000))
	{
		dsp_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF1A152) && (offset <= 0xF1A153))
	{
//		WriteLog("i2s: writing 0x%.2x to SCLK\n",data);
		if ((offset & 0x03) == 2)
			jerry_i2s_interrupt_divide = (jerry_i2s_interrupt_divide & 0x00FF) | ((uint32)data << 8);
		else
			jerry_i2s_interrupt_divide = (jerry_i2s_interrupt_divide & 0xFF00) | (uint32)data;

		jerry_i2s_interrupt_timer = -1;
		jerry_i2s_exec(0);
		return;
	}
	else if ((offset >= 0xF10000) && (offset <= 0xF10007))
	{
		switch(offset & 0x07)
		{
		case 0:
			jerry_timer_1_prescaler = (jerry_timer_1_prescaler & 0x00FF) | (data << 8);
			jerry_reset_timer_1();
			break;
		case 1: { jerry_timer_1_prescaler=(jerry_timer_1_prescaler&0xff00)|(data);		jerry_reset_timer_1(); return; }
		case 2: { jerry_timer_1_divider=(jerry_timer_1_divider&0x00ff)|(data<<8);		jerry_reset_timer_1(); return; }
		case 3: { jerry_timer_1_divider=(jerry_timer_1_divider&0xff00)|(data);			jerry_reset_timer_1(); return; }
		case 4: { jerry_timer_2_prescaler=(jerry_timer_2_prescaler&0x00ff)|(data<<8);	jerry_reset_timer_2(); return; }
		case 5: { jerry_timer_2_prescaler=(jerry_timer_2_prescaler&0xff00)|(data);		jerry_reset_timer_2(); return; }
		case 6: { jerry_timer_2_divider=(jerry_timer_2_divider&0x00ff)|(data<<8);		jerry_reset_timer_2(); return; }
		case 7: { jerry_timer_2_divider=(jerry_timer_2_divider&0xff00)|(data);			jerry_reset_timer_2(); return; }
		}
		return;
	}
	else if ((offset >= 0xF10010) && (offset <= 0xF10015))
	{
		clock_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF17C00) && (offset <= 0xF17C01))
	{
		anajoy_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF14000) && (offset <= 0xF14003))
	{
		joystick_byte_write(offset, data);
		eeprom_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF14000) && (offset <= 0xF1A0FF))
	{
		eeprom_byte_write(offset, data);
		return;
	}

//Need to protect write attempts to Wavetable ROM (F1D000-FFF)
	if (offset >= 0xF1D000 && offset <= 0xF1DFFF)
		return;

	jerry_ram_8[offset & 0xFFFF] = data;
}

//
// JERRY word access (write)
//

void jerry_word_write(unsigned offset, unsigned data)
{
#ifdef JERRY_DEBUG
	WriteLog( "JERRY: Writing word %04X at %06X\n", data, offset);
#endif

	if ((offset >= DSP_CONTROL_RAM_BASE) && (offset < DSP_CONTROL_RAM_BASE+0x20))
	{
		dsp_word_write(offset, data);
		return;
	}
	else if ((offset >= DSP_WORK_RAM_BASE) && (offset < DSP_WORK_RAM_BASE+0x2000))
	{
		dsp_word_write(offset, data);
		return;
	}
	else if (offset == 0xF1A152)
	{
//		WriteLog("i2s: writing 0x%.4x to SCLK\n",data);
		jerry_i2s_interrupt_divide = data & 0xFF;
		jerry_i2s_interrupt_timer = -1;
		jerry_i2s_exec(0);
	}
	else if ((offset >= 0xF10000) && (offset <= 0xF10007))
	{
		switch(offset & 0x07)
		{
		case 0:
			jerry_timer_1_prescaler = data;
			jerry_reset_timer_1();
			break;
		case 2:
			jerry_timer_1_divider = data;
			jerry_reset_timer_1();
			break;
		case 4:
			jerry_timer_2_prescaler = data;
			jerry_reset_timer_2();
			break;
		case 6:
			jerry_timer_2_divider = data;
			jerry_reset_timer_2();
		}
		// Need to handle (unaligned) cases???
		return;
	}
	else if ((offset >= 0xF1A148) && (offset < 0xF1A150)) 
	{ 
		pcm_word_write(offset - 0xF1A148, data); 
		return; 
	}
	else if ((offset >= 0xF10010) && (offset < 0xF10016))
	{
		clock_word_write(offset, data);
		return;
	}
	else if ((offset >= 0xF17C00) && (offset < 0xF17C02))
	{
		anajoy_word_write(offset, data);
		return;
	}
	else if ((offset >= 0xF14000) && (offset < 0xF14003))
	{
		joystick_word_write(offset, data);
		eeprom_word_write(offset, data);
		return;
	}
	else if ((offset >= 0xF14000) && (offset <= 0xF1A0FF))
	{
		eeprom_word_write(offset, data);
		return;
	}

//Need to protect write attempts to Wavetable ROM (F1D000-FFF)
	if (offset >= 0xF1D000 && offset <= 0xF1DFFF)
		return;

	jerry_ram_8[(offset+0) & 0xFFFF] = (data >> 8) & 0xFF;
	jerry_ram_8[(offset+1) & 0xFFFF] = data & 0xFF;
}
