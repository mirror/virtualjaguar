		uint32 c;
		if (jaguar_mainRom_crc32==0x66f8914c)
		{
			if (idx==12) 
				idx=64;
		}			
		paletteRam+=idx*4;

		if (flags&FLAGS_READMODIFY)
		{
			if (flags&FLAGS_HFLIP)
			{
				if (flags&FLAGS_TRANSPARENT)
				{
					while (iwidth)
					{
						c=jaguar_byte_read(ptr++);
						uint32 cl=c>>4;
						uint32 ch=c&0x0f;
						if (cl)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(cl<<1)+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(cl<<1)+1]);
						}
						else
							current_line_buffer-=2;
						if (ch)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(ch<<1)+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(ch<<1)+1]);
						}
						else
							current_line_buffer-=2;
						iwidth--;
					}
				}
				else
				{
					while (iwidth)
					{
						c=jaguar_byte_read(ptr++);
						uint32 cl=c>>4;
						uint32 ch=c&0x0f;
						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(cl<<1)+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(cl<<1)+1]);
						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(ch<<1)+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(ch<<1)+1]);
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
						c=jaguar_byte_read(ptr++);
						uint32 cl=c>>4;
						uint32 ch=c&0x0f;
						if (cl)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(cl<<1)+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(cl<<1)+1]);
						}
						else
							current_line_buffer+=2;
						if (ch)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(ch<<1)+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(ch<<1)+1]);
						}
						else
							current_line_buffer+=2;
						iwidth--;
					}
				}
				else
				{
					while (iwidth)
					{
						c=jaguar_byte_read(ptr++);
						uint32 cl=c>>4;
						uint32 ch=c&0x0f;
						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(cl<<1)+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(cl<<1)+1]);
						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(ch<<1)+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(ch<<1)+1]);
						iwidth--;
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
					while (iwidth)
					{
						c=jaguar_byte_read(ptr++);
						uint32 cl=c>>4;
						uint32 ch=c&0x0f;
						if (cl)
						{
							*current_line_buffer--=paletteRam[(cl<<1)+0];
							*current_line_buffer--=paletteRam[(cl<<1)+1];
						}
						else
							current_line_buffer-=2;
						if (ch)
						{
							*current_line_buffer--=paletteRam[(ch<<1)+0];
							*current_line_buffer--=paletteRam[(ch<<1)+1];
						}
						else
							current_line_buffer-=2;
						iwidth--;
					}
				}
				else
				{
					while (iwidth)
					{
						c=jaguar_byte_read(ptr++);
						uint32 cl=c>>4;
						uint32 ch=c&0x0f;
						*current_line_buffer--=paletteRam[(cl<<1)+0];
						*current_line_buffer--=paletteRam[(cl<<1)+1];
						*current_line_buffer--=paletteRam[(ch<<1)+0];
						*current_line_buffer--=paletteRam[(ch<<1)+1];
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
						c=jaguar_byte_read(ptr++);
						uint32 cl=c>>4;
						uint32 ch=c&0x0f;
						if (cl)
						{
							*current_line_buffer++=paletteRam[(cl<<1)+0];
							*current_line_buffer++=paletteRam[(cl<<1)+1];
						}
						else
							current_line_buffer+=2;
						if (ch)
						{
							*current_line_buffer++=paletteRam[(ch<<1)+0];
							*current_line_buffer++=paletteRam[(ch<<1)+1];
						}
						else
							current_line_buffer+=2;
						iwidth--;
					}
				}
				else
				{
					while (iwidth)
					{
						c=jaguar_byte_read(ptr++);
						uint32 cl=c>>4;
						uint32 ch=c&0x0f;
						*current_line_buffer++=paletteRam[(cl<<1)+0];
						*current_line_buffer++=paletteRam[(cl<<1)+1];
						*current_line_buffer++=paletteRam[(ch<<1)+0];
						*current_line_buffer++=paletteRam[(ch<<1)+1];
						iwidth--;
					}
				}
			}
		}
