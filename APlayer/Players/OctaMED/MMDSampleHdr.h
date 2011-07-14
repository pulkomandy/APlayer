/******************************************************************************/
/* MMDSampleHdr header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MMDSampleHdr_h
#define __MMDSampleHdr_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Sample.h"


/******************************************************************************/
/* Instrument type definitions                                                */
/******************************************************************************/
#define MMD_INSTR_16BIT				0x0010
#define MMD_INSTR_STEREO			0x0020
#define MMD_INSTR_DELTACODE			0x0040
#define MMD_INSTR_PACK				0x0080



/******************************************************************************/
/* MMDSampleHdr class                                                         */
/*                                                                            */
/* Sample header in MMD files:                                                */
/*                                                                            */
/* uint32 length       Length of *one* *unpacked* channel in *bytes*          */
/* uint16 type         See the above flag bits for definitions.               */
/*                     Bits 0-3 reserved for multi-octave instruments, which  */
/*                     are not supported.                                     */
/*                                                                            */
/* If type & MMD_INSTR_PACK, these fields follow:                             */
/*                                                                            */
/* uint16 packType     See MMD_INSTRPACK_xx above                             */
/* uint16 subType      Packer subtype, see above                              */
/* uint8  commonFlags  Flags common to all packtypes (none defined so far)    */
/* uint8  packerFlags  Flags for the specific packtype                        */
/* uint32 leftChLen    Packed length of left channel in bytes                 */
/* uint32 rightChLen   Packed length of right channel in bytes. (ONLY IN      */
/*                     STEREO SAMPLES)                                        */
/* + possible other packer-dependent fields                                   */
/******************************************************************************/
class MMDSampleHdr
{
public:
	MMDSampleHdr(OctaMED *octaMED, PFile *f, PString &errorStr, PResource *res);
	virtual ~MMDSampleHdr(void);

	Sample *AllocSample(void) const;
	void ReadSampleData(PFile *f, Sample *dest);

	bool IsSample(void) const;
	bool IsSynth(void) const;
	bool IsHybrid(void) const;
	bool IsStereo(void) const;

protected:
	OctaMED *med;

	uint32 numFrames;
	bool sixtBit;
	bool stereo;
	int16 type;
	uint16 packType;
	uint16 subType;
	bool skipThis;
};

#endif
