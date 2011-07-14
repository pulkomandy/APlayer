/******************************************************************************/
/* APModuleLoader header file.                                                */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APModuleLoader_h
#define __APModuleLoader_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* APModuleLoader class                                                       */
/******************************************************************************/
struct AddOnInfo;

class APModuleLoader
{
public:
	APModuleLoader(void);
	virtual ~APModuleLoader(void);

	bool LoadModule(PString fileName, bool changeType, PString &errorStr);
	void FreeModule(void);

	APAddOnPlayer *GetPlayer(int32 &index) const;

	PString GetModuleFormat(void) const;
	PString GetPlayerName(void) const;
	uint32 GetModuleSize(void) const;

protected:
	bool FindPlayerViaFileType(PFile *modFile);
	bool FindPlayer(PFile *modFile);
	bool ConvertModule(APAgent_ConvertModule *convInfo);
	void DecrunchFile(APAgent_DecrunchFile *decrunchInfo);

	PCacheFile *file;
	PFile *usingFile;
	uint32 fileLength;
	PString moduleFormat;
	PString playerName;

	const AddOnInfo *playerInfo;
	APAddOnPlayer *player;
};

#endif
