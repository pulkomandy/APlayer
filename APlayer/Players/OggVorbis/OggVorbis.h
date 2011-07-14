/******************************************************************************/
/* OggVorbis header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __OggVorbis_h
#define __OggVorbis_h

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

// OggVorbis headers
#include "VorbisFile.h"
#include "Mdct.h"


/******************************************************************************/
/* OggVorbis class                                                            */
/******************************************************************************/
struct VorbisLookFloor0;

class OggVorbis : public APAddOnPlayer
{
public:
	OggVorbis(APGlobalData *global, PString fileName);
	virtual ~OggVorbis(void);

	virtual float GetVersion(void);

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
	static void *_ogg_calloc(int32 count, int32 bytes);
	static void *_ogg_malloc(int32 bytes);
	static void *_ogg_realloc(void *oldData, int32 newSize);
	static void _ogg_free(void *data);

	//
	// VorbisFile
	//
	int32 OV_Open(PFile *f, OggVorbisFile *vf, int8 *initial, int32 iBytes);
	int32 OV_Clear(OggVorbisFile *vf);

	int32 OV_RawSeek(OggVorbisFile *vf, int64 pos);
	int32 OV_PCMSeek(OggVorbisFile *vf, int64 pos);
	int32 OV_TimeSeek(OggVorbisFile *vf, double seconds);
	int32 OV_PCMSeekPage(OggVorbisFile *vf, int64 pos);

	int32 OV_Read(OggVorbisFile *vf, float ***buffer, int32 length, int32 *bitStream);

	VorbisInfo *OV_Info(OggVorbisFile *vf, int32 link);
	VorbisComment *OV_Comment(OggVorbisFile *vf, int32 link);

	int64 OV_PCM_Total(OggVorbisFile *vf, int32 i);
	double OV_Time_Total(OggVorbisFile *vf, int32 i);
	double OV_Time_Tell(OggVorbisFile *vf);
	int32 OV_Bitrate(OggVorbisFile *vf, int32 i);
	int32 OV_BitrateInstant(OggVorbisFile *vf);

	int32 _OV_Open1(PFile *f, OggVorbisFile *vf, int8 *initial, int32 iBytes);
	int32 _OV_Open2(OggVorbisFile *vf);

	int32 _OpenSeekable2(OggVorbisFile *vf);
	int32 _FetchHeaders(OggVorbisFile *vf, VorbisInfo *vi, VorbisComment *vc, int32 *serialNo, OggPage *ogPtr);
	void _PrefetchAllHeaders(OggVorbisFile *vf, int64 dataOffset);
	int32 _BisectForwardSerialNo(OggVorbisFile *vf, int64 begin, int64 searched, int64 end, int32 currentNo, int32 m);

	void _SeekHelper(OggVorbisFile *vf, int64 offset);
	int32 _GetData(OggVorbisFile *vf);
	int32 _ProcessPacket(OggVorbisFile *vf, int32 readp);

	int64 _GetPrevPage(OggVorbisFile *vf, OggPage *og);
	int64 _GetNextPage(OggVorbisFile *vf, OggPage *og, int64 boundary);

	void _DecodeClear(OggVorbisFile *vf);
	void _MakeDecodeReady(OggVorbisFile *vf);

	//
	// OGG
	//
	// Bitwise functions
	void OggPack_ReadInit(OggPackBuffer *b, uint8 *buf, int32 bytes);
	int32 OggPack_Read(OggPackBuffer *b, int32 bits);
	int32 OggPack_Read1(OggPackBuffer *b);
	int32 OggPack_Look(OggPackBuffer *b, int32 bits);
	void OggPack_Adv(OggPackBuffer *b, int32 bits);

	// Framing functions
	int32 Ogg_Page_Version(OggPage *og);
	bool Ogg_Page_Continued(OggPage *og);
	bool Ogg_Page_BOS(OggPage *og);
	bool Ogg_Page_EOS(OggPage *og);
	int64 Ogg_Page_GranulePos(OggPage *og);
	int32 Ogg_Page_SerialNo(OggPage *og);
	int32 Ogg_Page_PageNo(OggPage *og);
	void Ogg_Page_ChecksumSet(OggPage *og);

	int32 Ogg_Stream_Init(OggStreamState *os, int32 serialNo);
	int32 Ogg_Stream_Clear(OggStreamState *os);
	int32 Ogg_Stream_Reset(OggStreamState *os);
	int32 Ogg_Stream_PageIn(OggStreamState *os, OggPage *og);
	int32 Ogg_Stream_PacketOut(OggStreamState *os, OggPacket *op);
	int32 Ogg_Stream_PacketPeek(OggStreamState *os, OggPacket *op);

	int32 Ogg_Sync_Init(OggSyncState *oy);
	int32 Ogg_Sync_Clear(OggSyncState *oy);
	int32 Ogg_Sync_Reset(OggSyncState *oy);
	int8 *Ogg_Sync_Buffer(OggSyncState *oy, int32 size);
	int32 Ogg_Sync_Wrote(OggSyncState *oy, int32 bytes);
	int32 Ogg_Sync_PageSeek(OggSyncState *oy, OggPage *og);
	int32 Ogg_Sync_PageOut(OggSyncState *oy, OggPage *og);

	void _os_BodyExpand(OggStreamState *os, int32 needed);
	void _os_LacingExpand(OggStreamState *os, int32 needed);
	int32 _PacketOut(OggStreamState *os, OggPacket *op, bool adv);

	//
	// Vorbis
	//
	// Common functions
	static int32 ilog2(uint32 v);
	static int32 ilog(uint32 v);
	static int32 icount(uint32 v);

	// Block functions
	int32 Vorbis_Block_Init(VorbisDSPState *v, VorbisBlock *vb);
	int32 Vorbis_Block_Clear(VorbisBlock *vb);

	void Vorbis_DSP_Clear(VorbisDSPState *v);

	int32 Vorbis_Synthesis_Init(VorbisDSPState *v, VorbisInfo *vi);
	int32 Vorbis_Synthesis_BlockIn(VorbisDSPState *v, VorbisBlock *vb);
	int32 Vorbis_Synthesis_PCMOut(VorbisDSPState *v, float ***pcm);
	int32 Vorbis_Synthesis_Read(VorbisDSPState *v, int32 n);

	void _Vorbis_Block_Ripcord(VorbisBlock *vb);
	void *_Vorbis_Block_Alloc(VorbisBlock *vb, int32 bytes);
	int32 _Vds_Shared_Init(VorbisDSPState *v, VorbisInfo *vi, bool encp);

	// Codebook functions
	int32 Vorbis_Staticbook_Unpack(OggPackBuffer *opb, StaticCodebook *s);

	int32 Vorbis_Book_Decode(Codebook *book, OggPackBuffer *b);
	int32 Vorbis_Book_DecodeVSet(Codebook *book, float *a, OggPackBuffer *b, int32 n);

	static int32 Vorbis_Book_DecodeVsAdd(OggVorbis *obj, Codebook *book, float *a, OggPackBuffer *b, int32 n);
	static int32 Vorbis_Book_DecodeVAdd(OggVorbis *obj, Codebook *book, float *a, OggPackBuffer *b, int32 n);
	static int32 Vorbis_Book_DecodeVvAdd(OggVorbis *obj, Codebook *book, float **a, int32 offset, int32 ch, OggPackBuffer *b, int32 n);

	static uint32 BitReverse(uint32 x);
	int32 DecodePackedEntryNumber(Codebook *book, OggPackBuffer *b);

	// Info functions
	void Vorbis_Comment_Init(VorbisComment *vc);
	void Vorbis_Comment_Clear(VorbisComment *vc);

	void Vorbis_Info_Init(VorbisInfo *vi);
	void Vorbis_Info_Clear(VorbisInfo *vi);
	int32 Vorbis_Info_BlockSize(VorbisInfo *vi, int32 zo);

	int32 Vorbis_Unpack_Info(VorbisInfo *vi, OggPackBuffer *opb);
	int32 Vorbis_Unpack_Comment(VorbisComment *vc, OggPackBuffer *opb);
	int32 Vorbis_Unpack_Books(VorbisInfo *vi, OggPackBuffer *opb);

	int32 Vorbis_Synthesis_HeaderIn(VorbisInfo *vi, VorbisComment *vc, OggPacket *op);

	void _V_ReadString(OggPackBuffer *o, int8 *buf, int32 bytes);

	// Lsp functions
	void Vorbis_LspToCurve(float *curve, int32 *map, int32 n, int32 ln, float *lsp, int32 m, float amp, float ampOffset);

	// Mdct functions
	void Mdct_Init(MdctLookup *lookup, int32 n);
	void Mdct_Clear(MdctLookup *l);
	void Mdct_Backward(MdctLookup *init, DATA_TYPE *in, DATA_TYPE *out);
	void Mdct_BitReverse(MdctLookup *init, DATA_TYPE *x);
	void Mdct_Butterflies(MdctLookup *init, DATA_TYPE *x, int32 points);
	void Mdct_ButterflyFirst(DATA_TYPE *t, DATA_TYPE *x, int32 points);
	void Mdct_ButterflyGeneric(DATA_TYPE *t, DATA_TYPE *x, int32 points, int32 trigInt);
	void Mdct_Butterfly32(DATA_TYPE *x);
	void Mdct_Butterfly16(DATA_TYPE *x);
	void Mdct_Butterfly8(DATA_TYPE *x);

	// Sharedbook functions
	void Vorbis_Staticbook_Clear(StaticCodebook *b);
	void Vorbis_Staticbook_Destroy(StaticCodebook *b);

	int32 Vorbis_Book_InitDecode(Codebook *c, const StaticCodebook *s);
	void Vorbis_Book_Clear(Codebook *b);

	int32 _Book_MapType1_QuantVals(const StaticCodebook *b);
	float *_Book_Unquantize(const StaticCodebook *b, int32 n, int32 *sparseMap);
	uint32 *_MakeWords(int32 *l, int32 n, int32 sparseCount);

	float _Float32_Unpack(int32 val);

	static int Sort32a(const void *a, const void *b);

	// Synthesis functions
	int32 Vorbis_Synthesis(VorbisBlock *vb, OggPacket *op);

	int32 Vorbis_Packet_BlockSize(VorbisInfo *vi, OggPacket *op);

	// Window functions
	float *_Vorbis_Window(int32 type, int32 left);
	void _Vorbis_ApplyWindow(float *d, float *window[2], int32 *blockSizes, int32 lW, int32 w, int32 nW);

public:
	// Floor0 functions
	void Floor0_Map_LazyInit(VorbisBlock *vb, VorbisInfoFloor *infoX, VorbisLookFloor0 *look);
	static VorbisInfoFloor *Floor0_Unpack(OggVorbis *obj, VorbisInfo *vi, OggPackBuffer *opb);
	static VorbisLookFloor *Floor0_Look(VorbisDSPState *vd, VorbisInfoFloor *i);
	static void Floor0_FreeInfo(VorbisInfoFloor *i);
	static void Floor0_FreeLook(VorbisLookFloor *i);
	static void *Floor0_Inverse1(OggVorbis *obj, VorbisBlock *vb, VorbisLookFloor *i);
	static int32 Floor0_Inverse2(OggVorbis *obj, VorbisBlock *vb, VorbisLookFloor *i, void *memo, float *out);

	// Floor1 functions
	static int icomp(const void *a, const void *b);
	static VorbisInfoFloor *Floor1_Unpack(OggVorbis *obj, VorbisInfo *vi, OggPackBuffer *opb);
	static VorbisLookFloor *Floor1_Look(VorbisDSPState *vd, VorbisInfoFloor *in);
	static void Floor1_FreeInfo(VorbisInfoFloor *i);
	static void Floor1_FreeLook(VorbisLookFloor *i);
	static void *Floor1_Inverse1(OggVorbis *obj, VorbisBlock *vb, VorbisLookFloor *in);
	static int32 Floor1_Inverse2(OggVorbis *obj, VorbisBlock *vb, VorbisLookFloor *in, void *memo, float *out);
	int32 RenderPoint(int32 x0, int32 x1, int32 y0, int32 y1, int32 x);
	void RenderLine(int32 x0, int32 x1, int32 y0, int32 y1, float *d);

	// Mapping0 functions
	static VorbisInfoMapping *Mapping0_Unpack(OggVorbis *obj, VorbisInfo *vi, OggPackBuffer *opb);
	static void Mapping0_FreeInfo(VorbisInfoMapping *i);
	static int32 Mapping0_Inverse(OggVorbis *obj, VorbisBlock *vb, VorbisInfoMapping *l);

	// Residue0 functions
	static VorbisInfoResidue *Res0_Unpack(OggVorbis *obj, VorbisInfo *vi, OggPackBuffer *opb);
	static VorbisLookResidue *Res0_Look(VorbisDSPState *vd, VorbisInfoResidue *vr);
	static void Res0_FreeInfo(VorbisInfoResidue *i);
	static void Res0_FreeLook(VorbisLookResidue *i);
	static int32 Res0_Inverse(OggVorbis *obj, VorbisBlock *vb, VorbisLookResidue *vl, float **in, bool *nonZero, int32 ch);
	static int32 Res1_Inverse(OggVorbis *obj, VorbisBlock *vb, VorbisLookResidue *vl, float **in, bool *nonZero, int32 ch);
	static int32 Res2_Inverse(OggVorbis *obj, VorbisBlock *vb, VorbisLookResidue *vl, float **in, bool *nonZero, int32 ch);
	int32 _01Inverse(VorbisBlock *vb, VorbisLookResidue *vl, float **in, int32 ch, int32 (*decodePart)(OggVorbis *, Codebook *, float *, OggPackBuffer *, int32));

protected:
	// Variables
	PResource *res;

	PFile *oggFile;
	bool eof;
	int16 oldPos;

	int32 channels;
	int32 rate;
	int32 totalTime;

	int16 **chanBuffers;

	OggVorbisFile vorbisFile;

	// Comment information
	PString title;
	PString artist;

	uint32 trackNumber;
	PString album;
	PString genre;
	PString organization;
	PString copyright;
	PString description;
	PString vendor;
	int32 bitrate;
};

#endif
