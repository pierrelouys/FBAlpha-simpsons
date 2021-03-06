// FB Alpha The Simpsons driver module
// Based on MAME driver by Ernesto Corvi and various others

#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "k053260.h"
#include "eeprom.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvKonROM;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROMExp0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvGfxROMExp1;
static unsigned char *DrvSndROM;
static unsigned char *DrvKonRAM;
static unsigned char *DrvPalRAM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvZ80RAM;

static unsigned int *DrvPalette;
static unsigned char DrvRecalc;

static unsigned char *nDrvBank;

static int videobank;
static int init_eeprom_count;
static int simpsons_firq_enabled;
static int K053246Irq;

static int bg_colorbase;
static int sprite_colorbase;
static int layer_colorbase[3];
static int layerpri[3];

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvJoy4[8];
static unsigned char DrvJoy5[8];
static unsigned char DrvDiag;
static unsigned char DrvReset;
static unsigned char DrvInputs[5];

static int nCyclesDone[2];

static struct BurnInputInfo SimpsonsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy5 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy5 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy5 + 2,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy3 + 7,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 fire 2"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy5 + 3,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	DrvJoy4 + 7,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy4 + 2,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy4 + 3,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy4 + 0,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy4 + 1,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy4 + 4,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy4 + 5,	"p4 fire 2"	},

	{"Service",		BIT_DIGITAL,	&DrvDiag,	"diag"		},
	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
};

STDINPUTINFO(Simpsons)

static struct BurnInputInfo Simpsn2pInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy5 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy5 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Service",		BIT_DIGITAL,	&DrvDiag,	"diag"		},
	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
};

STDINPUTINFO(Simpsn2p)

void simpsons_main_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x1fa0:
		case 0x1fa1:
		case 0x1fa2:
		case 0x1fa3:
		case 0x1fa4:
		case 0x1fa5:
		case 0x1fa6:
		case 0x1fa7:
			K053246Write(address & 7, data);
		return;

		case 0x1fc0:
			K052109RMRDLine = data & 0x08;
			K053246_set_OBJCHA_line(~data & 0x20);
		return;

		case 0x1fc2:
		{
			if (data == 0xff) return; // ok?

			EEPROMWrite((data & 0x10) >> 3, (data & 0x08) >> 3, (data & 0x80) >> 7);

			videobank = data & 3;
			simpsons_firq_enabled = data & 0x04;
		}
		return;

		case 0x1fc6:
		case 0x1fc7:
			K053260Write(0, address & 1, data);
		return;
	}

	if ((address & 0xf000) == 0x0000) {
		if (videobank & 1) {
			DrvPalRAM[address & 0x0fff] = data;
			return;
		}
	}

	if ((address & 0xfff0) == 0x1fb0) {
		K053251Write(address & 0x0f, data);
		return;
	}

	if ((address & 0xe000) == 0x2000) {
		if (videobank & 2) {
			address ^= 1;
			DrvSprRAM[address & 0x1fff] = data;
			return;
		}
	}

	if ((address & 0xc000) == 0x0000) {
		K052109Write(address & 0x3fff, data);
		return;
	}
}

unsigned char simpsons_main_read(unsigned short address)
{
	switch (address)
	{
		case 0x1f81:
		{
			int res = ((EEPROMRead() & 1) << 4) | 0x20 | (~DrvDiag & 1);

			if (init_eeprom_count > 0)
			{
				init_eeprom_count--;
				res &= 0xfe;
			}
			return res;
		}

		case 0x1f80:
			return DrvInputs[4];

		case 0x1f90:
			return DrvInputs[0];

		case 0x1f91:
			return DrvInputs[1];

		case 0x1f92:
			return DrvInputs[2];

		case 0x1f93:
			return DrvInputs[3];

		case 0x1fc4: 
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			return 0;

		case 0x1fc6:
		case 0x1fc7:
			return K053260Read(0, (address & 1)+2);
		
		case 0x1fc8:
      		case 0x1fc9:
         		return K053246Read(address & 1);

		case 0x1fca:
			return 0; // watchdog
	}

	if ((address & 0xf000) == 0x0000) {
		if (videobank & 1) {
			return DrvPalRAM[address & 0x0fff];
		}
	}

	if ((address & 0xe000) == 0x2000) {
		if (videobank & 2) {
			address ^= 1;
			return DrvSprRAM[address & 0x1fff];
		}
	}

	if ((address & 0xc000) == 0x0000) {
		return K052109Read(address & 0x3fff);
	}

	return 0;
}

static void DrvZ80Bankswitch(int data)
{
	data &= 0x07;
	if (data < 2) return;

	int nBank = (data & 7) * 0x4000;

	nDrvBank[1] = data;

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + nBank);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + nBank);
}

void __fastcall simpsons_sound_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0xf800:
			BurnYM2151SelectRegister(data);
		return;

		case 0xf801:
			BurnYM2151WriteRegister(data);
		return;

		case 0xfa00:
			nCyclesDone[1] += ZetRun(100);
			ZetNmi();
		return;

		case 0xfe00:
			DrvZ80Bankswitch(data);
		return;
	}

	if (address >= 0xfc00 && address < 0xfc30) {
		K053260Write(0, address & 0xff, data);
		return;
	}
}

unsigned char __fastcall simpsons_sound_read(unsigned short address)
{
	switch (address)
	{
		case 0xf800:
			return 0xff;
		case 0xf801:
			return BurnYM2151ReadStatus();
	}

	if (address >= 0xfc00 && address < 0xfc30) {
		if ((address & 0x3f) == 0x01) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);

		return K053260Read(0, address & 0xff);
	}

	return 0;
}

static void simpsons_set_lines(int lines)
{
	nDrvBank[0] = lines;

	int nBank = (lines & 0x3f) * 0x2000;

	konamiMapMemory(DrvKonROM + 0x10000 + nBank, 0x6000, 0x7fff, KON_ROM); 
}

static void K052109Callback(int layer, int bank, int *code, int *color, int *, int *)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
	*code &= 0x7fff;
}

static void K053247Callback(int *code, int *color, int *priority)
{
	int pri = (*color & 0x0f80) >> 6;
	if (pri <= layerpri[2])					*priority = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority = 1;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority = 2;
	else 							*priority = 3;

	*color = sprite_colorbase + (*color & 0x001f);

	*code &= 0x7fff;
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

	K053260Reset(0);

	EEPROMReset();

	videobank = 0;

	if (EEPROMAvailable()) {
		init_eeprom_count = 0;
	} else {
		init_eeprom_count = 10;
	}

	simpsons_firq_enabled = 0;
	K053246Irq = 0;

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x090000;
	DrvZ80ROM		= Next; Next += 0x020000;

	DrvGfxROM0		= Next; Next += 0x100000;
	DrvGfxROMExp0		= Next; Next += 0x200000;
	DrvGfxROM1		= Next; Next += 0x400000;
	DrvGfxROMExp1		= Next; Next += 0x800000;

	DrvSndROM		= Next; Next += 0x200000;

	DrvPalette		= (unsigned int*)Next; Next += 0x800 * sizeof(int);

	AllRam			= Next;

	DrvZ80RAM		= Next; Next += 0x000800;

	DrvKonRAM		= Next; Next += 0x002000;
	DrvPalRAM		= Next; Next += 0x001000;
	DrvSprRAM		= Next; Next += 0x002000;

	nDrvBank		= Next; Next += 0x000002;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane[4] = { 0x018, 0x010, 0x008, 0x000 };
	int XOffs[8] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007 };
	int YOffs[8] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x100000);
	konami_rom_deinterleave_4(DrvGfxROM1, 0x400000);

	GfxDecode(0x8000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);

	K053247GfxDecode(DrvGfxROM1, DrvGfxROMExp1, 0x400000);

	return 0;
}

static const eeprom_interface simpsons_eeprom_intf =
{
	7,			// address bits
	8,			// data bits
	"011000",		// read command
	"011100",		// write command
	0,			// erase command
	"0100000000000",	// lock command
	"0100110000000",	// unlock command
	0,
	0
};

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
		if (BurnLoadRom(DrvKonROM  + 0x030000,  1, 1)) return 1;
		if (BurnLoadRom(DrvKonROM  + 0x050000,  2, 1)) return 1;
		if (BurnLoadRom(DrvKonROM  + 0x070000,  3, 1)) return 1;
		memcpy (DrvKonROM + 0x08000, DrvKonROM + 0x88000, 0x8000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x080000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x100000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x200000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x300000, 10, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 11, 1)) return 1;
		if (BurnLoadRom(DrvSndROM  + 0x100000, 12, 1)) return 1;

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvKonRAM,		0x4000, 0x5fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x10000,	0x6000, 0x7fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x08000,	0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(simpsons_main_write);
	konamiSetReadHandler(simpsons_main_read);
	konamiSetlinesCallback(simpsons_set_lines);
	konamiClose();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + 0x08000);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + 0x08000);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80RAM);
	ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80RAM);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(simpsons_sound_write);
	ZetSetReadHandler(simpsons_sound_read);
	ZetMemEnd();
	ZetClose();

	EEPROMInit(&simpsons_eeprom_intf);

	K052109Init(DrvGfxROM0, 0x0fffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(7, 0);

	K053247Init(DrvGfxROM1, 0x3fffff, K053247Callback);
	K053247SetSpriteOffset(-60, 39);

	BurnYM2151Init(3579545, 25.0);

	K053260Init(0, 3579545, DrvSndROM, 0x140000);

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

	EEPROMExit();

	BurnYM2151Exit();
	K053260Exit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static void simpsons_objdma()
{
	int counter, num_inactive;
	unsigned char *dstptr;
	unsigned short *src, *dst;

	K053247Export(&dstptr, 0, 0, 0, &counter);
	src = (unsigned short*)DrvSprRAM;
	dst = (unsigned short*)dstptr;
	num_inactive = counter = 256;

	do {
		if ((*src & 0x8000) && (*src & 0xff))
		{
			memcpy(dst, src, 0x10);
			dst += 8;
			num_inactive--;
		}
		src += 8;
	}
	while (--counter);

	if (num_inactive) do { *dst = 0; dst += 8; } while (--num_inactive);
}

static void sortlayers(int *layer,int *pri)
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
}

static int DrvDraw()
{
	if (DrvRecalc) {
		KonamiRecalcPal(DrvPalRAM, DrvPalette, 0x1000);
	}

	K052109UpdateScroll();

	int layer[3];

	bg_colorbase       = K053251GetPaletteIndex(0);
	sprite_colorbase   = K053251GetPaletteIndex(1);
	layer_colorbase[0] = K053251GetPaletteIndex(2);
	layer_colorbase[1] = K053251GetPaletteIndex(3);
	layer_colorbase[2] = K053251GetPaletteIndex(4);

	layerpri[0] = K053251GetPriority(2);
	layerpri[1] = K053251GetPriority(3);
	layerpri[2] = K053251GetPriority(4);
	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;

	sortlayers(layer,layerpri);

	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 16 * bg_colorbase;
	}

	if (nSpriteEnable & 8) K053247SpritesRender(DrvGfxROMExp1, 3);		// title (simpsons behind clouds)
	if (nBurnLayer & 1)    K052109RenderLayer(layer[0], 0, DrvGfxROMExp0);	// title (back-most cloud)
	if (nSpriteEnable & 4) K053247SpritesRender(DrvGfxROMExp1, 2);		// smithers' on first stage
	if (nBurnLayer & 2)    K052109RenderLayer(layer[1], 0, DrvGfxROMExp0);	// main layer (buildings, stage 1)
	if (nSpriteEnable & 2) K053247SpritesRender(DrvGfxROMExp1, 1);		// smithers' thugs on stage 1
	if (nBurnLayer & 4)    K052109RenderLayer(layer[2], 0, DrvGfxROMExp0);	// game over text
	if (nSpriteEnable & 1) K053247SpritesRender(DrvGfxROMExp1, 0);		// not used? seems to make sense here...

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();
	konamiNewFrame();

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

	int nInterleave = nBurnSoundLen;
	int nSoundBufferPos = 0;
	int nCyclesTotal[2] = { 3000000 / 60, 3579545 / 60 };
	
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	ZetOpen(0);
	konamiOpen(0);

	for (int i = 0; i < nInterleave; i++) {
		int nNext, nCyclesSegment;

		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[0];
		nCyclesSegment = konamiRun(nCyclesSegment);
		nCyclesDone[0] += nCyclesSegment;

		if (i == 1 && K053246Irq && simpsons_firq_enabled) {
			konamiSetIrqLine(KONAMI_FIRQ_LINE, KONAMI_HOLD_LINE);
		}

		K053246Irq = K053246_is_IRQ_enabled();

		nNext = (i + 1) * nCyclesTotal[1] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[1];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[1] += nCyclesSegment;

		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen / nInterleave;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K053260Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (K053246Irq) simpsons_objdma();
	if (K052109_irq_enabled) konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	if (pBurnSoundOut) {
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K053260Update(0, pSoundBuf, nSegmentLength);
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
		*pnMin = 0x029705;
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
		K053260Scan(nAction);

		KonamiICScan(nAction);

		EEPROMScan(nAction, pnMin);

		SCAN_VAR(videobank);
		SCAN_VAR(init_eeprom_count);
		SCAN_VAR(simpsons_firq_enabled);
		SCAN_VAR(K053246Irq);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		simpsons_set_lines(nDrvBank[0]);
		konamiClose();

		ZetOpen(0);
		DrvZ80Bankswitch(nDrvBank[1]);
		ZetClose();
	}

	return 0;
}


// The Simpsons (4 Players World, set 1)

static struct BurnRomInfo simpsonsRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-g01.17c",	0x020000, 0x9f843def, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-j13.13c",	0x020000, 0xaade2abd, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-j12.15c",	0x020000, 0x479e12f2, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-e03.6g",		0x020000, 0x866b7a35, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12
};

STD_ROM_PICK(simpsons)
STD_ROM_FN(simpsons)

struct BurnDriver BurnDrvSimpsons = {
	"simpsons", NULL, NULL, "1991",
	"The Simpsons (4 Players World, set 1)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_PREFIX_KONAMI, //GBF_SCRFIGHT, 0,
	NULL, simpsonsRomInfo, simpsonsRomName, SimpsonsInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	288, 224, 4, 3
};


// The Simpsons (4 Players World, set 2)

static struct BurnRomInfo simps4paRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-g01.17c",	0x020000, 0x9f843def, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-m13.13c",	0x020000, 0xf36c9423, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-l12.15c",	0x020000, 0x84f9d9ba, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-e03.6g",		0x020000, 0x866b7a35, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12
};

STD_ROM_PICK(simps4pa)
STD_ROM_FN(simps4pa)

struct BurnDriver BurnDrvSimps4pa = {
	"simps4pa", "simpsons", NULL, "1991",
	"The Simpsons (4 Players World, set 2)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_KONAMI, //GBF_SCRFIGHT, 0,
	NULL, simps4paRomInfo, simps4paRomName, SimpsonsInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	288, 224, 4, 3
};


// The Simpsons (2 Players World, set 1)

static struct BurnRomInfo simpsn2pRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-p01.17c",	0x020000, 0x07ceeaea, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-013.13c",	0x020000, 0x8781105a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-012.15c",	0x020000, 0x244f9289, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-g03.6g",		0x020000, 0x76c1850c, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12
};

STD_ROM_PICK(simpsn2p)
STD_ROM_FN(simpsn2p)

struct BurnDriver BurnDrvSimpsn2p = {
	"simpsn2p", "simpsons", NULL, "1991",
	"The Simpsons (2 Players World, set 1)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, //GBF_SCRFIGHT, 0,
	NULL, simpsn2pRomInfo, simpsn2pRomName, Simpsn2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	288, 224, 4, 3
};


// The Simpsons (2 Players World, set 2)

static struct BurnRomInfo simps2paRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-p01.17c",	0x020000, 0x07ceeaea, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-_13.13c",	0x020000, 0x54e6df66, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-_12.15c",	0x020000, 0x96636225, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-g03.6g",		0x020000, 0x76c1850c, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12
};

STD_ROM_PICK(simps2pa)
STD_ROM_FN(simps2pa)

struct BurnDriver BurnDrvSimps2pa = {
	"simps2pa", "simpsons", NULL, "1991",
	"The Simpsons (2 Players World, set 2)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, //GBF_SCRFIGHT, 0,
	NULL, simps2paRomInfo, simps2paRomName, SimpsonsInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	288, 224, 4, 3
};


// The Simpsons (2 Players Asia)

static struct BurnRomInfo simp2paRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-p01.17c",	0x020000, 0x07ceeaea, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-113.13c",	0x020000, 0x8781105a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-112.15c",	0x020000, 0x3bd69404, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-e03.6g",		0x020000, 0x866b7a35, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12
};

STD_ROM_PICK(simp2pa)
STD_ROM_FN(simp2pa)

struct BurnDriver BurnDrvSimp2pa = {
	"simp2pa", "simpsons", NULL, "1991",
	"The Simpsons (2 Players Asia)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, //GBF_SCRFIGHT, 0,
	NULL, simp2paRomInfo, simp2paRomName, Simpsn2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	288, 224, 4, 3
};


// The Simpsons (2 Players Japan)

static struct BurnRomInfo simps2pjRomDesc[] = {
	{ "072-s02.16c",	0x020000, 0x265f7a47, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-t01.17c",	0x020000, 0x91de5c2d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-213.13c",	0x020000, 0xb326a9ae, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-212.15c",	0x020000, 0x584d9d37, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-g03.6g",		0x020000, 0x76c1850c, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12
};

STD_ROM_PICK(simps2pj)
STD_ROM_FN(simps2pj)

struct BurnDriver BurnDrvSimps2pj = {
	"simps2pj", "simpsons", NULL, "1991",
	"The Simpsons (2 Players Japan)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, //GBF_SCRFIGHT, 0,
	NULL, simps2pjRomInfo, simps2pjRomName, Simpsn2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc,
	288, 224, 4, 3
};
