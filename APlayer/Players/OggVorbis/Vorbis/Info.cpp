/******************************************************************************/
/* Vorbis info functions.                                                     */
/******************************************************************************/


// Player headers
#include "Registry.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Vorbis_Comment_Init() initialize the comment structure to a known state.   */
/*                                                                            */
/* Input:  "vc" is a pointer to the structure to initialize.                  */
/******************************************************************************/
void OggVorbis::Vorbis_Comment_Init(VorbisComment *vc)
{
	memset(vc, 0, sizeof(*vc));
}



/******************************************************************************/
/* Vorbis_Comment_Clear() clear non-flat storage within.                      */
/*                                                                            */
/* Input:  "vc" is a pointer to the structure to cleanup.                     */
/******************************************************************************/
void OggVorbis::Vorbis_Comment_Clear(VorbisComment *vc)
{
	if (vc)
	{
		int32 i;

		for (i = 0; i < vc->comments; i++)
			_ogg_free(vc->userComments[i]);

		_ogg_free(vc->userComments);
		_ogg_free(vc->commentLengths);
		_ogg_free(vc->vendor);

		memset(vc, 0, sizeof(*vc));
	}
}



/******************************************************************************/
/* Vorbis_Info_Init() initialize the information structure to a known state.  */
/*                                                                            */
/* Input:  "vi" is a pointer to the structure to initialize.                  */
/******************************************************************************/
void OggVorbis::Vorbis_Info_Init(VorbisInfo *vi)
{
	memset(vi, 0, sizeof(*vi));
	vi->codecSetup = _ogg_calloc(1, sizeof(CodecSetupInfo));
}



/******************************************************************************/
/* Vorbis_Info_Clear() clear non-flat storage within.                         */
/*                                                                            */
/* Input:  "vi" is a pointer to the structure to cleanup.                     */
/******************************************************************************/
void OggVorbis::Vorbis_Info_Clear(VorbisInfo *vi)
{
	if (vi != NULL)
	{
		CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;
		int32 i;

		if (ci)
		{
			for (i = 0; i < ci->modes; i++)
				_ogg_free(ci->modeParam[i]);

			for (i = 0; i < ci->maps; i++)		// Unpack does the range checking
				mappingP[ci->mapType[i]]->FreeInfo(ci->mapParam[i]);

			for (i = 0; i < ci->floors; i++)	// Unpack does the range checking
				floorP[ci->floorType[i]]->FreeInfo(ci->floorParam[i]);

			for (i = 0; i < ci->residues; i++)	// Unpack does the range checking
				residueP[ci->residueType[i]]->FreeInfo(ci->residueParam[i]);

			for (i = 0; i < ci->books; i++)
			{
				if (ci->bookParam[i])
				{
					// Knows if the book was not allocated
					Vorbis_Staticbook_Destroy(ci->bookParam[i]);
				}

				if (ci->fullBooks)
					Vorbis_Book_Clear(ci->fullBooks + i);
			}

			_ogg_free(ci->fullBooks);
			_ogg_free(ci);
		}

		memset(vi, 0, sizeof(*vi));
	}
}



/******************************************************************************/
/* Vorbis_Info_BlockSize() returns the size of the block number given.        */
/*                                                                            */
/* Input:  "vi" is a pointer to the information structure.                    */
/*         "zo" is the block number (0-1).                                    */
/*                                                                            */
/* Output: The block size.                                                    */
/******************************************************************************/
int32 OggVorbis::Vorbis_Info_BlockSize(VorbisInfo *vi, int32 zo)
{
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;

	return (ci ? ci->blockSizes[zo] : -1);
}



/******************************************************************************/
/* Vorbis_Unpack_Info() unpacks the header information.                       */
/*                                                                            */
/* Input:  "vi" is a pointer to the Vorbis information structure.             */
/*         "opb" is a pointer to the data buffer.                             */
/*                                                                            */
/* Output: 0 for success, else an error code.                                 */
/******************************************************************************/
int32 OggVorbis::Vorbis_Unpack_Info(VorbisInfo *vi, OggPackBuffer *opb)
{
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;

	if (!ci)
		return (OV_EFAULT);

	vi->version = OggPack_Read(opb, 32);
	if (vi->version != 0)
		return (OV_EVERSION);

	vi->channels       = OggPack_Read(opb, 8);
	vi->rate           = OggPack_Read(opb, 32);

	vi->bitrateUpper   = OggPack_Read(opb, 32);
	vi->bitrateNominal = OggPack_Read(opb, 32);
	vi->bitrateLower   = OggPack_Read(opb, 32);

	ci->blockSizes[0]  = 1 << OggPack_Read(opb, 4);
	ci->blockSizes[1]  = 1 << OggPack_Read(opb, 4);

	if (vi->rate < 1)
		goto err_out;

	if (vi->channels < 1)
		goto err_out;

	if (ci->blockSizes[0] < 8)
		goto err_out;

	if (ci->blockSizes[1] < ci->blockSizes[0])
		goto err_out;

	if (OggPack_Read(opb, 1) != 1)
		goto err_out;		// EOP check

	return (0);

err_out:
	Vorbis_Info_Clear(vi);
	return (OV_EBADHEADER);
}



/******************************************************************************/
/* Vorbis_Unpack_Comment() unpacks the comment information.                   */
/*                                                                            */
/* Input:  "vi" is a pointer to the comment structure.                        */
/*         "opb" is a pointer to the data buffer.                             */
/*                                                                            */
/* Output: 0 for success, else an error code.                                 */
/******************************************************************************/
int32 OggVorbis::Vorbis_Unpack_Comment(VorbisComment *vc, OggPackBuffer *opb)
{
	int32 i;

	int32 vendorLen = OggPack_Read(opb, 32);
	if (vendorLen < 0)
		goto err_out;

	vc->vendor = (char *)_ogg_calloc(vendorLen + 1, 1);
	_V_ReadString(opb, (int8 *)vc->vendor, vendorLen);

	vc->comments = OggPack_Read(opb, 32);
	if (vc->comments < 0)
		goto err_out;

	vc->userComments   = (char **)_ogg_calloc(vc->comments + 1, sizeof(*vc->userComments));
	vc->commentLengths = (int32 *)_ogg_calloc(vc->comments + 1, sizeof(*vc->commentLengths));

	for (i = 0; i < vc->comments; i++)
	{
		int32 len = OggPack_Read(opb, 32);
		if (len < 0)
			goto err_out;

		vc->commentLengths[i] = len;
		vc->userComments[i]   = (char *)_ogg_calloc(len + 1, 1);
		_V_ReadString(opb, (int8 *)vc->userComments[i], len);
	}

	if (OggPack_Read(opb, 1) != 1)
		goto err_out;				// EOP check

	return (0);

err_out:
	Vorbis_Comment_Clear(vc);
	return (OV_EBADHEADER);
}



/******************************************************************************/
/* Vorbis_Unpack_Books() unpacks the codebook information.                    */
/*                                                                            */
/* Input:  "vi" is a pointer to the Vorbis information structure.             */
/*         "opb" is a pointer to the data buffer.                             */
/*                                                                            */
/* Output: 0 for success, else an error code.                                 */
/******************************************************************************/
int32 OggVorbis::Vorbis_Unpack_Books(VorbisInfo *vi, OggPackBuffer *opb)
{
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;
	int32 i;

	if (!ci)
		return (OV_EFAULT);

	// Codebooks
	ci->books = OggPack_Read(opb, 8) + 1;
	for (i = 0; i < ci->books; i++)
	{
		ci->bookParam[i] = (StaticCodebook *)_ogg_calloc(1, sizeof(*ci->bookParam[i]));

		if (Vorbis_Staticbook_Unpack(opb, ci->bookParam[i]))
			goto err_out;
	}

	// Time backend settings; hooks are unused
	{
		int32 times = OggPack_Read(opb, 6) + 1;
		for (i = 0; i < times; i++)
		{
			int32 test = OggPack_Read(opb, 16);
			if ((test < 0) || (test >= VI_TIMEB))
				goto err_out;
		}
	}

	// Floor backend settings
	ci->floors = OggPack_Read(opb, 6) + 1;
	for (i = 0; i < ci->floors; i++)
	{
		ci->floorType[i] = OggPack_Read(opb, 16);
		if ((ci->floorType[i] < 0) || (ci->floorType[i] >= VI_FLOORB))
			goto err_out;

		ci->floorParam[i] = floorP[ci->floorType[i]]->Unpack(this, vi, opb);
		if (!ci->floorParam[i])
			goto err_out;
	}

	// Residue backend settings
	ci->residues = OggPack_Read(opb, 6) + 1;
	for (i = 0; i < ci->residues; i++)
	{
		ci->residueType[i] = OggPack_Read(opb, 16);
		if ((ci->residueType[i] < 0) || (ci->residueType[i] >= VI_RESB))
			goto err_out;

		ci->residueParam[i] = residueP[ci->residueType[i]]->Unpack(this, vi, opb);
		if (!ci->residueParam[i])
			goto err_out;
	}

	// Map backend settings
	ci->maps = OggPack_Read(opb, 6) + 1;
	for (i = 0; i < ci->maps; i++)
	{
		ci->mapType[i] = OggPack_Read(opb, 16);
		if ((ci->mapType[i] < 0) || (ci->mapType[i] >= VI_MAPB))
			goto err_out;

		ci->mapParam[i] = mappingP[ci->mapType[i]]->Unpack(this, vi, opb);
		if (!ci->mapParam[i])
			goto err_out;
	}

	// Mode settings
	ci->modes = OggPack_Read(opb, 6) + 1;
	for (i = 0; i < ci->modes; i++)
	{
		ci->modeParam[i] = (VorbisInfoMode *)_ogg_calloc(1, sizeof(*ci->modeParam[i]));

		ci->modeParam[i]->blockFlag     = OggPack_Read(opb, 1);
		ci->modeParam[i]->windowType    = OggPack_Read(opb, 16);
		ci->modeParam[i]->transformType = OggPack_Read(opb, 16);
		ci->modeParam[i]->mapping       = OggPack_Read(opb, 8);

		if (ci->modeParam[i]->windowType >= VI_WINDOWB)
			goto err_out;

		if (ci->modeParam[i]->transformType >= VI_WINDOWB)
			goto err_out;

		if (ci->modeParam[i]->mapping >= ci->maps)
			goto err_out;
	}

	if (OggPack_Read(opb, 1) != 1)
		goto err_out;		// Top level EOP check

	return (0);

err_out:
	Vorbis_Info_Clear(vi);
	return (OV_EBADHEADER);
}



/******************************************************************************/
/* Vorbis_Synchesis_HeaderIn() reads the header.                              */
/*                                                                            */
/* The Vorbis header is in three packets; the initial small packet in the     */
/* first page that identifies basic parameters, a second packet with          */
/* bitstream comments and a third packet that holds the codebook.             */
/*                                                                            */
/* Input:  "vi" is a pointer to the Vorbis information structure.             */
/*         "vc" is a pointer to the comment structure.                        */
/*         "og" is a pointer to the Ogg packet.                               */
/*                                                                            */
/* Output: 0 for success, else an error code.                                 */
/******************************************************************************/
int32 OggVorbis::Vorbis_Synthesis_HeaderIn(VorbisInfo *vi, VorbisComment *vc, OggPacket *op)
{
	OggPackBuffer opb;

	if (op)
	{
		OggPack_ReadInit(&opb, op->packet, op->bytes);

		// Which of the three types of the header is this?
		// Also verify header-ness, vorbis
		{
			int8 buffer[6];
			int32 packType = OggPack_Read(&opb, 8);
			memset(buffer, 0, 6);
			_V_ReadString(&opb, buffer, 6);

			if (memcmp(buffer, "vorbis", 6))
			{
				// Not a vorbis header
				return (OV_ENOTVORBIS);
			}

			switch (packType)
			{
				// Least significant *bit* is read first
				case 0x01:
				{
					if (!op->b_o_s)
					{
						// Not the initial packet
						return (OV_EBADHEADER);
					}

					if (vi->rate != 0)
					{
						// Previously initialized info header
						return (OV_EBADHEADER);
					}

					return (Vorbis_Unpack_Info(vi, &opb));
				}

				// Least significant *bit* is read first
				case 0x03:
				{
					if (vi->rate == 0)
					{
						// Um... we didn't get the initial header
						return (OV_EBADHEADER);
					}

					return (Vorbis_Unpack_Comment(vc, &opb));
				}

				// Least significant *bit* is read first
				case 0x05:
				{
					if ((vi->rate == 0) || (vc->vendor == NULL))
					{
						// Um... we didn't get the initial header or comments yet
						return (OV_EBADHEADER);
					}

					return (Vorbis_Unpack_Books(vi, &opb));
				}

				default:
				{
					// Not a valid vorbis header type
					return (OV_EBADHEADER);
				}
			}
		}
	}

	return (OV_EBADHEADER);
}



/******************************************************************************/
/* _V_ReadString() read a single string.                                      */
/******************************************************************************/
void OggVorbis::_V_ReadString(OggPackBuffer *o, int8 *buf, int32 bytes)
{
	while (bytes--)
		*buf++ = OggPack_Read(o, 8);
}
