/******************************************************************************/
/* Mixer Interface.                                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "APChannel.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Block.h"
#include "Instr.h"
#include "Sample.h"
#include "Tempo.h"
#include "Mixer.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MED_Mixer::MED_Mixer(OctaMED *octaMED)
{
	int32 cnt;
	uint32 cnt2;

	// Remember the player pointer
	med = octaMED;

	// Initialize member variables
	md_Channels = 0;

	// Fill in the frequency table
	for (cnt = 0; cnt < 16; cnt++)
	{
		// C-1 for this fine tune table (C-1 freq = 1046.502261 Hz)
		// Freq diff between C#1 and C-1 = 62.22826... = 8 * 7.778532836
		double calcVar = 1046.502261 + (cnt - 8) * 7.778532836;
		for (cnt2 = 0; cnt2 < 6 * 12; cnt2++)
		{
			md_freqTable[cnt][cnt2] = (uint16)calcVar;
			calcVar *= 1.059463094;		// 12th root of 2
		}
	}

	// Initialize the startSyn table
	for (cnt = 0; cnt < MAX_TRACKS; cnt++)
		startSyn[cnt] = false;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MED_Mixer::~MED_Mixer(void)
{
}



/******************************************************************************/
/* GetNoteFrequency() calculates the frequency from the note given.           */
/******************************************************************************/
uint32 MED_Mixer::GetNoteFrequency(NOTE_NUM note, int32 fineTune)
{
	ASSERT((fineTune > -9) && (fineTune < 8));

	if (note > 71)
		return (md_freqTable[fineTune + 8][71]);

	return (md_freqTable[fineTune + 8][note]);
}



/******************************************************************************/
/* GetInstrNoteFreq() returns note's frequency, also handling special notes.  */
/******************************************************************************/
uint32 MED_Mixer::GetInstrNoteFreq(NOTE_NUM note, Instr *i)
{
	switch (note)
	{
		case NOTE_DEF:
			return (i->GetValidDefFreq());

		case NOTE_11k:
			return (11025);

		case NOTE_22k:
			return (22050);

		case NOTE_44k:
			return (44100);

		default:
		{
			// TN: The Amiga does not have the extra octave,
			// so we make a little hack here to get The Last Ninja II
			// to play correctly at position 26-27
			if (i->GetSample()->IsSynthSound())
			{
				if (note <= 0x7f)
				{
					while (note > 60)
						note -= 12;

					return (GetNoteFrequency(note - 1, i->GetFineTune()));
				}
				else
					return (0);
			}
			else
			{
				if (note <= 0x7f)
					return (GetNoteFrequency(note - 1, i->GetFineTune()));
				else
					return (0);
			}
		}
	}
}



/******************************************************************************/
/* SetMixTempo() calculates the frequency to use for the tempo given.         */
/******************************************************************************/
void MED_Mixer::SetMixTempo(Tempo &newTempo)
{
	// Calculate the number of hertz the player should run in
	if (newTempo.bpm)
		med->playFreq = (newTempo.tempo * newTempo.linesPerBeat) / 10;
	else
		med->playFreq = 1.0 / (0.474326 / newTempo.tempo * 1.3968255);
}



/******************************************************************************/
/* Start()                                                                    */
/******************************************************************************/
void MED_Mixer::Start(uint32 channels)
{
	md_Channels = channels;
}



/******************************************************************************/
/* Stop()                                                                     */
/******************************************************************************/
void MED_Mixer::Stop(void)
{
}



/******************************************************************************/
/* Play() will begin to play the sample given with all the arguments.         */
/*                                                                            */
/* Input:  "chNum" is the channel number.                                     */
/*         "smp" is a pointer to the sample to play.                          */
/*         "freq" is the playback frequency.                                  */
/*         "startOffs" is the playback start offset (in samples).             */
/*         "loopStart" is the loop start offset (in samples).                 */
/*         "loopLen" is the loop length (in samples).                         */
/*         "flags" is some misc. flags.                                       */
/******************************************************************************/
void MED_Mixer::Play(uint32 chNum, Sample *smp, uint32 freq, uint32 startOffs, uint32 loopStart, uint32 loopLen, uint16 flags)
{
	if (chNum >= md_Channels)
		return;

	// Make some check for unimplemented features
	ASSERT(!smp->s_isStereo);
	ASSERT((flags & MIXER_PLAY_BACKWARDS) == 0);

	// Fix out of range offsets
	if (startOffs >= smp->s_length)
		startOffs = smp->s_length;

	// Okay, tell APlayer to play the sample
	med->virtChannels[chNum]->PlaySample(smp->s_data[0], startOffs, smp->s_length, smp->s_is16Bit ? 16 : 8);

	// Set loop?
	if ((flags & MIXER_PLAY_LOOP) && (loopLen > 2))
		med->virtChannels[chNum]->SetLoop(loopStart, loopLen, flags & MIXER_PLAY_PINGPONGLOOP ? APLOOP_PingPong : APLOOP_Normal);
}



/******************************************************************************/
/* ChangeSamplePosition() will add the "change" argument to the current       */
/*      position of the current playing sample.                               */
/*                                                                            */
/* Input:  "chNum" is the channel number to change.                           */
/*         "change" is a value to add to the current sample position.         */
/******************************************************************************/
void MED_Mixer::ChangeSamplePosition(uint32 chNum, int32 change)
{
	// Not implemented yet! If you have any modules that uses
	// this effect, please send it to me
	ASSERT(false);
}



/******************************************************************************/
/* SetSamplePosition() will set the sample position to the value given.       */
/*                                                                            */
/* Input:  "chNum" is the channel number to change.                           */
/*         "newPos" is the new sample position.                               */
/******************************************************************************/
void MED_Mixer::SetSamplePosition(uint32 chNum, int32 newPos)
{
	// Not implemented yet! If you have any modules that uses
	// this effect, please send it to me
	ASSERT(false);
}



/******************************************************************************/
/* SetChannelFreq() will change the frequency of the channel given.           */
/*                                                                            */
/* Input:  "chNum" is the channel number to change.                           */
/*         "freq" is the new frequency.                                       */
/******************************************************************************/
void MED_Mixer::SetChannelFreq(uint32 chNum, int32 freq)
{
	if (chNum >= md_Channels)
		return;

	if ((freq <= 0) || (freq > 65535))
	{
		MED_Mixer::MuteChannel(chNum);
		return;
	}

	// Tell APlayer about the frequency change
	med->virtChannels[chNum]->SetFrequency(freq);
}



/******************************************************************************/
/* SetChannelVolPan() will set the channel's volume and panning.              */
/*                                                                            */
/* Input:  "chNum" is the channel number to change.                           */
/*         "volume" is the new volume (0-127).                                */
/*         "pan" is the new panning (-16 - +16).                              */
/******************************************************************************/
void MED_Mixer::SetChannelVolPan(uint32 chNum, uint16 volume, int16 pan)
{
	if (chNum >= md_Channels)
		return;

	// Now set the volume and panning
	med->virtChannels[chNum]->SetVolume(volume * 2);
	med->virtChannels[chNum]->SetPanning(pan * 8 + 128);
}



/******************************************************************************/
/* PrepareSynthSound() initialize the channel to be ready to play a synth     */
/*      sound.                                                                */
/*                                                                            */
/* Input:  "chNum" is the channel number to change.                           */
/******************************************************************************/
void MED_Mixer::PrepareSynthSound(uint32 chNum)
{
	if (chNum < md_Channels)
		startSyn[chNum] = true;
}



/******************************************************************************/
/* SetSynthWaveform() will play the synth sound.                              */
/*                                                                            */
/* Input:  "chNum" is the channel number to change.                           */
/*         "data" is the new start address to the synth sound.                */
/*         "length" is the new length.                                        */
/******************************************************************************/
void MED_Mixer::SetSynthWaveform(uint32 chNum, void *data, uint32 length)
{
	if (chNum < md_Channels)
	{
		// Should we trig the sound?
		if (startSyn[chNum])
		{
			// Yap, do it
			med->virtChannels[chNum]->PlaySample(data, 0, length);
			startSyn[chNum] = false;
		}

		// Set the loop
		med->virtChannels[chNum]->SetLoop(data, 0, length, APLOOP_Retrig);
	}
}



/******************************************************************************/
/* MuteChannel() stops the channel given.                                     */
/******************************************************************************/
void MED_Mixer::MuteChannel(uint32 chNum)
{
	if (chNum < md_Channels)
		med->virtChannels[chNum]->Mute();
}
