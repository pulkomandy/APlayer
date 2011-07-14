/******************************************************************************/
/* MikConverter header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MikConverter_h
#define __MikConverter_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"
#include "PResource.h"

// APlayerKit headers
#include "APAddOns.h"

// Agent headers
#include "MUniTrk.h"
#include "MikMod_Internals.h"


/******************************************************************************/
/* MikConverter class                                                         */
/******************************************************************************/
class MikConverter : public MUniTrk
{
public:
	MikConverter(PResource *resource);
	virtual ~MikConverter(void);

	virtual bool CheckModule(APAgent_ConvertModule *convInfo) = 0;
	ap_result ConvertModule(APAgent_ConvertModule *convInfo);
	virtual ap_result ModuleConverter(APAgent_ConvertModule *convInfo) = 0;

	void FreeAll(void);

	ap_result ConvertToUniMod(PFile *source, PFile *dest);

protected:
	void ShowError(int32 id);

	bool AllocPositions(int32 total);
	bool AllocPatterns(void);
	bool AllocTracks(void);
	bool AllocSamples(void);
	bool AllocInstruments(void);
	bool AllocLinear(void);
	void FreeLinear(void);

	void ReadComment(PFile *file, uint16 len, bool translate);
	void ReadLinedComment(PFile *file, uint16 len, uint16 lineLen, bool translate);

	// Shared converter functions
	void S3MIT_ProcessCmd(uint8 cmd, uint8 inf, uint32 flags);
	void S3MIT_CreateOrders(int32 curious);
	int32 SpeedToFinetune(int32 speed, int32 sampNum);
	uint32 GetFrequency(uint16 flags, uint32 period);
	uint16 GetLinearPeriod(uint16 note, uint32 fine);

	PResource *res;
	PCharSet_OEM_850 charSet850;

	MODULE of;

	// S3M/IT variables
	int8 remap[UF_MAXCHAN];
	uint8 *posLookup;

	uint8 posLookupCnt;
	uint16 *origPositions;

	bool filters;						// Resonant filters in use
	uint8 activeMacro;					// Active midi macro number for Sxx,xx < 80h
	uint8 filterMacros[UF_MAXMACRO];	// Midi macros settings
	FILTER filterSettings[UF_MAXFILTER];// Computed filter settings

	int32 *noteIndex;
	int32 noteIndexCount;

	// User variables
	bool curious;
};



/******************************************************************************/
/* Mik669 class                                                               */
/******************************************************************************/
struct S69HEADER;
struct S69NOTE;

class Mik669 : public MikConverter
{
public:
	Mik669(PResource *resource) : MikConverter(resource) {};
	virtual ~Mik669(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	void LoadPatterns(PFile *file);

	S69HEADER *mh;
	S69NOTE *s69Pat;
};



/******************************************************************************/
/* MikAMF class                                                               */
/******************************************************************************/
struct AMFHEADER;
struct AMFNOTE;

class MikAMF : public MikConverter
{
public:
	MikAMF(PResource *resource) : MikConverter(resource) {};
	virtual ~MikAMF(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	bool UnpackTrack(PFile *file);
	uint8 *ConvertTrack(void);

	AMFHEADER *mh;
	AMFNOTE *track;
};



/******************************************************************************/
/* MikDSM class                                                               */
/******************************************************************************/
struct DSMSONG;
struct DSMNOTE;

class MikDSM : public MikConverter
{
public:
	MikDSM(PResource *resource) : MikConverter(resource) {};
	virtual ~MikDSM(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	void GetBlockHeader(PFile *file);
	void ReadPattern(PFile *file);
	uint8 *ConvertTrack(DSMNOTE *tr);

	DSMSONG *mh;
	DSMNOTE *dsmBuf;

	uint32	blockId;
	uint32	blockLn;
	uint32	blockLp;
};



/******************************************************************************/
/* MikFAR class                                                               */
/******************************************************************************/
struct FARHEADER1;
struct FARHEADER2;
struct FARNOTE;

class MikFAR : public MikConverter
{
public:
	MikFAR(PResource *resource) : MikConverter(resource) {};
	virtual ~MikFAR(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	uint8 *ConvertTrack(FARNOTE *n, int32 rows);

	FARHEADER1 *mh1;
	FARHEADER2 *mh2;
	FARNOTE *pat;
};



/******************************************************************************/
/* MikGDM class                                                               */
/******************************************************************************/
struct GDMHEADER;
struct GDMNOTE;

class MikGDM : public MikConverter
{
public:
	MikGDM(PResource *resource) : MikConverter(resource) {};
	virtual ~MikGDM(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	void ReadPattern(PFile *file);
	uint8 *ConvertTrack(GDMNOTE *tr);

	GDMHEADER *mh;
	GDMNOTE *gdmBuf;
};



/******************************************************************************/
/* MikIMF class                                                                */
/******************************************************************************/
struct IMFHEADER;
struct IMFNOTE;

class MikIMF : public MikConverter
{
public:
	MikIMF(PResource *resource) : MikConverter(resource) {};
	virtual ~MikIMF(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	void ReadPattern(PFile *file, int32 size, uint16 rows);
	void ProcessCmd(uint8 eff, uint8 inf);
	uint8 *ConvertTrack(IMFNOTE *tr, uint16 rows);

	IMFHEADER *mh;
	IMFNOTE *imfPat;
};



/******************************************************************************/
/* MikIT class                                                                */
/******************************************************************************/
struct ITHEADER;
struct ITNOTE;

class MikIT : public MikConverter
{
public:
	MikIT(PResource *resource) : MikConverter(resource) {};
	virtual ~MikIT(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	void GetNumChannels(PFile *file, uint16 patRows);
	uint8 *ConvertTrack(ITNOTE *tr, uint16 numRows);
	void ReadPattern(PFile *file, uint16 patRows);
	void LoadMidiString(PFile *file, char *dest);
	void LoadMidiConfiguration(PFile *file);

	ITHEADER *mh;
	ITNOTE *itPat;
	uint8 *mask;
	ITNOTE *last;
	uint32 *paraPtr;

	int32 numTrk;
	uint32 oldEffect;
};



/******************************************************************************/
/* MikS3M class                                                               */
/******************************************************************************/
struct S3MHEADER;
struct S3MNOTE;

class MikS3M : public MikConverter
{
public:
	MikS3M(PResource *resource) : MikConverter(resource) {};
	virtual ~MikS3M(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	void GetNumChannels(PFile *file);
	void ReadPattern(PFile *file);
	uint8 *ConvertTrack(S3MNOTE *tr);

	S3MHEADER *mh;
	S3MNOTE *s3mBuf;
	uint16 *paraPtr;
	uint32 tracker;
};



/******************************************************************************/
/* MikSTM class                                                               */
/******************************************************************************/
struct STMHEADER;
struct STMNOTE;

class MikSTM : public MikConverter
{
public:
	MikSTM(PResource *resource) : MikConverter(resource) {};
	virtual ~MikSTM(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	void LoadPatterns(PFile *file);
	uint8 *ConvertTrack(STMNOTE *n);
	void ConvertNote(STMNOTE *n);

	STMHEADER *mh;
	STMNOTE *stmBuf;
};



/******************************************************************************/
/* MikSTX class                                                               */
/******************************************************************************/
struct STXHEADER;
struct STXNOTE;

class MikSTX : public MikConverter
{
public:
	MikSTX(PResource *resource) : MikConverter(resource) {};
	virtual ~MikSTX(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	void ReadPattern(PFile *file);
	uint8 *ConvertTrack(STXNOTE *tr);

	STXHEADER *mh;
	STXNOTE *stxBuf;
	uint16 *paraPtr;
};



/******************************************************************************/
/* MikULT class                                                               */
/******************************************************************************/
struct ULTEVENT;

class MikULT : public MikConverter
{
public:
	MikULT(PResource *resource) : MikConverter(resource) {};
	virtual ~MikULT(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	int32 ReadUltEvent(PFile *file, ULTEVENT *event);
};



/******************************************************************************/
/* MikUNI class                                                               */
/******************************************************************************/
struct UNISMP05;

class MikUNI : public MikConverter
{
public:
	MikUNI(PResource *resource) : MikConverter(resource) {};
	virtual ~MikUNI(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	uint8 *ReadTrack(PFile *file);
	void LoadSmp6(PFile *file);
	void LoadInstr6(PFile *file);
	void LoadInstr5(PFile *file);
	void LoadSmp5(void);

	UNISMP05 *wh;
	UNISMP05 *s;
	uint16 uniVersion;
};



/******************************************************************************/
/* MikXM class                                                                */
/******************************************************************************/
struct XMHEADER;
struct XMWAVHEADER;
struct XMNOTE;

class MikXM : public MikConverter
{
public:
	MikXM(PResource *resource) : MikConverter(resource) {};
	virtual ~MikXM(void) {};

	bool CheckModule(APAgent_ConvertModule *convInfo);
	ap_result ModuleConverter(APAgent_ConvertModule *convInfo);

protected:
	uint8 ReadNote(PFile *file, XMNOTE *n);
	uint8 *Convert(XMNOTE *xmTrack, uint16 rows);
	void LoadPatterns(PFile *file, bool dummyPat);
	void LoadInstruments(PFile *file);
	void FixEnvelope(ENVPT *cur, int32 pts);

	XMHEADER *mh;
	XMWAVHEADER *wh;
	XMWAVHEADER *s;
	uint32 *nextWav;
};

#endif
