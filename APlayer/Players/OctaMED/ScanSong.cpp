/******************************************************************************/
/* ScanSong Interface.                                                        */
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
#include "Block.h"
#include "Instr.h"
#include "Song.h"
#include "SubSong.h"
#include "ScanSong.h"


/******************************************************************************/
/* ScanTrack class                                                            */
/******************************************************************************/

/******************************************************************************/
/* DoTrack()                                                                  */
/******************************************************************************/
void ScanTrack::DoTrack(MED_Block &blk, TRACK_NUM trkNum)
{
	DoRange(blk, trkNum, 0, trkNum, blk.Lines() - 1);
}



/******************************************************************************/
/* DoRange()                                                                  */
/******************************************************************************/
void ScanTrack::DoRange(MED_Block &blk, TRACK_NUM st, LINE_NUM sl, TRACK_NUM et, LINE_NUM el)
{
	TRACK_NUM trk;
	LINE_NUM ln;
	PAGE_NUM pg;

	for (trk = st; (trk <= et) && (trk < blk.Tracks()); trk++)
	{
		lastINum = 0;

		for (ln = sl; (ln <= el) && (ln < blk.Lines()); ln++)
		{
			MED_Note &note = blk.Note(ln, trk);

			if (note.instrNum != 0)
				lastINum = note.instrNum;

			NoteOperation(note);

			for (pg = 0; pg < blk.Pages(); pg++)
			{
				CmdOperation(blk.Cmd(ln, trk, pg));
				NoteCmdOperation(note, blk.Cmd(ln, trk, pg));
			}
		}
	}
}





/******************************************************************************/
/* ScanBlock class                                                            */
/******************************************************************************/

/******************************************************************************/
/* DoBlock()                                                                  */
/******************************************************************************/
void ScanBlock::DoBlock(MED_Block &blk)
{
	TRACK_NUM trk;

	for (trk = 0; trk < blk.Tracks(); trk++)
		ScanTrack::DoTrack(blk, trk);
}





/******************************************************************************/
/* ScanSubSong class                                                          */
/******************************************************************************/

/******************************************************************************/
/* DoSubSong()                                                                */
/******************************************************************************/
void ScanSubSong::DoSubSong(SubSong &ss)
{
	BLOCK_NUM blk;

	SubSongOperation(ss);
	for (blk = 0; blk < ss.NumBlocks(); blk++)
		ScanBlock::DoBlock(ss.Block(blk));
}





/******************************************************************************/
/* ScanSong class                                                             */
/******************************************************************************/

/******************************************************************************/
/* DoSong()                                                                   */
/******************************************************************************/
void ScanSong::DoSong(Song &sg)
{
	uint32 sNum;

	for (sNum = 0; sNum < sg.NumSubSongs(); sNum++)
		ScanSubSong::DoSubSong(*sg.GetSubSong(sNum));
}





/******************************************************************************/
/* ScanSongConvertToMixMode class                                             */
/******************************************************************************/

/******************************************************************************/
/* Do()                                                                       */
/******************************************************************************/
void ScanSongConvertToMixMode::Do(Song &sg, bool isType0)
{
	uint32 cnt;

	type0 = isType0;

	// Check which instruments will be transposed...
	transpInstr[0] = true;
	iTrans[0]      = 0;

	for (cnt = 1; cnt <= MAX_INSTRS; cnt++)
	{
		Instr *i = sg.GetInstr(cnt - 1);

		if (!sg.SampleSlotUsed(cnt - 1))
			transpInstr[cnt] = true;
		else
		{
			if ((!sg.GetSample(cnt - 1)->IsSynthSound()) || (sg.GetSample(cnt - 1)->GetLength() != 0))
				transpInstr[cnt] = true;
			else
				transpInstr[cnt] = false;
		}

		iTrans[cnt] = i->GetTransp();
		isMidi[cnt] = i->IsMIDI();
	}

	ScanSong::DoSong(sg);
}



/******************************************************************************/
/* NoteOperation()                                                            */
/******************************************************************************/
void ScanSongConvertToMixMode::NoteOperation(MED_Note &note)
{
	if ((note.noteNum != 0) && (note.noteNum <= (0x7f - 24)) && (transpInstr[lastINum]))
	{
		if (isMidi[lastINum])
		{
			if (type0)
				note.noteNum += 24;		// For MMD0 mods, transpose 2 oct, otherwise nothing
		}
		else
		{
			// Kludge for broken 4-ch modules that use x-4/x-5/x-6... as x-3
			while ((int32)note.noteNum + iTrans[lastINum] > 3 * 12)
				note.noteNum -= 12;

			// The actual transposition up
			note.noteNum += 24;
		}
	}
}



/******************************************************************************/
/* SubSongOperation()                                                         */
/******************************************************************************/
void ScanSongConvertToMixMode::SubSongOperation(SubSong &ss)
{
	static int8 panVals[8] = { -16, 16, 16, -16, -16, 16, 16, -16 };
	uint32 cnt;

	// For each subsong, set panning & stereo mode
	ss.SetStereo(true);

	for (cnt = 0; cnt < 8; cnt++)
		ss.SetTrackPan(cnt, panVals[cnt]);
}





/******************************************************************************/
/* ScanSongConvertTempo class                                                 */
/******************************************************************************/
const uint8 ScanSongConvertTempo::bpmVals[9] =
{
	179, 164, 152, 141, 131, 123, 116, 110, 104
};



/******************************************************************************/
/* CmdOperation()                                                             */
/******************************************************************************/
void ScanSongConvertTempo::CmdOperation(MED_Cmd &cmd)
{
	uint8 data;

	if ((cmd.GetCmd() == 0x0f) && (data = cmd.GetDataB()) && (data <= 240))
		cmd.SetData(data > 10 ? 99 : bpmVals[data - 1]);
}



/******************************************************************************/
/* SubSongOperation()                                                         */
/******************************************************************************/
void ScanSongConvertTempo::SubSongOperation(SubSong &ss)
{
	uint16 oldTempo;

	ss.SetTempoMode(true);		// BPM tempo
	ss.SetTempoLPB(4);

	oldTempo = ss.GetTempoBPM();
	ss.SetTempoBPM(oldTempo >= 10 ? 99 : bpmVals[oldTempo - 1]);
}





/******************************************************************************/
/* ScanConvertOldVolToNew class                                               */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ScanConvertOldVolToNew::ScanConvertOldVolToNew(bool volHex)
{
	hex = volHex;
}



/******************************************************************************/
/* CmdOperation()                                                             */
/******************************************************************************/
void ScanConvertOldVolToNew::CmdOperation(MED_Cmd &cmd)
{
	if (cmd.GetCmd() == 0x0c)
	{
		uint8 data;

		if ((data = cmd.GetData2()) != 0)
			cmd.SetData(data, 0);
		else
		{
			if ((data = cmd.GetDataB()) != 0)
			{
				if (!hex)
					data = min((data >> 4) * 10 + (data & 0x0f), 64);

				cmd.SetData(data * 2 - 1, 0);
			}
		}
	}
}
