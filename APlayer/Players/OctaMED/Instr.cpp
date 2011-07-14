/******************************************************************************/
/* Instr Interface.                                                           */
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

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "LimVar.h"
#include "Player.h"
#include "Sample.h"
#include "Song.h"
#include "Instr.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Instr::Instr(void)
{
	Reset();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Instr::~Instr(void)
{
}



/******************************************************************************/
/* GetSample() returns the sample that corresponds with this instrument.      */
/*      Returns NULL if no sample in slot, or this is not a sample            */
/*      instrument.                                                           */
/*                                                                            */
/* Output: A pointer to the sample or NULL.                                   */
/******************************************************************************/
Sample *Instr::GetSample(void) const
{
	if (!IsMIDI())
		return (med->sg->GetSample(i_num));

	return (NULL);
}



/******************************************************************************/
/* SetNum() sets the instrument number.                                       */
/*                                                                            */
/* Input:  "octaMED" is a pointer to the core object.                         */
/*         "num" is the number the instrument should have.                    */
/******************************************************************************/
void Instr::SetNum(OctaMED *octaMED, INST_NUM num)
{
	med   = octaMED;
	i_num = num;
}



/******************************************************************************/
/* SetName() set the instrument name.                                         */
/*                                                                            */
/* Input:  "newName" is the name.                                             */
/******************************************************************************/
void Instr::SetName(const char *newName)
{
	PCharSet_Amiga charSet;

	i_name.SetString(newName, &charSet);
}



/******************************************************************************/
/* SetVol() set the volume.                                                   */
/*                                                                            */
/* Input:  "vol" is the volume to set.                                        */
/******************************************************************************/
void Instr::SetVol(uint32 vol)
{
	i_vol = vol;
}



/******************************************************************************/
/* SetRepeat() set the start loop.                                            */
/*                                                                            */
/* Input:  "newRep" is the new repeat position.                               */
/*         "keepEnd" indicate if you want to keep the previous end position.  */
/******************************************************************************/
void Instr::SetRepeat(uint32 newRep, bool keepEnd)
{
	if (keepEnd)
	{
		uint32 repEnd = i_repStart + i_repLength;	// The 1st sample following loop
		i_repStart = newRep;

		if (i_repStart >= (repEnd - 1))
			KillLoop();

		i_repLength = repEnd - i_repStart;
	}
	else
		i_repStart = newRep;

	ValidateLoop();
}



/******************************************************************************/
/* SetRepeatLen() set the loop length.                                        */
/*                                                                            */
/* Input:  "newLen" is the new length.                                        */
/******************************************************************************/
void Instr::SetRepeatLen(uint32 newLen)
{
	i_repLength = newLen;
	ValidateLoop();
}



/******************************************************************************/
/* SetTransp() set the sample transpose value.                                */
/*                                                                            */
/* Input:  "iTransp" is the transpose to set.                                 */
/******************************************************************************/
void Instr::SetTransp(int16 iTransp)
{
	i_sTrans = iTransp;
}



/******************************************************************************/
/* SetFineTune() changes the finetune                                         */
/*                                                                            */
/* Input:  "newFT" is the new finetune value.                                 */
/******************************************************************************/
void Instr::SetFineTune(int16 newFT)
{
	i_fineTune = newFT;
}



/******************************************************************************/
/* SetHold() set the hold value.                                              */
/*                                                                            */
/* Input:  "hold" is the hold value to set.                                   */
/******************************************************************************/
void Instr::SetHold(uint16 hold)
{
	i_hold = hold;
}



/******************************************************************************/
/* SetDecay() set the decay value.                                            */
/*                                                                            */
/* Input:  "decay" is the decay value to set.                                 */
/******************************************************************************/
void Instr::SetDecay(uint16 decay)
{
	i_decay = decay;
}



/******************************************************************************/
/* SetDefPitch() set the default pitch value.                                 */
/*                                                                            */
/* Input:  "newPitch" is the pitch value to set.                              */
/******************************************************************************/
void Instr::SetDefPitch(NOTE_NUM newPitch)
{
	if (newPitch == 0)
		SetDefFreq(0);
	else
		SetDefFreq(med->plr->GetNoteFrequency(newPitch - 1, GetFineTune()));
}



/******************************************************************************/
/* SetDefFreq() set the default frequency value.                              */
/*                                                                            */
/* Input:  "newFreq" is the frequency value to set.                           */
/******************************************************************************/
void Instr::SetDefFreq(uint32 newFreq)
{
	i_defFreq = newFreq;
}



/******************************************************************************/
/* SetMIDICh() set the midi channel.                                          */
/*                                                                            */
/* Input:  "midiCh" is the midi channel number to use.                        */
/******************************************************************************/
void Instr::SetMIDICh(uint32 midiCh)
{
	i_midiCh = midiCh;

	ValidateLoop();
}



/******************************************************************************/
/* GetName() returns the name.                                                */
/*                                                                            */
/* Output: The name.                                                          */
/******************************************************************************/
PString Instr::GetName(void)
{
	return (i_name);
}



/******************************************************************************/
/* GetVol() returns the volume.                                               */
/*                                                                            */
/* Output: The volume.                                                        */
/******************************************************************************/
uint32 Instr::GetVol(void) const
{
	return (i_vol);
}



/******************************************************************************/
/* GetHold() returns the hold value.                                          */
/*                                                                            */
/* Output: The hold value.                                                    */
/******************************************************************************/
uint16 Instr::GetHold(void)
{
	return (i_hold);
}



/******************************************************************************/
/* GetDecay() returns the decay value.                                        */
/*                                                                            */
/* Output: The decay value.                                                   */
/******************************************************************************/
uint16 Instr::GetDecay(void)
{
	return (i_decay);
}



/******************************************************************************/
/* GetValidDefFreq() returns a valid default frequency for the instrument.    */
/*                                                                            */
/* Output: The frequency.                                                     */
/******************************************************************************/
uint32 Instr::GetValidDefFreq(void) const
{
	return (i_defFreq != 0 ? i_defFreq : 22050);
}



/******************************************************************************/
/* GetRepeat() returns the repeat start position.                             */
/*                                                                            */
/* Output: The repeat start position.                                         */
/******************************************************************************/
uint32 Instr::GetRepeat(void)
{
	return (i_repStart);
}



/******************************************************************************/
/* GetRepeatLen() returns the repeat length.                                  */
/*                                                                            */
/* Output: The repeat length.                                                 */
/******************************************************************************/
uint32 Instr::GetRepeatLen(void)
{
	return (i_repLength);
}



/******************************************************************************/
/* GetTransp() returns the sample transpose value.                            */
/*                                                                            */
/* Output: The transpose.                                                     */
/******************************************************************************/
int16 Instr::GetTransp(void)
{
	return (i_sTrans);
}



/******************************************************************************/
/* GetFineTune() returns the fine tune.                                       */
/*                                                                            */
/* Output: The fine tune.                                                     */
/******************************************************************************/
int16 Instr::GetFineTune(void)
{
	return (i_fineTune);
}



/******************************************************************************/
/* IsMIDI() checks to see if the instrument is a midi instrument.             */
/*                                                                            */
/* Output: True for midi, false if not.                                       */
/******************************************************************************/
bool Instr::IsMIDI(void) const
{
	return (i_midiCh != 0 ? true : false);
}



/******************************************************************************/
/* ValidateLoop() validates the loop information.                             */
/******************************************************************************/
void Instr::ValidateLoop(void)
{
	Sample *sample = GetSample();

	if (sample != NULL)
	{
		uint32 length = sample->GetLength();

		if (length == 0)
			KillLoop();
		else
		{
			if (i_repStart >= length)
				i_repStart = length - 1;

			if ((i_repStart + i_repLength) >= length)
				i_repLength = (length - 1) - i_repStart;
		}
	}
}



/******************************************************************************/
/* Update()                                                                   */
/******************************************************************************/
void Instr::Update(void)
{
}



/******************************************************************************/
/* Reset() initialize all the member variables.                               */
/******************************************************************************/
void Instr::Reset(void)
{
	i_vol       = 0;
	i_sTrans    = 0;
	i_hold      = 0;
	i_decay     = 0;
	i_fineTune  = 0;
	i_repStart  = 0;
	i_repLength = 0;
	i_flags     = 0;
	i_midiCh    = 0;
	i_defFreq   = 0;
}



/******************************************************************************/
/* KillLoop() sets the instrument to have no loop.                            */
/******************************************************************************/
void Instr::KillLoop(void)
{
	i_repStart  = 0;
	i_repLength = 0;
	i_flags    &= ~LOOP;
}
