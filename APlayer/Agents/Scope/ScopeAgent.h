/******************************************************************************/
/* Scope Agent header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ScopeAgent_h
#define __ScopeAgent_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSynchronize.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* ScopeAgent class                                                           */
/******************************************************************************/
class ScopeAgent : public APAddOnAgent
{
public:
	ScopeAgent(APGlobalData *global, PString fileName);
	virtual ~ScopeAgent(void);

	virtual float GetVersion(void);

	virtual const APDisplayInfo *GetDisplayInfo(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual ap_result Run(int32 index, uint32 command, void *args);

	void WindowClosed(void);

protected:
	void MixedData(APAgent_MixedData *mixedData);
	void StopShowing(void);

	PResource *res;
	PMutex infoLock;
	APDisplayInfo displayInfo;
};

#endif
