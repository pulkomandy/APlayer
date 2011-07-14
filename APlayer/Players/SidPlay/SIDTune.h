/******************************************************************************/
/* SIDTune header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDTUNE_H
#define __SIDTUNE_H

// PolyKit headers
#include "POS.h"
#include "PString.h"

// Player headers
#include "SIDFile.h"


/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define sidClassMaxSongs			256

#define SIDTUNE_SPEED_VBI			0			// Vertical-Blanking-Interrupt
#define SIDTUNE_SPEED_CIA_1A		60			// CIA 1 Timer A

#define SIDTUNE_CLOCK_UNKNOWN		0			// These are also used in the
#define SIDTUNE_CLOCK_PAL			0x01		// emulator engine!
#define SIDTUNE_CLOCK_NTSC			0x02		// (binary)
#define SIDTUNE_CLOCK_ANY			(SIDTUNE_CLOCK_PAL | SIDTUNE_CLOCK_NTSC)

#define SIDTUNE_SIDMODEL_UNKNOWN	0
#define SIDTUNE_SIDMODEL_6581		0x01
#define SIDTUNE_SIDMODEL_8580		0x02
#define SIDTUNE_SIDMODEL_ANY		(SIDTUNE_SIDMODEL_6581 | SIDTUNE_SIDMODEL_8580)

#define sidMaxLogicalVoices			4
#define sidNoiseSeed				0x7ffff8;



/******************************************************************************/
/* Endian types and values                                                    */
/******************************************************************************/
#if defined(__INTEL__)
  #define LO 0
  #define HI 1
  #define LOLO 0
  #define LOHI 1
  #define HILO 2
  #define HIHI 3
#else
  #define LO 1
  #define HI 0
  #define LOLO 3
  #define LOHI 2
  #define HILO 1
  #define HIHI 0
#endif

union CPULWord
{
	uint16 w[2];	// Single 16-bit low and high word
	uint32 l;		// Complete 32-bit longword
};

union CPULBWord
{
	uint8 b[4];		// Single 8-bit bytes
	uint32 l;		// Complete 32-bit longword
};



/******************************************************************************/
/* Type definations                                                           */
/******************************************************************************/
typedef float FilterFloat;



/******************************************************************************/
/* SIDTune info structure                                                     */
/******************************************************************************/
typedef struct sidTuneInfo
{
	// Consider the following fields as read-only!
	//
	// Currently, the only way to get the class to accept values which
	// were written to these fields is creating a derived class.
	//
	uint16 loadAddr;
	uint16 initAddr;
	uint16 playAddr;
	uint16 startSong;
	uint16 songs;

	//
	// Available after song initialization
	//
	uint16 irqAddr;					// If (playAddr == 0), interrupt handler was
									// installed and starts calling the C64 player
									// at this address.
	uint16 currentSong;				// The one that has been initialized
	uint8 songSpeed;				// Initialized playing speed
	uint8 clockSpeed;				// Initialized clock chip speed
	bool musPlayer;					// Whether Sidplayer routine has been installed
	bool psidSpecific;				// Whether PlaySID specific extensions are used
	uint8 clock;					// Required clock chip speed (PAL, NTSC)
	uint8 sidModel;					// Required SID chip model
	bool fixLoad;					// Whether load address might be duplicate
	uint16 lengthInSeconds;			// --- Reserved ---

	uint8 relocStartPage;			// First available page for relocation
	uint8 relocPages;				// Number of pages available for relocation

	uint32 c64DataLen;				// Length of raw C64 data

	//
	// Song, title, credits, ...
	//
	PString nameString;
	PString authorString;
	PString copyrightString;
	PString infoStrings[5];
} sidTypeInfo;



/******************************************************************************/
/* SIDTune class                                                              */
/******************************************************************************/
class SIDTune
{
public:
	SIDTune(SIDFile *file);
	virtual ~SIDTune(void);

	uint16 SelectSong(uint16 selectedSong);
	void SetIRQAddress(uint16 address);

	uint8 GetSongSpeed(void);
	uint16 GetPlayAddr(void);

	bool PlaceSidTuneInC64Mem(uint8 *c64Buf);
	void MUS_InstallPlayer(uint8 *c64Buf);

	sidTuneInfo info;
	bool status;

protected:
	void ConvertOldStyleSpeedToTables(uint32 oldStyleSpeed);
	bool CacheRawData(void *sourceBuffer, uint32 sourceBufLen);

	SIDFile *sidFile;

	uint8 songSpeed[sidClassMaxSongs];
	uint8 clockSpeed[sidClassMaxSongs];		// Not fully used by file formats
	uint16 songLength[sidClassMaxSongs];	// Reserved

	bool isCached;
	uint8 *cachePtr;
	uint32 cacheLen;

private:
	void SafeConstructor(SIDFile *file);
};

#endif
