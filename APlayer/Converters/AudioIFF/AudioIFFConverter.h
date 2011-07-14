/******************************************************************************/
/* AudioIFF Converter header file.                                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __AudioIFFConverter_h
#define __AudioIFFConverter_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"


/******************************************************************************/
/* AudioIFFConverter class                                                    */
/******************************************************************************/
class AudioIFFConverter : public APAddOnConverter
{
public:
	AudioIFFConverter(APGlobalData *global, PString fileName);
	virtual ~AudioIFFConverter(void);

	virtual float GetVersion(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual PString GetExtension(int32 index);
	virtual PString GetTypeString(int32 index);

	// Loader functions
	virtual ap_result FileCheck(int32 index, PFile *file);
	virtual bool LoaderInit(int32 index);
	virtual void LoaderEnd(int32 index);

	virtual ap_result LoadHeader(PFile *file, APConverter_SampleFormat *convInfo, PString &errorStr);
	virtual uint32 LoadData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

	virtual uint32 GetTotalSampleLength(const APConverter_SampleFormat *convInfo);
	virtual uint32 SetSamplePosition(PFile *file, uint32 position, const APConverter_SampleFormat *convInfo);

	// Saver functions
	virtual bool SaverInit(int32 index, const APConverter_SampleFormat *convInfo);
	virtual void SaverEnd(int32 index, const APConverter_SampleFormat *convInfo);

	virtual ap_result SaveHeader(PFile *file, const APConverter_SampleFormat *convInfo);
	virtual ap_result SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);
	virtual ap_result SaveTail(PFile *file, const APConverter_SampleFormat *convInfo);

protected:
	void ConvertToIeeeExtended(double num, uint8 *bytes);
	double ConvertFromIeeeExtended(const uint8 *bytes);

	PResource *res;

	// Loader variables
	int8 *loadBuffer;
	uint32 fillCount;
	uint32 offset;

	int64 ssndStart;
	uint32 ssndLength;

	// Saver variables
	int8 *saveBuffer;
	uint32 bufLength;
	uint32 total;

	uint32 commPos;
	uint32 ssndPos;
};

#endif
