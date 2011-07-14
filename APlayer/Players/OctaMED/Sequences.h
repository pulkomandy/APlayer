/******************************************************************************/
/* Sequences header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Sequences_h
#define __Sequences_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PList.h"

// Player headers
#include "MEDTypes.h"


/******************************************************************************/
/* SectSeqEntry class                                                         */
/******************************************************************************/
class SectSeqEntry
{
public:
	SectSeqEntry(PSEQ_NUM init);

	operator PSEQ_NUM(void);
	PSEQ_NUM operator = (const PSEQ_NUM set);

protected:
	PSEQ_NUM num;
};



/******************************************************************************/
/* PlaySeqEntry class                                                         */
/******************************************************************************/
// Playing sequence commands
#define PSEQCMD_NONE			0
#define PSEQCMD_STOP			1
#define PSEQCMD_POSJUMP			2
#define PSEQCMD_LAST			2



class PlaySeqEntry
{
public:
	PlaySeqEntry(BLOCK_NUM init);
	virtual ~PlaySeqEntry(void);

	bool IsCmd(void) const;

	int32 GetCmd(void) const;

	void SetCmd(int32 cmdNum, BLOCK_NUM cmdLevel);

	operator BLOCK_NUM(void);

protected:
	BLOCK_NUM num;
	int32 pSeqCmd;
};



/******************************************************************************/
/* PlaySeq class                                                              */
/******************************************************************************/
class PlaySeq : public PList<PlaySeqEntry *>
{
public:
	virtual ~PlaySeq(void);

	void SetName(const char *newName);

protected:
	PString name;
};

#endif
