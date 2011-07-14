/******************************************************************************/
/* SpinSquare Agent header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SpinSquareAgent_h
#define __SpinSquareAgent_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSynchronize.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* SpinSquareAgent class                                                      */
/******************************************************************************/
class SpinSquareAgent : public APAddOnAgent
{
public:
	SpinSquareAgent(APGlobalData *global, PString fileName);
	virtual ~SpinSquareAgent(void);

	virtual float GetVersion(void);

	virtual const APDisplayInfo *GetDisplayInfo(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual ap_result Run(int32 index, uint32 command, void *args);

	void WindowClosed(void);

protected:
	void ChannelChange(APAgent_ChannelChange *channelInfo);
	void StopShowing(void);

	PResource *res;
	PMutex infoLock;
	APDisplayInfo displayInfo;
};

#endif
