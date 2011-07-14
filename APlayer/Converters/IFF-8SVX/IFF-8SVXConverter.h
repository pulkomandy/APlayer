/******************************************************************************/
/* IFF-8SVX Converter header file.                                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __IFF8SVXConverter_h
#define __IFF8SVXConverter_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define BUFFER_SIZE						16384



/******************************************************************************/
/* Sample formats                                                             */
/******************************************************************************/
#define IFF8SVX_FORMAT_PCM				0x00	// Plain PCM
#define IFF8SVX_FORMAT_FIBONNACI		0x01	// Fibonnaci Delta Encoding



/******************************************************************************/
/* IFF8SVXFormat class                                                        */
/******************************************************************************/
class IFF8SVXFormat
{
public:
	IFF8SVXFormat(PResource *resource);
	virtual ~IFF8SVXFormat(void);

	// Loader functions
	virtual void LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo) = 0;
	virtual void LoaderCleanup(void) = 0;
	virtual void ResetLoader(uint32 position);

	virtual uint32 DecodeSampleData(PFile *file, PFile *file2, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo) = 0;

	virtual uint32 GetTotalSampleLength(const APConverter_SampleFormat *convInfo) = 0;
	virtual int64 GetRightChannelPosition(uint32 totalLen) = 0;
	virtual uint32 CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo) = 0;
	virtual uint32 CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo) = 0;

	void InitBasicLoader(void);
	void CleanupBasicLoader(void);
	void ResetBasicLoader(void);

	// Saver functions
	virtual void SaverInitialize(const APConverter_SampleFormat *convInfo);
	virtual void SaverCleanup(void);

	virtual uint16 GetFormatNumber(void) const;

	virtual uint32 WriteData(PFile *file, PFile *stereoFile, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo);
	virtual uint32 WriteEnd(PFile *file, PFile *stereoFile, const APConverter_SampleFormat *convInfo);

protected:
	// Helper functions
	uint32 GetFileData1(PFile *file, int8 *buffer, uint32 length);
	uint32 GetFileData2(PFile *file, int8 *buffer, uint32 length);

	PResource *res;

	// Loader variables
	uint32 samplesLeft;

private:
	// Loader variables
	int8 *loadBuffer1;
	uint32 loadBuffer1FillCount;
	uint32 loadBuffer1Offset;

	int8 *loadBuffer2;
	uint32 loadBuffer2FillCount;
	uint32 loadBuffer2Offset;
};



/******************************************************************************/
/* IFF8SVXPCM class                                                           */
/******************************************************************************/
class IFF8SVXPCM : public IFF8SVXFormat
{
public:
	IFF8SVXPCM(PResource *resource);
	virtual ~IFF8SVXPCM(void);

	// Loader functions
	virtual void LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo);
	virtual void LoaderCleanup(void);

	virtual uint32 DecodeSampleData(PFile *file, PFile *file2, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

	virtual uint32 GetTotalSampleLength(const APConverter_SampleFormat *convInfo);
	virtual int64 GetRightChannelPosition(uint32 totalLen);
	virtual uint32 CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo);
	virtual uint32 CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo);

	// Saver functions
	virtual uint16 GetFormatNumber(void) const;

	virtual uint32 WriteData(PFile *file, PFile *stereoFile, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

protected:
	// Loader variables
	uint32 fileSize;

	int8 *decodeBuffer;
	uint32 offset;
};



/******************************************************************************/
/* IFF8SVXFibonnaci class                                                     */
/******************************************************************************/
class IFF8SVXFibonnaci : public IFF8SVXFormat
{
public:
	// Constructor / Destructor
	IFF8SVXFibonnaci(PResource *resource);
	virtual ~IFF8SVXFibonnaci(void);

	// Loader functions
	virtual void LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo);
	virtual void LoaderCleanup(void);
	virtual void ResetLoader(uint32 position);

	virtual uint32 DecodeSampleData(PFile *file, PFile *file2, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

	virtual uint32 GetTotalSampleLength(const APConverter_SampleFormat *convInfo);
	virtual int64 GetRightChannelPosition(uint32 totalLen);
	virtual uint32 CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo);
	virtual uint32 CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo);

	// Saver functions
	virtual void SaverInitialize(const APConverter_SampleFormat *convInfo);

	virtual uint16 GetFormatNumber(void) const;

	virtual uint32 WriteData(PFile *file, PFile *stereoFile, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

protected:
	int8 DecodeBuffer(uint8 *source, float *dest, uint32 todo, int8 startVal, bool skip);
	uint8 GetNextNibble(int8 sample, int8 &lastVal);

	// Loader variables
	uint32 decompressedSize;

	int8 *decodeBuffer;
	uint32 offset;

	bool loadFirstBuf;
	int8 startVal1;
	int8 startVal2;

	// Saver variables
	bool saveFirstBuf;
	int8 lastVal1;
	int8 lastVal2;
};



/******************************************************************************/
/* IFF8SVXConverter class                                                     */
/******************************************************************************/
class IFF8SVXConverter : public APAddOnConverter
{
public:
	IFF8SVXConverter(APGlobalData *global, PString fileName);
	virtual ~IFF8SVXConverter(void);

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

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);

	// Saver functions
	virtual BWindow *ShowSaverSettings(void);

	virtual bool SaverInit(int32 index, const APConverter_SampleFormat *convInfo);
	virtual void SaverEnd(int32 index, const APConverter_SampleFormat *convInfo);

	virtual ap_result SaveHeader(PFile *file, const APConverter_SampleFormat *convInfo);
	virtual ap_result SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);
	virtual ap_result SaveTail(PFile *file, const APConverter_SampleFormat *convInfo);

protected:
	PResource *res;
	IFF8SVXFormat *format;

	// Loader variables
	PFile *file2;

	int64 dataStart1;
	int64 dataStart2;

	uint16 sampFormat;

	PString copyright;
	PString annotation;
	uint8 octaves;

	uint32 totalLength;

	// Saver variables
	PFile *stereoFile;

	int8 *saveBuffer;
	uint32 bufLength;
	uint32 total;
	uint32 loopLength;

	uint32 dataPos;
};

#endif
