		uint8	*paletteRam=&tom_ram_8[0x400];
		paletteRam+=(idx&0x1)*256*2;
		if (flags&FLAGS_READMODIFY)
		{
			if (flags&FLAGS_HFLIP)
			{
				while (scaled_width)
				{
					uint32 c=jaguar_byte_read(ptr+(cnt>>16));
					c<<=1;
					if (flags&FLAGS_TRANSPARENT)
					{					
						if (c)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[c+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[c+1]);
						}
						else
							current_line_buffer-=2;
					}
					else
					{
						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[c+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[c+1]);
					}

					cnt+=hscale_fixed;
					scaled_width--;
				}
			}
			else
			{
				while (scaled_width)
				{
					uint32 c=jaguar_byte_read(ptr+(cnt>>16));
					c<<=1;
					if (flags&FLAGS_TRANSPARENT)
					{					
						if (c)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[c+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[c+1]);
						}
						else
							current_line_buffer+=2;
					}
					else
					{
						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[c+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[c+1]);
					}

					cnt+=hscale_fixed;
					scaled_width--;
				}
			}
		}
		else
		{
			if (flags&FLAGS_HFLIP)
			{
				while (scaled_width)
				{
					uint32 c=jaguar_byte_read(ptr+(cnt>>16));
					c<<=1;
					if (flags&FLAGS_TRANSPARENT)
					{					
						if (c)
						{
							*current_line_buffer--=paletteRam[c+0];
							*current_line_buffer--=paletteRam[c+1];
						}
						else
							current_line_buffer-=2;
					}
					else
					{
						*current_line_buffer--=paletteRam[c+0];
						*current_line_buffer--=paletteRam[c+1];
					}

					cnt+=hscale_fixed;
					scaled_width--;
				}
			}
			else
			{
				while (scaled_width)
				{
					uint32 c=jaguar_byte_read(ptr+(cnt>>16));
					c<<=1;
					if (flags&FLAGS_TRANSPARENT)
					{					
						if (c)
						{
							*current_line_buffer++=paletteRam[c+0];
							*current_line_buffer++=paletteRam[c+1];
						}
						else
							current_line_buffer+=2;
					}
					else
					{
						*current_line_buffer++=paletteRam[c+0];
						*current_line_buffer++=paletteRam[c+1];
					}

					cnt+=hscale_fixed;
					scaled_width--;
				}
			}
		}
