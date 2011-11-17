//
// EVENT.H: System timing support functionality
//
// by James L. Hammons
//

#ifndef __EVENT_H__
#define __EVENT_H__

#include "types.h"

#define RISC_CYCLE_IN_USEC        0.03760684198
#define M68K_CYCLE_IN_USEC        (RISC_CYCLE_IN_USEC * 2)

#define HORIZ_PERIOD_IN_USEC_NTSC 63.555555555
#define HORIZ_PERIOD_IN_USEC_PAL  64.0

#define USEC_TO_RISC_CYCLES(u) (uint32)(((u) / RISC_CYCLE_IN_USEC) + 0.5)
#define USEC_TO_M68K_CYCLES(u) (uint32)(((u) / M68K_CYCLE_IN_USEC) + 0.5)

void InitializeEventList(void);
void SetCallbackTime(void (* callback)(void), double time);
void RemoveCallback(void (* callback)(void));
void AdjustCallbackTime(void (* callback)(void), double time);
double GetTimeToNextEvent(void);
void HandleNextEvent(void);

#endif	// __EVENT_H__
