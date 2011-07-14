/******************************************************************************/
/* APChannelParser header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APChannelParser_h
#define __APChannelParser_h

// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "APChannel.h"
#include "APAddOns.h"

// Server headers
#include "APMixerBase.h"


/******************************************************************************/
/* APChannelParser class                                                      */
/******************************************************************************/
class APChannelParser : public APChannel
{
public:
	APChannelParser(void);
	virtual ~APChannelParser(void);

	void ParseSampleInfo(VINFO *info, int32 clickBuffer);
	uint32 ParseInfo(VINFO *info, int32 clickBuffer);
	void Active(bool active);

	void SetSampleInfo(APSamplePlayerInfo *sampInfo);

protected:
	uint32 privateFlags;
	APSamplePlayerInfo samplePlayInfo;
};

#endif
