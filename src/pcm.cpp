//
// PCM Handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "pcm.h"

//#define PCM_DUMP

#define sample_rate 44100
#define sample_bits 16
#define buffer_size 4
#define buffer_modulo (sample_rate * buffer_size)

static int16 * pcm_left;
static int16 * pcm_right;
static uint32 pcm_left_pos;
static uint32 pcm_right_pos;
static uint32 pcm_left_playback_pos;
static uint32 pcm_right_playback_pos;
static uint32 pcm_sample_rate = sample_rate;
static uint32 pcm_inc = (pcm_sample_rate << 8) / sample_rate;
#ifdef PCM_DUMP
static FILE * fp_left;
static FILE * fp_right;
#endif


void pcm_set_sample_rate(int rate)
{
	pcm_sample_rate = rate;
	pcm_inc = (pcm_sample_rate << 8) / sample_rate;
//	fprintf(log_get(),"pcm: sample rate is %i hz, sample increment is %i (%f)\n",pcm_sample_rate,pcm_inc,((float)pcm_inc)/256.0f);
}

void pcm_updateOne(int channel, int16 * data, uint32 length)
{
	if (channel == 0)
	{
		while (length)
		{
			*data++ = pcm_left[(pcm_left_playback_pos >> 8) % buffer_modulo];
			pcm_left_playback_pos += pcm_inc;
			length--;
		}
	}
	else
	{
		while (length)
		{
			*data++ = pcm_right[(pcm_right_playback_pos >> 8) % buffer_modulo];
			pcm_right_playback_pos += pcm_inc;
			length--;
		}
	}
}

void pcm_init(void)
{
	memory_malloc_secure((void **)&pcm_left, buffer_modulo * sizeof(int16), "Left dac buffer");
	memory_malloc_secure((void **)&pcm_right, buffer_modulo * sizeof(int16), "Right dac buffer");
	pcm_reset();
#ifdef PCM_DUMP
	fp_left = fopen("leftdac.raw", "wb");
	fp_right = fopen("rightdac.raw", "wb");
#endif
}

void pcm_reset(void)
{
	pcm_left_pos = 0;
	pcm_right_pos = 0;
	pcm_left_playback_pos = 0;
	pcm_right_playback_pos = 0;
}

void pcm_done(void)
{
#ifdef PCM_DUMP
	fclose(fp_left);
	fclose(fp_right);
#endif
	fprintf(log_get(), "PCM: Done.\n");
}

void pcm_update(void)
{
}

void pcm_render_left_dac(void)
{
#ifdef PCM_DUMP
	fwrite(pcm_left, 1, sample_rate * 2, fp_left);
#endif
}

void pcm_render_right_dac(void)
{
#ifdef PCM_DUMP
	fwrite(pcm_right, 1, sample_rate * 2, fp_right);
#endif
}

void pcm_byte_write(uint32 offset, uint8 data)
{
//	fprintf(log_get(),"pcm: writing 0x%.2x at 0x%.8x\n",data,offset);
}

void pcm_word_write(uint32 offset, uint16 data)
{
	if (offset == 2)
	{
		pcm_left[pcm_left_pos % buffer_modulo] = data;
		pcm_left_pos++;
		if ((pcm_left_pos % buffer_modulo) == 0)
			pcm_render_left_dac();
	}
	else
	if (offset == 6)
	{
		pcm_right[pcm_right_pos % buffer_modulo] = data;
		pcm_right_pos++;
		if ((pcm_right_pos % buffer_modulo) == 0)
			pcm_render_right_dac();
	}
}

uint8 pcm_byte_read(uint32 offset)
{
//	fprintf(log_get(),"pcm: reading byte from 0x%.8x\n",offset);
	return 0xFF;
}

uint16 pcm_word_read(uint32 offset)
{
//	fprintf(log_get(),"pcm: reading word from 0x%.8x\n",offset);
	return 0xFFFF;
}
