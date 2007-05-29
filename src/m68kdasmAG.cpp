/*
 *   A MC68000/MC68010 disassembler
 *
 *   Note: this is probably not the most efficient disassembler in the world :-)
 *
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 */

#include <string.h>
#include <stdio.h>

static char *ccodes[16] = { "T ", "F ", "HI", "LS", "CC", "CS", "NE", "EQ", "VC", "VS", "PL", "MI", "GE", "LT", "GT", "LE" };

#define PARAM_WORD(v) ((v) = *(unsigned short *)&p[0], p += 2)
#define PARAM_LONG(v) ((v) = (*(unsigned short *)&p[0] << 16) + *(unsigned short *)&p[2], p += 4)

#ifdef LOGGING

static char *MakeEA (int lo, char *pBase, int size, int *count)
{
	static char buffer[2][80];
	static int which;

	unsigned char *p = (unsigned char *)pBase;
	char *buf = buffer[which];
	int reg = lo & 7;
	unsigned long pm;
	int temp;

	which ^= 1;
	switch ((lo >> 3) & 7)
	{
		case 0:
			sprintf (buf, "D%d", reg);
			break;
		case 1:
			sprintf (buf, "A%d", reg);
			break;
		case 2:
			sprintf (buf, "(A%d)", reg);
			break;
		case 3:
			sprintf (buf, "(A%d)+", reg);
			break;
		case 4:
			sprintf (buf, "-(A%d)", reg);
			break;
		case 5:
			PARAM_WORD (pm);
			if (pm & 0x8000)
				sprintf (buf, "(-$%X,A%d)", -(signed short)pm & 0xffff, reg);
			else
				sprintf (buf, "($%lX,A%d)", pm, reg);
			break;
		case 6:
			PARAM_WORD (pm);
			temp = pm & 0xff;
			if (temp & 0x80)
				sprintf (buf, "(-$%X,A%d,D%ld.%c)", -(signed char)temp & 0xff, reg, (pm >> 12) & 7, (pm & 800) ? 'L' : 'W');
			else
				sprintf (buf, "($%X,A%d,D%ld.%c)", temp, reg, (pm >> 12) & 7, (pm & 800) ? 'L' : 'W');
			break;
		case 7:
			switch (reg)
			{
				case 0:
					PARAM_WORD (pm);
					sprintf (buf, "$%lX", pm);
					break;
				case 1:
					PARAM_LONG (pm);
					sprintf (buf, "$%lX", pm);
					break;
				case 2:
					PARAM_WORD (pm);
					if (pm & 0x8000)
						sprintf (buf, "(-$%X,PC)", -(signed short)pm & 0xffff);
					else
						sprintf (buf, "($%lX,PC)", pm);
					break;
				case 3:
					PARAM_WORD (pm);
					temp = pm & 0xff;
					if (temp & 0x80)
						sprintf (buf, "(-$%X,PC,D%ld.%c)", -(signed char)temp & 0xff, (pm >> 12) & 7, (pm & 800) ? 'L' : 'W');
					else
						sprintf (buf, "($%X,PC,D%ld.%c)", temp, (pm >> 12) & 7, (pm & 800) ? 'L' : 'W');
					break;
				case 4:
					if (size == 1)
					{
						PARAM_WORD (pm);
						temp = pm & 0xff;
						sprintf (buf, "#$%X", temp);
					}
					else if (size == 2)
					{
						PARAM_WORD (pm);
						sprintf (buf, "#$%lX", pm);
					}
					else
					{
						PARAM_LONG (pm);
						sprintf (buf, "#$%lX", pm);
					}
					break;
			}
			break;
	}

	*count = p - ((unsigned char*)pBase);
	return buf;
}

static char *MakeRegList (char *p, unsigned short pm)
{
	int start = -1, sep = 0;
	int i;

	for (i = 0; i < 8; i++, pm >>= 1)
	{
		if ((pm & 1) && start == -1)
			start = i;
		else if (!(pm & 1) && start != -1)
		{
			if (sep++) p += sprintf (p, "/");
			if (start == i - 1) p += sprintf (p, "D%d", start);
			else p += sprintf (p, "D%d-D%d", start, i - 1);
			start = -1;
		}
	}
	if (start != -1)
	{
		if (sep++) p += sprintf (p, "/");
		if (start == 7) p += sprintf (p, "D7");
		else p += sprintf (p, "D%d-D7", start);
		start = -1;
	}

	for (i = 0; i < 8; i++, pm >>= 1)
	{
		if ((pm & 1) && start == -1)
			start = i;
		else if (!(pm & 1) && start != -1)
		{
			if (sep++) p += sprintf (p, "/");
			if (start == i - 1) p += sprintf (p, "A%d", start);
			else p += sprintf (p, "A%d-A%d", start, i - 1);
			start = -1;
		}
	}
	if (start != -1)
	{
		if (sep++) p += sprintf (p, "/");
		if (start == 7) p += sprintf (p, "A7");
		else p += sprintf (p, "A%d-A7", start);
	}

	return p;
}

static char *MakeRevRegList (char *p, unsigned short pm)
{
	int start = -1, sep = 0;
	int i;

	for (i = 0; i < 8; i++, pm <<= 1)
	{
		if ((pm & 0x8000) && start == -1)
			start = i;
		else if (!(pm & 0x8000) && start != -1)
		{
			if (sep++) p += sprintf (p, "/");
			if (start == i - 1) p += sprintf (p, "D%d", start);
			else p += sprintf (p, "D%d-D%d", start, i - 1);
			start = -1;
		}
	}
	if (start != -1)
	{
		if (sep++) p += sprintf (p, "/");
		if (start == 7) p += sprintf (p, "D7");
		else p += sprintf (p, "D%d-D7", start);
		start = -1;
	}

	for (i = 0; i < 8; i++, pm <<= 1)
	{
		if ((pm & 0x8000) && start == -1)
			start = i;
		else if (!(pm & 0x8000) && start != -1)
		{
			if (sep++) p += sprintf (p, "/");
			if (start == i - 1) p += sprintf (p, "A%d", start);
			else p += sprintf (p, "A%d-A%d", start, i - 1);
			start = -1;
		}
	}
	if (start != -1)
	{
		if (sep++) p += sprintf (p, "/");
		if (start == 7) p += sprintf (p, "A7");
		else p += sprintf (p, "A%d-A7", start);
	}

	return p;
}


int Dasm68000 (char *pBase, char *buffer, int pc)
{
	char *ea, *ea2, *p = pBase;
	unsigned short op, lo, rhi, rlo;
	unsigned long pm;
	int count;

	PARAM_WORD(op);

	lo = op & 0x3f;
	rhi = (op >> 9) & 7;
	rlo = op & 7;
	switch (op & 0xffc0)
	{
		case 0x0000:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "ORI      #$%lX,CCR", pm & 0xff);
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "ORI.B    #$%lX,%s", pm & 0xff, ea);
			}
			break;
		case 0x0040:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "ORI      #$%lX,SR", pm & 0xffff);
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "ORI.W    #$%lX,%s", pm & 0xffff, ea);
			}
			break;
		case 0x0080:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ORI.L    #$%lX,%s", pm, ea);
			break;
		case 0x0100: case 0x0300: case 0x0500: case 0x0700: case 0x0900: case 0x0b00: case 0x0d00: case 0x0f00:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD(pm);
				sprintf (buffer, "MOVEP.W  ($%lX,A%d),D%d", pm, rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "BTST     D%d,%s", rhi, ea);
			}
			break;
		case 0x0140: case 0x0340: case 0x0540: case 0x0740: case 0x0940: case 0x0b40: case 0x0d40: case 0x0f40:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD(pm);
				sprintf (buffer, "MOVEP.L  ($%lX,A%d),D%d", pm, rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "BCHG     D%d,%s", rhi, ea);
			}
			break;
		case 0x0180: case 0x0380: case 0x0580: case 0x0780: case 0x0980: case 0x0b80: case 0x0d80: case 0x0f80:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD(pm);
				sprintf (buffer, "MOVEP.W  D%d,($%lX,A%d)", rhi, pm, rlo);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "BCLR     D%d,%s", rhi, ea);
			}
			break;
		case 0x01c0: case 0x03c0: case 0x05c0: case 0x07c0: case 0x09c0: case 0x0bc0: case 0x0dc0: case 0x0fc0:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD(pm);
				sprintf (buffer, "MOVEP.L  D%d,($%lX,A%d)", rhi, pm, rlo);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "BSET     D%d,%s", rhi, ea);
			}
			break;
		case 0x0200:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "ANDI     #$%lX,CCR", pm & 0xff);
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "ANDI.B   #$%lX,%s", pm & 0xff, ea);
			}
			break;
		case 0x0240:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "ANDI     #$%lX,SR", pm & 0xffff);
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "ANDI.W   #$%lX,%s", pm & 0xffff, ea);
			}
			break;
		case 0x0280:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ANDI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0400:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "SUBI.B   #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0440:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "SUBI.W   #$%lX,%s", pm & 0xffff, ea);
			break;
		case 0x0480:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "SUBI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0600:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "ADDI.B   #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0640:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "ADDI.W   #$%lX,%s", pm & 0xffff, ea);
			break;
		case 0x0680:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ADDI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0800:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "BTST     #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0840:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "BCHG     #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0880:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "BCLR     #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x08c0:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "BSET     #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0a00:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "EORI     #$%lX,CCR", pm & 0xff);
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "EORI.B   #$%lX,%s", pm & 0xff, ea);
			}
			break;
		case 0x0a40:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "EORI     #$%lX,SR", pm & 0xffff);
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "EORI.W   #$%lX,%s", pm & 0xffff, ea);
			}
			break;
		case 0x0a80:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "EORI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0c00:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "CMPI.B   #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0c40:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CMPI.W   #$%lX,%s", pm & 0xffff, ea);
			break;
		case 0x0c80:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "CMPI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0e00:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 1, &count); p += count;
			if (pm & 0x0800)
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.B  A%ld,%s", (pm >> 12) & 7, ea);
				else
					sprintf (buffer, "MOVES.B  D%ld,%s", (pm >> 12) & 7, ea);
			}
			else
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.B  %s,A%ld", ea, (pm >> 12) & 7);
				else
					sprintf (buffer, "MOVES.B  %s,D%ld", ea, (pm >> 12) & 7);
			}
			break;
		case 0x0e40:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			if (pm & 0x0800)
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.W  A%ld,%s", (pm >> 12) & 7, ea);
				else
					sprintf (buffer, "MOVES.W  D%ld,%s", (pm >> 12) & 7, ea);
			}
			else
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.W  %s,A%ld", ea, (pm >> 12) & 7);
				else
					sprintf (buffer, "MOVES.W  %s,D%ld", ea, (pm >> 12) & 7);
			}
			break;
		case 0x0e80:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			if (pm & 0x0800)
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.L  A%ld,%s", (pm >> 12) & 7, ea);
				else
					sprintf (buffer, "MOVES.L  D%ld,%s", (pm >> 12) & 7, ea);
			}
			else
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.L  %s,A%ld", ea, (pm >> 12) & 7);
				else
					sprintf (buffer, "MOVES.L  %s,D%ld", ea, (pm >> 12) & 7);
			}
			break;
		case 0x1000: case 0x1080: case 0x10c0: case 0x1100: case 0x1140: case 0x1180: case 0x11c0:
		case 0x1200: case 0x1280: case 0x12c0: case 0x1300: case 0x1340: case 0x1380: case 0x13c0:
		case 0x1400: case 0x1480: case 0x14c0: case 0x1500: case 0x1540: case 0x1580:
		case 0x1600: case 0x1680: case 0x16c0: case 0x1700: case 0x1740: case 0x1780:
		case 0x1800: case 0x1880: case 0x18c0: case 0x1900: case 0x1940: case 0x1980:
		case 0x1a00: case 0x1a80: case 0x1ac0: case 0x1b00: case 0x1b40: case 0x1b80:
		case 0x1c00: case 0x1c80: case 0x1cc0: case 0x1d00: case 0x1d40: case 0x1d80:
		case 0x1e00: case 0x1e80: case 0x1ec0: case 0x1f00: case 0x1f40: case 0x1f80:
			ea = MakeEA (lo, p, 1, &count); p += count; ea2 = MakeEA (((op >> 9) & 0x07) + ((op >> 3) & 0x38), p, 1, &count); p += count;
			sprintf (buffer, "MOVE.B   %s,%s", ea, ea2);
			break;
		case 0x2000: case 0x2080: case 0x20c0: case 0x2100: case 0x2140: case 0x2180: case 0x21c0:
		case 0x2200: case 0x2280: case 0x22c0: case 0x2300: case 0x2340: case 0x2380: case 0x23c0:
		case 0x2400: case 0x2480: case 0x24c0: case 0x2500: case 0x2540: case 0x2580:
		case 0x2600: case 0x2680: case 0x26c0: case 0x2700: case 0x2740: case 0x2780:
		case 0x2800: case 0x2880: case 0x28c0: case 0x2900: case 0x2940: case 0x2980:
		case 0x2a00: case 0x2a80: case 0x2ac0: case 0x2b00: case 0x2b40: case 0x2b80:
		case 0x2c00: case 0x2c80: case 0x2cc0: case 0x2d00: case 0x2d40: case 0x2d80:
		case 0x2e00: case 0x2e80: case 0x2ec0: case 0x2f00: case 0x2f40: case 0x2f80:
			ea = MakeEA (lo, p, 4, &count); p += count; ea2 = MakeEA (((op >> 9) & 0x07) + ((op >> 3) & 0x38), p, 4, &count); p += count;
			sprintf (buffer, "MOVE.L   %s,%s", ea, ea2);
			break;
		case 0x2040: case 0x2240: case 0x2440: case 0x2640: case 0x2840: case 0x2a40: case 0x2c40: case 0x2e40:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "MOVEA.L  %s,A%d", ea, rhi);
			break;
		case 0x3000: case 0x3080: case 0x30c0: case 0x3100: case 0x3140: case 0x3180: case 0x31c0:
		case 0x3200: case 0x3280: case 0x32c0: case 0x3300: case 0x3340: case 0x3380: case 0x33c0:
		case 0x3400: case 0x3480: case 0x34c0: case 0x3500: case 0x3540: case 0x3580:
		case 0x3600: case 0x3680: case 0x36c0: case 0x3700: case 0x3740: case 0x3780:
		case 0x3800: case 0x3880: case 0x38c0: case 0x3900: case 0x3940: case 0x3980:
		case 0x3a00: case 0x3a80: case 0x3ac0: case 0x3b00: case 0x3b40: case 0x3b80:
		case 0x3c00: case 0x3c80: case 0x3cc0: case 0x3d00: case 0x3d40: case 0x3d80:
		case 0x3e00: case 0x3e80: case 0x3ec0: case 0x3f00: case 0x3f40: case 0x3f80:
			ea = MakeEA (lo, p, 2, &count); p += count; ea2 = MakeEA (((op >> 9) & 0x07) + ((op >> 3) & 0x38), p, 2, &count); p += count;
			sprintf (buffer, "MOVE.W   %s,%s", ea, ea2);
			break;
		case 0x3040: case 0x3240: case 0x3440: case 0x3640: case 0x3840: case 0x3a40: case 0x3c40: case 0x3e40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVEA.W  %s,A%d", ea, rhi);
			break;
		case 0x4000:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "NEGX.B   %s", ea);
			break;
		case 0x4040:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "NEGX.W   %s", ea);
			break;
		case 0x4080:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "NEGX.L   %s", ea);
			break;
		case 0x40c0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVE     SR,%s", ea);
			break;
		case 0x4180: case 0x4380: case 0x4580: case 0x4780: case 0x4980: case 0x4b80: case 0x4d80: case 0x4f80:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CHK.W    %s,D%d", ea, rhi);
			break;
		case 0x41c0: case 0x43c0: case 0x45c0: case 0x47c0: case 0x49c0: case 0x4bc0: case 0x4dc0: case 0x4fc0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "LEA      %s,A%d", ea, rhi);
			break;
		case 0x4200:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "CLR.B    %s", ea);
			break;
		case 0x4240:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CLR.W    %s", ea);
			break;
		case 0x4280:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "CLR.L    %s", ea);
			break;
		case 0x42c0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVE     CCR,%s", ea);
			break;
		case 0x4400:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "NEG.B    %s", ea);
			break;
		case 0x4440:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "NEG.W    %s", ea);
			break;
		case 0x4480:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "NEG.L    %s", ea);
			break;
		case 0x44c0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVE     %s,CCR", ea);
			break;
		case 0x4600:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "NOT.B    %s", ea);
			break;
		case 0x4640:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "NOT.W    %s", ea);
			break;
		case 0x4680:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "NOT.L    %s", ea);
			break;
		case 0x46c0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVE     %s,SR", ea);
			break;
		case 0x4800:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "NBCD.B   %s", ea);
			break;
		case 0x4840:
			if ((lo & 0x38) == 0x00)
				sprintf (buffer, "SWAP     D%d", rlo);
			else
			{
				ea = MakeEA (lo, p, 4, &count); p += count;
				sprintf (buffer, "PEA      %s", ea);
			}
			break;
		case 0x4880:
			if ((lo & 0x38) == 0x00)
				sprintf (buffer, "EXT.W    D%d", rlo);
			else
			{
				char *b = buffer;
				PARAM_WORD (pm);	ea = MakeEA (lo, p, 2, &count); p += count;
				b += sprintf (b, "MOVEM.W  ");
				if ((lo & 0x38) != 0x20) b = MakeRegList (b, pm);
				else b = MakeRevRegList (b, pm);
				sprintf (b, ",%s", ea);
			}
			break;
		case 0x48c0:
			if ((lo & 0x38) == 0x00)
				sprintf (buffer, "EXT.L    D%d", rlo);
			else
			{
				char *b = buffer;
				PARAM_WORD (pm);	ea = MakeEA (lo, p, 4, &count); p += count;
				b += sprintf (b, "MOVEM.L  ");
				if ((lo & 0x38) != 0x20) b = MakeRegList (b, pm);
				else b = MakeRevRegList (b, pm);
				sprintf (b, ",%s", ea);
			}
			break;
		case 0x4a00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "TST.B    %s", ea);
			break;
		case 0x4a40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "TST.W    %s", ea);
			break;
		case 0x4a80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "TST.L    %s", ea);
			break;
		case 0x4ac0:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "TAS.B    %s", ea);
			break;
		case 0x4c80:
			{
				char *b = buffer;
				PARAM_WORD (pm);	ea = MakeEA (lo, p, 2, &count); p += count;
				b += sprintf (b, "MOVEM.W  %s,", ea);
				b = MakeRegList (b, pm);
			}
			break;
		case 0x4cc0:
			{
				char *b = buffer;
				PARAM_WORD (pm);	ea = MakeEA (lo, p, 4, &count); p += count;
				b += sprintf (b, "MOVEM.L  %s,", ea);
				b = MakeRegList (b, pm);
			}
			break;
		case 0x4e40:
			if ((lo & 30) == 0x00)
				sprintf (buffer, "TRAP     #$%X", lo & 15);
			else if ((lo & 0x38) == 0x10)
			{
				PARAM_WORD (pm);
				sprintf (buffer, "LINK     A%d,#$%lX", rlo, pm);
			}
			else if ((lo & 0x38) == 0x18)
			{
				sprintf (buffer, "UNLK     A%d", rlo);
			}
			else if ((lo & 0x38) == 0x20)
				sprintf (buffer, "MOVE     A%d,USP", rlo);
			else if ((lo & 0x38) == 0x28)
				sprintf (buffer, "MOVE     USP,A%d", rlo);
			else if (lo == 0x30)
				sprintf (buffer, "RESET");
			else if (lo == 0x31)
				sprintf (buffer, "NOP");
			else if (lo == 0x32)
				sprintf (buffer, "STOP");
			else if (lo == 0x33)
				sprintf (buffer, "RTE");
			else if (lo == 0x35)
				sprintf (buffer, "RTS");
			else if (lo == 0x36)
				sprintf (buffer, "TRAPV");
			else if (lo == 0x37)
				sprintf (buffer, "RTR");
			else if (lo == 0x3a)
			{
				PARAM_WORD (pm);
				switch (pm & 0xfff)
				{
					case 0x000:	ea = "SFC";	break;
					case 0x001:	ea = "DFC"; break;
					case 0x800: ea = "USP"; break;
					case 0x801: ea = "VBR"; break;
					default: ea = "???"; break;
				}
				if (pm & 0x8000)
					sprintf (buffer, "MOVEC    %s,A%ld", ea, (pm >> 12) & 7);
				else
					sprintf (buffer, "MOVEC    %s,D%ld", ea, (pm >> 12) & 7);
			}
			else if (lo == 0x3b)
			{
				PARAM_WORD (pm);
				switch (pm & 0xfff)
				{
					case 0x000:	ea = "SFC";	break;
					case 0x001:	ea = "DFC"; break;
					case 0x800: ea = "USP"; break;
					case 0x801: ea = "VBR"; break;
					default: ea = "???"; break;
				}
				if (pm & 0x8000)
					sprintf (buffer, "MOVEC    A%ld,%s", (pm >> 12) & 7, ea);
				else
					sprintf (buffer, "MOVEC    D%ld,%s", (pm >> 12) & 7, ea);
			}
			else
				sprintf (buffer, "DC.W     $%X", op);
			break;
		case 0x4e80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "JSR      %s", ea);
			break;
		case 0x4ec0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "JMP      %s", ea);
			break;
		case 0x5000: case 0x5200: case 0x5400: case 0x5600: case 0x5800: case 0x5a00: case 0x5c00: case 0x5e00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "ADDQ.B   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x5040: case 0x5240: case 0x5440: case 0x5640: case 0x5840: case 0x5a40: case 0x5c40: case 0x5e40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "ADDQ.W   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x5080: case 0x5280: case 0x5480: case 0x5680: case 0x5880: case 0x5a80: case 0x5c80: case 0x5e80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ADDQ.L   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x50c0: case 0x52c0: case 0x54c0: case 0x56c0: case 0x58c0: case 0x5ac0: case 0x5cc0: case 0x5ec0:
		case 0x51c0: case 0x53c0: case 0x55c0: case 0x57c0: case 0x59c0: case 0x5bc0: case 0x5dc0: case 0x5fc0:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD (pm);
				if (pm & 0x8000)
					sprintf (buffer, "DB%s     D%d,*-$%X [%X]", ccodes[(op >> 8) & 15], rlo, (int)(-(signed short)pm) - 2, pc + (signed short)pm + 2);
				else
					sprintf (buffer, "DB%s     D%d,*+$%lX [%lX]", ccodes[(op >> 8) & 15], rlo, pm - 2, pc + pm + 2);
			}
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "S%s.B    %s", ccodes[(op >> 8) & 15], ea);
			}
			break;
		case 0x5100: case 0x5300: case 0x5500: case 0x5700: case 0x5900: case 0x5b00: case 0x5d00: case 0x5f00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "SUBQ.B   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x5140: case 0x5340: case 0x5540: case 0x5740: case 0x5940: case 0x5b40: case 0x5d40: case 0x5f40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "SUBQ.W   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x5180: case 0x5380: case 0x5580: case 0x5780: case 0x5980: case 0x5b80: case 0x5d80: case 0x5f80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "SUBQ.L   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x6000: case 0x6040: case 0x6080: case 0x60c0:
			pm = op & 0xff;
			if (pm == 0)
			{
				PARAM_WORD(pm);
				if (pm & 0x8000)
					sprintf (buffer, "BRA      *-$%X [%X]", (int)(-(signed short)pm) - 2, pc + (signed short)pm + 2);
				else
					sprintf (buffer, "BRA      *+$%lX [%lX]", pm + 2, pc + pm + 2);
			}
			else
			{
				if (pm & 0x80)
					sprintf (buffer, "BRA.S    *-$%X [%X]", (int)(-(signed char)pm) - 2, pc + (signed char)pm + 2);
				else
					sprintf (buffer, "BRA.S    *+$%lX [%lX]", pm + 2, pc + pm + 2);
			}
			break;
		case 0x6100: case 0x6140: case 0x6180: case 0x61c0:
			pm = op & 0xff;
			if (pm == 0)
			{
				PARAM_WORD(pm);
				if (pm & 0x8000)
					sprintf (buffer, "BSR      *-$%X [%X]", (int)(-(signed short)pm) - 2, pc + (signed short)pm + 2);
				else
					sprintf (buffer, "BSR      *+$%lX [%lX]", pm + 2, pc + pm + 2);
			}
			else
			{
				if (pm & 0x80)
					sprintf (buffer, "BSR.S    *-$%X [%X]", (int)(-(signed char)pm) - 2, pc + (signed char)pm + 2);
				else
					sprintf (buffer, "BSR.S    *+$%lX [%lX]", pm + 2, pc + pm + 2);
			}
			break;
		case 0x6200: case 0x6240: case 0x6280: case 0x62c0: case 0x6300: case 0x6340: case 0x6380: case 0x63c0:
		case 0x6400: case 0x6440: case 0x6480: case 0x64c0: case 0x6500: case 0x6540: case 0x6580: case 0x65c0:
		case 0x6600: case 0x6640: case 0x6680: case 0x66c0: case 0x6700: case 0x6740: case 0x6780: case 0x67c0:
		case 0x6800: case 0x6840: case 0x6880: case 0x68c0: case 0x6900: case 0x6940: case 0x6980: case 0x69c0:
		case 0x6a00: case 0x6a40: case 0x6a80: case 0x6ac0: case 0x6b00: case 0x6b40: case 0x6b80: case 0x6bc0:
		case 0x6c00: case 0x6c40: case 0x6c80: case 0x6cc0: case 0x6d00: case 0x6d40: case 0x6d80: case 0x6dc0:
		case 0x6e00: case 0x6e40: case 0x6e80: case 0x6ec0: case 0x6f00: case 0x6f40: case 0x6f80: case 0x6fc0:
			pm = op & 0xff;
			if (pm == 0)
			{
				PARAM_WORD(pm);
				if (pm & 0x8000)
					sprintf (buffer, "B%s      *-$%X [%X]", ccodes[(op >> 8) & 15], (int)(-(signed short)pm) - 2, pc + (signed short)pm + 2);
				else
					sprintf (buffer, "B%s      *+$%lX [%lX]", ccodes[(op >> 8) & 15], pm + 2, pc + pm + 2);
			}
			else
			{
				if (pm & 0x80)
					sprintf (buffer, "B%s.S    *-$%X [%X]", ccodes[(op >> 8) & 15], (int)(-(signed char)pm) - 2, pc + (signed char)pm + 2);
				else
					sprintf (buffer, "B%s.S    *+$%lX [%lX]", ccodes[(op >> 8) & 15], pm + 2, pc + pm + 2);
			}
			break;
		case 0x7000: case 0x7040: case 0x7080: case 0x70c0:
		case 0x7200: case 0x7240: case 0x7280: case 0x72c0:
		case 0x7400: case 0x7440: case 0x7480: case 0x74c0:
		case 0x7600: case 0x7640: case 0x7680: case 0x76c0:
		case 0x7800: case 0x7840: case 0x7880: case 0x78c0:
		case 0x7a00: case 0x7a40: case 0x7a80: case 0x7ac0:
		case 0x7c00: case 0x7c40: case 0x7c80: case 0x7cc0:
		case 0x7e00: case 0x7e40: case 0x7e80: case 0x7ec0:
			pm = op & 0xff;
			if (pm & 0x80)
				sprintf (buffer, "MOVEQ    #$-%X,D%d", -(signed char)pm, rhi);
			else
				sprintf (buffer, "MOVEQ    #$%lX,D%d", pm, rhi);
			break;
		case 0x8000: case 0x8200: case 0x8400: case 0x8600: case 0x8800: case 0x8a00: case 0x8c00: case 0x8e00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "OR.B     %s,D%d", ea, rhi);
			break;
		case 0x8040: case 0x8240: case 0x8440: case 0x8640: case 0x8840: case 0x8a40: case 0x8c40: case 0x8e40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "OR.W     %s,D%d", ea, rhi);
			break;
		case 0x8080: case 0x8280: case 0x8480: case 0x8680: case 0x8880: case 0x8a80: case 0x8c80: case 0x8e80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "OR.L     %s,D%d", ea, rhi);
			break;
		case 0x80c0: case 0x82c0: case 0x84c0: case 0x86c0: case 0x88c0: case 0x8ac0: case 0x8cc0: case 0x8ec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "DIVU.W   %s,D%d", ea, rhi);
			break;
		case 0x8100: case 0x8300: case 0x8500: case 0x8700: case 0x8900: case 0x8b00: case 0x8d00: case 0x8f00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "OR.B     D%d,%s", rhi, ea);
			break;
		case 0x8140: case 0x8340: case 0x8540: case 0x8740: case 0x8940: case 0x8b40: case 0x8d40: case 0x8f40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "OR.W     D%d,%s", rhi, ea);
			break;
		case 0x8180: case 0x8380: case 0x8580: case 0x8780: case 0x8980: case 0x8b80: case 0x8d80: case 0x8f80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "OR.L     D%d,%s", rhi, ea);
			break;
		case 0x81c0: case 0x83c0: case 0x85c0: case 0x87c0: case 0x89c0: case 0x8bc0: case 0x8dc0: case 0x8fc0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "DIVS.W   %s,D%d", ea, rhi);
			break;
		case 0x9000: case 0x9200: case 0x9400: case 0x9600: case 0x9800: case 0x9a00: case 0x9c00: case 0x9e00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "SUB.B    %s,D%d", ea, rhi);
			break;
		case 0x9040: case 0x9240: case 0x9440: case 0x9640: case 0x9840: case 0x9a40: case 0x9c40: case 0x9e40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "SUB.W    %s,D%d", ea, rhi);
			break;
		case 0x9080: case 0x9280: case 0x9480: case 0x9680: case 0x9880: case 0x9a80: case 0x9c80: case 0x9e80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "SUB.L    %s,D%d", ea, rhi);
			break;
		case 0x90c0: case 0x92c0: case 0x94c0: case 0x96c0: case 0x98c0: case 0x9ac0: case 0x9cc0: case 0x9ec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "SUBA.W   %s,A%d", ea, rhi);
			break;
		case 0x9100: case 0x9300: case 0x9500: case 0x9700: case 0x9900: case 0x9b00: case 0x9d00: case 0x9f00:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "SUBX.B   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "SUBX.B   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 1,&count); p += count;
				sprintf (buffer, "SUB.B    D%d,%s", rhi, ea);
			}
			break;
		case 0x9140: case 0x9340: case 0x9540: case 0x9740: case 0x9940: case 0x9b40: case 0x9d40: case 0x9f40:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "SUBX.W   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "SUBX.W   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "SUB.W    D%d,%s", rhi, ea);
			}
			break;
		case 0x9180: case 0x9380: case 0x9580: case 0x9780: case 0x9980: case 0x9b80: case 0x9d80: case 0x9f80:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "SUBX.L   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "SUBX.L   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 4, &count); p += count;
				sprintf (buffer, "SUB.L    D%d,%s", rhi, ea);
			}
			break;
		case 0x91c0: case 0x93c0: case 0x95c0: case 0x97c0: case 0x99c0: case 0x9bc0: case 0x9dc0: case 0x9fc0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "SUBA.L   %s,A%d", ea, rhi);
			break;
		case 0xb000: case 0xb200: case 0xb400: case 0xb600: case 0xb800: case 0xba00: case 0xbc00: case 0xbe00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "CMP.B    %s,D%d", ea, rhi);
			break;
		case 0xb040: case 0xb240: case 0xb440: case 0xb640: case 0xb840: case 0xba40: case 0xbc40: case 0xbe40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CMP.W    %s,D%d", ea, rhi);
			break;
		case 0xb080: case 0xb280: case 0xb480: case 0xb680: case 0xb880: case 0xba80: case 0xbc80: case 0xbe80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "CMP.L    %s,D%d", ea, rhi);
			break;
		case 0xb0c0: case 0xb2c0: case 0xb4c0: case 0xb6c0: case 0xb8c0: case 0xbac0: case 0xbcc0: case 0xbec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CMPA.W   %s,A%d", ea, rhi);
			break;
		case 0xb100: case 0xb300: case 0xb500: case 0xb700: case 0xb900: case 0xbb00: case 0xbd00: case 0xbf00:
			if ((lo & 0x38) == 0x08)
				sprintf (buffer, "CMPM.B   (A%d)+,(A%d)+", rlo, rhi);
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "EOR.B    D%d,%s", rhi, ea);
			}
			break;
		case 0xb140: case 0xb340: case 0xb540: case 0xb740: case 0xb940: case 0xbb40: case 0xbd40: case 0xbf40:
			if ((lo & 0x38) == 0x08)
				sprintf (buffer, "CMPM.W   (A%d)+,(A%d)+", rlo, rhi);
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "EOR.W    D%d,%s", rhi, ea);
			}
			break;
		case 0xb180: case 0xb380: case 0xb580: case 0xb780: case 0xb980: case 0xbb80: case 0xbd80: case 0xbf80:
			if ((lo & 0x38) == 0x08)
				sprintf (buffer, "CMPM.L   (A%d)+,(A%d)+", rlo, rhi);
			else
			{
				ea = MakeEA (lo, p, 4, &count); p += count;
				sprintf (buffer, "EOR.L    D%d,%s", rhi, ea);
			}
			break;
		case 0xb1c0: case 0xb3c0: case 0xb5c0: case 0xb7c0: case 0xb9c0: case 0xbbc0: case 0xbdc0: case 0xbfc0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "CMPA.L   %s,A%d", ea, rhi);
			break;
		case 0xc000: case 0xc200: case 0xc400: case 0xc600: case 0xc800: case 0xca00: case 0xcc00: case 0xce00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "AND.B    %s,D%d", ea, rhi);
			break;
		case 0xc040: case 0xc240: case 0xc440: case 0xc640: case 0xc840: case 0xca40: case 0xcc40: case 0xce40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "AND.W    %s,D%d", ea, rhi);
			break;
		case 0xc080: case 0xc280: case 0xc480: case 0xc680: case 0xc880: case 0xca80: case 0xcc80: case 0xce80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "AND.L    %s,D%d", ea, rhi);
			break;
		case 0xc0c0: case 0xc2c0: case 0xc4c0: case 0xc6c0: case 0xc8c0: case 0xcac0: case 0xccc0: case 0xcec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MULU.W   %s,D%d", ea, rhi);
			break;
		case 0xc100: case 0xc300: case 0xc500: case 0xc700: case 0xc900: case 0xcb00: case 0xcd00: case 0xcf00:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "ABCD.B   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "ABCD.B   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "AND.B    D%d,%s", rhi, ea);
			}
			break;
		case 0xc140: case 0xc340: case 0xc540: case 0xc740: case 0xc940: case 0xcb40: case 0xcd40: case 0xcf40:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "EXG      A%d,A%d", rhi, rlo);
				else
					sprintf (buffer, "EXG      D%d,D%d", rhi, rlo);
			}
			else
			{
				ea = MakeEA (lo, p, 2,&count); p += count;
				sprintf (buffer, "AND.W    D%d,%s", rhi, ea);
			}
			break;
		case 0xc180: case 0xc380: case 0xc580: case 0xc780: case 0xc980: case 0xcb80: case 0xcd80: case 0xcf80:
			if ((lo & 0x38) == 0x08)
				sprintf (buffer, "EXG      D%d,A%d", rhi, rlo);
			else
			{
				ea = MakeEA (lo, p, 4, &count); p += count;
				sprintf (buffer, "AND.L    D%d,%s", rhi, ea);
			}
			break;
		case 0xc1c0: case 0xc3c0: case 0xc5c0: case 0xc7c0: case 0xc9c0: case 0xcbc0: case 0xcdc0: case 0xcfc0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MULS.W   %s,D%d", ea, rhi);
			break;
		case 0xd000: case 0xd200: case 0xd400: case 0xd600: case 0xd800: case 0xda00: case 0xdc00: case 0xde00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "ADD.B    %s,D%d", ea, rhi);
			break;
		case 0xd040: case 0xd240: case 0xd440: case 0xd640: case 0xd840: case 0xda40: case 0xdc40: case 0xde40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "ADD.W    %s,D%d", ea, rhi);
			break;
		case 0xd080: case 0xd280: case 0xd480: case 0xd680: case 0xd880: case 0xda80: case 0xdc80: case 0xde80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ADD.L    %s,D%d", ea, rhi);
			break;
		case 0xd0c0: case 0xd2c0: case 0xd4c0: case 0xd6c0: case 0xd8c0: case 0xdac0: case 0xdcc0: case 0xdec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "ADDA.W   %s,A%d", ea, rhi);
			break;
		case 0xd100: case 0xd300: case 0xd500: case 0xd700: case 0xd900: case 0xdb00: case 0xdd00: case 0xdf00:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "ADDX.B   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "ADDX.B   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "ADD.B    D%d,%s", rhi, ea);
			}
			break;
		case 0xd140: case 0xd340: case 0xd540: case 0xd740: case 0xd940: case 0xdb40: case 0xdd40: case 0xdf40:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "ADDX.W   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "ADDX.W   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "ADD.W    D%d,%s", rhi, ea);
			}
			break;
		case 0xd180: case 0xd380: case 0xd580: case 0xd780: case 0xd980: case 0xdb80: case 0xdd80: case 0xdf80:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "ADDX.L   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "ADDX.L   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 4,&count); p += count;
				sprintf (buffer, "ADD.L    D%d,%s", rhi, ea);
			}
			break;
		case 0xd1c0: case 0xd3c0: case 0xd5c0: case 0xd7c0: case 0xd9c0: case 0xdbc0: case 0xddc0: case 0xdfc0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ADDA.L   %s,A%d", ea, rhi);
			break;
		case 0xe000: case 0xe200: case 0xe400: case 0xe600: case 0xe800: case 0xea00: case 0xec00: case 0xee00:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASR.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSR.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXR.B   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROR.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASR.B    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSR.B    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXR.B   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROR.B    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe040: case 0xe240: case 0xe440: case 0xe640: case 0xe840: case 0xea40: case 0xec40: case 0xee40:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASR.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSR.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXR.W   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROR.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASR.W    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSR.W    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXR.W   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROR.W    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe080: case 0xe280: case 0xe480: case 0xe680: case 0xe880: case 0xea80: case 0xec80: case 0xee80:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASR.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSR.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXR.L   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROR.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASR.L    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSR.L    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXR.L   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROR.L    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe0c0: case 0xe2c0: case 0xe4c0: case 0xe6c0:
		case 0xe1c0: case 0xe3c0: case 0xe5c0: case 0xe7c0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			switch ((op >> 8) & 7)
			{
				case 0:	sprintf (buffer, "ASR.L    #1,%s", ea);		break;
				case 1:	sprintf (buffer, "ASL.L    #1,%s", ea);		break;
				case 2:	sprintf (buffer, "LSR.L    #1,%s", ea);		break;
				case 3:	sprintf (buffer, "LSL.L    #1,%s", ea);		break;
				case 4:	sprintf (buffer, "ROXR.L   #1,%s", ea);		break;
				case 5:	sprintf (buffer, "ROXL.L   #1,%s", ea);		break;
				case 6:	sprintf (buffer, "ROR.L    #1,%s", ea);		break;
				case 7:	sprintf (buffer, "ROL.L    #1,%s", ea);		break;
			}
			break;
		case 0xe100: case 0xe300: case 0xe500: case 0xe700: case 0xe900: case 0xeb00: case 0xed00: case 0xef00:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASL.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSL.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXL.B   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROL.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASL.B    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSL.B    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXL.B   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROL.B    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe140: case 0xe340: case 0xe540: case 0xe740: case 0xe940: case 0xeb40: case 0xed40: case 0xef40:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASL.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSL.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXL.W   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROL.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASL.W    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSL.W    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXL.W   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROL.W    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe180: case 0xe380: case 0xe580: case 0xe780: case 0xe980: case 0xeb80: case 0xed80: case 0xef80:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASL.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSL.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXL.L   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROL.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASL.L    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSL.L    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXL.L   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROL.L    D%d,D%d", rhi, rlo);		break;
			}
			break;
		default:
			sprintf (buffer, "DC.W     $%X", op);
			break;
	}

	return p - pBase;
}

#else

static char *MakeEA (int lo, char *pBase, int size, int *count)
{
	static char buffer[2][80];
	static int which;

	unsigned char *p = (unsigned char *)pBase;
	char *buf = buffer[which];
	int reg = lo & 7;
	unsigned long pm;
	int temp;

	which ^= 1;
	switch ((lo >> 3) & 7)
	{
		case 0:
			sprintf (buf, "D%d", reg);
			break;
		case 1:
			sprintf (buf, "A%d", reg);
			break;
		case 2:
			sprintf (buf, "(A%d)", reg);
			break;
		case 3:
			sprintf (buf, "(A%d)+", reg);
			break;
		case 4:
			sprintf (buf, "-(A%d)", reg);
			break;
		case 5:
			PARAM_WORD (pm);
			if (pm & 0x8000)
				sprintf (buf, "(-$%X,A%d)", -(signed short)pm & 0xffff, reg);
			else
				sprintf (buf, "($%lX,A%d)", pm, reg);
			break;
		case 6:
			PARAM_WORD (pm);
			temp = pm & 0xff;
			if (temp & 0x80)
				sprintf (buf, "(-$%X,A%d,D%ld.%c)", -(signed char)temp & 0xff, reg, (pm >> 12) & 7, (pm & 800) ? 'L' : 'W');
			else
				sprintf (buf, "($%X,A%d,D%ld.%c)", temp, reg, (pm >> 12) & 7, (pm & 800) ? 'L' : 'W');
			break;
		case 7:
			switch (reg)
			{
				case 0:
					PARAM_WORD (pm);
					sprintf (buf, "$%lX", pm);
					break;
				case 1:
					PARAM_LONG (pm);
					sprintf (buf, "$%lX", pm);
					break;
				case 2:
					PARAM_WORD (pm);
					if (pm & 0x8000)
						sprintf (buf, "(-$%X,PC)", -(signed short)pm & 0xffff);
					else
						sprintf (buf, "($%lX,PC)", pm);
					break;
				case 3:
					PARAM_WORD (pm);
					temp = pm & 0xff;
					if (temp & 0x80)
						sprintf (buf, "(-$%X,PC,D%ld.%c)", -(signed char)temp & 0xff, (pm >> 12) & 7, (pm & 800) ? 'L' : 'W');
					else
						sprintf (buf, "($%X,PC,D%ld.%c)", temp, (pm >> 12) & 7, (pm & 800) ? 'L' : 'W');
					break;
				case 4:
					if (size == 1)
					{
						PARAM_WORD (pm);
						temp = pm & 0xff;
						sprintf (buf, "#$%X", temp);
					}
					else if (size == 2)
					{
						PARAM_WORD (pm);
						sprintf (buf, "#$%lX", pm);
					}
					else
					{
						PARAM_LONG (pm);
						sprintf (buf, "#$%lX", pm);
					}
					break;
			}
			break;
	}

	*count = p - ((unsigned char*)pBase);
	return buf;
}

static char *MakeRegList (char *p, unsigned short pm)
{
	int start = -1, sep = 0;
	int i;

	for (i = 0; i < 8; i++, pm >>= 1)
	{
		if ((pm & 1) && start == -1)
			start = i;
		else if (!(pm & 1) && start != -1)
		{
			if (sep++) p += sprintf (p, "/");
			if (start == i - 1) p += sprintf (p, "D%d", start);
			else p += sprintf (p, "D%d-D%d", start, i - 1);
			start = -1;
		}
	}
	if (start != -1)
	{
		if (sep++) p += sprintf (p, "/");
		if (start == 7) p += sprintf (p, "D7");
		else p += sprintf (p, "D%d-D7", start);
		start = -1;
	}

	for (i = 0; i < 8; i++, pm >>= 1)
	{
		if ((pm & 1) && start == -1)
			start = i;
		else if (!(pm & 1) && start != -1)
		{
			if (sep++) p += sprintf (p, "/");
			if (start == i - 1) p += sprintf (p, "A%d", start);
			else p += sprintf (p, "A%d-A%d", start, i - 1);
			start = -1;
		}
	}
	if (start != -1)
	{
		if (sep++) p += sprintf (p, "/");
		if (start == 7) p += sprintf (p, "A7");
		else p += sprintf (p, "A%d-A7", start);
	}

	return p;
}

static char *MakeRevRegList (char *p, unsigned short pm)
{
	int start = -1, sep = 0;
	int i;

	for (i = 0; i < 8; i++, pm <<= 1)
	{
		if ((pm & 0x8000) && start == -1)
			start = i;
		else if (!(pm & 0x8000) && start != -1)
		{
			if (sep++) p += sprintf (p, "/");
			if (start == i - 1) p += sprintf (p, "D%d", start);
			else p += sprintf (p, "D%d-D%d", start, i - 1);
			start = -1;
		}
	}
	if (start != -1)
	{
		if (sep++) p += sprintf (p, "/");
		if (start == 7) p += sprintf (p, "D7");
		else p += sprintf (p, "D%d-D7", start);
		start = -1;
	}

	for (i = 0; i < 8; i++, pm <<= 1)
	{
		if ((pm & 0x8000) && start == -1)
			start = i;
		else if (!(pm & 0x8000) && start != -1)
		{
			if (sep++) p += sprintf (p, "/");
			if (start == i - 1) p += sprintf (p, "A%d", start);
			else p += sprintf (p, "A%d-A%d", start, i - 1);
			start = -1;
		}
	}
	if (start != -1)
	{
		if (sep++) p += sprintf (p, "/");
		if (start == 7) p += sprintf (p, "A7");
		else p += sprintf (p, "A%d-A7", start);
	}

	return p;
}


int Dasm68000 (char *pBase, char *buffer, int pc)
{
	char *ea, *ea2, *p = pBase;
	unsigned short op, lo, rhi, rlo;
	unsigned long pm;
	int count;

	PARAM_WORD(op);

	lo = op & 0x3f;
	rhi = (op >> 9) & 7;
	rlo = op & 7;
	switch (op & 0xffc0)
	{
		case 0x0000:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "ORI      #$%lX,CCR", pm & 0xff);
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "ORI.B    #$%lX,%s", pm & 0xff, ea);
			}
			break;
		case 0x0040:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "ORI      #$%lX,SR", pm & 0xffff);
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "ORI.W    #$%lX,%s", pm & 0xffff, ea);
			}
			break;
		case 0x0080:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ORI.L    #$%lX,%s", pm, ea);
			break;
		case 0x0100: case 0x0300: case 0x0500: case 0x0700: case 0x0900: case 0x0b00: case 0x0d00: case 0x0f00:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD(pm);
				sprintf (buffer, "MOVEP.W  ($%lX,A%d),D%d", pm, rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "BTST     D%d,%s", rhi, ea);
			}
			break;
		case 0x0140: case 0x0340: case 0x0540: case 0x0740: case 0x0940: case 0x0b40: case 0x0d40: case 0x0f40:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD(pm);
				sprintf (buffer, "MOVEP.L  ($%lX,A%d),D%d", pm, rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "BCHG     D%d,%s", rhi, ea);
			}
			break;
		case 0x0180: case 0x0380: case 0x0580: case 0x0780: case 0x0980: case 0x0b80: case 0x0d80: case 0x0f80:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD(pm);
				sprintf (buffer, "MOVEP.W  D%d,($%lX,A%d)", rhi, pm, rlo);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "BCLR     D%d,%s", rhi, ea);
			}
			break;
		case 0x01c0: case 0x03c0: case 0x05c0: case 0x07c0: case 0x09c0: case 0x0bc0: case 0x0dc0: case 0x0fc0:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD(pm);
				sprintf (buffer, "MOVEP.L  D%d,($%lX,A%d)", rhi, pm, rlo);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "BSET     D%d,%s", rhi, ea);
			}
			break;
		case 0x0200:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "ANDI     #$%lX,CCR", pm & 0xff);
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "ANDI.B   #$%lX,%s", pm & 0xff, ea);
			}
			break;
		case 0x0240:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "ANDI     #$%lX,SR", pm & 0xffff);
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "ANDI.W   #$%lX,%s", pm & 0xffff, ea);
			}
			break;
		case 0x0280:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ANDI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0400:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "SUBI.B   #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0440:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "SUBI.W   #$%lX,%s", pm & 0xffff, ea);
			break;
		case 0x0480:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "SUBI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0600:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "ADDI.B   #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0640:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "ADDI.W   #$%lX,%s", pm & 0xffff, ea);
			break;
		case 0x0680:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ADDI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0800:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "BTST     #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0840:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "BCHG     #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0880:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "BCLR     #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x08c0:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "BSET     #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0a00:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "EORI     #$%lX,CCR", pm & 0xff);
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "EORI.B   #$%lX,%s", pm & 0xff, ea);
			}
			break;
		case 0x0a40:
			PARAM_WORD(pm);
			if (lo == 0x3c)
				sprintf (buffer, "EORI     #$%lX,SR", pm & 0xffff);
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "EORI.W   #$%lX,%s", pm & 0xffff, ea);
			}
			break;
		case 0x0a80:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "EORI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0c00:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "CMPI.B   #$%lX,%s", pm & 0xff, ea);
			break;
		case 0x0c40:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CMPI.W   #$%lX,%s", pm & 0xffff, ea);
			break;
		case 0x0c80:
			PARAM_LONG(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "CMPI.L   #$%lX,%s", pm, ea);
			break;
		case 0x0e00:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 1, &count); p += count;
			if (pm & 0x0800)
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.B  A%ld,%s", (pm >> 12) & 7, ea);
				else
					sprintf (buffer, "MOVES.B  D%ld,%s", (pm >> 12) & 7, ea);
			}
			else
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.B  %s,A%ld", ea, (pm >> 12) & 7);
				else
					sprintf (buffer, "MOVES.B  %s,D%ld", ea, (pm >> 12) & 7);
			}
			break;
		case 0x0e40:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 2, &count); p += count;
			if (pm & 0x0800)
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.W  A%ld,%s", (pm >> 12) & 7, ea);
				else
					sprintf (buffer, "MOVES.W  D%ld,%s", (pm >> 12) & 7, ea);
			}
			else
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.W  %s,A%ld", ea, (pm >> 12) & 7);
				else
					sprintf (buffer, "MOVES.W  %s,D%ld", ea, (pm >> 12) & 7);
			}
			break;
		case 0x0e80:
			PARAM_WORD(pm); ea = MakeEA (lo, p, 4, &count); p += count;
			if (pm & 0x0800)
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.L  A%ld,%s", (pm >> 12) & 7, ea);
				else
					sprintf (buffer, "MOVES.L  D%ld,%s", (pm >> 12) & 7, ea);
			}
			else
			{
				if (pm & 0x8000)
					sprintf (buffer, "MOVES.L  %s,A%ld", ea, (pm >> 12) & 7);
				else
					sprintf (buffer, "MOVES.L  %s,D%ld", ea, (pm >> 12) & 7);
			}
			break;
		case 0x1000: case 0x1080: case 0x10c0: case 0x1100: case 0x1140: case 0x1180: case 0x11c0:
		case 0x1200: case 0x1280: case 0x12c0: case 0x1300: case 0x1340: case 0x1380: case 0x13c0:
		case 0x1400: case 0x1480: case 0x14c0: case 0x1500: case 0x1540: case 0x1580:
		case 0x1600: case 0x1680: case 0x16c0: case 0x1700: case 0x1740: case 0x1780:
		case 0x1800: case 0x1880: case 0x18c0: case 0x1900: case 0x1940: case 0x1980:
		case 0x1a00: case 0x1a80: case 0x1ac0: case 0x1b00: case 0x1b40: case 0x1b80:
		case 0x1c00: case 0x1c80: case 0x1cc0: case 0x1d00: case 0x1d40: case 0x1d80:
		case 0x1e00: case 0x1e80: case 0x1ec0: case 0x1f00: case 0x1f40: case 0x1f80:
			ea = MakeEA (lo, p, 1, &count); p += count; ea2 = MakeEA (((op >> 9) & 0x07) + ((op >> 3) & 0x38), p, 1, &count); p += count;
			sprintf (buffer, "MOVE.B   %s,%s", ea, ea2);
			break;
		case 0x2000: case 0x2080: case 0x20c0: case 0x2100: case 0x2140: case 0x2180: case 0x21c0:
		case 0x2200: case 0x2280: case 0x22c0: case 0x2300: case 0x2340: case 0x2380: case 0x23c0:
		case 0x2400: case 0x2480: case 0x24c0: case 0x2500: case 0x2540: case 0x2580:
		case 0x2600: case 0x2680: case 0x26c0: case 0x2700: case 0x2740: case 0x2780:
		case 0x2800: case 0x2880: case 0x28c0: case 0x2900: case 0x2940: case 0x2980:
		case 0x2a00: case 0x2a80: case 0x2ac0: case 0x2b00: case 0x2b40: case 0x2b80:
		case 0x2c00: case 0x2c80: case 0x2cc0: case 0x2d00: case 0x2d40: case 0x2d80:
		case 0x2e00: case 0x2e80: case 0x2ec0: case 0x2f00: case 0x2f40: case 0x2f80:
			ea = MakeEA (lo, p, 4, &count); p += count; ea2 = MakeEA (((op >> 9) & 0x07) + ((op >> 3) & 0x38), p, 4, &count); p += count;
			sprintf (buffer, "MOVE.L   %s,%s", ea, ea2);
			break;
		case 0x2040: case 0x2240: case 0x2440: case 0x2640: case 0x2840: case 0x2a40: case 0x2c40: case 0x2e40:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "MOVEA.L  %s,A%d", ea, rhi);
			break;
		case 0x3000: case 0x3080: case 0x30c0: case 0x3100: case 0x3140: case 0x3180: case 0x31c0:
		case 0x3200: case 0x3280: case 0x32c0: case 0x3300: case 0x3340: case 0x3380: case 0x33c0:
		case 0x3400: case 0x3480: case 0x34c0: case 0x3500: case 0x3540: case 0x3580:
		case 0x3600: case 0x3680: case 0x36c0: case 0x3700: case 0x3740: case 0x3780:
		case 0x3800: case 0x3880: case 0x38c0: case 0x3900: case 0x3940: case 0x3980:
		case 0x3a00: case 0x3a80: case 0x3ac0: case 0x3b00: case 0x3b40: case 0x3b80:
		case 0x3c00: case 0x3c80: case 0x3cc0: case 0x3d00: case 0x3d40: case 0x3d80:
		case 0x3e00: case 0x3e80: case 0x3ec0: case 0x3f00: case 0x3f40: case 0x3f80:
			ea = MakeEA (lo, p, 2, &count); p += count; ea2 = MakeEA (((op >> 9) & 0x07) + ((op >> 3) & 0x38), p, 2, &count); p += count;
			sprintf (buffer, "MOVE.W   %s,%s", ea, ea2);
			break;
		case 0x3040: case 0x3240: case 0x3440: case 0x3640: case 0x3840: case 0x3a40: case 0x3c40: case 0x3e40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVEA.W  %s,A%d", ea, rhi);
			break;
		case 0x4000:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "NEGX.B   %s", ea);
			break;
		case 0x4040:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "NEGX.W   %s", ea);
			break;
		case 0x4080:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "NEGX.L   %s", ea);
			break;
		case 0x40c0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVE     SR,%s", ea);
			break;
		case 0x4180: case 0x4380: case 0x4580: case 0x4780: case 0x4980: case 0x4b80: case 0x4d80: case 0x4f80:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CHK.W    %s,D%d", ea, rhi);
			break;
		case 0x41c0: case 0x43c0: case 0x45c0: case 0x47c0: case 0x49c0: case 0x4bc0: case 0x4dc0: case 0x4fc0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "LEA      %s,A%d", ea, rhi);
			break;
		case 0x4200:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "CLR.B    %s", ea);
			break;
		case 0x4240:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CLR.W    %s", ea);
			break;
		case 0x4280:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "CLR.L    %s", ea);
			break;
		case 0x42c0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVE     CCR,%s", ea);
			break;
		case 0x4400:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "NEG.B    %s", ea);
			break;
		case 0x4440:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "NEG.W    %s", ea);
			break;
		case 0x4480:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "NEG.L    %s", ea);
			break;
		case 0x44c0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVE     %s,CCR", ea);
			break;
		case 0x4600:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "NOT.B    %s", ea);
			break;
		case 0x4640:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "NOT.W    %s", ea);
			break;
		case 0x4680:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "NOT.L    %s", ea);
			break;
		case 0x46c0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MOVE     %s,SR", ea);
			break;
		case 0x4800:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "NBCD.B   %s", ea);
			break;
		case 0x4840:
			if ((lo & 0x38) == 0x00)
				sprintf (buffer, "SWAP     D%d", rlo);
			else
			{
				ea = MakeEA (lo, p, 4, &count); p += count;
				sprintf (buffer, "PEA      %s", ea);
			}
			break;
		case 0x4880:
			if ((lo & 0x38) == 0x00)
				sprintf (buffer, "EXT.W    D%d", rlo);
			else
			{
				char *b = buffer;
				PARAM_WORD (pm);	ea = MakeEA (lo, p, 2, &count); p += count;
				b += sprintf (b, "MOVEM.W  ");
				if ((lo & 0x38) != 0x20) b = MakeRegList (b, pm);
				else b = MakeRevRegList (b, pm);
				sprintf (b, ",%s", ea);
			}
			break;
		case 0x48c0:
			if ((lo & 0x38) == 0x00)
				sprintf (buffer, "EXT.L    D%d", rlo);
			else
			{
				char *b = buffer;
				PARAM_WORD (pm);	ea = MakeEA (lo, p, 4, &count); p += count;
				b += sprintf (b, "MOVEM.L  ");
				if ((lo & 0x38) != 0x20) b = MakeRegList (b, pm);
				else b = MakeRevRegList (b, pm);
				sprintf (b, ",%s", ea);
			}
			break;
		case 0x4a00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "TST.B    %s", ea);
			break;
		case 0x4a40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "TST.W    %s", ea);
			break;
		case 0x4a80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "TST.L    %s", ea);
			break;
		case 0x4ac0:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "TAS.B    %s", ea);
			break;
		case 0x4c80:
			{
				char *b = buffer;
				PARAM_WORD (pm);	ea = MakeEA (lo, p, 2, &count); p += count;
				b += sprintf (b, "MOVEM.W  %s,", ea);
				b = MakeRegList (b, pm);
			}
			break;
		case 0x4cc0:
			{
				char *b = buffer;
				PARAM_WORD (pm);	ea = MakeEA (lo, p, 4, &count); p += count;
				b += sprintf (b, "MOVEM.L  %s,", ea);
				b = MakeRegList (b, pm);
			}
			break;
		case 0x4e40:
			if ((lo & 30) == 0x00)
				sprintf (buffer, "TRAP     #$%X", lo & 15);
			else if ((lo & 0x38) == 0x10)
			{
				PARAM_WORD (pm);
				sprintf (buffer, "LINK     A%d,#$%lX", rlo, pm);
			}
			else if ((lo & 0x38) == 0x18)
			{
				sprintf (buffer, "UNLK     A%d", rlo);
			}
			else if ((lo & 0x38) == 0x20)
				sprintf (buffer, "MOVE     A%d,USP", rlo);
			else if ((lo & 0x38) == 0x28)
				sprintf (buffer, "MOVE     USP,A%d", rlo);
			else if (lo == 0x30)
				sprintf (buffer, "RESET");
			else if (lo == 0x31)
				sprintf (buffer, "NOP");
			else if (lo == 0x32)
				sprintf (buffer, "STOP");
			else if (lo == 0x33)
				sprintf (buffer, "RTE");
			else if (lo == 0x35)
   				sprintf (buffer, "RTS");
			else if (lo == 0x36)
				sprintf (buffer, "TRAPV");
			else if (lo == 0x37)
				sprintf (buffer, "RTR");
			else if (lo == 0x3a)
			{
				PARAM_WORD (pm);
				switch (pm & 0xfff)
				{
					case 0x000:	ea = "SFC";	break;
					case 0x001:	ea = "DFC"; break;
					case 0x800: ea = "USP"; break;
					case 0x801: ea = "VBR"; break;
					default: ea = "???"; break;
				}
				if (pm & 0x8000)
					sprintf (buffer, "MOVEC    %s,A%ld", ea, (pm >> 12) & 7);
				else
					sprintf (buffer, "MOVEC    %s,D%ld", ea, (pm >> 12) & 7);
			}
			else if (lo == 0x3b)
			{
				PARAM_WORD (pm);
				switch (pm & 0xfff)
				{
					case 0x000:	ea = "SFC";	break;
					case 0x001:	ea = "DFC"; break;
					case 0x800: ea = "USP"; break;
					case 0x801: ea = "VBR"; break;
					default: ea = "???"; break;
				}
				if (pm & 0x8000)
					sprintf (buffer, "MOVEC    A%ld,%s", (pm >> 12) & 7, ea);
				else
					sprintf (buffer, "MOVEC    D%ld,%s", (pm >> 12) & 7, ea);
			}
			else
				sprintf (buffer, "DC.W     $%X", op);
			break;
		case 0x4e80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "JSR      %s", ea);
			break;
		case 0x4ec0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "JMP      %s", ea);
			break;
		case 0x5000: case 0x5200: case 0x5400: case 0x5600: case 0x5800: case 0x5a00: case 0x5c00: case 0x5e00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "ADDQ.B   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x5040: case 0x5240: case 0x5440: case 0x5640: case 0x5840: case 0x5a40: case 0x5c40: case 0x5e40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "ADDQ.W   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x5080: case 0x5280: case 0x5480: case 0x5680: case 0x5880: case 0x5a80: case 0x5c80: case 0x5e80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ADDQ.L   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x50c0: case 0x52c0: case 0x54c0: case 0x56c0: case 0x58c0: case 0x5ac0: case 0x5cc0: case 0x5ec0:
		case 0x51c0: case 0x53c0: case 0x55c0: case 0x57c0: case 0x59c0: case 0x5bc0: case 0x5dc0: case 0x5fc0:
			if ((lo & 0x38) == 0x08)
			{
				PARAM_WORD (pm);
				if (pm & 0x8000)
					sprintf (buffer, "DB%s     D%d,*-$%X [%X]", ccodes[(op >> 8) & 15], rlo, (int)(-(signed short)pm) - 2, pc + (signed short)pm + 2);
				else
					sprintf (buffer, "DB%s     D%d,*+$%lX [%lX]", ccodes[(op >> 8) & 15], rlo, pm - 2, pc + pm + 2);
			}
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "S%s.B    %s", ccodes[(op >> 8) & 15], ea);
			}
			break;
		case 0x5100: case 0x5300: case 0x5500: case 0x5700: case 0x5900: case 0x5b00: case 0x5d00: case 0x5f00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "SUBQ.B   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x5140: case 0x5340: case 0x5540: case 0x5740: case 0x5940: case 0x5b40: case 0x5d40: case 0x5f40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "SUBQ.W   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x5180: case 0x5380: case 0x5580: case 0x5780: case 0x5980: case 0x5b80: case 0x5d80: case 0x5f80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "SUBQ.L   #%d,%s", (rhi == 0) ? 8 : rhi, ea);
			break;
		case 0x6000: case 0x6040: case 0x6080: case 0x60c0:
			pm = op & 0xff;
			if (pm == 0)
			{
				PARAM_WORD(pm);
				if (pm & 0x8000)
					sprintf (buffer, "BRA      *-$%X [%X]", (int)(-(signed short)pm) - 2, pc + (signed short)pm + 2);
				else
					sprintf (buffer, "BRA      *+$%lX [%lX]", pm + 2, pc + pm + 2);
			}
			else
			{
				if (pm & 0x80)
					sprintf (buffer, "BRA.S    *-$%X [%X]", (int)(-(signed char)pm) - 2, pc + (signed char)pm + 2);
				else
					sprintf (buffer, "BRA.S    *+$%lX [%lX]", pm + 2, pc + pm + 2);
			}
			break;
		case 0x6100: case 0x6140: case 0x6180: case 0x61c0:
			pm = op & 0xff;
			if (pm == 0)
			{
				PARAM_WORD(pm);
				if (pm & 0x8000)
					sprintf (buffer, "BSR      *-$%X [%X]", (int)(-(signed short)pm) - 2, pc + (signed short)pm + 2);
				else
					sprintf (buffer, "BSR      *+$%lX [%lX]", pm + 2, pc + pm + 2);
			}
			else
			{
				if (pm & 0x80)
					sprintf (buffer, "BSR.S    *-$%X [%X]", (int)(-(signed char)pm) - 2, pc + (signed char)pm + 2);
				else
					sprintf (buffer, "BSR.S    *+$%lX [%lX]", pm + 2, pc + pm + 2);
			}
			break;
		case 0x6200: case 0x6240: case 0x6280: case 0x62c0: case 0x6300: case 0x6340: case 0x6380: case 0x63c0:
		case 0x6400: case 0x6440: case 0x6480: case 0x64c0: case 0x6500: case 0x6540: case 0x6580: case 0x65c0:
		case 0x6600: case 0x6640: case 0x6680: case 0x66c0: case 0x6700: case 0x6740: case 0x6780: case 0x67c0:
		case 0x6800: case 0x6840: case 0x6880: case 0x68c0: case 0x6900: case 0x6940: case 0x6980: case 0x69c0:
		case 0x6a00: case 0x6a40: case 0x6a80: case 0x6ac0: case 0x6b00: case 0x6b40: case 0x6b80: case 0x6bc0:
		case 0x6c00: case 0x6c40: case 0x6c80: case 0x6cc0: case 0x6d00: case 0x6d40: case 0x6d80: case 0x6dc0:
		case 0x6e00: case 0x6e40: case 0x6e80: case 0x6ec0: case 0x6f00: case 0x6f40: case 0x6f80: case 0x6fc0:
			pm = op & 0xff;
			if (pm == 0)
			{
				PARAM_WORD(pm);
				if (pm & 0x8000)
					sprintf (buffer, "B%s      *-$%X [%X]", ccodes[(op >> 8) & 15], (int)(-(signed short)pm) - 2, pc + (signed short)pm + 2);
				else
					sprintf (buffer, "B%s      *+$%lX [%lX]", ccodes[(op >> 8) & 15], pm + 2, pc + pm + 2);
			}
			else
			{
				if (pm & 0x80)
					sprintf (buffer, "B%s.S    *-$%X [%X]", ccodes[(op >> 8) & 15], (int)(-(signed char)pm) - 2, pc + (signed char)pm + 2);
				else
					sprintf (buffer, "B%s.S    *+$%lX [%lX]", ccodes[(op >> 8) & 15], pm + 2, pc + pm + 2);
			}
			break;
		case 0x7000: case 0x7040: case 0x7080: case 0x70c0:
		case 0x7200: case 0x7240: case 0x7280: case 0x72c0:
		case 0x7400: case 0x7440: case 0x7480: case 0x74c0:
		case 0x7600: case 0x7640: case 0x7680: case 0x76c0:
		case 0x7800: case 0x7840: case 0x7880: case 0x78c0:
		case 0x7a00: case 0x7a40: case 0x7a80: case 0x7ac0:
		case 0x7c00: case 0x7c40: case 0x7c80: case 0x7cc0:
		case 0x7e00: case 0x7e40: case 0x7e80: case 0x7ec0:
			pm = op & 0xff;
			if (pm & 0x80)
				sprintf (buffer, "MOVEQ    #$-%X,D%d", -(signed char)pm, rhi);
			else
				sprintf (buffer, "MOVEQ    #$%lX,D%d", pm, rhi);
			break;
		case 0x8000: case 0x8200: case 0x8400: case 0x8600: case 0x8800: case 0x8a00: case 0x8c00: case 0x8e00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "OR.B     %s,D%d", ea, rhi);
			break;
		case 0x8040: case 0x8240: case 0x8440: case 0x8640: case 0x8840: case 0x8a40: case 0x8c40: case 0x8e40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "OR.W     %s,D%d", ea, rhi);
			break;
		case 0x8080: case 0x8280: case 0x8480: case 0x8680: case 0x8880: case 0x8a80: case 0x8c80: case 0x8e80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "OR.L     %s,D%d", ea, rhi);
			break;
		case 0x80c0: case 0x82c0: case 0x84c0: case 0x86c0: case 0x88c0: case 0x8ac0: case 0x8cc0: case 0x8ec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "DIVU.W   %s,D%d", ea, rhi);
			break;
		case 0x8100: case 0x8300: case 0x8500: case 0x8700: case 0x8900: case 0x8b00: case 0x8d00: case 0x8f00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "OR.B     D%d,%s", rhi, ea);
			break;
		case 0x8140: case 0x8340: case 0x8540: case 0x8740: case 0x8940: case 0x8b40: case 0x8d40: case 0x8f40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "OR.W     D%d,%s", rhi, ea);
			break;
		case 0x8180: case 0x8380: case 0x8580: case 0x8780: case 0x8980: case 0x8b80: case 0x8d80: case 0x8f80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "OR.L     D%d,%s", rhi, ea);
			break;
		case 0x81c0: case 0x83c0: case 0x85c0: case 0x87c0: case 0x89c0: case 0x8bc0: case 0x8dc0: case 0x8fc0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "DIVS.W   %s,D%d", ea, rhi);
			break;
		case 0x9000: case 0x9200: case 0x9400: case 0x9600: case 0x9800: case 0x9a00: case 0x9c00: case 0x9e00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "SUB.B    %s,D%d", ea, rhi);
			break;
		case 0x9040: case 0x9240: case 0x9440: case 0x9640: case 0x9840: case 0x9a40: case 0x9c40: case 0x9e40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "SUB.W    %s,D%d", ea, rhi);
			break;
		case 0x9080: case 0x9280: case 0x9480: case 0x9680: case 0x9880: case 0x9a80: case 0x9c80: case 0x9e80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "SUB.L    %s,D%d", ea, rhi);
			break;
		case 0x90c0: case 0x92c0: case 0x94c0: case 0x96c0: case 0x98c0: case 0x9ac0: case 0x9cc0: case 0x9ec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "SUBA.W   %s,A%d", ea, rhi);
			break;
		case 0x9100: case 0x9300: case 0x9500: case 0x9700: case 0x9900: case 0x9b00: case 0x9d00: case 0x9f00:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "SUBX.B   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "SUBX.B   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 1,&count); p += count;
				sprintf (buffer, "SUB.B    D%d,%s", rhi, ea);
			}
			break;
		case 0x9140: case 0x9340: case 0x9540: case 0x9740: case 0x9940: case 0x9b40: case 0x9d40: case 0x9f40:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "SUBX.W   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "SUBX.W   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "SUB.W    D%d,%s", rhi, ea);
			}
			break;
		case 0x9180: case 0x9380: case 0x9580: case 0x9780: case 0x9980: case 0x9b80: case 0x9d80: case 0x9f80:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "SUBX.L   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "SUBX.L   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 4, &count); p += count;
				sprintf (buffer, "SUB.L    D%d,%s", rhi, ea);
			}
			break;
		case 0x91c0: case 0x93c0: case 0x95c0: case 0x97c0: case 0x99c0: case 0x9bc0: case 0x9dc0: case 0x9fc0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "SUBA.L   %s,A%d", ea, rhi);
			break;
		case 0xb000: case 0xb200: case 0xb400: case 0xb600: case 0xb800: case 0xba00: case 0xbc00: case 0xbe00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "CMP.B    %s,D%d", ea, rhi);
			break;
		case 0xb040: case 0xb240: case 0xb440: case 0xb640: case 0xb840: case 0xba40: case 0xbc40: case 0xbe40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CMP.W    %s,D%d", ea, rhi);
			break;
		case 0xb080: case 0xb280: case 0xb480: case 0xb680: case 0xb880: case 0xba80: case 0xbc80: case 0xbe80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "CMP.L    %s,D%d", ea, rhi);
			break;
		case 0xb0c0: case 0xb2c0: case 0xb4c0: case 0xb6c0: case 0xb8c0: case 0xbac0: case 0xbcc0: case 0xbec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "CMPA.W   %s,A%d", ea, rhi);
			break;
		case 0xb100: case 0xb300: case 0xb500: case 0xb700: case 0xb900: case 0xbb00: case 0xbd00: case 0xbf00:
			if ((lo & 0x38) == 0x08)
				sprintf (buffer, "CMPM.B   (A%d)+,(A%d)+", rlo, rhi);
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "EOR.B    D%d,%s", rhi, ea);
			}
			break;
		case 0xb140: case 0xb340: case 0xb540: case 0xb740: case 0xb940: case 0xbb40: case 0xbd40: case 0xbf40:
			if ((lo & 0x38) == 0x08)
				sprintf (buffer, "CMPM.W   (A%d)+,(A%d)+", rlo, rhi);
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "EOR.W    D%d,%s", rhi, ea);
			}
			break;
		case 0xb180: case 0xb380: case 0xb580: case 0xb780: case 0xb980: case 0xbb80: case 0xbd80: case 0xbf80:
			if ((lo & 0x38) == 0x08)
				sprintf (buffer, "CMPM.L   (A%d)+,(A%d)+", rlo, rhi);
			else
			{
				ea = MakeEA (lo, p, 4, &count); p += count;
				sprintf (buffer, "EOR.L    D%d,%s", rhi, ea);
			}
			break;
		case 0xb1c0: case 0xb3c0: case 0xb5c0: case 0xb7c0: case 0xb9c0: case 0xbbc0: case 0xbdc0: case 0xbfc0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "CMPA.L   %s,A%d", ea, rhi);
			break;
		case 0xc000: case 0xc200: case 0xc400: case 0xc600: case 0xc800: case 0xca00: case 0xcc00: case 0xce00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "AND.B    %s,D%d", ea, rhi);
			break;
		case 0xc040: case 0xc240: case 0xc440: case 0xc640: case 0xc840: case 0xca40: case 0xcc40: case 0xce40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "AND.W    %s,D%d", ea, rhi);
			break;
		case 0xc080: case 0xc280: case 0xc480: case 0xc680: case 0xc880: case 0xca80: case 0xcc80: case 0xce80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "AND.L    %s,D%d", ea, rhi);
			break;
		case 0xc0c0: case 0xc2c0: case 0xc4c0: case 0xc6c0: case 0xc8c0: case 0xcac0: case 0xccc0: case 0xcec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MULU.W   %s,D%d", ea, rhi);
			break;
		case 0xc100: case 0xc300: case 0xc500: case 0xc700: case 0xc900: case 0xcb00: case 0xcd00: case 0xcf00:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "ABCD.B   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "ABCD.B   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "AND.B    D%d,%s", rhi, ea);
			}
			break;
		case 0xc140: case 0xc340: case 0xc540: case 0xc740: case 0xc940: case 0xcb40: case 0xcd40: case 0xcf40:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "EXG      A%d,A%d", rhi, rlo);
				else
					sprintf (buffer, "EXG      D%d,D%d", rhi, rlo);
			}
			else
			{
				ea = MakeEA (lo, p, 2,&count); p += count;
				sprintf (buffer, "AND.W    D%d,%s", rhi, ea);
			}
			break;
		case 0xc180: case 0xc380: case 0xc580: case 0xc780: case 0xc980: case 0xcb80: case 0xcd80: case 0xcf80:
			if ((lo & 0x38) == 0x08)
				sprintf (buffer, "EXG      D%d,A%d", rhi, rlo);
			else
			{
				ea = MakeEA (lo, p, 4, &count); p += count;
				sprintf (buffer, "AND.L    D%d,%s", rhi, ea);
			}
			break;
		case 0xc1c0: case 0xc3c0: case 0xc5c0: case 0xc7c0: case 0xc9c0: case 0xcbc0: case 0xcdc0: case 0xcfc0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "MULS.W   %s,D%d", ea, rhi);
			break;
		case 0xd000: case 0xd200: case 0xd400: case 0xd600: case 0xd800: case 0xda00: case 0xdc00: case 0xde00:
			ea = MakeEA (lo, p, 1, &count); p += count;
			sprintf (buffer, "ADD.B    %s,D%d", ea, rhi);
			break;
		case 0xd040: case 0xd240: case 0xd440: case 0xd640: case 0xd840: case 0xda40: case 0xdc40: case 0xde40:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "ADD.W    %s,D%d", ea, rhi);
			break;
		case 0xd080: case 0xd280: case 0xd480: case 0xd680: case 0xd880: case 0xda80: case 0xdc80: case 0xde80:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ADD.L    %s,D%d", ea, rhi);
			break;
		case 0xd0c0: case 0xd2c0: case 0xd4c0: case 0xd6c0: case 0xd8c0: case 0xdac0: case 0xdcc0: case 0xdec0:
			ea = MakeEA (lo, p, 2, &count); p += count;
			sprintf (buffer, "ADDA.W   %s,A%d", ea, rhi);
			break;
		case 0xd100: case 0xd300: case 0xd500: case 0xd700: case 0xd900: case 0xdb00: case 0xdd00: case 0xdf00:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "ADDX.B   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "ADDX.B   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 1, &count); p += count;
				sprintf (buffer, "ADD.B    D%d,%s", rhi, ea);
			}
			break;
		case 0xd140: case 0xd340: case 0xd540: case 0xd740: case 0xd940: case 0xdb40: case 0xdd40: case 0xdf40:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "ADDX.W   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "ADDX.W   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 2, &count); p += count;
				sprintf (buffer, "ADD.W    D%d,%s", rhi, ea);
			}
			break;
		case 0xd180: case 0xd380: case 0xd580: case 0xd780: case 0xd980: case 0xdb80: case 0xdd80: case 0xdf80:
			if ((lo & 0x30) == 0)
			{
				if (lo & 0x08)
					sprintf (buffer, "ADDX.L   -(A%d),-(A%d)", rlo, rhi);
				else
					sprintf (buffer, "ADDX.L   D%d,D%d", rlo, rhi);
			}
			else
			{
				ea = MakeEA (lo, p, 4,&count); p += count;
				sprintf (buffer, "ADD.L    D%d,%s", rhi, ea);
			}
			break;
		case 0xd1c0: case 0xd3c0: case 0xd5c0: case 0xd7c0: case 0xd9c0: case 0xdbc0: case 0xddc0: case 0xdfc0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			sprintf (buffer, "ADDA.L   %s,A%d", ea, rhi);
			break;
		case 0xe000: case 0xe200: case 0xe400: case 0xe600: case 0xe800: case 0xea00: case 0xec00: case 0xee00:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASR.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSR.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXR.B   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROR.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASR.B    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSR.B    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXR.B   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROR.B    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe040: case 0xe240: case 0xe440: case 0xe640: case 0xe840: case 0xea40: case 0xec40: case 0xee40:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASR.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSR.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXR.W   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROR.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASR.W    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSR.W    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXR.W   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROR.W    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe080: case 0xe280: case 0xe480: case 0xe680: case 0xe880: case 0xea80: case 0xec80: case 0xee80:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASR.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSR.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXR.L   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROR.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASR.L    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSR.L    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXR.L   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROR.L    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe0c0: case 0xe2c0: case 0xe4c0: case 0xe6c0:
		case 0xe1c0: case 0xe3c0: case 0xe5c0: case 0xe7c0:
			ea = MakeEA (lo, p, 4, &count); p += count;
			switch ((op >> 8) & 7)
			{
				case 0:	sprintf (buffer, "ASR.L    #1,%s", ea);		break;
				case 1:	sprintf (buffer, "ASL.L    #1,%s", ea);		break;
				case 2:	sprintf (buffer, "LSR.L    #1,%s", ea);		break;
				case 3:	sprintf (buffer, "LSL.L    #1,%s", ea);		break;
				case 4:	sprintf (buffer, "ROXR.L   #1,%s", ea);		break;
				case 5:	sprintf (buffer, "ROXL.L   #1,%s", ea);		break;
				case 6:	sprintf (buffer, "ROR.L    #1,%s", ea);		break;
				case 7:	sprintf (buffer, "ROL.L    #1,%s", ea);		break;
			}
			break;
		case 0xe100: case 0xe300: case 0xe500: case 0xe700: case 0xe900: case 0xeb00: case 0xed00: case 0xef00:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASL.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSL.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXL.B   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROL.B    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASL.B    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSL.B    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXL.B   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROL.B    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe140: case 0xe340: case 0xe540: case 0xe740: case 0xe940: case 0xeb40: case 0xed40: case 0xef40:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASL.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSL.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXL.W   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROL.W    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASL.W    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSL.W    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXL.W   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROL.W    D%d,D%d", rhi, rlo);		break;
			}
			break;
		case 0xe180: case 0xe380: case 0xe580: case 0xe780: case 0xe980: case 0xeb80: case 0xed80: case 0xef80:
			switch ((lo >> 3) & 7)
			{
				case 0:	sprintf (buffer, "ASL.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 1:	sprintf (buffer, "LSL.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 2:	sprintf (buffer, "ROXL.L   #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 3:	sprintf (buffer, "ROL.L    #%d,D%d", (rhi == 0) ? 8 : rhi, rlo);		break;
				case 4:	sprintf (buffer, "ASL.L    D%d,D%d", rhi, rlo);		break;
				case 5:	sprintf (buffer, "LSL.L    D%d,D%d", rhi, rlo);		break;
				case 6:	sprintf (buffer, "ROXL.L   D%d,D%d", rhi, rlo);		break;
				case 7:	sprintf (buffer, "ROL.L    D%d,D%d", rhi, rlo);		break;
			}
			break;
		default:
			sprintf (buffer, "DC.W     $%X", op);
			break;
	}

	return p - pBase;
}



#endif
