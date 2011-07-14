/******************************************************************************/
/* MPG123Player header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MPG123Player_h
#define __MPG123Player_h

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


/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define NEW_DCT9

#define CHECK_BUF_SIZE			8192
#define CHECK_FRAMES			10

#define DOUBLE_TO_REAL(x)		(x)
#define REAL_TO_SHORT(x)		((int16)(x))
#define REAL_PLUS_32767			32767.0
#define REAL_MINUS_32768		-32768.0
#define REAL_MUL(x, y)			((x) * (y))

#define AUDIOBUFSIZE			16384		// n * 64 with n = 1, 2, 3 ...

#define SBLIMIT					32
#define SCALE_BLOCK				12
#define SSLIMIT					18

#define MPG_MD_STEREO			0
#define MPG_MD_JOINT_STEREO		1
#define MPG_MD_DUAL_CHANNEL		2
#define MPG_MD_MONO				3

#define MAXFRAMESIZE			4096
#define HDRCMPMASK				0xfffffd00

#define MAX_INPUT_FRAMESIZE		1920
#define SYNC_HEAD_MASK			0xffff0000
#define SYNC_HEAD_MASK_FF		0x0000f000
#define LOOK_AHEAD_NUM			3
#define SCAN_LENGTH				65536		// TN: Changed from 16384

#define CHECK_FOR_RIFF			0x0001
#define CHECK_FOR_ID3_V1		0x0002
#define CHECK_FOR_ID3_V2		0x0004

#define NTOM_MUL				(32768)

#define BACKBUF_SIZE			(8192)		// Can hold 4096 - 1 = 4095 bytes!

// VBR flags
#define VBR_FRAMES_FLAG			0x0001
#define VBR_BYTES_FLAG			0x0002
#define VBR_TOC_FLAG			0x0004
#define VBR_SCALE_FLAG			0x0008



/******************************************************************************/
/* Math constants                                                             */
/******************************************************************************/
#ifndef M_PI
#define M_PI					3.14159265358979323846
#endif

#ifndef M_SQRT2
#define M_SQRT2					1.41421356237309504880
#endif



/******************************************************************************/
/* Extra typedefs                                                             */
/******************************************************************************/
typedef float real;



/******************************************************************************/
/* CL-Amp player info structure                                               */
/******************************************************************************/
typedef struct PlayerInfoStruct
{
	int32 type;
	int32 layer;
	int32 bitRate;
	int32 frequency;
	int32 frames;
	int32 channelMode;
	int32 emphasis;
	bool priv;
	bool crc;
	bool copyright;
	bool original;
} PlayerInfoStruct;



/******************************************************************************/
/* Parameter structure                                                        */
/******************************************************************************/
typedef struct Parameter
{
	bool tryResync;			// Resync stream after error
} Parameter;



/******************************************************************************/
/* VBRHeader structure                                                        */
/******************************************************************************/
typedef struct VBRHeader
{
	uint32 flags;
	uint32 frames;
	uint32 bytes;
	uint32 scale;
	uint8 toc[100];
} VBRHeader;



/******************************************************************************/
/* Bitstream structure                                                        */
/******************************************************************************/
typedef struct Bitstream_Info
{
	int32 bitIndex;
	uint8 *wordPointer;
} Bitstream_Info;



/******************************************************************************/
/* MPStr structure                                                            */
/******************************************************************************/
typedef struct MPStr
{
	real hybrid_block[2][2][SBLIMIT * SSLIMIT];
	int32 hybrid_blc[2];
} MPStr;



/******************************************************************************/
/* Frame structure                                                            */
/******************************************************************************/
typedef struct AlTable
{
	int16 bits;
	int16 d;
} AlTable;



/******************************************************************************/
/* Frame structure                                                            */
/******************************************************************************/
class MPG123Player;

typedef struct Frame
{
	const AlTable *alloc;
	int32 stereo;
	int32 jsBound;
	int32 single;
	int32 ii_sbLimit;
	int32 downSampleSbLimit;
	int32 lsf;
	bool mpeg25;
	int32 downSample;
	int32 headerChange;
	int32 lay;
	int32 errorProtection;
	int32 bitRateIndex;
	int32 samplingFrequency;
	int32 padding;
	int32 extension;
	int32 mode;
	int32 modeExt;
	int32 copyright;
	int32 original;
	int32 emphasis;
	int32 frameSize;		// Computed frame size
	int32 padSize;
	int32 sideInfoSize;		// Layer 3 sideInfo header size

	// FIXME: Move this to another place
	uint32 firstHead;
	uint32 thisHead;
	int32 freeFormatSize;
} Frame;



/******************************************************************************/
/* BandInfoStruct structure                                                   */
/******************************************************************************/
typedef struct BandInfoStruct
{
	int32 longIdx[23];
	int32 longDiff[22];
	int32 shortIdx[14];
	int32 shortDiff[13];
} BandInfoStruct;



/******************************************************************************/
/* Granules info structure                                                    */
/******************************************************************************/
typedef struct GrInfoS
{
	int32 scfsi;
	uint32 part2_3length;
	uint32 bigValues;
	uint32 scaleFacCompress;
	uint32 blockType;
	uint32 mixedBlockFlag;
	uint32 tableSelect[3];
	uint32 subblockGain[3];
	uint32 maxBand[3];
	uint32 maxBandl;
	uint32 maxB;
	uint32 region1Start;
	uint32 region2Start;
	uint32 preFlag;
	uint32 scaleFacScale;
	uint32 count1TableSelect;
	real *fullGain[3];
	real *pow2Gain;
} GrInfoS;



/******************************************************************************/
/* Side info structure                                                        */
/******************************************************************************/
typedef struct III_SideInfo
{
	uint32 mainDataBegin;
	uint32 privateBits;
	struct
	{
		GrInfoS gr[2];
	} ch[2];
} III_SideInfo;



/******************************************************************************/
/* Huffman structure                                                          */
/******************************************************************************/
typedef struct NewHuff
{
	uint32 linBits;
	const int16 *table;
} NewHuff;



/******************************************************************************/
/* Reader structure                                                           */
/******************************************************************************/
typedef struct Reader
{
	uint8 *backBuf;
	bool mark;
	int32 bufPos;
	int32 bufStart;
	int32 bufEnd;
	int32 bufSize;
} Reader;



/******************************************************************************/
/* Tables                                                                     */
/******************************************************************************/
extern const int32 intWinBase[];
extern const int32 tabSel_123[2][3][16];
extern const int32 freqs[9];
extern const int32 translate[3][2][16];
extern const AlTable *allocTables[5];
extern const int32 sbLims[5];
extern const double mulMul[27];
extern const int32 base[3][9];
extern const BandInfoStruct bandInfo[9];
extern const int32 preTab1[22];
extern const int32 preTab2[22];
extern const NewHuff ht[];
extern const NewHuff htc[];



/******************************************************************************/
/* MPG123Player class                                                         */
/******************************************************************************/
class MPG123Player : public APAddOnPlayer
{
public:
	MPG123Player(APGlobalData *global, PString fileName);
	virtual ~MPG123Player(void);

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
	void DecodeVBRFrame(Frame *fr);
	void GetMP3Tags(void);

	// CL-Amp + mpg123 functions
	void SetPlayInfo(PlayerInfoStruct *info, Frame *fr);
	void PlayFrame(MPStr *mp, Frame *fr);

	// Common functions
	void ReadFrameInit(Frame *fr);
	bool HeadCheck(uint32 head);
	int32 SyncStream(Reader *rds, Frame *fr, int32 flags, int32 *skipped);
	int32 SkipRiff(Reader *rds);
	int32 SkipNewID3(Reader *rds);
	bool ReadFrame(Reader *rds, Frame *fr);
	bool DecodeHeader(Frame *fr, uint32 newHead);
	void SetPointer(int32 ssize, int32 backStep);
	int32 ComputeBPF(Frame *fr);
	int32 CalcNumFrames(Frame *fr);
	int32 CalcTotalTime(Frame *fr, int32 totalFrames);
	int32 CalcFileLength(void);

	// Dct functions
	void Dct12(real *in, real *rawOut1, real *rawOut2, register real *wi, register real *ts);
	void Dct36(real *inBuf, real *o1, real *o2, real *winTab, real *tsBuf);
	void Dct64(real *out0, real *out1, real *samples);

	// Decode functions
	int32 Synth_1to1(real *bandPtr, int32 channel, uint8 *out, int32 *pnt);

	// GetBits functions
	void BackBits(Bitstream_Info *bitBuf, int32 numberOfBits);
	int32 GetBitOffset(Bitstream_Info *bitBuf);
	int32 GetByte(Bitstream_Info *bitBuf);
	uint32 GetBits(Bitstream_Info *bitBuf, int32 numberOfBits);
	uint32 GetBitsFast(Bitstream_Info *bitBuf, int32 numberOfBits);
	uint32 Get1Bit(Bitstream_Info *bitBuf);

	// Layer1 functions
	void I_StepOne(uint32 bAlloc[], uint32 scaleIndex[2][SBLIMIT], Frame *fr);
	void I_StepTwo(real fraction[2][SBLIMIT], uint32 bAlloc[2 * SBLIMIT], uint32 scaleIndex[2][SBLIMIT], Frame *fr);
	int32 DoLayer1(MPStr *mp, Frame *fr);

	// Layer2 functions
	void InitLayer2(void);
	void II_StepOne(uint32 *bitAlloc, int32 *scale, Frame *fr);
	void II_StepTwo(uint32 *bitAlloc, real fraction[2][4][SBLIMIT], int32 *scale, Frame *fr, int32 x1);
	void II_SelectTable(Frame *fr);
	int32 DoLayer2(MPStr *mp, Frame *fr);

	// Layer3 functions
	void InitLayer3(int32 downSampleSbLimit);
	bool III_GetSideInfo(III_SideInfo *si, int32 stereo, int32 msStereo, int32 sFreq, int32 single, int32 lsf);
	int32 III_GetScaleFactors1(int32 *scf, GrInfoS *grInfo);
	int32 III_GetScaleFactors2(int32 *scf, GrInfoS *grInfo, int32 iStereo);
	int32 III_DequantizeSample(real xr[SBLIMIT][SSLIMIT], int32 *scf, GrInfoS *grInfo, int32 sFreq, int32 part2Bits);
	void III_IStereo(real xrBuf[2][SBLIMIT][SSLIMIT], int32 *scaleFac, GrInfoS *grInfo, int32 sFreq, int32 msStereo, int32 lsf);
	void III_AntiAlias(real xr[SBLIMIT][SSLIMIT], GrInfoS *grInfo);
	void III_Hybrid(MPStr *mp, real fsIn[SBLIMIT][SSLIMIT], real tsOut[SSLIMIT][SBLIMIT], int32 ch, GrInfoS *grInfo, Frame *fr);
	int32 DoLayer3(MPStr *mp, Frame *fr);

	// Readers functions
	int32 BufDiff(Reader *rds, int32 start, int32 end);
	int32 FullRead(Reader *rds, PFile *file, uint8 *buf, int32 count);
	void Readers_AddData(Reader *rds, uint8 *buf, int32 len);
	void Readers_PushbackHeader(Reader *rds, uint32 aLong);
	void Readers_MarkPos(Reader *rds);
	void Readers_GotoMark(Reader *rds);
	void DefaultInit(Reader *rds);
	bool HeadRead(Reader *rds, uint32 *newHead);
	bool HeadShift(Reader *rds, uint32 *head);
	bool ReadFrameBody(Reader *rds, uint8 *buf, int32 size);
	int32 SkipBytes(Reader *rds, int32 len);

	// Tabinit functions
	void MakeDecodeTables(int32 scaleVal);

	// VBRHead functions
	uint32 Get32Bits(uint8 *buf);
	bool GetVBRHeader(VBRHeader *head, uint8 *buf, Frame *fr);

	// Assembler functions
#ifdef __POWERPC__

	static bool HaveMMX(void) { return (false); };
	static bool Have3DNow(void) { return (false); };

	static void Dct36_3DNow(real *inBuf, real *o1, real *o2, real *winTab, real *tsBuf, real *cos9, real *tfcos36) {};
	static int32 Synth_1to1_3DNow(real *bandPtr, int32 channel, uint8 *out, int32 *pnt, real *buffs, int32 *bo, real *decWin, real **pnts) { return (0); };
	static void Synth_1to1_MMX(real *bandPtr, int32 channel, int16 *out, int16 *buffer, int32 *bo, int32 *decWins) {};
	static void MakeDecodeTables_MMX(int32 scaleVal, int32 *decWin, int32 *decWins) {};

#else

	static bool HaveMMX(void) asm("HaveMMX");
	static bool Have3DNow(void) asm("Have3DNow");

	static void Dct36_3DNow(real *inBuf, real *o1, real *o2, real *winTab, real *tsBuf, real *cos9, real *tfcos36) asm("Dct36_3DNow");
	static int32 Synth_1to1_3DNow(real *bandPtr, int32 channel, uint8 *out, int32 *pnt, real *buffs, int32 *bo, real *decWin, real **pnts) asm("Synth_1to1_3DNow");
	static void Synth_1to1_MMX(real *bandPtr, int32 channel, int16 *out, int16 *buffer, int32 *bo, int32 *decWins) asm("Synth_1to1_MMX");
	static void MakeDecodeTables_MMX(int32 scaleVal, int32 *decWin, int32 *decWins) asm("MakeDecodeTables_MMX");

#endif

	// Variables
	PResource *res;

	bool gotMMX;
	bool got3DNow;

	PFile *mpgFile;
	int32 firstFramePosition;
	int32 totalTime;
	int32 currTime;
	double timePerFrame;
	int16 oldPos;

	// VBR variables
	bool vbrFile;
	int32 vbrFrames;
	int32 vbrTotalBytes;
	int32 vbrScale;
	uint8 vbrTOC[100];

	uint8 bitUpdateCounter;
	int32 oldBitRate;

	// TAG variables
	bool tag;
	PString songName;
	PString artist;
	PString album;
	PString year;
	PString comment;
	PString genre;
	uint8 trackNum;

	// CL-Amp variables
	PlayerInfoStruct playInfo;
	PlayerInfoStruct showPlayInfo;
	Reader reader;

	int32 currFrame;
	bool endOfFile;

	// MPG123 variables
	MPStr mpStr;
	Parameter param;

	uint8 *pcmSample1;
	uint8 *pcmSample2;
	int32 pcmPoint;
	int32 outscale;

	Frame frame;

	Bitstream_Info bsi;
	int32 bsBufEnd[2];
	int32 bsBufOld_End;
	uint8 bsSpace[2][MAXFRAMESIZE + 512];
	uint8 *bsBuf;
	uint8 *bsBufOld;
	int32 bsNum;

	VBRHeader head;
	int32 vbr;

	real decWin[512 + 32];
	int32 decWin_MMX[512 + 32];
	int32 decWins_MMX[512 + 32];
	real cos64[16];
	real cos32[8];
	real cos16[4];
	real cos8[2];
	real cos4[1];
	real *pnts[5];

	int32 *ii_table[10];
	int32 grp_3tab[32 * 3];		// Used: 27
	int32 grp_5tab[128 * 3];	// Used: 125
	int32 grp_9tab[1024 * 3];	// Used: 729
	real muls[27][64];			// Also used by layer 1

	real isPow[8207];
	real aa_ca[8];
	real aa_cs[8];
	real cos1[12][6];
	real win[4][36];
	real win1[4][36];
	real gainPow2[256 + 118 + 4];
	real cos9[9];
	real cos6_1;
	real cos6_2;
	real tfcos36[9];
	real tfcos12[3];

#ifdef NEW_DCT9
	real new_cos9[3];
	real new_cos18[3];
#endif

	int32 longLimit[9][23];
	int32 shortLimit[9][14];

	int32 mapBuf0[9][152];
	int32 mapBuf1[9][156];
	int32 mapBuf2[9][44];
	int32 *map[9][3];
	int32 *mapEnd[9][3];

	uint32 n_slen2[512];		// MPEG 2.0 slen for 'normal' mode
	uint32 i_slen2[256];		// MPEG 2.0 slen for intensity stereo

	real *stereoTabs[3][2][2];
	real tan1_1[16];
	real tan2_1[16];
	real tan1_2[16];
	real tan2_2[16];
	real pow1_1[2][16];
	real pow2_1[2][16];
	real pow1_2[2][16];
	real pow2_2[2][16];

	uint32 ntomVal[2];
	uint32 ntomStep;

	int32 halfPhase;
	int32 bo;

	real buffs[2][2][0x110];
	int16 buffs_MMX[2][2][0x110];

	int32 tabLen[3];
	int32 *tables[3];

	uint32 scfsiBuf[64];
};

#endif
