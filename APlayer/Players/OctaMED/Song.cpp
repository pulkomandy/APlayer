/******************************************************************************/
/* Song Interface.                                                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PFile.h"
#include "PList.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "MMDSampleHdr.h"
#include "Sample.h"
#include "SynthSound.h"
#include "Song.h"


/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "octaMED" is a pointer to the core object.                         */
/******************************************************************************/
Song::Song(OctaMED *octaMED)
{
	INST_NUM cnt;

	// Initialize member variables
	med       = octaMED;
	current   = NULL;
	currNum   = 0;
	currInstr = 0;

	for (cnt = 0; cnt < MAX_INSTRS; cnt++)
	{
		sg_sm[cnt] = NULL;
		sg_i[cnt].SetNum(octaMED, cnt);
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Song::~Song(void)
{
	int32 i, count;

	// Delete all the subsongs
	count = sg_ss.CountItems();
	for (i = 0; i < count; i++)
		delete sg_ss.GetItem(i);

	// Delete all samples
	for (i = 0; i < MAX_INSTRS; i++)
		delete sg_sm[i];
}



/******************************************************************************/
/* Sample2Instrument() will try to lookup an instrument for a sample.         */
/*                                                                            */
/* Input:  "sample" is the sample you want to map from.                       */
/*                                                                            */
/* Output: The instrument that uses the sample given.                         */
/******************************************************************************/
Instr *Song::Sample2Instrument(const Sample *sample)
{
	int32 i;

	for (i = 0; i < MAX_INSTRS; i++)
	{
		if (sample == sg_sm[i])
			return (&(sg_i[i]));
	}

	return (NULL);
}



/******************************************************************************/
/* AppendNew() will append a new subsong.                                     */
/*                                                                            */
/* Input:  "empty" indicates to create an empty subsong or not.               */
/******************************************************************************/
void Song::AppendNew(bool empty)
{
	SubSong *ss = new SubSong(med, empty);
	if (ss == NULL)
		throw PMemoryException();

	sg_ss.AddTail(ss);
	if (current == NULL)
		current = ss;
}



/******************************************************************************/
/* CurrSS() returns the current subsong.                                      */
/*                                                                            */
/* Output: A pointer to the current subsong.                                  */
/******************************************************************************/
SubSong *Song::CurrSS(void)
{
	return (current);
}



/******************************************************************************/
/* NumSubSongs() returns the number of subsongs in the song object.           */
/*                                                                            */
/* Output: The number of subsongs.                                            */
/******************************************************************************/
uint32 Song::NumSubSongs(void)
{
	return (sg_ss.CountItems());
}



/******************************************************************************/
/* SetSSNum() will set the pointer to point on the subsong given.             */
/*                                                                            */
/* Input:  "ssNum" is the subsong number to point to.                         */
/******************************************************************************/
void Song::SetSSNum(int32 ssNum)
{
	if (ssNum < 0)
		ssNum = 0;

	currNum = min(NumSubSongs() - 1, (uint32)ssNum);
	current = sg_ss.GetItem(currNum);
}



/******************************************************************************/
/* SampleSlotUsed() will check to see if the sample slot at the given         */
/*      instrument is used or not.                                            */
/*                                                                            */
/* Input:  "iNum" is the instrument number to check on.                       */
/*                                                                            */
/* Output: True if the slot is used, false if not.                            */
/******************************************************************************/
bool Song::SampleSlotUsed(INST_NUM iNum)
{
	return (sg_sm[iNum] != NULL ? true : false);
}



/******************************************************************************/
/* InstrSlotUsed() will check to see if the instrument slot at the given      */
/*      instrument is used or not.                                            */
/*                                                                            */
/* Input:  "iNum" is the instrument number to check on.                       */
/*                                                                            */
/* Output: True if the slot is used, false if not.                            */
/******************************************************************************/
bool Song::InstrSlotUsed(INST_NUM iNum)
{
	return (((sg_sm[iNum] != NULL) || sg_i[iNum].IsMIDI()) ? true : false);
}



/******************************************************************************/
/* GetSubSong() returns the subsong given.                                    */
/*                                                                            */
/* Input:  "sNum" is the subsong number to return.                            */
/*                                                                            */
/* Output: A pointer to the subsong.                                          */
/******************************************************************************/
SubSong *Song::GetSubSong(uint32 sNum)
{
	return (sg_ss.GetItem(sNum));
}



/******************************************************************************/
/* GetSample() returns the sample given.                                      */
/*                                                                            */
/* Input:  "num" is the sample number to return.                              */
/*                                                                            */
/* Output: A pointer to the sample.                                           */
/******************************************************************************/
Sample *Song::GetSample(INST_NUM num)
{
	return (sg_sm[num]);
}



/******************************************************************************/
/* GetSynthSound() returns the synth sound object on the instrument given.    */
/*                                                                            */
/* Input:  "num" is the synth sound number to return.                         */
/*                                                                            */
/* Output: A pointer to the synth sound.                                      */
/******************************************************************************/
SynthSound *Song::GetSynthSound(INST_NUM num)
{
	return ((SynthSound *)sg_sm[num]);
}



/******************************************************************************/
/* CurrInstr() returns the current instrument.                                */
/*                                                                            */
/* Output: A pointer to the instrument.                                       */
/******************************************************************************/
Instr *Song::CurrInstr(void)
{
	return (&sg_i[currInstr]);
}



/******************************************************************************/
/* GetInstr() returns the instrument given.                                   */
/*                                                                            */
/* Input:  "iNum" is the instrument number to return.                         */
/*                                                                            */
/* Output: A pointer to the instrument.                                       */
/******************************************************************************/
Instr *Song::GetInstr(INST_NUM iNum)
{
	return (&sg_i[iNum]);
}



/******************************************************************************/
/* CurrInstrNum() returns the current instrument number.                      */
/*                                                                            */
/* Output: The current instrument number.                                     */
/******************************************************************************/
INST_NUM Song::CurrInstrNum(void)
{
	return (currInstr);
}



/******************************************************************************/
/* SetSample() assigns the sample to the instrument.                          */
/*                                                                            */
/* Input:  "num" is the instrument number to assign to.                       */
/*         "sample" is the sample to use.                                     */
/******************************************************************************/
void Song::SetSample(INST_NUM num, Sample *s)
{
	if (s != sg_sm[num])
	{
		delete sg_sm[num];
		sg_sm[num] = s;
		sg_i[num].ValidateLoop();
	}

	if (num == CurrInstrNum())
		UpdateSample();
}



/******************************************************************************/
/* SetInstrNum() sets the current instrument and update it.                   */
/*                                                                            */
/* Input:  "iNum" is the instrument number to set.                            */
/******************************************************************************/
void Song::SetInstrNum(INST_NUM iNum)
{
	sg_i[currInstr = iNum].Update();
}



/******************************************************************************/
/* SetAnnoText() sets the annotation text.                                    */
/*                                                                            */
/* Input:  "text" is a pointer to the text to remember.                       */
/******************************************************************************/
void Song::SetAnnoText(const char *text)
{
	PCharSet_Amiga charSet;

	annoText.SetString(text, &charSet);
}



/******************************************************************************/
/* UpdateSample() update the sample information.                              */
/******************************************************************************/
void Song::UpdateSample(void)
{
	SetInstrNum(currInstr);
}



/******************************************************************************/
/* ReadSynthSound() read in synth sound information from the file.            */
/******************************************************************************/
void Song::ReadSynthSound(INST_NUM iNum, PFile *f, PString &errorStr, PResource *res, bool isHybrid)
{
	int64 startOffs;
	SynthSound *sy;
	uint32 *wfPtr;
	MMD_SynthSound synHdr;
	uint32 cnt2;

	// Remember the position in the file
	startOffs = f->GetPosition() - 6;

	// Allocate the synth sound object
	sy = new SynthSound(med);
	if (sy == NULL)
		throw PMemoryException();

	try
	{
		// Read in the synth structure from the file
		synHdr.decay     = f->Read_UINT8();
		f->Seek(3, PFile::pSeekCurrent);
		synHdr.rpt       = f->Read_B_UINT16();
		synHdr.rptLen    = f->Read_B_UINT16();
		synHdr.volTblLen = f->Read_B_UINT16();
		synHdr.wfTblLen  = f->Read_B_UINT16();
		synHdr.volSpeed  = f->Read_UINT8();
		synHdr.wfSpeed   = f->Read_UINT8();
		synHdr.numWfs    = f->Read_B_UINT16();

		// The easiest things first...
		sy->SetVolSpeed(synHdr.volSpeed);
		sy->SetWFSpeed(synHdr.wfSpeed);

		for (cnt2 = 0; cnt2 < synHdr.volTblLen; )
		{
			uint8 data = f->Read_UINT8();
			sy->SetVolData(cnt2++, data);

			if (data == SynthSound::CMD_END)
			{
				sy->SetVolTableLen(cnt2);
				break;
			}
		}

		f->Seek(synHdr.volTblLen - cnt2, PFile::pSeekCurrent);

		for (cnt2 = 0; cnt2 < synHdr.wfTblLen; )
		{
			uint8 data = f->Read_UINT8();
			sy->SetWFData(cnt2++, data);

			if (data == SynthSound::CMD_END)
			{
				sy->SetWFTableLen(cnt2);
				break;
			}
		}

		f->Seek(synHdr.wfTblLen - cnt2, PFile::pSeekCurrent);

		wfPtr = new uint32[synHdr.numWfs];
		if (wfPtr == NULL)
			throw PMemoryException();

		try
		{
			f->ReadArray_B_UINT32s(wfPtr, synHdr.numWfs);

			for (cnt2 = 0; cnt2 < synHdr.numWfs; cnt2++)
			{
				SynthWF *wf;

				f->Seek(startOffs + wfPtr[cnt2], PFile::pSeekBegin);

				wf = new SynthWF();
				if (wf == NULL)
					throw PMemoryException();

				sy->AddTail(wf);

				// As the first wave, read the sample if hybrid
				if ((cnt2 == 0) && isHybrid)
				{
					MMDSampleHdr hybHdr(med, f, errorStr, res);

					if (hybHdr.IsSample())
						hybHdr.ReadSampleData(f, sy);
				}
				else
				{
					wf->sywfLength = f->Read_B_UINT16();
					f->Read(wf->sywfData, wf->sywfLength * 2);
				}
			}

			SetSample(iNum, sy);
		}
		catch(...)
		{
			delete[] wfPtr;
			throw;
		}

		delete[] wfPtr;
	}
	catch(...)
	{
		delete sy;
		throw;
	}
}



/******************************************************************************/
/* operator++ will set the pointer to the next subsong.                       */
/******************************************************************************/
void Song::operator++(int)
{
	SetSSNum(currNum + 1);
}
