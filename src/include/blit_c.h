#ifndef blitter_code_0x30000000
#define blitter_code_0x30000000
void blitter_0x30000000(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_1(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34010004
#define blitter_code_0x34010004
void blitter_0x34010004(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c002a4
#define blitter_code_0x30c002a4
void blitter_0x30c002a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				{
				int intensity = srcdata & 0xFF;
				int ia = gd_ia >> 16;
				if(ia & 0x80)
					ia = 0xFFFFFF00 | ia;
				intensity += ia;
				if(intensity < 0)
					intensity = 0;
				if(intensity > 0xFF)
					intensity = 0xFF;
				writedata = (srcdata & 0xFF00) | intensity;
				}
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c042a4
#define blitter_code_0x00c042a4
void blitter_0x00c042a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c042a4
#define blitter_code_0x08c042a4
void blitter_0x08c042a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (srcdata == READ_RDATA_16(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34010024
#define blitter_code_0x34010024
void blitter_0x34010024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010222
#define blitter_code_0x24010222
void blitter_0x24010222(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_4(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_4(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_4(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_4(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c04292
#define blitter_code_0x00c04292
void blitter_0x00c04292(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_4(a2);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_4(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_4(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c0409b
#define blitter_code_0x00c0409b
void blitter_0x00c0409b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30000003
#define blitter_code_0x30000003
void blitter_0x30000003(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34010003
#define blitter_code_0x34010003
void blitter_0x34010003(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c000ad
#define blitter_code_0x30c000ad
void blitter_0x30c000ad(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a2);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c0009b
#define blitter_code_0x30c0009b
void blitter_0x30c0009b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401001b
#define blitter_code_0x3401001b
void blitter_0x3401001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c000a4
#define blitter_code_0x30c000a4
void blitter_0x30c000a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34010023
#define blitter_code_0x34010023
void blitter_0x34010023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30000023
#define blitter_code_0x30000023
void blitter_0x30000023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010024
#define blitter_code_0x24010024
void blitter_0x24010024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c000a4
#define blitter_code_0x00c000a4
void blitter_0x00c000a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c040a4
#define blitter_code_0x00c040a4
void blitter_0x00c040a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04010024
#define blitter_code_0x04010024
void blitter_0x04010024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401001c
#define blitter_code_0x2401001c
void blitter_0x2401001c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3000002b
#define blitter_code_0x3000002b
void blitter_0x3000002b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3000001b
#define blitter_code_0x3000001b
void blitter_0x3000001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c0009b
#define blitter_code_0x00c0009b
void blitter_0x00c0009b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x1000001b
#define blitter_code_0x1000001b
void blitter_0x1000001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x10000023
#define blitter_code_0x10000023
void blitter_0x10000023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401001d
#define blitter_code_0x3401001d
void blitter_0x3401001d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c000e4
#define blitter_code_0x00c000e4
void blitter_0x00c000e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			srczdata=z_i[colour_index]>>16;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				{
				int intensity = srcdata & 0xFF;
				int ia = gd_ia >> 16;
				if(ia & 0x80)
					ia = 0xFFFFFF00 | ia;
				intensity += ia;
				if(intensity < 0)
					intensity = 0;
				if(intensity > 0xFF)
					intensity = 0xFF;
				writedata = (srcdata & 0xFF00) | intensity;
				}
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010064
#define blitter_code_0x24010064
void blitter_0x24010064(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a2, a2_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c040e4
#define blitter_code_0x00c040e4
void blitter_0x00c040e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				{
				int intensity = srcdata & 0xFF;
				int ia = gd_ia >> 16;
				if(ia & 0x80)
					ia = 0xFFFFFF00 | ia;
				intensity += ia;
				if(intensity < 0)
					intensity = 0;
				if(intensity > 0xFF)
					intensity = 0xFF;
				writedata = (srcdata & 0xFF00) | intensity;
				}
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2000002b
#define blitter_code_0x2000002b
void blitter_0x2000002b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00a00058
#define blitter_code_0x00a00058
void blitter_0x00a00058(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a1, a1_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata |= ~srcdata & dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x050100d8
#define blitter_code_0x050100d8
void blitter_0x050100d8(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a1);
			srczdata = READ_RDATA_1(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a2, a2_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c00280
#define blitter_code_0x00c00280
void blitter_0x00c00280(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_1(a2);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_1(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04010064
#define blitter_code_0x04010064
void blitter_0x04010064(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a2, a2_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010003
#define blitter_code_0x24010003
void blitter_0x24010003(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010023
#define blitter_code_0x24010023
void blitter_0x24010023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c0429b
#define blitter_code_0x00c0429b
void blitter_0x00c0429b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c042db
#define blitter_code_0x00c042db
void blitter_0x00c042db(void)
{
	uint32 srcdata   = 0;
	uint32 srczdata  = 0;
	uint32 dstdata   = 0;
	uint32 dstzdata  = 0;
	uint32 writedata = 0;
	uint32 inhibit   = 0;
	uint32 compare_value=READ_RDATA_8(PATTERNDATA, a1,a1_phrase_mode);

	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			srcdata   = 0;
			writedata = 0;

			srcdata = READ_PIXEL_8(a1);
			if (srcdata != compare_value )
			{
				dstdata = READ_PIXEL_8(a2);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_8(a2, writedata);
			} 
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x1401001b
#define blitter_code_0x1401001b
void blitter_0x1401001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010004
#define blitter_code_0x24010004
void blitter_0x24010004(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x308000a4
#define blitter_code_0x308000a4
void blitter_0x308000a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14010024
#define blitter_code_0x14010024
void blitter_0x14010024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14010023
#define blitter_code_0x14010023
void blitter_0x14010023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30e000a4
#define blitter_code_0x30e000a4
void blitter_0x30e000a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata |= ~srcdata & dstdata;
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x10000013
#define blitter_code_0x10000013
void blitter_0x10000013(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c00292
#define blitter_code_0x00c00292
void blitter_0x00c00292(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_4(a2);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_4(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x008000a4
#define blitter_code_0x008000a4
void blitter_0x008000a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00800092
#define blitter_code_0x00800092
void blitter_0x00800092(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_4(a2);
			dstdata = READ_RDATA_4(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_4(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2cc10023
#define blitter_code_0x2cc10023
void blitter_0x2cc10023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34c10023
#define blitter_code_0x34c10023
void blitter_0x34c10023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x38c0429b
#define blitter_code_0x38c0429b
void blitter_0x38c0429b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3cc1001b
#define blitter_code_0x3cc1001b
void blitter_0x3cc1001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2cc1001b
#define blitter_code_0x2cc1001b
void blitter_0x2cc1001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c0429b
#define blitter_code_0x08c0429b
void blitter_0x08c0429b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x1cc1001b
#define blitter_code_0x1cc1001b
void blitter_0x1cc1001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0cc1001b
#define blitter_code_0x0cc1001b
void blitter_0x0cc1001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3cc10023
#define blitter_code_0x3cc10023
void blitter_0x3cc10023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c0009b
#define blitter_code_0x08c0009b
void blitter_0x08c0009b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x28c0429b
#define blitter_code_0x28c0429b
void blitter_0x28c0429b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x18c0429b
#define blitter_code_0x18c0429b
void blitter_0x18c0429b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34010005
#define blitter_code_0x34010005
void blitter_0x34010005(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401002d
#define blitter_code_0x3401002d
void blitter_0x3401002d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14010025
#define blitter_code_0x14010025
void blitter_0x14010025(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34010025
#define blitter_code_0x34010025
void blitter_0x34010025(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401006c
#define blitter_code_0x3401006c
void blitter_0x3401006c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a2, a2_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a2, a2_phrase_mode);
			WRITE_PIXEL_32(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401022a
#define blitter_code_0x2401022a
void blitter_0x2401022a(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_4(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_4(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_4(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_4(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401006a
#define blitter_code_0x2401006a
void blitter_0x2401006a(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a2, a2_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a2, a2_phrase_mode);
			WRITE_PIXEL_32(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c002e4
#define blitter_code_0x00c002e4
void blitter_0x00c002e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_PIXEL_16(a2);
			dstzdata = READ_RDATA_16(DSTZ, a2, a2_phrase_mode);
			srczdata=z_i[colour_index]>>16;
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				{
				int intensity = srcdata & 0xFF;
				int ia = gd_ia >> 16;
				if(ia & 0x80)
					ia = 0xFFFFFF00 | ia;
				intensity += ia;
				if(intensity < 0)
					intensity = 0;
				if(intensity > 0xFF)
					intensity = 0xFF;
				writedata = (srcdata & 0xFF00) | intensity;
				}
				WRITE_PIXEL_16(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04010212
#define blitter_code_0x04010212
void blitter_0x04010212(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_4(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_4(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_4(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_4(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2c010023
#define blitter_code_0x2c010023
void blitter_0x2c010023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3c010023
#define blitter_code_0x3c010023
void blitter_0x3c010023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401006b
#define blitter_code_0x3401006b
void blitter_0x3401006b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_32(PATTERNDATA, a2, a2_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_32(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c000ad
#define blitter_code_0x00c000ad
void blitter_0x00c000ad(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a2);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00e002ad
#define blitter_code_0x00e002ad
void blitter_0x00e002ad(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a2);
			dstdata = READ_PIXEL_32(a1);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
				writedata |= ~srcdata & dstdata;
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c040a4
#define blitter_code_0x08c040a4
void blitter_0x08c040a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (srcdata == READ_RDATA_16(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c000e4
#define blitter_code_0x08c000e4
void blitter_0x08c000e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			srczdata=z_i[colour_index]>>16;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				{
				int intensity = srcdata & 0xFF;
				int ia = gd_ia >> 16;
				if(ia & 0x80)
					ia = 0xFFFFFF00 | ia;
				intensity += ia;
				if(intensity < 0)
					intensity = 0;
				if(intensity > 0xFF)
					intensity = 0xFF;
				writedata = (srcdata & 0xFF00) | intensity;
				}
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2c010024
#define blitter_code_0x2c010024
void blitter_0x2c010024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3c010024
#define blitter_code_0x3c010024
void blitter_0x3c010024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0c010024
#define blitter_code_0x0c010024
void blitter_0x0c010024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x1401001d
#define blitter_code_0x1401001d
void blitter_0x1401001d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34c10003
#define blitter_code_0x34c10003
void blitter_0x34c10003(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x306002a4
#define blitter_code_0x306002a4
void blitter_0x306002a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
				writedata |= ~srcdata & dstdata;
				writedata |= srcdata  & ~dstdata;
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c00292
#define blitter_code_0x08c00292
void blitter_0x08c00292(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_4(a2);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_4(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14c10013
#define blitter_code_0x14c10013
void blitter_0x14c10013(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x38c0009b
#define blitter_code_0x38c0009b
void blitter_0x38c0009b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34c1001b
#define blitter_code_0x34c1001b
void blitter_0x34c1001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x1401001c
#define blitter_code_0x1401001c
void blitter_0x1401001c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c000db
#define blitter_code_0x08c000db
void blitter_0x08c000db(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 dstdata   = 0;
			uint32 writedata = 0;
			srcdata = READ_PIXEL_8(a1);
			dstdata = READ_RDATA_8(DSTDATA, a2, a2_phrase_mode);
			writedata |= srcdata  & ~dstdata;
			writedata |= srcdata  & dstdata;
			WRITE_PIXEL_8(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0401001b
#define blitter_code_0x0401001b
void blitter_0x0401001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0401001d
#define blitter_code_0x0401001d
void blitter_0x0401001d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0401001c
#define blitter_code_0x0401001c
void blitter_0x0401001c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x36013624
#define blitter_code_0x36013624
void blitter_0x36013624(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_ZDATA_16(a1);
			srczdata=z_i[colour_index]>>16;
			if (srczdata == dstzdata)	inhibit = 1;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			WRITE_ZDATA_16(a1, srczdata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04c10284
#define blitter_code_0x04c10284
void blitter_0x04c10284(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14010004
#define blitter_code_0x14010004
void blitter_0x14010004(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x06c076e4
#define blitter_code_0x06c076e4
void blitter_0x06c076e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			srczdata = READ_RDATA_16(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_PIXEL_16(a2);
			dstzdata = READ_ZDATA_16(a2);
			srczdata=z_i[colour_index]>>16;
			if (srczdata == dstzdata)	inhibit = 1;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (srcdata == READ_RDATA_16(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
				WRITE_ZDATA_16(a2, srczdata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c040a4
#define blitter_code_0x30c040a4
void blitter_0x30c040a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			srczdata=z_i[colour_index]>>16;
			if (srcdata == READ_RDATA_16(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				{
				int intensity = srcdata & 0xFF;
				int ia = gd_ia >> 16;
				if(ia & 0x80)
					ia = 0xFFFFFF00 | ia;
				intensity += ia;
				if(intensity < 0)
					intensity = 0;
				if(intensity > 0xFF)
					intensity = 0xFF;
				writedata = (srcdata & 0xFF00) | intensity;
				}
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c040db
#define blitter_code_0x00c040db
void blitter_0x00c040db(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a1);
			dstdata = READ_RDATA_8(DSTDATA, a2, a2_phrase_mode);
			if (srcdata == READ_RDATA_8(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c0429b
#define blitter_code_0x30c0429b
void blitter_0x30c0429b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c0409b
#define blitter_code_0x08c0409b
void blitter_0x08c0409b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c0029b
#define blitter_code_0x00c0029b
void blitter_0x00c0029b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x20c00003
#define blitter_code_0x20c00003
void blitter_0x20c00003(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c0001b
#define blitter_code_0x00c0001b
void blitter_0x00c0001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x20c0002b
#define blitter_code_0x20c0002b
void blitter_0x20c0002b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c0001c
#define blitter_code_0x30c0001c
void blitter_0x30c0001c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c000db
#define blitter_code_0x00c000db
void blitter_0x00c000db(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a1);
			dstdata = READ_RDATA_8(DSTDATA, a2, a2_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_8(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c00023
#define blitter_code_0x00c00023
void blitter_0x00c00023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34c10024
#define blitter_code_0x34c10024
void blitter_0x34c10024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3cc10024
#define blitter_code_0x3cc10024
void blitter_0x3cc10024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04c10084
#define blitter_code_0x04c10084
void blitter_0x04c10084(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04010004
#define blitter_code_0x04010004
void blitter_0x04010004(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c000e4
#define blitter_code_0x30c000e4
void blitter_0x30c000e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04c10004
#define blitter_code_0x04c10004
void blitter_0x04c10004(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0cc10004
#define blitter_code_0x0cc10004
void blitter_0x0cc10004(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x1cc10024
#define blitter_code_0x1cc10024
void blitter_0x1cc10024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0cc10084
#define blitter_code_0x0cc10084
void blitter_0x0cc10084(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24c10024
#define blitter_code_0x24c10024
void blitter_0x24c10024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2cc10024
#define blitter_code_0x2cc10024
void blitter_0x2cc10024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401002c
#define blitter_code_0x3401002c
void blitter_0x3401002c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14010014
#define blitter_code_0x14010014
void blitter_0x14010014(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14010013
#define blitter_code_0x14010013
void blitter_0x14010013(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30000005
#define blitter_code_0x30000005
void blitter_0x30000005(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3000001d
#define blitter_code_0x3000001d
void blitter_0x3000001d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c0002c
#define blitter_code_0x30c0002c
void blitter_0x30c0002c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3000002d
#define blitter_code_0x3000002d
void blitter_0x3000002d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x10000025
#define blitter_code_0x10000025
void blitter_0x10000025(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x28c0002c
#define blitter_code_0x28c0002c
void blitter_0x28c0002c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3002022c
#define blitter_code_0x3002022c
void blitter_0x3002022c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata = (srcdata & 0xff) + (dstdata & 0xff);
				if (writedata > 0xff) writedata = 0xff;
				writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
				if (writedata > 0xfff) writedata = 0xfff;
				writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c040e4
#define blitter_code_0x08c040e4
void blitter_0x08c040e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c042a4
#define blitter_code_0x30c042a4
void blitter_0x30c042a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401002b
#define blitter_code_0x3401002b
void blitter_0x3401002b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14010005
#define blitter_code_0x14010005
void blitter_0x14010005(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2c01001b
#define blitter_code_0x2c01001b
void blitter_0x2c01001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401001c
#define blitter_code_0x3401001c
void blitter_0x3401001c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x05010083
#define blitter_code_0x05010083
void blitter_0x05010083(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x05010084
#define blitter_code_0x05010084
void blitter_0x05010084(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34010064
#define blitter_code_0x34010064
void blitter_0x34010064(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a2, a2_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14010064
#define blitter_code_0x14010064
void blitter_0x14010064(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a2, a2_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401021b
#define blitter_code_0x2401021b
void blitter_0x2401021b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010218
#define blitter_code_0x24010218
void blitter_0x24010218(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401021a
#define blitter_code_0x2401021a
void blitter_0x2401021a(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_4(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_4(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_4(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_4(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401021c
#define blitter_code_0x2401021c
void blitter_0x2401021c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c00064
#define blitter_code_0x00c00064
void blitter_0x00c00064(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a1, a1_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_16(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c00024
#define blitter_code_0x00c00024
void blitter_0x00c00024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x01c042e4
#define blitter_code_0x01c042e4
void blitter_0x01c042e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_PIXEL_16(a2);
			dstzdata = READ_RDATA_16(DSTZ, a2, a2_phrase_mode);
			srczdata=z_i[colour_index]>>16;
			if (srcdata == READ_RDATA_16(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				{
				int intensity = srcdata & 0xFF;
				int ia = gd_ia >> 16;
				if(ia & 0x80)
					ia = 0xFFFFFF00 | ia;
				intensity += ia;
				if(intensity < 0)
					intensity = 0;
				if(intensity > 0xFF)
					intensity = 0xFF;
				writedata = (srcdata & 0xFF00) | intensity;
				}
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x20c0001c
#define blitter_code_0x20c0001c
void blitter_0x20c0001c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010219
#define blitter_code_0x24010219
void blitter_0x24010219(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_2(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_2(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_2(a1);
			dstzdata = READ_RDATA_2(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_2(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_2(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x08c000a4
#define blitter_code_0x08c000a4
void blitter_0x08c000a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2c01001c
#define blitter_code_0x2c01001c
void blitter_0x2c01001c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3c01001c
#define blitter_code_0x3c01001c
void blitter_0x3c01001c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2c010218
#define blitter_code_0x2c010218
void blitter_0x2c010218(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_1(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3050021b
#define blitter_code_0x3050021b
void blitter_0x3050021b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata |= ~srcdata & ~dstdata;
				writedata |= srcdata  & ~dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34c10004
#define blitter_code_0x34c10004
void blitter_0x34c10004(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x05010218
#define blitter_code_0x05010218
void blitter_0x05010218(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30020224
#define blitter_code_0x30020224
void blitter_0x30020224(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
				writedata = (srcdata & 0xff) + (dstdata & 0xff);
				if (writedata > 0xff) writedata = 0xff;
				writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
				if (writedata > 0xfff) writedata = 0xfff;
				writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24c1001b
#define blitter_code_0x24c1001b
void blitter_0x24c1001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0501021b
#define blitter_code_0x0501021b
void blitter_0x0501021b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04c1001b
#define blitter_code_0x04c1001b
void blitter_0x04c1001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24c10023
#define blitter_code_0x24c10023
void blitter_0x24c10023(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x01c040a4
#define blitter_code_0x01c040a4
void blitter_0x01c040a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04c10024
#define blitter_code_0x04c10024
void blitter_0x04c10024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0501021c
#define blitter_code_0x0501021c
void blitter_0x0501021c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30e00224
#define blitter_code_0x30e00224
void blitter_0x30e00224(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata |= ~srcdata & dstdata;
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401002d
#define blitter_code_0x2401002d
void blitter_0x2401002d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401002c
#define blitter_code_0x2401002c
void blitter_0x2401002c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010005
#define blitter_code_0x24010005
void blitter_0x24010005(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04010220
#define blitter_code_0x04010220
void blitter_0x04010220(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c000ed
#define blitter_code_0x00c000ed
void blitter_0x00c000ed(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a1);
			dstdata = READ_RDATA_32(DSTDATA, a2, a2_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_32(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x040e026d
#define blitter_code_0x040e026d
void blitter_0x040e026d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_PIXEL_32(a2);
			dstzdata = READ_RDATA_32(DSTZ, a2, a2_phrase_mode);
				writedata = (srcdata & 0xff) + (dstdata & 0xff);
				writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
				writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
				WRITE_PIXEL_32(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0c01002c
#define blitter_code_0x0c01002c
void blitter_0x0c01002c(void)
{
//	fprintf(log_get(),"blitter_0x0c01002c: 0x%.8x 0x%.8x 0x%.8x 0x%.8x cpu %i pc=0x%.8x\n",
//					   gd_i[0],gd_c[0],gd_ia,gd_ca,jaguar_cpu_in_exec,gpu_read_pc()); 

	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 dstdata   = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			} 
			else 
			{ 
				writedata=dstdata; 
			}
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x040e026c
#define blitter_code_0x040e026c
void blitter_0x040e026c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_PIXEL_32(a2);
			dstzdata = READ_RDATA_32(DSTZ, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata = (srcdata & 0xff) + (dstdata & 0xff);
				writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
				writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_32(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c04280
#define blitter_code_0x00c04280
void blitter_0x00c04280(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_1(a2);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_1(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_1(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04010200
#define blitter_code_0x04010200
void blitter_0x04010200(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_1(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00600280
#define blitter_code_0x00600280
void blitter_0x00600280(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_1(a2);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata |= ~srcdata & dstdata;
				writedata |= srcdata  & ~dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_1(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04010005
#define blitter_code_0x04010005
void blitter_0x04010005(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_32(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0401002c
#define blitter_code_0x0401002c
void blitter_0x0401002c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c000db
#define blitter_code_0x30c000db
void blitter_0x30c000db(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a1);
			dstdata = READ_RDATA_8(DSTDATA, a2, a2_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_8(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c002ad
#define blitter_code_0x30c002ad
void blitter_0x30c002ad(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a2);
			dstdata = READ_PIXEL_32(a1);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3401009b
#define blitter_code_0x3401009b
void blitter_0x3401009b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04c1022d
#define blitter_code_0x04c1022d
void blitter_0x04c1022d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_32(a1);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x008002ad
#define blitter_code_0x008002ad
void blitter_0x008002ad(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a2);
			dstdata = READ_PIXEL_32(a1);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x14c1022d
#define blitter_code_0x14c1022d
void blitter_0x14c1022d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_32(a1);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34c1022d
#define blitter_code_0x34c1022d
void blitter_0x34c1022d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_32(a1);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_32(PATTERNDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c0029b
#define blitter_code_0x30c0029b
void blitter_0x30c0029b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24c1021b
#define blitter_code_0x24c1021b
void blitter_0x24c1021b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_8(a1);
			dstzdata = READ_RDATA_8(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c042ad
#define blitter_code_0x30c042ad
void blitter_0x30c042ad(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a2);
			dstdata = READ_PIXEL_32(a1);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_32(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3e01261c
#define blitter_code_0x3e01261c
void blitter_0x3e01261c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_ZDATA_16(a1);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			srczdata=z_i[colour_index]>>16;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			WRITE_ZDATA_16(a1, srczdata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00e00280
#define blitter_code_0x00e00280
void blitter_0x00e00280(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_1(a2);
			dstdata = READ_PIXEL_1(a1);
			dstzdata = READ_RDATA_1(DSTZ, a1, a1_phrase_mode);
				writedata |= ~srcdata & dstdata;
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00e00292
#define blitter_code_0x00e00292
void blitter_0x00e00292(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_4(a2);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
				writedata |= ~srcdata & dstdata;
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_4(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x308002a4
#define blitter_code_0x308002a4
void blitter_0x308002a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x00c002a4
#define blitter_code_0x00c002a4
void blitter_0x00c002a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x01c042a4
#define blitter_code_0x01c042a4
void blitter_0x01c042a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x06c066e4
#define blitter_code_0x06c066e4
void blitter_0x06c066e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			srczdata = READ_RDATA_16(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_PIXEL_16(a2);
			dstzdata = READ_ZDATA_16(a2);
			if (srczdata >  dstzdata)	inhibit = 1;
			if (srcdata == READ_RDATA_16(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
				WRITE_ZDATA_16(a2, srczdata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04010222
#define blitter_code_0x04010222
void blitter_0x04010222(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_4(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_4(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_4(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_4(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x24010224
#define blitter_code_0x24010224
void blitter_0x24010224(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
				WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2000001b
#define blitter_code_0x2000001b
void blitter_0x2000001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3000005b
#define blitter_code_0x3000005b
void blitter_0x3000005b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a1, a1_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a2, a2_phrase_mode);
			WRITE_PIXEL_8(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x38000100
#define blitter_code_0x38000100
void blitter_0x38000100(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_1(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x1c010024
#define blitter_code_0x1c010024
void blitter_0x1c010024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x000242e4
#define blitter_code_0x000242e4
void blitter_0x000242e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_PIXEL_16(a2);
			dstzdata = READ_RDATA_16(DSTZ, a2, a2_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata = (srcdata & 0xff) + (dstdata & 0xff);
				if (writedata > 0xff) writedata = 0xff;
				writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
				if (writedata > 0xfff) writedata = 0xfff;
				writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2c01002c
#define blitter_code_0x2c01002c
void blitter_0x2c01002c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c042e4
#define blitter_code_0x30c042e4
void blitter_0x30c042e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_PIXEL_16(a2);
			dstzdata = READ_RDATA_16(DSTZ, a2, a2_phrase_mode);
			if (srcdata == READ_RDATA_16(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3c01002c
#define blitter_code_0x3c01002c
void blitter_0x3c01002c(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0cc10212
#define blitter_code_0x0cc10212
void blitter_0x0cc10212(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_4(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_4(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_4(a1);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_4(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_4(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x38000301
#define blitter_code_0x38000301
void blitter_0x38000301(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_2(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_2(a1);
			dstzdata = READ_RDATA_2(DSTZ, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_2(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401002b
#define blitter_code_0x2401002b
void blitter_0x2401002b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c004ad
#define blitter_code_0x30c004ad
void blitter_0x30c004ad(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a2);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04012000
#define blitter_code_0x04012000
void blitter_0x04012000(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_1(DSTDATA, a1, a1_phrase_mode);
			srczdata=z_i[colour_index]>>16;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_1(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34012000
#define blitter_code_0x34012000
void blitter_0x34012000(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_1(DSTDATA, a1, a1_phrase_mode);
			srczdata=z_i[colour_index]>>16;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30c004ad
#define blitter_code_0x30c004ad
void blitter_0x30c004ad(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_32(a2);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			dstzdata = READ_RDATA_32(DSTZ, a1, a1_phrase_mode);
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x04012000
#define blitter_code_0x04012000
void blitter_0x04012000(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_1(DSTDATA, a1, a1_phrase_mode);
			srczdata=z_i[colour_index]>>16;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_1(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34012000
#define blitter_code_0x34012000
void blitter_0x34012000(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_1(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_1(DSTDATA, a1, a1_phrase_mode);
			srczdata=z_i[colour_index]>>16;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_1(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30808c92
#define blitter_code_0x30808c92
void blitter_0x30808c92(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_4(a2);
			dstdata = READ_RDATA_4(DSTDATA, a1, a1_phrase_mode);
			dstzdata = READ_RDATA_4(DSTZ, a1, a1_phrase_mode);
			if (srczdata <  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_4(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0040409b
#define blitter_code_0x0040409b
void blitter_0x0040409b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x1000001d
#define blitter_code_0x1000001d
void blitter_0x1000001d(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a1, a1_phrase_mode);
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_32(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0000409b
#define blitter_code_0x0000409b
void blitter_0x0000409b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a2);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (srcdata == READ_RDATA_8(PATTERNDATA, a2,a2_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_8(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x0cc10024
#define blitter_code_0x0cc10024
void blitter_0x0cc10024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401001b
#define blitter_code_0x2401001b
void blitter_0x2401001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
				WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x01c042db
#define blitter_code_0x01c042db
void blitter_0x01c042db(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_8(a1);
			dstdata = READ_PIXEL_8(a2);
			dstzdata = READ_RDATA_8(DSTZ, a2, a2_phrase_mode);
			if (srcdata == READ_RDATA_8(PATTERNDATA, a1,a1_phrase_mode)) inhibit=1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x34010224
#define blitter_code_0x34010224
void blitter_0x34010224(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_RDATA_16(DSTZ, a1, a1_phrase_mode);
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30808800
#define blitter_code_0x30808800
void blitter_0x30808800(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_1(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_1(DSTDATA, a1, a1_phrase_mode);
			if (srczdata <  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_1(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x06c026e4
#define blitter_code_0x06c026e4
void blitter_0x06c026e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			srczdata = READ_RDATA_16(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_PIXEL_16(a2);
			dstzdata = READ_ZDATA_16(a2);
			srczdata=z_i[colour_index]>>16;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
				WRITE_ZDATA_16(a2, srczdata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x06012624
#define blitter_code_0x06012624
void blitter_0x06012624(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_ZDATA_16(a1);
			srczdata=z_i[colour_index]>>16;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_16(PATTERNDATA, a1, a1_phrase_mode);
				writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a1, writedata);
				WRITE_ZDATA_16(a1, srczdata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			gd_i[colour_index] += gd_ia;
			gd_c[colour_index] += gd_ca;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x000200e4
#define blitter_code_0x000200e4
void blitter_0x000200e4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a1);
			dstdata = READ_RDATA_16(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata = (srcdata & 0xff) + (dstdata & 0xff);
				if (writedata > 0xff) writedata = 0xff;
				writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
				if (writedata > 0xfff) writedata = 0xfff;
				writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
			} else { srczdata=dstzdata; writedata=dstdata; }
			if (!inhibit)
			{
				WRITE_PIXEL_16(a2, writedata);
			}
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x37c026a4
#define blitter_code_0x37c026a4
void blitter_0x37c026a4(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_PIXEL_16(a2);
			srczdata = READ_RDATA_16(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_PIXEL_16(a1);
			dstzdata = READ_ZDATA_16(a1);
			srczdata=z_i[colour_index]>>16;
			if (srczdata >  dstzdata)	inhibit = 1;
			if (!inhibit)
			{
				writedata |= srcdata  & ~dstdata;
				writedata |= srcdata  & dstdata;
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_16(a1, writedata);
			WRITE_ZDATA_16(a1, srczdata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
			z_i[colour_index]+=zadd;
			 colour_index=(colour_index+1)&0x3;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x3c01001b
#define blitter_code_0x3c01001b
void blitter_0x3c01001b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_8(SRCDATA, a2, a2_phrase_mode);
			srczdata = READ_RDATA_8(SRCZINT, a2, a2_phrase_mode);
			dstdata = READ_RDATA_8(DSTDATA, a1, a1_phrase_mode);
			if (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;
			if (!inhibit)
			{
				writedata= READ_RDATA_8(PATTERNDATA, a1, a1_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_8(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x2401006b
#define blitter_code_0x2401006b
void blitter_0x2401006b(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_32(SRCDATA, a1, a1_phrase_mode);
			srczdata = READ_RDATA_32(SRCZINT, a1, a1_phrase_mode);
			dstdata = READ_RDATA_32(DSTDATA, a2, a2_phrase_mode);
			if (!inhibit)
			{
				writedata= READ_RDATA_32(PATTERNDATA, a2, a2_phrase_mode);
			} else { srczdata=dstzdata; writedata=dstdata; }
			WRITE_PIXEL_32(a2, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
#ifndef blitter_code_0x30000024
#define blitter_code_0x30000024
void blitter_0x30000024(void)
{
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			uint32 srcdata   = 0;
			uint32 srczdata  = 0;
			uint32 dstdata   = 0;
			uint32 dstzdata  = 0;
			uint32 writedata = 0;
			uint32 inhibit   = 0;
			srcdata = READ_RDATA_16(SRCDATA, a2, a2_phrase_mode);
			dstdata = READ_RDATA_16(DSTDATA, a1, a1_phrase_mode);
			WRITE_PIXEL_16(a1, writedata);
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;
		}
		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;
	}
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
#endif
