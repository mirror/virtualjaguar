//
// OS agnostic CDROM interface functions
//
// by James L. Hammons
//
// This file is basically a shell to keep the front-end clean and also pull in the
// appropriate back-end code depending on which target is being compiled for.
//

#include "cdintf.h"									// Every OS has to implement these

// OS dependent implementations

#if defined(__GCCWIN32__)

#include "cdintf_win32.cpp"

#elif defined(__GCCUNIX__)
	#if defined(_OSX_)

#include "cdintf_osx.cpp"

	#else

#include "cdintf_linux.cpp"

	#endif
#endif
