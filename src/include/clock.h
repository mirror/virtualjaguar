//
// CLOCK.H: System timing support functionality
//
// by James L. Hammons
//

#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "types.h"

// Note that these are NTSC timings:

#define RISC_CYCLE_IN_USEC     0.03760684198
#define M68K_CYCLE_IN_USEC     (RISC_CYCLE_IN_USEC * 2)
#define HORIZ_PERIOD_IN_USEC   63.5555

#define USEC_TO_RISC_CYCLES(u) (uint32)(((u) / RISC_CYCLE_IN_USEC) + 0.5)
#define USEC_TO_M68K_CYCLES(u) (uint32)(((u) / M68K_CYCLE_IN_USEC) + 0.5)

void InitializeEventList(void);
void SetCallbackTime(void (* callback)(void), double time);
void RemoveCallback(void (* callback)(void));
void AdjustCallbackTime(void (* callback)(void), double time);
double GetTimeToNextEvent(void);
void HandleNextEvent(void);

#endif	// __CLOCK_H__
