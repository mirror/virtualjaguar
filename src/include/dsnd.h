#ifndef __DSOUND_H__
#define __DSOUND_H__


void ws_audio_init(void);
void ws_audio_reset(void);
void ws_audio_done(void);
int  ws_audio_seal_init(void);
void ws_audio_seal_done(void);
void ws_audio_clear_channel(int Channel);
int  ws_audio_play_channel(int Channel);
int  ws_audio_stop_channel(int Channel);
void ws_write_state(int fp);
void ws_read_state(int fp);
void system_sound_clear(void);
void system_sound_shutdown(void);
void system_sound_update(void);


#endif
