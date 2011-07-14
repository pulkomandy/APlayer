/******************************************************************************/
/* MediaKit Agent header file.                                                */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MediaKitAgent_h
#define __MediaKitAgent_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* MediaKitAgent class                                                        */
/******************************************************************************/
class MediaKitAgent : public APAddOnAgent
{
public:
	MediaKitAgent(APGlobalData *global, PString addOnName);
	virtual ~MediaKitAgent(void);

	virtual float GetVersion(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual ap_result Run(int32 index, uint32 command, void *args);

protected:
	ap_result InitHardware(APAgent_InitHardware *args);
	void EndHardware(void);

	void GetOutputInformation(APAgent_OutputInfo *outputInfo);
	void SetVolume(APAgent_SetVolume *setVolume);

	void StartPlaying(void);
	void StopPlaying(void);

	static void EnterStream(void *theCookie, void *buffer, size_t size, const media_raw_audio_format &format);

	PResource *res;

	Mixer theMixer;
	void *mixerHandle;

	BSoundPlayer *soundPlayer;

	int16 *sampBuffer;		// Pointer to the mixer buffer
	int32 sampBufLen;		// Sample buffer length
	int16 volume;			// The output volume
};

#endif
