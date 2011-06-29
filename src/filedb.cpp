//
// filedb.cpp - File database
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  02/15/2010  Created this file
//

#include "filedb.h"


#if 0
struct RomIdentifier
{
	const uint32 crc32;
	const char name[128];
//	const char file[128];
	const uint32 flags;
};

enum FileFlags { FF_ROM=1, FF_ALPINE=2, FF_BIOS=4, FF_REQ_DSP=8, FF_REQ_BIOS=16, FF_NON_WORKING=32, FF_BAD_DUMP=64 };
#endif

// Should have another flag for whether or not it requires DSP, BIOS,
// whether it's a .rom, it's a BIOS, etc...
// ... And now we do! :-D

RomIdentifier romList[] = {
	{ 0x0509C85E, "Raiden (World)", FF_ROM },
	{ 0x08F15576, "Iron Soldier (World) (v1.04)", FF_ROM },
	{ 0x0957A072, "Kasumi Ninja (World)", FF_ROM },
	{ 0x0AC83D77, "NBA Jam T.E. (World)", FF_ROM },
	{ 0x0EC5369D, "Evolution - Dino Dudes (World)", FF_ROM },
	{ 0x0F6A1C2C, "Ultra Vortek (World)", FF_ROM },
	{ 0x14915F20, "White Men Can't Jump (World)", FF_ROM },
	{ 0x1660F070, "Power Drive Rally (World)", FF_ROM },
	{ 0x1E451446, "Trevor McFur in the Crescent Galaxy (World)", FF_ROM },
//	{ 0x20DBFF9F, "Breakout 2000", FF_ROM },
	{ 0x27594C6A, "Defender 2000 (World)", FF_ROM },
	{ 0x2E17D5DA, "Bubsy in Fractured Furry Tales (World)", FF_ROM },
	{ 0x348E6449, "Double Dragon V - The Shadow Falls (World)", FF_ROM },
	{ 0x3615AF6A, "Fever Pitch Soccer (World) (En,Fr,De,Es,It)", FF_ROM },
	{ 0x38A130ED, "Troy Aikman NFL Football (World)", FF_ROM },
	{ 0x3C044941, "Skyhammer (World)", FF_ROM },
	{ 0x40E1A1D0, "Air Cars (World)", FF_ROM },
	{ 0x42A13EC5, "Soccer Kid (World)", FF_ROM },
	{ 0x45AA46BA, "Space War 2000 (World)", FF_ROM },
	{ 0x47EBC158, "Theme Park (World)", FF_ROM },
	{ 0x4899628F, "Hover Strike (World)", FF_ROM },
	{ 0x53DF6440, "Space War 2000 (World) (OVERDUMP)", FF_ROM },
	{ 0x55A0669C, "[BIOS] Atari Jaguar Developer CD (World)", FF_BIOS },
	{ 0x58272540, "Syndicate (World)", FF_ROM },
	{ 0x5A101212, "Sensible Soccer - International Edition (World)", FF_ROM },
	{ 0x5B6BB205, "Ruiner Pinball (World)", FF_ROM },
	{ 0x5CFF14AB, "Pinball Fantasies (World)", FF_ROM },
	{ 0x5E2CDBC0, "Doom (World)", FF_ROM },
	{ 0x61C7EEC0, "Zero 5 (World)", FF_ROM },
	{ 0x67F9AB3A, "Battle Sphere Gold (World)", FF_ROM },
	{ 0x687068D5, "[BIOS] Atari Jaguar CD (World)", FF_BIOS },
	{ 0x6B2B95AD, "Tempest 2000 (World)", FF_ROM },
	{ 0x6EB774EB, "Worms (World)", FF_ROM },
	{ 0x6F8B2547, "Super Burnout (World)", FF_ROM },
	{ 0x817A2273, "Pitfall - The Mayan Adventure (World)", FF_ROM },
	{ 0x8975F48B, "Zool 2 (World)", FF_ROM },
	{ 0x8D15DBC6, "[BIOS] Atari Jaguar Stubulator '94 (World)", FF_BIOS },
	{ 0x8FEA5AB0, "Dragon - The Bruce Lee Story (World)", FF_ROM },
	{ 0x97EB4651, "I-War (World)", FF_ROM },
	{ 0xA27823D8, "Ultra Vortek (World) (v0.94) (Beta)", FF_ROM },
	{ 0xA56D0798, "Protector - Special Edition (World)", FF_ROM },
	{ 0xA9F8A00E, "Rayman (World)", FF_ROM },
	{ 0xB14C4753, "Fight for Life (World)", FF_ROM },
	{ 0xBCB1A4BF, "Brutal Sports Football (World)", FF_ROM },
	{ 0xBDA405C6, "Cannon Fodder (World)", FF_ROM },
	{ 0xBDE67498, "Cybermorph (World) (Rev 1)", FF_ROM },
	{ 0xC5562581, "Zoop! (World)", FF_ROM },
	{ 0xC654681B, "Total Carnage (World)", FF_ROM },
	{ 0xC6C7BA62, "Fight for Life (World) (Alt)", FF_ROM | FF_BAD_DUMP },
	{ 0xC9608717, "Val d'Isere Skiing and Snowboarding (World)", FF_ROM },
	{ 0xCAF33BD6, "Towers II", FF_ROM },
	{ 0xCD5BF827, "Attack of the Mutant Penguins (World)", FF_ROM | FF_REQ_DSP },
	{ 0xD6C19E34, "Iron Soldier 2 (World)", FF_ROM },
	{ 0xD8696F23, "Breakout 2000", FF_ALPINE },
	{ 0xDA9C4162, "Missile Command 3D (World)", FF_ROM },
	{ 0xDC187F82, "Alien vs Predator (World)", FF_ROM },
	{ 0xDDFF49F5, "Rayman (Prototype)", FF_ALPINE },
	{ 0xDE55DCC7, "Flashback - The Quest for Identity (World) (En,Fr)", FF_ROM },
	{ 0xE28756DE, "Atari Karts (World)", FF_ROM },
	{ 0xE60277BB, "[BIOS] Atari Jaguar Stubulator '93 (World)", FF_BIOS },
	{ 0xE91BD644, "Wolfenstein 3D (World)", FF_ROM },
	{ 0xEC22F572, "SuperCross 3D (World)", FF_ROM },
	{ 0xECF854E7, "Cybermorph (World) (Rev 2)", FF_ROM },
	{ 0xEEE8D61D, "Club Drive (World)", FF_ROM },
	{ 0xF0360DB3, "Hyper Force (World)", FF_ROM },
	{ 0xFA7775AE, "Checkered Flag (World)", FF_ROM },
	{ 0xFAE31DD0, "Flip Out! (World)", FF_ROM },
	{ 0xFB731AAA, "[BIOS] Atari Jaguar (World)", FF_BIOS },
	{ 0xFFFFFFFF, "***END***", 0 }
};
