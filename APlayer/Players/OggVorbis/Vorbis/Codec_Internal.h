/******************************************************************************/
/* Codec_Internal header file.                                                */
/******************************************************************************/


#ifndef __Codec_Internal_h
#define __Codec_Internal_h

#include "POS.h"
#include "Codebook.h"


typedef void VorbisLookFloor;
typedef void VorbisLookResidue;
typedef void VorbisLookTransform;


// Mode
typedef struct VorbisInfoMode
{
	int32 blockFlag;
	int32 windowType;
	int32 transformType;
	int32 mapping;
} VorbisInfoMode;



typedef void VorbisInfoFloor;
typedef void VorbisInfoResidue;
typedef void VorbisInfoMapping;



typedef struct BackendLookupState
{
	// Local lookup storage
	float *window[2];				// Block, leadin, leadout, type
	VorbisLookTransform **transform[2];		// Block, type

	int32 modeBits;
	VorbisLookFloor **flr;
	VorbisLookResidue **residue;
} BackendLookupState;



// CodecSetupInfo contains all the setup information specific to the
// specific compression/decompression mode in progress (eg,
// psychoacoustic settings, channel setup, options, codebook etc.)
typedef struct CodecSetupInfo
{
	// Vorbis supports only short and long blocks, but allows the
	// encoder to choose the sizes
	int32 blockSizes[2];

	// Modes are the primary means of supporting on-the-fly different
	// blocksizes, different channel mappings (LR or M/A),
	// different residue backends, etc. Each mode consists of a
	// blocksize flag and a mapping (along with the mapping setup)
	int32 modes;
	int32 maps;
	int32 floors;
	int32 residues;
	int32 books;

	VorbisInfoMode *modeParam[64];
	int32 mapType[64];
	VorbisInfoMapping *mapParam[64];
	int32 floorType[64];
	VorbisInfoFloor *floorParam[64];
	int32 residueType[64];
	VorbisInfoResidue *residueParam[64];
	StaticCodebook *bookParam[256];
	Codebook *fullBooks;
} CodecSetupInfo;

#endif
