// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Removal of unsafe macros and addition of typdefs by James L. Hammons
//

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __PORT__
#include <string.h>
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
#ifdef __GCCUNIX__
typedef long long		__int64;		// Could probably remove this crap with some judicious pruning
#endif	// #ifdef __GCCUNIX__
typedef int				HWND;
#else
#include <windows.h>
#endif	// #ifdef __PORT__

// This is only good on certain intel 32-bit platforms...
// You may need to tweak to suit your specific platform.

typedef unsigned char		UINT8;
typedef signed char			INT8;
typedef unsigned short		UINT16;
typedef signed short		INT16;
typedef unsigned long		UINT32;
typedef signed long			INT32;
typedef unsigned long long	UINT64;
typedef signed long long	INT64;

typedef UINT8	uint8;
typedef INT8	int8;
typedef UINT16	uint16;
typedef INT16	int16;
typedef UINT32	uint32;
typedef INT32	int32;
typedef UINT64	uint64;
typedef INT64	int64;

#endif	// #ifndef __TYPES_H__
