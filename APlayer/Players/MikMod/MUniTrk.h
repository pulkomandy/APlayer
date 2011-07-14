/******************************************************************************/
/* MUniTrk header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MUniTrk_h
#define __MUniTrk_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MikMod_Internals.h"


/******************************************************************************/
/* MUniTrk class                                                              */
/******************************************************************************/
#define BUFPAGE			128			// Smallest unibuffer size

class MUniTrk
{
public:
	MUniTrk(void);
	virtual ~MUniTrk(void);

	uint8 *UniFindRow(uint8 *t, uint16 row);
	void UniSetRow(uint8 *t);
	uint8 UniGetByte(void);
	uint16 UniGetWord(void);
	void UniSkipOpcode(void);

	bool UniInit(void);
	void UniCleanup(void);
	void UniReset(void);
	bool UniExpand(int32 wanted);
	void UniWriteByte(uint8 data);
	void UniWriteWord(uint16 data);
	void UniNewLine(void);
	uint16 UniTrkLen(uint8 *t);
	uint8 *UniDup(void);

	void UniEffect(uint16 eff, uint16 dat);
	void UniPTEffect(uint8 eff, uint8 dat, uint16 flags);
	void UniVolEffect(uint8 eff, uint8 dat);

	inline void UniNote(uint16 note) { UniEffect(UNI_NOTE, note); };
	inline void UniInstrument(uint16 ins) { UniEffect(UNI_INSTRUMENT, ins); };

protected:
	uint8 *rowStart;		// Start address of a row
	uint8 *rowEnd;			// End address of a row (exclusive)
	uint8 *rowPc;			// Current unimod(tm) programcounter

	uint8 lastByte;			// for UniSkipOpcode()

	uint8 *uniBuf;			// Pointer to the temporary unitrk buffer
	uint16 uniMax;			// Maximum number of bytes to be written to this buffer

	uint16 unipc;			// Index in the buffer where next opcode will be written
	uint16 unitt;			// Holds index of the rep/len byte of a row
	uint16 lastp;			// Holds index to the previous row (needed for compressing)
};

#endif
