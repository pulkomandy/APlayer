/******************************************************************************/
/* VorbisFile functions.                                                      */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Player headers
#include "VorbisFile.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define CHUNKSIZE			8500



/******************************************************************************/
/* OV_Open()                                                                  */
/******************************************************************************/
int32 OggVorbis::OV_Open(PFile *f, OggVorbisFile *vf, int8 *initial, int32 iBytes)
{
	int32 ret = _OV_Open1(f, vf, initial, iBytes);
	if (ret)
		return (ret);

	return (_OV_Open2(vf));
}



/******************************************************************************/
/* OV_Clear()                                                                 */
/******************************************************************************/
int32 OggVorbis::OV_Clear(OggVorbisFile *vf)
{
	if (vf)
	{
		Vorbis_Block_Clear(&vf->vb);
		Vorbis_DSP_Clear(&vf->vd);
		Ogg_Stream_Clear(&vf->os);

		if (vf->vi && vf->links)
		{
			int32 i;

			for (i = 0; i < vf->links; i++)
			{
				Vorbis_Info_Clear(vf->vi + i);
				Vorbis_Comment_Clear(vf->vc + i);
			}

			_ogg_free(vf->vi);
			_ogg_free(vf->vc);
		}

		_ogg_free(vf->dataOffsets);
		_ogg_free(vf->pcmLengths);
		_ogg_free(vf->serialNos);
		_ogg_free(vf->offsets);

		Ogg_Sync_Clear(&vf->oy);

		memset(vf, 0, sizeof(*vf));
	}

	return (0);
}



/******************************************************************************/
/* OV_RawSeek() seek to an offset relative to the *compressed* data. This     */
/*      also scans packets to update the PCM cursor. It will cross a logical  */
/*      bitstream boundary, but only if it can't get any packets out of the   */
/*      tail of the bitstream we seek to (so no surprises).                   */
/******************************************************************************/
int32 OggVorbis::OV_RawSeek(OggVorbisFile *vf, int64 pos)
{
	OggStreamState workOs;

	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	if (!vf->seekable)
		return (OV_ENOSEEK);	// Don't dump machine if we can't seek

	if ((pos < 0) || (pos > vf->offsets[vf->links]))
		return (OV_EINVAL);

	// Clear out decoding machine state
	vf->pcmOffset = -1;
	_DecodeClear(vf);

	_SeekHelper(vf, pos);

	// We need to make sure the pcmOffset is set, but we don't want to
	// advance the raw cursor past good packets just to get to the first
	// with a granulepos. That's not equivalent behaviour to beginning
	// decoding as immediately after the seek position as possible.
	//
	// So, a hack. We use two stream states; a local scratch state and
	// the shared vf->os stream state. We use the local state to
	// scan, and the shared state as a buffer for later decoding.
	//
	// Unfortuantely, on the last page we still advance to last packet
	// because the granulepos on the last page is not necessarily on a
	// packet boundary, and we need to make sure the granpos is correct.
	{
		OggPage og;
		OggPacket op;
		int32 lastBlock = 0;
		int32 accBlock = 0;
		int32 thisBlock = 0;
		bool eosFlag = false;

		memset(&workOs, 0, sizeof(workOs));

		while (true)
		{
			if (vf->readyState == STREAMSET)
			{
				// Snarf/scan a packet if we can
				int32 result = Ogg_Stream_PacketOut(&workOs, &op);

				if (result > 0)
				{
					if (vf->vi[vf->currentLink].codecSetup)
						thisBlock = Vorbis_Packet_BlockSize(vf->vi + vf->currentLink, &op);

					if (eosFlag)
						Ogg_Stream_PacketOut(&vf->os, NULL);
					else
					{
						if (lastBlock)
							accBlock += (lastBlock + thisBlock) >> 2;
					}

					if (op.granulePos != -1)
					{
						int32 i, link = vf->currentLink;
						int64 granulePos = op.granulePos;

						for (i = 0; i < link; i++)
							granulePos += vf->pcmLengths[i];

						vf->pcmOffset = granulePos - accBlock;
						break;
					}

					lastBlock = thisBlock;
					continue;
				}
			}

			if (!lastBlock)
			{
				if (_GetNextPage(vf, &og, -1) < 0)
				{
					vf->pcmOffset = OV_PCM_Total(vf, -1);
					break;
				}
			}
			else
			{
				// Huh? Bogus stream with packets but no granulepos
				vf->pcmOffset = -1;
				break;
			}

			// Has our decoding just traversed a bitstream boundary?
			if (vf->readyState == STREAMSET)
			{
				if (vf->currentSerialNo != Ogg_Page_SerialNo(&og))
				{
					_DecodeClear(vf);	// Clear out stream state
					Ogg_Stream_Clear(&workOs);
				}
			}

			if (vf->readyState < STREAMSET)
			{
				int32 link;

				vf->currentSerialNo = Ogg_Page_SerialNo(&og);

				for (link = 0; link < vf->links; link++)
				{
					if (vf->serialNos[link] == vf->currentSerialNo)
						break;
				}

				if (link == vf->links)
					goto seek_error;	// Sign of a bogus stream. Error out, leave machine uninitialized

				vf->currentLink = link;

				Ogg_Stream_Init(&vf->os, vf->currentSerialNo);
				Ogg_Stream_Reset(&vf->os);
				Ogg_Stream_Init(&workOs, vf->currentSerialNo);
				Ogg_Stream_Reset(&workOs);
				vf->readyState = STREAMSET;
			}

			Ogg_Stream_PageIn(&vf->os, &og);
			Ogg_Stream_PageIn(&workOs, &og);
			eosFlag = Ogg_Page_EOS(&og);
		}
	}

	Ogg_Stream_Clear(&workOs);
	return (0);

seek_error:
	// Dump the machine so we're in a known state
	vf->pcmOffset = -1;
	Ogg_Stream_Clear(&workOs);
	_DecodeClear(vf);

	return (OV_EBADLINK);
}



/******************************************************************************/
/* OV_PCMSeek()                                                               */
/******************************************************************************/
int32 OggVorbis::OV_PCMSeek(OggVorbisFile *vf, int64 pos)
{
	int32 thisBlock, lastBlock = 0;
	int32 ret = OV_PCMSeekPage(vf, pos);

	if (ret < 0)
		return (ret);

	// Discard leading packets we don't need for the lapping of the
	// position we want; don't decode them
	while (true)
	{
		OggPacket op;
		OggPage og;

		int32 ret = Ogg_Stream_PacketPeek(&vf->os, &op);
		if (ret > 0)
		{
			thisBlock = Vorbis_Packet_BlockSize(vf->vi + vf->currentLink, &op);
			if (lastBlock)
				vf->pcmOffset += (lastBlock + thisBlock) >> 2;

			if (vf->pcmOffset + ((thisBlock + Vorbis_Info_BlockSize(vf->vi, 1)) >> 2) >= pos)
				break;

			Ogg_Stream_PacketOut(&vf->os, NULL);

			// End of logical stream case is hard, especially with exact
			// length positioning
			if (op.granulePos > -1)
			{
				int32 i;

				// Always believe the stream markers
				vf->pcmOffset = op.granulePos;

				for (i = 0; i < vf->currentLink; i++)
					vf->pcmOffset += vf->pcmLengths[i];
			}

			lastBlock = thisBlock;
		}
		else
		{
			if ((ret < 0) && (ret != OV_HOLE))
				break;

			// Suck in a new page
			if (_GetNextPage(vf, &og, -1) < 0)
				break;

			if (vf->currentSerialNo != Ogg_Page_SerialNo(&og))
				_DecodeClear(vf);

			if (vf->readyState < STREAMSET)
			{
				int32 link;

				vf->currentSerialNo = Ogg_Page_SerialNo(&og);

				for (link = 0; link < vf->links; link++)
				{
					if (vf->serialNos[link] == vf->currentSerialNo)
						break;
				}

				if (link == vf->links)
					return (OV_EBADLINK);

				vf->currentLink = link;

				Ogg_Stream_Init(&vf->os, vf->currentSerialNo);
				Ogg_Stream_Reset(&vf->os);
				vf->readyState = STREAMSET;
				lastBlock = 0;
			}

			Ogg_Stream_PageIn(&vf->os, &og);
		}
	}

	// Discard samples until we reach the desired position. Crossing a
	// logical bitstream boundary with abandon is OK
	_MakeDecodeReady(vf);

	while (vf->pcmOffset < pos)
	{
		float **pcm;
		int32 target = pos - vf->pcmOffset;
		int32 samples = Vorbis_Synthesis_PCMOut(&vf->vd, &pcm);

		if (samples > target)
			samples = target;

		Vorbis_Synthesis_Read(&vf->vd, samples);
		vf->pcmOffset += samples;

		if (samples < target)
		{
			if (_ProcessPacket(vf, 1) <= 0)
				vf->pcmOffset = OV_PCM_Total(vf, -1);	// eof
		}
	}

	return (0);
}



/******************************************************************************/
/* OV_TimeSeek()                                                              */
/******************************************************************************/
int32 OggVorbis::OV_TimeSeek(OggVorbisFile *vf, double seconds)
{
	int32 link = -1;
	int64 pcmTotal = OV_PCM_Total(vf, -1);
	double timeTotal = OV_Time_Total(vf, -1);

	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	if (!vf->seekable)
		return (OV_ENOSEEK);

	if ((seconds < 0.0) || (seconds > timeTotal))
		return (OV_EINVAL);

	// Which bitstream section does this time offset occur in?
	for (link = vf->links - 1; link >= 0; link--)
	{
		pcmTotal  -= vf->pcmLengths[link];
		timeTotal -= OV_Time_Total(vf, link);

		if (seconds >= timeTotal)
			break;
	}

	// Enough information to convert time offset to pcm offset
	{
		int64 target = (int64)(pcmTotal + (seconds - timeTotal) * vf->vi[link].rate);
		return (OV_PCMSeek(vf, target));
	}
}



/******************************************************************************/
/* OV_PCMSeekPage()                                                           */
/******************************************************************************/
int32 OggVorbis::OV_PCMSeekPage(OggVorbisFile *vf, int64 pos)
{
	int32 link = -1;
	int32 ret;
	int64 total = OV_PCM_Total(vf, -1);

	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	if (!vf->seekable)
		return (OV_ENOSEEK);

	if ((pos < 0) || (pos > total))
		return (OV_EINVAL);

	// Which bitstream section does this pcm offset occur in?
	for (link = vf->links - 1; link >= 0; link--)
	{
		total -= vf->pcmLengths[link];
		if (pos >= total)
			break;
	}

	// Search within the logical bitstream for the page with the highest
	// pcmPos preceeding (or equal to) pos. There is a danger here;
	// missing pages or incorrect frame number information in the
	// bitstream could make our task impossible. Account for that (it
	// would be an error condition)
	//
	// New search algorithm by HB (Nicholas Vinen)
	{
		int64 target    = pos - total;
		int32 end       = vf->offsets[link + 1];
		int32 begin     = vf->offsets[link];
		int64 endTime   = vf->pcmLengths[link];
		int64 beginTime = 0;
		int32 best      = begin;
		OggPage og;

		while (begin < end)
		{
			int32 bisect;

			if ((end - begin) < CHUNKSIZE)
				bisect = begin;
			else
			{
				// Take a (pretty decent) guess
				bisect = begin + (target - beginTime) * (end - begin) / (endTime - beginTime) - CHUNKSIZE;

				if (bisect <= begin)
					bisect = begin + 1;
			}

			_SeekHelper(vf, bisect);

			while (begin < end)
			{
				ret = _GetNextPage(vf, &og, end - bisect);
				if (ret == OV_EREAD)
					goto seek_error;

				if (ret < 0)
				{
					if (bisect <= (begin + 1))
						end = begin;	// Found it
					else
					{
						if (bisect == 0)
							goto seek_error;

						bisect -= CHUNKSIZE;
						if (bisect <= begin)
							bisect = begin + 1;

						_SeekHelper(vf, bisect);
					}
				}
				else
				{
					int64 granulePos = Ogg_Page_GranulePos(&og);
					if (granulePos < target)
					{
						best = ret;		// Raw offset of packet with granulepos
						begin = vf->offset;	// Raw offset of next page
						beginTime = granulePos;

						if ((target - begin) > 44100)
							break;

						bisect = begin;	// *not* begin + 1
					}
					else
					{
						if (bisect <= (begin + 1))
							end = begin;	// Found it
						else
						{
							if (end == vf->offset)	// We're pretty close - we'd be stuck in an endless loop otherwise
							{
								end     = ret;
								bisect -= CHUNKSIZE;

								if (bisect <= begin)
									bisect = begin + 1;

								_SeekHelper(vf, bisect);
							}
							else
							{
								end     = ret;
								endTime = granulePos;
								break;
							}
						}
					}
				}
			}
		}

		// Found our page. Seek to it, update pcm offset. Easier case than
		// raw_seek, don't keep packets preceeding granulepos
		{
			OggPage og;
			OggPacket op;

			// Clear out decoding machine state
			_DecodeClear(vf);

			// Seek
			_SeekHelper(vf, best);

			if (_GetNextPage(vf, &og, -1) < 0)
				return (OV_EOF);		// Shouldn't happen

			vf->currentSerialNo = Ogg_Page_SerialNo(&og);
			vf->currentLink     = link;

			Ogg_Stream_Init(&vf->os, vf->currentSerialNo);
			Ogg_Stream_Reset(&vf->os);
			vf->readyState = STREAMSET;

			Ogg_Stream_PageIn(&vf->os, &og);

			// Pull out all but last packet; the one with granulepos
			while (true)
			{
				ret = Ogg_Stream_PacketPeek(&vf->os, &op);
				if (ret == 0)
				{
					// !!! The packet finishing this page originated on a
					// preceeding page. Keep fetching previous pages until we
					// get one with a granulepos or without the 'continued' flag
					// set. Then just use raw_seek for simplicity
					while (true)
					{
						ret = _GetPrevPage(vf, &og);
						if (ret < 0)
							goto seek_error;

						if ((Ogg_Page_GranulePos(&og) > -1) || !Ogg_Page_Continued(&og))
							return (OV_RawSeek(vf, ret));

						vf->offset = ret;
					}
				}

				if (ret < 0)
					goto seek_error;

				if (op.granulePos != -1)
				{
					vf->pcmOffset = op.granulePos + total;
					break;
				}
				else
					ret = Ogg_Stream_PacketOut(&vf->os, NULL);
			}
		}
	}

	// Verify result
	if ((vf->pcmOffset > pos) || (pos > OV_PCM_Total(vf, -1)))
	{
		ret = OV_EFAULT;
		goto seek_error;
	}

	return (0);

seek_error:
	// Dump machine so we're in a known state
	vf->pcmOffset = -1;
	_DecodeClear(vf);

	return (ret);
}



/******************************************************************************/
/* OV_Read()                                                                  */
/*                                                                            */
/* length is number of samples.                                               */
/******************************************************************************/
int32 OggVorbis::OV_Read(OggVorbisFile *vf, float ***buffer, int32 length, int32 *bitStream)
{
	int32 samples;

	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	while (true)
	{
		if (vf->readyState >= STREAMSET)
		{
			samples = Vorbis_Synthesis_PCMOut(&vf->vd, buffer);
			if (samples)
				break;
		}

		// Suck in another packet
		{
			int32 ret = _ProcessPacket(vf, 1);
			if (ret == OV_EOF)
				return (0);

			if (ret <= 0)
				return (ret);
		}
	}

	if (samples > 0)
	{
		if (samples > length)
			samples = length;

		Vorbis_Synthesis_Read(&vf->vd, samples);
		vf->pcmOffset += samples;

		if (bitStream)
			*bitStream = vf->currentLink;
	}

	return (samples);
}



/******************************************************************************/
/* OV_Info()                                                                  */
/******************************************************************************/
VorbisInfo *OggVorbis::OV_Info(OggVorbisFile *vf, int32 link)
{
	if (vf->seekable)
	{
		if (link < 0)
		{
			if (vf->readyState >= STREAMSET)
				return (vf->vi + vf->currentLink);
			else
				return (vf->vi);
		}
		else
		{
			if (link >= vf->links)
				return (NULL);
			else
				return (vf->vi + link);
		}
	}
	else
		return (vf->vi);
}



/******************************************************************************/
/* OV_Comment()                                                               */
/******************************************************************************/
VorbisComment *OggVorbis::OV_Comment(OggVorbisFile *vf, int32 link)
{
	if (vf->seekable)
	{
		if (link < 0)
		{
			if (vf->readyState >= STREAMSET)
				return (vf->vc + vf->currentLink);
			else
				return (vf->vc);
		}
		else
		{
			if (link >= vf->links)
				return (NULL);
			else
				return (vf->vc + link);
		}
	}
	else
		return (vf->vc);
}



/******************************************************************************/
/* OV_PCM_Total() returns total PCM length (samples) of content if i == -1,   */
/*      PCM length (samples) of that logical bitstream for i == 0 to n,       */
/*      OV_EINVAL if the stream is not seekable (we can't know the length)    */
/*      or only partially open.                                               */
/******************************************************************************/
int64 OggVorbis::OV_PCM_Total(OggVorbisFile *vf, int32 i)
{
	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	if (!vf->seekable || (i >= vf->links))
		return (OV_EINVAL);

	if (i < 0)
	{
		int64 acc = 0;
		int32 i;

		for (i = 0; i < vf->links; i++)
			acc += OV_PCM_Total(vf, i);

		return (acc);
	}
	else
		return (vf->pcmLengths[i]);
}



/******************************************************************************/
/* OV_Time_Total() returns total seconds of content if i == -1,               */
/*      seconds in that logical bitstream for i == 0 to n,                    */
/*      OV_EINVAL if the stream is not seekable (we can't know the length)    */
/*      or only partially open.                                               */
/******************************************************************************/
double OggVorbis::OV_Time_Total(OggVorbisFile *vf, int32 i)
{
	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	if (!vf->seekable || (i >= vf->links))
		return (OV_EINVAL);

	if (i < 0)
	{
		double acc = 0.0;
		int32 i;

		for (i = 0; i < vf->links; i++)
			acc += OV_Time_Total(vf, i);

		return (acc);
	}
	else
		return ((float)(vf->pcmLengths[i]) / vf->vi[i].rate);
}



/******************************************************************************/
/* OV_Time_Tell() returns time offset (seconds) of the next PCM sample to be  */
/*      read.                                                                 */
/******************************************************************************/
double OggVorbis::OV_Time_Tell(OggVorbisFile *vf)
{
	int32 link = -1;
	int64 pcmTotal = 0;
	double timeTotal = 0.0;

	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	if (vf->seekable)
	{
		pcmTotal  = OV_PCM_Total(vf, -1);
		timeTotal = OV_Time_Total(vf, -1);

		// Which bitstream section does this time offset occur in?
		for (link = vf->links - 1; link >= 0; link--)
		{
			pcmTotal  -= vf->pcmLengths[link];
			timeTotal -= OV_Time_Total(vf, link);

			if (vf->pcmOffset >= pcmTotal)
				break;
		}
	}

	return ((double)timeTotal + (double)(vf->pcmOffset - pcmTotal) / vf->vi[link].rate);
}






/******************************************************************************/
/* OV_Bitrate() finds the average bitrate used in the bitstream.              */
/******************************************************************************/
int32 OggVorbis::OV_Bitrate(OggVorbisFile *vf, int32 i)
{
	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	if (i >= vf->links)
		return (OV_EINVAL);

	if (!vf->seekable && (i != 0))
		return (OV_Bitrate(vf, 0));

	if (i < 0)
	{
		int64 bits = 0;
		int32 i;

		for (i = 0; i < vf->links; i++)
			bits += (vf->offsets[i + 1] - vf->dataOffsets[i]) * 8;

		return ((int32)rint(bits / OV_Time_Total(vf, -1)));
	}
	else
	{
		if (vf->seekable)
		{
			// Return the actual bitrate
			return ((int32)rint((vf->offsets[i + 1] - vf->dataOffsets[i]) * 8 / OV_Time_Total(vf, i)));
		}
		else
		{
			// Return nominal if set
			if (vf->vi[i].bitrateNominal > 0)
				return (vf->vi[i].bitrateNominal);
			else
			{
				if (vf->vi[i].bitrateUpper > 0)
				{
					if (vf->vi[i].bitrateLower > 0)
						return ((vf->vi[i].bitrateUpper + vf->vi[i].bitrateLower) / 2);
					else
						return (vf->vi[i].bitrateUpper);
				}

				return (OV_FALSE);
			}
		}
	}
}



/******************************************************************************/
/* OV_BitrateInstant() returns the actual bitrate since last call.            */
/******************************************************************************/
int32 OggVorbis::OV_BitrateInstant(OggVorbisFile *vf)
{
	int32 link = (vf->seekable ? vf->currentLink : 0);
	int32 ret;

	if (vf->readyState < OPENED)
		return (OV_EINVAL);

	if (vf->sampTrack == 0)
		return (OV_FALSE);

	ret = (int32)(vf->bitTrack / vf->sampTrack * vf->vi[link].rate + 0.5);
	vf->bitTrack  = 0.0;
	vf->sampTrack = 0.0;

	return (ret);
}



/******************************************************************************/
/* _OV_Open1()                                                                */
/******************************************************************************/
int32 OggVorbis::_OV_Open1(PFile *f, OggVorbisFile *vf, int8 *initial, int32 iBytes)
{
	int32 ret;

	memset(vf, 0, sizeof(*vf));
	vf->datasource = f;

	// Init the framing state
	Ogg_Sync_Init(&vf->oy);

	// Perhaps some data was previously read into a buffer for testing
	// against other stream types. Allow initialization from this
	// previously read data (as we may be reading from a non-seekable
	// stream)
	if (initial)
	{
		int8 *buffer = Ogg_Sync_Buffer(&vf->oy, iBytes);
		memcpy(buffer, initial, iBytes);
		Ogg_Sync_Wrote(&vf->oy, iBytes);
	}

	// Can we seek?
	vf->seekable = true;

	// No seeking yet; Setup a 'single' (current) logical bitstream
	// entry for partial open
	vf->links = 1;
	vf->vi    = (VorbisInfo *)_ogg_calloc(vf->links, sizeof(*vf->vi));
	vf->vc    = (VorbisComment *)_ogg_calloc(vf->links, sizeof(*vf->vc));

	// Try to fetch the headers, maintaining all the storage
	if ((ret = _FetchHeaders(vf, vf->vi, vf->vc, &vf->currentSerialNo, NULL)) < 0)
	{
		vf->datasource = NULL;
		OV_Clear(vf);
	}
	else
	{
		if (vf->readyState < PARTOPEN)
			vf->readyState = PARTOPEN;
	}

	return (ret);
}



/******************************************************************************/
/* _OV_Open2()                                                                */
/******************************************************************************/
int32 OggVorbis::_OV_Open2(OggVorbisFile *vf)
{
	if (vf->readyState < OPENED)
		vf->readyState = OPENED;

	if (vf->seekable)
	{
		int32 ret = _OpenSeekable2(vf);
		if (ret)
		{
			vf->datasource = NULL;
			OV_Clear(vf);
		}

		return (ret);
	}

	return (0);
}



/******************************************************************************/
/* _OpenSeekable2()                                                           */
/******************************************************************************/
int32 OggVorbis::_OpenSeekable2(OggVorbisFile *vf)
{
	int32 serialNo = vf->currentSerialNo;
	int64 dataOffset = vf->offset;
	OggPage og;
	int64 end;

	// We're partial open and have a first link header state in
	// storage in vf
	//
	// We can seek, so set out learning all about this file
	vf->datasource->SeekToEnd();
	vf->offset = vf->end = vf->datasource->GetPosition();

	// We get the offset for the last page of the physical bitstream.
	// Most OggVorbis files will contain a single logical bitstream
	end = _GetPrevPage(vf, &og);
	if (end < 0)
	{
		OV_Clear(vf);
		return (end);
	}

	// More than one logical bitstream?
	if (Ogg_Page_SerialNo(&og) != serialNo)
	{
		// Chained bitstream. Bisect-search each logical bitstream
		// section. Do so based on serial number only
		if (_BisectForwardSerialNo(vf, 0, 0, end + 1, serialNo, 0) < 0)
		{
			OV_Clear(vf);
			return (OV_EREAD);
		}
	}
	else
	{
		// Only one logical bitstream
		if (_BisectForwardSerialNo(vf, 0, end, end + 1, serialNo, 0))
		{
			OV_Clear(vf);
			return (OV_EREAD);
		}
	}

	// The initial header memory is referenced by vf after; don't free it
	_PrefetchAllHeaders(vf, dataOffset);

	return (OV_RawSeek(vf, 0));
}



/******************************************************************************/
/* _FetchHeaders()                                                            */
/******************************************************************************/
int32 OggVorbis::_FetchHeaders(OggVorbisFile *vf, VorbisInfo *vi, VorbisComment *vc, int32 *serialNo, OggPage *ogPtr)
{
	OggPage og;
	OggPacket op;
	int32 i, ret = 0;

	if (!ogPtr)
	{
		ret = _GetNextPage(vf, &og, CHUNKSIZE);
		if (ret == OV_EREAD)
			return (OV_EREAD);

		if (ret < 0)
			return (OV_ENOTVORBIS);

		ogPtr = &og;
	}

	if (serialNo)
		*serialNo = Ogg_Page_SerialNo(ogPtr);

	Ogg_Stream_Init(&vf->os, Ogg_Page_SerialNo(ogPtr));
	vf->readyState = STREAMSET;

	// Extract the initial header from the first page and verify that the
	// Ogg bitstream is in fact Vorbis data
	Vorbis_Info_Init(vi);
	Vorbis_Comment_Init(vc);

	i = 0;
	while (i < 3)
	{
		Ogg_Stream_PageIn(&vf->os, ogPtr);

		while (i < 3)
		{
			int32 result = Ogg_Stream_PacketOut(&vf->os, &op);
			if (result == 0)
				break;

			if (result == -1)
			{
				ret = OV_EBADHEADER;
				goto bail_header;
			}

			if ((ret = Vorbis_Synthesis_HeaderIn(vi, vc, &op)))
				goto bail_header;

			i++;
		}

		if (i < 3)
		{
			if (_GetNextPage(vf, ogPtr, CHUNKSIZE) < 0)
			{
				ret = OV_EBADHEADER;
				goto bail_header;
			}
		}
	}

	return (0);

bail_header:
	Vorbis_Info_Clear(vi);
	Vorbis_Comment_Clear(vc);
	Ogg_Stream_Clear(&vf->os);
	vf->readyState = OPENED;

	return (ret);
}



/******************************************************************************/
/* _PrefetchAllHeaders()                                                      */
/******************************************************************************/
void OggVorbis::_PrefetchAllHeaders(OggVorbisFile *vf, int64 dataOffset)
{
	OggPage og;
	int32 i;
	int64 ret;

	vf->vi          = (VorbisInfo *)_ogg_realloc(vf->vi, vf->links * sizeof(*vf->vi));
	vf->vc          = (VorbisComment *)_ogg_realloc(vf->vc, vf->links * sizeof(*vf->vc));
	vf->dataOffsets = (int64 *)_ogg_malloc(vf->links * sizeof(*vf->dataOffsets));
	vf->pcmLengths  = (int64 *)_ogg_malloc(vf->links * sizeof(*vf->pcmLengths));
	vf->serialNos   = (int32 *)_ogg_malloc(vf->links * sizeof(*vf->serialNos));

	for (i = 0; i < vf->links; i++)
	{
		if (i == 0)
		{
			// We already grabbed the initial header earlier. Just set the offset
			vf->dataOffsets[i] = dataOffset;
		}
		else
		{
			// Seek to the location of the initial header
			_SeekHelper(vf, vf->offsets[i]);
			if (_FetchHeaders(vf, vf->vi + i, vf->vc + i, NULL, NULL) < 0)
				vf->dataOffsets[i] = -1;
			else
			{
				vf->dataOffsets[i] = vf->offset;
				Ogg_Stream_Clear(&vf->os);
			}
		}

		// Get the serial number and PCM length of this link. To do this,
		// get the last page of the stream
		{
			int64 end = vf->offsets[i + 1];
			_SeekHelper(vf, end);

			while (true)
			{
				ret = _GetPrevPage(vf, &og);
				if (ret < 0)
				{
					// This should not be possible
					Vorbis_Info_Clear(vf->vi + i);
					Vorbis_Comment_Clear(vf->vc + i);
					break;
				}

				if (Ogg_Page_GranulePos(&og) != -1)
				{
					vf->serialNos[i]  = Ogg_Page_SerialNo(&og);
					vf->pcmLengths[i] = Ogg_Page_GranulePos(&og);
					break;
				}

				vf->offset = ret;
			}
		}
	}
}



/******************************************************************************/
/* _BisectForwardSerialNo() finds each bitstream link one at a time using a   */
/*      bisection search (has to begin knowing the offset of the lb's         */
/*      initial page). Recurses for each link so it can alloc the link        */
/*      storage after finding them all, then unroll and fill the cache at the */
/*      same time.                                                            */
/******************************************************************************/
int32 OggVorbis::_BisectForwardSerialNo(OggVorbisFile *vf, int64 begin, int64 searched, int64 end, int32 currentNo, int32 m)
{
	int64 endSearched = end;
	int64 next = end;
	OggPage og;
	int64 ret;

	// The below guards against garbage seperating the last and first pages of two links
	while (searched < endSearched)
	{
		int64 bisect;

		if ((endSearched - searched) < CHUNKSIZE)
			bisect = searched;
		else
			bisect = (searched + endSearched) / 2;

		_SeekHelper(vf, bisect);
		ret = _GetNextPage(vf, &og, -1);
		if (ret == OV_EREAD)
			return (OV_EREAD);

		if ((ret < 0) || (Ogg_Page_SerialNo(&og) != currentNo))
		{
			endSearched = bisect;
			if (ret >= 0)
				next = ret;
		}
		else
			searched = ret + og.headerLen + og.bodyLen;
	}

	_SeekHelper(vf, next);
	ret = _GetNextPage(vf, &og, -1);
	if (ret == OV_EREAD)
		return (OV_EREAD);

	if ((searched >= end) || (ret < 0))
	{
		vf->links          = m + 1;
		vf->offsets        = (int64 *)_ogg_malloc((m + 2) * sizeof(*vf->offsets));
		vf->offsets[m + 1] = searched;
	}
	else
	{
		ret = _BisectForwardSerialNo(vf, next, vf->offset, end, Ogg_Page_SerialNo(&og), m + 1);
		if (ret == OV_EREAD)
			return (OV_EREAD);
	}

	vf->offsets[m] = begin;

	return (0);
}



/******************************************************************************/
/* _SeekHelper()                                                              */
/******************************************************************************/
void OggVorbis::_SeekHelper(OggVorbisFile *vf, int64 offset)
{
	if (vf->datasource)
	{
		vf->datasource->Seek(offset, PFile::pSeekBegin);
		vf->offset = offset;
		Ogg_Sync_Reset(&vf->oy);
	}
	else
	{
		// Shouldn't happen unless someone writes a broken callback
		return;
	}
}



/******************************************************************************/
/* _GetData()                                                                 */
/******************************************************************************/
int32 OggVorbis::_GetData(OggVorbisFile *vf)
{
	if (vf->datasource)
	{
		int8 *buffer = Ogg_Sync_Buffer(&vf->oy, CHUNKSIZE);
		int32 bytes = vf->datasource->Read(buffer, CHUNKSIZE);

		if (bytes > 0)
			Ogg_Sync_Wrote(&vf->oy, bytes);

		return (bytes);
	}
	else
		return (0);
}



/******************************************************************************/
/* _ProcessPacket() fetch and process a packet. Handles the case where we're  */
/*      at a bitstream boundary and dumps the decoding machine. If the        */
/*      decoding machine is unloaded, it loads it.                            */
/******************************************************************************/
int32 OggVorbis::_ProcessPacket(OggVorbisFile *vf, int32 readp)
{
	OggPage og;

	// Handle one packet. Try to fetch it from current stream state
	// extract packets from page
	while (true)
	{
		// Process a packet if we can. If the machine isn't loaded,
		// neither is a page
		if (vf->readyState == INITSET)
		{
			while (true)
			{
				OggPacket op;
				int32 result = Ogg_Stream_PacketOut(&vf->os, &op);
				int64 granulePos;

				if (result == -1)
					return (OV_HOLE);	// Hole in the data

				if (result > 0)
				{
					// Got a packet. Process it
					granulePos = op.granulePos;

					// Lazy check for lazy header handling. The header packets
					// aren't audio, so if/when we submit them, Vorbis_Synthesis
					// will reject them
					if (!Vorbis_Synthesis(&vf->vb, &op))
					{
						// Suck in the synthesis data and track bitrate
						{
							int32 oldSamples = Vorbis_Synthesis_PCMOut(&vf->vd, NULL);
							Vorbis_Synthesis_BlockIn(&vf->vd, &vf->vb);
							vf->sampTrack += Vorbis_Synthesis_PCMOut(&vf->vd, NULL) - oldSamples;
							vf->bitTrack  += op.bytes * 8;
						}

						// Update the PCM offset
						if ((granulePos != -1) && !op.e_o_s)
						{
							int32 link = (vf->seekable ? vf->currentLink : 0);
							int32 i, samples;

							// This packet has a pcmOffset on it (the last packet
							// completed on a page carries the offset). After processing
							// (above), we know the PCM position of the *last* sample
							// ready to be returned. Find the offset of the *first*.
							//
							// As an aside, this trick is inaccurate if we begin
							// reading a new right at the last page; the end-of-stream
							// granulepos declares the last frame in the stream, and the
							// last packet of the last page may be a partial frame.
							// So, we need a previous granulepos from an in-sequence page
							// to have a reference point. Thus the !op.e_o_s clause above
							samples = Vorbis_Synthesis_PCMOut(&vf->vd, NULL);

							granulePos -= samples;
							for (i = 0; i < link; i++)
								granulePos += vf->pcmLengths[i];

							vf->pcmOffset = granulePos;
						}

						return (1);
					}
				}
				else
					break;
			}
		}

		if (vf->readyState >= OPENED)
		{
			if (!readp)
				return (0);

			if (_GetNextPage(vf, &og, -1) < 0)
				return (OV_EOF);

			// Bitrate tracking; add the header's bytes here, the body bytes
			// are done by packet above
			vf->bitTrack += og.headerLen * 8;

			// Has our decoding just traversed a bitstream boundary?
			if (vf->readyState == INITSET)
			{
				if (vf->currentSerialNo != Ogg_Page_SerialNo(&og))
				{
					_DecodeClear(vf);

					if (!vf->seekable)
					{
						Vorbis_Info_Clear(vf->vi);
						Vorbis_Comment_Clear(vf->vc);
					}
				}
			}
		}

		// Do we need to load a new machine before submitting the page?
		// This is different in the seekable and non-seekable cases.
		//
		// In the seekable case, we already have all the header
		// information loaded and cached; we just initialize the machine
		// with it and continue on our merry way.
		//
		// In the non-seekable (streaming) case, we'll only be at a
		// boundary if we just left the previous logical bitstream and
		// we're now nominally at the header of the next bitstream
		if (vf->readyState != INITSET)
		{
			int32 link;

			if (vf->readyState < STREAMSET)
			{
				if (vf->seekable)
				{
					vf->currentSerialNo = Ogg_Page_SerialNo(&og);

					// Match the serialno to bitstream section. We use this rather than
					// offset positions to avoid problems near logical bitstream
					// boundaries
					for (link = 0; link < vf->links; link++)
					{
						if (vf->serialNos[link] == vf->currentSerialNo)
							break;
					}

					if (link == vf->links)
						return (OV_EBADLINK);	// Sign of bogus stream. Error out, leave machine uninitialized

					vf->currentLink = link;

					Ogg_Stream_Init(&vf->os, vf->currentSerialNo);
					Ogg_Stream_Reset(&vf->os);
					vf->readyState = STREAMSET;
				}
				else
				{
					// We're streaming
					// Fetch the three header packets, build the info struct
					int32 ret = _FetchHeaders(vf, vf->vi, vf->vc, &vf->currentSerialNo, &og);
					if (ret)
						return (ret);

					vf->currentLink++;
					link = 0;
				}
			}

			_MakeDecodeReady(vf);
		}

		Ogg_Stream_PageIn(&vf->os, &og);
	}
}



/******************************************************************************/
/* _GetPrevPage()                                                             */
/******************************************************************************/
int64 OggVorbis::_GetPrevPage(OggVorbisFile *vf, OggPage *og)
{
	int64 begin = vf->offset;
	int64 ret;
	int64 offset = -1;

	while (offset == -1)
	{
		begin -= CHUNKSIZE;
		if (begin < 0)
			begin = 0;

		_SeekHelper(vf, begin);

		while (vf->offset < (begin + CHUNKSIZE))
		{
			ret = _GetNextPage(vf, og, begin + CHUNKSIZE - vf->offset);
			if (ret == OV_EREAD)
				return (OV_EREAD);

			if (ret < 0)
				break;
			else
				offset = ret;
		}
	}

	// We have the offset. Actually snork and hold the page now
	_SeekHelper(vf, offset);
	ret = _GetNextPage(vf, og, CHUNKSIZE);

	if (ret < 0)
	{
		// This shouldn't be possible
		return (OV_EFAULT);
	}

	return (offset);
}



/******************************************************************************/
/* _GetNextPage()                                                             */
/******************************************************************************/
int64 OggVorbis::_GetNextPage(OggVorbisFile *vf, OggPage *og, int64 boundary)
{
	if (boundary > 0)
		boundary += vf->offset;

	while (true)
	{
		int32 more;

		if ((boundary > 0) && (vf->offset >= boundary))
			return (OV_FALSE);

		more = Ogg_Sync_PageSeek(&vf->oy, og);
		if (more < 0)
		{
			// Skipped n bytes
			vf->offset -= more;
		}
		else
		{
			if (more == 0)
			{
				if (!boundary)
					return (OV_FALSE);

				{
					int32 ret = _GetData(vf);
					if (ret == 0)
						return (OV_EOF);

					if (ret < 0)
						return (OV_EREAD);
				}
			}
			else
			{
				// Got a page. Return the offset at the page beginning,
				// advance the internal offset past the page end
				int64 ret = vf->offset;
				vf->offset += more;

				return (ret);
			}
		}
	}
}



/******************************************************************************/
/* _DecodeClear() clear out the current logical bitstream decoder.            */
/******************************************************************************/
void OggVorbis::_DecodeClear(OggVorbisFile *vf)
{
	Ogg_Stream_Clear(&vf->os);
	Vorbis_DSP_Clear(&vf->vd);
	Vorbis_Block_Clear(&vf->vb);
	vf->readyState = OPENED;

	vf->bitTrack  = 0.0;
	vf->sampTrack = 0.0;
}



/******************************************************************************/
/* _MakeDecodeReady()                                                         */
/******************************************************************************/
void OggVorbis::_MakeDecodeReady(OggVorbisFile *vf)
{
	if (vf->readyState != STREAMSET)
		return;

	if (vf->seekable)
		Vorbis_Synthesis_Init(&vf->vd, vf->vi + vf->currentLink);
	else
		Vorbis_Synthesis_Init(&vf->vd, vf->vi);

	Vorbis_Block_Init(&vf->vd, &vf->vb);
	vf->readyState = INITSET;
}
