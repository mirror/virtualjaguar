		int count=0;
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
					count+=4;
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
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					iwidth--;
					count+=4;
					if (count==8) // 8 bytes = a phrase (64 bits)
					{
						ptr+=8*(pitch-1);
						count=0;
					}
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
					count+=4;
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
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					*current_line_buffer++=jaguar_byte_read(ptr++);
					count+=4;
					if (count==8) // 8 bytes = a phrase (64 bits)
					{
						ptr+=8*(pitch-1);
						count=0;
					}
					iwidth--;
				}
			}
		}
