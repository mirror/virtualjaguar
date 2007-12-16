//
// MEMORY.H: Header file
//

#ifndef __MEMORY_H__
#define __MEMORY_H__

//This module needs some serious cleanup. !!! FIX !!!
#include <stdio.h>								// For FILE struct
#include "types.h"

void InitMemory(void);
void MemoryDone(void);
void memory_malloc_secure(void ** new_ptr, uint32 size, const char * info);
//void * memory_malloc_secure2(uint32 size, const char * info);
void * memory_malloc(uint32 size, const char * info);
void memory_free(void * ptr);
void memory_memoryUsage(FILE * fp);

#endif
