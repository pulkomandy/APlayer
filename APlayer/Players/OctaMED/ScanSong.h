/******************************************************************************/
/* ScanSong header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ScanSong_h
#define __ScanSong_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "Block.h"
#include "Song.h"
#include "SubSong.h"


/******************************************************************************/
/* ScanTrack class                                                            */
/******************************************************************************/
class ScanTrack
{
public:
	virtual void DoTrack(MED_Block &blk, TRACK_NUM trkNum);

	void DoRange(MED_Block &blk, TRACK_NUM st, LINE_NUM sl, TRACK_NUM et, LINE_NUM el);

protected:
	virtual void CmdOperation(MED_Cmd &cmd) {};
	virtual void NoteOperation(MED_Note &note) {};
	virtual void NoteCmdOperation(MED_Note &note, MED_Cmd &cmd) {};

	INST_NUM lastINum;			// Last instrument number (for instr. 0)
};



/******************************************************************************/
/* ScanBlock class                                                            */
/******************************************************************************/
class ScanBlock : public ScanTrack
{
public:
	virtual void DoBlock(MED_Block &blk);
};



/******************************************************************************/
/* ScanSubSong class                                                          */
/******************************************************************************/
class ScanSubSong : public ScanBlock
{
public:
	virtual void DoSubSong(SubSong &ss);

protected:
	virtual void SubSongOperation(SubSong &ss) {};
};



/******************************************************************************/
/* ScanSong class                                                             */
/******************************************************************************/
class ScanSong : public ScanSubSong
{
public:
	virtual void DoSong(Song &sg);
};



/******************************************************************************/
/* ScanSongConvertToMixMode class                                             */
/******************************************************************************/
class ScanSongConvertToMixMode : public ScanSong
{
public:
	virtual void Do(Song &sg, bool isType0);

protected:
	virtual void NoteOperation(MED_Note &note);
	virtual void SubSongOperation(SubSong &ss);

	bool transpInstr[MAX_INSTRS + 1];
	int32 iTrans[MAX_INSTRS + 1];
	bool isMidi[MAX_INSTRS + 1];
	bool type0;
};



/******************************************************************************/
/* ScanSongConvertTempo class                                                 */
/******************************************************************************/
class ScanSongConvertTempo : public ScanSong
{
protected:
	virtual void CmdOperation(MED_Cmd &cmd);
	virtual void SubSongOperation(SubSong &ss);

	static const uint8 bpmVals[9];
};



/******************************************************************************/
/* ScanConvertOldVolToNew class                                               */
/******************************************************************************/
class ScanConvertOldVolToNew : public ScanBlock
{
public:
	ScanConvertOldVolToNew(bool volHex);

protected:
	virtual void CmdOperation(MED_Cmd &cmd);

	bool hex;
};

#endif
