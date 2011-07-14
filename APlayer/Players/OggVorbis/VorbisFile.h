/******************************************************************************/
/* VorbisFile header file.                                                    */
/******************************************************************************/


#ifndef __VorbisFile_h
#define __VorbisFile_h

#include "POS.h"
#include "PFile.h"

#include "Ogg.h"
#include "Codec.h"


/******************************************************************************/
/* Ready states                                                               */
/******************************************************************************/
#define PARTOPEN			1
#define OPENED				2
#define STREAMSET			3
#define INITSET				4



/******************************************************************************/
/* OggVorbisFile structure                                                    */
/******************************************************************************/
typedef struct OggVorbisFile
{
	PFile *datasource;
	bool seekable;
	int64 offset;
	int64 end;
	OggSyncState oy;

	// If the file handle isn't seekable (eg, a pipe), only the current
	// stream appears
	int32 links;
	int64 *offsets;
	int64 *dataOffsets;
	int32 *serialNos;
	int64 *pcmLengths;		// Overloaded to maintain binary compatability; x2 size, stores both beginning and end values
	VorbisInfo *vi;
	VorbisComment *vc;

	// Decoding working state local storage
	int64 pcmOffset;
	int32 readyState;
	int32 currentSerialNo;
	int32 currentLink;

	double bitTrack;
	double sampTrack;

	OggStreamState os;		// Take physical pages, weld into a logical stream of packets
	VorbisDSPState vd;		// Central working state for the packet->PCM decoder
	VorbisBlock vb;			// Local working space for packet->PCM decode
} OggVorbisFile;

#endif
