/******************************************************************************/
/* Vorbis codebook functions.                                                 */
/******************************************************************************/


// Player headers
#include "OggVorbis.h"


/******************************************************************************/
/* Vorbis_Staticbook_Unpack() unpacks a codebook from the packet buffer into  */
/*      the codebook struct, readies the codebook auxiliary structures for    */
/*      decode.                                                               */
/*                                                                            */
/* Input:  "opb" is a pointer to the packet buffer.                           */
/*         "s" is a pointer to the codebook.                                  */
/*                                                                            */
/* Output: 0 for success, -1 for failure.                                     */
/******************************************************************************/
int32 OggVorbis::Vorbis_Staticbook_Unpack(OggPackBuffer *opb, StaticCodebook *s)
{
	int32 i, j;

	memset(s, 0, sizeof(*s));
	s->allocedP = true;

	// Make sure alignment is correct
	if (OggPack_Read(opb, 24) != 0x564342)
		goto _eofout;

	// First the basic parameters
	s->dim     = OggPack_Read(opb, 16);
	s->entries = OggPack_Read(opb, 24);
	if (s->entries == -1)
		goto _eofout;

	// Codeword ordering.... length ordered or unordered?
	switch (OggPack_Read(opb, 1))
	{
		// Unordered
		case 0:
		{
			s->lengthList = (int32 *)_ogg_malloc(sizeof(*s->lengthList) * s->entries);

			// Allocated but unused entries?
			if (OggPack_Read(opb, 1))
			{
				// Yes, unused entries
				for (i = 0; i < s->entries; i++)
				{
					if (OggPack_Read(opb, 1))
					{
						int32 num = OggPack_Read(opb, 5);
						if (num == -1)
							goto _eofout;

						s->lengthList[i] = num + 1;
					}
					else
						s->lengthList[i] = 0;
				}
			}
			else
			{
				// All entries used; no tagging
				for (i = 0; i < s->entries; i++)
				{
					int32 num = OggPack_Read(opb, 5);
					if (num == -1)
						goto _eofout;

					s->lengthList[i] = num + 1;
				}
			}
			break;
		}

		// Ordered
		case 1:
		{
			int32 length = OggPack_Read(opb, 5) + 1;
			s->lengthList = (int32 *)_ogg_malloc(sizeof(*s->lengthList) * s->entries);

			for (i = 0; i < s->entries;)
			{
				int32 num = OggPack_Read(opb, ilog(s->entries - i));
				if (num == -1)
					goto _eofout;

				for (j = 0; (j < num) && (i < s->entries); j++, i++)
					s->lengthList[i] = length;

				length++;
			}
			break;
		}

		// EOF
		default:
			return (-1);
	}

	// Do we have a mapping to unpack?
	switch ((s->mapType = OggPack_Read(opb, 4)))
	{
		// No mapping
		case 0:
			break;

		// Implicitly populated value mapping
		// Explicitly populated value mapping
		case 1:
		case 2:
		{
			s->qMin       = OggPack_Read(opb, 32);
			s->qDelta     = OggPack_Read(opb, 32);
			s->qQuant     = OggPack_Read(opb, 4) + 1;
			s->qSequenceP = OggPack_Read(opb, 1);

			{
				int32 quantVals = 0;

				switch (s->mapType)
				{
					case 1:
					{
						quantVals = _Book_MapType1_QuantVals(s);
						break;
					}

					case 2:
					{
						quantVals = s->entries * s->dim;
						break;
					}
				}

				// Quantized values
				s->quantList = (int32 *)_ogg_malloc(sizeof(*s->quantList) * quantVals);
				for (i = 0; i < quantVals; i++)
					s->quantList[i] = OggPack_Read(opb, s->qQuant);

				if (quantVals && (s->quantList[quantVals - 1] == -1))
					goto _eofout;
			}
			break;
		}

		default:
			goto _errout;
	}

	// All set
	return (0);

_errout:
_eofout:
	Vorbis_Staticbook_Clear(s);
	return (-1);
}



/******************************************************************************/
/* Vorbis_Book_Decode() returns the entry number or -1 on EOF.                */
/*                                                                            */
/* Input:  "book" is a pointer to the book to work on.                        */
/*         "b" is a pointer to the packet buffer.                             */
/*                                                                            */
/* Output: The entry number or -1 for EOF.                                    */
/******************************************************************************/
int32 OggVorbis::Vorbis_Book_Decode(Codebook *book, OggPackBuffer *b)
{
	int32 packedEntry = DecodePackedEntryNumber(book, b);
	if (packedEntry >= 0)
		return (book->decIndex[packedEntry]);

	// If there's no decIndex, the codebook unpacking isn't collapsed
	return (packedEntry);
}



/******************************************************************************/
/* Vorbis_Book_DecodeVSet()                                                   */
/******************************************************************************/
int32 OggVorbis::Vorbis_Book_DecodeVSet(Codebook *book, float *a, OggPackBuffer *b, int32 n)
{
	int32 i, j, entry;
	float *t;

	for (i = 0; i < n;)
	{
		entry = DecodePackedEntryNumber(book, b);
		if (entry == -1)
			return (-1);

		t = book->valueList + entry * book->dim;

		for (j = 0; j < book->dim;)
			a[i++] = t[j++];
	}

	return (0);
}



/******************************************************************************/
/* Vorbis_Book_DecodeVsAdd()                                                  */
/******************************************************************************/
int32 OggVorbis::Vorbis_Book_DecodeVsAdd(OggVorbis *obj, Codebook *book, float *a, OggPackBuffer *b, int32 n)
{
	int32 step = n / book->dim;
	int32 *entry = new int32[step];
	float **t    = new float *[step];
	int32 i, j, o;

	for (i = 0; i < step; i++)
	{
		entry[i] = obj->DecodePackedEntryNumber(book, b);
		if (entry[i] == -1)
		{
			delete[] t;
			delete[] entry;

			return (-1);
		}

		t[i] = book->valueList + entry[i] * book->dim;
	}

	for (i = 0, o = 0; i < book->dim; i++, o += step)
	{
		for (j = 0; j < step; j++)
			a[o + j] += t[j][i];
	}

	delete[] t;
	delete[] entry;

	return (0);
}



/******************************************************************************/
/* Vorbis_Book_DecodeVAdd()                                                   */
/******************************************************************************/
int32 OggVorbis::Vorbis_Book_DecodeVAdd(OggVorbis *obj, Codebook *book, float *a, OggPackBuffer *b, int32 n)
{
	int32 i, j, entry;
	float *t;

	if (book->dim > 8)
	{
		for (i = 0; i < n;)
		{
			entry = obj->DecodePackedEntryNumber(book, b);
			if (entry == -1)
				return (-1);

			t = book->valueList + entry * book->dim;

			for (j = 0; j < book->dim;)
				a[i++] += t[j++];
		}
	}
	else
	{
		for (i = 0; i < n;)
		{
			entry = obj->DecodePackedEntryNumber(book, b);
			if (entry == -1)
				return (-1);

			t = book->valueList + entry * book->dim;
			j = 0;

			switch (book->dim)
			{
				case 8:
					a[i++] += t[j++];

				case 7:
					a[i++] += t[j++];

				case 6:
					a[i++] += t[j++];

				case 5:
					a[i++] += t[j++];

				case 4:
					a[i++] += t[j++];

				case 3:
					a[i++] += t[j++];

				case 2:
					a[i++] += t[j++];

				case 1:
					a[i++] += t[j++];

				case 0:
					break;
			}
		}
	}

	return (0);
}



/******************************************************************************/
/* Vorbis_Book_DecodeVvAdd()                                                  */
/******************************************************************************/
int32 OggVorbis::Vorbis_Book_DecodeVvAdd(OggVorbis *obj, Codebook *book, float **a, int32 offset, int32 ch, OggPackBuffer *b, int32 n)
{
	int32 i, j, entry;
	int32 chPtr = 0;

	for (i = offset / ch; i < (offset + n) / ch;)
	{
		entry = obj->DecodePackedEntryNumber(book, b);
		if (entry == -1)
			return (-1);

		{
			const float *t = book->valueList + entry * book->dim;
			for (j = 0; j < book->dim; j++)
			{
				a[chPtr++][i] += t[j];
				if (chPtr == ch)
				{
					chPtr = 0;
					i++;
				}
			}
		}
	}

	return (0);
}



/******************************************************************************/
/* BitReverse()                                                               */
/*                                                                            */
/* The 'eliminate the decode tree' optimization actually requires the         */
/* codewords to be MSb first, not LSb. This is an annoying inelegancy (and    */
/* one of the first places where carefully thought out design turned out to   */
/* be wrong; Vorbis II and future Ogg codecs should go to an MSb bitpacker),  */
/* but not actually the huge hit it appears to be. The first-stage decode     */
/* table catches most words so that bitreverse is not in the main execution   */
/* path.                                                                      */
/******************************************************************************/
uint32 OggVorbis::BitReverse(uint32 x)
{
	x =     ((x >> 16) & 0x0000ffff) | ((x << 16) & 0xffff0000);
	x =     ((x >>  8) & 0x00ff00ff) | ((x <<  8) & 0xff00ff00);
	x =     ((x >>  4) & 0x0f0f0f0f) | ((x <<  4) & 0xf0f0f0f0);
	x =     ((x >>  2) & 0x33333333) | ((x <<  2) & 0xcccccccc);
	return (((x >>  1) & 0x55555555) | ((x <<  1) & 0xaaaaaaaa));
}



/******************************************************************************/
/* DecodePackedEntryNumber()                                                  */
/******************************************************************************/
int32 OggVorbis::DecodePackedEntryNumber(Codebook *book, OggPackBuffer *b)
{
	int32 read = book->decMaxLength;
	int32 lo, hi;
	int32 lOk = OggPack_Look(b, book->decFirstTabLen);

	if (lOk >= 0)
	{
		int32 entry = book->decFirstTable[lOk];
		if (entry & 0x80000000UL)
		{
			lo = (entry >> 15) & 0x7fff;
			hi = book->usedEntries - (entry & 0x7fff);
		}
		else
		{
			OggPack_Adv(b, book->decCodeLengths[entry - 1]);
			return (entry - 1);
		}
	}
	else
	{
		lo = 0;
		hi = book->usedEntries;
	}

	lOk = OggPack_Look(b, read);

	while ((lOk < 0) && (read > 1))
		lOk = OggPack_Look(b, --read);

	if (lOk < 0)
		return (-1);

	// Bisect search for the codeword in the ordered list
	{
		uint32 testWord = BitReverse((uint32)lOk);

		while ((hi - lo) > 1)
		{
			int32 p = (hi - lo) >> 1;
			int32 test = book->codeList[lo + p] > testWord;
			lo += p & (test - 1);
			hi -= p & (-test);
		}

		if (book->decCodeLengths[lo] <= read)
		{
			OggPack_Adv(b, book->decCodeLengths[lo]);
			return (lo);
		}
	}

	OggPack_Adv(b, read);
	return (-1);
}
