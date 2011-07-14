/******************************************************************************/
/* SubSong Interface.                                                         */
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
#include "PList.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Block.h"
#include "LimVar.h"
#include "Player.h"
#include "PlayPosition.h"
#include "Sequences.h"
#include "Tempo.h"
#include "SubSong.h"


/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "octaMED" is a pointer to the core object.                         */
/*         "empty" indicates to create an empty subsong or not.               */
/******************************************************************************/
SubSong::SubSong(OctaMED *octaMED, bool empty) : position(octaMED)
{
	TRACK_NUM cnt;

	// Initialize member variables
	med          = octaMED;

	s_playTransp = 0;
	s_channels   = 4;
	s_volAdj     = 100;
	s_masterVol  = 64;
	filter       = false;

	position.SetParent(this);

	// Set default values
	for (cnt = 0; cnt < MAX_TRACKS; cnt++)
	{
		SetTrackVol(cnt, 64);
		SetTrackPan(cnt, 0);
	}

	s_flags = STEREOMODE;

	if (!empty)
	{
		Append(new MED_Block(64, 4));
		AppendNewSec(0);
		PlaySeq *newPSeq = new PlaySeq();
		Append(newPSeq);
		newPSeq->AddTail(new PlaySeqEntry(0));
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SubSong::~SubSong(void)
{
	int32 i, count;

	// Delete all the blocks
	count = s_blocks.CountItems();
	for (i = 0; i < count; i++)
		delete s_blocks.GetItem(i);

	// Delete all the sequences
	count = s_pSeqs.CountItems();
	for (i = 0; i < count; i++)
		delete s_pSeqs.GetItem(i);

	// Delete all the sections
	count = s_sections.CountItems();
	for (i = 0; i < count; i++)
		delete s_sections.GetItem(i);
}



/******************************************************************************/
/* AppendNewSec()                                                             */
/******************************************************************************/
void SubSong::AppendNewSec(uint16 secNum)
{
	Append(new SectSeqEntry(secNum));
}



/******************************************************************************/
/* Append()                                                                   */
/******************************************************************************/
void SubSong::Append(SectSeqEntry *sec)
{
	if (sec != NULL)
		s_sections.AddTail(sec);
}



/******************************************************************************/
/* Append()                                                                   */
/******************************************************************************/
void SubSong::Append(PlaySeq *pSeq)
{
	s_pSeqs.AddTail(pSeq);
}



/******************************************************************************/
/* Append()                                                                   */
/******************************************************************************/
void SubSong::Append(PlaySeqEntry *pse)
{
	Append(pse, position.PSeq());
}



/******************************************************************************/
/* Append()                                                                   */
/******************************************************************************/
void SubSong::Append(PlaySeqEntry *pse, PSEQ_NUM pSeq)
{
	if (pse != NULL)
	{
		PlaySeq &ps = PSeq(pSeq);
		ps.AddTail(pse);
	}
}



/******************************************************************************/
/* Append()                                                                   */
/******************************************************************************/
void SubSong::Append(MED_Block *blk)
{
	s_blocks.AddTail(blk);
}



/******************************************************************************/
/* PSeq()                                                                     */
/******************************************************************************/
PlaySeq &SubSong::PSeq(uint32 pos)
{
	return (*s_pSeqs.GetItem(pos));
}



/******************************************************************************/
/* Block()                                                                    */
/******************************************************************************/
MED_Block &SubSong::Block(BLOCK_NUM pos)
{
	return (*s_blocks.GetItem(pos));
}



/******************************************************************************/
/* Sect()                                                                     */
/******************************************************************************/
SectSeqEntry &SubSong::Sect(uint32 pos)
{
	return (*s_sections.GetItem(pos));
}



/******************************************************************************/
/* Pos()                                                                      */
/******************************************************************************/
const PlayPosition &SubSong::Pos(void) const
{
	return (position);
}



/******************************************************************************/
/* SetPos()                                                                   */
/******************************************************************************/
void SubSong::SetPos(const PlayPosition &newPos)
{
	position = newPos;
}



/******************************************************************************/
/* SetTempoBPM()                                                              */
/******************************************************************************/
void SubSong::SetTempoBPM(uint16 newBPM)
{
	s_tempo.tempo = newBPM;
	med->plr->SetMixTempo(s_tempo);
}



/******************************************************************************/
/* SetTempoTPL()                                                              */
/******************************************************************************/
void SubSong::SetTempoTPL(uint16 newTPL)
{
	s_tempo.ticksPerLine = newTPL;
}



/******************************************************************************/
/* SetTempoLPB()                                                              */
/******************************************************************************/
void SubSong::SetTempoLPB(uint8 newLPB)
{
	s_tempo.linesPerBeat = newLPB;
	med->plr->SetMixTempo(s_tempo);
}



/******************************************************************************/
/* SetTempoMode()                                                             */
/******************************************************************************/
void SubSong::SetTempoMode(bool bpm)
{
	s_tempo.bpm = bpm;
	med->plr->SetMixTempo(s_tempo);
}



/******************************************************************************/
/* SetTempo()                                                                 */
/******************************************************************************/
void SubSong::SetTempo(Tempo &tempo)
{
	s_tempo = tempo;
	med->plr->SetMixTempo(s_tempo);
}



/******************************************************************************/
/* SetStartTempo()                                                            */
/******************************************************************************/
void SubSong::SetStartTempo(Tempo &tempo)
{
	s_startTempo = tempo;
}



/******************************************************************************/
/* SetPlayTranspose()                                                         */
/******************************************************************************/
void SubSong::SetPlayTranspose(int16 pTransp)
{
	s_playTransp = pTransp;
}



/******************************************************************************/
/* SetNumChannels()                                                           */
/******************************************************************************/
void SubSong::SetNumChannels(uint32 channels)
{
	med->plr->Stop();
	s_channels = channels;
}



/******************************************************************************/
/* SetVolAdjust()                                                             */
/******************************************************************************/
void SubSong::SetVolAdjust(int16 volAdjust)
{
	s_volAdj = volAdjust;
	med->plr->RecalcVolAdjust();
}



/******************************************************************************/
/* SetMasterVol()                                                             */
/******************************************************************************/
void SubSong::SetMasterVol(int32 vol)
{
	s_masterVol = (uint8)vol;
}



/******************************************************************************/
/* SetTrackVol()                                                              */
/******************************************************************************/
void SubSong::SetTrackVol(TRACK_NUM trk, int32 vol)
{
	s_trkVol[trk] = (uint8)vol;
}



/******************************************************************************/
/* SetTrackPan()                                                              */
/******************************************************************************/
void SubSong::SetTrackPan(TRACK_NUM trk, int32 pan)
{
	s_trkPan[trk] = (int8)pan;
}



/******************************************************************************/
/* SetStereo()                                                                */
/******************************************************************************/
void SubSong::SetStereo(bool stereo)
{
	if (stereo)
		s_flags |= STEREOMODE;
	else
		s_flags &= ~STEREOMODE;
}



/******************************************************************************/
/* SetSlide1st()                                                              */
/******************************************************************************/
void SubSong::SetSlide1st(bool slide)
{
	if (slide)
		s_flags |= STSLIDE;
	else
		s_flags &= ~STSLIDE;
}



/******************************************************************************/
/* SetGM()                                                                    */
/******************************************************************************/
void SubSong::SetGM(bool gmMode)
{
	if (gmMode)
		s_flags |= GM;
	else
		s_flags &= ~GM;
}



/******************************************************************************/
/* SetFreePan()                                                               */
/******************************************************************************/
void SubSong::SetFreePan(bool fp)
{
	if (fp)
		s_flags |= FREEPAN;
	else
		s_flags &= ~FREEPAN;

	med->plr->RecalcVolAdjust();
}



/******************************************************************************/
/* SetAmigaFilter()                                                           */
/******************************************************************************/
void SubSong::SetAmigaFilter(bool amigaFilter)
{
	filter = amigaFilter;
}



/******************************************************************************/
/* SetSongName()                                                              */
/******************************************************************************/
void SubSong::SetSongName(char *name)
{
	PCharSet_Amiga charSet;

	s_name.SetString(name, &charSet);
}



/******************************************************************************/
/* SetTrackName()                                                             */
/******************************************************************************/
void SubSong::SetTrackName(TRACK_NUM trk, char *newName)
{
	PCharSet_Amiga charSet;

	s_trkName[trk].SetString(newName, &charSet);
}



/******************************************************************************/
/* NumSections()                                                              */
/******************************************************************************/
uint32 SubSong::NumSections(void)
{
	return (s_sections.CountItems());
}



/******************************************************************************/
/* NumBlocks()                                                                */
/******************************************************************************/
uint32 SubSong::NumBlocks(void)
{
	return (s_blocks.CountItems());
}



/******************************************************************************/
/* NumPlaySeqs()                                                              */
/******************************************************************************/
uint32 SubSong::NumPlaySeqs(void)
{
	return (s_pSeqs.CountItems());
}



/******************************************************************************/
/* GetTempoBPM()                                                              */
/******************************************************************************/
uint16 SubSong::GetTempoBPM(void) const
{
	return (s_tempo.tempo);
}



/******************************************************************************/
/* GetTempoTPL()                                                              */
/******************************************************************************/
uint16 SubSong::GetTempoTPL(void) const
{
	return (s_tempo.ticksPerLine);
}



/******************************************************************************/
/* GetTempo()                                                                 */
/******************************************************************************/
Tempo &SubSong::GetTempo(void)
{
	return (s_tempo);
}



/******************************************************************************/
/* GetStartTempo()                                                            */
/******************************************************************************/
Tempo &SubSong::GetStartTempo(void)
{
	return (s_startTempo);
}



/******************************************************************************/
/* GetPlayTranspose()                                                         */
/******************************************************************************/
int16 SubSong::GetPlayTranspose(void) const
{
	return (s_playTransp);
}



/******************************************************************************/
/* GetNumChannels()                                                           */
/******************************************************************************/
uint32 SubSong::GetNumChannels(void) const
{
	return (s_channels);
}



/******************************************************************************/
/* GetMasterVol()                                                             */
/******************************************************************************/
int16 SubSong::GetMasterVol(void) const
{
	return (s_masterVol);
}



/******************************************************************************/
/* GetTrackVol()                                                              */
/******************************************************************************/
int32 SubSong::GetTrackVol(TRACK_NUM trk) const
{
	return (s_trkVol[trk]);
}



/******************************************************************************/
/* GetTrackPan()                                                              */
/******************************************************************************/
int32 SubSong::GetTrackPan(TRACK_NUM trk) const
{
	return (s_trkPan[trk]);
}



/******************************************************************************/
/* GetSlide1st()                                                              */
/******************************************************************************/
bool SubSong::GetSlide1st(void) const
{
	return ((s_flags & STSLIDE) ? true : false);
}



/******************************************************************************/
/* GetAmigaFilter()                                                           */
/******************************************************************************/
bool SubSong::GetAmigaFilter(void) const
{
	return (filter);
}



/******************************************************************************/
/* GetSongName()                                                              */
/******************************************************************************/
PString SubSong::GetSongName(void) const
{
	return (s_name);
}
