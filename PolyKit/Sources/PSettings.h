/******************************************************************************/
/* PSettings header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PSettings_h
#define __PSettings_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSynchronize.h"
#include "PList.h"
#include "ImportExport.h"


/******************************************************************************/
/* PSettings class                                                            */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PSettings
{
public:
	PSettings(void);
	virtual ~PSettings(void);

	void LoadFile(PString fileName, PString company, PString product);
	void SaveFile(PString fileName, PString company, PString product);
	void CloneSettings(const PSettings *source);

	void SetChangeFlag(bool flag);
	bool HasChanged(void) const;

	PString GetStringEntryValue(PString section, PString entry, PString defaultValue = "") const;
	PString GetStringEntryValue(PString section, int32 entryNum, PString &entryName, PString defaultValue = "") const;
	int32 GetIntEntryValue(PString section, PString entry, int32 defaultValue = 0) const;
	int32 GetIntEntryValue(PString section, int32 entryNum, PString &entryName, int32 defaultValue = 0) const;

	void WriteStringEntryValue(PString section, PString entry, PString value);
	void WriteIntEntryValue(PString section, PString entry, int32 value);

	bool EntryExist(PString section, PString entry) const;

	bool RemoveEntry(PString section, PString entry);
	bool RenameEntry(PString section, PString entry, PString newEntry);

protected:
	enum PFileLineType { pComment, pSection, pEntry };

	typedef struct PFileLine
	{
		PFileLineType type;
		PString line;
	} PFileLine;

	int32 FindSection(PString section) const;
	int32 FindEntry(int32 startIndex, PString entry, int32 &insertPos) const;
	int32 FindEntryByNumber(int32 startIndex, int32 entryNum, int32 &insertPos) const;

	PMRSWLock *listLock;
	PList<PFileLine> lines;
	bool changed;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
