/******************************************************************************/
/* Sample header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Sample_h
#define __Sample_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APList.h"


/******************************************************************************/
/* Sample class                                                               */
/******************************************************************************/
class Sample : public APAddOnPlayer
{
public:
	Sample(APGlobalData *global, PString fileName);
	virtual ~Sample(void);

	virtual float GetVersion(void);

	virtual int32 GetCount(void);
	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);
	virtual PString GetModTypeString(int32 index);

	virtual ap_result ModuleCheck(int32 index, PFile *file);
	virtual ap_result LoadModule(int32 index, PFile *file, PString &errorStr);

	virtual bool InitPlayer(int32 index);
	virtual void EndPlayer(int32 index);
	virtual void InitSound(int32 index, uint16 songNum);
	virtual void Play(void);

	virtual void GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo);
	virtual PString GetModuleName(void);
	virtual PString GetAuthor(void);
	virtual uint16 GetModuleChannels(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);

protected:
	PResource *res;

	APList<APAddOnInformation *> convInfo;
	PList<int32> convLoaders;

	APAddOnConverter *converter;
	int32 converterIndex;
	APConverter_SampleFormat sampFormat;

	float *sampBuffer;
	int16 **chanBuffers;

	PFile *sampFile;
	int32 totalLength;
	int32 samplesRead;
	int16 oldPos;
};

#endif
