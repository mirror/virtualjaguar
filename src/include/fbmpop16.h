		if (flags & FLAGS_READMODIFY)
		{
			if (flags & FLAGS_HFLIP)
			{
				while (iwidth)
				{
					uint8 dataHi = jaguar_byte_read(ptr++), dataLo = jaguar_byte_read(ptr++);

					if (flags & FLAGS_TRANSPARENT)
					{					
//						uint16 data = jaguar_byte_read(ptr++);
//						data <<= 8;
//						data |= jaguar_byte_read(ptr++);
/*						if (data)
						{
							*current_line_buffer-- = BLEND_Y(*current_line_buffer, data >> 8);
							*current_line_buffer-- = BLEND_CC(*current_line_buffer, data & 0xFF);
						}*/
						if (dataHi | dataLo)
						{
							*current_line_buffer-- = BLEND_Y(*current_line_buffer, dataHi);
							*current_line_buffer-- = BLEND_CC(*current_line_buffer, dataLo);
						}
						else
							current_line_buffer -= 2;
					}
					else	// !FLAGS_TRANSPARENT
					{
//						*current_line_buffer-- = BLEND_Y(*current_line_buffer, jaguar_byte_read(ptr++));
//						*current_line_buffer-- = BLEND_CC(*current_line_buffer, jaguar_byte_read(ptr++));
						*current_line_buffer-- = BLEND_Y(*current_line_buffer, dataHi);
						*current_line_buffer-- = BLEND_CC(*current_line_buffer, dataLo);
					}
					iwidth--;
				}
			}
			else	// !FLAGS_HFLIP
			{
				while (iwidth)
				{
					uint8 dataHi = jaguar_byte_read(ptr++), dataLo = jaguar_byte_read(ptr++);

					if (flags & FLAGS_TRANSPARENT)
					{					
/*						uint16 data = jaguar_byte_read(ptr++);
						data <<= 8;
						data |= jaguar_byte_read(ptr++);
						if (data)
						{
							*current_line_buffer++ = BLEND_Y(*current_line_buffer, data >> 8);
							*current_line_buffer++ = BLEND_CC(*current_line_buffer, data & 0xFF);
						}*/
						if (dataHi | dataLo)
						{
							*current_line_buffer++ = BLEND_Y(*current_line_buffer, dataHi);
							*current_line_buffer++ = BLEND_CC(*current_line_buffer, dataLo);
						}
						else
							current_line_buffer += 2;
					}
					else
					{
//						*current_line_buffer++ = BLEND_Y(*current_line_buffer, jaguar_byte_read(ptr++));
//						*current_line_buffer++ = BLEND_CC(*current_line_buffer, jaguar_byte_read(ptr++));
						*current_line_buffer++ = BLEND_Y(*current_line_buffer, dataHi);
						*current_line_buffer++ = BLEND_CC(*current_line_buffer, dataLo);
					}
					iwidth--;
				}
			}
		}
		else	// !FLAGS_READMODIFY
		{
			if (flags & FLAGS_HFLIP)
			{
				while (iwidth)
				{
					uint8 dataHi = jaguar_byte_read(ptr++), dataLo = jaguar_byte_read(ptr++);

					if (flags & FLAGS_TRANSPARENT)
					{					
/*						uint16 data = jaguar_byte_read(ptr++);
						data <<= 8;
						data |= jaguar_byte_read(ptr++);
						if (data)
						{
							*current_line_buffer-- = data >> 8;
							*current_line_buffer-- = data & 0xFF;
						}*/
						if (dataHi | dataLo)
							*current_line_buffer-- = dataHi,
							*current_line_buffer-- = dataLo;
						else
							current_line_buffer -= 2;
					}
					else
					{
//						*current_line_buffer-- = jaguar_byte_read(ptr++);
//						*current_line_buffer-- = jaguar_byte_read(ptr++);
						*current_line_buffer-- = dataHi;
						*current_line_buffer-- = dataLo;
					}
					iwidth--;
				}
			}
			else		// !FLAGS_HFLIP
			{
				int count = 0;

				while (iwidth)
				{
					uint8 dataHi = jaguar_byte_read(ptr++), dataLo = jaguar_byte_read(ptr++);

					if (flags & FLAGS_TRANSPARENT)
					{
/*						uint16 data = jaguar_byte_read(ptr++);
						data <<= 8;
						data |= jaguar_byte_read(ptr++);
						if (data)
						{
							*current_line_buffer++ = data >> 8;
							*current_line_buffer++ = data & 0xFF;
						}*/
						if (dataHi | dataLo)
							*current_line_buffer++ = dataHi,
							*current_line_buffer++ = dataLo;
						else
							current_line_buffer += 2;
					}
					else
					{
//						*current_line_buffer++ = jaguar_byte_read(ptr++);
//						*current_line_buffer++ = jaguar_byte_read(ptr++);
						*current_line_buffer++ = dataHi;
						*current_line_buffer++ = dataLo;
					}				
					count += 2;
					if (count == 8) // 8 bytes = a phrase (64 bits)
					{
						ptr += 8 * (pitch - 1);
						count = 0;
					}
					iwidth--;
				}
			}
		}
