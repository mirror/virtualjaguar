#include "eeprom.h"

#define eeprom_LOG

static uint16 eeprom_ram[64];

void eeprom_set_di(uint32 state); 
void eeprom_set_cs(uint32 state);
uint32 eeprom_get_do(void);

#define EE_STATE_START			1
#define EE_STATE_OP_A			2
#define EE_STATE_OP_B			3
#define EE_STATE_0				4
#define EE_STATE_1				5
#define EE_STATE_2				6
#define EE_STATE_3				7
#define EE_STATE_0_0			8
#define EE_READ_ADDRESS			9
#define EE_STATE_0_0_0			10
#define EE_STATE_0_0_1			11
#define EE_STATE_0_0_2			12
#define EE_STATE_0_0_3			13
#define EE_STATE_0_0_1_0		14
#define EE_READ_DATA			15
#define EE_STATE_BUSY			16
#define EE_STATE_1_0			17
#define EE_STATE_1_1			18
#define EE_STATE_2_0			19
#define EE_STATE_3_0			20

uint16 jerry_ee_state = EE_STATE_START;
uint16 jerry_ee_op = 0;
uint16 jerry_ee_rstate = 0;
uint16 jerry_ee_address_data = 0;
uint16 jerry_ee_address_cnt = 6;
uint16 jerry_ee_data = 0;
uint16 jerry_ee_data_cnt = 16;
uint16 jerry_writes_enabled = 0;
uint16 jerry_ee_direct_jump = 0;
FILE * jerry_ee_fp;
extern char * jaguar_boot_dir;

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
void eeprom_init(void)
{
	static char eeprom_filename[256];
	sprintf(eeprom_filename,"%s\\%s%.8x.eep",jaguar_boot_dir,jaguar_eeproms_path,jaguar_mainRom_crc32);
	jerry_ee_fp=fopen(eeprom_filename,"rb");
	if (jerry_ee_fp)
	{
		fread(eeprom_ram,1,128,jerry_ee_fp);
		fclose(jerry_ee_fp);
		fprintf(log_get(),"eeprom: loaded from %s\n",eeprom_filename);
		jerry_ee_fp=fopen(eeprom_filename,"wrb");
	}
	else
	{
		fprintf(log_get(),"eeprom: creating %s\n",eeprom_filename);
		jerry_ee_fp=fopen(eeprom_filename,"wb");
		if (jerry_ee_fp==NULL)
			fprintf(log_get(),"eeprom: could not open/create %s\n",eeprom_filename);
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
void eeprom_reset(void)
{
	if (jerry_ee_fp==NULL)
		memset(eeprom_ram,0xff,64*2);
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
void eeprom_done(void)
{
	if (jerry_ee_fp)
	{
		fwrite(eeprom_ram,1,128,jerry_ee_fp);
		fclose(jerry_ee_fp);
	}
	else
		fprintf(log_get(),"eeprom: not saved\n");
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
void eeprom_byte_write(uint32 offset, uint8 data)
{
	switch (offset)
	{
	case 0xf14001: break;
	case 0xf14801: eeprom_set_di(data&0x01); break;
	case 0xf15001: eeprom_set_cs(1); break;
//	default: fprintf(log_get(),"eeprom: unmapped 0x%.8x\n",offset); break;
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
void eeprom_word_write(uint32 offset, uint16 data)
{
	eeprom_byte_write(offset+0,(data>>8)&0xff);
	eeprom_byte_write(offset+1,(data&0xff));
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
uint8 eeprom_byte_read(uint32 offset)
{
	switch (offset)
	{
	case 0xf14001: return(eeprom_get_do());  break;
	case 0xf14801: break;
	case 0xf15001: eeprom_set_cs(1); break;
//	default: fprintf(log_get(),"eeprom: unmapped 0x%.8x\n",offset); break;
	}
	return(0x00);
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
uint16 eeprom_word_read(uint32 offset)
{
	uint16 data=eeprom_byte_read(offset+0);
	data<<=8;
	data|=eeprom_byte_read(offset+1);
	return(data);
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
void eeprom_set_di(uint32 data)
{
//	fprintf(log_get(),"eeprom: di=%i\n",data);
//	fprintf(log_get(),"eeprom: state %i\n",jerry_ee_state);
	switch (jerry_ee_state)
	{
	case EE_STATE_START: { 
							jerry_ee_state=EE_STATE_OP_A; 
							break;
						 }
	case EE_STATE_OP_A:  { 
							jerry_ee_op=(data<<1); 
							jerry_ee_state=EE_STATE_OP_B; 
							break;
						 }
	case EE_STATE_OP_B:  {
							jerry_ee_op|=data; 
							jerry_ee_direct_jump=0;
//							fprintf(log_get(),"eeprom: opcode %i\n",jerry_ee_op);
							switch (jerry_ee_op)
							{
							case 0: jerry_ee_state=EE_STATE_0; break;
							case 1: jerry_ee_state=EE_STATE_1; break;
							case 2: jerry_ee_state=EE_STATE_2; break;
							case 3: jerry_ee_state=EE_STATE_3; break;
							}
							eeprom_set_di(data);
							break;
						 }
	case EE_STATE_0:	{
							jerry_ee_rstate=EE_STATE_0_0;
							jerry_ee_state=EE_READ_ADDRESS;
							jerry_ee_direct_jump=1;
							jerry_ee_address_cnt=6;
							jerry_ee_address_data=0;
							break;
						}
	case EE_STATE_0_0:	{
							switch ((jerry_ee_address_data>>4)&0x03)
							{
							case 0: jerry_ee_state=EE_STATE_0_0_0; break;
							case 1: jerry_ee_state=EE_STATE_0_0_1; break;
							case 2: jerry_ee_state=EE_STATE_0_0_2; break;
							case 3: jerry_ee_state=EE_STATE_0_0_3; break;
							}
							eeprom_set_di(data);
							break;
						}
	case EE_STATE_0_0_0:{
							// writes disable
							// fprintf(log_get(),"eeprom: read only\n");
							jerry_writes_enabled=0;
							jerry_ee_state=EE_STATE_START;
							break;
						}
	case EE_STATE_0_0_1:{
							// writes all
							jerry_ee_rstate=EE_STATE_0_0_1_0;
							jerry_ee_state=EE_READ_DATA;
							jerry_ee_data_cnt=16;
							jerry_ee_data=0;
							jerry_ee_direct_jump=1;
							break;
						}
	case EE_STATE_0_0_1_0:{
							// fprintf(log_get(),"eeprom: filling eeprom with 0x%.4x\n",data);
							if (jerry_writes_enabled)
							{
								for (int i=0;i<64;i++)
									eeprom_ram[i]=jerry_ee_data;
							}
							//else 
							//	fprintf(log_get(),"eeprom: not writing because read only\n");
							jerry_ee_state=EE_STATE_BUSY;
							break;
						  }
	case EE_STATE_0_0_2:{
							// erase all
							//fprintf(log_get(),"eeprom: erasing eeprom\n");
							if (jerry_writes_enabled)
							{
								for (int i=0;i<64;i++)
									eeprom_ram[i]=0xffff;
							}
							jerry_ee_state=EE_STATE_BUSY;
							break;
						}
	case EE_STATE_0_0_3:{
							// writes enable
							//fprintf(log_get(),"eeprom: read/write\n");
							jerry_writes_enabled=1;
							jerry_ee_state=EE_STATE_START;
							break;
						}
	case EE_STATE_1:	{
							jerry_ee_rstate=EE_STATE_1_0;
							jerry_ee_state=EE_READ_ADDRESS;
							jerry_ee_address_cnt=6;
							jerry_ee_address_data=0;
							jerry_ee_direct_jump=1;
							break;
						}
	case EE_STATE_1_0:	{
							jerry_ee_rstate=EE_STATE_1_1;
							jerry_ee_state=EE_READ_DATA;
							jerry_ee_data_cnt=16;
							jerry_ee_data=0;
							jerry_ee_direct_jump=1;
							break;
						}
	case EE_STATE_1_1:	{
							//fprintf(log_get(),"eeprom: writing 0x%.4x at 0x%.2x\n",jerry_ee_data,jerry_ee_address_data);
							if (jerry_writes_enabled)
								eeprom_ram[jerry_ee_address_data]=jerry_ee_data;
							jerry_ee_state=EE_STATE_BUSY;
							break;
						}
	case EE_STATE_2:	{
							jerry_ee_rstate=EE_STATE_2_0;
							jerry_ee_state=EE_READ_ADDRESS;
							jerry_ee_address_cnt=6;
							jerry_ee_address_data=0;
							jerry_ee_data_cnt=16;
							jerry_ee_data=0;
							break;
						}
	case EE_STATE_3:	{
							jerry_ee_rstate=EE_STATE_3_0;
							jerry_ee_state=EE_READ_ADDRESS;
							jerry_ee_address_cnt=6;
							jerry_ee_address_data=0;
							jerry_ee_direct_jump=1;
							break;
						}
	case EE_STATE_3_0:	{
							//fprintf(log_get(),"eeprom: erasing 0x%.2x\n",jerry_ee_address_data);
							if (jerry_writes_enabled)
								eeprom_ram[jerry_ee_address_data]=0xffff;
							jerry_ee_state=EE_STATE_BUSY;
							break;
						}
	case EE_READ_DATA: 
						{
							//fprintf(log_get(),"eeprom:\t\t\t%i bit %i\n",data,jerry_ee_data_cnt-1);
							jerry_ee_data<<=1;
							jerry_ee_data|=data;
							jerry_ee_data_cnt--;
							if (!jerry_ee_data_cnt)
							{
								jerry_ee_state=jerry_ee_rstate;
								if (jerry_ee_direct_jump)
									eeprom_set_di(data);
							}
							break;
						}
	case EE_READ_ADDRESS: 
						{
							jerry_ee_address_data<<=1;
							jerry_ee_address_data|=data;
							jerry_ee_address_cnt--;
//							fprintf(log_get(),"eeprom:\t%i bits remaining\n",jerry_ee_address_cnt);
							if (!jerry_ee_address_cnt)
							{
								jerry_ee_state=jerry_ee_rstate;
								//fprintf(log_get(),"eeprom:\t\tread address 0x%.2x\n",jerry_ee_address_data);
								if (jerry_ee_direct_jump)
									eeprom_set_di(data);
							}
							break;
						}
	default:			{ 
							jerry_ee_state=EE_STATE_OP_A; 
							break;
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
void eeprom_set_cs(uint32 state)
{
//	fprintf(log_get(),"eeprom: cs=%i\n",state);
	jerry_ee_state=EE_STATE_START;
	jerry_ee_op=0;
	jerry_ee_rstate=0;
	jerry_ee_address_data=0;
	jerry_ee_address_cnt=6;
	jerry_ee_data=0;
	jerry_ee_data_cnt=16;
	jerry_writes_enabled=1;
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
uint32 eeprom_get_do(void)
{
	uint16 data=1;
	switch (jerry_ee_state)
	{
	case EE_STATE_START: {
							data=1;
							break;
						 }
	case EE_STATE_BUSY: {
							jerry_ee_state=EE_STATE_START;
							data=0;
							break;
						 }
	case EE_STATE_2_0:	{
							jerry_ee_data_cnt--;
							data=((eeprom_ram[jerry_ee_address_data]&(1<<jerry_ee_data_cnt))>>jerry_ee_data_cnt);
							if (!jerry_ee_data_cnt)
							{
								//fprintf(log_get(),"eeprom: read 0x%.4x at 0x%.2x cpu %i pc=0x%.8x\n",eeprom_ram[jerry_ee_address_data],jerry_ee_address_data,jaguar_cpu_in_exec,s68000readPC());
								jerry_ee_state=EE_STATE_START;
							}
							break;
						}
	}
//	fprintf(log_get(),"eeprom: do=%i\n",data);
	return(data);
}
