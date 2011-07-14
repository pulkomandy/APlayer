/******************************************************************************/
/* Block header file.                                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Block_h
#define __Block_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// Player headers
#include "MEDTypes.h"


/******************************************************************************/
/* MED_Note class                                                             */
/******************************************************************************/
// Special note numbers
#define NOTE_STP		0x80
#define NOTE_DEF		0x81
#define NOTE_11k		0x82
#define NOTE_22k		0x83
#define NOTE_44k		0x84
#define MAX_NOTENUM		0x84		// The highest currently supported note number

class MED_Note
{
public:
	NOTE_NUM noteNum;
	uint8 instrNum;
};



/******************************************************************************/
/* MED_Cmd class                                                              */
/******************************************************************************/
class MED_Cmd
{
public:
	void SetCmdData(uint8 cmd, uint8 data, uint8 data2);
	void SetData(uint8 data, uint8 data2);
	void SetData(uint8 data);
	void SetData2(uint8 data);

	uint8 GetCmd(void) const;
	uint16 GetData(void) const;
	uint8 GetDataB(void) const;
	uint8 GetData2(void) const;

protected:
	uint8 cmdNum;
	uint8 data0;
	uint8 data1;
	uint8 pad;			// Longword align
};



/******************************************************************************/
/* MED_Block class                                                            */
/******************************************************************************/
class MED_Block
{
public:
	MED_Block(LINE_NUM lines, TRACK_NUM tracks, PAGE_NUM pages = 1);
	virtual ~MED_Block(void);

	void SetName(const char *newName);
	void SetCmdPages(PAGE_NUM numOfPages);

	MED_Note &Note(LINE_NUM line, TRACK_NUM track);
	MED_Cmd &Cmd(LINE_NUM line, TRACK_NUM track, PAGE_NUM page);

	LINE_NUM Lines(void) const;
	TRACK_NUM Tracks(void) const;
	PAGE_NUM Pages(void) const;

protected:
	PString name;

	MED_Note *grid;
	MED_Cmd **cmdPages;

	LINE_NUM numLines;
	TRACK_NUM numTracks;
	PAGE_NUM numCmdPages;
};

#endif
