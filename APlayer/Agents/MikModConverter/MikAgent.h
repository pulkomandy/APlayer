/******************************************************************************/
/* MikAgent header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MikAgent_h
#define __MikAgent_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PList.h"

// APlayerKit headers
#include "APAddOns.h"

// Agent headers
#include "MikConverter.h"


/******************************************************************************/
/* MikAgent class                                                             */
/******************************************************************************/
class MikAgent : public APAddOnAgent
{
public:
	MikAgent(APGlobalData *global, PString fileName);
	virtual ~MikAgent(void);

	virtual float GetVersion(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual bool InitAgent(int32 index);
	virtual void EndAgent(int32 index);
	virtual ap_result Run(int32 index, uint32 command, void *args);

protected:
	ap_result ConvertModule(APAgent_ConvertModule *convInfo);

	PResource *res;

	PList<MikConverter *> converters;
};

#endif
