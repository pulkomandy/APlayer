/******************************************************************************/
/* Codebook header file.                                                      */
/******************************************************************************/


#ifndef __Codebook_h
#define __Codebook_h

//
// This structure encapsulates huffman and VQ style encoding books; it
// doesn't do anything specific to either.
//
// valueList/quantList are non NULL (and q_* significant) only if
// there's entry->value mapping to be done.
//
// If encode-side mapping must be done (and thus the entry need to be
// hunted), the auxiliary encode pointer will point to a decision
// tree. This is true of both VQ and huffman, but is mostly useful
// with VQ.
//
typedef struct StaticCodebook
{
	int32 dim;				// Codebook dimensions (elements per vector)
	int32 entries;			// Codebook entries
	int32 *lengthList;		// Codeword length in bits

	// Mapping
	int32 mapType;			// 0 = None
							// 1 = Implicitly populated values from map column
							// 2 = Listed arbitrary values

	// The below does a linear, single monotonic sequence mapping
	int32 qMin;				// Packed 32 bit float; quant value 0 maps to minVal
	int32 qDelta;			// Packed 32 bit float; val 1 - val 0 == delta
	int32 qQuant;			// Bits: 0 < quant <= 16
	bool qSequenceP;		// Bitflag

	int32 *quantList;		// Map == 1: (int)(entries^(1/dim)) element column map
							// Map == 2: List of dim*entries quantized entry vals

	bool allocedP;
} StaticCodebook;



typedef struct Codebook
{
	int32 dim;				// Codebook dimensions (elements per vector)
	int32 entries;			// Codebook entries
	int32 usedEntries;		// Populated codebook entries
	const StaticCodebook *c;

	// For encode, the below are entry-ordered, fully populated.
	// For decode, the below are ordered by bitreversed codeword and only
	// used entries are populated
	float *valueList;		// List of dim*entries actual entry values
	uint32 *codeList;		// List of bitstream codewords for each entry

	int32 *decIndex;		// Only used if sparseness collapsed
	int8 *decCodeLengths;
	uint32 *decFirstTable;
	int32 decFirstTabLen;
	int32 decMaxLength;
} Codebook;

#endif
