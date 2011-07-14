/******************************************************************************/
/* MikMod header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MikMod_h
#define __MikMod_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"

// Player headers
#include "MikMod_Internals.h"
#include "MUniTrk.h"


/******************************************************************************/
/* ITPACK structure                                                           */
/******************************************************************************/
typedef struct ITPACK
{
	uint16 bits;				// Current number of bits
	uint16 bufBits;				// Bits in buffer
	int16 last;					// Last output
	uint8 buf;					// Bit buffer
} ITPACK;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	uint8 speed;
	uint16 tempo;
	PTimeSpan time;
} PosInfo;



/******************************************************************************/
/* Subsong time structure                                                     */
/******************************************************************************/
typedef struct SongTime
{
	int32 startPos;
	PTimeSpan totalTime;
	PList<PosInfo> posInfoList;
} SongTime;



/******************************************************************************/
/* Effect function type declaration                                           */
/******************************************************************************/
class MikMod;
typedef int32 (*effect_func)(MikMod *, uint16, uint16, MP_CONTROL *, MODULE *, int16);



/******************************************************************************/
/* MikMod class                                                               */
/******************************************************************************/
class MikMod : public APAddOnPlayer
{
public:
	MikMod(APGlobalData *global, PString fileName);
	virtual ~MikMod(void);

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
	virtual void EndSound(int32 index);
	virtual void Play(void);

	virtual PString GetModuleName(void);
	virtual uint16 GetVirtualChannels(void);
	virtual uint16 GetModuleChannels(void);
	virtual const uint16 *GetSubSongs(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetInstrumentInfo(uint32 num, APInstInfo *info);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	ap_result CreateUniStructs(PFile *file, PString &errorStr);
	uint8 *TrkRead(PFile *file);
	ap_result FindSamples(PFile *file, PString &errorStr);
	bool DecompressIT8(ITPACK *status, PFile *file, int8 *dest, uint32 length, uint16 *inCnt);
	bool DecompressIT16(ITPACK *status, PFile *file, int16 *dest, uint32 length, uint16 *inCnt);
	void SetTempo(uint16 tempo);

	// Player functions
	void Player_HandleTick(void);
	void PT_UpdateVoices(MODULE *mod, int32 max_Volume);
	void PT_Notes(MODULE *mod);
	void PT_EffectsPass1(MODULE *mod);
	void PT_EffectsPass2(MODULE *mod);
	void PT_NNA(MODULE *mod);
	void PT_SetupVoices(MODULE *mod);

	int32 GetRandom(int32 ceil);
	uint16 GetPeriod(uint16 flags, uint16 note, uint32 speed);
	uint16 GetOldPeriod(uint16 note, uint32 speed);
	uint16 GetLinearPeriod(uint16 note, uint32 fine);
	uint16 GetLogPeriod(uint16 note, uint32 fine);
	uint32 GetFrequency(uint16 flags, uint32 period);

	int16 Interpolate(int16 p, int16 p1, int16 p2, int16 v1, int16 v2);
	int16 InterpolateEnv(int16 p, ENVPT *a, ENVPT *b);
	int16 DoPan(int16 envPan, int16 pan);

	int16 StartEnvelope(ENVPR *t, uint8 flg, uint8 pts, uint8 susBeg, uint8 susEnd, uint8 beg, uint8 end, ENVPT *p, uint8 keyOff, int16 v);
	int16 ProcessEnvelope(MP_VOICE *aout, ENVPR *t, int16 v);
	int32 MP_FindEmptyChannel(MODULE *mod);

	// Effect functions
	int32 PT_PlayEffects(MODULE *mod, int16 channel, MP_CONTROL *a);
	void DoEEffects(uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel, uint8 dat);

	// PT effect helper functions
	void DoArpeggio(uint16 tick, uint16 flags, MP_CONTROL *a, uint8 style);
	void DoToneSlide(uint16 tick, MP_CONTROL *a);
	void DoVibrato(uint16 tick, MP_CONTROL *a);
	void DoVolSlide(MP_CONTROL *a, uint8 dat);

	// S3M effect helper functions
	void DoS3MVolSlide(uint16 tick, uint16 flags, MP_CONTROL *a, uint8 inf);
	void DoS3MSlideDn(uint16 tick, MP_CONTROL *a, uint8 inf);
	void DoS3MSlideUp(uint16 tick, MP_CONTROL *a, uint8 inf);

	// IT effect helper functions
	void DoITToneSlide(uint16 tick, MP_CONTROL *a, uint8 dat);
	void DoITVibrato(uint16 tick, MP_CONTROL *a, uint8 dat);
	void DoNNAEffects(MODULE *mod, MP_CONTROL *a, uint8 dat);

	// Special effect functions
	static int32 DoNothing(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoKeyOff(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoKeyFade(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoVolEffects(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);

	// PT effect functions
	static int32 DoPTEffect0(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect1(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect2(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect3(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect4(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect5(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect6(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect7(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect8(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffect9(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffectA(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffectB(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffectC(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffectD(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffectE(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoPTEffectF(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);

	// S3M effect functions
	static int32 DoS3MEffectA(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoS3MEffectD(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoS3MEffectE(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoS3MEffectF(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoS3MEffectI(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoS3MEffectQ(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoS3MEffectR(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoS3MEffectT(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoS3MEffectU(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);

	// XM effect functions
	static int32 DoXMEffect6(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectA(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectE1(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectE2(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectEA(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectEB(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectG(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectH(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectL(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectP(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectX1(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoXMEffectX2(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);

	// IT effect functions
	static int32 DoITEffectG(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectH(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectI(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectM(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectN(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectP(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectS0(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectT(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectU(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectW(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoITEffectY(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);

	// ULT effect functions
	static int32 DoULTEffect9(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);

	// OctaMED effect functions
	static int32 DoMEDSpeed(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoMEDEffectF1(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoMEDEffectF2(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);
	static int32 DoMEDEffectF3(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);

	// Oktalyzer effect functions
	static int32 DoOktArp(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel);

	// Voice specific functions
	void VoicePlay(uint8 voice, SAMPLE *s, uint32 start);
	void VoiceSetVolume(uint8 voice, uint16 vol);
	void VoiceSetPanning(uint8 voice, uint32 pan);
	void VoiceSetFrequency(uint8 voice, uint32 frq);
	void VoiceStop(uint8 voice);
	bool VoiceStopped(uint8 voice);

	// Memory functions
	bool AllocPositions(int32 total);
	bool AllocPatterns(void);
	bool AllocTracks(void);
	bool AllocSamples(void);
	bool AllocInstruments(void);
	void FreeAll(void);

	// Variables
	PResource *res;

	PList<SongTime *> songTimeList;

	uint16 songTab[2];
	uint16 currentSong;

	uint16 bpmTempo;

	effect_func effects[UNI_LAST];

	MODULE of;
	MUniTrk uniTrk;

	// Voice variables
	uint8 md_sngChn;
};

#endif
