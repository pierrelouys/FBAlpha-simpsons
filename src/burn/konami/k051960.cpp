// K051960

#include "tiles_generic.h"
#include "konamiic.h"

int K051960_irq_enabled;
int K051960_nmi_enabled;
int K051960_spriteflip;

static unsigned char *K051960Ram = NULL;
static unsigned char K051960SpriteRomBank[3];
int K051960ReadRoms;
static int K051960RomOffset;
static unsigned int K051960RomMask;
static unsigned char *K051960Rom;
static unsigned char blank_tile[0x100];

static int nSpriteXOffset;
static int nSpriteYOffset;

typedef void (*K051960_Callback)(int *Code, int *Colour, int *Priority, int *Shadow);
static K051960_Callback K051960Callback;

void K051960SpritesRender(unsigned char *pSrc, int Priority)
{
#define NUM_SPRITES 128
	int Offset, PriCode;
	int SortedList[NUM_SPRITES];

	for (Offset = 0; Offset < NUM_SPRITES; Offset++) SortedList[Offset] = -1;

	for (Offset = 0; Offset < 0x400; Offset += 8) {
		if (K051960Ram[Offset] & 0x80) {
			SortedList[K051960Ram[Offset] & 0x7f] = Offset;
		}
	}

	for (PriCode = 0; PriCode < NUM_SPRITES; PriCode++) {
		int ox, oy, Code, Colour, Pri, Shadow, Size, w, h, x, y, xFlip, yFlip, xZoom, yZoom;

		static const int xOffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
		static const int yOffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
		static const int Width[8] =  { 1, 2, 1, 2, 4, 2, 4, 8 };
		static const int Height[8] = { 1, 1, 2, 2, 2, 4, 4, 8 };

		Offset = SortedList[PriCode];
		if (Offset == -1) continue;

		Code = K051960Ram[Offset + 2] + ((K051960Ram[Offset + 1] & 0x1f) << 8);
		Colour = K051960Ram[Offset + 3] & 0xff;
		Pri = 0;
		Shadow = Colour & 0x80;
		K051960Callback(&Code, &Colour, &Pri, &Shadow);

		if (Priority != -1) {
			if (Pri != Priority) continue; // not the best way to go about this...
		}

		Size = (K051960Ram[Offset + 1] & 0xe0) >> 5;
		w = Width[Size];
		h = Height[Size];

		if (w >= 2) Code &= ~0x01;
		if (h >= 2) Code &= ~0x02;
		if (w >= 4) Code &= ~0x04;
		if (h >= 4) Code &= ~0x08;
		if (w >= 8) Code &= ~0x10;
		if (h >= 8) Code &= ~0x20;

		ox = (256 * K051960Ram[Offset + 6] + K051960Ram[Offset + 7]) & 0x01ff;
		oy = 256 - ((256 * K051960Ram[Offset + 4] + K051960Ram[Offset + 5]) & 0x01ff);

		xFlip = K051960Ram[Offset + 6] & 0x02;
		yFlip = K051960Ram[Offset + 4] & 0x02;
		xZoom = (K051960Ram[Offset + 6] & 0xfc) >> 2;
		yZoom = (K051960Ram[Offset + 4] & 0xfc) >> 2;
		xZoom = 0x10000 / 128 * (128 - xZoom);
		yZoom = 0x10000 / 128 * (128 - yZoom);

		if (xZoom == 0x10000 && yZoom == 0x10000) {
			int sx,sy;

			for (y = 0; y < h; y++)	{
				sy = oy + 16 * y;
				sy -= nSpriteYOffset;
				sy -= 16;
				
				for (x = 0; x < w; x++)	{
					int c = Code;

					sx = ox + 16 * x;
					if (xFlip) c += xOffset[(w - 1 - x)];
					else c += xOffset[x];
					if (yFlip) c += yOffset[(h - 1 - y)];
					else c += yOffset[y];
					
					sx &= 0x1ff;
					sx -= 104;
					sx -= nSpriteXOffset;
					
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, c, sx, sy, Colour, 4, 0, 0, pSrc);
						} else {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, c, sx, sy, Colour, 4, 0, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, c, sx, sy, Colour, 4, 0, 0, pSrc);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, c, sx, sy, Colour, 4, 0, 0, pSrc);
						}
					}
				}
			}
		} else {
			int sx, sy, zw, zh;
			
			for (y = 0; y < h; y++)	{
				sy = oy + ((yZoom * y + (1 << 11)) >> 12);
				zh = (oy + ((yZoom * (y + 1) + (1 << 11)) >> 12)) - sy;
				sy -=nSpriteYOffset;
				sy -= 16;

				for (x = 0; x < w; x++)	{
					int c = Code;

					sx = ox + ((xZoom * x + (1 << 11)) >> 12);
					zw = (ox + ((xZoom * (x + 1) + (1 << 11)) >> 12)) - sx;
					if (xFlip) c += xOffset[(w - 1 - x)];
					else c += xOffset[x];
					if (yFlip) c += yOffset[(h - 1 - y)];
					else c += yOffset[y];
					
					sx &= 0x1ff;
					sx -= 104;
					sx -= nSpriteXOffset;

					RenderZoomedTile(pTransDraw, pSrc, c, Colour << 4 /*assume 4 bpp*/, 0, sx, sy, xFlip, yFlip, 16, 16, zw << 12, zh << 12);
				}
			}
		}		
	}
}

unsigned char K0519060FetchRomData(unsigned int Offset)
{
	int Code, Colour, Pri, Shadow, Off1, Addr;
	
	Addr = K051960RomOffset + (K051960SpriteRomBank[0] << 8) + ((K051960SpriteRomBank[1] & 0x03) << 16);
	Code = (Addr & 0x3ffe0) >> 5;
	Off1 = Addr & 0x1f;
	Colour = ((K051960SpriteRomBank[1] & 0xfc) >> 2) + ((K051960SpriteRomBank[2] & 0x03) << 6);
	Pri = 0;
	Shadow = Colour & 0x80;
	K051960Callback(&Code, &Colour, &Pri, &Shadow);
	
	Addr = (Code << 7) | (Off1 << 2) | Offset;
	Addr &= K051960RomMask;
	
	return K051960Rom[Addr];
}

unsigned char K051960Read(unsigned int Offset)
{
	if (K051960ReadRoms) {
		K051960RomOffset = (Offset & 0x3fc) >> 2;
		K0519060FetchRomData(Offset & 3);
	}
	
	return K051960Ram[Offset];
}

void K051960Write(unsigned int Offset, unsigned char Data)
{
	K051960Ram[Offset] = Data;
}

void K051960SetCallback(void (*Callback)(int *Code, int *Colour, int *Priority, int *Shadow))
{
	K051960Callback = Callback;
}

void K051960SetSpriteOffset(int x, int y)
{
	nSpriteXOffset = x;
	nSpriteYOffset = y;
}

void K051960Reset()
{
	memset(K051960SpriteRomBank, 0, 3);
	K051960ReadRoms = 0;
	K051960RomOffset = 0;

	K051960_irq_enabled = 0;
	K051960_nmi_enabled = 0;
	K051960_spriteflip = 0;
}

void K051960Init(unsigned char* pRomSrc, unsigned int RomMask)
{
	nSpriteXOffset = nSpriteYOffset = 0;

	K051960Ram = (unsigned char*)malloc(0x400);
	
	K051960RomMask = RomMask;
	
	K051960Rom = pRomSrc;
	
	KonamiIC_K051960InUse = 1;

	memset (blank_tile, 0, 0x100);

	nSpriteXOffset = nSpriteYOffset = 0;
}

void K051960Exit()
{
	free(K051960Ram);
	K051960Ram = NULL;
	
	K051960Callback = NULL;
	K051960RomMask = 0;
	K051960Rom = NULL;
	
	memset(K051960SpriteRomBank, 0, 3);
	K051960ReadRoms = 0;
	K051960RomOffset = 0;

	K051960Callback = NULL;
}

void K051960Scan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = K051960Ram;
		ba.nLen	  = 0x400;
		ba.szName = "K051960 Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(K051960SpriteRomBank);
		SCAN_VAR(K051960ReadRoms);
		SCAN_VAR(K051960RomOffset);
		SCAN_VAR(K051960_irq_enabled);
		SCAN_VAR(K051960_nmi_enabled);
		SCAN_VAR(K051960_spriteflip);
	}
}

void K051937Write(unsigned int Offset, unsigned char Data)
{
	if (Offset == 0) {
		K051960_irq_enabled = Data & 0x01;
		K051960_nmi_enabled = Data & 0x04;
		K051960_spriteflip  = Data & 0x08;
		K051960ReadRoms     = Data & 0x20;
		return;
	}
	
	if (Offset >= 2 && Offset <= 4) {
		K051960SpriteRomBank[Offset - 2] = Data;
		return;
	}
}

unsigned char K051937Read(unsigned int Offset)
{
	if (K051960ReadRoms && Offset >= 4 && Offset < 8)
	{
		return K0519060FetchRomData(Offset & 3);
	}
	else
	{
		if (Offset == 0)
		{
			static int counter;
			return (counter++) & 1;
		}

		return 0;
	}
}

