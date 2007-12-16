//
// System time handlers
//
// by James L. Hammons
//
// STILL TO DO:
//
// - Handling for an event that occurs NOW
//

#include "clock.h"

#include "log.h"

#define EVENT_LIST_SIZE       512

/*
// Current execution path:

do
{
	HandleJoystick

	// Execute one frame

    for(int i=0; i<numLines; i++)
    {
		ExecuteM68K
		ExecutePITs
		ExecuteGPU
		ExecuteDSP
		// Render Scanline
		CallObjectProcessor
		MoveLineBufToBackBuf
    }

	RenderBackbuffer
}
while (true)

// What we need to do:

Set up timers (frame [for native render--on vblank interval?], OP line, PITs)

do
{
	double timeToNextEvent = GetTimeToNextEvent();

	m68k_execute(USEC_TO_M68K_CYCLES(timeToNextEvent));
	GPUExec(USEC_TO_RISC_CYCLES(timeToNextEvent));
	DSPExec(USEC_TO_RISC_CYCLES(timeToNextEvent));

	HandleNextEvent();
}
while (true)


*/

// Now, a bit of weirdness: It seems that the number of lines displayed on the screen
// makes the effective refresh rate either 30 or 25 Hz!

// NOTE ABOUT TIMING SYSTEM DATA STRUCTURES:

// A queue won't work for this system because we can't guarantee that an event will go
// in with a time that is later than the ones already queued up. So we just use a simple
// list.

// Although if we used an insertion sort we could, but it wouldn't work for adjusting
// times...

struct Event
{
    bool valid;
    double eventTime;
    void (* timerCallback)(void);
};

Event eventList[EVENT_LIST_SIZE];
uint32 nextEvent;

void InitializeEventList(void)
{
    for(uint32 i=0; i<EVENT_LIST_SIZE; i++)
        eventList[i].valid = false;
}

//We just slap the next event into the list, no checking, no nada...
void SetCallbackTime(void (* callback)(void), double time)
{
    for(uint32 i=0; i<EVENT_LIST_SIZE; i++)
    {
        if (!eventList[i].valid)
        {
//WriteLog("SCT: Found callback slot #%u...\n", i);
            eventList[i].timerCallback = callback;
            eventList[i].eventTime = time;
            eventList[i].valid = true;

            return;
        }
    }

    WriteLog("SetCallbackTime() failed to find an empty slot in the list!\n");
}

void RemoveCallback(void (* callback)(void))
{
    for(uint32 i=0; i<EVENT_LIST_SIZE; i++)
    {
        if (eventList[i].valid && eventList[i].timerCallback == callback)
        {
            eventList[i].valid = false;

            return;
        }
    }
}

void AdjustCallbackTime(void (* callback)(void), double time)
{
    for(uint32 i=0; i<EVENT_LIST_SIZE; i++)
    {
        if (eventList[i].valid && eventList[i].timerCallback == callback)
        {
            eventList[i].eventTime = time;

            return;
        }
    }
}

double GetTimeToNextEvent(void)
{
    double time = 0;
    bool firstTime = true;

    for(uint32 i=0; i<EVENT_LIST_SIZE; i++)
    {
        if (eventList[i].valid)
        {
            if (firstTime)
                time = eventList[i].eventTime, nextEvent = i, firstTime = false;
            else
            {
                if (eventList[i].eventTime < time)
                    time = eventList[i].eventTime, nextEvent = i;
            }
        }
    }

    return time;
}

void HandleNextEvent(void)
{
    double elapsedTime = eventList[nextEvent].eventTime;
    void (* event)(void) = eventList[nextEvent].timerCallback;

    for(uint32 i=0; i<EVENT_LIST_SIZE; i++)
        if (eventList[i].valid)
            eventList[i].eventTime -= elapsedTime;

    eventList[nextEvent].valid = false;      // Remove event from list...

    (*event)();
}


/*
void OPCallback(void)
{
    DoFunkyOPStuffHere();

    SetCallbackTime(OPCallback, HORIZ_PERIOD_IN_USEC);
}

void VICallback(void)
{
    double oneFrameInUsec = 16666.66666666;
    SetCallbackTime(VICallback, oneFrameInUsec / numberOfLines);
}

void JaguarInit(void)
{
    double oneFrameInUsec = 16666.66666666;
    SetCallbackTime(VICallback, oneFrameInUsec / numberOfLines);
    SetCallbackTime(OPCallback, );
}

void JaguarExec(void)
{
    while (true)
    {
        double timeToNextEvent = GetTimeToNextEvent();

        m68k_execute(USEC_TO_M68K_CYCLES(timeToNextEvent));
        GPUExec(USEC_TO_RISC_CYCLES(timeToNextEvent));
        DSPExec(USEC_TO_RISC_CYCLES(timeToNextEvent));

        if (!HandleNextEvent())
            break;
    }
}

// NOTES: The timers count RISC cycles, and when the dividers count down to zero they can interrupt either the DSP and/or CPU.

// NEW:
// TOM Programmable Interrupt Timer handler
// NOTE: TOM's PIT is only enabled if the prescaler is != 0
//       The PIT only generates an interrupt when it counts down to zero, not when loaded!

void TOMResetPIT()
{
    // Need to remove previous timer from the queue, if it exists...
    RemoveCallback(TOMPITCallback);

    if (TOMPITPrescaler)
    {
        double usecs = (TOMPITPrescaler + 1) * (TOMPITDivider + 1) * RISC_CYCLE_IN_USEC;
        SetCallbackTime(TOMPITCallback, usecs);
    }
}

void TOMPITCallback(void)
{
    INT1_RREG |= 0x08;                         // Set TOM PIT interrupt pending
    GPUSetIRQLine(GPUIRQ_TIMER, ASSERT_LINE);  // It does the 'IRQ enabled' checking

    if (INT1_WREG & 0x08)
        m68k_set_irq(7);                       // Generate 68K NMI

    TOMResetPIT();
}

// Small problem with this approach: If a timer interrupt is already pending,
// the pending timer needs to be replaced with the new one! (Taken care of above, BTW...)

TOMWriteWord(uint32 address, uint16 data)
{
    if (address == PIT0)
    {
        TOMPITPrescaler = data;
        TOMResetPIT();
    }
    else if (address == PIT1)
    {
        TOMPITDivider = data;
        TOMResetPIT();
    }
}

*/
