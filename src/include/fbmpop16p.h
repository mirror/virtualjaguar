		int count=0;
		if (flags&FLAGS_READMODIFY)
		{
			if (flags&FLAGS_HFLIP)
			{
				while (iwidth)
				{
					if (flags&FLAGS_TRANSPARENT)
					{					
						uint16 data=jaguar_byte_read(ptr++);
						data<<=8;
						data|=jaguar_byte_read(ptr++);
						if (data)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,(data>>8));
							*current_line_buffer--=BLEND_CC(*current_line_buffer,(data&0xff));
						}
						else
							current_line_buffer-=2;
					}
					else
					{
						*current_line_buffer--=BLEND_Y(*current_line_buffer,jaguar_byte_read(ptr++));
						*current_line_buffer--=BLEND_CC(*current_line_buffer,jaguar_byte_read(ptr++));
					}
					count+=2;
					if (count==8) // 8 bytes = a phrase (64 bits)
					{
						ptr-=8*(pitch-1);
						count=0;
					}
					iwidth--;
				}
			}
			else
			{
				while (iwidth)
				{
					if (flags&FLAGS_TRANSPARENT)
					{					
						uint16 data=jaguar_byte_read(ptr++);
						data<<=8;
						data|=jaguar_byte_read(ptr++);
						if (data)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,(data>>8));
							*current_line_buffer++=BLEND_CC(*current_line_buffer,(data&0xff));
						}
						else
							current_line_buffer+=2;
					}
					else
					{
						*current_line_buffer++=BLEND_Y(*current_line_buffer,jaguar_byte_read(ptr++));
						*current_line_buffer++=BLEND_CC(*current_line_buffer,jaguar_byte_read(ptr++));
					}
					count+=2;
					if (count==8) // 8 bytes = a phrase (64 bits)
					{
						ptr+=8*(pitch-1);
						count=0;
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
					if (flags&FLAGS_TRANSPARENT)
					{					
						uint16 data=jaguar_byte_read(ptr++);
						data<<=8;
						data|=jaguar_byte_read(ptr++);
						if (data)
						{
							*current_line_buffer--=(data>>8);
							*current_line_buffer--=(data&0xff);
						}
						else
							current_line_buffer-=2;
					}
					else
					{
						*current_line_buffer--=jaguar_byte_read(ptr++);
						*current_line_buffer--=jaguar_byte_read(ptr++);
					}
					count+=2;
					if (count==8) // 8 bytes = a phrase (64 bits)
					{
						ptr-=8*(pitch-1);
						count=0;
					}
					iwidth--;
				}
			}
			else
			{
				while (iwidth)
				{
					if (flags&FLAGS_TRANSPARENT)
					{					
						uint16 data=jaguar_byte_read(ptr++);
						data<<=8;
						data|=jaguar_byte_read(ptr++);
						if (data)
						{
							*current_line_buffer++=(data>>8);
							*current_line_buffer++=(data&0xff);
						}
						else
							current_line_buffer+=2;
					}
					else
					{
						*current_line_buffer++=jaguar_byte_read(ptr++);
						*current_line_buffer++=jaguar_byte_read(ptr++);
					}				
					count+=2;
					if (count==8) // 8 bytes = a phrase (64 bits)
					{
						ptr+=8*(pitch-1);
						count=0;
					}
					iwidth--;
				}
			}
		}
