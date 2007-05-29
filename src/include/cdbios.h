#ifndef __CDBIOS_H__
#define __CDBIOS_H__

#include "jaguar.h"

void cd_bios_init(void);
void cd_bios_reset(void);
void cd_bios_done(void);
void cd_bios_process(uint32 cmd);
void cd_bios_boot(char *filename);
void cd_bios_exec(uint32 scanline);


#endif
