
		uint32 c;
		paletteRam+=(idx<<2);
		if (flags&FLAGS_READMODIFY)
		{
			if (flags&FLAGS_HFLIP)
			{
				while (iwidth)
				{
					c=jaguar_byte_read(ptr++);
					
					if (flags&FLAGS_TRANSPARENT)
					{
						#define PUTPIXEL_1_TB_HFLIP(C) if (C) { *current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(1<<1)+0]); *current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(1<<1)+1]); } else current_line_buffer-=2; 
						
						PUTPIXEL_1_TB_HFLIP(c&0x80)
						PUTPIXEL_1_TB_HFLIP(c&0x40)
						PUTPIXEL_1_TB_HFLIP(c&0x20)
						PUTPIXEL_1_TB_HFLIP(c&0x10)
						PUTPIXEL_1_TB_HFLIP(c&0x08)
						PUTPIXEL_1_TB_HFLIP(c&0x04)
						PUTPIXEL_1_TB_HFLIP(c&0x02)
						PUTPIXEL_1_TB_HFLIP(c&0x01)
					}
					else
					{
						#define PUTPIXEL_1_B_HFLIP(C) if (C) { *current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(1<<1)+0]); *current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(1<<1)+1]); } else { *current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[0]); *current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[1]); }
						PUTPIXEL_1_B_HFLIP(c&0x80)
						PUTPIXEL_1_B_HFLIP(c&0x40)
						PUTPIXEL_1_B_HFLIP(c&0x20)
						PUTPIXEL_1_B_HFLIP(c&0x10)
						PUTPIXEL_1_B_HFLIP(c&0x08)
						PUTPIXEL_1_B_HFLIP(c&0x04)
						PUTPIXEL_1_B_HFLIP(c&0x02)
						PUTPIXEL_1_B_HFLIP(c&0x01)
					}
					iwidth--;
				}
			}
			else
			{
				while (iwidth)
				{
					c=jaguar_byte_read(ptr++);
					
					if (flags&FLAGS_TRANSPARENT)
					{
						#define PUTPIXEL_1_TB(C) if (C) { *current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(1<<1)+0]); *current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(1<<1)+1]); } else current_line_buffer+=2; 
						
						PUTPIXEL_1_TB(c&0x80)
						PUTPIXEL_1_TB(c&0x40)
						PUTPIXEL_1_TB(c&0x20)
						PUTPIXEL_1_TB(c&0x10)
						PUTPIXEL_1_TB(c&0x08)
						PUTPIXEL_1_TB(c&0x04)
						PUTPIXEL_1_TB(c&0x02)
						PUTPIXEL_1_TB(c&0x01)
					}
					else
					{
						#define PUTPIXEL_1_B(C) if (C) { *current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(1<<1)+0]); *current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(1<<1)+1]); } else { *current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[0]); *current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[1]); }
						PUTPIXEL_1_B(c&0x80)
						PUTPIXEL_1_B(c&0x40)
						PUTPIXEL_1_B(c&0x20)
						PUTPIXEL_1_B(c&0x10)
						PUTPIXEL_1_B(c&0x08)
						PUTPIXEL_1_B(c&0x04)
						PUTPIXEL_1_B(c&0x02)
						PUTPIXEL_1_B(c&0x01)
					}
					iwidth--;
				}
			}
		}
		else
		{
			if (flags&FLAGS_HFLIP)
			{
				while (iwidth)
				{
					c=jaguar_byte_read(ptr++);
					
					if (flags&FLAGS_TRANSPARENT)
					{
						#define PUTPIXEL_1_T_HFLIP(C) if (C) { *current_line_buffer--=paletteRam[(1<<1)+0]; *current_line_buffer--=paletteRam[(1<<1)+1]; } else current_line_buffer-=2; 
						
						PUTPIXEL_1_T_HFLIP(c&0x80)
						PUTPIXEL_1_T_HFLIP(c&0x40)
						PUTPIXEL_1_T_HFLIP(c&0x20)
						PUTPIXEL_1_T_HFLIP(c&0x10)
						PUTPIXEL_1_T_HFLIP(c&0x08)
						PUTPIXEL_1_T_HFLIP(c&0x04)
						PUTPIXEL_1_T_HFLIP(c&0x02)
						PUTPIXEL_1_T_HFLIP(c&0x01)
					}
					else
					{
						#define PUTPIXEL_1_HFLIP(C) if (C) { *current_line_buffer--=paletteRam[(1<<1)+0]; *current_line_buffer--=paletteRam[(1<<1)+1]; } else { *current_line_buffer--=paletteRam[0]; *current_line_buffer--=paletteRam[1]; }
						PUTPIXEL_1_HFLIP(c&0x80)
						PUTPIXEL_1_HFLIP(c&0x40)
						PUTPIXEL_1_HFLIP(c&0x20)
						PUTPIXEL_1_HFLIP(c&0x10)
						PUTPIXEL_1_HFLIP(c&0x08)
						PUTPIXEL_1_HFLIP(c&0x04)
						PUTPIXEL_1_HFLIP(c&0x02)
						PUTPIXEL_1_HFLIP(c&0x01)
					}
					iwidth--;
				}
			}
			else
			{
				while (iwidth)
				{
					c=jaguar_byte_read(ptr++);
					
					if (flags&FLAGS_TRANSPARENT)
					{
						#define PUTPIXEL_1_T(C) if (C) { *current_line_buffer++=paletteRam[(1<<1)+0]; *current_line_buffer++=paletteRam[(1<<1)+1]; } else current_line_buffer+=2; 
						
						PUTPIXEL_1_T(c&0x80)
						PUTPIXEL_1_T(c&0x40)
						PUTPIXEL_1_T(c&0x20)
						PUTPIXEL_1_T(c&0x10)
						PUTPIXEL_1_T(c&0x08)
						PUTPIXEL_1_T(c&0x04)
						PUTPIXEL_1_T(c&0x02)
						PUTPIXEL_1_T(c&0x01)
					}
					else
					{
						#define PUTPIXEL_1(C) if (C) { *current_line_buffer++=paletteRam[(1<<1)+0]; *current_line_buffer++=paletteRam[(1<<1)+1]; } else { *current_line_buffer++=paletteRam[0]; *current_line_buffer++=paletteRam[1]; }
						PUTPIXEL_1(c&0x80)
						PUTPIXEL_1(c&0x40)
						PUTPIXEL_1(c&0x20)
						PUTPIXEL_1(c&0x10)
						PUTPIXEL_1(c&0x08)
						PUTPIXEL_1(c&0x04)
						PUTPIXEL_1(c&0x02)
						PUTPIXEL_1(c&0x01)
					}
					iwidth--;
				}
			}
		}
