/******************************************************************************/
/* The MikMod sound library include file.                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MIKMODSTD_h
#define __MIKMODSTD_h

// PolyKit headers
#include "POS.h"
#include "PString.h"


/******************************************************************************/
/* Samples                                                                    */
/******************************************************************************/
// Sample format [loading and in-memory] flags
#define SF_16BITS				0x0001
#define SF_SIGNED				0x0002
#define SF_STEREO				0x0004
#define SF_BIG_ENDIAN			0x0008
#define SF_DELTA				0x0010
#define SF_ITPACKED				0x0020

#define SF_FORMATMASK			0x003f

// General Playback flags
#define SF_LOOP					0x0100
#define SF_BIDI					0x0200
#define SF_REVERSE				0x0400
#define SF_SUSTAIN				0x0800

// Module-only Playback flags
#define SF_OWNPAN				0x1000
#define SF_UST_LOOP				0x2000

// Panning constants
#define PAN_LEFT				0
#define PAN_HALFLEFT			64
#define PAN_CENTER				128
#define PAN_HALFRIGHT			192
#define PAN_RIGHT				255
#define PAN_SURROUND			512		// Panning value for Dolby Surround



/******************************************************************************/
/* SAMPLE structure                                                           */
/******************************************************************************/
typedef struct SAMPLE
{
	int16		panning;		// Panning (0-255 or PAN_SURROUND)
	uint32		speed;			// Base playing speed/frequency of note
	uint8		volume;			// Volume 0-64
//	uint16		inFlags;		// Sample format on disk
	uint16		flags;			// Sample format in memory
	uint32		length;			// Length of sample (in samples!)
	uint32		loopStart;		// Repeat position (relative to start, in samples)
	uint32		loopEnd;		// Repeat end
	uint32		susBegin;		// Sustain loop begin (in samples)  \ Not supported
	uint32		susEnd;			// Sustain loop end                 / yet!

	// Variables used by the module player only!
	uint8		globVol;		// Global volume
	uint8		vibFlags;		// Autovibrato flag stuffs
	uint8		vibType;		// Vibratos moved from INSTRUMENT to SAMPLE
	uint8		vibSweep;
	uint8		vibDepth;
	uint8		vibRate;
	PString		sampleName;		// Name of the sample

	// Values used internally only (not saved in disk formats)
	uint16		aVibPos;		// Autovibrato pos [player use]
//	uint8		divFactor;		// For sample scaling, maintains proper period slides
	uint32		seekPos;		// Seek position in file
	void *		handle;			// Sample handle. Points to the sample in memory
} SAMPLE;



/******************************************************************************/
/* Internal module representation (UniMod)                                    */
/******************************************************************************/
#define INSTNOTES				120
#define ENVPOINTS				32



/******************************************************************************/
/* ENVPT structure                                                            */
/******************************************************************************/
typedef struct ENVPT
{
	int16		pos;
	int16		val;
} ENVPT;



/******************************************************************************/
/* INSTRUMENT structure                                                       */
/******************************************************************************/
typedef struct INSTRUMENT
{
	PString		insName;

	uint8		flags;

	uint16		sampleNumber[INSTNOTES];
	uint8		sampleNote[INSTNOTES];

	uint8		nnaType;
	uint8		dca;			// Duplicate check action
	uint8		dct;			// Duplicate check type
	uint8		globVol;
	uint16		volFade;
	uint16		panning;		// Instrument-based panning var

	uint8		pitPanSep;		// Pitch pan separation (0 to 255)
	uint8		pitPanCenter;	// Pitch pan center (0 to 119)
	uint8		rVolVar;		// Random volume varations (0 - 100%)
	uint8		rPanVar;		// Randon panning varations (0 - 100%)

	// Volume envelope
	uint8		volFlg;			// bit 0: on 1: sustain 2: loop
	uint8		volPts;
	uint8		volSusBeg;
	uint8		volSusEnd;
	uint8		volBeg;
	uint8		volEnd;
	ENVPT		volEnv[ENVPOINTS];

	// Panning envelope
	uint8		panFlg;			// bit 0: on 1: sustain 2: loop
	uint8		panPts;
	uint8		panSusBeg;
	uint8		panSusEnd;
	uint8		panBeg;
	uint8		panEnd;
	ENVPT		panEnv[ENVPOINTS];

	// Pitch envelope
	uint8		pitFlg;			// bit 0: on 1: sustain 2: loop
	uint8		pitPts;
	uint8		pitSusBeg;
	uint8		pitSusEnd;
	uint8		pitBeg;
	uint8		pitEnd;
	ENVPT		pitEnv[ENVPOINTS];
} INSTRUMENT;



/******************************************************************************/
/* Module flags                                                               */
/******************************************************************************/
#define UF_MAXCHAN				64		// Maximum master channels supported

#define UF_XMPERIODS			0x0001	// XM periods / finetuning
#define UF_LINEAR				0x0002	// LINEAR periods (UF_XMPERIODS must be set)
#define UF_INST					0x0004	// Instruments are used
#define UF_NNA					0x0008	// New note actions used (set numvoices rather than numchn)
#define UF_S3MSLIDES			0x0010	// Uses old S3M volume slides
#define UF_BGSLIDES				0x0020	// Continue volume slides in the background
#define UF_HIGHBPM				0x0040	// Can use >255 bpm
#define UF_NOWRAP				0x0080	// XM-type (i.e. illogical) pattern break semantics
#define UF_ARPMEM				0x0100	// IT: Need arpeggio memory
#define UF_FT2QUIRKS			0x0200	// Emulate some FT2 replay quirks
#define UF_PANNING				0x0400	// Module uses panning effects or have non-tracker default initial panning



/******************************************************************************/
/* UNIMOD structure                                                           */
/******************************************************************************/
struct MP_CONTROL;
struct MP_VOICE;

typedef struct MODULE
{
	// General module information
	PString		songName;				// Name of the song
//	PString		modType;				// String type of module loaded
	PString		comment;				// Module comments

	uint16		flags;					// See module flags above
	uint8		numChn;					// Number of module channels
	uint8		numVoices;				// Max # voices used for full NNA playback
	uint16		numPos;					// Number of positions in this song
	uint16		numPat;					// Number of patterns in this song
	uint16		numIns;					// Number of instruments
	uint16		numSmp;					// Number of samples
	INSTRUMENT *instruments;			// All instruments
	SAMPLE *	samples;				// All samples
	uint8		realChn;				// Real number of channels used
	uint8		totalChn;				// Total number of channels used (incl NNAs)

	// Playback settings
	uint16		repPos;					// Restart position
	uint8		initSpeed;				// Initial song speed
	uint16		initTempo;				// Initial song tempo
	uint8		initVolume;				// Initial global volume (0 - 128)
	uint16		panning[UF_MAXCHAN];	// Panning positions
	uint8		chanVol[UF_MAXCHAN];	// Channel positions
	uint16		bpm;					// Current beats-per-minute speed
	uint16		sngSpd;					// Current song speed
	int16		volume;					// Song volume (0-128) (or user volume)

	bool		extSpd;					// Extended speed flag (default enabled)
	bool		panFlag;				// Panning flag (default enabled)
	bool		wrap;					// Wrap module? (default disabled)
	bool		loop;					// Allow module to loop? (default enabled)
	bool		fadeOut;				// Volume fade out during last pattern

	uint16		patPos;					// Current row number
	int16		sngPos;					// Current song position
	uint32		sngTime;				// Current song time in 2^-10 seconds

	int16		relSpd;					// Relative speed factor

	// Internal module representation
	uint16		numTrk;					// Number of tracks
	uint8 **	tracks;					// Array of numTrk pointers to tracks
	uint16 *	patterns;				// Array of Patterns
	uint16 *	pattRows;				// Array of number of rows for each pattern
	uint16 *	positions;				// All positions

//	bool		forbid;					// If true, no player update!
	uint16		numRow;					// Number of rows in current pattern
	uint16		vbTick;					// Tick counter (counts from 0 to sngspd)
	uint16		sngRemainder;			// Used for song computation

	MP_CONTROL *control;				// Effect Channel information
	MP_VOICE *	voice;					// Audio Voice information

	uint8		globalSlide;			// Global volume slide rate
	uint8		pat_repCrazy;			// Module has just looped to position -1
	uint16		patBrk;					// Position where to start a new pattern
	uint8		patDly;					// Patterndelay counter (command memory)
	uint8		patDly2;				// Patterndelay counter (real one)
	int16		posJmp;					// Flag to indicate a position jump is needed...
	uint16		bpmLimit;				// Threshold to detect BPM or speed values

	bool		equalPos;				// APlayer: Used for end detection
} MODULE;

#endif
