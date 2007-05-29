//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
// 20.02.2002: fixed sector to msf conversion
// 19.02.2002: fixed a bug in toc encoding function
//
//
//
//////////////////////////////////////////////////////////////////////////////
#include "include/cdbios.h"

/* Added by SDLEMU (http://sdlemu.ngemu.com */
/* Some GCC UNIX includes                   */
#ifdef __GCCUNIX__
	#include <unistd.h>
#endif

#define ERR_FLAG	0x3e00
#define CD_TOC		0x2c00
#define BIOS_VER    0x3004


#define CDBIOS_INIT		0
#define CDBIOS_INITM	1
#define CDBIOS_INITF	2



#define CD_INIT		0x3000
#define CD_MODE		0x3006
#define CD_ACK		0x300C
#define CD_JERI		0x3012
#define CD_SPIN		0x3018
#define CD_STOP		0x301E
#define CD_MUTE		0x3024
#define CD_UMUTE	0x302A
#define CD_PAUSE	0x3030
#define CD_UPAUSE	0x3036
#define CD_READ		0x303C
#define CD_UREAD	0x3042
#define CD_SETUP	0x3048
#define CD_PTR		0x304E
#define CD_OSAMP	0x3054
#define CD_GETTOC	0x306A
#define CD_INITM	0x3060
#define CD_INITF	0x3066


char *cdbios_command[]={"init","mode","ack","jeri","spin","stop","mute","umute","pause","upause",
						"read","uread","setup","ptr","osamp","getoc","initm","initf",
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define CDROM_AUDIO_MODE	0
#define CDROM_DATA_MODE		1

#define CDROM_SINGLE_SPEED	0
#define CDROM_DOUBLE_SPEED  1

#define CDROM_STATUS_OK		0
#define CDROM_STATUS_ERROR	1


uint32 cdrom_mode				= CDROM_DATA_MODE;
uint32 cdrom_speed				= CDROM_SINGLE_SPEED;
uint32 cdrom_oversample_factor	= 1;
uint32 cdbios_session=0;

uint32 cdrom_destination_buffer_beginning;
uint32 cdrom_destination_buffer_end;
uint32 cdrom_time_code;
uint32 cdrom_seek_only;
uint32 cdrom_partition_marker;
uint32 cdrom_circular_buffer_size;

uint32 cdbios_init_type;
uint32 *cdbios_sector_lut;
uint32 cdbios_init_done=0;

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
void cdbios_build_sector_lut(void)
{
	uint32 last_sector=0;
	int32 offset=0;
	uint32 sector=0;

	s_cdi_track *last_track=&cdi_descriptor->sessions[cdi_descriptor->nb_of_sessions-1].tracks[cdi_descriptor->sessions[cdi_descriptor->nb_of_sessions-1].nb_of_tracks-1];
	
	last_sector=last_track->start_lba+last_track->total_length-1;


	cdbios_sector_lut=(uint32*)malloc((last_sector+1)*sizeof(uint32));
	memset(cdbios_sector_lut,0xff,(last_sector+1)*sizeof(uint32));

	for (int session=0;session<cdi_descriptor->nb_of_sessions;session++)
	{
		for (int track=0;track<cdi_descriptor->sessions[session].nb_of_tracks;track++)
		{
			s_cdi_track *current_track=&cdi_descriptor->sessions[session].tracks[track];

			if (offset<((int32)(current_track->start_lba-1)))
			{
				fprintf(log_get(),"cdbios: postgap between %i and %i\n",offset,current_track->start_lba-1);

				// fill-in postgap
				for (;offset<current_track->start_lba;offset++)
				{
					cdbios_sector_lut[offset]=0xffffffff;
				}

			}			
			fprintf(log_get(),"cdbios: data between %i and %i\n",offset,current_track->start_lba+current_track->total_length-1);
			for (;offset<current_track->start_lba+current_track->total_length;offset++)
			{
				cdbios_sector_lut[offset]=sector;
				sector++;
			}

		}
	}

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
void cdbios_get_time(uint32 sectors, uint32 *mm, uint32 *ss, uint32 *ff)
{
	uint32 _sectors=sectors;

	uint32 _mm=(sectors/(60*75));
	sectors-=(_mm*(60*75));
	uint32 _ss=(sectors/75);
	sectors-=(_ss*75);
	uint32 _ff=sectors;

	*mm=_mm;
	*ss=_ss;
	*ff=_ff;
	

//	fprintf(log_get(),"[%.2i:%.2i:%.2i]\n",_mm,_ss,_ff);
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
void cdbios_encode_toc(void)
{
	int i;
	uint32 base=CD_TOC;

	jaguar_byte_write(base++,0x00);
	jaguar_byte_write(base++,0x00);
	jaguar_byte_write(base++,0x01);

	int nbtracks=0;
	for (i=0;i<cdi_descriptor->nb_of_sessions;i++)
		nbtracks+=cdi_descriptor->sessions[i].nb_of_tracks;

	jaguar_byte_write(base++,nbtracks+1);
	jaguar_byte_write(base++,cdi_descriptor->nb_of_sessions);
	
	uint32 mm=0;
	uint32 ss=0;
	uint32 ff=0;

	cdbios_get_time(cdi_descriptor->sessions[cdi_descriptor->nb_of_sessions-1].tracks[cdi_descriptor->sessions[cdi_descriptor->nb_of_sessions-1].nb_of_tracks-1].start_lba+
					cdi_descriptor->sessions[cdi_descriptor->nb_of_sessions-1].tracks[cdi_descriptor->sessions[cdi_descriptor->nb_of_sessions-1].nb_of_tracks-1].length+
					cdi_descriptor->sessions[cdi_descriptor->nb_of_sessions-1].tracks[cdi_descriptor->sessions[cdi_descriptor->nb_of_sessions-1].nb_of_tracks-1].pregap_length,
					&mm,&ss,&ff);

	jaguar_byte_write(base++,mm);
	jaguar_byte_write(base++,ss);
	jaguar_byte_write(base++,ff);

	int track_count=1;
	for (i=0;i<cdi_descriptor->nb_of_sessions;i++)
		for (int j=0;j<cdi_descriptor->sessions[i].nb_of_tracks;j++)
		{
			s_cdi_track *track=&cdi_descriptor->sessions[i].tracks[j];
			
//			fprintf(log_get(),"track %i\n",track_count);

			jaguar_byte_write(base++,track_count);
			cdbios_get_time(track->start_lba+track->pregap_length,&mm,&ss,&ff);
			jaguar_byte_write(base++,mm);
			jaguar_byte_write(base++,ss);
			jaguar_byte_write(base++,ff);
			jaguar_byte_write(base++,i);
			cdbios_get_time(track->length,&mm,&ss,&ff);
			jaguar_byte_write(base++,mm);
			jaguar_byte_write(base++,ss);
			jaguar_byte_write(base++,ff);
				
			track_count++;
		}

	cdi_tracks_count=track_count;
	cdi_tracks=(s_cdi_track**)malloc(track_count*sizeof(s_cdi_track*));
	track_count=0;
	for (i=0;i<cdi_descriptor->nb_of_sessions;i++)
	{
		for (int j=0;j<cdi_descriptor->sessions[i].nb_of_tracks;j++)
		{
			cdi_tracks[track_count]=&cdi_descriptor->sessions[i].tracks[j];
			track_count++;
		}
	}
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
void cdbios_decode_toc(void)
{
	uint32 addr=0x2c00;
	uint32 nb_tracks;
	
	fprintf(log_get(),"cdbios: toc:\n");
	fprintf(log_get(),"cdbios:\n");
	addr+=2;
	fprintf(log_get(),"cdbios: minimum track number: %i\n",jaguar_byte_read(addr++));
	fprintf(log_get(),"cdbios: maximum track number: %i\n",nb_tracks=jaguar_byte_read(addr++));
	fprintf(log_get(),"cdbios: number of sessions  : %i\n",jaguar_byte_read(addr++));
	fprintf(log_get(),"cdbios: start of last lead out time: %.2i:%.2i:%.2i\n",
					  jaguar_byte_read(addr++),jaguar_byte_read(addr++),jaguar_byte_read(addr++));

	uint32 mm,ss,ff;

	nb_tracks--;

	while (nb_tracks)
	{
		fprintf(log_get(),"cdbios:\n");
		fprintf(log_get(),"cdbios: track          : %i\n",jaguar_byte_read(addr++));
		
		mm=jaguar_byte_read(addr++);
		ss=jaguar_byte_read(addr++);
		ff=jaguar_byte_read(addr++);

		fprintf(log_get(),"cdbios: start at       : %.2i:%.2i:%.2i\n",mm,ss,ff);
		fprintf(log_get(),"cdbios: session        : %i\n",jaguar_byte_read(addr++));
		
		mm=jaguar_byte_read(addr++);
		ss=jaguar_byte_read(addr++);
		ff=jaguar_byte_read(addr++);

		fprintf(log_get(),"cdbios: duration       : %.2i:%.2i:%.2i\n",mm,ss,ff);
		nb_tracks--;
	}
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
void cd_bios_boot(char *filename)
{
	cd_bios_init();


	cdi_fp=cdi_open(filename);
	cdi_descriptor=cdi_get_descriptor(cdi_fp,log_get());
	if (cdi_descriptor==0)
	{
		fprintf(log_get(),"cdi: bad image\n");
		cdi_close(cdi_fp);
		return;
	}
	cdi_dump_descriptor(log_get(),cdi_descriptor);
	cdbios_build_sector_lut();

	uint8 *code=cdi_extract_boot_code(cdi_fp,cdi_descriptor);

	// copy the code to ram
	for (uint32 i=0;i<cdi_code_length;i++)
		jaguar_byte_write(cdi_load_address+i,code[i]);

	// set the boot address
	jaguar_long_write(0x00000004,cdi_load_address);

	// write the toc
	cdbios_encode_toc();

	// dump the toc
	cdbios_decode_toc();

//	fprintf(log_get(),"cdbios: boot code:\n");
//	jaguar_dasm(cdi_load_address, cdi_code_length);
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
void cd_bios_init(void)
{
	fprintf(log_get(),"cd_bios: initializing\n");
	for (int i=0;i<19;i++)
	{
		// RESET opcode
		jaguar_word_write(0x3000+(i*6),0x4E70);
	}
	cd_bios_reset();
	cdbios_init_done=1;
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
void cd_bios_reset(void)
{
	fprintf(log_get(), "cd_bios: reset\n");
	jaguar_word_write(BIOS_VER, 0x0405);
	cdrom_mode				= CDROM_DATA_MODE;
	cdrom_speed				= CDROM_SINGLE_SPEED;
	cdrom_oversample_factor	= 1;
	cdbios_session			= 0;
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
void cd_bios_done(void)
{
	close(cdi_fp);
	fprintf(log_get(), "CD BIOS: Done.\n");
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
void cdbios_cmd_init(void)
{
	cdbios_init_type = CDBIOS_INIT;
    jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);
	fprintf(log_get(), "cdbios: init(0x%.8x)\n", m68k_get_reg(NULL, M68K_REG_A0));
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
void cdbios_cmd_mode(void)
{
//	uint32 data = s68000context.dreg[0];
	uint32 data = m68k_get_reg(NULL, M68K_REG_D0);

	uint32 cdrom_mode = (data>>1) & 0x01;
	uint32 cdrom_speed = data & 0x01;
	jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);

//	fprintf(log_get(),"cdbios: %s\n",(cdrom_mode==CDROM_AUDIO_MODE)?"audio mode":"data mode");
//	fprintf(log_get(),"cdbios: %s\n",(cdrom_speed==CDROM_DOUBLE_SPEED)?"double speed":"single speed");
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
void cdbios_cmd_ack(void)
{
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_jeri(void)
{
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_spin(void)
{
//	uint16 spin=(1<<(s68000context.dreg[0]&0xffff));
	uint16 spin = (1 << (m68k_get_reg(NULL, M68K_REG_D0) & 0xFFFF));
	cdbios_session = spin;
    jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);
//	fprintf(log_get(),"cdbios: switching to session %i\n",spin);
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
void cdbios_cmd_stop(void)
{
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_mute(void)
{
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_umute(void)
{
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_pause(void)
{
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_upause(void)
{
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
}
void cdi_read_block(uint32 sector, uint8 *buffer, uint32 count)
{
	while (count)
	{
		cdi_load_sector(cdbios_sector_lut[sector],buffer);
		buffer+=2352;
		sector++;
		count--;
	}
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
void cdbios_cmd_read(void)
{
	static uint8 cdbios_sector[2352];

/*	cdrom_destination_buffer_beginning=s68000context.areg[0];
	cdrom_destination_buffer_end=s68000context.areg[1];
	cdrom_time_code=(s68000context.dreg[0])&0x7fffffff;
	cdrom_seek_only=s68000context.dreg[0]&0x80000000;*/
	cdrom_destination_buffer_beginning = m68k_get_reg(NULL, M68K_REG_A0);
	cdrom_destination_buffer_end = m68k_get_reg(NULL, M68K_REG_A1);
	cdrom_time_code = m68k_get_reg(NULL, M68K_REG_D0) & 0x7FFFFFFF;
	cdrom_seek_only = m68k_get_reg(NULL, M68K_REG_D0) & 0x80000000;

/*	cdrom_partition_marker=s68000context.dreg[1];
	if (s68000context.dreg[2]!=0)
		cdrom_circular_buffer_size=(1<<s68000context.dreg[2]);*/
	cdrom_partition_marker = m68k_get_reg(NULL, M68K_REG_D1);
	if (m68k_get_reg(NULL, M68K_REG_D2) != 0)
		cdrom_circular_buffer_size = 1 << m68k_get_reg(NULL, M68K_REG_D2);
	else
		cdrom_circular_buffer_size = 0xFFFFFFFF; // no circular buffer;

	uint32 mm = (10 * ((cdrom_time_code >> 20) & 0x0F)) + ((cdrom_time_code >> 16) & 0x0F);
	uint32 ss = (10 * ((cdrom_time_code >> 12) & 0x0F)) + ((cdrom_time_code >> 8) & 0x0F);
	uint32 ff = (10 * ((cdrom_time_code >> 4) & 0x0F)) + ((cdrom_time_code >> 0) & 0x0F);

	fprintf(log_get(),"cdbios: read(0x%.8x, 0x%.8x, %.2i:%.2i:%.2i, %i) %s\n",
						cdrom_destination_buffer_beginning,
						cdrom_destination_buffer_end,
						mm,ss,ff,
						cdrom_circular_buffer_size,cdrom_seek_only?"(seek only)":"");

	if (!cdrom_seek_only)
	{
		if (cdbios_init_type==CDBIOS_INITM)
		{
			fprintf(log_get(),"cdbios: partition marker: %c%c%c%c (0x%.8x)\n",
				((cdrom_partition_marker>>24)&0xff),
				((cdrom_partition_marker>>16)&0xff),
				((cdrom_partition_marker>>8)&0xff),
				(cdrom_partition_marker&0xff),
				cdrom_partition_marker
				);
		}
		if (((int32)cdrom_circular_buffer_size)==-1)
			cdrom_circular_buffer_size=0xffffffff;

		uint32 track_offset=((ss+(60*mm))*75)+ff;

		fprintf(log_get(),"cdbios: track offset: %i\n",track_offset);
		uint32 nb_sectors=(cdrom_destination_buffer_end-cdrom_destination_buffer_beginning)/2352;
		uint32 reste=(cdrom_destination_buffer_end-cdrom_destination_buffer_beginning)%2352;
		uint32 buffer_offset=0;
		uint32 nb_bytes_to_read=cdrom_destination_buffer_end-cdrom_destination_buffer_beginning+1;

		if (cdbios_init_type==CDBIOS_INITF)
		{
//			if (cdrom_destination_buffer_beginning==0x00046000)
//				return;

			uint8 *buffer=(uint8*)malloc((nb_sectors+1)*2352);
			cdi_read_block(track_offset,buffer,nb_sectors+1);
			for (uint32 k=0;k<nb_bytes_to_read;k++)
				jaguar_byte_write(cdrom_destination_buffer_beginning+k,buffer[k]);
			free(buffer);
		}
		else
		if (cdbios_init_type==CDBIOS_INITM)
		{
			uint8 *buffer=(uint8*)malloc((31+nb_sectors+1)*2352);
			cdi_read_block(track_offset,buffer,nb_sectors+1);
			
			uint32	count=(31+nb_sectors+1)*2352;
			uint8	*alias=buffer;
			uint32	marker_count=2;

			while (count-4)
			{
				if (
					(alias[0]==((cdrom_partition_marker>>24)&0xff))&&
					(alias[1]==((cdrom_partition_marker>>16)&0xff))&&
					(alias[2]==((cdrom_partition_marker>>8)&0xff))&&
					(alias[3]==((cdrom_partition_marker>>0)&0xff))
					)
				{
					marker_count--;
					if (marker_count)
					{
						alias+=3;
					}
					else
					{
						exit(0);
						alias+=4;
						for (uint32 k=0;k<nb_bytes_to_read;k++)
							jaguar_byte_write(cdrom_destination_buffer_beginning+k,alias[k]);
						free(buffer);
						return;
					}
				}
				alias++;
				count--;
			}
			free(buffer);
		}
		else
		{
			fprintf(log_get(),"cdbios: unsupported read mode\n");
		}
	}
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_uread(void)
{
	// stop the recording started with a cd_read command
	// for now, there is nothing to do
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_setup(void)
{
	// initialize the cd system
    jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);
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
void cdbios_cmd_ptr(void)
{
//	s68000context.areg[0]=cdrom_destination_buffer_end+4;
	m68k_set_reg(M68K_REG_A0, cdrom_destination_buffer_end + 4);
    jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);
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
void cdbios_cmd_osamp(void)
{
//	cdrom_oversample_factor=(1<<(s68000context.dreg[0]&0xffff));
	cdrom_oversample_factor = 1 << (m68k_get_reg(NULL, M68K_REG_D0) & 0xFFFF);
    jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);
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
void cdbios_cmd_gettoc(void)
{
    jaguar_word_write(ERR_FLAG,CDROM_STATUS_OK);
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
void cdbios_cmd_initm(void)
{
	cdbios_init_type = CDBIOS_INITM;
    jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);
//	fprintf(log_get(),"cdbios: initm(0x%.8x)\n",s68000context.areg[0]);
	fprintf(log_get(), "cdbios: initm(0x%.8x)\n", m68k_get_reg(NULL, M68K_REG_A0));
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
void cdbios_cmd_initf(void)
{
	cdbios_init_type = CDBIOS_INITF;
    jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);
//	fprintf(log_get(),"cdbios: initf(0x%.8x)\n",s68000context.areg[0]);
	fprintf(log_get(), "cdbios: initf(0x%.8x)\n", m68k_get_reg(NULL, M68K_REG_A0));
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
void cd_bios_process(uint32 offset)
{
	if (!cdbios_init_done)
		return;

	fprintf(log_get(), "cdbios: processing command '%s'\n", cdbios_command[(offset-0x3000) / 6]);
	fprintf(log_get(), "cdbios: 68K PC=%08X SP=%08X\n", jaguar_long_read(m68k_get_reg(NULL, M68K_REG_A7)), m68k_get_reg(NULL, M68K_REG_A7));

	switch (offset)
	{
	case CD_INIT:	cdbios_cmd_init();		break;		
	case CD_MODE:	cdbios_cmd_mode();		break;		
	case CD_ACK:	cdbios_cmd_ack();		break;	
	case CD_JERI:	cdbios_cmd_jeri();		break;
	case CD_SPIN:	cdbios_cmd_spin();		break;
	case CD_STOP:	cdbios_cmd_stop();		break;
	case CD_MUTE:	cdbios_cmd_mute();		break;
	case CD_UMUTE:	cdbios_cmd_umute();		break;
	case CD_PAUSE:	cdbios_cmd_pause();		break;
	case CD_UPAUSE:	cdbios_cmd_upause();	break;
	case CD_READ:	cdbios_cmd_read();		break;
	case CD_UREAD:	cdbios_cmd_uread();		break;
	case CD_SETUP:	cdbios_cmd_setup();		break;
	case CD_PTR:	cdbios_cmd_ptr();		break;
	case CD_OSAMP:	cdbios_cmd_osamp();		break;
	case CD_GETTOC:	cdbios_cmd_gettoc();	break;
	case CD_INITM:	cdbios_cmd_initm();		break;
	case CD_INITF:	cdbios_cmd_initf();		break;

	default: fprintf(log_get(), "cdbios: unemulated entry point at %08X 68K PC=%08X SP=%08X\n", offset, jaguar_long_read(m68k_get_reg(NULL, M68K_REG_A7)), m68k_get_reg(NULL, M68K_REG_A7));
		jaguar_word_write(ERR_FLAG, CDROM_STATUS_OK);
	}

	// rts
//	s68000context.pc=jaguar_long_read(s68000context.areg[7]);
//	s68000context.areg[7]=(s68000context.areg[7]+4)&0xffffffff;
	m68k_set_reg(M68K_REG_PC, jaguar_long_read(m68k_get_reg(NULL, M68K_REG_A7)));
	m68k_set_reg(M68K_REG_A7, (m68k_get_reg(NULL, M68K_REG_A7) + 4) & 0xFFFFFFFF);
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
void cd_bios_exec(uint32 scanline)
{
	// single speed mode: 150 Kb/s (153600 bytes/s)
	// single speed mode: 300 Kb/s (307200 bytes/s)
	uint32 bytes_per_scanline=(153600*((cdrom_speed==CDROM_DOUBLE_SPEED)?2:1))/525;
}
