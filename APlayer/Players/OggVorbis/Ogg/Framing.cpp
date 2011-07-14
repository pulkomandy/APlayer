/******************************************************************************/
/* Ogg frame functions.                                                       */
/******************************************************************************/


// Player headers
#include "OggVorbis.h"


/******************************************************************************/
/* CRC lookup table                                                           */
/******************************************************************************/
static uint32 crcLookup[256] =
{
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
	0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
	0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
	0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
	0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
	0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
	0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
	0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
	0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
	0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
	0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
	0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
	0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
	0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
	0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
	0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
	0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
	0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
	0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
	0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
	0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
	0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
	0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};



/******************************************************************************/
/* Ogg_Page_Version() returns the version for the current page.               */
/*                                                                            */
/* Input:  "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: The version.                                                       */
/******************************************************************************/
int32 OggVorbis::Ogg_Page_Version(OggPage *og)
{
	return (og->header[4]);
}



/******************************************************************************/
/* Ogg_Page_Continued() checks to see if the current page is a continued one  */
/*      or not.                                                               */
/*                                                                            */
/* Input:  "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: True if the page is continued, false if not.                       */
/******************************************************************************/
bool OggVorbis::Ogg_Page_Continued(OggPage *og)
{
	return (og->header[5] & 0x01);
}



/******************************************************************************/
/* Ogg_Page_BOS() checks to see if the current page is the first page.        */
/*                                                                            */
/* Input:  "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: True if the page is the first one, false if not.                   */
/******************************************************************************/
bool OggVorbis::Ogg_Page_BOS(OggPage *og)
{
	return (og->header[5] & 0x02);
}



/******************************************************************************/
/* Ogg_Page_EOS() checks to see if the current page is the last page.         */
/*                                                                            */
/* Input:  "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: True if the page is the last one, false if not.                    */
/******************************************************************************/
bool OggVorbis::Ogg_Page_EOS(OggPage *og)
{
	return (og->header[5] & 0x04);
}



/******************************************************************************/
/* Ogg_Page_GranulePos() returns the granule position of the current page.    */
/*                                                                            */
/* Input:  "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: The granule position.                                              */
/******************************************************************************/
int64 OggVorbis::Ogg_Page_GranulePos(OggPage *og)
{
	uint8 *page = og->header;
	int64 granulePos = page[13] & 0xff;
	granulePos = (granulePos << 8) | (page[12] & 0xff);
	granulePos = (granulePos << 8) | (page[11] & 0xff);
	granulePos = (granulePos << 8) | (page[10] & 0xff);
	granulePos = (granulePos << 8) | (page[9] & 0xff);
	granulePos = (granulePos << 8) | (page[8] & 0xff);
	granulePos = (granulePos << 8) | (page[7] & 0xff);
	granulePos = (granulePos << 8) | (page[6] & 0xff);

	return (granulePos);
}



/******************************************************************************/
/* Ogg_Page_SerialNo() returns the serial number for the current page.        */
/*                                                                            */
/* Input:  "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: The serial number.                                                 */
/******************************************************************************/
int32 OggVorbis::Ogg_Page_SerialNo(OggPage *og)
{
	return (og->header[14] | (og->header[15] << 8) | (og->header[16] << 16) | (og->header[17] << 24));
}



/******************************************************************************/
/* Ogg_Page_PageNo() returns the page number for the current page.            */
/*                                                                            */
/* Input:  "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: The page number.                                                   */
/******************************************************************************/
int32 OggVorbis::Ogg_Page_PageNo(OggPage *og)
{
	return (og->header[18] | (og->header[19] << 8) | (og->header[20] << 16) | (og->header[21] << 24));
}



/******************************************************************************/
/* Ogg_Page_ChecksumSet() checksum the page.                                  */
/*                                                                            */
/* Direct table CRC; Note that this will be faster in the future if we        */
/* perform the checksum silmultaneously with other copies.                    */
/*                                                                            */
/* Input:  "og" is a pointer to the page structure.                           */
/******************************************************************************/
void OggVorbis::Ogg_Page_ChecksumSet(OggPage *og)
{
	if (og)
	{
		uint32 crcReg = 0;
		int32 i;

		// Safety; needed for API behavior, but not framing code
		og->header[22] = 0;
		og->header[23] = 0;
		og->header[24] = 0;
		og->header[25] = 0;

		for (i = 0; i < og->headerLen; i++)
			crcReg = (crcReg << 8) ^ crcLookup[((crcReg >> 24) & 0xff) ^ og->header[i]];

		for (i = 0; i < og->bodyLen; i++)
			crcReg = (crcReg << 8) ^ crcLookup[((crcReg >> 24) & 0xff) ^ og->body[i]];

		og->header[22] = crcReg & 0xff;
		og->header[23] = (crcReg >> 8) & 0xff;
		og->header[24] = (crcReg >> 16) & 0xff;
		og->header[25] = (crcReg >> 24) & 0xff;
	}
}



/******************************************************************************/
/* Ogg_Stream_Init() initialize the stream state structure to a known state.  */
/*                                                                            */
/* Input:  "os" is a pointer to the structure to initialize.                  */
/*         "serialNo" is the serial number to use.                            */
/*                                                                            */
/* Output: 0 for successful, -1 for failure.                                  */
/******************************************************************************/
int32 OggVorbis::Ogg_Stream_Init(OggStreamState *os, int32 serialNo)
{
	if (os)
	{
		memset(os, 0, sizeof(*os));

		os->bodyStorage   = 16 * 1024;
		os->bodyData      = (uint8 *)_ogg_malloc(os->bodyStorage * sizeof(*os->bodyData));

		os->lacingStorage = 1024;
		os->lacingVals    = (int32 *)_ogg_malloc(os->lacingStorage * sizeof(*os->lacingVals));
		os->granuleVals   = (int64 *)_ogg_malloc(os->lacingStorage * sizeof(*os->granuleVals));

		os->serialNo      = serialNo;

		return (0);
	}

	return (-1);
}



/******************************************************************************/
/* Ogg_Stream_Clear() clear non-flat storage within.                          */
/*                                                                            */
/* Input:  "os" is a pointer to the structure to cleanup.                     */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 OggVorbis::Ogg_Stream_Clear(OggStreamState *os)
{
	if (os)
	{
		_ogg_free(os->bodyData);
		_ogg_free(os->lacingVals);
		_ogg_free(os->granuleVals);

		memset(os, 0, sizeof(*os));
	}

	return (0);
}



/******************************************************************************/
/* Ogg_Stream_Reset() clear things to an initial state.                       */
/*                                                                            */
/* Input:  "os" is a pointer to the structure to reset.                       */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 OggVorbis::Ogg_Stream_Reset(OggStreamState *os)
{
	os->bodyFill       = 0;
	os->bodyReturned   = 0;

	os->lacingFill     = 0;
	os->lacingPacket   = 0;
	os->lacingReturned = 0;

	os->headerFill     = 0;

	os->e_o_s          = false;
	os->b_o_s          = false;
	os->pageNo         = -1;
	os->packetNo       = 0;
	os->granulePos     = 0;

	return (0);
}



/******************************************************************************/
/* Ogg_Stream_PageIn() add the incoming page to the stream state; we          */
/*      decompose the page into packet segments here as well.                 */
/*                                                                            */
/* Input:  "os" is a pointer to the state structure.                          */
/*         "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: 0 for success, -1 for failure.                                     */
/******************************************************************************/
int32 OggVorbis::Ogg_Stream_PageIn(OggStreamState *os, OggPage *og)
{
	uint8 *header  = og->header;
	uint8 *body    = og->body;
	int32 bodySize = og->bodyLen;
	int32 segPtr   = 0;

	int32 version    = Ogg_Page_Version(og);
	bool continued   = Ogg_Page_Continued(og);
	bool bos         = Ogg_Page_BOS(og);
	bool eos         = Ogg_Page_EOS(og);
	int64 granulePos = Ogg_Page_GranulePos(og);
	int32 serialNo   = Ogg_Page_SerialNo(og);
	int32 pageNo     = Ogg_Page_PageNo(og);
	int32 segments   = header[26];

	// Cleanup 'returned data'
	{
		int32 lr = os->lacingReturned;
		int32 br = os->bodyReturned;

		// Body data
		if (br)
		{
			os->bodyFill -= br;
			if (os->bodyFill)
				memmove(os->bodyData, os->bodyData + br, os->bodyFill);

			os->bodyReturned = 0;
		}

		if (lr)
		{
			// Segment table
			if (os->lacingFill - lr)
			{
				memmove(os->lacingVals, os->lacingVals + lr, (os->lacingFill - lr) * sizeof(*os->lacingVals));
				memmove(os->granuleVals, os->granuleVals + lr, (os->lacingFill - lr) * sizeof(*os->granuleVals));
			}

			os->lacingFill    -= lr;
			os->lacingPacket  -= lr;
			os->lacingReturned = 0;
		}
	}

	// Check the serial number
	if (serialNo != os->serialNo)
		return (-1);

	if (version > 0)
		return (-1);

	_os_LacingExpand(os, segments + 1);

	// Are we in sequence?
	if (pageNo != os->pageNo)
	{
		int32 i;

		// Unroll previous partial packet (if any)
		for (i = os->lacingPacket; i < os->lacingFill; i++)
			os->bodyFill -= os->lacingVals[i] & 0xff;

		os->lacingFill = os->lacingPacket;

		// Make a note of dropped data in segment table
		if (os->pageNo != -1)
		{
			os->lacingVals[os->lacingFill++] = 0x400;
			os->lacingPacket++;
		}

		// Are we a 'continued packet' page? If so, we'll need to skip
		// some segments
		if (continued)
		{
			bos = false;
			for (; segPtr < segments; segPtr++)
			{
				int32 val = header[27 + segPtr];
				body     += val;
				bodySize -= val;

				if (val < 255)
				{
					segPtr++;
					break;
				}
			}
		}
	}

	if (bodySize)
	{
		_os_BodyExpand(os, bodySize);
		memcpy(os->bodyData + os->bodyFill, body, bodySize);
		os->bodyFill += bodySize;
	}

	{
		int32 saved = -1;

		while (segPtr < segments)
		{
			int32 val = header[27 + segPtr];

			os->lacingVals[os->lacingFill]  = val;
			os->granuleVals[os->lacingFill] = -1;

			if (bos)
			{
				os->lacingVals[os->lacingFill] |= 0x100;
				bos = false;
			}

			if (val < 255)
				saved = os->lacingFill;

			os->lacingFill++;
			segPtr++;

			if (val < 255)
				os->lacingPacket = os->lacingFill;
		}

		// Set the granulePos on the last granuleVal of the last full packet
		if (saved != -1)
			os->granuleVals[saved] = granulePos;
	}

	if (eos)
	{
		os->e_o_s = true;

		if (os->lacingFill > 0)
			os->lacingVals[os->lacingFill - 1] |= 0x200;
	}

	os->pageNo = pageNo + 1;

	return (0);
}



/******************************************************************************/
/* Ogg_Stream_PacketOut() outputs a packet to the codec-specific decoding     */
/*      engine.                                                               */
/*                                                                            */
/* Input:  "os" is a pointer to the state structure.                          */
/*         "op" is a pointer to the packet structure.                         */
/*                                                                            */
/* Output: -1 if we are out of sync, 1 in all other cases.                    */
/******************************************************************************/
int32 OggVorbis::Ogg_Stream_PacketOut(OggStreamState *os, OggPacket *op)
{
	return (_PacketOut(os, op, true));
}



/******************************************************************************/
/* Ogg_Stream_PacketPeek() look after a packet, but does not return it.       */
/*                                                                            */
/* Input:  "os" is a pointer to the state structure.                          */
/*         "op" is a pointer to the packet structure.                         */
/*                                                                            */
/* Output: -1 if we are out of sync, 1 in all other cases.                    */
/******************************************************************************/
int32 OggVorbis::Ogg_Stream_PacketPeek(OggStreamState *os, OggPacket *op)
{
	return (_PacketOut(os, op, false));
}



/******************************************************************************/
/* Ogg_Sync_Init() initialize the sync state structure to a known state.      */
/*                                                                            */
/* Input:  "oy" is a pointer to the structure to initialize.                  */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 OggVorbis::Ogg_Sync_Init(OggSyncState *oy)
{
	if (oy)
		memset(oy, 0, sizeof(*oy));

	return (0);
}



/******************************************************************************/
/* Ogg_Sync_Clear() clear non-flat storage within.                            */
/*                                                                            */
/* Input:  "oy" is a pointer to the structure to cleanup.                     */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 OggVorbis::Ogg_Sync_Clear(OggSyncState *oy)
{
	if (oy)
	{
		_ogg_free(oy->data);

		Ogg_Sync_Init(oy);
	}

	return (0);
}



/******************************************************************************/
/* Ogg_Sync_Reset() clear things to an initial state.                         */
/*                                                                            */
/* Input:  "oy" is a pointer to the structure to reset.                       */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 OggVorbis::Ogg_Sync_Reset(OggSyncState *oy)
{
	oy->fill        = 0;
	oy->returned    = 0;
	oy->unsynced    = false;
	oy->headerBytes = 0;
	oy->bodyBytes   = 0;

	return (0);
}



/******************************************************************************/
/* Ogg_Sync_Buffer() allocates a properly-sized buffer.                       */
/*                                                                            */
/* Input:  "oy" is a pointer to the synchronize structure.                    */
/*         "size" is the minimum size of the buffer. Additional bytes will be */
/*         allocated.                                                         */
/*                                                                            */
/* Output: A pointer to the allocated buffer.                                 */
/******************************************************************************/
int8 *OggVorbis::Ogg_Sync_Buffer(OggSyncState *oy, int32 size)
{
	// First, clear out any space that has been previously returned
	if (oy->returned)
	{
		oy->fill -= oy->returned;

		if (oy->fill > 0)
			memmove(oy->data, oy->data + oy->returned, oy->fill);

		oy->returned = 0;
	}

	if (size > (oy->storage - oy->fill))
	{
		// We need to extend the internal buffer
		int32 newSize = size + oy->fill + 4096;		// An extra page to be nice

		if (oy->data)
			oy->data = (uint8 *)_ogg_realloc(oy->data, newSize);
		else
			oy->data = (uint8 *)_ogg_malloc(newSize);

		oy->storage = newSize;
	}

	// Expose a segment at least as large as requested at the fill mark
	return ((int8 *)oy->data + oy->fill);
}



/******************************************************************************/
/* Ogg_Sync_Wrote() tells the synchronize structure how many bytes you have   */
/*      wrote into the allocated buffer.                                      */
/*                                                                            */
/* Input:  "oy" is a pointer to the synchronize structure.                    */
/*         "bytes" is the number of bytes you have wrote.                     */
/*                                                                            */
/* Output: -1 if the number of bytes written overflows the internal storage   */
/*         of the structure.                                                  */
/*         0 in all other cases.                                              */
/******************************************************************************/
int32 OggVorbis::Ogg_Sync_Wrote(OggSyncState *oy, int32 bytes)
{
	if ((oy->fill + bytes) > oy->storage)
		return (-1);

	oy->fill += bytes;

	return (0);
}



/******************************************************************************/
/* Ogg_Sync_PageSeek() synchronize the sync structure to the next page.       */
/*                                                                            */
/* Input:  "oy" is a pointer to the synchronize structure.                    */
/*         "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: -n Skipped n bytes.                                                */
/*         0 Page not ready; more data (no bytes skipped).                    */
/*         n Page synced at current location; page length n bytes.            */
/******************************************************************************/
int32 OggVorbis::Ogg_Sync_PageSeek(OggSyncState *oy, OggPage *og)
{
	uint8 *page = oy->data + oy->returned;
	uint8 *next;
	int32 bytes = oy->fill - oy->returned;

	if (oy->headerBytes == 0)
	{
		int32 headerBytes, i;

		if (bytes < 27)
			return (0);		// Not enough for a header

		// Verify capture pattern
		if (memcmp(page, "OggS", 4))
			goto sync_fail;

		headerBytes = page[26] + 27;
		if (bytes < headerBytes)
			return (0);		// Not enough for header + seg table

		// Count up body length in the segment table
		for (i = 0; i < page[26]; i++)
			oy->bodyBytes += page[27 + i];

		oy->headerBytes = headerBytes;
	}

	if ((oy->bodyBytes + oy->headerBytes) > bytes)
		return (0);

	// The whole test page is buffered. Verify the checksum
	{
		// Grab the checksum bytes, set the header field to zero
		int8 chksum[4];
		OggPage log;

		memcpy(chksum, page + 22, 4);
		memset(page + 22, 0, 4);

		// Set up a temp page struct and recompute the checksum
		log.header    = page;
		log.headerLen = oy->headerBytes;
		log.body      = page + oy->headerBytes;
		log.bodyLen   = oy->bodyBytes;
		Ogg_Page_ChecksumSet(&log);

		// Compare
		if (memcmp(chksum, page + 22, 4))
		{
			// D'oh. Mismatch! Corrupt page (or miscapture and not a page at all)
			// Replace the computed checksum with the one actually read in
			memcpy(page + 22, chksum, 4);

			// Bad checksum. Lose sync
			goto sync_fail;
		}
	}

	// Yes, have a whole page all ready to go
	{
		uint8 *page = oy->data + oy->returned;
		int32 bytes;

		if (og)
		{
			og->header    = page;
			og->headerLen = oy->headerBytes;
			og->body      = page + oy->headerBytes;
			og->bodyLen   = oy->bodyBytes;
		}

		oy->unsynced    = false;
		oy->returned   += (bytes = oy->headerBytes + oy->bodyBytes);
		oy->headerBytes = 0;
		oy->bodyBytes   = 0;

		return (bytes);
	}

sync_fail:
	oy->headerBytes = 0;
	oy->bodyBytes   = 0;

	// Search for possible capture
	next = (uint8 *)memchr(page + 1, 'O', bytes - 1);
	if (!next)
		next = oy->data + oy->fill;

	oy->returned = next - oy->data;
	return (-(next - page));
}



/******************************************************************************/
/* Ogg_Sync_PageOut() synchronize the stream and get a page.                  */
/*                                                                            */
/* Input:  "oy" is a pointer to the synchronize structure.                    */
/*         "og" is a pointer to the page structure.                           */
/*                                                                            */
/* Output: -1 Recapture (hole in data).                                       */
/*         0 Need more data.                                                  */
/*         1 Page returned.                                                   */
/******************************************************************************/
int32 OggVorbis::Ogg_Sync_PageOut(OggSyncState *oy, OggPage *og)
{
	// All we need to do is verify a page at the head of the stream
	// buffer. If it doesn't verify, we look for the next potential
	// frame
	while (true)
	{
		int32 ret = Ogg_Sync_PageSeek(oy, og);
		if (ret > 0)
		{
			// Have a page
			return (1);
		}

		if (ret == 0)
		{
			// Need more data
			return (0);
		}

		// Head did not start a synced page... skipped some bytes
		if (!oy->unsynced)
		{
			oy->unsynced = true;
			return (-1);
		}

		// Loop. Keep looking
	}
}



/******************************************************************************/
/* _os_BodyExpand() expand the body buffer.                                   */
/*                                                                            */
/* Input:  "os" is a pointer to the stream state structure.                   */
/*         "needed" is the minimum number of elements needed.                 */
/******************************************************************************/
void OggVorbis::_os_BodyExpand(OggStreamState *os, int32 needed)
{
	if (os->bodyStorage <= (os->bodyFill + needed))
	{
		os->bodyStorage += (needed + 1024);
		os->bodyData     = (uint8 *)_ogg_realloc(os->bodyData, os->bodyStorage * sizeof(*os->bodyData));
	}
}



/******************************************************************************/
/* _os_LacingExpand() expand the lacing buffer.                               */
/*                                                                            */
/* Input:  "os" is a pointer to the stream state structure.                   */
/*         "needed" is the minimum number of elements needed.                 */
/******************************************************************************/
void OggVorbis::_os_LacingExpand(OggStreamState *os, int32 needed)
{
	if (os->lacingStorage <= (os->lacingFill + needed))
	{
		os->lacingStorage += (needed + 32);
		os->lacingVals     = (int32 *)_ogg_realloc(os->lacingVals, os->lacingStorage * sizeof(*os->lacingVals));
		os->granuleVals    = (int64 *)_ogg_realloc(os->granuleVals, os->lacingStorage * sizeof(*os->granuleVals));
	}
}



/******************************************************************************/
/* _PacketOut()                                                               */
/*                                                                            */
/* Input:  "os" is a pointer to the state structure.                          */
/*         "op" is a pointer to the packet structure.                         */
/*         "adv" advanced decoding on and off.                                */
/*                                                                            */
/* Output: -1 if we are out of sync, 1 in all other cases.                    */
/******************************************************************************/
int32 OggVorbis::_PacketOut(OggStreamState *os, OggPacket *op, bool adv)
{
	// The last part of decode. We have the stream broken into packet
	// segments. Now we need to group them into packets (or return the
	// out of sync markers)
	int32 ptr = os->lacingReturned;

	if (os->lacingPacket <= ptr)
		return (0);

	if (os->lacingVals[ptr] & 0x400)
	{
		// We need to tell the codec there's a gap; it might need to
		// handle previous packet dependencies
		os->lacingReturned++;
		os->packetNo++;
		return (-1);
	}

	// Just using peek as an inexpensive way to ask if there's a whole packet waiting
	if (!op && !adv)
		return (1);

	// Gather the whole packet. We'll have no holes or a partial packet
	{
		int32 size = os->lacingVals[ptr] & 0xff;
		int32 bytes = size;
		bool eos = os->lacingVals[ptr] & 0x200;		// Last packet of the stream?
		bool bos = os->lacingVals[ptr] & 0x100;		// First packet of the stream?

		while (size == 255)
		{
			int32 val = os->lacingVals[++ptr];
			size = val & 0xff;

			if (val & 0x200)
				eos = true;

			bytes += size;
		}

		if (op)
		{
			op->e_o_s      = eos;
			op->b_o_s      = bos;
			op->packet     = os->bodyData + os->bodyReturned;
			op->packetNo   = os->packetNo;
			op->granulePos = os->granuleVals[ptr];
			op->bytes      = bytes;
		}

		if (adv)
		{
			os->bodyReturned  += bytes;
			os->lacingReturned = ptr + 1;
			os->packetNo++;
		}
	}

	return (1);
}
