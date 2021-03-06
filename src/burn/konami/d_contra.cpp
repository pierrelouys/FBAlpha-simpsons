// FB Alpha Contra driver module
// Based on MAME driver by Carlos A. Lozano, Phil Stroffolino, Jose T. Gomez, and Eric Hustvedt

#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "m6809_intf.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvM6809ROM0;
static unsigned char *DrvM6809ROM1;
static unsigned char *DrvM6809RAM0;
static unsigned char *DrvM6809RAM1;
static unsigned char *DrvM6809RAM2;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvPROMs;
static unsigned char *DrvColTable;
static unsigned char *DrvPalRAM;
static unsigned char *DrvFgCRAM;
static unsigned char *DrvFgVRAM;
static unsigned char *DrvTxCRAM;
static unsigned char *DrvTxVRAM;
static unsigned char *DrvBgCRAM;
static unsigned char *DrvBgVRAM;
static unsigned char *DrvSprRAM;
static unsigned int  *DrvPalette;
static unsigned int  *Palette;
static unsigned char  DrvRecalc;
static unsigned char *pDrvSprRAM0;
static unsigned char *pDrvSprRAM1;

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvInputs[3];
static unsigned char DrvDip[3];
static unsigned char DrvReset;

static unsigned char trigger_sound_irq;

static unsigned char soundlatch;
static unsigned char nBankData;

static unsigned char K007121_ctrlram[2][8];
static int K007121_flipscreen[2];

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvJoy1 + 0, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL  , DrvJoy1 + 1, "p2 coin"   },

	{"Start 1"           , BIT_DIGITAL  , DrvJoy1 + 3, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvJoy1 + 4, "p2 start"  },

	{"P1 Left"           , BIT_DIGITAL  , DrvJoy2 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvJoy2 + 1, "p1 right"  },
	{"P1 Up"             , BIT_DIGITAL  , DrvJoy2 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvJoy2 + 3, "p1 down"   },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvJoy2 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvJoy2 + 5, "p1 fire 2" },

	{"P2 Left"           , BIT_DIGITAL  , DrvJoy3 + 0, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvJoy3 + 1, "p2 right"  },
	{"P2 Up"             , BIT_DIGITAL  , DrvJoy3 + 2, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvJoy3 + 3, "p2 down"   },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvJoy3 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvJoy3 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset,   "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvJoy1 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0,  "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1,  "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2,  "dip"       },
};

STDINPUTINFO(Drv)


static struct BurnDIPInfo DrvDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL },
	{0x13, 0xff, 0xff, 0xff, NULL },
	{0x14, 0xff, 0xff, 0xff, NULL },

	{0x12, 0xfe,    0,   16, "Coin A"  },
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits" },
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits" },
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"  },
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits" },
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits" },
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits" },
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits" },
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits" },
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits" },
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits" },
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits" },
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits" },
	{0x12, 0x01, 0x0f, 0x00, "Free Play"         },

	{0x12, 0xfe,    0,   15, "Coin B"  },
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit"  },
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit"  },
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit"  },
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits" },
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits" },
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"  },
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits" },
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits" },
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits" },
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits" },
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits" },
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits" },
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits" },
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits" },
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits" },

	{0x13, 0xfe,    0,    4, "Lives"  },
	{0x13, 0x01, 0x03, 0x03, "2" },
	{0x13, 0x01, 0x03, 0x02, "3" },
	{0x13, 0x01, 0x03, 0x01, "5" },
	{0x13, 0x01, 0x03, 0x00, "7" },

	{0x13, 0xfe,    0,    2, "Cabinet"  },
	{0x13, 0x01, 0x04, 0x00, "Upright" },
	{0x13, 0x01, 0x04, 0x04, "Cocktail" },

	{0x13, 0xfe,    0,    4, "Bonus Life"  },
	{0x13, 0x01, 0x18, 0x18, "30000 70000" },
	{0x13, 0x01, 0x18, 0x10, "40000 80000" },
	{0x13, 0x01, 0x18, 0x08, "40000" },
	{0x13, 0x01, 0x18, 0x00, "50000" },

	{0x13, 0xfe,    0,    4, "Difficulty"  },
	{0x13, 0x01, 0x60, 0x60, "Easy" },
	{0x13, 0x01, 0x60, 0x40, "Normal" },
	{0x13, 0x01, 0x60, 0x20, "Hard" },
	{0x13, 0x01, 0x60, 0x00, "Hardest" },

	{0x13, 0xfe,    0,    2, "Demo Sounds"  },
	{0x13, 0x01, 0x80, 0x80, "Off" },
	{0x13, 0x01, 0x80, 0x00, "On" },

	{0x14, 0xfe,    0,    2, "Flip Screen"  },
	{0x14, 0x01, 0x01, 0x01, "Off" },
	{0x14, 0x01, 0x01, 0x00, "On" },

	{0x14, 0xfe,    0,    2, "Service Mode" },
	{0x14, 0x01, 0x04, 0x00, "Off" },
	{0x14, 0x01, 0x04, 0x04, "On" },

	{0x14, 0xfe,    0,    2, "Upright Controls"  },
	{0x14, 0x01, 0x02, 0x02, "Single" },
	{0x14, 0x01, 0x02, 0x00, "Dual" },

	{0x14, 0xfe,    0,    2, "Sound"  },
	{0x14, 0x01, 0x08, 0x00, "Mono" },
	{0x14, 0x01, 0x08, 0x08, "Stereo" },
};

STDDIPINFO(Drv)

static void K007121_ctrl_w(int chip, int offset, int data)
{
	if (offset == 7) K007121_flipscreen[chip] = data & 0x08;

	K007121_ctrlram[chip][offset] = data;
}

static void contra_K007121_ctrl_0_w(int offset, int data)
{
	if (offset == 3)
	{
		if (data & 0x08)
			memcpy (pDrvSprRAM0, DrvSprRAM + 0x000, 0x800);
		else
			memcpy (pDrvSprRAM0, DrvSprRAM + 0x800, 0x800);
	}

	K007121_ctrl_w(0,offset,data);
}

static void contra_K007121_ctrl_1_w(int offset, int data)
{
	if (offset == 3)
	{
		if (data&0x8)
			memcpy(pDrvSprRAM1, DrvM6809RAM1 + 0x0800, 0x800);
		else
			memcpy(pDrvSprRAM1, DrvM6809RAM1 + 0x1000, 0x800);
	}

	K007121_ctrl_w(1,offset,data);
}

void contra_bankswitch_w(int data)
{
	nBankData = data & 0x0f;
	int bankaddress = 0x10000 + nBankData * 0x2000;

	if (bankaddress < 0x28000)
		M6809MapMemory(DrvM6809ROM0 + bankaddress, 0x6000, 0x7fff, M6809_ROM);
}

unsigned char DrvContraM6809ReadByte(unsigned short address)
{
	switch (address)
	{
		case 0x0010:
		case 0x0011:
		case 0x0012:
			return DrvInputs[address & 3];

		case 0x0014:
		case 0x0015:
		case 0x0016:
			return DrvDip[address & 3];
	}

	return 0;
}

void DrvContraM6809WriteByte(unsigned short address, unsigned char data)
{
	if ((address & 0xff00) == 0x0c00) {
		int offset = address & 0xff;

		DrvPalRAM[offset] = data;

		unsigned short col = DrvPalRAM[offset & ~1] | (DrvPalRAM[offset | 1] << 8);

		unsigned char r, g, b;

		r = (col >> 0) & 0x1f;
		r = (r << 3) | (r >> 2);

		g = (col >> 5) & 0x1f;
		g = (g << 3) | (g >> 2);

		b = (col >> 10) & 0x1f;
		b = (b << 3) | (b >> 2);

		unsigned int color = (r << 16) | (g << 8) | b;

		DrvRecalc = 1;
		Palette[offset >> 1] = color;

		return;
	}

	switch (address)
	{
		case 0x0000:
		case 0x0001:
		case 0x0002:
		case 0x0003:
		case 0x0004:
		case 0x0005:
		case 0x0006:
		case 0x0007:
			contra_K007121_ctrl_0_w(address & 7, data);
		return;

		case 0x0018:
			// coin counter
		return;

		case 0x001a:
			trigger_sound_irq = 1;
		return;

		case 0x001c:
			soundlatch = data;
		return;

		case 0x0060:
		case 0x0061:
		case 0x0062:
		case 0x0063:
		case 0x0064:
		case 0x0065:
		case 0x0066:
		case 0x0067:
			contra_K007121_ctrl_1_w(address & 7, data);
		return;

		case 0x7000:
			contra_bankswitch_w(data);
		return;
	}
}

unsigned char DrvContraM6809SoundReadByte(unsigned short address)
{
	switch (address)
	{
		case 0x0000:
			return soundlatch;

		case 0x2001:
			return BurnYM2151ReadStatus();
	}

	return 0;
}

void DrvContraM6809SoundWriteByte(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x2000:
			BurnYM2151SelectRegister(data);
		return;

		case 0x2001:
			BurnYM2151WriteRegister(data);
		return;
	}
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvM6809ROM0	= Next; Next += 0x030000;
	DrvM6809ROM1	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x100000;
	DrvGfxROM1	= Next; Next += 0x100000;

	DrvPROMs	= Next; Next += 0x000400;

	DrvColTable	= Next; Next += 0x001000;

	DrvPalette	= (unsigned int*)Next; Next += 0x01000 * sizeof(int);

	AllRam		= Next;

	DrvM6809RAM0	= Next; Next += 0x001000;
	DrvM6809RAM1	= Next; Next += 0x001800;
	DrvM6809RAM2	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x000100;
	DrvFgCRAM	= Next; Next += 0x000400;
	DrvFgVRAM	= Next; Next += 0x000400;
	DrvTxCRAM	= Next; Next += 0x000400;
	DrvTxVRAM	= Next; Next += 0x000400;
	DrvBgCRAM	= Next; Next += 0x000400;
	DrvBgVRAM	= Next; Next += 0x000400;
	DrvSprRAM	= Next; Next += 0x001000;

	pDrvSprRAM0	= Next; Next += 0x000800;
	pDrvSprRAM1	= Next; Next += 0x000800;

	Palette		= (unsigned int*)Next; Next += 0x00080 * sizeof(int);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static int DrvGfxDecode(unsigned char *src)
{
	int Plane[4] = { 0,  1,  2,  3 };
	int XOffs[8] = { 0,  4,  8, 12, 16, 20, 24, 28 };
	int YOffs[8] = { 0, 32, 64, 96, 128, 160, 192, 224 };

	unsigned char *tmp = (unsigned char*)malloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, src, 0x80000);

	GfxDecode(0x4000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, tmp, src);

	free (tmp);

	return 0;
}

static int DrvColorTableInit()
{
	for (int chip = 0; chip < 2; chip++)
	{
		for (int pal = 0; pal < 8; pal++)
		{
			int clut = ((chip << 1) | (pal & 1)) << 8;

			for (int i = 0; i < 0x100; i++)
			{
				unsigned char ctabentry;

				if (((pal & 0x01) == 0) && (DrvPROMs[clut | i] == 0))
					ctabentry = 0;
				else
					ctabentry = (pal << 4) | (DrvPROMs[clut | i] & 0x0f);

				DrvColTable[(chip << 11) | (pal << 8) | i] = ctabentry;
			}
		}
	}

	return 0;
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	memset (K007121_ctrlram, 0, 2 * 8);
	memset (K007121_flipscreen, 0, 2 * sizeof(int));

	for (int i = 0; i < 2; i++) {
		M6809Open(i);
		M6809Reset();
		M6809Close();
	}

	BurnYM2151Reset();

	trigger_sound_irq = 0;
	soundlatch = 0;
	nBankData = 0;

	return 0;
}

static void DrvYM2151IrqHandler(int Irq)
{
	if (Irq) {
		M6809SetIRQ(M6809_FIRQ_LINE, M6809_IRQSTATUS_ACK);
	} else {
		M6809SetIRQ(M6809_FIRQ_LINE, M6809_IRQSTATUS_NONE);
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
		if (BurnLoadRom(DrvM6809ROM0 + 0x00000, 0, 1)) return 1;
		memcpy (DrvM6809ROM0 + 0x20000, DrvM6809ROM0, 0x08000);
		if (BurnLoadRom(DrvM6809ROM0 + 0x10000, 1, 1)) return 1;

		if (BurnLoadRom(DrvM6809ROM1 + 0x08000, 2, 1)) return 1;

		if (BurnDrvGetFlags() & BDF_BOOTLEG)
		{
			for (int i = 0; i < 8; i++) {
				if (BurnLoadRom(DrvGfxROM0 + i * 0x10000, 3  + i, 1)) return 1;
				if (BurnLoadRom(DrvGfxROM1 + i * 0x10000, 11 + i, 1)) return 1;
			}

			for (int i = 0; i < 4; i++) {
				if (BurnLoadRom(DrvPROMs + i * 0x100, i + 18, 1)) return 1;
			}
		} else {
			if (BurnLoadRom(DrvGfxROM0 + 0, 3, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 1, 4, 2)) return 1;

			if (BurnLoadRom(DrvGfxROM1 + 0, 5, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 1, 6, 2)) return 1;

			for (int i = 0; i < 4; i++) {
				if (BurnLoadRom(DrvPROMs + i * 0x100, i + 7, 1)) return 1;
			}
		}

		DrvGfxDecode(DrvGfxROM0);
		DrvGfxDecode(DrvGfxROM1);

		DrvColorTableInit();
	}

	M6809Init(2);
	M6809Open(0);
	M6809MapMemory(DrvPalRAM,		0x0c00, 0x0cff, M6809_ROM);
	M6809MapMemory(DrvM6809RAM0,		0x1000, 0x1fff, M6809_RAM);
	M6809MapMemory(DrvFgCRAM,		0x2000, 0x23ff, M6809_RAM);
	M6809MapMemory(DrvFgVRAM,		0x2400, 0x27ff, M6809_RAM);
	M6809MapMemory(DrvTxCRAM,		0x2800, 0x2bff, M6809_RAM);
	M6809MapMemory(DrvTxVRAM,		0x2c00, 0x2fff, M6809_RAM);
	M6809MapMemory(DrvSprRAM,		0x3000, 0x3fff, M6809_RAM);
	M6809MapMemory(DrvBgCRAM,		0x4000, 0x43ff, M6809_RAM);
	M6809MapMemory(DrvBgVRAM,		0x4400, 0x47ff, M6809_RAM);
	M6809MapMemory(DrvM6809RAM1,		0x4800, 0x5fff, M6809_RAM);
//	M6809MapMemory(DrvM6809ROM0 + 0x10000, 	0x6000, 0x7fff, M6809_ROM);
	M6809MapMemory(DrvM6809ROM0 + 0x08000,	0x8000, 0xffff, M6809_ROM);
	M6809SetReadByteHandler(DrvContraM6809ReadByte);
	M6809SetWriteByteHandler(DrvContraM6809WriteByte);
	M6809Close();

	M6809Open(1);
	M6809MapMemory(DrvM6809RAM2, 		0x6000, 0x67ff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM1 + 0x08000,	0x8000, 0xffff, M6809_ROM);
	M6809SetReadByteHandler(DrvContraM6809SoundReadByte);
	M6809SetWriteByteHandler(DrvContraM6809SoundWriteByte);
	M6809Close();

	BurnYM2151Init(3579545, 60.0);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	M6809Exit();
	BurnYM2151Exit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static void draw_bg()
{
	int bit0 = (K007121_ctrlram[1][0x05] >> 0) & 0x03;
	int bit1 = (K007121_ctrlram[1][0x05] >> 2) & 0x03;
	int bit2 = (K007121_ctrlram[1][0x05] >> 4) & 0x03;
	int bit3 = (K007121_ctrlram[1][0x05] >> 6) & 0x03;
	int mask = (K007121_ctrlram[1][0x04] & 0xf0) >> 4;
	int scrollx = K007121_ctrlram[1][0x00] - 40;
	int scrolly = K007121_ctrlram[1][0x02];
	int flipscreen = K007121_flipscreen[1];

	for (int offs = 0; offs < 0x400; offs++)
	{
		int sx = (offs & 0x1f) << 3;
		int sy = (offs >> 5) << 3;

		sx -= scrollx;
		sy -= scrolly;
		if (sx < -7) sx += 296;
		if (sy < -7) sy += 256;
		sy -= 16;

		int attr = DrvBgCRAM[offs];

		int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((K007121_ctrlram[1][0x03] & 0x01) << 5);

		bank = (bank & ~(mask << 1)) | ((K007121_ctrlram[0][0x04] & mask) << 1);

		int color = ((K007121_ctrlram[1][6]&0x30)*2+16)+(attr&7);

		int code = DrvBgVRAM[offs] | (bank << 8);

		if (flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, (280 - sx)-8, 224 - sy, color, 4, 0x800, DrvGfxROM1);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0x800, DrvGfxROM1);
		}
	}
}

static void draw_fg()
{
	int bit0 = (K007121_ctrlram[0][0x05] >> 0) & 0x03;
	int bit1 = (K007121_ctrlram[0][0x05] >> 2) & 0x03;
	int bit2 = (K007121_ctrlram[0][0x05] >> 4) & 0x03;
	int bit3 = (K007121_ctrlram[0][0x05] >> 6) & 0x03;
	int mask = (K007121_ctrlram[0][0x04] & 0xf0) >> 4;
	int scrollx = K007121_ctrlram[0][0x00] - 40;
	int scrolly = K007121_ctrlram[0][0x02];
	int flipscreen = K007121_flipscreen[0];

	for (int offs = 0; offs < 0x400; offs++)
	{
		int sx = (offs & 0x1f) << 3;
		int sy = (offs >> 5) << 3;

		sx -= scrollx;
		sy -= scrolly;
		if (sx < -7) sx += 296;
		if (sy < -7) sy += 256;

		sy -= 16;

		int attr = DrvFgCRAM[offs];

		int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((K007121_ctrlram[0][0x03] & 0x01) << 5);

		bank = (bank & ~(mask << 1)) | ((K007121_ctrlram[0][0x04] & mask) << 1);

		int color = ((K007121_ctrlram[0][6]&0x30)*2+16)+(attr&7);

		int code = DrvFgVRAM[offs] | (bank << 8);

		if (flipscreen) {
			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, (280 - sx)-8, 224 - sy, color, 4, 0, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
		}
	}
}

static void draw_tx()
{
	int bit0 = (K007121_ctrlram[0][0x05] >> 0) & 0x03;
	int bit1 = (K007121_ctrlram[0][0x05] >> 2) & 0x03;
	int bit2 = (K007121_ctrlram[0][0x05] >> 4) & 0x03;
	int bit3 = (K007121_ctrlram[0][0x05] >> 6) & 0x03;
	int flipscreen = K007121_flipscreen[0];

	for (int offs = 0x40; offs < 0x3c0; offs++)
	{
		int sx = (offs & 0x1f) << 3;
		if (sx > 39) continue;
		int sy = (offs >> 5) << 3;

		int attr = DrvTxCRAM[offs];

		int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10);

		int color = ((K007121_ctrlram[0][6]&0x30)*2+16)+(attr&7);

		int code = DrvTxVRAM[offs] | (bank << 8);

		if (flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, (sx ^ 0xf8) + 24, (sy ^ 0xf8) - 16, color, 4, 0, DrvGfxROM0);
		} else {
			Render8x8Tile(pTransDraw, code, sx, sy - 16, color, 4, 0, DrvGfxROM0);
		}
	}
}


static void K007121_sprites_draw(int chip, unsigned char *gfx_base, unsigned char *ctable,
			const unsigned char *source, int base_color,
			int global_x_offset, int global_y_offset,
			int bank_base, int pri_mask, int color_offset)
{
	int flipscreen = K007121_flipscreen[chip];
	int i,num,inc,offs[5],trans;
	int is_flakatck = (ctable == NULL);

	if (is_flakatck)
	{
		num = 0x40;
		inc = -0x20;
		source += 0x3f << 5;
		offs[0] = 0x0e;
		offs[1] = 0x0f;
		offs[2] = 0x06;
		offs[3] = 0x04;
		offs[4] = 0x08;
		trans = 0;
	}
	else
	{
		num = 0x40;

		inc = 5;
		offs[0] = 0x00;
		offs[1] = 0x01;
		offs[2] = 0x02;
		offs[3] = 0x03;
		offs[4] = 0x04;
		trans = 0;

		if (pri_mask != -1)
		{
			source += (num-1)*inc;
			inc = -inc;
		}
	}

	for (i = 0;i < num;i++)
	{
		int number = source[offs[0]];
		int sprite_bank = source[offs[1]] & 0x0f;
		int sx = source[offs[3]];
		int sy = source[offs[2]];
		int attr = source[offs[4]];
		int color = base_color + ((source[offs[1]] & 0xf0) >> 4);
		int xflip = attr & 0x10;
		int yflip = attr & 0x20;
		int width,height;
		int transparent_color = 0;
		static const int x_offset[4] = {0x0,0x1,0x4,0x5};
		static const int y_offset[4] = {0x0,0x2,0x8,0xa};
		int x,y, ex, ey;

		if (attr & 0x01) sx -= 256;
		if (sy >= 240) sy -= 256;

		number += ((sprite_bank & 0x3) << 8) + ((attr & 0xc0) << 4);
		number = number << 2;
		number += (sprite_bank >> 2) & 3;

		if (!is_flakatck || source[0x00])
		{
			number += bank_base;

			switch (attr & 0x0e)
			{
				case 0x06: width = height = 1; break;
				case 0x04: width = 1; height = 2; number &= (~2); break;
				case 0x02: width = 2; height = 1; number &= (~1); break;
				case 0x00: width = height = 2; number &= (~3); break;
				case 0x08: width = height = 4; number &= (~3); break;
				default: width = 1; height = 1;
			}

			for (y = 0; y < height; y++)
			{
				for (x = 0;x < width;x++)
				{
					ex = xflip ? (width-1-x) : x;
					ey = yflip ? (height-1-y) : y;

					if (flipscreen)
					{
						if (pri_mask != -1)
						;// not implemented
						else
							if (yflip) {
								if (xflip) {
									Render8x8Tile_Mask_Clip(pTransDraw, number + x_offset[ex] + y_offset[ey], 248-(sx+x*8)-global_x_offset+24, 248-(sy+y*8)+global_y_offset, color, 4, transparent_color, color_offset, gfx_base);
								} else {
									Render8x8Tile_Mask_FlipX_Clip(pTransDraw, number + x_offset[ex] + y_offset[ey], 248-(sx+x*8)-global_x_offset+24, 248-(sy+y*8)+global_y_offset, color, 4, transparent_color, color_offset, gfx_base);
								}
							} else {
								if (xflip) {
									Render8x8Tile_Mask_FlipY_Clip(pTransDraw, number + x_offset[ex] + y_offset[ey], 248-(sx+x*8)-global_x_offset+24, 248-(sy+y*8)+global_y_offset, color, 4, transparent_color, color_offset, gfx_base);
								} else {
									Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, number + x_offset[ex] + y_offset[ey], 248-(sx+x*8)-global_x_offset+24, 248-(sy+y*8)+global_y_offset, color, 4, transparent_color, color_offset, gfx_base);
								}
							}
					}
					else
					{
						if (pri_mask != -1)
						;// not implemented
						else
							if (yflip) {
								if (xflip) {
									Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, number + x_offset[ex] + y_offset[ey], global_x_offset+sx+x*8, (sy+y*8)+global_y_offset, color, 4, transparent_color, color_offset, gfx_base);
								} else {
									Render8x8Tile_Mask_FlipY_Clip(pTransDraw, number + x_offset[ex] + y_offset[ey], global_x_offset+sx+x*8, (sy+y*8)+global_y_offset, color, 4, transparent_color, color_offset, gfx_base);
								}
							} else {
								if (xflip) {
									Render8x8Tile_Mask_FlipX_Clip(pTransDraw, number + x_offset[ex] + y_offset[ey], global_x_offset+sx+x*8, (sy+y*8)+global_y_offset, color, 4, transparent_color, color_offset, gfx_base);
								} else {
									Render8x8Tile_Mask_Clip(pTransDraw, number + x_offset[ex] + y_offset[ey], global_x_offset+sx+x*8, (sy+y*8)+global_y_offset, color, 4, transparent_color, color_offset, gfx_base);
								}
							}
					}
				}
			}
		}

		source += inc;
	}
}

static void draw_sprites(int bank, unsigned char *gfx_base, int color_offset)
{
	int base_color = (K007121_ctrlram[bank][6]&0x30)<<1;
	const unsigned char *source = bank ? pDrvSprRAM1 : pDrvSprRAM0;

	K007121_sprites_draw(bank, gfx_base, DrvColTable, source, base_color, 40, -16, 0, -1, color_offset);
}


static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x1000; i++) {
			int rgb = Palette[DrvColTable[i]];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	draw_bg();
	draw_fg();

	draw_sprites(0, DrvGfxROM0, 0x000);
	draw_sprites(1, DrvGfxROM1, 0x800);

	draw_tx();

	BurnTransferCopy(DrvPalette);

	return 0;
}


static int DrvFrame()
{
	int nInterleave = 10;
	
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = DrvInputs[1] = DrvInputs[2] = 0xff;

		for (int i = 0 ; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	int nCyclesSegment = 0;
	int nSoundBufferPos = 0;
	int nCyclesTotal[2] =  { 1500000 / 60, 2000000 / 60 };
	int nCyclesDone[2] =  { 0, 0 };

	for (int i = 0; i < nInterleave; i++) {
		int nCurrentCPU, nNext;
		
		nCurrentCPU = 0;
		M6809Open(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += M6809Run(nCyclesSegment);
		if (i == (nInterleave - 1)) {
			M6809SetIRQ(0, M6809_IRQSTATUS_AUTO);
		}
		M6809Close();

		nCurrentCPU = 1;
		M6809Open(nCurrentCPU);
		if (trigger_sound_irq) {
			M6809SetIRQ(0, M6809_IRQSTATUS_AUTO);
			trigger_sound_irq = 0;
		}
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += M6809Run(nCyclesSegment);

		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen / nInterleave;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}

		M6809Close();
	}
		
	if (pBurnSoundOut) {
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			M6809Open(1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			M6809Close();
		}
	}
	
	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029696;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  = K007121_ctrlram;
		ba.nLen	  = 2 * 8;
		ba.szName = "K007121 Control RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		M6809Scan(nAction);

		BurnYM2151Scan(nAction);

		SCAN_VAR(K007121_flipscreen[0]);
		SCAN_VAR(K007121_flipscreen[1]);
		SCAN_VAR(soundlatch);
		SCAN_VAR(nBankData);

		if (nAction & ACB_WRITE) {
			M6809Open(0);
			contra_bankswitch_w(nBankData);
			M6809Close();
		}
	}

	return 0;
}


// Contra (US, Set 1)

static struct BurnRomInfo contraRomDesc[] = {
	{ "633m03.18a",	0x10000, 0xd045e1da, 1 | BRF_PRG  | BRF_ESS }, //  0 m6809 #0 Code
	{ "633i02.17a",	0x10000, 0xb2f7bd9a, 1 | BRF_PRG  | BRF_ESS }, //  1

	{ "633e01.12a",	0x08000, 0xd1549255, 2 | BRF_PRG  | BRF_ESS }, //  2 m6809 #1 Code

	{ "633e04.7d",	0x40000, 0x14ddc542, 3 | BRF_GRA },            //  3 Chip 0 Tiles
	{ "633e05.7f",	0x40000, 0x42185044, 3 | BRF_GRA },            //  4

	{ "633e06.16d",	0x40000, 0x9cf6faae, 4 | BRF_GRA },            //  5 Chip 1 Tiles
	{ "633e07.16f",	0x40000, 0xf2d06638, 4 | BRF_GRA },            //  6

	{ "633e08.10g",	0x00100, 0x9f0949fa, 5 | BRF_GRA },            //  7 Color Proms
	{ "633e09.12g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            //  8
	{ "633f10.18g",	0x00100, 0x2b244d84, 5 | BRF_GRA },            //  9
	{ "633f11.20g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 10
};

STD_ROM_PICK(contra)
STD_ROM_FN(contra)

struct BurnDriver BurnDrvContra = {
	"contra", NULL, NULL, "1987",
	"Contra (US, Set 1)\0", NULL, "Konami", "GX633",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, contraRomInfo, contraRomName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	224, 280, 3, 4
};


// Contra (US, Set 2)

static struct BurnRomInfo contra1RomDesc[] = {
	{ "633e03.18a",	0x10000, 0x7fc0d8cf, 1 | BRF_PRG  | BRF_ESS }, //  0 m6809 #0 Code
	{ "633i02.17a",	0x10000, 0xb2f7bd9a, 1 | BRF_PRG  | BRF_ESS }, //  1

	{ "633e01.12a",	0x08000, 0xd1549255, 2 | BRF_PRG  | BRF_ESS }, //  2 m6809 #1 Code

	{ "633e04.7d",	0x40000, 0x14ddc542, 3 | BRF_GRA },            //  3 Chip 0 Tiles
	{ "633e05.7f",	0x40000, 0x42185044, 3 | BRF_GRA },            //  4

	{ "633e06.16d",	0x40000, 0x9cf6faae, 4 | BRF_GRA },            //  5 Chip 1 Tiles
	{ "633e07.16f",	0x40000, 0xf2d06638, 4 | BRF_GRA },            //  6

	{ "633e08.10g",	0x00100, 0x9f0949fa, 5 | BRF_GRA },            //  7 Color Proms
	{ "633e09.12g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            //  8
	{ "633f10.18g",	0x00100, 0x2b244d84, 5 | BRF_GRA },            //  9
	{ "633f11.20g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 10
};

STD_ROM_PICK(contra1)
STD_ROM_FN(contra1)

struct BurnDriver BurnDrvContra1 = {
	"contra1", "contra", NULL, "1987",
	"Contra (US, Set 2)\0", NULL, "Konami", "GX633",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, contra1RomInfo, contra1RomName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	224, 280, 3, 4
};


// Contra (bootleg)

static struct BurnRomInfo contrabRomDesc[] = {
	{ "633m03.18a",	0x10000, 0xd045e1da, 1 | BRF_PRG  | BRF_ESS }, //  0 m6809 #0 Code
	{ "633i02.17a",	0x10000, 0xb2f7bd9a, 1 | BRF_PRG  | BRF_ESS }, //  1

	{ "633e01.12a",	0x08000, 0xd1549255, 2 | BRF_PRG  | BRF_ESS }, //  2 m6809 #1 Code

	{ "g-7.rom",	0x10000, 0x57f467d2, 3 | BRF_GRA },            //  3 Chip 0 Tiles
	{ "g-10.rom",	0x10000, 0xe6db9685, 3 | BRF_GRA },            //  4
	{ "g-9.rom",	0x10000, 0x875c61de, 3 | BRF_GRA },            //  5
	{ "g-8.rom",	0x10000, 0x642765d6, 3 | BRF_GRA },            //  6
	{ "g-15.rom",	0x10000, 0xdaa2324b, 3 | BRF_GRA },            //  7
	{ "g-16.rom",	0x10000, 0xe27cc835, 3 | BRF_GRA },            //  8
	{ "g-17.rom",	0x10000, 0xce4330b9, 3 | BRF_GRA },            //  9
	{ "g-18.rom",	0x10000, 0x1571ce42, 3 | BRF_GRA },            // 10

	{ "g-4.rom",	0x10000, 0x2cc7e52c, 4 | BRF_GRA },            // 11 Chip 1 Tiles
	{ "g-5.rom",	0x10000, 0xe01a5b9c, 4 | BRF_GRA },            // 12
	{ "g-6.rom",	0x10000, 0xaeea6744, 4 | BRF_GRA },            // 13
	{ "g-14.rom",	0x10000, 0x765afdc7, 4 | BRF_GRA },            // 14
	{ "g-11.rom",	0x10000, 0xbd9ba92c, 4 | BRF_GRA },            // 15
	{ "g-12.rom",	0x10000, 0xd0be7ec2, 4 | BRF_GRA },            // 16
	{ "g-13.rom",	0x10000, 0x2b513d12, 4 | BRF_GRA },            // 17

	{ "633e08.10g",	0x00100, 0x9f0949fa, 5 | BRF_GRA },            // 18 Color Proms
	{ "633e09.12g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 19
	{ "633f10.18g",	0x00100, 0x2b244d84, 5 | BRF_GRA },            // 20
	{ "633f11.20g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 21

	{ "conprom.53",	0x00100, 0x05a1da7e, 0 | BRF_OPT },            // 22
};

STD_ROM_PICK(contrab)
STD_ROM_FN(contrab)

struct BurnDriver BurnDrvContrab = {
	"contrab", "contra", NULL, "1987",
	"Contra (bootleg)\0", NULL, "bootleg", "GX633",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, contrabRomInfo, contrabRomName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	224, 280, 3, 4
};


// Contra (Japan)

static struct BurnRomInfo contrajRomDesc[] = {
	{ "633n03.18a",	0x10000, 0xfedab568, 1 | BRF_PRG  | BRF_ESS }, //  0 m6809 #0 Code
	{ "633k02.17a",	0x10000, 0x5d5f7438, 1 | BRF_PRG  | BRF_ESS }, //  1

	{ "633e01.12a",	0x08000, 0xd1549255, 2 | BRF_PRG  | BRF_ESS }, //  2 m6809 #1 Code

	{ "633e04.7d",	0x40000, 0x14ddc542, 3 | BRF_GRA },            //  3 Chip 0 Tiles
	{ "633e05.7f",	0x40000, 0x42185044, 3 | BRF_GRA },            //  4

	{ "633e06.16d",	0x40000, 0x9cf6faae, 4 | BRF_GRA },            //  5 Chip 1 Tiles
	{ "633e07.16f",	0x40000, 0xf2d06638, 4 | BRF_GRA },            //  6

	{ "633e08.10g",	0x00100, 0x9f0949fa, 5 | BRF_GRA },            //  7 Color Proms
	{ "633e09.12g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            //  8
	{ "633f10.18g",	0x00100, 0x2b244d84, 5 | BRF_GRA },            //  9
	{ "633f11.20g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 10
};

STD_ROM_PICK(contraj)
STD_ROM_FN(contraj)

struct BurnDriver BurnDrvContraj = {
	"contraj", "contra", NULL, "1987",
	"Contra (Japan)\0", NULL, "Konami", "GX633",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, contrajRomInfo, contrajRomName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	224, 280, 3, 4
};


// Contra (Japan bootleg)

static struct BurnRomInfo contrajbRomDesc[] = {
	{ "g-2.18a",	0x10000, 0xbdb9196d, 1 | BRF_PRG  | BRF_ESS }, //  0 m6809 #0 Code
	{ "633k02.17a",	0x10000, 0x5d5f7438, 1 | BRF_PRG  | BRF_ESS }, //  1

	{ "633e01.12a",	0x08000, 0xd1549255, 2 | BRF_PRG  | BRF_ESS }, //  2 m6809 #1 Code

	{ "g-7.rom",	0x10000, 0x57f467d2, 3 | BRF_GRA },            //  3 Chip 0 Tiles
	{ "g-10.rom",	0x10000, 0xe6db9685, 3 | BRF_GRA },            //  4
	{ "g-9.rom",	0x10000, 0x875c61de, 3 | BRF_GRA },            //  5
	{ "g-8.rom",	0x10000, 0x642765d6, 3 | BRF_GRA },            //  6
	{ "g-15.rom",	0x10000, 0xdaa2324b, 3 | BRF_GRA },            //  7
	{ "g-16.rom",	0x10000, 0xe27cc835, 3 | BRF_GRA },            //  8
	{ "g-17.rom",	0x10000, 0xce4330b9, 3 | BRF_GRA },            //  9
	{ "g-18.rom",	0x10000, 0x1571ce42, 3 | BRF_GRA },            // 10

	{ "g-4.rom",	0x10000, 0x2cc7e52c, 4 | BRF_GRA },            // 11 Chip 1 Tiles
	{ "g-5.rom",	0x10000, 0xe01a5b9c, 4 | BRF_GRA },            // 12
	{ "g-6.rom",	0x10000, 0xaeea6744, 4 | BRF_GRA },            // 13
	{ "g-14.rom",	0x10000, 0x765afdc7, 4 | BRF_GRA },            // 14
	{ "g-11.rom",	0x10000, 0xbd9ba92c, 4 | BRF_GRA },            // 15
	{ "g-12.rom",	0x10000, 0xd0be7ec2, 4 | BRF_GRA },            // 16
	{ "g-13.rom",	0x10000, 0x2b513d12, 4 | BRF_GRA },            // 17

	{ "633e08.10g",	0x00100, 0x9f0949fa, 5 | BRF_GRA },            // 18 Color Proms
	{ "633e09.12g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 19
	{ "633f10.18g",	0x00100, 0x2b244d84, 5 | BRF_GRA },            // 20
	{ "633f11.20g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 21
};

STD_ROM_PICK(contrajb)
STD_ROM_FN(contrajb)

struct BurnDriver BurnDrvContrajb = {
	"contrajb", "contra", NULL, "1987",
	"Contra (Japan bootleg)\0", NULL, "bootleg", "GX633",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, contrajbRomInfo, contrajbRomName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	224, 280, 3, 4
};


// Gryzor (Set 1)

static struct BurnRomInfo gryzorRomDesc[] = {
	{ "g2.18a",	0x10000, 0x92ca77bd, 1 | BRF_PRG  | BRF_ESS }, //  0 m6809 #0 Code
	{ "g3.17a",	0x10000, 0xbbd9e95e, 1 | BRF_PRG  | BRF_ESS }, //  1

	{ "633e01.12a",	0x08000, 0xd1549255, 2 | BRF_PRG  | BRF_ESS }, //  2 m6809 #1 Code

	{ "633e04.7d",	0x40000, 0x14ddc542, 3 | BRF_GRA },            //  3 Chip 0 Tiles
	{ "633e05.7f",	0x40000, 0x42185044, 3 | BRF_GRA },            //  4

	{ "633e06.16d",	0x40000, 0x9cf6faae, 4 | BRF_GRA },            //  5 Chip 1 Tiles
	{ "633e07.16f",	0x40000, 0xf2d06638, 4 | BRF_GRA },            //  6

	{ "633e08.10g",	0x00100, 0x9f0949fa, 5 | BRF_GRA },            //  7 Color Proms
	{ "633e09.12g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            //  8
	{ "633f10.18g",	0x00100, 0x2b244d84, 5 | BRF_GRA },            //  9
	{ "633f11.20g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 10
};

STD_ROM_PICK(gryzor)
STD_ROM_FN(gryzor)

struct BurnDriver BurnDrvGryzor = {
	"gryzor", "contra", NULL, "1987",
	"Gryzor (Set 1)\0", NULL, "Konami", "GX633",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, gryzorRomInfo, gryzorRomName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	224, 280, 3, 4
};


// Gryzor (Set 2)

static struct BurnRomInfo gryzoraRomDesc[] = {
	{ "633j03.18a",	0x10000, 0x20919162, 1 | BRF_PRG  | BRF_ESS }, //  0 m6809 #0 Code
	{ "633j02.17a",	0x10000, 0xb5922f9a, 1 | BRF_PRG  | BRF_ESS }, //  1

	{ "633e01.12a",	0x08000, 0xd1549255, 2 | BRF_PRG  | BRF_ESS }, //  2 m6809 #1 Code

	{ "633e04.7d",	0x40000, 0x14ddc542, 3 | BRF_GRA },            //  3 Chip 0 Tiles
	{ "633e05.7f",	0x40000, 0x42185044, 3 | BRF_GRA },            //  4

	{ "633e06.16d",	0x40000, 0x9cf6faae, 4 | BRF_GRA },            //  5 Chip 1 Tiles
	{ "633e07.16f",	0x40000, 0xf2d06638, 4 | BRF_GRA },            //  6

	{ "633e08.10g",	0x00100, 0x9f0949fa, 5 | BRF_GRA },            //  7 Color Proms
	{ "633e09.12g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            //  8
	{ "633f10.18g",	0x00100, 0x2b244d84, 5 | BRF_GRA },            //  9
	{ "633f11.20g",	0x00100, 0x14ca5e19, 5 | BRF_GRA },            // 10
};

STD_ROM_PICK(gryzora)
STD_ROM_FN(gryzora)

struct BurnDriver BurnDrvGryzora = {
	"gryzora", "contra", NULL, "1987",
	"Gryzor (Set 2)\0", NULL, "Konami", "GX633",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, gryzoraRomInfo, gryzoraRomName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	224, 280, 3, 4
};

