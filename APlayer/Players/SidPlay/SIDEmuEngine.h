/******************************************************************************/
/* SIDEmuEngine header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDEmuEngine_h
#define __SIDEmuEngine_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "SID6581.h"
#include "SID6510.h"


/******************************************************************************/
/* Function definations                                                       */
/******************************************************************************/
class SIDEmuEngine;
typedef void *(*FillFunc)(SIDEmuEngine *, void *, uint32);



/******************************************************************************/
/* sidEmuConfig class                                                         */
/*                                                                            */
/* An instance of this structure is used to transport emulator settings to    */
/* and from the interface class.                                              */
/******************************************************************************/
typedef struct sidEmuConfig
{
	uint16 frequency;		// [Frequency] = Hz, min=4000, max=48000
	int32 bitsPerSample;	// See below, ``Sample precision''
	int32 sampleFormat;		// See below, ``Sample encoding''
	int32 channels;			// See below, ``Number of physical audio channels''
	int32 sidChips;			// --- Unsupported ---
	int32 volumeControl;	// See below, ``Volume control modes''

	bool mos8580;			// True, false (just the waveforms)
	bool measuredVolume;	// True, false

	bool emulateFilter;		// True, false
	float filterFs;			// 1.0 <= Fs
	float filterFm;			// Fm != 0
	float filterFt;			//

	int32 memoryMode;		// MPU_BANK_SWITCHING, MPU_TRANSPARENT_ROM,
							// MPU_PLAYSID_ENVIRONMENT

	int32 clockSpeed;		// SIDTUNE_CLOCK_PAL, SIDTUNE_CLOCK_NTSC

	bool forceSongSpeed;	// True, false

	//
	// Working, but experimental.
	//
	int32 digiPlayerScans;	// 0=off, number of C64 music player calls used to
							// scan it for PlaySID Extended SID Register usage

	int32 autoPanning;		// see below, ``Auto-panning''
} sidEmuConfig;



/******************************************************************************/
/* Memory mode settings:                                                      */
/*                                                                            */
/* MPU_BANK_SWITCHING: Does emulate every bank-switching that one can         */
/* consider as useful for music players.                                      */
/*                                                                            */
/* MPU_TRANSPARENT_ROM: An emulator environment with partial bank-switching.  */
/* Required to run sidtunes which:                                            */
/*                                                                            */
/*  - use the RAM under the I/O address space                                 */
/*  - use RAM at $E000-$FFFA + jump to Kernal functions                       */
/*                                                                            */
/* MPU_PLAYSID_ENVIRONMENT: A PlaySID-like emulator environment. Required to  */
/* run sidtunes which:                                                        */
/*                                                                            */
/*  - are specific to PlaySID                                                 */
/*  - do not contain bank-switching code                                      */
/*                                                                            */
/* Sidtunes that would not run on a real C64 because their players were       */
/* prepared to assume the certain emulator environment provided by PlaySID.   */
/******************************************************************************/
enum
{
	// Memory mode settings
	//
	MPU_BANK_SWITCHING = 0x20,
	MPU_TRANSPARENT_ROM,
	MPU_PLAYSID_ENVIRONMENT,

	// Volume control modes. Use ``SIDEMU_NONE'' for no control
	//
	SIDEMU_VOLCONTROL = 0x40,
	SIDEMU_FULLPANNING,
	SIDEMU_HWMIXING,
	SIDEMU_STEREOSURROUND,

	// Auto-panning modes. Use ``SIDEMU_NONE'' for none
	//
	SIDEMU_CENTEREDAUTOPANNING = 0x50,

	// This can either be used as a dummy, or where one does not
	// want to make an alternative setting
	SIDEMU_NONE = 0x1000
};



/******************************************************************************/
/* Sample format and configuration constants. The values are intended to be   */
/* distinct from each other. Some of the constants have a most obvious value, */
/* so they can be used in calculations.                                       */
/******************************************************************************/

// Sample encoding (format)
#define SIDEMU_UNSIGNED_PCM				0x80
#define SIDEMU_SIGNED_PCM				0x7F

// Number of physical audio channels
// ``Stereo'' means interleaved channel data
#define SIDEMU_MONO						1
#define SIDEMU_STEREO					2

// Sample precision (bits per sample). The endianess of the stored samples
// is machine dependent
#define SIDEMU_8BIT						8
#define SIDEMU_16BIT					16



/******************************************************************************/
/* Auto-panning modes. Only valid for mixing modes ``SIDEMU_FULLPANNING'' or  */
/* ``SIDEMU_STEREOSURROUND''.                                                 */
/*                                                                            */
/* The volume levels left/right build the panning boundaries. The panning     */
/* range is the difference between left and right level. After enabling this  */
/* you can override the default levels with your own ones using the           */
/* SetVoiceVolume() function. A default is provided to ensure sane initial    */
/* settings.                                                                  */
/* NOTE: You can mute a voice by setting left=right=0 or total=0.             */
/*                                                                            */
/* Auto-panning starts each new note on the opposite pan-position and then    */
/* moves between the left and right volume level.                             */
/*                                                                            */
/* Centered auto-panning starts in the middle, moves outwards and then        */
/* toggles between the two pan-positions like normal auto-panning.            */
/******************************************************************************/

// Default filter parameters
#define SIDEMU_DEFAULTFILTERFS			400.0f
#define SIDEMU_DEFAULTFILTERFM			60.0f
#define SIDEMU_DEFAULTFILTERFT			0.05f



/******************************************************************************/
/* Volume control modes                                                       */
/*                                                                            */
/* Relative voice volume is ``total'' from 0 (mute) to 256 (max). If you use  */
/* it, you don't have to modulate each L/R level yourself.                    */
/*                                                                            */
/* A noticable difference between FULLPANNING and VOLCONTROL is FULLPANNING's */
/* capability to mix four (all) logical voices to a single physical audio     */
/* channel, whereas VOLCONTROL is only able to mix two (half of all) logical  */
/* voices to a physical channel.                                              */
/* Therefore VOLCONTROL results in slightly better sample quality, because    */
/* it mixes at higher amplitude. Especially when using a sample precision of  */
/* 8-bit and stereo. In mono mode both modes operate equally.                 */
/*                                                                            */
/* NOTE: Changing the volume control mode resets the current volume level     */
/* settings for all voices to a default:                                      */
/*                                                                            */
/*     MONO  | left | right     STEREO | left | right                         */
/*   -----------------------   -----------------------                        */
/*   voice 1 |  255 |   0      voice 1 |  255 |   0                           */
/*   voice 2 |  255 |   0      voice 2 |    0 | 255                           */
/*   voice 3 |  255 |   0      voice 3 |  255 |   0                           */
/*   voice 4 |  255 |   0      voice 4 |    0 | 255                           */
/*                                                                            */
/*   SURROUND | left | right                                                  */
/*   ------------------------                                                 */
/*    voice 1 |  255 |  255                                                   */
/*    voice 2 |  255 |  255                                                   */
/*    voice 3 |  255 |  255                                                   */
/*    voice 4 |  255 |  255                                                   */
/*                                                                            */
/*                                                                            */
/* Because of the asymmetric ``three-voice'' nature of most sidtunes, it is   */
/* strongly advised to *not* use plain stereo without pan-positioning the     */
/* voices.                                                                    */
/*                                                                            */
/*    int32 digiPlayerScans;                                                  */
/*                                                                            */
/* If the integer above is set to ``x'', the sidtune will be scanned x player */
/* calls for PlaySID digis on the fourth channel. If no digis are used, the   */
/* sidtune is hopefully ``three-voice-only'' and can be amplified a little    */
/* bit.                                                                       */
/*                                                                            */
/*                                                                            */
/* SIDEMU_NONE                                                                */
/*                                                                            */
/*   No volume control at all. Volume level of each voice is not adjustable.  */
/*   Voices cannot be turned off. No panning possible. Most likely maximum    */
/*   software mixing speed.                                                   */
/*                                                                            */
/*                                                                            */
/* SIDEMU_VOLCONTROL                                                          */
/*                                                                            */
/*   In SIDEMU_STEREO mode two voices should build a pair, satisfying the     */
/*   equation (leftlevel_A + leftlevel_B) <= 255. Generally, the equations:   */
/*     sum leftLevel(i) <= 512   and   sum rightLevel(i) <= 512               */
/*   must be satisfied, i = [1,2,3,4].                                        */
/*                                                                            */
/*   In SIDEMU_MONO mode only the left level is used to specify a voice's     */
/*   volume. If you specify a right level, it will be set to zero.            */
/*                                                                            */
/*                                                                            */
/* SIDEMU_FULLPANNING                                                         */
/*                                                                            */
/*   Volume level of each voice is adjustable between 255 (max) and 0 (off).  */
/*   Each voice can be freely positioned between left and right, or both.     */
/*                                                                            */
/*                                                                            */
/* SIDEMU_STEREOSURROUND                                                      */
/*                                                                            */
/*   Volume level of each voice is adjustable between 255 (max) and 0 (off).  */
/*   Each voice can be freely positioned between left and right.              */
/*   Effect is best for left=255 plus right=255.                              */
/*                                                                            */
/*                                                                            */
/* SIDEMU_HWMIXING                                                            */
/*                                                                            */
/*   Used for external mixing only. The sample buffer is split into four (4)  */
/*   equivalent chunks, each representing a single voice. The client has to   */
/*   take care of the sample buffer length to be dividable by four.           */
/******************************************************************************/



/******************************************************************************/
/* SIDEmuEngine class                                                         */
/******************************************************************************/
class SIDEmuEngine
{
public:
	// The constructor creates and initializes the object with defaults.
	// Upon successful creation, use SIDEmuEngine::GetConfig(...) to
	// retrieve the default settings.
	SIDEmuEngine(void);
	virtual ~SIDEmuEngine();

	bool GetStatus(void) { return (isReady); };
	void GetConfig(sidEmuConfig &outCfg);
	bool SetConfig(const sidEmuConfig &inCfg);

	void AmplifyThreeVoiceTunes(bool inIsThreeVoiceTune);
	void ResetSecondsThisSong(void);

	void MPUReset(void);
	uint8 *MPUReturnRAMBase(void);
	void Reset(void);

	SID6581 sid6581;
	SID6510 sid6510;

	// Used for time and listening mileage
	uint32 bytesCountTotal;
	uint32 bytesCountSong;
	int32 secondsTotal;
	int32 secondsThisSong;

	uint32 splitBufferLen;

	FillFunc sidEmuFillFunc;

protected:
	void SetRandomSeed(void);
	bool ResetSampleEmu(void);

	bool AllocMem(void);
	bool FreeMem(void);

	void ConfigureSID(void);

	void InitMixerEngine(void);
	void SetDefaultVoiceVolumes(void);
	bool SetVoiceVolume(int32 voice, uint8 leftLevel, uint8 rightLevel, uint16 total);
	void SetDefaultFilterStrength(void);
	void FilterTableInit(void);

	void MixerInit(bool threeVoiceAmplify, uint8 zero8, uint16 zero16);
	inline void SyncEm(void);
	static void *Fill16BitSplit(SIDEmuEngine *obj, void *buffer, uint32 numberOfSamples);

	bool isReady;
	sidEmuConfig config;

	// 6510-interpreter
	//
	bool mpuStatus;
	uint8 randomSeed;

	bool isThreeVoiceAmplified;		// Keep track of current mixer state
	bool isThreeVoiceTune;

	int8 *signedPanMix8;
	int16 *signedPanMix16;

	uint8 zero8Bit;					// ``zero''-sample
	uint16 zero16Bit;				// Either signed or unsigned

	FillFunc fillFunctions[2][2][4];

	uint8 mix8Mono[256 * sidMaxLogicalVoices];
	uint8 mix8Stereo[256 * (sidMaxLogicalVoices / 2)];

	uint16 mix16Mono[256 * sidMaxLogicalVoices];
	uint16 mix16Stereo[256 * (sidMaxLogicalVoices / 2)];
};

#endif
