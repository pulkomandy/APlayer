/******************************************************************************/
/* ModTracker header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ModTracker_h
#define __ModTracker_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APChannel.h"


/******************************************************************************/
/* Player type enums                                                          */
/******************************************************************************/
enum ModType
{
	modUnknown			= -1,
	modSoundTracker15	= 0,
	modSoundTracker31,
	modNoiseTracker,
	modStarTrekker,
	modStarTrekker8,
	modProTracker,
	modFastTracker,
	modTakeTracker,
	modMultiTracker
};



/******************************************************************************/
/* Effect numbers                                                             */
/******************************************************************************/
enum Effects
{
	effArpeggio = 0x00,				// 0x00
	effSlideUp,						// 0x01
	effSlideDown,					// 0x02
	effTonePortamento,				// 0x03
	effVibrato,						// 0x04
	effTonePort_VolSlide,			// 0x05
	effVibrato_VolSlide,			// 0x06
	effTremolo,						// 0x07
	effSampleOffset = 0x09,			// 0x09
	effVolumeSlide,					// 0x0a
	effPosJump,						// 0x0b
	effSetVolume,					// 0x0c
	effPatternBreak,				// 0x0d
	effExtraEffect,					// 0x0e
	effSetSpeed						// 0x0f
};



/******************************************************************************/
/* Extra effect numbers (ProTracker only)                                     */
/******************************************************************************/
enum ExtraEffects
{
	effSetFilter = 0x00,			// 0xe0
	effFineSlideUp = 0x10,			// 0xe1
	effFineSlideDown = 0x20,		// 0xe2
	effGlissandoCtrl = 0x30,		// 0xe3
	effVibratoWaveform = 0x40,		// 0xe4
	effSetFineTune = 0x50,			// 0xe5
	effJumpToLoop = 0x60,			// 0xe6
	effTremoloWaveform = 0x70,		// 0xe7
	effRetrig = 0x90,				// 0xe9
	effFineVolSlideUp = 0xa0,		// 0xea
	effFineVolSlideDown = 0xb0,		// 0xeb
	effNoteCut = 0xc0,				// 0xec
	effNoteDelay = 0xd0,			// 0xed
	effPatternDelay = 0xe0,			// 0xee
	effInvertLoop = 0xf0			// 0xef
};



/******************************************************************************/
/* StarTrekker AM Sample structure                                            */
/******************************************************************************/
typedef struct AMSample
{
	uint16	mark;
	uint32	pad00;
	uint16	startAmp;
	uint16	attack1Level;
	uint16	attack1Speed;
	uint16	attack2Level;
	uint16	attack2Speed;
	uint16	sustainLevel;
	uint16	decaySpeed;
	uint16	sustainTime;
	uint16	pad01;
	uint16	releaseSpeed;
	uint16	waveform;
	uint16	pitchFall;
	int16	vibAmp;
	uint16	vibSpeed;
	uint16	baseFreq;
	uint8	reserved[84];
} AMSample;



/******************************************************************************/
/* Sample Info structure                                                      */
/******************************************************************************/
typedef struct SampleInfo
{
	char		sampleName[23];			// Name of sample
	uint16		length;					// Length in words
	uint8		fineTune;				// Only the low nibble is used (mask it out and extend the sign)
	uint8		volume;					// The volume
	uint16		repeatStart;			// Number of words from the beginning where the loop starts
	uint16		repeatLength;			// The loop length in words
} SampleInfo;



/******************************************************************************/
/* ProModule structure                                                        */
/******************************************************************************/
typedef struct ProModule
{
	char		songName[21];			// Name of the module
	SampleInfo	samples[31];			// Sample structures
	uint8		songLength;				// Length of the song
	uint8		restart;				// Restart position in NoiseTracker
	uint8		positions[128];			// Pattern to play
	uint32		mark;					// Indentifier mark
} ProModule;



/******************************************************************************/
/* MultiModule structure                                                      */
/*                                                                            */
/* Notice that all 16 bit fields are stored in little endian.                 */
/******************************************************************************/
typedef struct MultiModule
{
	char		mark[3];				// Indentifier mark (MTM)
	uint8		version;				// File version
	char		songName[21];			// Name of the module
	uint16		trackNum;				// Number of tracks
	uint8		patternNum;				// Number of patterns
	uint8		songLength;				// Length of the song
	uint16		commentLength;			// Length of the comment field
	uint8		sampleNum;				// Number of samples
	uint8		attributes;				// Not used yet
	uint8		patternLength;			// Number of lines in each pattern
	uint8		channels;				// Number of channels
	uint8		panning[32];			// Panning table
} MultiModule;



/******************************************************************************/
/* MultiTracker Sample Info structure                                         */
/*                                                                            */
/* Notice that all 32 bit fields are stored in little endian.                 */
/******************************************************************************/
typedef struct MultiSampleInfo
{
	char		sampleName[23];			// Name of sample
	uint32		length;					// Length in bytes
	uint32		repeatStart;			// Number of bytes from the beginning where the loop starts
	uint32		repeatEnd;				// Number of bytes from the beginning where the loop ends
	uint8		fineTune;				// Only the low nibble is used (mask it out and extend the sign)
	uint8		volume;					// The volume (0-64)
	uint8		attributes;				// Not used
} MultiSampleInfo;



/******************************************************************************/
/* Sample structure                                                           */
/******************************************************************************/
typedef struct Sample
{
	PString		sampleName;				// Name of sample
	int8 *		start;					// Start address
	uint16		length;					// Length in words
	uint16		loopStart;				// Loop start offset in words
	uint16		loopLength;				// Loop length in words
	uint8		fineTune;				// Finetune (-8 - +7, but used as 0-15)
	uint8		volume;					// The volume (0-64)
} Sample;



/******************************************************************************/
/* TrackLine structure                                                        */
/******************************************************************************/
typedef struct TrackLine
{
	uint8		note;					// The note to play
	uint8		sample;					// The sample
	uint8		effect;					// The effect to use
	uint8		effectArg;				// Effect argument
} TrackLine;



/******************************************************************************/
/* Channel structure                                                          */
/******************************************************************************/
typedef struct Channel
{
	TrackLine		trackLine;
	int8 *			start;
	uint16			length;
	int8 *			loopStart;
	uint16			loopLength;
	uint16			startOffset;
	uint16			period;
	uint8			fineTune;
	int8			volume;
	uint8			panning;			// Only used in MultiTracker modules
	uint8			tonePortDirec;
	uint8			tonePortSpeed;
	uint16			wantedPeriod;
	uint8			vibratoCmd;
	int8			vibratoPos;
	uint8			tremoloCmd;
	int8			tremoloPos;
	uint8			waveControl;
	uint8			glissFunk;
	uint8			sampleOffset;
	uint8			pattPos;
	uint8			loopCount;
	uint8			funkOffset;
	int8 *			waveStart;
	uint16			realLength;
	int16			pick;
	bool			amSample;			// True if AM sample, false if normal
	uint16			amTodo;				// Switch number
	uint16			sampleNum;			// Current sample number
	int16			curLevel;			// Current AM level
	uint16			vibDegree;			// Vibrato degree
	int16			sustainCounter;		// Sustain time counter
} Channel;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	uint8			speed;
	uint8			tempo;
	PTimeSpan		time;
} PosInfo;



/******************************************************************************/
/* Subsong time structure                                                     */
/******************************************************************************/
typedef struct SongTime
{
	int32			startPos;
	PTimeSpan		totalTime;
	PList<PosInfo>	posInfoList;
} SongTime;



/******************************************************************************/
/* ModTracker class                                                           */
/******************************************************************************/
class ModTracker : public APAddOnPlayer
{
public:
	ModTracker(APGlobalData *global, PString fileName);
	virtual ~ModTracker(void);

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

	virtual PString GetModuleName(void);
	virtual uint16 GetModuleChannels(void);
	virtual const uint16 *GetSubSongs(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	ModType TestModule(PFile *file);
	bool CheckSampleName(PFile *file, uint16 num);

	ap_result LoadTracker(ModType type, PFile *file, PString &errorStr);
	ap_result LoadMultiTracker(PFile *file, PString &errorStr);
	ap_result ExtraLoad(PFile *file);
	void LoadModTracks(PFile *file, TrackLine **tracks, int32 channels);
	void Cleanup(void);

	void NextPos(void);
	void NoNewAllChan(void);
	void GetNewNote(void);
	void PlayVoice(TrackLine *trackData, APChannel *apChan, Channel &chan);
	void CheckEFX(APChannel *apChan, Channel &chan);
	void CheckMoreEFX(APChannel *apChan, Channel &chan);
	void ECommands(APChannel *apChan, Channel &chan);
	void SetTonePorta(Channel &chan);
	void UpdateFunk(Channel &chan);
	void ChangeTempo(uint8 newTempo);

	void Arpeggio(APChannel *apChan, Channel &chan);
	void PortUp(APChannel *apChan, Channel &chan);
	void PortDown(APChannel *apChan, Channel &chan);
	void TonePortamento(APChannel *apChan, Channel &chan, bool skip = false);
	void Vibrato(APChannel *apChan, Channel &chan, bool skip = false);
	void TonePlusVol(APChannel *apChan, Channel &chan);
	void VibratoPlusVol(APChannel *apChan, Channel &chan);
	void Tremolo(APChannel *apChan, Channel &chan);
	void SampleOffset(Channel &chan);
	void VolumeSlide(APChannel *apChan, Channel &chan);
	void PositionJump(Channel &chan);
	void VolumeChange(APChannel *apChan, Channel &chan);
	void PatternBreak(Channel &chan);
	void SetSpeed(Channel &chan);

	void Filter(Channel &chan);
	void FinePortUp(APChannel *apChan, Channel &chan);
	void FinePortDown(APChannel *apChan, Channel &chan);
	void SetGlissCon(Channel &chan);
	void SetVibCon(Channel &chan);
	void SetFineTune(Channel &chan);
	void JumpLoop(Channel &chan);
	void SetTreCon(Channel &chan);
	void RetrigNote(APChannel *apChan, Channel &chan);
	void VolumeFineUp(APChannel *apChan, Channel &chan);
	void VolumeFineDown(APChannel *apChan, Channel &chan);
	void NoteCut(APChannel *apChan, Channel &chan);
	void NoteDelay(APChannel *apChan, Channel &chan);
	void PatternDelay(Channel &chan);
	void FunkIt(Channel &chan);

	void AMHandler(void);

	PResource *res;
	PCharSet_Amiga amiCharSet;
	PCharSet_OEM_850 dosCharSet;
	ModType modType;

	PList<SongTime *> songTimeList;

	uint16 songTab[2];
	uint16 currentSong;

	PString songName;
	uint16 maxPattern;
	uint16 channelNum;
	uint16 sampleNum;
	uint16 songLength;
	uint16 trackNum;
	uint16 patternLength;
	uint16 restartPos;

	uint16 minPeriod;
	uint16 maxPeriod;

	uint8 panning[32];
	uint8 positions[128];

	Sample *samples;
	TrackLine **tracks;
	uint16 *sequences;
	AMSample *amData;

	Channel *channels;

	uint16 songPos;
	uint16 patternPos;
	uint16 breakPos;
	bool posJumpFlag;
	bool breakFlag;
	bool gotJump;
	bool gotBreak;
	uint8 tempo;
	uint8 speed;
	uint8 counter;
	uint8 lowMask;
	uint8 pattDelTime;
	uint8 pattDelTime2;
};

#endif
