//
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Removal of unsafe macros and addition of typdefs by James L. Hammons
//

#ifndef __TYPES_H__
#define __TYPES_H__

// This is only good on certain intel 32-bit platforms...
// You may need to tweak to suit your specific platform.

typedef unsigned char		uint8;
typedef signed char			int8;
typedef unsigned short		uint16;
typedef signed short		int16;
typedef unsigned			uint32;
typedef signed				int32;
typedef unsigned long long	uint64;
typedef signed long long	int64;

typedef uint8	UINT8;
typedef int8	INT8;
typedef uint16	UINT16;
typedef int16	INT16;
typedef uint32	UINT32;
typedef int32	INT32;
typedef uint64	UINT64;
typedef int64	INT64;

// Read/write tracing enumeration

enum { UNKNOWN, JAGUAR, DSP, GPU, TOM, JERRY, M68K, BLITTER, OP };

#endif	// __TYPES_H__
