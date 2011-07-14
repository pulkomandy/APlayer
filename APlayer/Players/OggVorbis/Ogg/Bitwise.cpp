/******************************************************************************/
/* Ogg bitwise functions.                                                     */
/******************************************************************************/


// Player headers
#include "OggVorbis.h"


/******************************************************************************/
/* Bitmask table                                                              */
/******************************************************************************/
static uint32 mask[] =
{
	0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000f,
	0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff, 0x000001ff,
	0x000003ff, 0x000007ff, 0x00000fff, 0x00001fff, 0x00003fff,
	0x00007fff, 0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
	0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
	0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff,
	0x3fffffff, 0x7fffffff, 0xffffffff
};



/******************************************************************************/
/* OggPack_ReadInit()                                                         */
/******************************************************************************/
void OggVorbis::OggPack_ReadInit(OggPackBuffer *b, uint8 *buf, int32 bytes)
{
	memset(b, 0, sizeof(*b));
	b->buffer  = b->ptr = buf;
	b->storage = bytes;
}



/******************************************************************************/
/* OggPack_Read()                                                             */
/******************************************************************************/
int32 OggVorbis::OggPack_Read(OggPackBuffer *b, int32 bits)
{
	uint32 ret;
	uint32 m = mask[bits];

	bits += b->endBit;

	if ((b->endByte + 4) >= b->storage)
	{
		// Not the main path
		ret = -1UL;
		if ((b->endByte * 8 + bits) > (b->storage * 8))
			goto overflow;
	}

	ret = b->ptr[0] >> b->endBit;
	if (bits > 8)
	{
		ret |= b->ptr[1] << (8 - b->endBit);
		if (bits > 16)
		{
			ret |= b->ptr[2] << (16 - b->endBit);
			if (bits > 24)
			{
				ret |= b->ptr[3] << (24 - b->endBit);
				if ((bits > 32) && b->endBit)
					ret |= b->ptr[4] << (32 - b->endBit);
			}
		}
	}

	ret &= m;

overflow:
	b->ptr     += bits / 8;
	b->endByte += bits / 8;
	b->endBit   = bits & 7;

	return (ret);
}



/******************************************************************************/
/* OggPack_Read1()                                                            */
/******************************************************************************/
int32 OggVorbis::OggPack_Read1(OggPackBuffer *b)
{
	uint32 ret;

	if (b->endByte >= b->storage)
	{
		// Not the main path
		ret = -1UL;
		goto overflow;
	}

	ret = (b->ptr[0] >> b->endBit) & 1;

overflow:
	b->endBit++;

	if (b->endBit > 7)
	{
		b->endBit = 0;
		b->ptr++;
		b->endByte++;
	}

	return (ret);
}



/******************************************************************************/
/* OggPack_Look()                                                             */
/******************************************************************************/
int32 OggVorbis::OggPack_Look(OggPackBuffer *b, int32 bits)
{
	uint32 ret;
	uint32 m = mask[bits];

	bits += b->endBit;

	if ((b->endByte + 4) >= b->storage)
	{
		// Not the main path
		if ((b->endByte * 8 + bits) > (b->storage * 8))
			return (-1);
	}

	ret = b->ptr[0] >> b->endBit;
	if (bits > 8)
	{
		ret |= b->ptr[1] << (8 - b->endBit);
		if (bits > 16)
		{
			ret |= b->ptr[2] << (16 - b->endBit);
			if (bits > 24)
			{
				ret |= b->ptr[3] << (24 - b->endBit);
				if ((bits > 32) && b->endBit)
					ret |= b->ptr[4] << (32 - b->endBit);
			}
		}
	}

	return (m & ret);
}



/******************************************************************************/
/* OggPack_Adv()                                                              */
/******************************************************************************/
void OggVorbis::OggPack_Adv(OggPackBuffer *b, int32 bits)
{
	bits       += b->endBit;
	b->ptr     += bits / 8;
	b->endByte += bits / 8;
	b->endBit   = bits & 7;
}
