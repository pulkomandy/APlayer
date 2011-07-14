/******************************************************************************/
/* RIFF-WAVE Converter header file.                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __RIFFWAVEConverter_h
#define __RIFFWAVEConverter_h

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
#define WAVE_FORMAT_G723_ADPCM			0x0014	// Antex Electronics Corporation
#define WAVE_FORMAT_ANTEX_ADPCME		0x0033	// Antex Electronics Corporation
#define WAVE_FORMAT_G721_ADPCM			0x0040	// Antex Electronics Corporation
#define WAVE_FORMAT_APTX				0x0025	// Audio Processing Technology
#define WAVE_FORMAT_AUDIOFILE_AF36		0x0024	// Audio File Inc.
#define WAVE_FORMAT_AUDIOFILE_AF10		0x0026	// Audio File Inc.
#define WAVE_FORMAT_CONTROL_RES_VQLPC	0x0034	// Control Resources Limited
#define WAVE_FORMAT_CONTROL_RES_CR10	0x0037	// Control Resources Limited
#define WAVE_FORMAT_CREATIVE_ADPCM		0x0200	// Creative Labs, Inc.
#define WAVE_FORMAT_DOLBY_AC2			0x0030	// Dolby Laboratories
#define WAVE_FORMAT_DSPGROUP_TRUESPEECH	0x0022	// DSP Group, Inc.
#define WAVE_FORMAT_DIGISTD				0x0015	// DSP Solutions, Inc.
#define WAVE_FORMAT_DIGIFIX				0x0016	// DSP Solutions, Inc.
#define WAVE_FORMAT_DIGIREAL			0x0035	// DSP Solutions, Inc.
#define WAVE_FORMAT_DIGIADPCM			0x0036	// DSP Solutions, Inc.
#define WAVE_FORMAT_ECHOSC1				0x0023	// Echo Speech Corporation
#define WAVE_FORMAT_FM_TOWNS_SND		0x0300	// Fujitsu Corp.
#define WAVE_FORMAT_IBM_CVSD			0x0005	// IBM Corporation
#define WAVE_FORMAT_OLIGSM				0x1000	// Ing. C. Olivetti & C., S.p.A.
#define WAVE_FORMAT_OLIADPCM			0x1001	// Ing. C. Olivetti & C., S.p.A.
#define WAVE_FORMAT_OLICELP				0x1002	// Ing. C. Olivetti & C., S.p.A.
#define WAVE_FORMAT_OLISBC				0x1003	// Ing. C. Olivetti & C., S.p.A.
#define WAVE_FORMAT_OLIOPR				0x1004	// Ing. C. Olivetti & C., S.p.A.
#define WAVE_FORMAT_IMA_ADPCM			0x0011	// Intel Corporation
#define WAVE_FORMAT_DVI_ADPCM			0x0011	// Intel Corporation
#define WAVE_FORMAT_UNKNOWN				0x0000	// Microsoft Corporation
#define WAVE_FORMAT_PCM					0x0001	// Microsoft Corporation
#define WAVE_FORMAT_ADPCM				0x0002	// Microsoft Corporation
#define WAVE_FORMAT_ALAW				0x0006	// Microsoft Corporation
#define WAVE_FORMAT_MULAW				0x0007	// Microsoft Corporation
#define WAVE_FORMAT_GSM610				0x0031	// Microsoft Corporation
#define WAVE_FORMAT_MPEG				0x0050	// Microsoft Corporation
#define WAVE_FORMAT_NMS_VBXADPCM		0x0038	// Natural MicroSystems
#define WAVE_FORMAT_OKI_ADPCM			0x0010	// OKI
#define WAVE_FORMAT_SIERRA_ADPCM		0x0013	// Sierra Semiconductor Corp
#define WAVE_FORMAT_SONARC				0x0021	// Speech Compression
#define WAVE_FORMAT_MEDIASPACE_ADPCM	0x0012	// Videologic
#define WAVE_FORMAT_YAMAHA_ADPCM		0x0020	// Yamaha Corporation of America



/******************************************************************************/
/* RIFFWAVEFormat class                                                       */
/******************************************************************************/
class RIFFWAVEFormat
{
public:
	RIFFWAVEFormat(PResource *resource);
	virtual ~RIFFWAVEFormat(void);

	// Loader functions
	virtual void LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo) = 0;
	virtual void LoaderCleanup(void) = 0;

	virtual uint32 LoadExtraHeaderInfo(PFile *file, const APConverter_SampleFormat *convInfo, PString &errorStr);
	virtual uint32 LoadFactChunk(PFile *file);
	virtual uint32 DecodeSampleData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo) = 0;

	virtual uint32 GetTotalSampleLength(const APConverter_SampleFormat *convInfo) = 0;
	virtual uint32 CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo) = 0;
	virtual uint32 CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo) = 0;

	void InitBasicLoader(uint16 blkAlign);
	void CleanupBasicLoader(void);
	void ResetBasicLoader(void);

	// Saver functions
	virtual void SaverInitialize(const APConverter_SampleFormat *convInfo);
	virtual void SaverCleanup(void);

	virtual uint16 GetFormatNumber(void) const;
	virtual uint32 GetAverageBytesSecond(const APConverter_SampleFormat *convInfo);
	virtual uint16 GetBlockAlign(const APConverter_SampleFormat *convInfo);
	virtual uint16 GetSampleSize(uint16 sampSize);

	virtual void WriteExtraFmtInfo(PFile *file);
	virtual void WriteFactChunk(PFile *file);
	virtual uint32 WriteData(PFile *file, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo);
	virtual uint32 WriteEnd(PFile *file, const APConverter_SampleFormat *convInfo);

protected:
	// Helper functions
	uint32 GetFileData(PFile *file, int8 *buffer, uint32 length);

	PResource *res;

	// Loader variables
	uint32 samplesLeft;
	uint16 blockAlign;

private:
	// Loader variables
	int8 *loadBuffer;
	uint32 loadBufferFillCount;
	uint32 loadBufferOffset;
};



/******************************************************************************/
/* RIFFWAVEMicrosoftPCM class                                                 */
/******************************************************************************/
class RIFFWAVEMicrosoftPCM : public RIFFWAVEFormat
{
public:
	RIFFWAVEMicrosoftPCM(PResource *resource);
	virtual ~RIFFWAVEMicrosoftPCM(void);

	// Loader functions
	virtual void LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo);
	virtual void LoaderCleanup(void);

	virtual uint32 DecodeSampleData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

	virtual uint32 GetTotalSampleLength(const APConverter_SampleFormat *convInfo);
	virtual uint32 CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo);
	virtual uint32 CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo);

	// Saver functions
	virtual uint16 GetFormatNumber(void) const;
	virtual uint32 GetAverageBytesSecond(const APConverter_SampleFormat *convInfo);
	virtual uint16 GetBlockAlign(const APConverter_SampleFormat *convInfo);

	virtual uint32 WriteData(PFile *file, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

protected:
	uint32 fileSize;

	int8 *decodeBuffer;
	uint32 offset;
};



/******************************************************************************/
/* RIFFWAVEMicrosoftADPCM class                                               */
/******************************************************************************/
class RIFFWAVEMicrosoftADPCM : public RIFFWAVEFormat
{
public:
	// Constructor / Destructor
	RIFFWAVEMicrosoftADPCM(PResource *resource);
	virtual ~RIFFWAVEMicrosoftADPCM(void);

	// Loader functions
	virtual void LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo);
	virtual void LoaderCleanup(void);

	virtual uint32 LoadExtraHeaderInfo(PFile *file, const APConverter_SampleFormat *convInfo, PString &errorStr);
	virtual uint32 DecodeSampleData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

	virtual uint32 GetTotalSampleLength(const APConverter_SampleFormat *convInfo);
	virtual uint32 CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo);
	virtual uint32 CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo);

	// Saver functions
	virtual void SaverInitialize(const APConverter_SampleFormat *convInfo);
	virtual void SaverCleanup(void);

	virtual uint16 GetFormatNumber(void) const;
	virtual uint32 GetAverageBytesSecond(const APConverter_SampleFormat *convInfo);
	virtual uint16 GetBlockAlign(const APConverter_SampleFormat *convInfo);
	virtual uint16 GetSampleSize(uint16 sampSize);

	virtual void WriteExtraFmtInfo(PFile *file);
	virtual void WriteFactChunk(PFile *file);
	virtual uint32 WriteData(PFile *file, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo);
	virtual uint32 WriteEnd(PFile *file, const APConverter_SampleFormat *convInfo);

protected:
	typedef struct ADPCMCOEFSET
	{
		int16 coef1;
		int16 coef2;
	} ADPCMCOEFSET;

	int16 Decode(int8 deltaCode, uint16 channel);

	uint32 EncodeBuffer(PFile *file, const APConverter_SampleFormat *convInfo);
	void AdpcmBlockMashI(int32 chans, const int16 *ip, int32 n, int32 *st, uint8 *oBuff, int32 ba);
	void AdpcmMashChannel(int32 ch, int32 chans, const int16 *ip, int32 n, int32 *st, uint8 *oBuff);
	int32 AdpcmMashS(int32 ch, int32 chans, int16 v[2], const ADPCMCOEFSET *iCoef, const int16 *iBuff, int32 n, int32 *ioStep, uint8 *oBuff);

	uint32 fileSize;

	int8 *decodeBuffer;
	uint32 offset;

	uint16 samplesPerBlock;
	uint16 coefNum;
	ADPCMCOEFSET *coefSets;

	uint8 predictor[2];
	int16 delta[2];
	int16 samp1[2];
	int16 samp2[2];

	int64 factPos;
	int16 *encodeBuffer;
	uint32 encodeSize;
	uint32 encodeFilled;
	uint32 encodedSamples;

	uint8 *packet;
	int32 state[16];
};



/******************************************************************************/
/* RIFFWAVEConverter class                                                    */
/******************************************************************************/
class RIFFWAVEConverter : public APAddOnConverter
{
public:
	RIFFWAVEConverter(APGlobalData *global, PString fileName);
	virtual ~RIFFWAVEConverter(void);

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
	RIFFWAVEFormat *format;

	// Loader variables
	int64 dataStart;

	uint16 sampFormat;
	uint32 bytesPerSecond;
	uint16 blockAlign;

	// Saver variables
	int8 *saveBuffer;
	uint32 bufLength;
	uint32 total;

	uint32 dataPos;
};

#endif
