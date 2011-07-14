/******************************************************************************/
/* MikMod I/O interface.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Player headers
#include "MikMod.h"


/******************************************************************************/
/* AllocPositions() allocates enough memory to hold the positions.            */
/*                                                                            */
/* Input:  "total" is the total number of positions to allocate.              */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikMod::AllocPositions(int32 total)
{
	if ((of.positions = new uint16[total]) == NULL)
		return (false);

	// Clear the allocated memory
	memset(of.positions, 0, sizeof(uint16) * total);

	return (true);
}



/******************************************************************************/
/* AllocPatterns() allocates enough memory to hold the patterns.              */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikMod::AllocPatterns(void)
{
	int32 s, t, tracks = 0;

	// Allocate track sequencing array
	if (!(of.patterns = new uint16[(of.numPat + 1) * of.numChn]))
		return (false);

	if (!(of.pattRows = new uint16[of.numPat + 1]))
		return (false);

	// Clear the arrays
	memset(of.patterns, 0, sizeof(uint16) * (of.numPat + 1) * of.numChn);
	memset(of.pattRows, 0, sizeof(uint16) * (of.numPat + 1));

	// Initialize the patterns to default values
	for (t = 0; t < of.numPat + 1; t++)
	{
		of.pattRows[t] = 64;
		for(s = 0; s < of.numChn; s++)
			of.patterns[(t * of.numChn) + s] = tracks++;
	}

	return (true);
}



/******************************************************************************/
/* AllocTracks() allocates enough memory to hold the tracks.                  */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikMod::AllocTracks(void)
{
	if (!(of.tracks = new uint8 *[of.numTrk]))
		return (false);

	// Clear the array
	memset(of.tracks, 0, sizeof(uint8 *) * of.numTrk);

	return (true);
}



/******************************************************************************/
/* AllocSamples() allocates enough memory to hold the sample info.            */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikMod::AllocSamples(void)
{
	uint16 u;

    if ((of.samples = new SAMPLE[of.numSmp]) == NULL)
		return (false);

	for (u = 0; u < of.numSmp; u++)
	{
		// Initialize the structure
		of.samples[u].speed     = 0;
		of.samples[u].volume    = 64;
		of.samples[u].panning   = 128;
		of.samples[u].length    = 0;
		of.samples[u].loopStart = 0;
		of.samples[u].loopEnd   = 0;
		of.samples[u].susBegin  = 0;
		of.samples[u].susEnd    = 0;
		of.samples[u].flags     = 0;
		of.samples[u].globVol   = 64;
		of.samples[u].vibFlags  = 0;
		of.samples[u].vibType   = 0;
		of.samples[u].vibSweep  = 0;
		of.samples[u].vibDepth  = 0;
		of.samples[u].vibRate   = 0;
		of.samples[u].aVibPos   = 0;
		of.samples[u].seekPos   = 0;
		of.samples[u].handle    = NULL;
	}

	return (true);
}



/******************************************************************************/
/* AllocInstruments() allocates enough memory to hold the instrument info.    */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikMod::AllocInstruments(void)
{
	int32 t, n;

	if ((of.instruments = new INSTRUMENT[of.numIns]) == NULL)
		return (false);

	for (t = 0; t < of.numIns; t++)
	{
		for (n = 0; n < 120; n++)	// Init note / sample lookup table
		{
			of.instruments[t].sampleNote[n]   = n;
			of.instruments[t].sampleNumber[n] = t;
		}

		of.instruments[t].flags        = 0;
		of.instruments[t].nnaType      = 0;
		of.instruments[t].dca          = 0;
		of.instruments[t].dct          = 0;
		of.instruments[t].globVol      = 64;
		of.instruments[t].panning      = 0;
		of.instruments[t].pitPanSep    = 0;
		of.instruments[t].pitPanCenter = 0;
		of.instruments[t].rVolVar      = 0;
		of.instruments[t].rPanVar      = 0;
		of.instruments[t].volFade      = 0;
		of.instruments[t].volFlg       = 0;
		of.instruments[t].volPts       = 0;
		of.instruments[t].volSusBeg    = 0;
		of.instruments[t].volSusEnd    = 0;
		of.instruments[t].volBeg       = 0;
		of.instruments[t].volEnd       = 0;
		of.instruments[t].panFlg       = 0;
		of.instruments[t].panPts       = 0;
		of.instruments[t].panSusBeg    = 0;
		of.instruments[t].panSusEnd    = 0;
		of.instruments[t].panBeg       = 0;
		of.instruments[t].panEnd       = 0;
		of.instruments[t].pitFlg       = 0;
		of.instruments[t].pitPts       = 0;
		of.instruments[t].pitSusBeg    = 0;
		of.instruments[t].pitSusEnd    = 0;
		of.instruments[t].pitBeg       = 0;
		of.instruments[t].pitEnd       = 0;
		memset(of.instruments[t].volEnv, 0, 32 * sizeof(ENVPT));
		memset(of.instruments[t].panEnv, 0, 32 * sizeof(ENVPT));
		memset(of.instruments[t].pitEnv, 0, 32 * sizeof(ENVPT));
	}

	return (true);
}



/******************************************************************************/
/* FreeAll() frees all memory allocated.                                      */
/******************************************************************************/
void MikMod::FreeAll(void)
{
	uint16 t;

	// Free the position structure
	delete[] of.positions;
	of.positions = NULL;

	// Free the patterns
	delete[] of.patterns;
	delete[] of.pattRows;
	of.patterns = NULL;
	of.pattRows = NULL;

	// Free the tracks
	if (of.tracks != NULL)
	{
		for (t = 0; t < of.numTrk; t++)
			delete[] of.tracks[t];
	}

	delete[] of.tracks;
	of.tracks = NULL;

	// Free the instruments
	delete[] of.instruments;
	of.instruments = NULL;

	// Free the sample data
	if (of.samples != NULL)
	{
		for (t = 0; t < of.numSmp; t++)
			delete[] of.samples[t].handle;
	}

	// Free the sample structures
	delete[] of.samples;
	of.samples = NULL;
}
