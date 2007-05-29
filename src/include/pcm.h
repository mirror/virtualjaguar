#ifndef __PCM_H__
#define __PCM_H__

#include "jaguar.h"

void pcm_init(void);
void pcm_reset(void);
void pcm_done(void);

void pcm_update(void);
void pcm_byte_write(uint32 offset, uint8 data);
void pcm_word_write(uint32 offset, uint16 data);
uint8 pcm_byte_read(uint32 offset);
uint16 pcm_word_read(uint32 offset);
void pcm_updateOne(int channel, int16 * data, uint32 length);
void pcm_set_sample_rate(int rate);

#endif
