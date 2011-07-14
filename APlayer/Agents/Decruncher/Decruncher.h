/******************************************************************************/
/* Decruncher header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Decruncher_h
#define __Decruncher_h

// PolyKit headers
#include "POS.h"
#include "PBinary.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* Decruncher class                                                           */
/******************************************************************************/
class Decruncher
{
public:
	Decruncher(void);
	virtual ~Decruncher(void);

	virtual bool Determine(APAgent_DecrunchFile *decrunchInfo) = 0;
	virtual uint32 GetUnpackedSize(APAgent_DecrunchFile *decrunchInfo) = 0;
	virtual ap_result Unpack(PBinary &sourceBuf, PBinary &destBuf) = 0;
};



/******************************************************************************/
/* Decrunch_PowerPacker class                                                 */
/******************************************************************************/
class Decrunch_PowerPacker : public Decruncher
{
public:
	Decrunch_PowerPacker(void) {};
	virtual ~Decrunch_PowerPacker(void) {};

	virtual bool Determine(APAgent_DecrunchFile *decrunchInfo);
	virtual uint32 GetUnpackedSize(APAgent_DecrunchFile *decrunchInfo);
	virtual ap_result Unpack(PBinary &sourceBuf, PBinary &destBuf);

protected:
	uint32 GetBits(uint32 num);

	const uint8 *source;
	uint32 counter;
	uint32 shift_in;
};



/******************************************************************************/
/* Unpack_XPK_SQSH class                                                      */
/******************************************************************************/
class Decrunch_XPK_SQSH : public Decruncher
{
public:
	Decrunch_XPK_SQSH(void) {};
	virtual ~Decrunch_XPK_SQSH(void) {};

	virtual bool Determine(APAgent_DecrunchFile *decrunchInfo);
	virtual uint32 GetUnpackedSize(APAgent_DecrunchFile *decrunchInfo);
	virtual ap_result Unpack(PBinary &sourceBuf, PBinary &destBuf);

protected:
	void UnSQSH(uint8 *source, uint8 *dest);
	int32 bfextu(uint8 *p, int32 bo, int32 bc);
	int32 bfexts(uint8 *p, int32 bo, int32 bc);
};

#endif
