// FB Alpha Crime Fighters driver module
// Based on MAME driver by Manuel Abadia

#include "tiles_generic.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "k007232.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvKonROM;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvGfxROMExp0;
static unsigned char *DrvGfxROMExp1;
static unsigned char *DrvSndROM;
static unsigned char *DrvBankRAM;
static unsigned char *DrvKonRAM;
static unsigned char *DrvPalRAM;
static unsigned char *DrvZ80RAM;
static unsigned int  *DrvPalette;
static unsigned char DrvRecalc;

static unsigned char *soundlatch;
static unsigned char *nDrvRamBank;
static unsigned char *nDrvKonamiBank;

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvJoy4[8];
static unsigned char DrvJoy5[8];
static unsigned char DrvDips[3];
static unsigned char DrvInputs[5];
static unsigned char DrvReset;

static struct BurnInputInfo CrimfghtInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy5 + 0,	"p1 coin"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy5 + 1,	"p2 coin"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy5 + 2,	"p3 coin"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 fire 2"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy5 + 3,	"p4 coin"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy4 + 2,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy4 + 3,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy4 + 0,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy4 + 1,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy4 + 4,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy4 + 5,	"p4 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy5 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Crimfght)

static struct BurnInputInfo CrimfgtjInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Crimfgtj)

static struct BurnDIPInfo CrimfghtDIPList[]=
{
	{0x1e, 0xff, 0xff, 0xff, NULL			},
	{0x1f, 0xff, 0xff, 0x5f, NULL			},
	{0x20, 0xff, 0xff, 0xfb, NULL			},

	{0   , 0xfe, 0   ,   16, "Coinage"		},
	{0x1e, 0x01, 0x0f, 0x02, "4 Coins 1 Credits "	},
	{0x1e, 0x01, 0x0f, 0x05, "3 Coins 1 Credits "	},
	{0x1e, 0x01, 0x0f, 0x08, "2 Coins 1 Credits "	},
	{0x1e, 0x01, 0x0f, 0x04, "3 Coins 2 Credits "	},
	{0x1e, 0x01, 0x0f, 0x01, "4 Coins 3 Credits "	},
	{0x1e, 0x01, 0x0f, 0x0f, "1 Coin 1 Credits "	},
	{0x1e, 0x01, 0x0f, 0x03, "3 Coins 4 Credits "	},
	{0x1e, 0x01, 0x0f, 0x07, "2 Coins 3 Credits "	},
	{0x1e, 0x01, 0x0f, 0x0e, "1 Coin 2 Credits "	},
	{0x1e, 0x01, 0x0f, 0x06, "2 Coins 5 Credits "	},
	{0x1e, 0x01, 0x0f, 0x0d, "1 Coin 3 Credits "	},
	{0x1e, 0x01, 0x0f, 0x0c, "1 Coin 4 Credits "	},
	{0x1e, 0x01, 0x0f, 0x0b, "1 Coin 5 Credits "	},
	{0x1e, 0x01, 0x0f, 0x0a, "1 Coin 6 Credits "	},
	{0x1e, 0x01, 0x0f, 0x09, "1 Coin 7 Credits "	},
	{0x1e, 0x01, 0x0f, 0x00, "1 Coin/99 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x1f, 0x01, 0x60, 0x60, "Easy"			},
	{0x1f, 0x01, 0x60, 0x40, "Normal"		},
	{0x1f, 0x01, 0x60, 0x20, "Difficult"		},
	{0x1f, 0x01, 0x60, 0x00, "Very difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x1f, 0x01, 0x80, 0x80, "Off"			},
	{0x1f, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x20, 0x01, 0x01, 0x01, "Off"			},
//	{0x20, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x20, 0x01, 0x04, 0x00, "Off"			},
	{0x20, 0x01, 0x04, 0x04, "On"			},
};

STDDIPINFO(Crimfght)

static struct BurnDIPInfo CrimfgtjDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL			},
	{0x11, 0xff, 0xff, 0x5f, NULL			},
	{0x12, 0xff, 0xff, 0xfb, NULL			},

	{0   , 0xfe, 0   ,    16, "Coinage"		},
	{0x10, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x10, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x10, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x10, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x10, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x10, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x10, 0x01, 0x0f, 0x00, "1 Coin/99 Credits"	},

	{0   , 0xfe, 0   ,   16, "Coin B"		},
	{0x10, 0x01, 0xf0, 0x20, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x50, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x10, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x10, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x10, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x10, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x10, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x10, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x10, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x11, 0x01, 0x60, 0x60, "Easy"			},
	{0x11, 0x01, 0x60, 0x40, "Normal"		},
	{0x11, 0x01, 0x60, 0x20, "Difficult"		},
	{0x11, 0x01, 0x60, 0x00, "Very difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0x03, 0x03, "1"			},
	{0x12, 0x01, 0x03, 0x02, "2"			},
	{0x12, 0x01, 0x03, 0x01, "3"			},
	{0x12, 0x01, 0x03, 0x00, "4"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x04, 0x00, "Off"			},
	{0x12, 0x01, 0x04, 0x04, "On"			},
};

STDDIPINFO(Crimfgtj)

static void set_ram_bank(int data)
{
	nDrvRamBank[0] = data;

	if (data & 0x20) {
		konamiMapMemory(DrvPalRAM,  0x0000, 0x03ff, KON_RAM);
	} else {
		konamiMapMemory(DrvBankRAM, 0x0000, 0x03ff, KON_RAM);
	}
}

void crimfght_main_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x3f8c:
			*soundlatch = data;
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		break;
	}

	if (address >= 0x2000 && address <= 0x5fff) {
		K052109_051960_w(address - 0x2000, data);
		return;
	}
}

unsigned char crimfght_main_read(unsigned short address)
{
	switch (address)
	{
		case 0x3f80:
			return DrvInputs[4];

		case 0x3f81:
			return DrvInputs[0];

		case 0x3f82:
			return DrvInputs[1];

		case 0x3f83:
			return DrvDips[1];

		case 0x3f84:
			return DrvDips[2];

		case 0x3f85:
			return DrvInputs[2];

		case 0x3f86:
			return DrvInputs[3];

		case 0x3f87:
			return DrvDips[0];

		case 0x3f88:
			// watchdog reset
			return 0;
	}

	if (address >= 0x2000 && address <= 0x5fff) {
		return K052109_051960_r(address - 0x2000);
	}

	return 0;
}

void __fastcall crimfght_sound_write(unsigned short address, unsigned char data)
{
	if ((address & 0xfff0) == 0xe000) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}

	switch (address)
	{
		case 0xa000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xa001:
			BurnYM2151WriteRegister(data);
		return;
	}
}

unsigned char __fastcall crimfght_sound_read(unsigned short address)
{
	if ((address & 0xfff0) == 0xe000) {
		return K007232ReadReg(0, address & 0x0f);
	}

	switch (address)
	{
		case 0xa000:
		case 0xa001:
			return BurnYM2151ReadStatus();

		case 0xc000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

static void DrvK007232VolCallback(int v)
{
	K007232SetVolume(0, 0, (v >> 0x4) * 0x11, 0);
	K007232SetVolume(0, 1, 0, (v & 0x0f) * 0x11);
}

static void DrvYM2151WritePort(unsigned int, unsigned int data)
{
	int bank_A = ((data >> 1) & 0x01);
	int bank_B = ((data) & 0x01);

	k007232_set_bank(0, bank_A, bank_B);
}

static void crimfght_set_lines(int lines)
{
	nDrvKonamiBank[0] = lines;

	set_ram_bank(lines & 0x20);
	K052109RMRDLine = lines & 0x40;

	int nBank = 0x10000 + ((lines & 0x0f) * 0x2000);

	konamiMapMemory(DrvKonROM + nBank, 0x6000, 0x7fff, KON_ROM); 
}

static void K052109Callback(int layer, int bank, int *code, int *color, int *flipx, int *)
{
	*flipx = *color & 0x20;
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
        *code &= 0x3fff;
	*color = (layer << 2) + ((*color & 0xc0) >> 6);
}

static void K051960Callback(int *code, int *color,int *priority, int *shadow)
{
	switch (*color & 0x70)
	{
		case 0x10: *priority = 0; break;
		case 0x00: *priority = 1; break;
		case 0x40: *priority = 2; break;
		case 0x20: *priority = 3; break;
	}

	*code |= (*color & 0x80) << 6;
	*code &= 0x1fff;
	*color = 16 + (*color & 0x0f);
	*shadow = 0;
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	konamiOpen(0);
	konamiReset();
	konamiClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();

	KonamiICReset();

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x040000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x080000;
	DrvGfxROM1		= Next; Next += 0x100000;
	DrvGfxROMExp0		= Next; Next += 0x100000;
	DrvGfxROMExp1		= Next; Next += 0x200000;

	DrvSndROM		= Next; Next += 0x040000;

	DrvPalette		= (unsigned int*)Next; Next += 0x200 * sizeof(int);

	AllRam			= Next;

	DrvBankRAM		= Next; Next += 0x000400;
	DrvKonRAM		= Next; Next += 0x001c00;
	DrvPalRAM		= Next; Next += 0x000400;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;

	nDrvRamBank		= Next; Next += 0x000001;
	nDrvKonamiBank		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane0[4] = { 0x018, 0x010, 0x008, 0x000 };
	int Plane1[4] = { 0x000, 0x008, 0x010, 0x018 };
	int XOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			  0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107 };
	int YOffs[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			  0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x080000);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x100000);

	GfxDecode(0x4000, 4,  8,  8, Plane0, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x2000, 4, 16, 16, Plane1, XOffs, YOffs, 0x400, DrvGfxROM1, DrvGfxROMExp1);

	return 0;
}

static int DrvInit()
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvKonROM  + 0x010000,  0, 1)) return 1;
		memcpy (DrvKonROM + 0x08000, DrvKonROM + 0x28000, 0x8000);
		memset (DrvKonROM + 0x28000, 0, 0x8000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  1, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x040000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,  5, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000,  6, 1)) return 1;

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvBankRAM,          0x0000, 0x03ff, KON_RAM);
	konamiMapMemory(DrvKonRAM,           0x0400, 0x1fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x10000, 0x6000, 0x7fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x08000, 0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(crimfght_main_write);
	konamiSetReadHandler(crimfght_main_read);
	konamiSetlinesCallback(crimfght_set_lines);
	konamiClose();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(crimfght_sound_write);
	ZetSetReadHandler(crimfght_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 100.0);
	BurnYM2151SetPortHandler(&DrvYM2151WritePort);

	K007232Init(0, 3579545, DrvSndROM, 0x40000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback);

	K052109Init(DrvGfxROM0, 0x7ffff);
	K052109SetCallback(K052109Callback);

	K051960Init(DrvGfxROM1, 0xfffff);
	K051960SetCallback(K051960Callback);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	konamiExit();
	ZetExit();

	K007232Exit();
	BurnYM2151Exit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static int DrvDraw()
{
	if (DrvRecalc) {
		KonamiRecalcPal(DrvPalRAM, DrvPalette, 0x400);
	}

	K052109UpdateScroll();

	K052109RenderLayer(1, 1, DrvGfxROMExp0);
	K051960SpritesRender(DrvGfxROMExp1, 2); 
	K052109RenderLayer(2, 0, DrvGfxROMExp0);
	K051960SpritesRender(DrvGfxROMExp1, 1);
	K052109RenderLayer(0, 0, DrvGfxROMExp0);
	K051960SpritesRender(DrvGfxROMExp1, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 5);
		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}
	}

	konamiNewFrame();
	ZetNewFrame();

	int nSoundBufferPos = 0;
	int nInterleave = 100;
	int nCyclesTotal[2] = { (((3000000 / 60) * 133) / 100) /* 33% overclock */, 3579545 / 60 };
	int nCyclesDone[2] = { 0, 0 };

	ZetOpen(0);
	konamiOpen(0);

	for (int i = 0; i < nInterleave; i++)
	{
		int nSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);

		nCyclesDone[0] += konamiRun(nSegment - nCyclesDone[0]);

		nSegment = (nCyclesTotal[1] / nInterleave) * (i + 1);

		nCyclesDone[1] += ZetRun(nSegment - nCyclesDone[1]);

		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	if (pBurnSoundOut) {
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
		}
	}

	konamiClose();
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029704;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		konamiCpuScan(nAction, pnMin);
		ZetScan(nAction);

		BurnYM2151Scan(nAction);
		K007232Scan(nAction, pnMin);

		KonamiICScan(nAction);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		set_ram_bank(nDrvRamBank[0]);
		crimfght_set_lines(nDrvKonamiBank[0]);
		konamiClose();
	}

	return 0;
}


// Crime Fighters (US 4 players)

static struct BurnRomInfo crimfghtRomDesc[] = {
	{ "821l02.f24",	0x20000, 0x588e7da6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code

	{ "821l01.h4",	0x08000, 0x0faca89e, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "821k06.k13",	0x40000, 0xa1eadb24, 3 | BRF_GRA },           //  2 Background Tiles
	{ "821k07.k19",	0x40000, 0x060019fa, 3 | BRF_GRA },           //  3

	{ "821k04.k2",	0x80000, 0x00e0291b, 4 | BRF_GRA },           //  4 Sprites
	{ "821k05.k8",	0x80000, 0xe09ea05d, 4 | BRF_GRA },           //  5

	{ "821k03.e5",	0x40000, 0xfef8505a, 5 | BRF_SND },           //  6 K007232 Samples

	{ "821a08.i15",	0x00100, 0x7da55800, 6 | BRF_OPT },           //  7 Proms
};

STD_ROM_PICK(crimfght)
STD_ROM_FN(crimfght)

struct BurnDriver BurnDrvCrimfght = {
	"crimfght", NULL, NULL, "1989",
	"Crime Fighters (US 4 players)\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, crimfghtRomInfo, crimfghtRomName, CrimfghtInputInfo, CrimfghtDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	304, 224, 4, 3
};


// Crime Fighters (Japan 2 Players)

static struct BurnRomInfo crimfgtjRomDesc[] = {
	{ "821p02.bin",	0x20000, 0xf33fa2e1, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code

	{ "821l01.h4",	0x08000, 0x0faca89e, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "821k06.k13",	0x40000, 0xa1eadb24, 3 | BRF_GRA },           //  2 Background Tiles
	{ "821k07.k19",	0x40000, 0x060019fa, 3 | BRF_GRA },           //  3

	{ "821k04.k2",	0x80000, 0x00e0291b, 4 | BRF_GRA },           //  4 Sprites
	{ "821k05.k8",	0x80000, 0xe09ea05d, 4 | BRF_GRA },           //  5

	{ "821k03.e5",	0x40000, 0xfef8505a, 5 | BRF_SND },           //  6 K007232 Samples

	{ "821a08.i15",	0x00100, 0x7da55800, 6 | BRF_OPT },           //  7 Proms
};

STD_ROM_PICK(crimfgtj)
STD_ROM_FN(crimfgtj)

struct BurnDriver BurnDrvCrimfgtj = {
	"crimfgtj", "crimfght", NULL, "1989",
	"Crime Fighters (Japan 2 Players)\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, crimfgtjRomInfo, crimfgtjRomName, CrimfgtjInputInfo, CrimfgtjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	304, 224, 4, 3
};


// Crime Fighters (World 2 Players)

static struct BurnRomInfo crimfgt2RomDesc[] = {
	{ "crimefb.r02",0x20000, 0x4ecdd923, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code

	{ "821l01.h4",	0x08000, 0x0faca89e, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "821k06.k13",	0x40000, 0xa1eadb24, 3 | BRF_GRA },           //  2 Background Tiles
	{ "821k07.k19",	0x40000, 0x060019fa, 3 | BRF_GRA },           //  3

	{ "821k04.k2",	0x80000, 0x00e0291b, 4 | BRF_GRA },           //  4 Sprites
	{ "821k05.k8",	0x80000, 0xe09ea05d, 4 | BRF_GRA },           //  5

	{ "821k03.e5",	0x40000, 0xfef8505a, 5 | BRF_SND },           //  6 K007232 Samples

	{ "821a08.i15",	0x00100, 0x7da55800, 6 | BRF_OPT },           //  7 Proms
};

STD_ROM_PICK(crimfgt2)
STD_ROM_FN(crimfgt2)

struct BurnDriver BurnDrvCrimfgt2 = {
	"crimfgt2", "crimfght", NULL, "1989",
	"Crime Fighters (World 2 Players)\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, crimfgt2RomInfo, crimfgt2RomName, CrimfgtjInputInfo, CrimfgtjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	304, 224, 4, 3
};
