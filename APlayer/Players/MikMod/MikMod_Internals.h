/******************************************************************************/
/* MikMod sound library internal definitions.                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MIKMOD_INTERNALS_h
#define __MIKMOD_INTERNALS_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MikModStd.h"


/******************************************************************************/
/* Internal module representation (UniMod) interface                          */
/******************************************************************************/
#define OCTAVE					12



/******************************************************************************/
/* Module Commands                                                            */
/******************************************************************************/
enum
{
	// Simple note
	UNI_NOTE = 1,
	// Instrument change
	UNI_INSTRUMENT,
	// Protracker effects
	UNI_PTEFFECT0,		// Arpeggio
	UNI_PTEFFECT1,		// Porta up
	UNI_PTEFFECT2,		// Porta down
	UNI_PTEFFECT3,		// Porta to note
	UNI_PTEFFECT4,		// Vibrato
	UNI_PTEFFECT5,		// Dual effect 3+A
	UNI_PTEFFECT6,		// Dual effect 4+A
	UNI_PTEFFECT7,		// Tremolo
	UNI_PTEFFECT8,		// Pan
	UNI_PTEFFECT9,		// Sample offset
	UNI_PTEFFECTA,		// Volume slide
	UNI_PTEFFECTB,		// Pattern jump
	UNI_PTEFFECTC,		// Set volume
	UNI_PTEFFECTD,		// Pattern break
	UNI_PTEFFECTE,		// Extended effects
	UNI_PTEFFECTF,		// Set speed
	// Scream tracker effects
	UNI_S3MEFFECTA,		// Set speed
	UNI_S3MEFFECTD,		// Volume slide
	UNI_S3MEFFECTE,		// Porta down
	UNI_S3MEFFECTF,		// Porta up
	UNI_S3MEFFECTI,		// Tremor
	UNI_S3MEFFECTQ,		// Retrig
	UNI_S3MEFFECTR,		// Tremolo
	UNI_S3MEFFECTT,		// Set tempo
	UNI_S3MEFFECTU, 	// Fine vibrato
	UNI_KEYOFF,			// Note off
	// Fast tracker effects
	UNI_KEYFADE,		// Note fade
	UNI_VOLEFFECTS,		// Volume column effects
	UNI_XMEFFECT4,		// Vibrato
	UNI_XMEFFECT6,		// Dual effect 4+A
	UNI_XMEFFECTA,		// Volume slide
	UNI_XMEFFECTE1,		// Fine porta up
	UNI_XMEFFECTE2,		// Fine porta down
	UNI_XMEFFECTEA,		// Fine volume slide up
	UNI_XMEFFECTEB,		// Fine volume slide down
	UNI_XMEFFECTG,		// Set global volume
	UNI_XMEFFECTH,		// Global volume slide
	UNI_XMEFFECTL,		// Set envelope position
	UNI_XMEFFECTP,		// Pan slide
	UNI_XMEFFECTX1,		// Extra fine porta up
	UNI_XMEFFECTX2,		// Extra fine porta down
	// Impulse tracker effects
	UNI_ITEFFECTG,		// Porta to note
	UNI_ITEFFECTH,		// Vibrato
	UNI_ITEFFECTI,		// Tremor
	UNI_ITEFFECTM,		// Set channel volume
	UNI_ITEFFECTN,		// Slide / Fineslide channel volume
	UNI_ITEFFECTP,		// Slide / Fineslide channel panning
	UNI_ITEFFECTT,		// Slide tempo
	UNI_ITEFFECTU,		// Fine vibrato
	UNI_ITEFFECTW,		// Slide / Fineslide global volume
	UNI_ITEFFECTY,		// Panbrello
	UNI_ITEFFECTZ,		// Resonant filters
	UNI_ITEFFECTS0,
	// UltraTracker effects
	UNI_ULTEFFECT9,		// Sample fine offset
 	// Effects below here are not used by the MikModConverter, but
 	// they are implemented to make it backwards compatible with
 	// UNIMOD files
 	//
 	// OctaMED effects
 	UNI_MEDSPEED,
 	UNI_MEDEFFECTF1,	// Play note twice
 	UNI_MEDEFFECTF2,	// Delay note
 	UNI_MEDEFFECTF3,	// Play note three times
 	// Oktalyzer effects
 	UNI_OKTARP,			// Arpeggio

	UNI_LAST
};



/******************************************************************************/
/* IT / S3M Extended SS effects                                               */
/******************************************************************************/
enum
{
	SS_GLISSANDO = 1,
	SS_FINETUNE,
	SS_VIBWAVE,
	SS_TREMWAVE,
	SS_PANWAVE,
	SS_FRAMEDELAY,
	SS_S7EFFECTS,
	SS_PANNING,
	SS_SURROUND,
	SS_HIOFFSET,
	SS_PATLOOP,
	SS_NOTECUT,
	SS_NOTEDELAY,
	SS_PATDELAY
};



/******************************************************************************/
/* IT volume column effect                                                    */
/******************************************************************************/
enum
{
	VOL_VOLUME = 1,
	VOL_PANNING,
	VOL_VOLSLIDE,
	VOL_PITCHSLIDEDN,
	VOL_PITCHSLIDEUP,
	VOL_PORTAMENTO,
	VOL_VIBRATO
};



/******************************************************************************/
/* IT resonant filter information                                             */
/******************************************************************************/
#define UF_MAXMACRO				0x10
#define UF_MAXFILTER			0x100

#define FILT_CUT				0x80
#define FILT_RESONANT			0x81

typedef struct FILTER
{
	uint8		filter;
	uint8		inf;
} FILTER;



/******************************************************************************/
/* Instruments                                                                */
/******************************************************************************/
// Instrument format flags
#define IF_OWNPAN				1
#define IF_PITCHPAN				2

// Envelope flags
#define EF_ON					1
#define EF_SUSTAIN				2
#define EF_LOOP					4
#define EF_VOLENV				8

// New Note Action flags
#define NNA_CUT					0
#define NNA_CONTINUE			1
#define NNA_OFF					2
#define NNA_FADE				3
#define NNA_MASK				3

#define DCT_OFF					0
#define DCT_NOTE				1
#define DCT_SAMPLE				2
#define DCT_INST				3

#define DCA_CUT					0
#define DCA_OFF					1
#define DCA_FADE				2

#define KEY_KICK				0
#define KEY_OFF					1
#define KEY_FADE				2
#define KEY_KILL				(KEY_OFF | KEY_FADE)

#define KICK_ABSENT				0
#define KICK_NOTE				1
#define KICK_KEYOFF				2
#define KICK_ENV				4

#define AV_IT					1		// IT vs. XM vibrato info



/******************************************************************************/
/* Playing                                                                    */
/******************************************************************************/
#define POS_NONE				(-2)			// No loop position defined
#define LAST_PATTERN			(uint16)(-1)	// Special 'end of song' pattern



/******************************************************************************/
/* Flags for S3MIT_ProcessCmd                                                 */
/******************************************************************************/
#define S3MIT_OLDSTYLE			1		// Behave as old ScreamTracker
#define S3MIT_IT				2		// Behave as ImpulseTracker
#define S3MIT_SCREAM			4		// Enforce ScreamTracker specific limits



/******************************************************************************/
/* ENVPR structure                                                            */
/******************************************************************************/
typedef struct ENVPR
{
	uint8		flg;			// Envelope flag
	uint8		pts;			// Number of envelope points
	uint8		susBeg;			// Envelope sustain index begin
	uint8		susEnd;			// Envelope sustain index end
	uint8		beg;			// Envelope loop begin
	uint8		end;			// Envelope loop end
	int16		p;				// Current envelope counter
	uint16		a;				// Envelope index a
	uint16		b;				// Envelope index b
	ENVPT *		env;			// Envelope points
} ENVPR;



/******************************************************************************/
/* MP_CHANNEL structure                                                       */
/******************************************************************************/
typedef struct MP_CHANNEL
{
	INSTRUMENT *i;
	SAMPLE *	s;
	uint8		sample;			// Which sample number
	uint8		note;			// The audible note (as heard, direct rep of period)
	int16		outVolume;		// Output volume (vol + sampVol + instVol)
	int8		chanVol;		// Channel's "global" volume
	uint16		fadeVol;		// Fading volume rate
	int16		panning;		// Panning position
	uint8		kick;			// If true = sample has to be restarted
	uint16		period;			// Period to play the sample at
	uint8		nna;			// New Note Action type + master/slave flags

	uint8		volFlg;			// Volume envelope settings
	uint8		panFlg;			// Panning envelope settings
	uint8		pitFlg;			// Pitch envelope settings

	uint8		keyOff;			// If true = fade out and stuff
	uint8*		handle;			// Which sample-handle
	uint8		noteDelay;		// (Used for note delay)
	int32		start;			// The starting byte index in the sample
} MP_CHANNEL;



/******************************************************************************/
/* MP_VOICE structure                                                         */
/*                                                                            */
/* Used by NNA only player (audio control. AUDTMP is used for full effects    */
/* control.                                                                   */
/******************************************************************************/
typedef struct MP_VOICE
{
	MP_CHANNEL	main;

	ENVPR		vEnv;
	ENVPR		pEnv;
	ENVPR		cEnv;

	uint16		aVibPos;		// Autovibrato pos
	uint16		aSwpPos;		// Autovibrato sweep pos

	uint32		totalVol;		// Total volume of channel (before global mixings)

	bool		mFlag;
	int16		masterChn;
	uint16		masterPeriod;
	MP_CONTROL *master;			// Index of "master" effects channel
} MP_VOICE;



/******************************************************************************/
/* MP_CONTROL structure                                                       */
/******************************************************************************/
typedef struct MP_CONTROL
{
	MP_CHANNEL	main;

	MP_VOICE *	slave;			// Audio slave of current effects control channel

	uint8		slaveChn;		// Audio slave of current effects control channel
	uint8		muted;			// If set, channel not played
	uint16		ultOffset;		// Fine sample offset memory
	uint8		aNote;			// The note that indexes the audible
	uint8		oldNote;
	int16		ownPer;
	int16		ownVol;
	uint8		dca;			// Duplicate check action
	uint8		dct;			// Duplicate check type
	uint8 *		row;			// Row currently playing on this channel
	int8		retrig;			// Retrig value (0 means don't retrig)
	uint32		speed;			// What finetune to use
	int16		volume;			// Amiga volume (0 t/m 64) to play the sample at

	int16		tmpVolume;		// Tmp volume
	uint16		tmpPeriod;		// Tmp period
	uint16		wantedPeriod;	// Period to slide to (with effect 3 or 5)

	uint8		arpMem;			// Arpeggio command memory
	uint8		pansSpd;		// Panslide speed
	uint16		slideSpeed;
	uint16		portSpeed;		// Noteslide speed (toneportamento)

	uint8		s3mTremor;		// S3M tremor (effect I) counter
	uint8		s3mTrOnOf;		// S3M tremor ontime/offtime
	uint8		s3mVolSlide;	// Last used volslide
	int8		sliding;
	uint8		s3mRtgSpeed;	// Last used retrig speed
	uint8		s3mRtgSlide;	// Last used retrig slide

	uint8		glissando;		// Glissando (0 means off)
	uint8		waveControl;

	int8		vibPos;			// Current vibrato position
	uint8		vibSpd;			// "" speed
	uint8		vibDepth;		// "" depth

	int8		trmPos;			// Current tremolo position
	uint8		trmSpd;			// "" speed
	uint8		trmDepth;		// "" depth

	uint8		fSlideUpSpd;
	uint8		fSlideDnSpd;
	uint8		fPortUpSpd;		// fx E1 (extra fine portamento up) data
	uint8		fPortDnSpd;		// fx E2 (extra fine portamento down) data
	uint8		ffPortUpSpd;	// fx X1 (extra fine portamento up) data
	uint8		ffPortDnSpd;	// fx X2 (extra fine portamento down) data

	uint32		hiOffset;		// Last used high order of sample offset
	uint16		sOffset;		// Last used low order of sample offset (effect 9)

	uint8		ssEffect;		// Last used Sxx effect
	uint8		ssData;			// Last used Sxx data info
	uint8		chanVolSlide;	// Last used channel volume slide

	uint8		panbWave;		// Current panbrello waveform
	uint8		panbPos;		// Current panbrello position
	int8		panbSpd;		// "" speed
	uint8		panbDepth;		// "" depth

	uint16		newSamp;		// Set to 1 upon a sample / inst change
	uint8		volEffect;		// Volume Column Effect Memory as used by IT
	uint8		volData;		// Volume Column Data Memory

	int16		pat_repPos;		// Patternloop position
	uint16		pat_repCnt;		// Times to loop
} MP_CONTROL;

#endif
