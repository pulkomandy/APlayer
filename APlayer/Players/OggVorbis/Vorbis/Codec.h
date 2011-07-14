/******************************************************************************/
/* Codec header file.                                                         */
/******************************************************************************/


#ifndef __Codec_h
#define __Codec_h

#include "Codec_Internal.h"


typedef struct VorbisInfo
{
	int32 version;
	int32 channels;
	int32 rate;

	// The below bitrate declarations are *hints*.
	// Combinations of the three values carry the following implications:
	//
	// All three set to the same value:
	//   Implies a fixed rate bitstream
	//
	// Only nominal set:
	//   Implies a VBR stream that averages the nominal bitrate. No hard
	//   upper/lower limit
	//
	// Upper and/or lower set:
	//   Implies a VBR bitstream that obeys the bitrate limits. Nominal
	//   may also be set to give a normal rate
	//
	// None set:
	//   The coder does not care to speculate
	int32 bitrateUpper;
	int32 bitrateNominal;
	int32 bitrateLower;

	void *codecSetup;
} VorbisInfo;



// VorbisDSPState buffers the current vorbis audio
// analysis/synthesis state. The DSP state belongs to a specific
// logical bitstream
typedef struct VorbisDSPState
{
	VorbisInfo *vi;

	float **pcm;
	float **pcmRet;
	int32 pcmStorage;
	int32 pcmCurrent;
	int32 pcmReturned;

	bool eofFlag;

	int32 lW;
	int32 w;
	int32 nW;
	int32 centerW;

	int64 granulePos;
	int64 sequence;

	int64 glueBits;
	int64 timeBits;
	int64 floorBits;
	int64 resBits;

	void *backendState;
} VorbisDSPState;



// VorbisBlock is a single block of data to be processed as part of
// the analysis/synthesis stream; it belongs to a specific logical
// bitstream, but is independant from other VorbisBlocks to belonging to
// that logical bitstream
typedef struct AllocChain
{
	void *ptr;
	AllocChain *next;
} AllocChain;



typedef struct VorbisBlock
{
	// Necessary stream state for linking to the framing abstraction
	float **pcm;				// This is a pointer into local storage
	OggPackBuffer opb;

	int32 lW;
	int32 w;
	int32 nW;
	int32 pcmEnd;
	int32 mode;

	bool eofFlag;
	int64 granulePos;
	int64 sequence;
	VorbisDSPState *vd;			// For read-only access of configuration

	void *localStore;
	int32 localTop;
	int32 localAlloc;
	int32 totalUse;
	AllocChain *reap;

	// Bitmetrics for the frame
	int32 glueBits;
	int32 timeBits;
	int32 floorBits;
	int32 resBits;
} VorbisBlock;



// The comments are not part of VorbisInfo so that VorbisInfo can be
// static storage
typedef struct VorbisComment
{
	// Unlimited user comment fields
	char **userComments;
	int32 *commentLengths;
	int32 comments;
	char *vendor;
} VorbisComment;



// Vorbis ERRORS and return codes
#define OV_FALSE				-1
#define OV_EOF					-2
#define OV_HOLE					-3

#define OV_EREAD				-128
#define OV_EFAULT				-129
#define OV_EINVAL				-131
#define OV_ENOTVORBIS			-132
#define OV_EBADHEADER			-133
#define OV_EVERSION				-134
#define OV_ENOTAUDIO			-135
#define OV_EBADPACKET			-136
#define OV_EBADLINK				-137
#define OV_ENOSEEK				-138

#endif
