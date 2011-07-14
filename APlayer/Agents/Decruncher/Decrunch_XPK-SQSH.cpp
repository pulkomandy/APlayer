/******************************************************************************/
/* Decruncher for XPK-SQSH class.                                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PBinary.h"
#include "PFile.h"

// APlayerKit headers
#include "APAddOns.h"

// Agent headers
#include "Decruncher.h"


/******************************************************************************/
/* Determine() will be check the file to see if it's packed with XPK-SQSH.    */
/*                                                                            */
/* Input:  "info" is a pointer to a info structure where to read and store    */
/*         information needed.                                                */
/******************************************************************************/
bool Decrunch_XPK_SQSH::Determine(APAgent_DecrunchFile *decrunchInfo)
{
	PFile *file = decrunchInfo->file;

	// Check the module size
	if (file->GetLength() < 46)
		return (false);

	// Check the marks
	file->SeekToBegin();

	if (file->Read_B_UINT32() != 'XPKF')
		return (false);

	file->Seek(4, PFile::pSeekCurrent);
	if (file->Read_B_UINT32() != 'SQSH')
		return (false);

	return (true);
}



/******************************************************************************/
/* GetUnpackedSize() returns the size of the unpacked data.                   */
/*                                                                            */
/* Input:  "info" is a pointer to a info structure where to read and store    */
/*         information needed.                                                */
/******************************************************************************/
uint32 Decrunch_XPK_SQSH::GetUnpackedSize(APAgent_DecrunchFile *decrunchInfo)
{
	decrunchInfo->file->Seek(12, PFile::pSeekBegin);

	return (decrunchInfo->file->Read_B_UINT32());
}



/******************************************************************************/
/* Unpack() will unpack the module.                                           */
/*                                                                            */
/* Input:  "sourceBuf" is a reference to the packed data.                     */
/*         "destBuf" is a reference where to write the unpacked data.         */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result Decrunch_XPK_SQSH::Unpack(PBinary &sourceBuf, PBinary &destBuf)
{
	uint8 *source, *dest;
	uint32 destLen, len;
	uint32 decrunched = 0;
	uint8 type;			// Type of chunk
	uint8 hchk;			// Header checksum
	uint16 chk;			// Chunk data checksum
	uint16 cp;			// Chunk packed length
	uint16 cu;			// Chunk unpacked length
	uint32 l, *lp;

	// Add safety buffer to the destination buffer
	destLen = destBuf.GetLength();
	destBuf.SetLength(destLen + 1024);

	// Get the buffer addresses
	source = sourceBuf.GetBufferForWriting() + 36;
	dest   = destBuf.GetBufferForWriting();

	len = destLen;

	while (len)
	{
		type = *source++;		// Type of chunk
		hchk = *source++;		// Chunk header checksum

		// Chunk data checksum
		chk  = (*source++) << 8;
		chk |= *source++;

		// Packed
		cp  = (*source++) << 8;
		cp |= *source++;

		// Unpacked
		cu  = (*source++) << 8;
		cu |= *source++;

		// Check header checksum
		hchk ^= type;
		hchk ^= chk ^ (chk >> 8);
		hchk ^= cp  ^ (cp >> 8);
		hchk ^= cu  ^ (cu >> 8);
		if (hchk != 0)
			return (AP_ERROR);

		// Make packed length size long aligned
		cp = (cp + 3) & 0xfffc;

		// Check chunk data checksum
		for (l = 0, lp = (uint32 *)(source + cp); lp != (uint32 *)source; )
			l ^= P_BENDIAN_TO_HOST_INT32(*--lp);

		chk ^= l & 0xffff;
		chk ^= l >> 16;
		if (chk != 0)
			return (AP_ERROR);

		// Check the type
		switch (type)
		{
			// Raw chunk (unpacked)
			case 0:
			{
				memcpy(dest, source, cu);
				dest       += cu;
				source     += cp;
				len        -= cu;
				decrunched += cu;
				break;
			}

			// Packed
			case 1:
			{
				UnSQSH(source, dest);
				dest       += cu;
				source     += cp;
				len        -= cu;
				decrunched += cu;
				break;
			}

			// Unknown
			default:
			{
				return (AP_ERROR);
			}
		}
	}

	// Check for corrupted data
	if (decrunched != destLen)
		return (AP_ERROR);

	return (AP_OK);
}



/******************************************************************************/
/* UnSQSH() will unpack a single chunk.                                       */
/*                                                                            */
/* Input:  "source" is a pointer to the source pointer.                       */
/*         "dest" is a pointer to the destination pointer.                    */
/******************************************************************************/
void Decrunch_XPK_SQSH::UnSQSH(uint8 *source, uint8 *dest)
{
	uint8 *a4, *a6;
	int32 d0, d1, d2, d3, d4, d5, d6, a2, a5;
	uint8 a3[] = { 2, 3, 4, 5, 6, 7, 8, 0, 3, 2, 4, 5, 6, 7, 8, 0, 4, 3, 5, 2, 6, 7, 8, 0, 5, 4,
				6, 2, 3, 7, 8, 0, 6, 5, 7, 2, 3, 4, 8, 0, 7, 6, 8, 2, 3, 4, 5, 0, 8, 7, 6, 2, 3, 4, 5, 0 };

	a6 = dest;
	a6 += *source++ << 8;
	a6 += *source++;
	d0 = d1 = d2 = d3 = a2 = 0;

	d3 = *(source++);
	*(dest++) = d3;

l6c6:
	if (d1 >= 8)
		goto l6dc;

	if (bfextu(source, d0, 1))
		goto l75a;

	d0 += 1;
	d5  = 0;
	d6  = 8;
	goto l734;

l6dc:
	if (bfextu(source, d0, 1))
		goto l726;

	d0 += 1;
	if (!bfextu(source, d0, 1))
		goto l75a;

	d0 += 1;
	if (bfextu(source, d0, 1))
		goto l6f6;

	d6 = 2;
	goto l708;

l6f6:
	d0 += 1;
	if (!bfextu(source, d0, 1))
		goto l706;

	d6 = bfextu(source, d0, 3);
	d0 += 3;
	goto l70a;

l706:
	d6 = 3;
l708:
	d0 += 1;
l70a:
	d6 = *(a3 + (8 * a2) + d6 - 17);
	if (d6 != 8)
		goto l730;

l718:
	if (d2 < 20)
		goto l722;

	d5 = 1;
	goto l732;

l722:
	d5 = 0;
	goto l734;

l726:
	d0 += 1;
	d6 = 8;

	if (d6 == a2)
		goto l718;

	d6 = a2;

l730:
	d5 = 4;
l732:
	d2 += 8;
l734:
	d4 = bfexts(source, d0, d6);

	d0 += d6;
	d3 -= d4;
	*dest++ = d3;

	d5--;
	if (d5 != -1)
		goto l734;

	if (d1 == 31)
		goto l74a;

	d1 += 1;

l74a:
	a2 = d6;
l74c:
	d6 = d2;
	d6 >>= 3;
	d2 -= d6;

	if (dest < a6)
		goto l6c6;

	dest = a6;
	return;

l75a:
	d0 += 1;
	if (bfextu(source, d0, 1))
		goto l766;

	d4 = 2;
	goto l79e;

l766:
	d0 += 1;
	if (bfextu(source, d0, 1))
		goto l772;

	d4 = 4;
	goto l79e;

l772:
	d0 += 1;
	if (bfextu(source, d0, 1))
		goto l77e;

	d4 = 6;
	goto l79e;

l77e:
	d0 += 1;
	if (bfextu(source, d0, 1))
		goto l792;

	d0 += 1;
	d6 = bfextu(source, d0, 3);
	d0 += 3;
	d6 += 8;
	goto l7a8;

l792:
	d0 += 1;
	d6 = bfextu(source, d0, 5);
	d0 += 5;
	d4 = 16;
	goto l7a6;

l79e:
	d0 += 1;
	d6 = bfextu(source, d0, 1);
	d0 += 1;

l7a6:
	d6 += d4;
l7a8:
	if (bfextu(source, d0, 1))
		goto l7c4;

	d0 += 1;
	if (bfextu(source, d0, 1))
		goto l7bc;

	d5 = 8;
	a5 = 0;
	goto l7ca;

l7bc:
	d5 = 14;
	a5 = -0x1100;
	goto l7ca;

l7c4:
	d5 = 12;
	a5 = -0x100;

l7ca:
	d0 += 1;
	d4 = bfextu(source, d0, d5);
	d0 += d5;
	d6 -= 3;

	if (d6 < 0)
		goto l7e0;

	if (d6 == 0)
		goto l7da;

	d1 -= 1;

l7da:
	d1 -= 1;
	if (d1 >= 0)
		goto l7e0;

	d1 = 0;

l7e0:
	d6 += 2;
	a4 = -1 + dest + a5 - d4;

l7ex:
	*dest++ = *a4++;
	d6--;
	if (d6 != -1)
		goto l7ex;

	d3 = *(--a4);
	goto l74c;
}



/******************************************************************************/
/* bfextu() emulate the 68020 bfextu command.                                 */
/******************************************************************************/
int32 Decrunch_XPK_SQSH::bfextu(uint8 *p, int32 bo, int32 bc)
{
	int32 r;

	p += bo / 8;
	r = *(p++);
	r <<= 8;
	r |= *(p++);
	r <<= 8;
	r |= *p;
	r <<= bo % 8;
	r &= 0xffffff;
	r >>= 24 - bc;

	return (r);
}



/******************************************************************************/
/* bfexts() emulate the 68020 bfexts command.                                 */
/******************************************************************************/
int32 Decrunch_XPK_SQSH::bfexts(uint8 *p, int32 bo, int32 bc)
{
	int32 r;

	p += bo / 8;
	r = *(p++);
	r <<= 8;
	r |= *(p++);
	r <<= 8;
	r |= *p;
	r <<= (bo % 8) + 8;
	r >>= 32 - bc;

	return (r);
}
