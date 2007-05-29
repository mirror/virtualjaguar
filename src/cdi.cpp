#include "cdi.h"

/* Added by SDLEMU (http://sdlemu.ngemu.com) */
/* Added for GCC UNIX compatibility          */
#ifdef __GCCUNIX__
	#include <unistd.h>
	#include <sys/types.h>
	#include <fcntl.h>

	#define O_BINARY (0)
#endif

#define CDI_V2  0x80000004
#define CDI_V3  0x80000005
#define CDI_V35 0x80000006

static uint8 cdi_track_start_mark[10] = { 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };



int cdi_fp;
uint32 cdi_load_address;
uint32 cdi_code_length;
s_cdi_descriptor	*cdi_descriptor;
s_cdi_track			**cdi_tracks;
uint32 				cdi_tracks_count;


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
int cdi_open(char *path)
{
	fprintf(log_get(),"cdi: opening %s\n",path);
	return(open(path,O_BINARY|O_RDONLY));
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
void cdi_close(int fp)
{
	fprintf(log_get(),"cdi: closing\n");
	close(fp);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __PORT__
long tell(int fd)
{
	return lseek(fd, 0LL, SEEK_CUR);
}
#endif
s_cdi_descriptor *cdi_get_descriptor(int fp, FILE *stdfp)
{
	s_cdi_descriptor *descriptor;
	
	uint32 previous_position=0;

	if (fp==-1)
		return(0);

	descriptor=(s_cdi_descriptor *)malloc(sizeof(s_cdi_descriptor));
	if (descriptor==0)
		return(0);

	lseek(fp,0,SEEK_END);
	descriptor->length=tell(fp);
	if (descriptor->length<8)
	{
		if (stdfp) fprintf(stdfp,"cdi: image is too short (%i bytes)\n",descriptor->length);
		free(descriptor);
		return(0);
	}
	lseek(fp,descriptor->length-8,SEEK_SET);
	read(fp,&descriptor->version,4);
	read(fp,&descriptor->header_offset,4);
	lseek(fp,descriptor->header_offset,SEEK_SET);
	read(fp,&descriptor->nb_of_sessions,2);

	descriptor->sessions=(s_cdi_session *)malloc(descriptor->nb_of_sessions*sizeof(s_cdi_session));
	if (descriptor->sessions==0)
	{
		free(descriptor);
		return(0);
	}
	if (stdfp) fprintf(stdfp,"cdi: %i sessions\n",descriptor->nb_of_sessions);
	uint32 track_position=0;
	for (uint16 session=0;session<descriptor->nb_of_sessions;session++)
	{
		read(fp,&descriptor->sessions[session].nb_of_tracks,2);
		descriptor->sessions[session].tracks=(s_cdi_track*)malloc(descriptor->sessions[session].nb_of_tracks*sizeof(s_cdi_track));

		if (stdfp) fprintf(stdfp,"cdi:\ncdi: reading session %i (%i tracks)\n",session,descriptor->sessions[session].nb_of_tracks);

		for (uint16 track=0;track<descriptor->sessions[session].nb_of_tracks;track++)
		{
			static char current_start_mark[10];
			s_cdi_track *current_track=&descriptor->sessions[session].tracks[track];
			if (stdfp) fprintf(stdfp,"cdi:\ncdi: \t\treading track %i\n",track);

			uint32 temp_value;

			read(fp,&temp_value, 4);
			if (temp_value != 0)
				lseek(fp, 8, SEEK_CUR); // extra data (DJ 3.00.780 and up)
	
			read(fp,current_start_mark, 10);
			if (memcmp(cdi_track_start_mark, current_start_mark, 10)) 
			{
				if (stdfp) fprintf(stdfp,"cdi: could not find the track start mark");
				return(0);
			}
			read(fp,current_start_mark, 10);
			if (memcmp(cdi_track_start_mark, current_start_mark, 10)) 
			{
				if (stdfp) fprintf(stdfp,"cdi: could not find the track start mark");
				return(0);
			}

			lseek(fp, 4, SEEK_CUR);
			read(fp,&current_track->filename_length, 1);
			lseek(fp, current_track->filename_length, SEEK_CUR);

			lseek(fp, 11, SEEK_CUR);
			lseek(fp, 4, SEEK_CUR);
			lseek(fp, 4, SEEK_CUR);
			read(fp,&temp_value, 4);
			if (temp_value == 0x80000000)
				lseek(fp, 8, SEEK_CUR); // DJ4
			lseek(fp, 2, SEEK_CUR);
			
			read(fp, &current_track->pregap_length, 4);
			if (current_track->pregap_length!=150)
			{
				fprintf(log_get(),"cdi: warning: pregap different than 150\n");
			}
			read(fp, &current_track->length, 4);
			if (stdfp) fprintf(stdfp,"cdi: \t\t\tpregap length : %i\n", current_track->pregap_length);
			if (stdfp) fprintf(stdfp,"cdi: \t\t\tlength : %i\n", current_track->length);

			lseek(fp, 6, SEEK_CUR);
			
			read(fp, &current_track->mode, 4);
			if (stdfp) fprintf(stdfp,"cdi: \t\t\tmode : %i\n", current_track->mode);

			lseek(fp, 12, SEEK_CUR);
			read(fp, &current_track->start_lba, 4);
			if (stdfp) fprintf(stdfp,"cdi: \t\t\tstart lba : %i\n", current_track->start_lba);
			read(fp, &current_track->total_length, 4);
			if (stdfp) fprintf(stdfp,"cdi: \t\t\ttotal length : %i\n", current_track->total_length);
			lseek(fp, 16, SEEK_CUR);
			read(fp, &current_track->sector_size_value, 4);
			if (stdfp) fprintf(stdfp,"cdi: \t\t\tsector size : %i\n", current_track->sector_size_value);

			switch(current_track->sector_size_value)
			{
			case 0 : current_track->sector_size = 2048; break;
			case 1 : current_track->sector_size = 2336; break;
			case 2 : current_track->sector_size = 2352; break;
			default: {
						if (stdfp) fprintf(stdfp,"cdi: \t\t\tunsupported %i bytes sector",current_track->sector_size_value);
						return(0);
					}
			}
			
			if (current_track->mode > 2) 
				if (stdfp) fprintf(stdfp,"cdi: \t\t\ttrack mode %i not supported",current_track->mode);

			lseek(fp, 29, SEEK_CUR);
			if (descriptor->version != CDI_V2)
			{
				lseek(fp, 5, SEEK_CUR);
				read(fp,&temp_value, 4);
				if (temp_value == 0xffffffff)
					lseek(fp, 78, SEEK_CUR); // extra data (DJ 3.00.780 and up)
			}
			current_track->position=track_position;
			track_position+=(current_track->pregap_length+current_track->length)*current_track->sector_size;
			previous_position=track_position;
		}
		lseek(fp, 4, SEEK_CUR);
		lseek(fp, 8, SEEK_CUR);
		if (descriptor->version != CDI_V2)
			lseek(fp, 1, SEEK_CUR);

	}
    if (descriptor->header_offset == 0) 
		return(0);

	return(descriptor);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
void cdi_dump_descriptor(FILE *fp,s_cdi_descriptor *cdi_descriptor)
{
	fprintf(fp,"cdi: %i Mb\n",cdi_descriptor->length>>20);
	fprintf(fp,"cdi: format version ");
	switch(cdi_descriptor->version)
	{
	case CDI_V2:	fprintf(fp,"2\n"); break;
	case CDI_V3:	fprintf(fp,"3\n"); break;
	case CDI_V35:	fprintf(fp,"3.5\n"); break;
	default:		fprintf(fp,"unknown\n"); break;
	}
	fprintf(fp,"cdi: %i sessions\n",cdi_descriptor->nb_of_sessions);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint8 *cdi_extract_boot_code(int fp, s_cdi_descriptor *cdi_descriptor)
{
	s_cdi_track *boot_track=&cdi_descriptor->sessions[1].tracks[0];
	uint32 boot_track_size=boot_track->length*boot_track->sector_size;

	uint8 *boot_track_data=(uint8*)malloc(boot_track_size);
	lseek(fp,2+(boot_track->position), SEEK_SET);
	read(fp,boot_track_data,boot_track_size);
	
	uint32 *boot_track_data_32=(uint32*)boot_track_data;
	uint32 offset=0;
	while (offset<(boot_track_size>>2))
	{
		if (boot_track_data_32[offset]==0x49205452)
			break;
		offset++;
	}
	if (offset==(boot_track_size>>2))
	{
		fprintf(log_get(),"cdi: cannot find the boot code\n");
		return(NULL);
	}
	offset=(offset<<2)+4;
	uint16 *data16=(uint16*)&boot_track_data[offset];
	cdi_load_address=*data16++; cdi_load_address<<=16; cdi_load_address|=*data16++;
	cdi_code_length=*data16++; cdi_code_length<<=16; cdi_code_length|=*data16++;
	fprintf(log_get(),"cdi: load address: 0x%.8x\n",cdi_load_address);
	fprintf(log_get(),"cdi: length: 0x%.8x\n",cdi_code_length);
	fprintf(log_get(),"cdi: byte swapping boot code\n");

	for (uint32 i=0;i<(cdi_code_length>>1);i++)
	{
		uint16 sdata=data16[i];
		sdata=(sdata>>8)|(sdata<<8);
		data16[i]=sdata;
	}
	return((uint8*)data16);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
void cdi_load_sector(uint32 sector, uint8 *buffer)
{
	if (sector==0xffffffff)
	{
		memset(buffer,0x00,2352);
		return;
	}

	sector*=2352;

	lseek(cdi_fp,2+sector,SEEK_SET);
	read(cdi_fp,buffer,2352);

	// byte swap
	for (uint32 i=0;i<2352;i+=2)
	{
		uint8 sdata=buffer[i+0];
		buffer[i+0]=buffer[i+1];
		buffer[i+1]=sdata;
	}

}
