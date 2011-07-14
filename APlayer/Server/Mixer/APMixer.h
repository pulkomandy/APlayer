/******************************************************************************/
/* APMixer header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APMixer_h
#define __APMixer_h

// PolyKit headers
#include "POS.h"
#include "PList.h"
#include "PThread.h"
#include "PSynchronize.h"

// APlayerKit headers
#include "APAddOns.h"
#include "APChannelParser.h"

// Server headers
#include "APApplication.h"
#include "APPlayer.h"
#include "APMixerBase.h"
#include "APMixerVisualize.h"


/******************************************************************************/
/* Other defines                                                              */
/******************************************************************************/
#define MAX_NUM_CHANNELS		64			// Maximum number of channels

#define RINGBUFFER_NUM			16			// Number of ring buffers
#define RINGBUFFER_SIZE			(16 * 1024)	// The size of each ring buffer in samples


typedef struct RingData
{
	PMutex *mutex;			// Mutex to lock the data
	int16 *buffer;			// Pointer to the buffer with sound data
	bool filled;			// True if the buffer is filled with data, else false
	int16 position;			// The song position in this buffer
} RingData;


typedef struct VirtualMixer
{
	AddOnInfo *agent;		// The agent that want the virtual mixer
	APMixerBase *mixer;		// The mixer itself
	APChannelParser **channels;	// The channel objects
	uint16 channelNum;		// The number of channels allocated
	bool available;			// True if the agent is available
} VirtualMixer;



/******************************************************************************/
/* APMixer class                                                              */
/******************************************************************************/
struct APFileHandle;

class APMixer
{
public:
	APMixer(void);
	virtual ~APMixer(void);

	bool InitMixer(APFileHandle handle, PMutex *lock, APPlayer *player, PString &result);
	void EndMixer(void);
	void StartMixer(void);
	void StopMixer(void);

	void PausePlaying(void);
	void ResumePlaying(void);
	void HoldPlaying(bool hold);
	bool UsingRingBuffers(void) const;

	void SetSongPosition(int16 newPos);

	void SetVolume(uint16 volume);
	void SetStereoSeparation(uint16 sep);
	void SetMixerMode(uint32 mode, bool enable);
	void EnableAmigaFilter(bool enable);
	void EnableChannel(uint16 channel, bool enable);

	void DisableVirtualMixer(AddOnInfo *agent);

	static int32 Mixer(void *handle, int16 *buffer, int32 count);

protected:
	int32 DoMixing1(int32 todo);
	void DoMixing2(int16 *buf, int32 todo);

	void AddAmigaFilter(int32 *dest, int32 todo);

	// Virtual mixer functions
	void InitVirtualMixer(void);
	void EndVirtualMixer(void);

	// Ring buffer functions
	void InitRingBuffer(void);
	void EndRingBuffer(void);
	void ResetRingBuffer(void);
	int32 DoRingBuffer(int16 *buf, int32 count);
	static int32 RingBufferFiller(void *userData);
	void SetNewPosition(void);

	// Variables
	PMutex mixerLock;
	PList<VirtualMixer> mixerList;

	bool channelsEnabled[MAX_NUM_CHANNELS];

	bool playing;
	bool holdPlaying;
	bool samplePlay;
	bool emulateFilter;

	PMutex *playerLock;
	APPlayer *playerInfo;

	APAddOnPlayer *currentPlayer;
	APMixerBase *currentMixer;
	APMixerVisualize *currentVisualizer;
	AddOnInfo *soundOutputInfo;
	APAddOnAgent *soundOutput;

	// Ring buffer variables
	bool useRingBuffer;
	bool playLocked;
	bool firstTime;
	bool allMutexesFree;

	int8 playIndex;
	int8 fillIndex;
	int8 endIndex;
	int8 newIndex;

	int32 playPosition;
	int32 endPosition;

	int16 reportCnt;
	int16 reportPos;
	int16 oldPos;
	int16 endSongPosition;

	PThread ringThread;		// The filler thread
	PEvent *exitEvent;		// Will be set when the filler thread has to exit
	PEvent *fillBuffer;		// Will be reset when the module starts over, so the filler thread won't use CPU power at the end
	PEvent *newPosSignal;	// Will be set when the user change the song position
	PEvent *readySignal;	// Will be set by the player thread when it has played a buffer
	RingData ringInfo[RINGBUFFER_NUM];

	// Mixer variables
	uint32 mixerMode;		// Which modes the mixer has to work in
	uint32 curMode;			// Is the current mode the mixer uses
	uint32 mixerFreq;		// The mixer frequency
	uint16 modChannelNum;	// The number of channels the module use

	int32 *mixBuffer;		// The buffer to hold the mixed data before it's converted
	int32 bufferSize;		// The maximum number of samples a buffer can be
	int32 tickLeft;			// Number of ticks left to call the player

	int32 filterPrevLeft;	// The previous value for the left channel
	int32 filterPrevRight;	// The previous value for the right channel
};

#endif
