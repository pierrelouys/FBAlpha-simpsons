#include "burnint.h"

static unsigned char K051733Ram[0x20];

void K051733Reset()
{
	memset (K051733Ram, 0, 0x20);
}

void K051733Write(int offset, int data)
{
	K051733Ram[offset & 0x1f] = data;
}

static int int_sqrt(UINT32 op)
{
	unsigned int i,step;

	i = 0x8000;
	step = 0x4000;
	while (step)
	{
		if (i*i == op) return i;
		else if (i*i > op) i -= step;
		else i += step;
		step >>= 1;
	}
	return i;
}

unsigned char K051733Read(int offset)
{
	int op1    = (K051733Ram[0x00] << 8) | K051733Ram[0x01];
	int op2    = (K051733Ram[0x02] << 8) | K051733Ram[0x03];
	int op3    = (K051733Ram[0x04] << 8) | K051733Ram[0x05];
	int rad    = (K051733Ram[0x06] << 8) | K051733Ram[0x07];
	int yobj1c = (K051733Ram[0x08] << 8) | K051733Ram[0x09];
	int xobj1c = (K051733Ram[0x0a] << 8) | K051733Ram[0x0b];
	int yobj2c = (K051733Ram[0x0c] << 8) | K051733Ram[0x0d];
	int xobj2c = (K051733Ram[0x0e] << 8) | K051733Ram[0x0f];

	offset &= 0x1f;

	switch (offset)
	{
		case 0x00:
			if (op2) return	(op1 / op2) >> 8;
			else return 0xff;

		case 0x01:
			if (op2) return	(op1 / op2) & 0xff;
			else return 0xff;

		case 0x02:
			if (op2) return	(op1 % op2) >> 8;
			else return 0xff;

		case 0x03:
			if (op2) return	(op1 % op2) & 0xff;
			else return 0xff;

		case 0x04:
			return int_sqrt(op3<<16) >> 8;

		case 0x05:
			return int_sqrt(op3<<16) & 0xff;

		case 0x06:
			return K051733Ram[0x13];

		case 0x07:{
			if (xobj1c + rad < xobj2c)
				return 0x80;

			if (xobj2c + rad < xobj1c)
				return 0x80;

			if (yobj1c + rad < yobj2c)
				return 0x80;

			if (yobj2c + rad < yobj1c)
				return 0x80;

			return 0;
		}

		case 0x0e:
			return ~K051733Ram[offset];

		case 0x0f:
			return ~K051733Ram[offset];

		default:
			return K051733Ram[offset];
	}

	return 0;
}

void K051733Scan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = K051733Ram;
		ba.nLen	  = 0x20;
		ba.szName = "K051733 Ram";
		BurnAcb(&ba);
	}
}
