/******************************************************************************/
/* APChannel header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APChannel_h
#define __APChannel_h

// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "Import_Export.h"


/******************************************************************************/
/* Useful constants                                                           */
/******************************************************************************/
enum AP_LoopType { APLOOP_Normal, APLOOP_PingPong, APLOOP_Retrig };

// Panning values
#define APPAN_LEFT			0
#define APPAN_CENTER		128
#define APPAN_RIGHT			255
#define APPAN_SURROUND		512



/******************************************************************************/
/* Channel flags. Can be used in visualizer agents.                           */
/******************************************************************************/
#define NP_MUTEIT			0x00000001	// Mute the channel
#define NP_TRIGIT			0x00000002	// Trig the sample (start over)
#define NP_16BIT			0x00000004	// Set this if the sample is 16 bit
#define NP_LOOP				0x00000008	// The sample loops
#define NP_PINGPONG			0x00000010	// Set this together with the NP_LOOP flag for ping-pong loop
#define NP_RETRIGLOOP		0x00000020	// Set this to retrig the sample when setting the loop information
#define NP_SPEAKERVOLUME	0x00000100	// Speaker volume changed. Overrules NP_VOLUME and NP_PANNING
#define NP_VOLUME			0x00000200	// Volume changed
#define NP_PANNING			0x00000400	// Panning changed
#define NP_FREQUENCY		0x00001000	// New frequency
#define NP_RELEASE			0x00002000	// Release the sample
#define NP_FILTER			0x40000000	// Enable global low-pass filter
#define NP_ACTIVE			0x80000000	// This is a readonly bit. When a sample is playing in the channel, it's set



/******************************************************************************/
/* APChannel class                                                            */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_APKIT APChannel
{
public:
	APChannel(void);
	virtual ~APChannel(void);

	// Functions used in the player
	void SetBuffer(const void *adr, uint32 length);
	void PlaySample(const void *adr, uint32 startOffset, uint32 length, uint8 bit = 8);
	void SetLoop(uint32 startOffset, uint32 length, AP_LoopType type = APLOOP_Normal);
	void SetLoop(const void *adr, uint32 startOffset, uint32 length, AP_LoopType type = APLOOP_Normal);
	void PlayReleasePart(const void *adr, uint32 length);

	void SetVolume(uint16 vol);
	void SetLeftVolume(uint16 vol);
	void SetRightVolume(uint16 vol);
	void SetPanning(uint16 pan);

	void SetFrequency(uint32 freq);
	void SetAmigaPeriod(uint32 period);

	bool IsActive(void) const;
	void Mute(void);

	uint16 GetVolume(void) const;
	uint32 GetFrequency(void) const;

protected:
	uint32 flags;
	const void *sampAddress;	// Start address of the sample
	const void *loopAddress;	// Start address of the loop/release sample
	uint32 sampStart;			// Start offset in the sample in samples, not bytes
	uint32 sampLength;			// Length of the sample in samples, not bytes
	uint32 loopStart;			// Loop offset in the sample in samples, not bytes
	uint32 loopLength;			// Loop length in samples, not bytes
	uint32 releaseLength;		// Length of the release part of the sample
	uint32 frequency;			// The frequency to play with
	uint16 volume;				// The volume (0-256)
	uint16 leftVolume;			// The left volume (0-256)
	uint16 rightVolume;			// The right volume (0-256)
	uint16 panning;				// The panning (0 = left; 128 = middle; 255 = right, 512 = Surround)
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
