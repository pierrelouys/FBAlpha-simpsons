// FB Alpha Haunted Castle / Akuma-Jou Dracula driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "konami_intf.h"
#include "burn_ym3812.h"
#include "K051649.h"
#include "k007232.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvKonROM;
static unsigned char *nDrvKonBank;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvPalROM;
static unsigned char *DrvSndROM;
static unsigned char *DrvKonRAM0;
static unsigned char *DrvKonRAM1;
static unsigned char *DrvPalRAM;
static unsigned char *DrvPf1RAM;
static unsigned char *DrvPf2RAM;
static unsigned char *DrvSprRAM1;
static unsigned char *DrvSprRAM2;
static unsigned char *DrvSprBuf1;
static unsigned char *DrvSprBuf2;
static unsigned char *DrvPf1Ctrl;
static unsigned char *DrvPf2Ctrl;
static unsigned char *DrvZ80RAM;
static unsigned char *Palette;
static unsigned int *DrvPalette;
static unsigned char DrvRecalc;

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvDips[3];
static unsigned char DrvInputs[3];
static unsigned char DrvReset;

static unsigned char *soundlatch;
static unsigned char *gfxbank;

static int watchdog;

static struct BurnInputInfo HcastleInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Hcastle)

static struct BurnDIPInfo HcastleDIPList[]=
{
	{0x12, 0xff, 0xff, 0x53, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x04, 0x00, "Upright"		},
	{0x12, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x18, 0x18, "Easy"			},
	{0x12, 0x01, 0x18, 0x10, "Normal"		},
	{0x12, 0x01, 0x18, 0x08, "Hard"			},
	{0x12, 0x01, 0x18, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Damage"		},
	{0x12, 0x01, 0x60, 0x60, "Small"		},
	{0x12, 0x01, 0x60, 0x40, "Normal"		},
	{0x12, 0x01, 0x60, 0x20, "Big"			},
	{0x12, 0x01, 0x60, 0x00, "Biggest"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x13, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x13, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,   15, "Coin B"		},
	{0x13, 0x01, 0xf0, 0x20, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x50, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Upright Controls"	},
	{0x14, 0x01, 0x02, 0x02, "Single"		},
	{0x14, 0x01, 0x02, 0x00, "Dual"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x08, 0x00, "No"			},
	{0x14, 0x01, 0x08, 0x08, "Yes"			},
};

STDDIPINFO(Hcastle)

static void bankswitch(int data)
{
	*nDrvKonBank = data & 0x0f;
	int bankaddress = *nDrvKonBank * 0x2000;

	konamiMapMemory(DrvKonROM + 0x10000 + bankaddress, 0x6000, 0x7fff, KON_ROM);
}

static void playfield_write(int address, int data, unsigned char *ctrl, unsigned char *spr, unsigned char *buf)
{
	address &= 7;

	if (address == 3) { // buffer sprites
		memcpy (buf, spr + ((data & 8) << 8), 0x800);
	}

	ctrl[address & 7] = data;
}

void hcastle_write(unsigned short address, unsigned char data)
{
	if ((address & 0xfff8) == 0x0000) {
		playfield_write(address, data, DrvPf1Ctrl, DrvSprRAM1, DrvSprBuf1);
		return;
	}

	if ((address & 0xfff8) == 0x0200) {
		playfield_write(address, data, DrvPf2Ctrl, DrvSprRAM2, DrvSprBuf2);
		return;
	}

	switch (address)
	{
		case 0x0400:
			bankswitch(data);
		return;

		case 0x0404:
			*soundlatch = data;
		return;

		case 0x0408:
		{
			float t = konamiTotalCycles() * 1.19318167;
			t -= ZetTotalCycles();
			if (t > 1) ZetRun((int)t);

			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		}
		return;

		case 0x040c:
			watchdog = 0;
		return;

		case 0x0410:

		return;

		case 0x0418:
			*gfxbank = data;
		return;
	}
}

unsigned char hcastle_read(unsigned short address)
{
	switch (address)
	{
		case 0x0410:
		case 0x0411:
		case 0x0412:
			return DrvInputs[address & 3];

		case 0x0413:
			return DrvDips[2];

		case 0x0414:
			return DrvDips[1];

		case 0x0415:
			return DrvDips[0];

		case 0x0418:
			return *gfxbank;
	}

	return 0;
}

static void sound_bankswitch(int data)
{
	int bank_A=(data&0x3);
	int bank_B=((data>>2)&0x3);
	bank_A=bank_B;
	k007232_set_bank(0, bank_A, bank_B );
}

void __fastcall hcastle_sound_write(unsigned short address, unsigned char data)
{
	if ((address & 0xff80) == 0x9800) {
		K051649WaveformWrite(address & 0x7f, data);
		return;
	}

	if ((address & 0xfff0) == 0x9880) {
		address &= 0x000f;

		if (address <= 0x09) {
			K051649FrequencyWrite(address & 0x0f, data);
			return;
		}

		if (address == 0x0f) {
			K051649KeyonoffWrite(data);
			return;
		}

		if (address >= 0x0a) {
			K051649VolumeWrite(address - 0x988a, data);
		}

		return;
	}

	if (address >= 0xb000 && address <= 0xb00d) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}

	switch (address)
	{
		case 0xa000:
		case 0xa001:
			BurnYM3812Write(address & 1, data);
		return;

		case 0xc000:
			sound_bankswitch(data);
		return;
	}
}

unsigned char __fastcall hcastle_sound_read(unsigned short address)
{
	if (address >= 0xb000 && address <= 0xb00d) {
		return K007232ReadReg(0, address & 0x0f);
	}

	switch (address)
	{
		case 0xa000:
		case 0xa001:
			return BurnYM3812Read(address & 1);

		case 0xd000:
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

inline static int DrvSynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / 3579545;
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

	K051649Reset();
	BurnYM3812Reset();

	watchdog = 0;

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x030000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x200000;
	DrvGfxROM1		= Next; Next += 0x200000;

	DrvPalROM		= Next; Next += 0x000400;

	DrvSndROM		= Next; Next += 0x080000;

	Palette			= Next; Next += 0x001000;
	DrvPalette		= (unsigned int*)Next; Next += 0x1000 * sizeof(unsigned int);

	AllRam			= Next;

	DrvKonRAM0		= Next; Next += 0x000020;
	DrvKonRAM1		= Next; Next += 0x000020;
	DrvPalRAM		= Next; Next += 0x002000;
	DrvPf1RAM		= Next; Next += 0x001000;
	DrvPf2RAM		= Next; Next += 0x001000;
	DrvSprRAM1		= Next; Next += 0x001000;
	DrvSprRAM2		= Next; Next += 0x001000;
	DrvSprBuf1		= Next; Next += 0x000800;
	DrvSprBuf2		= Next; Next += 0x000800;

	DrvPf1Ctrl		= Next; Next += 0x000008;
	DrvPf2Ctrl		= Next; Next += 0x000008;

	DrvZ80RAM		= Next; Next += 0x000800;

	nDrvKonBank		= Next; Next += 0x000001;
	soundlatch		= Next; Next += 0x000001;
	gfxbank			= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane[4] = { 0x000, 0x001, 0x002, 0x003 };
	int XOffs[8] = { 0x008, 0x00c, 0x000, 0x004, 0x018, 0x01c, 0x010, 0x014 };
	int YOffs[8] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0 };

	unsigned char *tmp = (unsigned char*)malloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x100000);

	GfxDecode(0x8000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x100000);

	GfxDecode(0x8000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM1);

	free (tmp);

	return 0;
}

static void DrvPaletteInit()
{
	for (int chip = 0; chip < 2; chip++)
	{
		for (int pal = 0; pal < 8; pal++)
		{
			int clut = (chip << 1) | (pal & 1);

			for (int i = 0; i < 0x100; i++)
			{
				unsigned char ctabentry;

				if (((pal & 0x01) == 0) && (DrvPalROM[(clut << 8) | i] == 0))
					ctabentry = 0;
				else
					ctabentry = (pal << 4) | (DrvPalROM[(clut << 8) | i] & 0x0f);

				Palette[(chip << 11) | (pal << 8) | i] = ctabentry;
			}
		}
	}
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
		if (BurnLoadRom(DrvKonROM  + 0x00000,  0, 1)) return 1;
		if (BurnLoadRom(DrvKonROM  + 0x10000,  1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x00000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x80000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x80000,  6, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x00000,  7, 1)) return 1;

		if (BurnLoadRom(DrvPalROM  + 0x00000,  8, 1)) return 1;
		if (BurnLoadRom(DrvPalROM  + 0x00100,  9, 1)) return 1;
		if (BurnLoadRom(DrvPalROM  + 0x00200, 10, 1)) return 1;
		if (BurnLoadRom(DrvPalROM  + 0x00300, 11, 1)) return 1;

		DrvPaletteInit();
		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvKonRAM0,		0x0020, 0x003f, KON_RAM);
	konamiMapMemory(DrvKonRAM1,		0x0220, 0x023f, KON_RAM);
	konamiMapMemory(DrvPalRAM,		0x0600, 0x1fff, KON_RAM);
	konamiMapMemory(DrvPf1RAM,		0x2000, 0x2fff, KON_RAM);
	konamiMapMemory(DrvSprRAM1,		0x3000, 0x3fff, KON_RAM);
	konamiMapMemory(DrvPf2RAM,		0x4000, 0x4fff, KON_RAM);
	konamiMapMemory(DrvSprRAM2,		0x5000, 0x5fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x10000,	0x6000, 0x7fff, KON_ROM);
	konamiMapMemory(DrvKonROM,		0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(hcastle_write);
	konamiSetReadHandler(hcastle_read);
	konamiClose();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(hcastle_sound_write);
	ZetSetReadHandler(hcastle_sound_read);
	ZetClose();

	BurnYM3812Init(3579545, NULL, DrvSynchroniseStream, 0);
	BurnTimerAttachZet(3579545);

	K007232Init(0, 3579545, DrvSndROM, 0x80000); // no idea...
	K007232SetPortWriteHandler(0, DrvK007232VolCallback);

	K051649Init(3579545/2, 0.64);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	konamiExit();
	ZetExit();

	K007232Exit();
	BurnYM3812Exit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static void draw_layer(unsigned char *ram, unsigned char *ctrl, unsigned char *gfx, int colbase, int base, int t)
{
	int bit0 = ((ctrl[0x05] >> 0) & 0x03) + 2;
	int bit1 = ((ctrl[0x05] >> 2) & 0x03) + 1;
	int bit2 = ((ctrl[0x05] >> 4) & 0x03) + 0;
	int bit3 = ((ctrl[0x05] >> 6) & 0x03) - 1;
	int col  = ((ctrl[0x06] << 1) & 0x60) | colbase;

	if (ctrl[0x03] & 0x01) base += 0x2000;

	int scrollx = ((ctrl[0x01] << 8) | ctrl[0x00]) & 0x1ff;
	int scrolly = ctrl[0x02] & 0xff;

	int tilemap_flip = ctrl[0x07] & 0x08; 

	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = (offs >> 6) << 3;

		sx -= scrollx;
		if (sx < -7) sx += 0x200;
		sy -= scrolly;
		if (sy < -7) sy += 0x100;

	//	if (sx >= nScreenWidth || sy < 9 || sy > 240) continue;

		int ofst = (offs & 0x1f) + ((offs >> 1) & 0x3e0) + ((offs & 0x20) << 6);

		int attr  = ram[ofst];
		int code  = ram[ofst + 0x400];
		int color = (attr & 0x07) | col;
		int bank  = ((attr & 0x80) >> 7) | ((attr >> bit0) & 0x02) | ((attr >> bit1) & 0x04) | ((attr >> bit2) & 0x08) | ((attr >> bit3) & 0x10);

		code += (bank << 8) + base;

		if (tilemap_flip) {
			sx = 0xf8 - sx;
			sy = 0xf8 - sy;

			if (t) {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy-16, color, 4, 0, 0, gfx);
			} else {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy-16, color, 4, 0, gfx);
			}
		} else {
			if (t) {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy-16, color, 4, 0, 0, gfx);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy-16, color, 4, 0, gfx);
			}
		}
	}
}

static void draw_sprites(int bank, unsigned char *source, unsigned char *ctrl, unsigned char *gfx, int col)
{
	int bank_base = (bank == 0) ? 0x4000 * (*gfxbank & 1) : 0;

	int base_color = ((ctrl[6] & 0x30) << 1) + col;
	int flipscreen =   ctrl[7] & 0x08;

	static const int x_offset[4] = { 0x00, 0x01, 0x04, 0x05 };
	static const int y_offset[4] = { 0x00, 0x02, 0x08, 0x0a };

	for (int i = 0; i < 0x40; i++)
	{
		int number = source[0];
		int sbank  = source[1] & 0x0f;
		int sy     = source[2];
		int sx     = source[3];
		int attr   = source[4];
		int xflip  = source[4] & 0x10;
		int yflip  = source[4] & 0x20;
		int color  = base_color + ((source[1] & 0xf0) >> 4);
		int width, height;

		if (attr & 0x01) sx -= 256;
		if (sy >= 240) sy -= 256;

		number += ((sbank & 3) << 8) + ((attr & 0xc0) << 4);
		number = (number << 2) | ((sbank >> 2) & 3);
		number += bank_base;

		switch (attr & 0x0e)
		{
			case 0x06: width = 1; height = 1; break;
			case 0x04: width = 1; height = 2; number &= (~2); break;
			case 0x02: width = 2; height = 1; number &= (~1); break;
			case 0x00: width = 2; height = 2; number &= (~3); break;
			case 0x08: width = 4; height = 4; number &= (~3); break;
			default:   width = 1; height = 1;
		}

		for (int y = 0; y < height; y++)
		{
			int yy = sy + y * 8;
			int ey = yflip ? (height-1-y) : y;

			for (int x = 0; x < width; x++)
			{
				int ex = xflip ? (width-1-x) : x;
				int xx = sx + x * 8;

				int code = number + x_offset[ex] + y_offset[ey];

				if (flipscreen) {
					if (yflip ^ 0x20) {
						if (xflip ^ 0x10) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, 248-xx, (248-yy)-16, color, 4, 0, 0, gfx);
						} else {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, 248-xx, (248-yy)-16, color, 4, 0, 0, gfx);
						}
					} else {
						if (xflip ^ 0x10) {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, 248-xx, (248-yy)-16, color, 4, 0, 0, gfx);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, code, 248-xx, (248-yy)-16, color, 4, 0, 0, gfx);
						}
					}
				} else {
					if (yflip) {
						if (xflip) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, xx, yy-16, color, 4, 0, 0, gfx);
						} else {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, xx, yy-16, color, 4, 0, 0, gfx);
						}
					} else {
						if (xflip) {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, xx, yy-16, color, 4, 0, 0, gfx);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, code, xx, yy-16, color, 4, 0, 0, gfx);
						}
					}
				}
			}
		}

		source += 5;
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		unsigned char r,g,b;
		unsigned int tmp[0x80];

		for (int i = 0; i < 0x100; i+=2) {
			unsigned short d = DrvPalRAM[i + 1] | (DrvPalRAM[i] << 8);
			r = (d >>  0) & 0x1f;
			g = (d >>  5) & 0x1f;
			b = (d >> 10) & 0x1f;

			r = (r << 3) | (r >> 2);
			g = (g << 3) | (g >> 2);
			b = (b << 3) | (b >> 2);

			tmp[i/2] = BurnHighCol(r, g, b, 0);
		}

		for (int i = 0; i < 0x1000; i++) {
			DrvPalette[i] = tmp[Palette[i]];
		}
	}

	if ((*gfxbank & 0x04) == 0)
	{
		draw_layer(DrvPf2RAM, DrvPf2Ctrl, DrvGfxROM1, 0x90, ((*gfxbank & 2) >> 1) * 0x4000, 0);
		draw_sprites(0, DrvSprBuf1, DrvPf1Ctrl, DrvGfxROM0, 0x00);
		draw_sprites(1, DrvSprBuf2, DrvPf2Ctrl, DrvGfxROM1, 0x80);
		draw_layer(DrvPf1RAM, DrvPf1Ctrl, DrvGfxROM0, 0x10, 0x0000, 1);
	}
	else
	{
		draw_layer(DrvPf2RAM, DrvPf2Ctrl, DrvGfxROM1, 0x90, ((*gfxbank & 2) >> 1) * 0x4000, 0);
		draw_layer(DrvPf1RAM, DrvPf1Ctrl, DrvGfxROM0, 0x10, 0x0000, 1);
		draw_sprites(0, DrvSprBuf1, DrvPf1Ctrl, DrvGfxROM0, 0x00);
		draw_sprites(1, DrvSprBuf2, DrvPf2Ctrl, DrvGfxROM1, 0x80);
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	if (watchdog++ == 60) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3);
		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// Clear opposites
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[2] & 0x03) == 0) DrvInputs[2] |= 0x03;
		if ((DrvInputs[2] & 0x0c) == 0) DrvInputs[2] |= 0x0c;
	}

	konamiNewFrame();
	ZetNewFrame();

	int nCyclesTotal[2] = { 3000000 / 60, 3579545 / 60 };

	ZetOpen(0);
	konamiOpen(0);

	konamiRun(nCyclesTotal[0]);
	konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	BurnTimerEndFrame(nCyclesTotal[1] - ZetTotalCycles());

	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		K007232Update(0, pBurnSoundOut, nBurnSoundLen);
		K051649Update(pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		konamiCpuScan(nAction, pnMin);
		ZetScan(nAction);

		BurnYM3812Scan(nAction, pnMin);
		K007232Scan(nAction, pnMin);
		K051649Scan(nAction, pnMin);

		SCAN_VAR(watchdog);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		bankswitch(*nDrvKonBank);
		konamiClose();
	}

	return 0;
}


// Haunted Castle (version M)

static struct BurnRomInfo hcastleRomDesc[] = {
	{ "m03.k12",	0x08000, 0xd85e743d, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "b06.k8",	0x20000, 0xabd07866, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "768.e01",	0x08000, 0xb9fff184, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "768c09.g21",	0x80000, 0xe3be3fdd, 3 | BRF_GRA }, 	      //  3 Bank #0 Tiles
	{ "768c08.g19",	0x80000, 0x9633db8b, 3 | BRF_GRA },           //  4

	{ "768c04.j5",	0x80000, 0x2960680e, 4 | BRF_GRA },           //  5 Bank #1 Tiles
	{ "768c05.j6",	0x80000, 0x65a2f227, 4 | BRF_GRA },           //  6

	{ "768c07.e17",	0x80000, 0x01f9889c, 5 | BRF_SND },           //  7 K007232 Samples

	{ "768c13.j21",	0x00100, 0xf5de80cb, 6 | BRF_GRA },           //  8 Color Proms
	{ "768c14.j22",	0x00100, 0xb32071b7, 6 | BRF_GRA },           //  9
	{ "768c11.i4",	0x00100, 0xf5de80cb, 6 | BRF_GRA },           // 10
	{ "768c10.i3",	0x00100, 0xb32071b7, 6 | BRF_GRA },           // 11

	{ "768b12.d20",	0x00100, 0x362544b8, 0 | BRF_GRA | BRF_OPT }, // 12 Priority Prom
};

STD_ROM_PICK(hcastle)
STD_ROM_FN(hcastle)

struct BurnDriverD BurnDrvHcastle = {
	"hcastle", NULL, NULL, "1988",
	"Haunted Castle (version M)\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, hcastleRomInfo, hcastleRomName, HcastleInputInfo, HcastleDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	256, 224, 4, 3
};


// Haunted Castle (version K)

static struct BurnRomInfo hcastleoRomDesc[] = {
	{ "768.k03",	0x08000, 0x40ce4f38, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "768.g06",	0x20000, 0xcdade920, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "768.e01",	0x08000, 0xb9fff184, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "768c09.g21",	0x80000, 0xe3be3fdd, 3 | BRF_GRA }, 	      //  3 Bank #0 Tiles
	{ "768c08.g19",	0x80000, 0x9633db8b, 3 | BRF_GRA },           //  4

	{ "768c04.j5",	0x80000, 0x2960680e, 4 | BRF_GRA },           //  5 Bank #1 Tiles
	{ "768c05.j6",	0x80000, 0x65a2f227, 4 | BRF_GRA },           //  6

	{ "768c07.e17",	0x80000, 0x01f9889c, 5 | BRF_SND },           //  7 K007232 Samples

	{ "768c13.j21",	0x00100, 0xf5de80cb, 6 | BRF_GRA },           //  8 Color Proms
	{ "768c14.j22",	0x00100, 0xb32071b7, 6 | BRF_GRA },           //  9
	{ "768c11.i4",	0x00100, 0xf5de80cb, 6 | BRF_GRA },           // 10
	{ "768c10.i3",	0x00100, 0xb32071b7, 6 | BRF_GRA },           // 11

	{ "768b12.d20",	0x00100, 0x362544b8, 0 | BRF_GRA | BRF_OPT }, // 12 Priority Prom
};

STD_ROM_PICK(hcastleo)
STD_ROM_FN(hcastleo)

struct BurnDriverD BurnDrvHcastleo = {
	"hcastleo", "hcastle", NULL, "1988",
	"Haunted Castle (version K)\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, hcastleoRomInfo, hcastleoRomName, HcastleInputInfo, HcastleDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	256, 224, 4, 3
};


// Akuma-Jou Dracula (Japan version P)

static struct BurnRomInfo hcastlejRomDesc[] = {
	{ "768p03.k12",	0x08000, 0xd509e340, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "768j06.k8",	0x20000, 0x42283c3e, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "768.e01",	0x08000, 0xb9fff184, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "768c09.g21",	0x80000, 0xe3be3fdd, 3 | BRF_GRA }, 	      //  3 Bank #0 Tiles
	{ "768c08.g19",	0x80000, 0x9633db8b, 3 | BRF_GRA },           //  4

	{ "768c04.j5",	0x80000, 0x2960680e, 4 | BRF_GRA },           //  5 Bank #1 Tiles
	{ "768c05.j6",	0x80000, 0x65a2f227, 4 | BRF_GRA },           //  6

	{ "768c07.e17",	0x80000, 0x01f9889c, 5 | BRF_SND },           //  7 K007232 Samples

	{ "768c13.j21",	0x00100, 0xf5de80cb, 6 | BRF_GRA },           //  8 Color Proms
	{ "768c14.j22",	0x00100, 0xb32071b7, 6 | BRF_GRA },           //  9
	{ "768c11.i4",	0x00100, 0xf5de80cb, 6 | BRF_GRA },           // 10
	{ "768c10.i3",	0x00100, 0xb32071b7, 6 | BRF_GRA },           // 11

	{ "768b12.d20",	0x00100, 0x362544b8, 0 | BRF_GRA | BRF_OPT }, // 12 Priority Prom
};

STD_ROM_PICK(hcastlej)
STD_ROM_FN(hcastlej)

struct BurnDriverD BurnDrvHcastlej = {
	"hcastlej", "hcastle", NULL, "1988",
	"Akuma-Jou Dracula (Japan version P)\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, hcastlejRomInfo, hcastlejRomName, HcastleInputInfo, HcastleDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	256, 224, 4, 3
};


// Akuma-Jou Dracula (Japan version N)

static struct BurnRomInfo hcastljoRomDesc[] = {
	{ "768n03.k12",	0x08000, 0x3e4dca2a, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "768j06.k8",	0x20000, 0x42283c3e, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "768.e01",	0x08000, 0xb9fff184, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "768c09.g21",	0x80000, 0xe3be3fdd, 3 | BRF_GRA }, 	      //  3 Bank #0 Tiles
	{ "768c08.g19",	0x80000, 0x9633db8b, 3 | BRF_GRA },           //  4

	{ "768c04.j5",	0x80000, 0x2960680e, 4 | BRF_GRA },           //  5 Bank #1 Tiles
	{ "768c05.j6",	0x80000, 0x65a2f227, 4 | BRF_GRA },           //  6

	{ "768c07.e17",	0x80000, 0x01f9889c, 5 | BRF_SND },           //  7 K007232 Samples

	{ "768c13.j21",	0x00100, 0xf5de80cb, 6 | BRF_GRA },           //  8 Color Proms
	{ "768c14.j22",	0x00100, 0xb32071b7, 6 | BRF_GRA },           //  9
	{ "768c11.i4",	0x00100, 0xf5de80cb, 6 | BRF_GRA },           // 10
	{ "768c10.i3",	0x00100, 0xb32071b7, 6 | BRF_GRA },           // 11

	{ "768b12.d20",	0x00100, 0x362544b8, 0 | BRF_GRA | BRF_OPT }, // 12 Priority Prom
};

STD_ROM_PICK(hcastljo)
STD_ROM_FN(hcastljo)

struct BurnDriverD BurnDrvHcastljo = {
	"hcastljo", "hcastle", NULL, "1988",
	"Akuma-Jou Dracula (Japan version N)\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, hcastljoRomInfo, hcastljoRomName, HcastleInputInfo, HcastleDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	256, 224, 4, 3
};
