		if (flags&FLAGS_READMODIFY)
		{
			if (flags&FLAGS_HFLIP)
			{
				if (flags&FLAGS_TRANSPARENT)
				{					
					while (scaled_width)
					{
						uint16 c=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+0);;
						c<<=8;
						c|=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+1);
						if (c)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,(c>>8));
							*current_line_buffer--=BLEND_CC(*current_line_buffer,(c&0xff));
						}
						else
							current_line_buffer-=2;
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
				else
				{
					while (scaled_width)
					{
						*current_line_buffer--=BLEND_Y(*current_line_buffer,jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+0));
						*current_line_buffer--=BLEND_CC(*current_line_buffer,jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+1));
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
			}
			else
			{
				if (flags&FLAGS_TRANSPARENT)
				{					
					while (scaled_width)
					{
						uint16 c=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+0);;
						c<<=8;
						c|=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+1);
						if (c)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,(c>>8));
							*current_line_buffer++=BLEND_CC(*current_line_buffer,(c&0xff));
						}
						else
							current_line_buffer+=2;
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
				else
				{
					while (scaled_width)
					{
						*current_line_buffer++=BLEND_Y(*current_line_buffer,jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+0));
						*current_line_buffer++=BLEND_CC(*current_line_buffer,jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+1));
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
			}
		}
		else
		{
			if (flags&FLAGS_HFLIP)
			{
				if (flags&FLAGS_TRANSPARENT)
				{					
					while (scaled_width)
					{
						uint16 c=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+0);;
						c<<=8;
						c|=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+1);
						if (c)
						{
							*current_line_buffer--=(c>>8);
							*current_line_buffer--=(c&0xff);
						}
						else
							current_line_buffer-=2;
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
				else
				{
					while (scaled_width)
					{
						*current_line_buffer--=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+0);
						*current_line_buffer--=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+1);
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
			}
			else
			{
				if (flags&FLAGS_TRANSPARENT)
				{					
					while (scaled_width)
					{
						uint16 c=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+0);;
						c<<=8;
						c|=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+1);
						if (c)
						{
							*current_line_buffer++=(c>>8);
							*current_line_buffer++=(c&0xff);
						}
						else
							current_line_buffer+=2;
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
				else
				{
					while (scaled_width)
					{
						*current_line_buffer++=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+0);
						*current_line_buffer++=jaguar_byte_read(ptr+((((cnt>>16) & (~0x3))*pitch)+((cnt>>16)&0x3)<<1)+1);
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
			}
		}
