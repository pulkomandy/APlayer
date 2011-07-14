/******************************************************************************/
/* APMixerNormal header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APMixerNormal_h
#define __APMixerNormal_h

// PolyKit headers
#include "POS.h"

// Server headers
#include "APMixerBase.h"


/******************************************************************************/
/* APMixerNormal class                                                        */
/******************************************************************************/
class APMixerNormal : public APMixerBase
{
public:
	APMixerNormal(void);
	virtual ~APMixerNormal(void);

	virtual int32 GetClickConstant(void);

protected:
	virtual bool InitMixer(void);
	virtual void EndMixer(void);

	// Mixer functions
	virtual void DoMixing(int32 *dest, int32 todo, uint32 mode);
	virtual void Mix32To16(int16 *dest, int32 *source, int32 count, uint32 mode);

	// Own functions
	void AddChannel(int32 *buf, int32 todo, uint32 mode);

	// Mixer functions using 32 bit position counter
	int32 Mix16MonoNormal(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo);
	int32 Mix16StereoNormal(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo);
	int32 Mix16SurroundNormal(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo);

	int32 Mix8MonoNormal(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo);
	int32 Mix8StereoNormal(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo);
	int32 Mix8SurroundNormal(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo);

	int32 Mix16MonoInterp(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo);
	int32 Mix16StereoInterp(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo);
	int32 Mix16SurroundInterp(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo);

	int32 Mix8MonoInterp(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo);
	int32 Mix8StereoInterp(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo);
	int32 Mix8SurroundInterp(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo);

	// Mixer functions using 64 bit position counter
	int64 Mix16MonoNormal64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo);
	int64 Mix16StereoNormal64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo);
	int64 Mix16SurroundNormal64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo);

	int64 Mix8MonoNormal64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo);
	int64 Mix8StereoNormal64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo);
	int64 Mix8SurroundNormal64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo);

	int64 Mix16MonoInterp64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo);
	int64 Mix16StereoInterp64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo);
	int64 Mix16SurroundInterp64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo);

	int64 Mix8MonoInterp64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo);
	int64 Mix8StereoInterp64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo);
	int64 Mix8SurroundInterp64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo);

	// Mixer variables
	VINFO *vnf;				// Pointer to current in use VINFO

	int64 idxSize;			// The current size of the playing sample in fixed point
	int64 idxLPos;			// The loop start position in fixed point
	int64 idxLEnd;			// The loop end position in fixed point
	int64 idxREnd;			// The release end position in fixed point
};

#endif
