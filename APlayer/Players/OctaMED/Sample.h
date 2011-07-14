/******************************************************************************/
/* Sample header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Sample_h
#define __Sample_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Mixer.h"


/******************************************************************************/
/* Sample class                                                               */
/******************************************************************************/
class Instr;

class Sample
{
public:
	Sample(OctaMED *octaMED, uint32 length = 0, bool stereo = false, bool sixtBit = false);
	virtual ~Sample(void);

	void SetProp(uint32 length, bool stereo, bool sixtBit);
	void SetLength(uint32 newLen, bool clear = false);
	void SetStereo(bool stereo);
	void Set16Bit(bool sixtBit);

	void SetData8(uint32 pos, uint16 ch, int8 smp);
	void SetData16(uint32 pos, uint16 ch, int16 smp);

	uint32 GetLength(void) const;
	const int8 *GetSampleAddress(uint16 ch) const;

	bool Is16Bit(void) const;

	Instr *GetInstrument(void) const;
	void ValidateLoop(void);

	virtual bool IsSynthSound(void) const;

protected:
	friend class MED_Mixer;

	int32 ByteLength(void) const;

	OctaMED *med;

	int8 *s_data[2];
	uint32 s_dataLen;				// Length of one sample
	uint32 s_length;				// Sample length in samples
	bool s_isStereo;
	bool s_is16Bit;
	uint16 channels;				// 1 = Mono, 2 = Stereo, 0 = None
};

#endif
