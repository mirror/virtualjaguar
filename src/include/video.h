//
// VIDEO.H: Header file
//

#ifndef __VIDEO_H__
#define __VIDEO_H__

#define VIRTUAL_SCREEN_WIDTH		320
#define VIRTUAL_SCREEN_HEIGHT_NTSC	240
#define VIRTUAL_SCREEN_HEIGHT_PAL	256

bool InitVideo(void);
void VideoDone(void);
void RenderBackbuffer(void);
void ResizeScreen(uint32 width, uint32 height);
uint32 GetSDLScreenPitch(void);
void ToggleFullscreen(void);

#endif	// __VIDEO_H__
