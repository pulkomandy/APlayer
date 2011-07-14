/******************************************************************************/
/* MikConverter Converter class.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PAlert.h"

// Agent headers
#include "MikConverter.h"
#include "MikMod_Internals.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Other defines                                                              */
/******************************************************************************/
#define HIGH_OCTAVE			2		// Number of above-range octaves



/******************************************************************************/
/* Tables                                                                     */
/******************************************************************************/
// ** Triton's linear periods to frequency translation table (for
// ** Fast Tracker 2 [XM] modules):
uint32 linTab[768] =
{
	535232,534749,534266,533784,533303,532822,532341,531861,
	531381,530902,530423,529944,529466,528988,528511,528034,
	527558,527082,526607,526131,525657,525183,524709,524236,
	523763,523290,522818,522346,521875,521404,520934,520464,
	519994,519525,519057,518588,518121,517653,517186,516720,
	516253,515788,515322,514858,514393,513929,513465,513002,
	512539,512077,511615,511154,510692,510232,509771,509312,
	508852,508393,507934,507476,507018,506561,506104,505647,
	505191,504735,504280,503825,503371,502917,502463,502010,
	501557,501104,500652,500201,499749,499298,498848,498398,
	497948,497499,497050,496602,496154,495706,495259,494812,
	494366,493920,493474,493029,492585,492140,491696,491253,
	490809,490367,489924,489482,489041,488600,488159,487718,
	487278,486839,486400,485961,485522,485084,484647,484210,
	483773,483336,482900,482465,482029,481595,481160,480726,
	480292,479859,479426,478994,478562,478130,477699,477268,
	476837,476407,475977,475548,475119,474690,474262,473834,
	473407,472979,472553,472126,471701,471275,470850,470425,
	470001,469577,469153,468730,468307,467884,467462,467041,
	466619,466198,465778,465358,464938,464518,464099,463681,
	463262,462844,462427,462010,461593,461177,460760,460345,
	459930,459515,459100,458686,458272,457859,457446,457033,
	456621,456209,455797,455386,454975,454565,454155,453745,
	453336,452927,452518,452110,451702,451294,450887,450481,
	450074,449668,449262,448857,448452,448048,447644,447240,
	446836,446433,446030,445628,445226,444824,444423,444022,
	443622,443221,442821,442422,442023,441624,441226,440828,
	440430,440033,439636,439239,438843,438447,438051,437656,
	437261,436867,436473,436079,435686,435293,434900,434508,
	434116,433724,433333,432942,432551,432161,431771,431382,
	430992,430604,430215,429827,429439,429052,428665,428278,
	427892,427506,427120,426735,426350,425965,425581,425197,
	424813,424430,424047,423665,423283,422901,422519,422138,
	421757,421377,420997,420617,420237,419858,419479,419101,
	418723,418345,417968,417591,417214,416838,416462,416086,
	415711,415336,414961,414586,414212,413839,413465,413092,
	412720,412347,411975,411604,411232,410862,410491,410121,
	409751,409381,409012,408643,408274,407906,407538,407170,
	406803,406436,406069,405703,405337,404971,404606,404241,
	403876,403512,403148,402784,402421,402058,401695,401333,
	400970,400609,400247,399886,399525,399165,398805,398445,
	398086,397727,397368,397009,396651,396293,395936,395579,
	395222,394865,394509,394153,393798,393442,393087,392733,
	392378,392024,391671,391317,390964,390612,390259,389907,
	389556,389204,388853,388502,388152,387802,387452,387102,
	386753,386404,386056,385707,385359,385012,384664,384317,
	383971,383624,383278,382932,382587,382242,381897,381552,
	381208,380864,380521,380177,379834,379492,379149,378807,

	378466,378124,377783,377442,377102,376762,376422,376082,
	375743,375404,375065,374727,374389,374051,373714,373377,
	373040,372703,372367,372031,371695,371360,371025,370690,
	370356,370022,369688,369355,369021,368688,368356,368023,
	367691,367360,367028,366697,366366,366036,365706,365376,
	365046,364717,364388,364059,363731,363403,363075,362747,
	362420,362093,361766,361440,361114,360788,360463,360137,
	359813,359488,359164,358840,358516,358193,357869,357547,
	357224,356902,356580,356258,355937,355616,355295,354974,
	354654,354334,354014,353695,353376,353057,352739,352420,
	352103,351785,351468,351150,350834,350517,350201,349885,
	349569,349254,348939,348624,348310,347995,347682,347368,
	347055,346741,346429,346116,345804,345492,345180,344869,
	344558,344247,343936,343626,343316,343006,342697,342388,
	342079,341770,341462,341154,340846,340539,340231,339924,
	339618,339311,339005,338700,338394,338089,337784,337479,
	337175,336870,336566,336263,335959,335656,335354,335051,
	334749,334447,334145,333844,333542,333242,332941,332641,
	332341,332041,331741,331442,331143,330844,330546,330247,
	329950,329652,329355,329057,328761,328464,328168,327872,
	327576,327280,326985,326690,326395,326101,325807,325513,
	325219,324926,324633,324340,324047,323755,323463,323171,
	322879,322588,322297,322006,321716,321426,321136,320846,
	320557,320267,319978,319690,319401,319113,318825,318538,
	318250,317963,317676,317390,317103,316817,316532,316246,
	315961,315676,315391,315106,314822,314538,314254,313971,
	313688,313405,313122,312839,312557,312275,311994,311712,
	311431,311150,310869,310589,310309,310029,309749,309470,
	309190,308911,308633,308354,308076,307798,307521,307243,
	306966,306689,306412,306136,305860,305584,305308,305033,
	304758,304483,304208,303934,303659,303385,303112,302838,
	302565,302292,302019,301747,301475,301203,300931,300660,
	300388,300117,299847,299576,299306,299036,298766,298497,
	298227,297958,297689,297421,297153,296884,296617,296349,
	296082,295815,295548,295281,295015,294749,294483,294217,
	293952,293686,293421,293157,292892,292628,292364,292100,
	291837,291574,291311,291048,290785,290523,290261,289999,
	289737,289476,289215,288954,288693,288433,288173,287913,
	287653,287393,287134,286875,286616,286358,286099,285841,
	285583,285326,285068,284811,284554,284298,284041,283785,
	283529,283273,283017,282762,282507,282252,281998,281743,
	281489,281235,280981,280728,280475,280222,279969,279716,
	279464,279212,278960,278708,278457,278206,277955,277704,
	277453,277203,276953,276703,276453,276204,275955,275706,
	275457,275209,274960,274712,274465,274217,273970,273722,
	273476,273229,272982,272736,272490,272244,271999,271753,
	271508,271263,271018,270774,270530,270286,270042,269798,
	269555,269312,269069,268826,268583,268341,268099,267857
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MikConverter::MikConverter(PResource *resource)
{
	// Initialize member variables
	res            = resource;

	posLookup      = NULL;
	origPositions  = NULL;

	noteIndex      = NULL;
	noteIndexCount = 0;

	// Initialize user variables
	curious = false;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MikConverter::~MikConverter(void)
{
}



/******************************************************************************/
/* ConvertModule() will convert the module to UniMod structures.              */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store the information needed.                                      */
/******************************************************************************/
ap_result MikConverter::ConvertModule(APAgent_ConvertModule *convInfo)
{
	int32 t;
	uint8 maxChan = UF_MAXCHAN;
	ap_result retVal;

	// Initialize the UNIMOD
	if (!(UniInit()))
	{
		ShowError(IDS_MIKC_ERR_LOADING_HEADER);
		return (AP_ERROR);
	}

	// Initialize the UNIMOD structure
	of.flags       = 0;
	of.numChn      = 0;
	of.numVoices   = 0;
	of.numPos      = 0;
	of.numPat      = 0;
	of.numIns      = 0;
	of.numSmp      = 0;
	of.instruments = NULL;
	of.samples     = NULL;

	of.repPos      = 0;
	of.initSpeed   = 0;
	of.initTempo   = 0;
	of.bpmLimit    = 33;
	of.initVolume  = 128;

	of.numTrk      = 0;
	of.tracks      = NULL;
	of.patterns    = NULL;
	of.pattRows    = NULL;
	of.positions   = NULL;

	// Init volume and panning array
	for (t = 0; t < UF_MAXCHAN; t++)
		of.chanVol[t] = 64;

	for (t = 0; t < UF_MAXCHAN; t++)
		of.panning[t] = ((t + 1) & 2) ? PAN_RIGHT : PAN_LEFT;

	// Convert the module
	retVal = ModuleConverter(convInfo);

	// If the module doesn't have any specific panning, create a
	// MOD-like panning, with the channels half-separated
	if (!(of.flags & UF_PANNING))
	{
		for (t = 0; t < of.numChn; t++)
			of.panning[t] = ((t + 1) & 2) ? PAN_HALFRIGHT : PAN_HALFLEFT;
	}

	// Find number of channels to use
	if (!(of.flags & UF_NNA) && (of.numChn < maxChan))
		maxChan = of.numChn;
	else
	{
		if ((of.numVoices) && (of.numVoices < maxChan))
			maxChan = of.numVoices;
	}

	if (maxChan < of.numChn)
		of.flags |= UF_NNA;

	of.numVoices = maxChan;

	// Clean up
	UniCleanup();

	return (retVal);
}



/******************************************************************************/
/* FreeAll() frees all memory allocated.                                      */
/******************************************************************************/
void MikConverter::FreeAll(void)
{
	uint16 t;

	// Free the linear array
	FreeLinear();

	// Free the position structure
	delete[] of.positions;
	of.positions = NULL;

	// Free the patterns
	delete[] of.patterns;
	delete[] of.pattRows;
	of.patterns = NULL;
	of.pattRows = NULL;

	// Free the tracks
	if (of.tracks)
	{
		for (t = 0; t < of.numTrk; t++)
			delete[] of.tracks[t];
	}

	delete[] of.tracks;
	of.tracks = NULL;

	// Free the instruments
	delete[] of.instruments;
	of.instruments = NULL;

	// Free the sample structures
	delete[] of.samples;
	of.samples = NULL;
}



/******************************************************************************/
/* ConvertToUniMod() will convert the UNIMOD structures to a real module.     */
/*                                                                            */
/* Input:  "source" is a pointer to the file object to read the data from.    */
/*         "dest" is a pointer to the file object to store the module in.     */
/*                                                                            */
/* Output: AP_OK if the convertion succeed, else AP_ERROR.                    */
/******************************************************************************/
ap_result MikConverter::ConvertToUniMod(PFile *source, PFile *dest)
{
	PCharSet_UTF8 utf8CharSet;
	int32 i, j;

	// Fill in the unimod header
	dest->Write_B_UINT32('APUN');					// Mark
	dest->Write_B_UINT16(0x0106);					// Version
	dest->Write_B_UINT16(of.flags);					// Flags
	dest->Write_UINT8(of.numChn);					// NumChn
	dest->Write_UINT8(of.numVoices);				// NumVoices
	dest->Write_B_UINT16(of.numPos);				// NumPos
	dest->Write_B_UINT16(of.numPat);				// NumPat
	dest->Write_B_UINT16(of.numTrk);				// NumTrk
	dest->Write_B_UINT16(of.numIns);				// NumIns
	dest->Write_B_UINT16(of.numSmp);				// NumSmp
	dest->Write_B_UINT16(of.repPos);				// RepPos
	dest->Write_UINT8(of.initSpeed);				// InitSpeed
	dest->Write_UINT8(of.initTempo);				// InitTempo
	dest->Write_UINT8(of.initVolume);				// InitVolume
	dest->Write_B_UINT16(of.bpmLimit);				// BPMLimit

	// Fill in the strings
	dest->WriteString(of.songName, &utf8CharSet);	// Song name
	dest->WriteString(of.comment, &utf8CharSet);	// Comment

	// Copy pattern positions
	for (i = 0; i < of.numPos; i++)
		dest->Write_B_UINT16(of.positions[i]);

	// Copy panning positions
	for (i = 0; i < of.numChn; i++)
		dest->Write_B_UINT16(of.panning[i]);

	// Copy channel volumes
	for (i = 0; i < of.numChn; i++)
		dest->Write_UINT8(of.chanVol[i]);

	// Copy the sample informations
	for (i = 0; i < of.numSmp; i++)
	{
		SAMPLE *samp = &of.samples[i];

		dest->Write_B_UINT16(samp->flags);			// Flags
		dest->Write_B_UINT32(samp->speed);			// Speed
		dest->Write_UINT8(samp->volume);			// Volume
		dest->Write_B_UINT16(samp->panning);		// Panning
		dest->Write_B_UINT32(samp->length);			// Length
		dest->Write_B_UINT32(samp->loopStart);		// LoopStart
		dest->Write_B_UINT32(samp->loopEnd);		// LoopEnd
		dest->Write_B_UINT32(samp->susBegin);		// SusBegin
		dest->Write_B_UINT32(samp->susEnd);			// SusEnd
		dest->Write_UINT8(samp->globVol);			// GlobVol
		dest->Write_UINT8(samp->vibFlags);			// VibFlags
		dest->Write_UINT8(samp->vibType);			// VibType
		dest->Write_UINT8(samp->vibSweep);			// VibSweep
		dest->Write_UINT8(samp->vibDepth);			// VibDepth
		dest->Write_UINT8(samp->vibRate);			// VibRate
		dest->WriteString(samp->sampleName, &utf8CharSet);	// SampleName
	}

	// Copy the instrument informations
	if (of.flags & UF_INST)
	{
		for (i = 0; i < of.numIns; i++)
		{
			INSTRUMENT *inst = &of.instruments[i];

			dest->Write_UINT8(inst->flags);			// Flags
			dest->Write_UINT8(inst->nnaType);		// NNAType
			dest->Write_UINT8(inst->dca);			// DCA
			dest->Write_UINT8(inst->dct);			// DCT
			dest->Write_UINT8(inst->globVol);		// GlobVol
			dest->Write_B_UINT16(inst->panning);	// Panning
			dest->Write_UINT8(inst->pitPanSep);		// PitPanSep
			dest->Write_UINT8(inst->pitPanCenter);	// PitPanCenter
			dest->Write_UINT8(inst->rVolVar);		// RVolVar
			dest->Write_UINT8(inst->rPanVar);		// RPanVar
			dest->Write_B_UINT16(inst->volFade);	// VolFade

			dest->Write_UINT8(inst->volFlg);		// VolFlg
			dest->Write_UINT8(inst->volPts);		// VolPts
			dest->Write_UINT8(inst->volSusBeg);		// VolSusBeg
			dest->Write_UINT8(inst->volSusEnd);		// VolSusEnd
			dest->Write_UINT8(inst->volBeg);		// VolBeg
			dest->Write_UINT8(inst->volEnd);		// VolEnd

			for (j = 0; j < ENVPOINTS; j++)
			{
				dest->Write_B_UINT16(inst->volEnv[j].pos);
				dest->Write_B_UINT16(inst->volEnv[j].val);
			}

			dest->Write_UINT8(inst->panFlg);		// PanFlg
			dest->Write_UINT8(inst->panPts);		// PanPts
			dest->Write_UINT8(inst->panSusBeg);		// PanSusBeg
			dest->Write_UINT8(inst->panSusEnd);		// PanSusEnd
			dest->Write_UINT8(inst->panBeg);		// PanBeg
			dest->Write_UINT8(inst->panEnd);		// PanEnd

			for (j = 0; j < ENVPOINTS; j++)
			{
				dest->Write_B_UINT16(inst->panEnv[j].pos);
				dest->Write_B_UINT16(inst->panEnv[j].val);
			}

			dest->Write_UINT8(inst->pitFlg);		// PitFlg
			dest->Write_UINT8(inst->pitPts);		// PitPts
			dest->Write_UINT8(inst->pitSusBeg);		// PitSusBeg
			dest->Write_UINT8(inst->pitSusEnd);		// PitSusEnd
			dest->Write_UINT8(inst->pitBeg);		// PitBeg
			dest->Write_UINT8(inst->pitEnd);		// PitEnd

			for (j = 0; j < ENVPOINTS; j++)
			{
				dest->Write_B_UINT16(inst->pitEnv[j].pos);
				dest->Write_B_UINT16(inst->pitEnv[j].val);
			}

			dest->WriteArray_B_UINT16s(inst->sampleNumber, INSTNOTES);	// SampleNumber
			dest->Write(inst->sampleNote, INSTNOTES);		// SampleNote
			dest->WriteString(inst->insName, &utf8CharSet);	// InsName
		}
	}

	// Copy number of rows in each pattern
	for (i = 0; i < of.numPat; i++)
		dest->Write_B_UINT16(of.pattRows[i]);

	// Copy indexes to the tracks
	for (i = 0; i < of.numPat * of.numChn; i++)
		dest->Write_B_UINT16(of.patterns[i]);

	// Copy the tracks
	for (i = 0; i < of.numTrk; i++)
	{
		if (of.tracks[i])
		{
			// Store the length
			uint16 temp = UniTrkLen(of.tracks[i]);

			dest->Write_B_UINT16(temp);
			dest->Write(of.tracks[i], temp);
		}
		else
			dest->Write_B_UINT16(0);
	}

	// Copy the samples
	for (i = 0; i < of.numSmp; i++)
	{
		SAMPLE *samp = &of.samples[i];
		uint32 temp = samp->length;

		if (temp != 0)
		{
			if (samp->seekPos != 0)
				source->Seek(samp->seekPos, PFile::pSeekBegin);

			if (samp->flags & SF_ITPACKED)
			{
				uint16 packLen;
				int32 endOffset = (i == of.numSmp - 1) ? source->GetLength() : of.samples[i + 1].seekPos;

				while (source->GetPosition() < endOffset)
				{
					// Copy the packed length
					packLen = source->Read_L_UINT16();
					dest->Write_L_UINT16(packLen);

					// Copy the sample
					uint8 *tempBuffer = new uint8[packLen];

					source->Read(tempBuffer, packLen);
					dest->Write(tempBuffer, packLen);

					delete[] tempBuffer;
				}
			}
			else
			{
				if (samp->flags & SF_STEREO)
					temp *= 2;

				if (samp->flags & SF_16BITS)
					temp *= 2;

				// Copy the sample
				uint8 *tempBuffer = new uint8[temp];

				source->Read(tempBuffer, temp);
				dest->Write(tempBuffer, temp);

				delete[] tempBuffer;
			}
		}
	}

	return (AP_OK);
}



/******************************************************************************/
/* ShowError() opens a message window and show the error.                     */
/*                                                                            */
/* Input:  "id" is the string id to show.                                     */
/******************************************************************************/
void MikConverter::ShowError(int32 id)
{
	PString title, msg;

	title.LoadString(res, IDS_MIKC_WIN_TITLE);
	msg.LoadString(res, id);

	PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
	alert.Show();
}



/******************************************************************************/
/* AllocPositions() allocates enough memory to hold the positions.            */
/*                                                                            */
/* Input:  "total" is the total number of positions to allocate.              */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikConverter::AllocPositions(int32 total)
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
bool MikConverter::AllocPatterns(void)
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
bool MikConverter::AllocTracks(void)
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
bool MikConverter::AllocSamples(void)
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
bool MikConverter::AllocInstruments(void)
{
	int32 t, n;

	if ((of.instruments = new INSTRUMENT[of.numIns]) == NULL)
		return (false);

	for (t = 0; t < of.numIns; t++)
	{
		for (n = 0; n < INSTNOTES; n++)	// Init note / sample lookup table
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
		memset(of.instruments[t].volEnv, 0, ENVPOINTS * sizeof(ENVPT));
		memset(of.instruments[t].panEnv, 0, ENVPOINTS * sizeof(ENVPT));
		memset(of.instruments[t].pitEnv, 0, ENVPOINTS * sizeof(ENVPT));
	}

	return (true);
}



/******************************************************************************/
/* AllocLinear() allocates enough memory to hold linear period information.   */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikConverter::AllocLinear(void)
{
	if (of.numSmp > noteIndexCount)
	{
		int32 *newNoteIndex;

		newNoteIndex = new int32[of.numSmp];
		if (newNoteIndex == NULL)
			return (false);

		if (noteIndex != NULL)
			memcpy(newNoteIndex, noteIndex, noteIndexCount * sizeof(int32));

		delete[] noteIndex;
		noteIndex      = newNoteIndex;
		noteIndexCount = of.numSmp;
	}

	return (true);
}



/******************************************************************************/
/* FreeLinear() free the linear array memory.                                 */
/******************************************************************************/
void MikConverter::FreeLinear(void)
{
	delete[] noteIndex;

	noteIndex      = NULL;
	noteIndexCount = 0;
}



/******************************************************************************/
/* ReadComment() reads the comment into the UNIMOD structure.                 */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the comment from.          */
/*         "len" is the number of bytes to read.                              */
/*         "translate" indicate if you want to translate new lines or not.    */
/******************************************************************************/
void MikConverter::ReadComment(PFile *file, uint16 len, bool translate)
{
	if (len)
	{
		int32 i;
		char *buffer;

		// Allocate the buffer
		buffer = new char[len + 1];
		if (buffer == NULL)
		{
			file->Seek(len, PFile::pSeekCurrent);
			return;
		}

		// Read the comment
		file->ReadString(buffer, len);
		buffer[len] = 0x00;

		// Set the string
		of.comment.SetString(buffer, &charSet850);

		// Translate IT linefeeds
		if (translate)
		{
			int32 strLen = of.comment.GetLength();
			int8 chrLen;

			// Switch character set
			of.comment.SwitchCharacterSet(&charSet850);

			for (i = 0; i < strLen; i++)
			{
				if (charSet850.ToUnicode(of.comment.GetAt(i).GetChar(), chrLen) == 0x266a)
					of.comment.SetAt(i, '\n');
			}

			// Switch back again
			of.comment.SwitchToHostCharacterSet();
		}

		// And delete the buffer
		delete[] buffer;
	}
}



/******************************************************************************/
/* ReadLinedComment() reads the comment into the UNIMOD structure.            */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the comment from.          */
/*         "len" is the number of bytes to read.                              */
/*         "lineLen" is the number of bytes per line.                         */
/*         "translate" indicate if you want to translate new lines or not.    */
/******************************************************************************/
void MikConverter::ReadLinedComment(PFile *file, uint16 len, uint16 lineLen, bool translate)
{
	ReadComment(file, len, translate);
}



/******************************************************************************/
/* S3MIT_ProcessCmd() convert a S3M/IT effect command to a unimod command.    */
/*                                                                            */
/* Input:  "cmd" is the command.                                              */
/*         "inf" is the info.                                                 */
/*         "flags" indicate to use old or new effects.                        */
/******************************************************************************/
void MikConverter::S3MIT_ProcessCmd(uint8 cmd, uint8 inf, uint32 flags)
{
	uint8 hi, lo;

	lo = inf & 0xf;
	hi = inf >> 4;

	if (cmd != 255)
	{
		switch (cmd)
		{
			// Axx set speed to xx
			case 1:
			{
				UniEffect(UNI_S3MEFFECTA, inf);
				break;
			}

			// Bxx position jump
			case 2:
			{
				if (inf < posLookupCnt)
				{
					// Switch to curious mode if necessary, for example
					// sympex.it, deep joy.it
					if (((int8)posLookup[inf] < 0) && (origPositions[inf] != 255))
						S3MIT_CreateOrders(1);

					if (!((int8)posLookup[inf] < 0))
						UniPTEffect(0xb, posLookup[inf], of.flags);
				}
				break;
			}

			// Cxx pattern break to row xx
			case 3:
			{
				if ((flags & S3MIT_OLDSTYLE) && !(flags & S3MIT_IT))
					UniPTEffect(0xd, ((inf >> 4) * 10) + (inf & 0xf), of.flags);
				else
					UniPTEffect(0xd, inf, of.flags);

				break;
			}

			// Dxy volumeslide
			case 4:
			{
				UniEffect(UNI_S3MEFFECTD, inf);
				break;
			}

			// Exy toneslide down
			case 5:
			{
				UniEffect(UNI_S3MEFFECTE, inf);
				break;
			}

			// Fxy toneslide up
			case 6:
			{
				UniEffect(UNI_S3MEFFECTF, inf);
				break;
			}

			// Gxx tone portamento, speed xx
			case 7:
			{
				if (flags & S3MIT_OLDSTYLE)
					UniPTEffect(0x3, inf, of.flags);
				else
					UniEffect(UNI_ITEFFECTG, inf);

				break;
			}

			// Hxy vibrato
			case 8:
			{
				if (flags & S3MIT_OLDSTYLE)
					UniPTEffect(0x4, inf, of.flags);
				else
					UniEffect(UNI_ITEFFECTH, inf);

				break;
			}

			// Ixy tremor, ontime x, offtime y
			case 9:
			{
				if (flags & S3MIT_OLDSTYLE)
					UniEffect(UNI_S3MEFFECTI, inf);
				else
					UniEffect(UNI_ITEFFECTI, inf);

				break;
			}

			// Jxy arpeggio
			case 0xa:
			{
				UniPTEffect(0x0, inf, of.flags);
				break;
			}

			// Kxy dual command H00 & Dxy
			case 0xb:
			{
				if (flags & S3MIT_OLDSTYLE)
					UniPTEffect(0x4, 0, of.flags);
				else
					UniEffect(UNI_ITEFFECTH, 0);

				UniEffect(UNI_S3MEFFECTD, inf);
				break;
			}

			// Lxy dual command G00 & Dxy
			case 0xc:
			{
				if (flags & S3MIT_OLDSTYLE)
					UniPTEffect(0x3, 0, of.flags);
				else
					UniEffect(UNI_ITEFFECTG, 0);

				UniEffect(UNI_S3MEFFECTD, inf);
				break;
			}

			// Mxx set channel volume
			case 0xd:
			{
				UniEffect(UNI_ITEFFECTM, inf);
				break;
			}

			// Nxy slide channel volume
			case 0xe:
			{
				UniEffect(UNI_ITEFFECTN, inf);
				break;
			}

			// Oxx set sampleoffset xx00h
			case 0xf:
			{
				UniPTEffect(0x9, inf, of.flags);
				break;
			}

			// Pxy slide panning commands
			case 0x10:
			{
				UniEffect(UNI_ITEFFECTP, inf);
				break;
			}

			// Qxy retrig (+ volumeslide)
			case 0x11:
			{
				UniWriteByte(UNI_S3MEFFECTQ);
				if (inf && !lo && !(flags & S3MIT_OLDSTYLE))
					UniWriteByte(1);
				else
					UniWriteByte(inf);

				break;
			}

			// Rxy tremolo speed x, depth y
			case 0x12:
			{
				UniEffect(UNI_S3MEFFECTR, inf);
				break;
			}

			// Sxx special commands
			case 0x13:
			{
				if (inf >= 0xf0)
				{
					// Change resonant filter settings if necessary
					if ((filters) && ((inf & 0xf) != activeMacro))
					{
						activeMacro = inf & 0xf;

						for (inf = 0; inf < 0x80; inf++)
							filterSettings[inf].filter = filterMacros[activeMacro];
					}
				}
				else
				{
					// Scream Tracker does not have samples larger than
					// 64 Kb, thus doesn't need the SAx effect
					if ((flags & S3MIT_SCREAM) && ((inf & 0xf0) == 0xa0))
						break;

					UniEffect(UNI_ITEFFECTS0, inf);
				}
				break;
			}

			// Txx tempo
			case 0x14:
			{
				if (inf > 0x20)
					UniEffect(UNI_S3MEFFECTT, inf);
				else
				{
					if (!(flags & S3MIT_OLDSTYLE))
						UniEffect(UNI_ITEFFECTT, inf);
				}
				break;
			}

			// Uxy fine vibrato speed x, depth y
			case 0x15:
			{
				if (flags & S3MIT_OLDSTYLE)
					UniEffect(UNI_S3MEFFECTU, inf);
				else
					UniEffect(UNI_ITEFFECTU, inf);

				break;
			}

			// Vxx set global volume
			case 0x16:
			{
				UniEffect(UNI_XMEFFECTG, inf);
				break;
			}

			// Wxy global volume slide
			case 0x17:
			{
				UniEffect(UNI_ITEFFECTW, inf);
				break;
			}

			// Xxx amiga command 8xx
			case 0x18:
			{
				if (flags & S3MIT_OLDSTYLE)
				{
					if (inf > 128)
						UniEffect(UNI_ITEFFECTS0, 0x91);		// Surround
					else
						UniPTEffect(0x8, (inf == 128) ? 255 : (inf << 1), of.flags);
				}
				else
					UniPTEffect(0x8, inf, of.flags);

				break;
			}

			// Yxy panbrello, speed x, depth y
			case 0x19:
			{
				UniEffect(UNI_ITEFFECTY, inf);
				break;
			}

			// Zxx midi/resonant filters
			case 0x1a:
			{
				if (filterSettings[inf].filter)
				{
					UniWriteByte(UNI_ITEFFECTZ);
					UniWriteByte(filterSettings[inf].filter);
					UniWriteByte(filterSettings[inf].inf);
				}
				break;
			}
		}
	}
}



/******************************************************************************/
/* S3MIT_CreateOrders() handles S3M and IT orders.                            */
/*                                                                            */
/* Input:  "curious" is curious flag.                                         */
/******************************************************************************/
void MikConverter::S3MIT_CreateOrders(int32 curious)
{
	int32 t;

	of.numPos = 0;

	memset(of.positions, 0, posLookupCnt * sizeof(uint16));
	memset(posLookup, -1, 256);

	for (t = 0; t < posLookupCnt; t++)
	{
		int32 order = origPositions[t];
		if (order == 255)
			order = LAST_PATTERN;

		of.positions[of.numPos] = order;
		posLookup[t] = of.numPos;	// Bug fix for freaky S3Ms / ITs

		if (origPositions[t] < 254)
			of.numPos++;
		else
		{
			// End of song special order
			if ((order == LAST_PATTERN) && (!(curious--)))
				break;
		}
	}
}



/******************************************************************************/
/* SpeedToFinetune() will convert the speed to a finetune value.              */
/*                                                                            */
/* Input:  "speed" is the speed to convert.                                   */
/*         "sampNum" is the index to the sample.                              */
/*                                                                            */
/* Output: Is the new finetune value.                                         */
/******************************************************************************/
int32 MikConverter::SpeedToFinetune(int32 speed, int32 sampNum)
{
	int32 cTmp = 0, tmp, note = 1, finetune = 0;

	speed >>= 1;

	while ((tmp = GetFrequency(of.flags, GetLinearPeriod(note << 1, 0))) < speed)
	{
		cTmp = tmp;
		note++;
	}

	if (tmp != speed)
	{
		if ((tmp - speed) < (speed - cTmp))
		{
			while (tmp > speed)
				tmp = GetFrequency(of.flags, GetLinearPeriod(note << 1, --finetune));
		}
		else
		{
			note--;
			while (cTmp < speed)
				cTmp = GetFrequency(of.flags, GetLinearPeriod(note << 1, ++finetune));
		}
	}

	noteIndex[sampNum] = note - 4 * OCTAVE;
	return (finetune);
}



/******************************************************************************/
/* GetFrequency() XM linear period to frequency conversion.                   */
/*                                                                            */
/* Input:  "flags" is the module flags.                                       */
/*         "period" is the period to convert.                                 */
/*                                                                            */
/* Output: The frequency.                                                     */
/******************************************************************************/
uint32 MikConverter::GetFrequency(uint16 flags, uint32 period)
{
	if (flags & UF_LINEAR)
	{
		int32 shift = ((int32)period / 768) - HIGH_OCTAVE;

		if (shift >= 0)
			return (linTab[period % 768] >> shift);
		else
			return (linTab[period % 768] << (-shift));
	}
	else
		return ((8363L * 1712L) / (period ? period : 1));
}



/******************************************************************************/
/* GetLinearPeriod() calculates the notes period and return it.               */
/*                                                                            */
/* Input:  "note" is the note to calculate the period on.                     */
/*         "fine" is the middle C speed.                                      */
/*                                                                            */
/* Output: The period.                                                        */
/******************************************************************************/
uint16 MikConverter::GetLinearPeriod(uint16 note, uint32 fine)
{
	uint16 t;

	t = ((20L + 2 * HIGH_OCTAVE) * OCTAVE + 2 - note) * 32L - (fine >> 1);

	return (t);
}
