/******************************************************************************/
/* DiskSaver Agent header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __DiskSaverAgent_h
#define __DiskSaverAgent_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PThread.h"
#include "PSynchronize.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* DiskSaverAgent class                                                       */
/******************************************************************************/
class DiskSaverAgent : public APAddOnAgent
{
public:
	DiskSaverAgent(APGlobalData *global, PString addOnName);
	virtual ~DiskSaverAgent(void);

	virtual float GetVersion(void);

	virtual const APConfigInfo *GetConfigInfo(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual bool InitAgent(int32 index);
	virtual ap_result Run(int32 index, uint32 command, void *args);

protected:
	void LoadSettings(void);
	void FixSettings(void);
	void InitSaver(APAgent_InitHardware *args);
	void EndSaver(void);
	int32 DoMixing(int16 *buffer, int32 bufSize);

	ap_result InitHardware(APAgent_InitHardware *args);
	void EndHardware(void);
	void GetOutputInformation(APAgent_OutputInfo *outputInfo);
	void StartPlaying(void);
	void StopPlaying(void);
	void PausePlaying(void);
	void ResumePlaying(void);
	void SetVolume(APAgent_SetVolume *setVolume);

	static int32 MixerThread(void *userData);
	static int32 AgentFunc(void *handle, int16 *buffer, int32 length);

	PResource *res;
	APConfigInfo cfgInfo;

	Mixer theMixer;
	void *mixerHandle;

	PFile *file;
	APConverter_SampleFormat convInfo;
	PString converterName;
	APAddOnConverter *converter;
	int32 converterIndex;

	PThread saveThread;
	PEvent *endEvent;
	PEvent *pauseEvent;

	PString soundAgentName;
	APAddOnAgent *soundAgent;
	int32 soundAgentIndex;

	int16 *mixBuffer;
	float *saveBuffer;
	int16 volume;
};

#endif
