/******************************************************************************/
/* APMixerBase header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APMixerBase_h
#define __APMixerBase_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* Mixer modes                                                                */
/******************************************************************************/
enum MixerModes
{
//	DMODE_16BITS   = 0x0001,
	DMODE_STEREO   = 0x0002,
//	DMODE_HQMIXER  = 0x0010,
	DMODE_SURROUND = 0x0100,
	DMODE_INTERP   = 0x0200,

	// APlayer specific modes
	DMODE_BOOST    = 0x8000
};



/******************************************************************************/
/* Defines used in the mixer                                                  */
/******************************************************************************/

// Sample format [loading and in-memory] flags
#define SF_16BITS				0x0001

// General Playback flags
#define SF_LOOP					0x0100
#define SF_BIDI					0x0200
#define SF_REVERSE				0x0400
#define SF_RELEASE				0x1000

// APlayer specific flags
#define SF_SPEAKER				0x8000

// Panning constants
#define PAN_LEFT				0
#define PAN_CENTER				128
#define PAN_RIGHT				256
#define PAN_SURROUND			512		// Panning value for Dolby Surround



/******************************************************************************/
/* Structures                                                                 */
/******************************************************************************/
typedef struct VINFO
{
	bool	enabled;		// True -> The channel is enabled
	bool	kick;			// True -> Sample has to be restarted
	bool	active;			// True -> Sample is playing
	uint16	flags;			// 16/8 bits, looping/one-shot etc.
	const void *adr;		// Address to the sample
	const void *loopAdr;	// Address to the loop point (mostly the same as adr above)
	uint32	start;			// Start index
	uint32	size;			// Sample size
	uint32	repPos;			// Loop start
	uint32	repEnd;			// Loop end
	uint32	releaseLen;		// Release length
	uint32	frq;			// Current frequency
	int32	leftVol;		// Current volume in left speaker
	int32	rightVol;		// Current volume in right speaker
	int32	pan;			// Current panning position
	int32	rampVol;
	int32	lVolSel;		// Volume factor in range 0-255
	int32	rVolSel;
	int32	oldLVol;
	int32	oldRVol;
	int64	current;		// Current index in the sample
	int64	increment;		// Increment value
} VINFO;



/******************************************************************************/
/* APMixerBase class                                                          */
/******************************************************************************/
class APMixerBase
{
public:
	APMixerBase(void);
	virtual ~APMixerBase(void);

	bool Initialize(uint32 frequency, uint16 channels);
	void ClearVoices(void);
	void Cleanup(void);

	void SetVolume(uint16 volume);
	void SetStereoSeparation(uint16 sep);

	VINFO *GetMixerChannels(void);

	bool IsActive(uint16 channel);
	void EnableChannel(uint16 channel, bool enable);

	virtual int32 GetClickConstant(void) = 0;

	void Mixing(int32 *dest, int32 todo, uint32 mode);
	void AddEffects(int32 *dest, int32 todo, uint32 mode);
	void ConvertMixedData(int16 *dest, int32 *source, int32 todo, uint32 mode);

protected:
	virtual bool InitMixer(void) = 0;
	virtual void EndMixer(void) = 0;

	// Mixer functions
	virtual void DoMixing(int32 *dest, int32 todo, uint32 mode) = 0;
	virtual void Mix32To16(int16 *dest, int32 *source, int32 count, uint32 mode) = 0;

	// Mixer variables
	uint32 mixerFreq;		// The mixer frequency
	uint16 masterVol;		// This is the master volume (0-256)
	uint16 channelNum;		// Number of channels this mixer use
	uint16 stereoSep;		// This is the stereo separation (0-128)

	VINFO *vinf;			// Pointer to VINFO structures
};

#endif
