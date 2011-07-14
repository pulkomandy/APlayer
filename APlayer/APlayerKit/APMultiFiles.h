/******************************************************************************/
/* APMultiFiles header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APMultiFiles_h
#define __APMultiFiles_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PTime.h"

// APlayerKit headers
#include "Import_Export.h"


/******************************************************************************/
/* APMultiFiles class                                                         */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_APKIT APMultiFiles
{
public:
	enum MultiFileType { apPlain, apList, /*apArchive*/ };

	typedef struct APMultiFileType
	{
		MultiFileType type;
		PString fileName;
		bool timeAvailable;
		PTimeSpan time;
//		PString archiveName;
	} APMultiFileType;

	// Callback function
	// On read: Return true to continue with the next file, false to stop loading
	// On write: Return true if you have returned a file type, false indicates no more entries
	typedef bool (*FileFunc)(APMultiFileType *, void *);

	// Functions
	APMultiFiles(void);
	virtual ~APMultiFiles(void);

	MultiFileType CheckForMultiFile(PString fileName);
	void GetMultiFiles(PString fileName, FileFunc func, void *userData);
	void SaveAPMLFile(PString fileName, FileFunc func, void *userData);

protected:
	enum ListFileType { apUnknown, apAPlayer, apCL_Amp, apSoundPlay, apWinAmp };

	ListFileType CheckForListFile(PString fileName);
	void LoadListFile(PString fileName, FileFunc func, void *userData);
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
