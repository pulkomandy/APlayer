/******************************************************************************/
/* SIDEmuPlayer header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDEmuPlayer_h
#define __SIDEmuPlayer_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "SIDTune.h"
#include "SIDEmuEngine.h"
#include "SID6581.h"
#include "SID6510.h"


/******************************************************************************/
/* Table used to check sidtune for usage of PlaySID digis.                    */
/******************************************************************************/
static const uint16 c64AddrTable[] =
{
	// PlaySID extended SID registers (0xd49d left out)
	//0xd41d, 0xd41e, 0xd41f,  // SID is too often written to as 32 bytes!
	0xd43d, 0xd43e, 0xd43f,
	0xd45d, 0xd45e, 0xd45f, 0xd47d, 0xd47e, 0xd47f,
	//0xd51d, 0xd51e, 0xd51f,  // SID is too often written to as 32 bytes!
	0xd53d, 0xd53e, 0xd53f,
	0xd55d, 0xd55e, 0xd55f, 0xd57d, 0xd57e, 0xd57f
};

// Number of addresses in c64AddrTable[]
static const int sidNumberOfC64Addr = sizeof(c64AddrTable) / sizeof(uint16);



/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/
#define FREQ_VBI_PAL					50
#define FREQ_VBI_NTSC					60



/******************************************************************************/
/* SIDEmuPlayer class                                                         */
/******************************************************************************/
class SIDEmuPlayer
{
public:
	SIDEmuPlayer(SID6581 *sid, SID6510 *cpu);
	virtual ~SIDEmuPlayer(void);

	bool EmuInitializeSong(SIDEmuEngine &thisEmuEngine, SIDTune &thisTune, uint16 songNumber);

protected:
	bool EmuInitializeSongOld(SIDEmuEngine &thisEmuEngine, SIDTune &thisTune, uint16 songNumber);

	SID6581 *sid6581;
	SID6510 *sid6510;

	uint8 oldValues[sidNumberOfC64Addr];
};

#endif
