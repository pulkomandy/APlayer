/******************************************************************************/
/* TagsLoad header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __TagsLoad_h
#define __TagsLoad_h

// PolyKit headers
#include "POS.h"
#include "PFile.h"

// Player headers
#include "MEDTypes.h"


/******************************************************************************/
/* Tag values                                                                 */
/******************************************************************************/
// Generic MMD tags
#define MMDTAG_END						0
#define MMDTAG_PTR						0x80000000	// Data needs relocation
#define MMDTAG_MUSTKNOW					0x40000000	// Loader must fail if this isn't recognized
#define MMDTAG_MUSTWARN					0x20000000	// Loader must warn if this isn't recognized

// ExpData tags
//
// # of effect groups, including the global group (will
// override settings in MMDSong struct), default = 1
#define MMDTAG_EXP_NUMFXGROUPS			1

// Track info tags
#define MMDTAG_TRK_NAME					MMDTAG_PTR | 1
#define MMDTAG_TRK_NAMELEN				2			// Includes zero terminator
#define MMDTAG_TRK_FXGROUP				3

// Effect info tags
#define MMDTAG_FX_ECHOTYPE				1
#define MMDTAG_FX_ECHOLEN				2
#define MMDTAG_FX_ECHODEPTH				3
#define MMDTAG_FX_STEREOSEP				4
#define MMDTAG_FX_GROUPNAME				MMDTAG_PTR | 5	// The Global Effects group shouldn't have name saved!
#define MMDTAG_FX_GRPNAMELEN			6			// Includes zero terminator



/******************************************************************************/
/* TagsLoad class                                                             */
/******************************************************************************/
class TagsLoad
{
public:
	TagsLoad(PFile *f);
	virtual ~TagsLoad(void);

	bool TagExists(uint32 tagNum);
	uint32 TagVal(uint32 tagNum);
	uint32 TagVal(uint32 tagNum, uint32 defVal);
	void SeekToTag(uint32 tagNum);

protected:
	PFile *file;

	uint32 numTags;
	uint32 *tags;
	bool *tagChecked;
};

#endif
