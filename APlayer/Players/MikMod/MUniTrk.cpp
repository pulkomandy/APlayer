/******************************************************************************/
/* MikMod UniTrk manipulation interface.                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// Player headers
#include "MUniTrk.h"


/******************************************************************************/
/* UniOperands[] is an array over how many bytes each operand use.            */
/******************************************************************************/
uint16 uniOperands[UNI_LAST] =
{
	0,		// not used
	1,		// UNI_NOTE
	1,		// UNI_INSTRUMENT
	1,		// UNI_PTEFFECT0
	1,		// UNI_PTEFFECT1
	1,		// UNI_PTEFFECT2
	1,		// UNI_PTEFFECT3
	1,		// UNI_PTEFFECT4
	1,		// UNI_PTEFFECT5
	1,		// UNI_PTEFFECT6
	1,		// UNI_PTEFFECT7
	1,		// UNI_PTEFFECT8
	1,		// UNI_PTEFFECT9
	1,		// UNI_PTEFFECTA
	1,		// UNI_PTEFFECTB
	1,		// UNI_PTEFFECTC
	1,		// UNI_PTEFFECTD
	1,		// UNI_PTEFFECTE
	1,		// UNI_PTEFFECTF
	1,		// UNI_S3MEFFECTA
	1,		// UNI_S3MEFFECTD
	1,		// UNI_S3MEFFECTE
	1,		// UNI_S3MEFFECTF
	1,		// UNI_S3MEFFECTI
	1,		// UNI_S3MEFFECTQ
	1,		// UNI_S3MEFFECTR
	1,		// UNI_S3MEFFECTT
	1,		// UNI_S3MEFFECTU
	0,		// UNI_KEYOFF
	1,		// UNI_KEYFADE
	2,		// UNI_VOLEFFECTS
	1,		// UNI_XMEFFECT4
	1,		// UNI_XMEFFECT6
	1,		// UNI_XMEFFECTA
	1,		// UNI_XMEFFECTE1
	1,		// UNI_XMEFFECTE2
	1,		// UNI_XMEFFECTEA
	1,		// UNI_XMEFFECTEB
	1,		// UNI_XMEFFECTG
	1,		// UNI_XMEFFECTH
	1,		// UNI_XMEFFECTL
	1,		// UNI_XMEFFECTP
	1,		// UNI_XMEFFECTX1
	1,		// UNI_XMEFFECTX2
	1,		// UNI_ITEFFECTG
	1,		// UNI_ITEFFECTH
	1,		// UNI_ITEFFECTI
	1,		// UNI_ITEFFECTM
	1,		// UNI_ITEFFECTN
	1,		// UNI_ITEFFECTP
	1,		// UNI_ITEFFECTT
	1,		// UNI_ITEFFECTU
	1,		// UNI_ITEFFECTW
	1,		// UNI_ITEFFECTY
	2,		// UNI_ITEFFECTZ
	1,		// UNI_ITEFFECTS0
	2,		// UNI_ULTEFFECT9
	2,		// UNI_MEDSPEED
	0,		// UNI_MEDEFFECTF1
	0,		// UNI_MEDEFFECTF2
	0,		// UNI_MEDEFFECTF3
	2,		// UNI_OKTARP
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MUniTrk::MUniTrk(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MUniTrk::~MUniTrk(void)
{
}



/******************************************************************************/
/* UniFindRow() finds the address of row number 'row' in the UniMod(tm)       */
/*      stream 't'. Returns NULL if the row can't be found.                   */
/*                                                                            */
/* Input:  "t" is a pointer to the track.                                     */
/*         "row" is the row to find.                                          */
/*                                                                            */
/* Output: The pointer to the row.                                            */
/******************************************************************************/
uint8 *MUniTrk::UniFindRow(uint8 *t, uint16 row)
{
	uint8 c, l;

	if (t)
	{
		while (true)
		{
			c = *t;				// Get rep/len byte
			if (!c)
				return (NULL);	// Zero ? -> end of track..

			l = (c >> 5) + 1;	// Extract repeat value
			if (l > row)
				break;			// Reached wanted row? -> return pointer

			row -= l;			// Haven't reached row yet.. update row
			t   += c & 0x1f;	// Point to the next row
		}
	}

	return (t);
}



/******************************************************************************/
/* UniSetRow() sets the internal variables to point to the specific row.      */
/*                                                                            */
/* Input:  "t" is a pointer to the tracks row.                                */
/******************************************************************************/
void MUniTrk::UniSetRow(uint8 *t)
{
	rowStart = t;
	rowPc    = rowStart;
	rowEnd   = t ? rowStart + (*(rowPc++) & 0x1f) : t;
}



/******************************************************************************/
/* UniGetByte() returns the next byte in the stream.                          */
/*                                                                            */
/* Output: The byte.                                                          */
/******************************************************************************/
uint8 MUniTrk::UniGetByte(void)
{
	return (lastByte = (rowPc < rowEnd) ? *(rowPc++) : 0);
}



/******************************************************************************/
/* UniGetWord() returns the next word in the stream.                          */
/*                                                                            */
/* Output: The word.                                                          */
/******************************************************************************/
uint16 MUniTrk::UniGetWord(void)
{
	return (((uint16)UniGetByte() << 8) | UniGetByte());
}



/******************************************************************************/
/* UniSkipOpcode() skips all the bytes to the opcode.                         */
/******************************************************************************/
void MUniTrk::UniSkipOpcode(void)
{
	if (lastByte < UNI_LAST)
	{
		uint16 t = uniOperands[lastByte];

		while (t--)
			UniGetByte();
	}
}



/******************************************************************************/
/* UniInit() initialize the uni stream.                                       */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MUniTrk::UniInit(void)
{
	uniMax = BUFPAGE;

	if (!(uniBuf = new uint8[uniMax]))
		return (false);

	// Clear the array
	memset(uniBuf, 0, uniMax);

	return (true);
}



/******************************************************************************/
/* UniCleanup() frees the unitrk stream.                                     */
/******************************************************************************/
void MUniTrk::UniCleanup(void)
{
	delete[] uniBuf;
	uniBuf = NULL;
}



/******************************************************************************/
/* UniReset() resets the index-pointers to create a new track.                */
/******************************************************************************/
void MUniTrk::UniReset(void)
{
	unitt     = 0;		// Reset index to rep/len byte
	unipc     = 1;		// First opcode will be written to index 1
	lastp     = 0;		// No previous row yet
	uniBuf[0] = 0;		// Clear rep/len byte
}



/******************************************************************************/
/* UniExpand() expands the buffer with the number of bytes wanted.            */
/*                                                                            */
/* Input:  "wanted" is the number of bytes to expand.                         */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MUniTrk::UniExpand(int32 wanted)
{
	if ((unipc + wanted) >= uniMax)
	{
		uint8 *newBuf;

		// Expand the buffer by BUFPAGE bytes
		newBuf = new uint8[uniMax + BUFPAGE];

		// Check if the allocation succeeded
		if (newBuf)
		{
			// Copy the old data to the new block
			memcpy(newBuf, uniBuf, uniMax);
			delete[] uniBuf;

			uniBuf  = newBuf;
			uniMax += BUFPAGE;

			return (true);
		}
		else
			return (false);
	}

	return (true);
}



/******************************************************************************/
/* UniWriteByte() appends one byte of data to the current row of a track.     */
/*                                                                            */
/* Input:  "data" is the byte to write.                                       */
/******************************************************************************/
void MUniTrk::UniWriteByte(uint8 data)
{
	if (UniExpand(1))
	{
		// Write byte to current position and update
		uniBuf[unipc++] = data;
	}
}



/******************************************************************************/
/* UniWriteWord() appends a word to the current row of a track.               */
/*                                                                            */
/* Input:  "data" is the word to write.                                       */
/******************************************************************************/
void MUniTrk::UniWriteWord(uint16 data)
{
	if (UniExpand(2))
	{
		uniBuf[unipc++] = data >> 8;
		uniBuf[unipc++] = data & 0xff;
	}
}



/******************************************************************************/
/* UniNewLine() closes the current row of a unitrk stream (updates the        */
/*         rep/len byte) and sets pointers to start a new row.                */
/******************************************************************************/
void MUniTrk::UniNewLine(void)
{
	uint16 n, l, len;

	n = (uniBuf[lastp] >> 5) + 1;		// Repeat of previous row
	l = (uniBuf[lastp] & 0x1f);			// Length of previous row

	len = unipc - unitt;				// Length of current row

	// Now, check if the previous and the current row are identical..
	// When they are, just increase the repeat field of the previous row
	if ((n < 8) && (len == l) && (memcmp(&uniBuf[lastp + 1], &uniBuf[unitt + 1], len - 1) == 0))
	{
		uniBuf[lastp] += 0x20;
		unipc = unitt + 1;
	}
	else
	{
		if (UniExpand(unitt - unipc))
		{
			// Current and previous row aren't equal... update the pointers
			uniBuf[unitt] = len;
			lastp         = unitt;
			unitt         = unipc++;
		}
	}
}



/******************************************************************************/
/* UniTrkLen() determines the length (in rows) of a unitrk stream.            */
/*                                                                            */
/* Input:  "t" is a pointer to a unitrk stream to get the length of.          */
/*                                                                            */
/* Output: The length of the track.                                           */
/******************************************************************************/
uint16 MUniTrk::UniTrkLen(uint8 *t)
{
	uint16 len = 0;
	uint8 c;

	while ((c = *t & 0x1f))
	{
		len += c;
		t   += c;
	}

	len++;

	return (len);
}



/******************************************************************************/
/* UniDup() terminates the current unitrk stream and returns a pointer to a   */
/*         copy of the stream.                                                */
/*                                                                            */
/* Output: The pointer to the copy of the stream.                             */
/******************************************************************************/
uint8 *MUniTrk::UniDup(void)
{
	uint8 *d;

	if (!UniExpand(unitt - unipc))
		return (NULL);

	uniBuf[unitt] = 0;

	if (!(d = new uint8[unipc]))
		return (NULL);

	memcpy(d, uniBuf, unipc);

	return (d);
}



/******************************************************************************/
/* UniEffect() appends an effect opcode to the unitrk stream.                 */
/*                                                                            */
/* Input:  "eff" is the effect number.                                        */
/*         "dat" is the effect data.                                          */
/******************************************************************************/
void MUniTrk::UniEffect(uint16 eff, uint16 dat)
{
	if ((!eff) || (eff >= UNI_LAST))
		return;

	UniWriteByte(eff);
	if (uniOperands[eff] == 2)
		UniWriteWord(dat);
	else
		UniWriteByte(dat);
}



/******************************************************************************/
/* UniPTEffect() appends UNI_PTEFFECTX opcode to the unitrk stream.           */
/*                                                                            */
/* Input:  "eff" is the effect number.                                        */
/*         "dat" is the effect argument.                                      */
/*         "flags" is the module flags at the moment.                         */
/******************************************************************************/
void MUniTrk::UniPTEffect(uint8 eff, uint8 dat, uint16 flags)
{
	// Don't write empty effect
	if ((eff) || (dat) || (flags & UF_ARPMEM))
		UniEffect(UNI_PTEFFECT0 + eff, dat);
}



/******************************************************************************/
/* UniVolEffect() appends UNI_VOLEFFECT + effect/dat to unistream.            */
/*                                                                            */
/* Input:  "eff" is the effect number.                                        */
/*         "dat" is the effect argument.                                      */
/******************************************************************************/
void MUniTrk::UniVolEffect(uint8 eff, uint8 dat)
{
	// Don't write empty effect
	if ((eff) || (dat))		// Don't write empty effect
	{
		UniWriteByte(UNI_VOLEFFECTS);
		UniWriteByte(eff);
		UniWriteByte(dat);
	}
}
