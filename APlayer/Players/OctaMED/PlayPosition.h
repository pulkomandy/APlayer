/******************************************************************************/
/* PlayPosition header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PlayPosition_h
#define __PlayPosition_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "Sequences.h"


/******************************************************************************/
/* CheckBound function                                                        */
/******************************************************************************/
template<class T> void CheckBound(T &value, T minimum, T maximum)
{
	if (value < minimum)
		value = minimum;
	else
	{
		if (value >= maximum)
			value = maximum - 1;
	}
}



/******************************************************************************/
/* PlayPosition class                                                         */
/******************************************************************************/
class OctaMED;
class SubSong;

class PlayPosition
{
public:
	PlayPosition(OctaMED *octaMED);
	virtual ~PlayPosition(void);

	enum adv_Mode
	{
		advSong, advBlock
	};

	enum pos_Def
	{
		songStart, songCurrPos, firstBlock, prevBlock, nextBlock, lastBlock,
		firstLine, lastLine, nextLine, prevLine
	};

	void SetParent(SubSong *parent);

	void Block(const BLOCK_NUM blk);
	void Line(const LINE_NUM line);
	void PSeqPos(const uint32 pos);
	void PSectPos(const uint32 pos);

	BLOCK_NUM Block(void) const;
	LINE_NUM Line(void) const;
	PSEQ_NUM PSeq(void) const;
	uint32 PSeqPos(void) const;
	uint32 PSectPos(void) const;

	void AdvancePos(bool (*cmdHandler)(OctaMED *octaMED, PlaySeqEntry &pse));
	void PatternBreak(const LINE_NUM newLNum, bool (*cmdHandler)(OctaMED *octaMED, PlaySeqEntry &pse));
	void PositionJump(const uint32 newPSeqPos, bool (*cmdHandler)(OctaMED *octaMED, PlaySeqEntry &pse));

	void SetAdvMode(adv_Mode mode);

	void operator = (const pos_Def);		// Set to position

protected:
	void AdvanceSongPosition(uint32 newPSeqPos, bool (*cmdHandler)(OctaMED *octaMED, PlaySeqEntry &pse));
	void VerifyRange(void);

	OctaMED *med;

	BLOCK_NUM pBlock;
	LINE_NUM pLine;
	PSEQ_NUM psq;
	uint32 pSecPos;
	uint32 pSeqPos;
	adv_Mode advMode;
	SubSong *parent_ss;
};

#endif
