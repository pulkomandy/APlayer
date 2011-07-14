/******************************************************************************/
/* PlayPosition Interface.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Sequences.h"
#include "SubSong.h"
#include "PlayPosition.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PlayPosition::PlayPosition(OctaMED *octaMED)
{
	med       = octaMED;
	pBlock    = 0;
	pLine     = 0;
	psq       = 0;
	pSecPos   = 0;
	pSeqPos   = 0;
	parent_ss = NULL;
	advMode   = advSong;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PlayPosition::~PlayPosition(void)
{
}



/******************************************************************************/
/* SetParent()                                                                */
/******************************************************************************/
void PlayPosition::SetParent(SubSong *parent)
{
	parent_ss = parent;
}



/******************************************************************************/
/* Block()                                                                    */
/******************************************************************************/
void PlayPosition::Block(const BLOCK_NUM blk)
{
	pBlock = blk;
	VerifyRange();
}



/******************************************************************************/
/* Line()                                                                     */
/******************************************************************************/
void PlayPosition::Line(const LINE_NUM line)
{
	pLine = line;
	VerifyRange();
}



/******************************************************************************/
/* PSeqPos()                                                                  */
/******************************************************************************/
void PlayPosition::PSeqPos(const uint32 pos)
{
	pSeqPos = pos;
	VerifyRange();
}



/******************************************************************************/
/* PSectPos()                                                                 */
/******************************************************************************/
void PlayPosition::PSectPos(const uint32 pos)
{
	pSecPos = pos;
	VerifyRange();
}



/******************************************************************************/
/* Block()                                                                    */
/******************************************************************************/
BLOCK_NUM PlayPosition::Block(void) const
{
	return (pBlock);
}



/******************************************************************************/
/* Line()                                                                     */
/******************************************************************************/
LINE_NUM PlayPosition::Line(void) const
{
	return (pLine);
}



/******************************************************************************/
/* PSeq()                                                                     */
/******************************************************************************/
PSEQ_NUM PlayPosition::PSeq(void) const
{
	return (psq);
}



/******************************************************************************/
/* PSeqPos()                                                                  */
/******************************************************************************/
uint32 PlayPosition::PSeqPos(void) const
{
	return (pSeqPos);
}



/******************************************************************************/
/* PSectPos()                                                                 */
/******************************************************************************/
uint32 PlayPosition::PSectPos(void) const
{
	return (pSecPos);
}



/******************************************************************************/
/* AdvancePos()                                                               */
/******************************************************************************/
void PlayPosition::AdvancePos(bool (*cmdHandler)(OctaMED *octaMED, PlaySeqEntry &pse))
{
	pLine++;
	if (pLine >= parent_ss->Block(pBlock).Lines())
	{
		pLine = 0;

		if (advMode == advSong)
		{
			AdvanceSongPosition(pSeqPos + 1, cmdHandler);

			// Tell APlayer that the position has changed
			med->ChangePosition();
		}
	}
}



/******************************************************************************/
/* PatternBreak()                                                             */
/******************************************************************************/
void PlayPosition::PatternBreak(const LINE_NUM newLNum, bool (*cmdHandler)(OctaMED *octaMED, PlaySeqEntry &pse))
{
	if (advMode == advSong)
		AdvanceSongPosition(pSeqPos + 1, cmdHandler);

	pLine = newLNum;
	if (pLine >= parent_ss->Block(pBlock).Lines())
		pLine = 0;

	// Tell APlayer that the position has changed
	med->ChangePosition();
}



/******************************************************************************/
/* PositionJump()                                                             */
/******************************************************************************/
void PlayPosition::PositionJump(const uint32 newPSeqPos, bool (*cmdHandler)(OctaMED *octaMED, PlaySeqEntry &pse))
{
	if (advMode == advSong)
	{
		if (newPSeqPos <= pSeqPos)
			med->endReached = true;

		AdvanceSongPosition(newPSeqPos, cmdHandler);
	}

	pLine = 0;

	// Tell APlayer that the position has changed
	med->ChangePosition();
}



/******************************************************************************/
/* SetAdvMode()                                                               */
/******************************************************************************/
void PlayPosition::SetAdvMode(adv_Mode mode)
{
	advMode = mode;
}



/******************************************************************************/
/* operator = (pos_Def)                                                       */
/******************************************************************************/
void PlayPosition::operator = (const pos_Def pos)
{
	switch (pos)
	{
		// Restart from the beginning of the song
		case songStart:
		{
			pSecPos = 0;
			pSeqPos = 0;
			pLine   = 0;
		}

		// From current section and sequence positions
		case songCurrPos:
		{
			psq = parent_ss->Sect(pSecPos);
			AdvanceSongPosition(pSeqPos, NULL);
			VerifyRange();
			break;
		}

		case nextBlock:
		{
			pBlock++;
			pLine = 0;
			VerifyRange();
			break;
		}

		case prevBlock:
		{
			if (pBlock > 0)
			{
				pBlock--;
				pLine = 0;
				VerifyRange();
			}
			break;
		}

		case firstBlock:
		{
			pBlock = 0;
			pLine  = 0;
			break;
		}

		case lastBlock:
		{
			pLine  = 0;
			pBlock = parent_ss->NumBlocks() - 1;
			break;
		}

		case firstLine:
		{
			pLine = 0;
			break;
		}

		case prevLine:
		{
			if (pLine > 0)
				pLine--;

			break;
		}

		case nextLine:
		{
			pLine++;
			VerifyRange();
			break;
		}

		default:
		{
			ASSERT(false);
			break;
		}
	}
}



/******************************************************************************/
/* AdvanceSongPosition()                                                      */
/******************************************************************************/
void PlayPosition::AdvanceSongPosition(uint32 newPSeqPos, bool (*cmdHandler)(OctaMED *octaMED, PlaySeqEntry &pse))
{
	uint32 jumpCount = 0;
	uint32 songRollOver = 0;

	pSeqPos = newPSeqPos;
	for (;;)
	{
		if (pSeqPos >= (uint32)parent_ss->PSeq(psq).CountItems())
		{
			if (songRollOver++ > 3)
			{
				// Extreme case hang-prevention
				pBlock = 0;
				break;
			}

			pSeqPos = 0;
			pSecPos++;

			if (pSecPos >= parent_ss->NumSections())
			{
				pSecPos         = 0;
				med->endReached = true;
			}

			psq = parent_ss->Sect(pSecPos);
		}

		PlaySeqEntry &pse = *parent_ss->PSeq(psq).GetItem(pSeqPos);
		BLOCK_NUM newBlk  = (BLOCK_NUM)pse;

		if (pse.IsCmd())
		{
			switch (pse.GetCmd())
			{
				case PSEQCMD_POSJUMP:
				{
					if (jumpCount++ < 10)
					{
						pSeqPos = newBlk;
						continue;
					}
				}

				default:
				{
					if ((cmdHandler != NULL) && (*cmdHandler)(med, pse))
						break;

					pSeqPos++;
					continue;
				}
			}
		}

		pBlock = newBlk;
		break;
	}
}



/******************************************************************************/
/* VerifyRange()                                                              */
/******************************************************************************/
void PlayPosition::VerifyRange(void)
{
	if (parent_ss != NULL)
	{
		CheckBound<uint32>(pSecPos, 0, parent_ss->NumSections());
		CheckBound<PSEQ_NUM>(psq, 0, parent_ss->NumPlaySeqs());
		CheckBound<uint32>(pSeqPos, 0, parent_ss->PSeq(psq).CountItems());
		CheckBound<BLOCK_NUM>(pBlock, 0, parent_ss->NumBlocks());
		CheckBound<LINE_NUM>(pLine, 0, parent_ss->Block(pBlock).Lines());
	}
}
