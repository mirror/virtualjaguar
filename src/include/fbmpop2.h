		uint32 c;
		paletteRam+=idx*2;
		if (flags&FLAGS_READMODIFY)
		{
			if (flags&FLAGS_HFLIP)
			{
				if (flags&FLAGS_TRANSPARENT)
				{
					while (iwidth)
					{
						c=jaguar_byte_read(ptr++);
						uint32 c0=(c>>6)&0x03;
						uint32 c1=(c>>4)&0x03;
						uint32 c2=(c>>2)&0x03;
						uint32 c3=(c>>0)&0x03;

						if (c0)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(c0<<1)+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(c0<<1)+1]);
						}
						else
							current_line_buffer-=2;
						if (c1)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(c1<<1)+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(c1<<1)+1]);
						}
						else
							current_line_buffer-=2;
						if (c2)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(c2<<1)+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(c2<<1)+1]);
						}
						else
							current_line_buffer-=2;
						if (c3)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(c3<<1)+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(c3<<1)+1]);
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
						uint32 c0=(c>>6)&0x03;
						uint32 c1=(c>>4)&0x03;
						uint32 c2=(c>>2)&0x03;
						uint32 c3=(c>>0)&0x03;

						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(c0<<1)+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(c0<<1)+1]);
						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(c1<<1)+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(c1<<1)+1]);
						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(c2<<1)+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(c2<<1)+1]);
						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[(c3<<1)+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[(c3<<1)+1]);
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
						uint32 c0=(c>>6)&0x03;
						uint32 c1=(c>>4)&0x03;
						uint32 c2=(c>>2)&0x03;
						uint32 c3=(c>>0)&0x03;

						if (c0)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(c0<<1)+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(c0<<1)+1]);
						}
						else
							current_line_buffer+=2;
						if (c1)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(c1<<1)+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(c1<<1)+1]);
						}
						else
							current_line_buffer+=2;
						if (c2)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(c2<<1)+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(c2<<1)+1]);
						}
						else
							current_line_buffer+=2;
						if (c3)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(c3<<1)+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(c3<<1)+1]);
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
						uint32 c0=(c>>6)&0x03;
						uint32 c1=(c>>4)&0x03;
						uint32 c2=(c>>2)&0x03;
						uint32 c3=(c>>0)&0x03;

						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(c0<<1)+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(c0<<1)+1]);
						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(c1<<1)+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(c1<<1)+1]);
						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(c2<<1)+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(c2<<1)+1]);
						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[(c3<<1)+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[(c3<<1)+1]);
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
						uint32 c0=(c>>6)&0x03;
						uint32 c1=(c>>4)&0x03;
						uint32 c2=(c>>2)&0x03;
						uint32 c3=(c>>0)&0x03;

						if (c0)
						{
							*current_line_buffer--=paletteRam[(c0<<1)+0];
							*current_line_buffer--=paletteRam[(c0<<1)+1];
						}
						else
							current_line_buffer-=2;
						if (c1)
						{
							*current_line_buffer--=paletteRam[(c1<<1)+0];
							*current_line_buffer--=paletteRam[(c1<<1)+1];
						}
						else
							current_line_buffer-=2;
						if (c2)
						{
							*current_line_buffer--=paletteRam[(c2<<1)+0];
							*current_line_buffer--=paletteRam[(c2<<1)+1];
						}
						else
							current_line_buffer-=2;
						if (c3)
						{
							*current_line_buffer--=paletteRam[(c3<<1)+0];
							*current_line_buffer--=paletteRam[(c3<<1)+1];
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
						uint32 c0=(c>>6)&0x03;
						uint32 c1=(c>>4)&0x03;
						uint32 c2=(c>>2)&0x03;
						uint32 c3=(c>>0)&0x03;

						*current_line_buffer--=paletteRam[(c0<<1)+0];
						*current_line_buffer--=paletteRam[(c0<<1)+1];
						*current_line_buffer--=paletteRam[(c1<<1)+0];
						*current_line_buffer--=paletteRam[(c1<<1)+1];
						*current_line_buffer--=paletteRam[(c2<<1)+0];
						*current_line_buffer--=paletteRam[(c2<<1)+1];
						*current_line_buffer--=paletteRam[(c3<<1)+0];
						*current_line_buffer--=paletteRam[(c3<<1)+1];
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
						uint32 c0=(c>>6)&0x03;
						uint32 c1=(c>>4)&0x03;
						uint32 c2=(c>>2)&0x03;
						uint32 c3=(c>>0)&0x03;

						if (c0)
						{
							*current_line_buffer++=paletteRam[(c0<<1)+0];
							*current_line_buffer++=paletteRam[(c0<<1)+1];
						}
						else
							current_line_buffer+=2;
						if (c1)
						{
							*current_line_buffer++=paletteRam[(c1<<1)+0];
							*current_line_buffer++=paletteRam[(c1<<1)+1];
						}
						else
							current_line_buffer+=2;
						if (c2)
						{
							*current_line_buffer++=paletteRam[(c2<<1)+0];
							*current_line_buffer++=paletteRam[(c2<<1)+1];
						}
						else
							current_line_buffer+=2;
						if (c3)
						{
							*current_line_buffer++=paletteRam[(c3<<1)+0];
							*current_line_buffer++=paletteRam[(c3<<1)+1];
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
						uint32 c0=(c>>6)&0x03;
						uint32 c1=(c>>4)&0x03;
						uint32 c2=(c>>2)&0x03;
						uint32 c3=(c>>0)&0x03;

						*current_line_buffer++=paletteRam[(c0<<1)+0];
						*current_line_buffer++=paletteRam[(c0<<1)+1];
						*current_line_buffer++=paletteRam[(c1<<1)+0];
						*current_line_buffer++=paletteRam[(c1<<1)+1];
						*current_line_buffer++=paletteRam[(c2<<1)+0];
						*current_line_buffer++=paletteRam[(c2<<1)+1];
						*current_line_buffer++=paletteRam[(c3<<1)+0];
						*current_line_buffer++=paletteRam[(c3<<1)+1];
						iwidth--;
					}
				}
			}
		}
