//
// Memory handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "memory.h"

typedef struct sMemBlockInfo
{
    void * ptr;
    char * info;
    UINT32 size;
    struct sMemBlockInfo * next;
    struct sMemBlockInfo * prev;
} sMemBlockInfo;

sMemBlockInfo memoryInfo;
UINT32 memoryMaxAllocated;
UINT32 currentAllocatedMemory;
UINT32 maximumAllocatedMemory;


void memory_addMemInfo(void * ptr, UINT32 size, char * info)
{
    sMemBlockInfo * alias;

    alias = &memoryInfo;
    while (alias->next)
    	alias=alias->next;
    alias->next = (sMemBlockInfo *)malloc(sizeof(sMemBlockInfo));
    if (alias->next == NULL)
    {
		exit(0);
        return;
    }
    alias->next->prev = alias;
    alias = alias->next;
    alias->next = NULL;
    alias->size = size;
    alias->ptr = ptr;
    alias->info = info;
}

void memory_init(void)
{
    memoryInfo.next = NULL;
    memoryInfo.prev = NULL;
    currentAllocatedMemory = 0;
    maximumAllocatedMemory = 0;
}

void * memory_malloc(UINT32 size, char * info)
{
    void * ptr;

    ptr = (void *)malloc(size);
    if (ptr == NULL)
        return NULL;

    memory_addMemInfo(ptr, size, info);
    currentAllocatedMemory += size;

    if (currentAllocatedMemory > maximumAllocatedMemory)
        maximumAllocatedMemory = currentAllocatedMemory;

    return ptr;
}

void memory_malloc_secure(void ** new_ptr, UINT32 size, char * info)
{
    void * ptr;

    fprintf(log_get(), "memory: allocating %i bytes of memory for <%s>...", size, (info == NULL) ? "unknown" : info);
    ptr = (void *)malloc(size);
    if (ptr == NULL)
    {
		fprintf(log_get(), "failed\n");
		log_done();
		exit(0);
    }
    memory_addMemInfo(ptr, size, info);
    currentAllocatedMemory += size;
    if (currentAllocatedMemory > maximumAllocatedMemory)
        maximumAllocatedMemory = currentAllocatedMemory;
    *new_ptr = ptr;
	fprintf(log_get(), "ok\n");
}

void memory_memoryUsage(FILE * fp)
{
    sMemBlockInfo * alias;
    UINT32 total = 0;

    fprintf(fp, "Memory usage:\n");
    alias = &memoryInfo;
    alias = alias->next;
    while (alias)
    {
        fprintf(fp, "\t%16i bytes : <%s> (@ 0x%.8x)\n", alias->size, alias->info, alias->ptr);
        total += alias->size;
        alias = alias->next;
    }
    fprintf(fp, "\n\t%16i bytes total(%i Mb)\n", total, total >> 20);
    fprintf(fp, "\n\t%16i bytes memory peak(%i Mb)\n", maximumAllocatedMemory, maximumAllocatedMemory >> 20);
}

void memory_done(void)
{
}

void memory_free(void * ptr)
{
    sMemBlockInfo * alias;

    alias = &memoryInfo;
    alias = alias->next;
    while (alias->ptr != ptr)
        alias = alias->next;
    free(ptr);
    currentAllocatedMemory -= alias->size;
    alias->prev->next = alias->next;
    if (alias->next != NULL)
        alias->next->prev = alias->prev;
    free(alias);
}
