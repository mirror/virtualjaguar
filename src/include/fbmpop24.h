		if (flags&FLAGS_HFLIP)
		{
			if (flags&FLAGS_TRANSPARENT)
			{
				while (iwidth)
				{
					uint32 data=jaguar_byte_read(ptr++);
					data<<=8;
					data|=jaguar_byte_read(ptr++);
					data<<=8;
					data|=jaguar_byte_read(ptr++);
					data<<=8;
					data|=jaguar_byte_read(ptr++);
					if (data)
					{
						*current_line_buffer--=(data>>24)&0xff;
						*current_line_buffer--=(data>>16)&0xff;
						*current_line_buffer--=(data>> 8)&0xff;
						*current_line_buffer--=data      &0xff;
					}
					else
						current_line_buffer-=4;
					iwidth--;
				}
			}
			else
			{
				while (iwidth)
				{
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					iwidth--;
				}
			}
		}
		else
		{
			if (flags&FLAGS_TRANSPARENT)
			{
				while (iwidth)
				{
					uint32 data=jaguar_byte_read(ptr++);
					data<<=8;
					data|=jaguar_byte_read(ptr++);
					data<<=8;
					data|=jaguar_byte_read(ptr++);
					data<<=8;
					data|=jaguar_byte_read(ptr++);
					if (data)
					{
						*current_line_buffer++=(data>>24)&0xff;
						*current_line_buffer++=(data>>16)&0xff;
						*current_line_buffer++=(data>> 8)&0xff;
						*current_line_buffer++=data      &0xff;
					}
					else
						current_line_buffer+=4;
					iwidth--;
				}
			}
			else
			{
				while (iwidth)
				{
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					iwidth--;
				}
			}
		}
