/******************************************************************************/
/* Ogg header file.                                                           */
/******************************************************************************/


#ifndef __Ogg_h
#define __Ogg_h


typedef struct OggPackBuffer
{
	int32 endByte;
	int32 endBit;

	uint8 *buffer;
	uint8 *ptr;
	int32 storage;
} OggPackBuffer;



// OggPage is used to encapsulate the data in one Ogg bitstream page
typedef struct OggPage
{
	uint8 *header;
	int32 headerLen;
	uint8 *body;
	int32 bodyLen;
} OggPage;



// OggStreamState contains the current encode/decode state of a logical
// Ogg bitstream
typedef struct OggStreamState
{
	uint8 *bodyData;			// Bytes from packet bodies
	int32 bodyStorage;			// Storage elements allocated
	int32 bodyFill;				// Elements stored; fill mark
	int32 bodyReturned;			// Elements of fill returned

	int32 *lacingVals;			// The values that will go to the segment table
	int64 *granuleVals;			// Granulepos values for headers. Not compact this way, but it is
								// simple coupled to the lacing fifo
	int32 lacingStorage;
	int32 lacingFill;
	int32 lacingPacket;
	int32 lacingReturned;

	uint8 header[282];			// Working space for header encode
	int32 headerFill;

	bool e_o_s;					// Set when we have buffered the last packet in the logical bitstream
	bool b_o_s;					// Set after we've written the initial page of a logical bitstream
	int32 serialNo;
	int32 pageNo;
	int64 packetNo;				// Sequence number for decode; the framing knowns where
								// there's a hole in the data, but we need coupling so
								// that the codec (which is in a seperate abstraction layer)
								// also knows about the gap
	int64 granulePos;
} OggStreamState;



// OggPacket is used to encapsulate the data and metadata belonging
// to a single raw Ogg/Vorbis packet
typedef struct OggPacket
{
	uint8 *packet;
	int32 bytes;
	bool b_o_s;
	bool e_o_s;

	int64 granulePos;

	int64 packetNo;				// Sequence number for decode; the framing
								// knows where there's a hole in the data,
								// but we need coupling so that the codec
								// (which is in a separate abstraction layer)
								// also knows about the gap
} OggPacket;



typedef struct OggSyncState
{
	uint8 *data;
	int32 storage;
	int32 fill;
	int32 returned;

	bool unsynced;
	int32 headerBytes;
	int32 bodyBytes;
} OggSyncState;

#endif
