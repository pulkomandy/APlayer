/******************************************************************************/
/* APMixerVisualize header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APMixerVisualize_h
#define __APMixerVisualize_h

// PolyKit headers
#include "POS.h"
#include "PSynchronize.h"
#include "PThread.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* APMixerVisualize class                                                     */
/******************************************************************************/
class APMixerVisualize
{
public:
	APMixerVisualize(void);
	virtual ~APMixerVisualize(void);

	bool Initialize(uint16 chanNum, const APChannel **channels, int32 maxBufferLen, bool stereo);
	void Cleanup(void);

	uint32 *GetFlagsArray(void) const;

	void TellAgents_MixedData(int16 *source, int32 size);
	void TellAgents_ChannelChanged(void);

protected:
	static int32 VisualizeThread(void *userData);

	// Structure holding all the channel information
	APAgent_ChannelChange channelInfo;
	PEvent *channelChangedEvent;

	// Buffer holding the sample data to show
	//
	// Note that we don't use locks on the buffer, because
	// we will stop the mixer thread if we do. This can cause
	// holes in the music if the user moves a window around.
	//
	// We just hope the threads are fast enough to show the
	// whole buffer before it is filled again. That's why
	// we use double buffering to minimize the risk
	int16 *buffer[2];
	int32 bufferLen;
	bool useStereo;

	PEvent *bufferFilledEvent;
	int32 currentBuffer;

	// Thread variables
	PEvent *exitEvent;
	PThread thread;
};

#endif
