/******************************************************************************/
/* SIDStil header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDStil_h
#define __SIDStil_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"
#include "PList.h"


/******************************************************************************/
/* SIDStil class                                                              */
/******************************************************************************/
class SIDStil
{
public:
	enum STILField { all, name, author, title, artist, comment };

	SIDStil(void);
	virtual ~SIDStil(void);

	bool SetBaseDir(PString pathToHVSC);
	PString GetBaseDir(void) const;

	PString GetGlobalComment(PString relPathToEntry);
	PString GetAbsGlobalComment(PString absPathToEntry);

	PString GetEntry(PString relPathToEntry, int32 tuneNo = 0, STILField field = all);
	PString GetAbsEntry(PString absPathToEntry, int32 tuneNo = 0, STILField field = all);

	PString GetBug(PString relPathToEntry, int32 tuneNo = 0);
	PString GetAbsBug(PString absPathToEntry, int32 tuneNo = 0);

protected:
	typedef struct DirList
	{
		PString dirName;
		int32 position;
	} DirList;

	bool GetDirs(PFile *inFile, PList<DirList *> &dirs, bool isStilFile);
	void DeleteDirList(PList<DirList *> &dirs);
	void CopyDirList(PList<DirList *> &toList, PList<DirList *> &fromList);

	bool PositionToEntry(PString entryStr, PFile *inFile, PList<DirList *> &dirs);
	void ReadEntry(PFile *inFile, PString &buffer);

	bool GetField(PString &result, PString buffer, int32 tuneNo = 0, STILField field = all);
	bool GetOneField(PString &result, PString buffer, STILField field);

	float stilVersion;

	PString baseDir;

	PCacheFile *stilFile;
	PCacheFile *bugFile;

	PList<DirList *> stilDirs;
	PList<DirList *> bugDirs;

	PString globalBuf;
	PString entryBuf;
	PString bugBuf;

	PString resultEntry;
	PString resultBug;
};

#endif
