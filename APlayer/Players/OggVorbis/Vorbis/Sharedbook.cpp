/******************************************************************************/
/* Vorbis sharedbook functions.                                               */
/******************************************************************************/


// Player headers
#include "OggVorbis.h"


/******************************************************************************/
/* Vorbis_Staticbook_Clear() clear non-flat storage within.                   */
/*                                                                            */
/* Input:  "b" is a pointer to the codebook to clear.                         */
/******************************************************************************/
void OggVorbis::Vorbis_Staticbook_Clear(StaticCodebook *b)
{
	if (b->allocedP)
	{
		_ogg_free(b->quantList);
		_ogg_free(b->lengthList);

		memset(b, 0, sizeof(*b));
	}
}



/******************************************************************************/
/* Vorbis_Staticbook_Destroy() destroys the book given.                       */
/*                                                                            */
/* Input:  "b" is a pointer to the codebook to destroy.                       */
/******************************************************************************/
void OggVorbis::Vorbis_Staticbook_Destroy(StaticCodebook *b)
{
	if (b->allocedP)
	{
		Vorbis_Staticbook_Clear(b);
		_ogg_free(b);
	}
}



/******************************************************************************/
/* Vorbis_Book_InitDecode() initialize the codebook for decoding.             */
/*                                                                            */
/* Input:  "c" is a pointer to the codebook to initialize.                    */
/*         "s" is a pointer to the static codebook.                           */
/*                                                                            */
/* Output: 0 for success, -1 for failure.                                     */
/******************************************************************************/
int32 OggVorbis::Vorbis_Book_InitDecode(Codebook *c, const StaticCodebook *s)
{
	int32 i, j, n = 0, tabN;
	int32 *sortIndex;

	memset(c, 0, sizeof(*c));

	// Count actually used entries
	for (i = 0; i < s->entries; i++)
	{
		if (s->lengthList[i] > 0)
			n++;
	}

	c->entries     = s->entries;
	c->usedEntries = n;
	c->dim         = s->dim;

	// Two different remappings go on here
	//
	// First, we collapse the likely sparse codebook down only to
	// actually represented values/words. This collapsing needs to be
	// indexed as map-valueless books are used to encode original entry
	// positions as integers.
	//
	// Second, we reorder all vectors, including the entry index above,
	// by sorted bitreversed codeword to allow treeless decode
	{
		// Perform sort
		uint32 *codes  = _MakeWords(s->lengthList, s->entries, c->usedEntries);
		uint32 **codeP = new uint32 *[n];

		if (codes == NULL)
		{
			delete[] codeP;
			goto err_out;
		}

		for (i = 0; i < n; i++)
		{
			codes[i] = BitReverse(codes[i]);
			codeP[i] = codes + i;
		}

		qsort(codeP, n, sizeof(*codeP), Sort32a);

		sortIndex = new int32[n];
		c->codeList = (uint32 *)_ogg_malloc(n * sizeof(*c->codeList));

		// The index is a reverse index
		for (i = 0; i < n; i++)
		{
			int32 position = codeP[i] - codes;
			sortIndex[position] = i;
		}

		for (i = 0; i < n; i++)
			c->codeList[sortIndex[i]] = codes[i];

		_ogg_free(codes);
		delete[] codeP;
	}

	c->valueList = _Book_Unquantize(s, n, sortIndex);
	c->decIndex  = (int32 *)_ogg_malloc(n * sizeof(*c->decIndex));

	for (n = 0, i = 0; i < s->entries; i++)
	{
		if (s->lengthList[i] > 0)
			c->decIndex[sortIndex[n++]] = i;
	}

	c->decCodeLengths = (int8 *)_ogg_malloc(n * sizeof(*c->decCodeLengths));
	for (n = 0, i = 0; i < s->entries; i++)
	{
		if (s->lengthList[i] > 0)
			c->decCodeLengths[sortIndex[n++]] = s->lengthList[i];
	}

	c->decFirstTabLen = ilog(c->usedEntries) - 4;	// This is magic
	if (c->decFirstTabLen < 5)
		c->decFirstTabLen = 5;

	if (c->decFirstTabLen > 8)
		c->decFirstTabLen = 8;

	tabN = 1 << c->decFirstTabLen;
	c->decFirstTable = (uint32 *)_ogg_calloc(tabN, sizeof(*c->decFirstTable));
	c->decMaxLength  = 0;

	for (i = 0; i < n; i++)
	{
		if (c->decMaxLength < c->decCodeLengths[i])
			c->decMaxLength = c->decCodeLengths[i];

		if (c->decCodeLengths[i] <= c->decFirstTabLen)
		{
			uint32 orig = BitReverse(c->codeList[i]);

			for (j = 0; j < (1 << (c->decFirstTabLen - c->decCodeLengths[i])); j++)
				c->decFirstTable[orig | (j << c->decCodeLengths[i])] = i + 1;
		}
	}

	// Now fill in 'unused' entries in the firsttable with hi/lo search
	// hints for the non-direct-hits
	{
		uint32 mask = 0xfffffffeUL << (31 - c->decFirstTabLen);
		int32 lo = 0, hi = 0;

		for (i = 0; i < tabN; i++)
		{
			uint32 word = i << (32 - c->decFirstTabLen);
			if (c->decFirstTable[BitReverse(word)] == 0)
			{
				while (((lo + 1) < n) && (c->codeList[lo + 1] <= word))
					lo++;

				while ((hi < n) && (word >= (c->codeList[hi] & mask)))
					hi++;

				// We only actually have 15 bits per hint to play with here.
				// In order to overflow gravefully (nothing breaks, efficiency
				// just drops), encode as the difference from the extremes
				{
					uint32 loVal = lo;
					uint32 hiVal = n - hi;

					if (loVal > 0x7fff)
						loVal = 0x7fff;

					if (hiVal > 0x7fff)
						hiVal = 0x7fff;

					c->decFirstTable[BitReverse(word)] = 0x80000000UL | (loVal << 15) | hiVal;
				}
			}
		}
	}

	delete[] sortIndex;

	return (0);

err_out:
	Vorbis_Book_Clear(c);
	return (-1);
}



/******************************************************************************/
/* Vorbis_Book_Clear() frees all intern buffers.                              */
/*                                                                            */
/* Input:  "b" is a pointer to the codebook to clear.                         */
/******************************************************************************/
void OggVorbis::Vorbis_Book_Clear(Codebook *b)
{
	// Static book is not cleared; we're likely called on the lookup and
	// the static codebook belongs to the info struct
	_ogg_free(b->valueList);

	_ogg_free(b->decIndex);
	_ogg_free(b->decCodeLengths);
	_ogg_free(b->decFirstTable);

	memset(b, 0, sizeof(*b));
}



/******************************************************************************/
/* _Book_MapType1_QuantVals()                                                 */
/*                                                                            */
/* There might be a straighforward one-line way to do the below that's        */
/* portable and totally safe against roundoff, but I haven't through of it.   */
/* Therefore, we opt on the side of caution.                                  */
/******************************************************************************/
int32 OggVorbis::_Book_MapType1_QuantVals(const StaticCodebook *b)
{
	int32 vals = (int32)floor(pow((float)b->entries, 1.0f / b->dim));

	// The above *should* be reliable, but we'll not assume that FP is
	// ever reliable when bitstream sync is at stake; verify via integer
	// means that vals really is the greatest value of dim for which
	// vals^b->dim <= b->entries
	// Threat the above as an initial guess
	while (true)
	{
		int32 acc = 1;
		int32 acc1 = 1;
		int32 i;

		for (i = 0; i < b->dim; i++)
		{
			acc  *= vals;
			acc1 *= vals + 1;
		}

		if ((acc <= b->entries) && (acc1 > b->entries))
			return (vals);
		else
		{
			if (acc > b->entries)
				vals--;
			else
				vals++;
		}
	}
}



/******************************************************************************/
/* _Book_Unquantize() unpacks the quantized list of values for encode/decode. */
/*                                                                            */
/* We need to deal with two map types: in map type 1, the values are          */
/* generated algorithmically (each column of the vector counts through the    */
/* values in the quant vector). In map type 2, all the values came in an      */
/* explicit list. Both value lists must be unpacked.                          */
/******************************************************************************/
float *OggVorbis::_Book_Unquantize(const StaticCodebook *b, int32 n, int32 *sparseMap)
{
	int32 j, k, count = 0;

	if ((b->mapType == 1) || (b->mapType == 2))
	{
		int32 quantVals;
		float minDel = _Float32_Unpack(b->qMin);
		float delta  = _Float32_Unpack(b->qDelta);
		float *r     = (float *)_ogg_calloc(n * b->dim, sizeof(*r));

		// Maptype 1 and 2 both use a quantized value vector, but different sizes
		switch (b->mapType)
		{
			case 1:
			{
				// Most of the time, entries % dimensions == 0, but we need to be
				// well defined. We define that the possible vales at each
				// scalar is values == entries / dim. If entries % dim != 0, we'll
				// have 'too few' values (values * dim < entries), which means that
				// we'll have 'left over' entries; left over entries use zeroed
				// values (and are wasted). So don't generate codebooks like that
				quantVals = _Book_MapType1_QuantVals(b);

				for (j = 0; j < b->entries; j++)
				{
					if ((sparseMap && b->lengthList[j]) || !sparseMap)
					{
						float last = 0.0f;
						int32 indexDiv = 1;

						for (k = 0; k < b->dim; k++)
						{
							int32 index = (j / indexDiv) % quantVals;
							float val = b->quantList[index];
							val = fabs(val) * delta + minDel + last;

							if (b->qSequenceP)
								last = val;

							if (sparseMap)
								r[sparseMap[count] * b->dim + k] = val;
							else
								r[count * b->dim + k] = val;

							indexDiv *= quantVals;
						}

						count++;
					}
				}
				break;
			}

			case 2:
			{
				for (j = 0; j < b->entries; j++)
				{
					if ((sparseMap && b->lengthList[j]) || !sparseMap)
					{
						float last = 0.0f;

						for (k = 0; k < b->dim; k++)
						{
							float val = b->quantList[j * b->dim + k];
							val = fabs(val) * delta + minDel + last;

							if (b->qSequenceP)
								last = val;

							if (sparseMap)
								r[sparseMap[count] * b->dim + k] = val;
							else
								r[count * b->dim + k] = val;
						}

						count++;
					}
				}
				break;
			}
		}

		return (r);
	}

	return (NULL);
}



/******************************************************************************/
/* _MakeWords() given a list of word lengths, generate a list of codewords.   */
/*      Works for length ordered or unordered, always assigns the lowest      */
/*      valued codeword first. Extended to handle unused entries (length 0).  */
/******************************************************************************/
uint32 *OggVorbis::_MakeWords(int32 *l, int32 n, int32 sparseCount)
{
	int32 i, j, count = 0;
	uint32 marker[33];
	uint32 *r = (uint32 *)_ogg_malloc((sparseCount ? sparseCount : n) * sizeof(*r));
	memset(marker, 0, sizeof(marker));

	for (i = 0; i < n; i++)
	{
		int32 length = l[i];
		if (length > 0)
		{
			uint32 entry = marker[length];

			// When we claim a node for an entry, we also claim the nodes
			// below it (pruning off the imagined tree that may have dangled
			// from it) as well as blocking the use of any nodes directly
			// above for leaves
			//
			// Update ourself
			if ((length < 32) && (entry >> length))
			{
				// Error condition; the lengths must specify an overpopulated tree
				_ogg_free(r);
				return (NULL);
			}

			r[count++] = entry;

			// Look to see if the next shorter marker points to the node
			// above. If so, update it and repeat
			{
				for (j = length; j > 0; j--)
				{
					if (marker[j] & 1)
					{
						// Have to jump branches
						if (j == 1)
							marker[1]++;
						else
							marker[j] = marker[j - 1] << 1;

						// Invariant says next upper marker would already
						// have been moved if it was on the same path
						break;
					}

					marker[j]++;
				}
			}

			// Prune the tree; the implicit invariant says all the longer
			// markers were dangling from our just-taken node. Dangle them
			// from our *new* node
			for (j = length + 1; j < 33; j++)
			{
				if ((marker[j] >> 1) == entry)
				{
					entry = marker[j];
					marker[j] = marker[j - 1] << 1;
				}
				else
					break;
			}
		}
		else
		{
			if (sparseCount == 0)
				count++;
		}
	}

	// Bitreverse the words because our bitwise packer/unpacker is LSb endian
	for (i = 0, count = 0; i < n; i++)
	{
		uint32 temp = 0;

		for (j = 0; j < l[i]; j++)
		{
			temp <<= 1;
			temp  |= (r[count] >> j) & 1;
		}

		if (sparseCount)
		{
			if (l[i])
				r[count++] = temp;
		}
		else
			r[count++] = temp;
	}

	return (r);
}



/******************************************************************************/
/* 32 bit float (not IEEE; nonnormalized mantissa + biased exponent):         */
/* neeeeeee eeemmmmm mmmmmmmm mmmmmmmm                                        */
/******************************************************************************/
#define VQ_FMAN				21
#define VQ_FEXP_BIAS		768		// Bias toward values smaller than 1



/******************************************************************************/
/* _Float32_Unpack() unpacks a 32 bit float to real float.                    */
/******************************************************************************/
float OggVorbis::_Float32_Unpack(int32 val)
{
	double mant = val & 0x1fffff;
	int32 sign  = val & 0x80000000;
	int32 exp   = (val & 0x7fe00000L) >> VQ_FMAN;

	if (sign)
		mant = -mant;

	return (ldexp(mant, exp - (VQ_FMAN - 1) - VQ_FEXP_BIAS));
}



/******************************************************************************/
/* Sort32a()                                                                  */
/******************************************************************************/
int OggVorbis::Sort32a(const void *a, const void *b)
{
	return (((**(uint32 **)a > **(uint32 **)b) << 1) - 1);
}
