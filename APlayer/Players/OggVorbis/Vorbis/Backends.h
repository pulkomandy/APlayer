/******************************************************************************/
/* Backends header file.                                                      */
/******************************************************************************/


#ifndef __Backends_h
#define __Backends_h

#include "POS.h"
#include "Ogg.h"
#include "Codec.h"
#include "Codec_Internal.h"
#include "OggVorbis.h"


// Floor backend generic
typedef struct Vorbis_Func_Floor
{
	VorbisInfoFloor *(*Unpack)(OggVorbis *, VorbisInfo *, OggPackBuffer *);
	VorbisLookFloor *(*Look)(VorbisDSPState *, VorbisInfoFloor *);
	void (*FreeInfo)(VorbisInfoFloor *);
	void (*FreeLook)(VorbisLookFloor *);
	void *(*Inverse1)(OggVorbis *, VorbisBlock *, VorbisLookFloor *);
	int32 (*Inverse2)(OggVorbis *, VorbisBlock *, VorbisLookFloor *, void *, float *);
} Vorbis_Func_Floor;



typedef struct VorbisInfoFloor0
{
	int32 order;
	int32 rate;
	int32 barkMap;

	int32 ampBits;
	int32 ampdB;

	int32 numBooks;							// <= 16
	int32 books[16];
} VorbisInfoFloor0;



#define VIF_POSIT	63
#define VIF_CLASS	16
#define VIF_PARTS	31



typedef struct VorbisInfoFloor1
{
	int32 partitions;						// 0 to 31
	int32 partitionClass[VIF_PARTS];		// 0 to 15

	int32 classDim[VIF_CLASS];				// 1 to 8
	int32 classSubs[VIF_CLASS];				// 0,1,2,3 (bits: 1 << n poss)
	int32 classBook[VIF_CLASS];				// subs ^ dim entries
	int32 classSubBook[VIF_CLASS][8];		// [VIF_CLASS][subs]

	int32 mult;								// 1 2 3 or 4
	int32 postList[VIF_POSIT + 2];			// First two implicit
} VorbisInfoFloor1;



// Residue backend generic
typedef struct Vorbis_Func_Residue
{
	VorbisInfoResidue *(*Unpack)(OggVorbis *, VorbisInfo *, OggPackBuffer *);
	VorbisLookResidue *(*Look)(VorbisDSPState *, VorbisInfoResidue *);
	void (*FreeInfo)(VorbisInfoResidue *);
	void (*FreeLook)(VorbisLookResidue *);
	int32 (*Inverse)(OggVorbis *, VorbisBlock *, VorbisLookResidue *, float **, bool *, int32);
} Vorbis_Func_Residue;



typedef struct VorbisInfoResidue0
{
	// Block-partitioned VQ coded straight residue
	int32 begin;
	int32 end;

	// First stage (lossless partitioning)
	int32 grouping;							// Group n vectors per partition
	int32 partitions;						// Possible codebooks for a partition
	int32 groupBook;						// Huffbook for partitioning
	int32 secondStages[64];					// Expanded out to pointers in lookup
	int32 bookList[256];					// List of second stage books
} VorbisInfoResidue0;



// Mapping backend generic
typedef struct Vorbis_Func_Mapping
{
	VorbisInfoMapping *(*Unpack)(OggVorbis *, VorbisInfo *, OggPackBuffer *);
	void (*FreeInfo)(VorbisInfoMapping *);
	int32 (*Inverse)(OggVorbis *, VorbisBlock *, VorbisInfoMapping *);
} Vorbis_Func_Mapping;



typedef struct VorbisInfoMapping0
{
	int32 subMaps;							// <= 16
	int32 chMuxList[256];					// Up to 256 channels in a Vorbis stream

	int32 floorSubMap[16];					// [mux] submap to floors
	int32 residueSubMap[16];				// [mux] submap to residue

	int32 couplingSteps;
	int32 couplingMag[256];
	int32 couplingAng[256];
} VorbisInfoMapping0;

#endif
