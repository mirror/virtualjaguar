		uint8	*paletteRam=&tom_ram_8[0x400];
		paletteRam+=(idx<<2);
		
		if (flags&FLAGS_HFLIP)
		{
			while (scaled_width)
			{
				uint32 c=jaguar_byte_read(ptr+(cnt>>19));
				c=(c>>(8-((cnt>>16)&0x07)))&0x01;
				if (flags&FLAGS_TRANSPARENT)
				{
					if (c)
					{
						*current_line_buffer--=paletteRam[(c<<1)+0];
						*current_line_buffer--=paletteRam[(c<<1)+1];
					}
					else
						current_line_buffer-=2;
				}
				else
				{
					*current_line_buffer--=paletteRam[(c<<1)+0];
					*current_line_buffer--=paletteRam[(c<<1)+1];
				}
				cnt+=hscale_fixed;
				scaled_width--;
			}
		}
		else
		{
			while (scaled_width)
			{
				uint32 c=jaguar_byte_read(ptr+(cnt>>19));
				c=(c>>(8-((cnt>>16)&0x07)))&0x01;
				if (flags&FLAGS_TRANSPARENT)
				{
					if (c)
					{
						*current_line_buffer++=paletteRam[(c<<1)+0];
						*current_line_buffer++=paletteRam[(c<<1)+1];
					}
					else
						current_line_buffer+=2;
				}
				else
				{
					*current_line_buffer++=paletteRam[(c<<1)+0];
					*current_line_buffer++=paletteRam[(c<<1)+1];
				}
				cnt+=hscale_fixed;
				scaled_width--;
			}
		}
