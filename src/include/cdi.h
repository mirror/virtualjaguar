#ifndef __CDI_H__
#define __CDI_H__

#include "jaguar.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef __PORT__
#include <io.h>
#endif
#include <fcntl.h>

struct s_cdi_track
{
	uint8	filename_length;
	uint32	pregap_length;
	uint32  length;
	uint32	mode;
	uint32	start_lba;
	uint32  total_length;
	uint32	sector_size;
	uint32	sector_size_value;
	uint32	position;
};

struct s_cdi_session
{
	uint16 nb_of_tracks;
	s_cdi_track *tracks;
};

struct s_cdi_descriptor
{
	uint32 length;
	uint32 version;
	uint32 header_offset;
	uint16 nb_of_sessions;
	s_cdi_session *sessions;
};

extern int				cdi_fp;
extern uint32			cdi_load_address;
extern uint32			cdi_code_length;
extern s_cdi_descriptor	*cdi_descriptor;
extern s_cdi_track		**cdi_tracks;
extern uint32 			cdi_tracks_count;

int cdi_open(char *path);
void cdi_close(int fp);
s_cdi_descriptor *cdi_get_descriptor(int fp, FILE *stdfp);
void cdi_dump_descriptor(FILE *fp,s_cdi_descriptor *cdi_descriptor);
uint8 *cdi_extract_boot_code(int fp, s_cdi_descriptor *cdi_descriptor);
void cdi_load_sector(uint32 sector, uint8 *buffer);

#endif
