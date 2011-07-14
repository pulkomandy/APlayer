/******************************************************************************/
/* AHX Player Interface.                                                      */
/*                                                                            */
/* Original player by Bernhard Wodok.                                         */
/* APlayer plug-in by Thomas Neumann.                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"

// Player headers
#include "AHX.h"
#include "Tables.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.08f



/******************************************************************************/
/* AHXOutput                                                                  */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
AHXOutput::AHXOutput(void)
{
	player = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
AHXOutput::~AHXOutput(void)
{
}



/******************************************************************************/
/* Init() initialize the output class.                                        */
/******************************************************************************/
void AHXOutput::Init(int32 frequency, int32 bits, int32 mixLen, float boost, int32 hz)
{
	int32 i, j;

	this->mixLen    = mixLen;
	this->frequency = frequency;
	this->bits      = bits;
	this->hz        = hz;

	// Generate volume table
	for (i = 0; i < 65; i++)
	{
		for (j = -128; j < 128; j++)
			volumeTable[i][j + 128] = (int32)(i * j * boost) / 64;
	}
}



/******************************************************************************/
/* Free() free all allocated resources used by the class.                     */
/******************************************************************************/
void AHXOutput::Free(void)
{
}



/******************************************************************************/
/* PrepareBuffers() generate the sampling buffer.                             */
/*                                                                            */
/* Input:  "buffers" is a pointer to the array holding the buffers to store   */
/*         the samples in.                                                    */
/******************************************************************************/
void AHXOutput::PrepareBuffers(int16 **buffers)
{
	int32 nrSamples = player->mixerFreq / hz / player->song->speedMultiplier;
	int16 *mb[4];
	int8 v;

	// Remember the sample buffer pointers
	for (v = 0; v < 4; v++)
	{
		mb[v] = buffers[v];
		memset(mb[v], 0, mixLen * player->mixerFreq / hz * sizeof(int16));
	}

	for (int32 f = 0; f < mixLen * player->song->speedMultiplier; f++)
	{
		// Call the play function
		player->PlayIRQ();

		// Create the sample buffers
		for (v = 0; v < 4; v++)
			GenerateBuffer(nrSamples, &mb[v], v);
	}
}



/******************************************************************************/
/* GenerateBuffer() generate one sampling buffer.                             */
/*                                                                            */
/* Input:  "nrSamples" is the number of samples to generate.                  */
/*         "mb" is a pointer where the pointer to the sampling buffer.        */
/*         "v" is the voice number.                                           */
/******************************************************************************/
void AHXOutput::GenerateBuffer(int32 nrSamples, int16 **mb, int8 v)
{
	static int32 pos[4] = { 0, 0, 0, 0 };

	if (player->voices[v].voiceVolume != 0)
	{
		float freq = Period2Freq(player->voices[v].voicePeriod);
		int32 delta = (int32)(freq * (1 << 16) / frequency);
		int32 samples_to_mix = nrSamples;
		int32 mixPos = 0;

		while (samples_to_mix != 0)
		{
			if (pos[v] > (0x280 << 16))
				pos[v] -= 0x280 << 16;

			int32 thisCount = min(samples_to_mix, ((0x280 << 16) - pos[v] - 1) / delta + 1);
			samples_to_mix -= thisCount;

			int32 *volTab = &volumeTable[player->voices[v].voiceVolume][128];

			for (int32 i = 0; i < thisCount; i++)
			{
				(*mb)[mixPos++] = volTab[player->voices[v].voiceBuffer[pos[v] >> 16]] << 7;
				pos[v] += delta;
			}
		}
	}

	*mb += nrSamples;
}





/******************************************************************************/
/* AHXSong                                                                    */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
AHXSong::AHXSong(void)
{
	restart      = 0;
	positionNr   = 0;
	trackLength  = 0;
	trackNr      = 0;
	instrumentNr = 0;
	subsongNr    = 0;

	positions    = NULL;
	tracks       = NULL;
	instruments  = NULL;
	subsongs     = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
AHXSong::~AHXSong(void)
{
}





/******************************************************************************/
/* AHXVoice                                                                   */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
AHXVoice::AHXVoice(void)
{
	Init();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
AHXVoice::~AHXVoice(void)
{
}



/******************************************************************************/
/* Init() initialize the voice variables.                                     */
/******************************************************************************/
void AHXVoice::Init(void)
{
	voiceVolume           = 0;
	voicePeriod           = 0;

	track                 = 0;
	transpose             = 0;
	nextTrack             = 0;
	nextTranspose         = 0;

	adsrVolume            = 0;
	adsr.aFrames          = 0;
	adsr.aVolume          = 0;
	adsr.dFrames          = 0;
	adsr.dVolume          = 0;
	adsr.sFrames          = 0;
	adsr.rFrames          = 0;
	adsr.rVolume          = 0;

	instrument            = NULL;

	instrPeriod           = 0;
	trackPeriod           = 0;
	vibratoPeriod         = 0;

	noteMaxVolume         = 0;
	perfSubVolume         = 0;
	trackMasterVolume     = 0x40;

	newWaveform           = false;
	waveform              = 0;
	plantSquare           = false;
	plantPeriod           = false;
	ignoreSquare          = false;

	trackOn               = true;
	fixedNote             = false;

	volumeSlideUp         = false;
	volumeSlideDown       = false;

	hardCut               = 0;
	hardCutRelease        = false;
	hardCutReleaseF       = 0;

	periodSlideSpeed      = 0;
	periodSlidePeriod     = 0;
	periodSlideLimit      = 0;
	periodSlideOn         = false;
	periodSlideWithLimit  = false;

	periodPerfSlideSpeed  = 0;
	periodPerfSlidePeriod = 0;
	periodPerfSlideOn     = false;

	vibratoDelay          = 0;
	vibratoCurrent        = 0;
	vibratoDepth          = 0;
	vibratoSpeed          = 0;

	squareOn              = false;
	squareInit            = false;
	squareWait            = 0;
	squareLowerLimit      = 0;
	squareUpperLimit      = 0;
	squarePos             = 0;
	squareSign            = 0;
	squareSlidingIn       = false;
	squareReverse         = false;

	filterOn              = false;
	filterInit            = false;
	filterWait            = 0;
	filterLowerLimit      = 0;
	filterUpperLimit      = 0;
	filterPos             = 0;
	filterSign            = 0;
	filterSpeed           = 0;
	filterSlidingIn       = false;
	ignoreFilter          = false;

	perfCurrent           = 0;
	perfSpeed             = 0;
	perfWait              = 0;

	waveLength            = 0;

	perfList              = NULL;

	noteDelayWait         = 0;
	noteDelayOn           = false;
	noteCutWait           = 0;
	noteCutOn             = false;

	audioSource           = NULL;

	audioPeriod           = 0;
	audioVolume           = 0;

	memset(voiceBuffer, 0, sizeof(voiceBuffer));
	memset(squareTempBuffer, 0, sizeof(squareTempBuffer));
}



/******************************************************************************/
/* CalcADSR() calculates the ADSR envelope.                                   */
/******************************************************************************/
void AHXVoice::CalcADSR(void)
{
	adsr.aFrames = instrument->envelope.aFrames;
	adsr.aVolume = instrument->envelope.aVolume * 256 / adsr.aFrames;
	adsr.dFrames = instrument->envelope.dFrames;
	adsr.dVolume = (instrument->envelope.dVolume - instrument->envelope.aVolume) * 256 / adsr.dFrames;
	adsr.sFrames = instrument->envelope.sFrames;
	adsr.rFrames = instrument->envelope.rFrames;
	adsr.rVolume = (instrument->envelope.rVolume - instrument->envelope.dVolume) * 256 / adsr.rFrames;
}





/******************************************************************************/
/* AHXWaves                                                                   */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
AHXWaves::AHXWaves(void)
{
	Generate();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
AHXWaves::~AHXWaves(void)
{
}



/******************************************************************************/
/* Generate() generates all the wave tables.                                  */
/******************************************************************************/
void AHXWaves::Generate(void)
{
	GenerateSawtooth(sawtooth04, 0x04);
	GenerateSawtooth(sawtooth08, 0x08);
	GenerateSawtooth(sawtooth10, 0x10);
	GenerateSawtooth(sawtooth20, 0x20);
	GenerateSawtooth(sawtooth40, 0x40);
	GenerateSawtooth(sawtooth80, 0x80);

	GenerateTriangle(triangle04, 0x04);
	GenerateTriangle(triangle08, 0x08);
	GenerateTriangle(triangle10, 0x10);
	GenerateTriangle(triangle20, 0x20);
	GenerateTriangle(triangle40, 0x40);
	GenerateTriangle(triangle80, 0x80);

	GenerateSquare(squares);

	GenerateWhiteNoise(whiteNoiseBig, 0x280 * 3);
	GenerateFilterWaveforms(triangle04, lowPasses, highPasses);
}



/******************************************************************************/
/* GenerateSawtooth() generates a sawtooth wave table.                        */
/*                                                                            */
/* Input:  "buffer" is a pointer to where to store the table.                 */
/*         "len" is the length of the table.                                  */
/******************************************************************************/
void AHXWaves::GenerateSawtooth(int8 *buffer, int32 len)
{
	int8 *edi = buffer;
	int32 ebx = 256 / (len - 1);
	int32 eax = -128;

	for (int32 ecx = 0; ecx < len; ecx++)
	{
		*edi++ = eax;
		eax   += ebx;
	}
}



/******************************************************************************/
/* GenerateTriangle() generates a triangle wave table.                        */
/*                                                                            */
/* Input:  "buffer" is a pointer to where to store the table.                 */
/*         "len" is the length of the table.                                  */
/******************************************************************************/
void AHXWaves::GenerateTriangle(int8 *buffer, int32 len)
{
	int32 ecx;
	int32 d2  = len;
	int32 d5  = d2 >> 2;
	int32 d1  = 128 / d5;
	int32 d4  = -(d2 >> 1);
	int8 *edi = buffer;
	int32 eax = 0;

	for (ecx = 0; ecx < d5; ecx++)
	{
		*edi++ = eax;
		eax   += d1;
	}

	*edi++ = 0x7f;

	if (d5 != 1)
	{
		eax = 128;
		for (ecx = 0; ecx < d5 - 1; ecx++)
		{
			eax   -= d1;
			*edi++ = eax;
		}
	}

	int8 *esi = edi + d4;
	for (ecx = 0; ecx < d5 * 2; ecx++)
	{
		*edi++ = *esi++;
		if (edi[-1] == 0x7f)
			edi[-1] = 0x80;
		else
			edi[-1] = -edi[-1];
	}
}



/******************************************************************************/
/* GenerateSquare() generates a square wave table.                            */
/*                                                                            */
/* Input:  "buffer" is a pointer to where to store the table.                 */
/******************************************************************************/
void AHXWaves::GenerateSquare(int8 *buffer)
{
	int32 ecx;
	int8 *edi = buffer;

	for (int32 ebx = 1; ebx <= 0x20; ebx++)
	{
		for (ecx = 0; ecx < (0x40 - ebx) * 2; ecx++)
			*edi++ = 0x80;

		for (ecx = 0; ecx < ebx * 2; ecx++)
			*edi++ = 0x7f;
	}
}



/******************************************************************************/
/* GenerateWhiteNoise() generates a white noise wave table.                   */
/*                                                                            */
/* Input:  "buffer" is a pointer to where to store the table.                 */
/*         "len" is the length of the table.                                  */
/******************************************************************************/
void AHXWaves::GenerateWhiteNoise(int8 *buffer, int32 len)
{
	memcpy(buffer, whiteNoiseTable, len);
}



/******************************************************************************/
/* Clip() test for bounds values.                                             */
/*                                                                            */
/* Input:  "x" is a pointer to the value to check.                            */
/******************************************************************************/
inline void AHXWaves::Clip(float *x)
{
	if (*x > 127.0f)
	{
		*x = 127.0f;
		return;
	}

	if (*x < -128.0f)
	{
		*x = -128.0f;
		return;
	}
}



/******************************************************************************/
/* GenerateFilterWaveforms() generates the filter waveform table.             */
/*                                                                            */
/* Input:  "buffer" is a pointer to a table to use to build the filter table. */
/*         "lowBuf" is a pointer to where to store the lowpass table.         */
/*         "highBuf" is a pointer to where to store the highpass table.       */
/******************************************************************************/
void AHXWaves::GenerateFilterWaveforms(int8 *buffer, int8 *lowBuf, int8 *highBuf)
{
	for (int32 temp = 0, freq = 8; temp < 31; temp++, freq += 3)
	{
		int8 *a0 = buffer;

		for (int32 waves = 0; waves < 6 + 6 + 0x20 + 1; waves++)
		{
			float fre = (float)freq * 1.25f / 100.0f;
			float high, mid = 0.0f, low = 0.0f;
			int32 i;

			for (i = 0; i <= lengthTable[waves]; i++)
			{
				high = a0[i] - mid - low;
				Clip(&high);

				mid += high * fre;
				Clip(&mid);

				low += mid * fre;
				Clip(&low);
			}

			for (i = 0; i <= lengthTable[waves]; i++)
			{
				high = a0[i] - mid - low;
				Clip(&high);

				mid += high * fre;
				Clip(&mid);

				low += mid * fre;
				Clip(&low);

				*lowBuf++ =  (int8)low;
				*highBuf++ = (int8)high;
			}

			a0 += lengthTable[waves] + 1;
		}
	}
}





/******************************************************************************/
/* AHX                                                                        */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
AHX::AHX(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	output        = NULL;
	song          = NULL;
	waves         = NULL;

	sampBuffer[0] = NULL;
	sampBuffer[1] = NULL;
	sampBuffer[2] = NULL;
	sampBuffer[3] = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
AHX::~AHX(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float AHX::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetCount() returns the number of add-ons in the library.                   */
/*                                                                            */
/* Output: The number of add-ons.                                             */
/******************************************************************************/
int32 AHX::GetCount(void)
{
	return (2);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 AHX::GetSupportFlags(int32 index)
{
	return (appSamplePlayer | appSetPosition);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString AHX::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_AHX_NAME + index);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString AHX::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_AHX_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString AHX::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_AHX_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a AHX/THX module.            */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result AHX::ModuleCheck(int32 index, PFile *file)
{
	int32 len;
	uint8 buf[4];

	// Check the module size
	len = file->GetLength();
	if (len < 14)
		return (AP_UNKNOWN);

	// Check the mark
	file->SeekToBegin();
	file->Read(buf, 4);

	if ((buf[0] == 'T') && (buf[1] == 'H') && (buf[2] == 'X') && (buf[3] == index))
		return (AP_OK);

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* LoadModule() will load the module into the memory.                         */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result AHX::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	int32 i, j;
	uint8 flag;
	uint8 byte1, byte2, byte3;
	int32 maxTrack;
	PCharSet_Amiga charSet;
	ap_result retVal = AP_ERROR;

	try
	{
		// Allocate the song
		song = new AHXSong();
		if (song == NULL)
		{
			errorStr.LoadString(res, IDS_AHX_ERR_MEMORY);
			throw PUserException();
		}

		// Seek to the revision and read it
		file->Seek(3, PFile::pSeekBegin);
		song->revision = file->Read_UINT8();

		// Skip songtitle offset
		file->Seek(2, PFile::pSeekCurrent);

		//
		// Header
		//
		flag = file->Read_UINT8();
		song->speedMultiplier = ((flag >> 5) & 3) + 1;

		song->positionNr   = ((flag & 0xf) << 8) | file->Read_UINT8();
		song->restart      = file->Read_B_UINT16();
		song->trackLength  = file->Read_UINT8();
		song->trackNr      = file->Read_UINT8();
		song->instrumentNr = file->Read_UINT8();
		song->subsongNr    = file->Read_UINT8();

		// Read subsongs
		song->subsongs = new int32[song->subsongNr];
		if (song->subsongs == NULL)
		{
			errorStr.LoadString(res, IDS_AHX_ERR_MEMORY);
			throw PUserException();
		}

		for (i = 0; i < song->subsongNr; i++)
		{
			song->subsongs[i] = file->Read_B_UINT16();

			// Do we need to fix the start index?
			if (song->subsongs[i] >= song->positionNr)
				song->subsongs[i] = 0;
		}

		// Read position list
		song->positions = new AHXPosition[song->positionNr];
		if (song->positions == NULL)
		{
			errorStr.LoadString(res, IDS_AHX_ERR_MEMORY);
			throw PUserException();
		}

		for (i = 0; i < song->positionNr; i++)
		{
			for (j = 0; j < 4; j++)
			{
				song->positions[i].track[j]     = file->Read_UINT8();
				song->positions[i].transpose[j] = (int8)file->Read_UINT8();
			}
		}

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_AHX_ERR_LOADING_HEADER);
			throw PUserException();
		}

		//
		// Tracks
		//
		maxTrack = song->trackNr;
		song->tracks = new AHXStep *[maxTrack + 1];
		if (song->tracks == NULL)
		{
			errorStr.LoadString(res, IDS_AHX_ERR_MEMORY);
			throw PUserException();
		}

		// Clear all the pointers
		memset(song->tracks, 0, (maxTrack + 1) * sizeof(AHXStep *));

		for (i = 0; i < maxTrack + 1; i++)
		{
			song->tracks[i] = new AHXStep[song->trackLength];
			if (song->tracks == NULL)
			{
				errorStr.LoadString(res, IDS_AHX_ERR_MEMORY);
				throw PUserException();
			}

			if (((flag & 0x80) == 0x80) && (i == 0))
			{
				for (j = 0; j < song->trackLength; j++)
				{
					song->tracks[i][j].note       = 0;
					song->tracks[i][j].instrument = 0;
					song->tracks[i][j].fx         = 0;
					song->tracks[i][j].fxParam    = 0;
				}
				continue;
			}

			for (j = 0; j < song->trackLength; j++)
			{
				// Read the 3 track bytes
				byte1 = file->Read_UINT8();
				byte2 = file->Read_UINT8();
				byte3 = file->Read_UINT8();

				song->tracks[i][j].note       = (byte1 >> 2) & 0x3f;
				song->tracks[i][j].instrument = ((byte1 & 0x3) << 4) | (byte2 >> 4);
				song->tracks[i][j].fx         = byte2 & 0xf;
				song->tracks[i][j].fxParam    = byte3;
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_AHX_ERR_LOADING_TRACKS);
				throw PUserException();
			}
		}

		//
		// Instruments
		//
		song->instruments = new AHXInstrument[song->instrumentNr + 1];
		if (song->instruments == NULL)
		{
			errorStr.LoadString(res, IDS_AHX_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the play list entry pointers
		for (i = 1; i < song->instrumentNr + 1; i++)
			song->instruments[i].playList.entries = NULL;

		for (i = 1; i < song->instrumentNr + 1; i++)
		{
			song->instruments[i].volume               = file->Read_UINT8();

			byte1 = file->Read_UINT8();
			song->instruments[i].filterSpeed          = (byte1 >> 3) & 0x1f;
			song->instruments[i].waveLength           = byte1 & 0x7;
			song->instruments[i].envelope.aFrames     = file->Read_UINT8();
			song->instruments[i].envelope.aVolume     = file->Read_UINT8();
			song->instruments[i].envelope.dFrames     = file->Read_UINT8();
			song->instruments[i].envelope.dVolume     = file->Read_UINT8();
			song->instruments[i].envelope.sFrames     = file->Read_UINT8();
			song->instruments[i].envelope.rFrames     = file->Read_UINT8();
			song->instruments[i].envelope.rVolume     = file->Read_UINT8();

			file->Seek(3, PFile::pSeekCurrent);
			byte1 = file->Read_UINT8();
			song->instruments[i].filterSpeed         |= ((byte1 >> 2) & 0x20);
			song->instruments[i].filterLowerLimit     = byte1 & 0x7f;
			song->instruments[i].vibratoDelay         = file->Read_UINT8();

			byte1 = file->Read_UINT8();
			song->instruments[i].hardCutReleaseFrames = (byte1 >> 4) & 7;
			song->instruments[i].hardCutRelease       = byte1 & 0x80 ? true : false;
			song->instruments[i].vibratoDepth         = byte1 & 0xf;
			song->instruments[i].vibratoSpeed         = file->Read_UINT8();
			song->instruments[i].squareLowerLimit     = file->Read_UINT8();
			song->instruments[i].squareUpperLimit     = file->Read_UINT8();
			song->instruments[i].squareSpeed          = file->Read_UINT8();
			song->instruments[i].filterUpperLimit     = file->Read_UINT8() & 0x3f;
			song->instruments[i].playList.speed       = file->Read_UINT8();
			song->instruments[i].playList.length      = file->Read_UINT8();

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_AHX_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}

			// Load play list
			song->instruments[i].playList.entries = new AHXPListEntry[song->instruments[i].playList.length];
			if (song->instruments[i].playList.entries == NULL)
			{
				errorStr.LoadString(res, IDS_AHX_ERR_MEMORY);
				throw PUserException();
			}

			for (j = 0; j < song->instruments[i].playList.length; j++)
			{
				byte1 = file->Read_UINT8();
				byte2 = file->Read_UINT8();

				song->instruments[i].playList.entries[j].fx[1]      = (byte1 >> 5) & 7;
				song->instruments[i].playList.entries[j].fx[0]      = (byte1 >> 2) & 7;
				song->instruments[i].playList.entries[j].waveform   = ((byte1 << 1) & 6) | (byte2 >> 7);
				song->instruments[i].playList.entries[j].fixed      = (byte2 >> 6) & 1;
				song->instruments[i].playList.entries[j].note       = byte2 & 0x3f;
				song->instruments[i].playList.entries[j].fxParam[0] = file->Read_UINT8();
				song->instruments[i].playList.entries[j].fxParam[1] = file->Read_UINT8();
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_AHX_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}
		}

		//
		// Strings
		//
		// Read the song title
		song->name = file->ReadLine(&charSet);

		// Read the instrument strings
		for (i = 1; i < song->instrumentNr + 1; i++)
			song->instruments[i].name = file->ReadLine(&charSet);

		retVal = AP_OK;
	}
	catch(PUserException e)
	{
		// Just delete the exception and clean up
		Cleanup();
	}
	catch(...)
	{
		// Clean up
		Cleanup();
		throw;
	}

	return (retVal);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool AHX::InitPlayer(int32 index)
{
	int32 i, j, k, m;
	int32 startPos, startRow, posHi, newPos;
	int32 track, effArg;
	SongTime *songTime;
	PosInfo posInfo;
	int32 curSpeed;
	bool pattBreak, done;
	float total;

	// Allocate output class
	output = new AHXOutput();
	if (output == NULL)
		throw PMemoryException();

	// Allocate wave class
	waves = new AHXWaves();
	if (waves == NULL)
		throw PMemoryException();

	// Allocate channel buffers
	bufLen = 2 * mixerFreq / 50;

	for (i = 0; i < 4; i++)
	{
		sampBuffer[i] = new int16[bufLen];
		if (sampBuffer[i] == NULL)
			throw PMemoryException();
	}

	// Initialize the player
	Init();

	// Initialize the output class
	output->player = this;
	output->Init(mixerFreq, 16, 2, 1.0f, 50);

	// Take each subsong
	for (i = 0; i <= song->subsongNr; i++)
	{
		// Allocate the song time structure
		songTime = new SongTime;
		if (songTime == NULL)
			throw PMemoryException();

		// Add the structure to the list
		songTimeList.AddTail(songTime);

		// Find the start position
		if (i == 0)
			startPos = 0;
		else
			startPos = song->subsongs[i - 1];

		songTime->startPos = startPos;

		// Initialize other start variables
		startRow = 0;
		curSpeed = 6;
		done     = false;
		total    = 0.0f;

		// Calculate the position times
		for (j = startPos; j < song->positionNr; j++)
		{
			// Add the position information to the list
			posInfo.speed = curSpeed;
			posInfo.time.SetTimeSpan(total);
			songTime->posInfoList.AddTail(posInfo);

			posHi     = 0;
			newPos    = -1;
			pattBreak = false;

			for (k = startRow; k < song->trackLength; k++)
			{
				startRow = 0;

				for (m = 0; m < 4; m++)
				{
					// Get track number
					track = song->positions[j].track[m];

					// Parse special effects
					switch (song->tracks[track][k].fx)
					{
						// Position jump HI
						case 0x0:
						{
							effArg = song->tracks[track][k].fxParam & 0x0f;
							if ((effArg > 0) && (effArg <= 9))
								posHi = effArg;
							
							break;
						}

						// Position jump
						case 0xb:
						{
							effArg    = song->tracks[track][k].fxParam;
							newPos    = posHi * 100 + (effArg & 0x0f) + (effArg >> 4) * 10;
							pattBreak = true;

							if (newPos <= j)
								done = true;
							break;
						}

						// Pattern break
						case 0xd:
						{
							effArg   = song->tracks[track][k].fxParam;
							startRow = (effArg & 0x0f) + (effArg >> 4) * 10;
							if (startRow > song->trackLength)
								startRow = 0;

							pattBreak = true;
							break;
						}

						// Speed
						case 0xf:
						{
							curSpeed = song->tracks[track][k].fxParam;
							if (curSpeed == 0)
								done = true;
							break;
						}
					}
				}

				// Add the row time
				total += (1000.0f * curSpeed / 50.0f / song->speedMultiplier);

				if (done || pattBreak)
					break;
			}

			if (newPos != -1)
				j = newPos - 1;

			if (done)
				break;
		}

		// Set the total time
		songTime->totalTime.SetTimeSpan(total);
	}

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void AHX::EndPlayer(int32 index)
{
	int32 i, count;

	// Clean up the song time list
	count = songTimeList.CountItems();
	for (i = 0; i < count; i++)
		delete songTimeList.GetItem(i);

	songTimeList.MakeEmpty();

	// Delete the player
	if (output != NULL)
	{
		output->Free();

		delete output;
		output = NULL;
	}

	// Delete the waves
	delete waves;
	waves  = NULL;

	// Delete the sample buffers
	for (int8 i = 0; i < 4; i++)
	{
		delete[] sampBuffer[i];
		sampBuffer[i] = NULL;
	}

	// Free the song
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void AHX::InitSound(int32 index, uint16 songNum)
{
	// Initialize the player
	InitSubsong(songNum);

	// Remember the current song number
	currentSong = songNum;
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void AHX::Play(void)
{
	// Play and prepare the sample buffers
	output->PrepareBuffers(sampBuffer);

	// Feed APlayer with the data
	for (int8 i = 0; i < 4; i++)
	{
		APChannel *channel = virtChannels[i];

		// Fill out the Channel object
		channel->SetBuffer(sampBuffer[i], bufLen);
		channel->SetVolume(256);
		channel->SetPanning(((i % 3) == 0) ? APPAN_LEFT : APPAN_RIGHT);
	}
}



/******************************************************************************/
/* GetSamplePlayerInfo() will fill out the sample info structure given.       */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to fill.      */
/******************************************************************************/
void AHX::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = mixerFreq;
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString AHX::GetModuleName(void)
{
	return (song->name);
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *AHX::GetSubSongs(void)
{
	songTab[0] = song->subsongNr + 1;	// Number of subsongs
	songTab[1] = 0;						// Default start song

	return (songTab);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 AHX::GetSongLength(void)
{
	return (song->positionNr);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 AHX::GetSongPosition(void)
{
	return (posNr);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void AHX::SetSongPosition(int16 pos)
{
	SongTime *songTime;

	// Change the position
	posNr          = pos;
	noteNr         = 0;
	posJumpNote    = 0;
	stepWaitFrames = 0;
	patternBreak   = false;
	getNewPosition = true;

	// Set the tempo
	songTime = songTimeList.GetItem(currentSong);
	if ((pos < songTime->startPos) || (pos >= songTime->posInfoList.CountItems()))
		tempo = 6;
	else
		tempo = songTime->posInfoList.GetItem(pos - songTime->startPos).speed;
}



/******************************************************************************/
/* GetTimeTable() will calculate the position time for each position and      */
/*      store them in the list given.                                         */
/*                                                                            */
/* Input:  "songNum" is the subsong number to get the time table for.         */
/*         "posTimes" is a reference to the list where you should store the   */
/*         start time for each position.                                      */
/*                                                                            */
/* Output: The total module time or 0 if time table is not supported.         */
/******************************************************************************/
PTimeSpan AHX::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	SongTime *songTime;
	int32 i, j, count;

	// Find the song time structure
	songTime = songTimeList.GetItem(songNum);

	// Well, fill the position time list up the empty times until
	// we reach the subsong position
	for (i = 0; i < songTime->startPos; i++)
		posTimes.AddTail(0);

	// Copy the position times
	count = songTime->posInfoList.CountItems();
	for (j = 0; j < count; j++, i++)
		posTimes.AddTail(songTime->posInfoList.GetItem(j).time);

	// And then fill the rest of the list with total time
	for (; i < song->positionNr; i++)
		posTimes.AddTail(songTime->totalTime);

	return (songTime->totalTime);
}



/******************************************************************************/
/* GetInfoString() returns the description and value string on the line       */
/*      given. If the line is out of range, false is returned.                */
/*                                                                            */
/* Input:  "line" is the line starting from 0.                                */
/*         "description" is a reference to where to store the description.    */
/*         "value" is a reference to where to store the value.                */
/*                                                                            */
/* Output: True if the information are returned, false if not.                */
/******************************************************************************/
bool AHX::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 3)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Song Length
		case 0:
		{
			description.LoadString(res, IDS_AHX_INFODESCLINE0);
			value.SetUNumber(GetSongLength());
			break;
		}

		// Used Tracks
		case 1:
		{
			description.LoadString(res, IDS_AHX_INFODESCLINE1);
			value.SetUNumber(song->trackNr);
			break;
		}

		// Used Instruments
		case 2:
		{
			description.LoadString(res, IDS_AHX_INFODESCLINE2);
			value.SetUNumber(song->instrumentNr);
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* GetSampleInfo() fills out the APSampleInfo structure given with the sample */
/*      information of the sample number given.                               */
/*                                                                            */
/* Input:  "num" is the sample number starting from 0.                        */
/*         "info" is a pointer to an APSampleInfo structure to fill out.      */
/*                                                                            */
/* Output: True if the information are returned, false if not.                */
/******************************************************************************/
bool AHX::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	AHXInstrument *inst;

	// First check the sample number for "out of range"
	if ((int32)num >= song->instrumentNr)
		return (false);

	// Get the pointer to the instrument data
	inst = &song->instruments[num + 1];

	// Fill out the sample info structure
	info->name       = inst->name;
	info->flags      = 0;
	info->type       = apSynth;
	info->bitSize    = 8;
	info->middleC    = 4144;
	info->volume     = inst->volume * 4;
	info->panning    = -1;
	info->address    = NULL;
	info->length     = 0;
	info->loopStart  = 0;
	info->loopLength = 0;

	return (true);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void AHX::Cleanup(void)
{
	int32 i;

	if (song->instruments != NULL)
	{
		for (i = 1; i < song->instrumentNr + 1; i++)
			delete[] song->instruments[i].playList.entries;

		delete[] song->instruments;
		song->instruments = NULL;
	}

	if (song->tracks != NULL)
	{
		for (i = 0; i < song->trackNr + 1; i++)
			delete[] song->tracks[i];

		delete[] song->tracks;
		song->tracks = NULL;
	}

	delete[] song->positions;
	song->positions = NULL;

	delete[] song->subsongs;
	song->subsongs = NULL;

	// Delete the song
	delete song;
	song = NULL;
}



/******************************************************************************/
/* Init() initialize the player.                                              */
/******************************************************************************/
void AHX::Init(void)
{
	waveformTab[0] = waves->triangle04;
	waveformTab[1] = waves->sawtooth04;
	waveformTab[2] = NULL;
	waveformTab[3] = waves->whiteNoiseBig;
}



/******************************************************************************/
/* InitSubsong() initialize the player to use the subsong given.              */
/*                                                                            */
/* Input:  "nr" is the subsong number to play.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool AHX::InitSubsong(int32 nr)
{
	int32 v;

	if (nr > song->subsongNr)
		return (false);

	if (nr == 0)
		posNr = 0;
	else
		posNr = song->subsongs[nr - 1];

	posJump        = 0;
	patternBreak   = false;
	mainVolume     = 0x40;
	playing        = 1;
	noteNr         = 0;
	posJumpNote    = 0;
	tempo          = 6;
	stepWaitFrames = 0;
	getNewPosition = true;
	songEndReached = false;
	timingValue    = 0;
	playingTime    = 0;

	for (v = 0; v < 4; v++)
		voices[v].Init();

	return (true);
}



/******************************************************************************/
/* PlayIRQ() is the main play function.                                       */
/******************************************************************************/
void AHX::PlayIRQ(void)
{
	if (stepWaitFrames <= 0)
	{
		if (getNewPosition)
		{
			int32 nextPos = (posNr + 1 == song->positionNr) ? 0 : (posNr + 1);

			for (int32 i = 0; i < 4; i++)
			{
				voices[i].track         = song->positions[posNr].track[i];
				voices[i].transpose     = song->positions[posNr].transpose[i];
				voices[i].nextTrack     = song->positions[nextPos].track[i];
				voices[i].nextTranspose = song->positions[nextPos].transpose[i];
			}

			getNewPosition = false;

			// Tell APlayer we have changed the position
			ChangePosition();
		}

		for (int32 i = 0; i < 4; i++)
			ProcessStep(i);

		stepWaitFrames = tempo;
	}

	// DoFrameStuff
	for (int32 i = 0; i < 4; i++)
		ProcessFrame(i);

	playingTime++;

	if ((tempo > 0) && (--stepWaitFrames <= 0))
	{
		if (!patternBreak)
		{
			noteNr++;
			if (noteNr >= song->trackLength)
			{
				posJump      = posNr + 1;
				posJumpNote  = 0;
				patternBreak = true;
			}
		}

		if (patternBreak)
		{
			if (posJump <= posNr)
			{
				// Tell APlayer we have changed the position and
				// the module has ended
				ChangePosition();
				endReached = true;
			}

			patternBreak = false;
			noteNr       = posJumpNote;
			posJumpNote  = 0;
			posNr        = posJump;
			posJump      = 0;

			if (posNr == song->positionNr)
			{
				songEndReached = true;
				posNr          = song->restart;

				// Tell APlayer we have changed the position and
				// the module has ended
				ChangePosition();
				endReached = true;
			}

			getNewPosition = true;
		}
	}

	// RemainPosition
	for (int32 a = 0; a < 4; a++)
		SetAudio(a);
}



/******************************************************************************/
/* ProcessStep()                                                              */
/******************************************************************************/
void AHX::ProcessStep(int32 v)
{
	if (!voices[v].trackOn)
		return;

	voices[v].volumeSlideUp   = 0;
	voices[v].volumeSlideDown = 0;

	int32 note       = song->tracks[song->positions[posNr].track[v]][noteNr].note;
	int32 instrument = song->tracks[song->positions[posNr].track[v]][noteNr].instrument;
	int32 fx         = song->tracks[song->positions[posNr].track[v]][noteNr].fx;
	int32 fxParam    = song->tracks[song->positions[posNr].track[v]][noteNr].fxParam;

	switch (fx)
	{
		// Position Jump HI
		case 0x0:
		{
			if (((fxParam & 0xf) > 0) && ((fxParam & 0xf) <= 9))
				posJump = fxParam & 0xf;

			break;
		}

		// Volume Slide + Tone Portamento
		case 0x5:

		// Volume Slide
		case 0xa:
		{
			voices[v].volumeSlideDown = fxParam & 0x0f;
			voices[v].volumeSlideUp   = fxParam >> 4;
			break;
		}

		// Position Jump
		case 0xb:
		{
			posJump      = posJump * 100 + (fxParam & 0x0f) + (fxParam >> 4) * 10;
			patternBreak = true;
			break;
		}

		// Patternbreak
		case 0xd:
		{
			posJump     = posNr + 1;
			posJumpNote = (fxParam & 0x0f) + (fxParam >> 4) * 10;

			if (posJumpNote > song->trackLength)
				posJumpNote = 0;

			patternBreak = true;
			break;
		}

		// Enhanced commands
		case 0xe:
		{
			switch (fxParam >> 4)
			{
				// Note Cut
				case 0xc:
				{
					if ((fxParam & 0x0f) < tempo)
					{
						voices[v].noteCutWait = fxParam & 0x0f;
						if (voices[v].noteCutWait != 0)
						{
							voices[v].noteCutOn      = true;
							voices[v].hardCutRelease = false;
						}
					}
					break;
				}

				// Note Delay
				case 0xd:
				{
					if (voices[v].noteDelayOn)
						voices[v].noteDelayOn = false;
					else
					{
						if ((fxParam & 0x0f) < tempo)
						{
							voices[v].noteDelayWait = fxParam & 0x0f;
							if (voices[v].noteDelayWait != 0)
							{
								voices[v].noteDelayOn = true;
								return;
							}
						}
					}
					break;
				}
			}
			break;
		}

		// Speed
		case 0xf:
		{
			tempo = fxParam;

			// Tell APlayer to end the module if the tempo is 0
			if (tempo == 0)
				endReached = true;

			break;
		}
	}

	// Instrument range check by Thomas Neumann
	if ((instrument != 0) && (instrument < song->instrumentNr))
	{
		voices[v].perfSubVolume     = 0x40;
		voices[v].periodSlideSpeed  = 0;
		voices[v].periodSlidePeriod = 0;
		voices[v].periodSlideLimit  = 0;
		voices[v].adsrVolume        = 0;
		voices[v].instrument        = &song->instruments[instrument];
		voices[v].CalcADSR();

		// InitOnInstrument
		voices[v].waveLength        = voices[v].instrument->waveLength;
		voices[v].noteMaxVolume     = voices[v].instrument->volume;

		// InitVibrato
		voices[v].vibratoCurrent    = 0;
		voices[v].vibratoDelay      = voices[v].instrument->vibratoDelay;
		voices[v].vibratoDepth      = voices[v].instrument->vibratoDepth;
		voices[v].vibratoSpeed      = voices[v].instrument->vibratoSpeed;
		voices[v].vibratoPeriod     = 0;

		// InitHardCut
		voices[v].hardCutRelease    = voices[v].instrument->hardCutRelease;
		voices[v].hardCut           = voices[v].instrument->hardCutReleaseFrames;

		// InitSquare
		voices[v].ignoreSquare      = false;
		voices[v].squareSlidingIn   = false;
		voices[v].squareWait        = 0;
		voices[v].squareOn          = false;

		int32 squareLower = voices[v].instrument->squareLowerLimit >> (5 - voices[v].waveLength);
		int32 squareUpper = voices[v].instrument->squareUpperLimit >> (5 - voices[v].waveLength);

		if (squareUpper < squareLower)
		{
			int32 t     = squareUpper;
			squareUpper = squareLower;
			squareLower = t;
		}

		voices[v].squareUpperLimit  = squareUpper;
		voices[v].squareLowerLimit  = squareLower;

		// InitFilter
		voices[v].ignoreFilter      = false;
		voices[v].filterWait        = 0;
		voices[v].filterOn          = false;
		voices[v].filterSlidingIn   = false;

		int32 d6 = voices[v].instrument->filterSpeed;
		int32 d3 = voices[v].instrument->filterLowerLimit;
		int32 d4 = voices[v].instrument->filterUpperLimit;

		if (d3 & 0x80)
			d6 |= 0x20;

		if (d4 & 0x80)
			d6 |= 0x40;

		voices[v].filterSpeed       = d6;

		d3 &= ~0x80;
		d4 &= ~0x80;

		if (d3 > d4)
		{
			int32 t = d3;
			d3      = d4;
			d4      = t;
		}

		voices[v].filterUpperLimit  = d4;
		voices[v].filterLowerLimit  = d3;
		voices[v].filterPos         = 32;

		// Init PerfList
		voices[v].perfWait          = 0;
		voices[v].perfCurrent       = 0;
		voices[v].perfSpeed         = voices[v].instrument->playList.speed;
		voices[v].perfList          = &voices[v].instrument->playList;
	}

	// NoInstrument
	voices[v].periodSlideOn = false;

	switch (fx)
	{
		// Override Filter
		case 0x4:
			break;

		// Set Squarewave-Offset
		case 0x9:
		{
			voices[v].squarePos    = fxParam >> (5 - voices[v].waveLength);
			voices[v].plantSquare  = true;
			voices[v].ignoreSquare = true;
			break;
		}

		// Tone Portamento + Volume Slide
		case 0x5:

		// Tone Portamento (Period Slide Up/Down w/ Limit)
		case 0x3:
		{
			if (fxParam != 0)
				voices[v].periodSlideSpeed = fxParam;

			if (note != 0)
			{
				int32 neue = periodTable[note];
				int32 alte = periodTable[voices[v].trackPeriod];

				alte -= neue;
				neue  = alte + voices[v].periodSlidePeriod;

				if (neue != 0)
					voices[v].periodSlideLimit = -alte;
			}

			voices[v].periodSlideOn        = true;
			voices[v].periodSlideWithLimit = true;
			goto NoNote;
		}
	}

	// Note kicking
	if (note != 0)
	{
		voices[v].trackPeriod = note;
		voices[v].plantPeriod = true;
	}

NoNote:
	switch (fx)
	{
		// Portamento up (Period slide down)
		case 0x1:
		{
			voices[v].periodSlideSpeed     = -fxParam;
			voices[v].periodSlideOn        = true;
			voices[v].periodSlideWithLimit = false;
			break;
		}

		// Portamento down (Period slide up)
		case 0x2:
		{
			voices[v].periodSlideSpeed     = fxParam;
			voices[v].periodSlideOn        = true;
			voices[v].periodSlideWithLimit = false;
			break;
		}

		// Volume
		case 0xc:
		{
			if (fxParam <= 0x40)
				voices[v].noteMaxVolume = fxParam;
			else
			{
				if (fxParam >= 0x50)
				{
					fxParam -= 0x50;
					if (fxParam <= 0x40)
					{
						for (int32 i = 0; i < 4; i++)
							voices[i].trackMasterVolume = fxParam;
					}
					else
					{
						fxParam -= (0xa0 - 0x50);
						if (fxParam <= 0x40)
							voices[v].trackMasterVolume = fxParam;
					}
				}
			}
			break;
		}

		// Enhanced commands
		case 0xe:
		{
			switch (fxParam >> 4)
			{
				// Fineslide up (Period fineslide down)
				case 0x1:
				{
					voices[v].periodSlidePeriod = -(fxParam & 0x0f);
					voices[v].plantPeriod       = true;
					break;
				}

				// Fineslide down (Period fineslide up)
				case 0x2:
				{
					voices[v].periodSlidePeriod = fxParam & 0x0f;
					voices[v].plantPeriod       = true;
					break;
				}

				// Vibrato control
				case 0x4:
				{
					voices[v].vibratoDepth = fxParam & 0x0f;
					break;
				}

				// Finevolume up
				case 0xa:
				{
					voices[v].noteMaxVolume += fxParam & 0x0f;

					if (voices[v].noteMaxVolume > 0x40)
						voices[v].noteMaxVolume = 0x40;

					break;
				}

				// Finevolume down
				case 0xb:
				{
					voices[v].noteMaxVolume -= fxParam & 0x0f;

					if (voices[v].noteMaxVolume < 0)
						voices[v].noteMaxVolume = 0;

					break;
				}
			}
			break;
		}
	}
}



/******************************************************************************/
/* ProcessFrame()                                                             */
/******************************************************************************/
void AHX::ProcessFrame(int32 v)
{
	if (!voices[v].trackOn)
		return;

	if (voices[v].noteDelayOn)
	{
		if (voices[v].noteDelayWait <= 0)
			ProcessStep(v);
		else
			voices[v].noteDelayWait--;
	}

	if (voices[v].hardCut != 0)
	{
		int32 nextInstrument;

		if ((noteNr + 1) < song->trackLength)
			nextInstrument = song->tracks[voices[v].track][noteNr + 1].instrument;
		else
			nextInstrument = song->tracks[voices[v].nextTrack][0].instrument;

		if (nextInstrument != 0)
		{
			int32 d1 = tempo - voices[v].hardCut;

			if (d1 < 0)
				d1 = 0;

			if (!voices[v].noteCutOn)
			{
				voices[v].noteCutOn       = true;
				voices[v].noteCutWait     = d1;
				voices[v].hardCutReleaseF = -(d1 - tempo);
			}
			else
				voices[v].hardCut = 0;
		}
	}

	if (voices[v].noteCutOn)
	{
		if (voices[v].noteCutWait <= 0)
		{
			voices[v].noteCutOn = false;

			if (voices[v].hardCutRelease)
			{
				voices[v].adsr.rVolume = -(voices[v].adsrVolume - (voices[v].instrument->envelope.rVolume << 8)) / voices[v].hardCutReleaseF;
				voices[v].adsr.rFrames = voices[v].hardCutReleaseF;
				voices[v].adsr.aFrames = 0;
				voices[v].adsr.dFrames = 0;
				voices[v].adsr.sFrames = 0;
			}
			else
				voices[v].noteMaxVolume = 0;
		}
		else
			voices[v].noteCutWait--;
	}

	// adsrEnvelope
	if (voices[v].adsr.aFrames != 0)
	{
		voices[v].adsrVolume += voices[v].adsr.aVolume;	// Delta

		if (--voices[v].adsr.aFrames <= 0)
			voices[v].adsrVolume = voices[v].instrument->envelope.aVolume << 8;
	}
	else
	{
		if (voices[v].adsr.dFrames != 0)
		{
			voices[v].adsrVolume += voices[v].adsr.dVolume;	// Delta

			if (--voices[v].adsr.dFrames <= 0)
				voices[v].adsrVolume = voices[v].instrument->envelope.dVolume << 8;
		}
		else
		{
			if (voices[v].adsr.sFrames != 0)
				voices[v].adsr.sFrames--;
			else
			{
				if (voices[v].adsr.rFrames != 0)
				{
					voices[v].adsrVolume += voices[v].adsr.rVolume;	// Delta

					if (--voices[v].adsr.rFrames <= 0)
						voices[v].adsrVolume = voices[v].instrument->envelope.rVolume << 8;
				}
			}
		}
	}

	// VolumeSlide
	voices[v].noteMaxVolume = voices[v].noteMaxVolume + voices[v].volumeSlideUp - voices[v].volumeSlideDown;

	if (voices[v].noteMaxVolume < 0)
		voices[v].noteMaxVolume = 0;

	if (voices[v].noteMaxVolume > 0x40)
		voices[v].noteMaxVolume = 0x40;

	// Portamento
	if (voices[v].periodSlideOn)
	{
		if (voices[v].periodSlideWithLimit)
		{
			int32 d0 = voices[v].periodSlidePeriod - voices[v].periodSlideLimit;
			int32 d2 = voices[v].periodSlideSpeed;

			if (d0 > 0)
				d2 = -d2;

			if (d0 != 0)
			{
				int32 d3 = (d0 + d2) ^ d0;

				if (d3 >= 0)
					d0 = voices[v].periodSlidePeriod + d2;
				else
					d0 = voices[v].periodSlideLimit;

				voices[v].periodSlidePeriod = d0;
				voices[v].plantPeriod       = true;
			}
		}
		else
		{
			voices[v].periodSlidePeriod += voices[v].periodSlideSpeed;
			voices[v].plantPeriod        = true;
		}
	}

	// Vibrato
	if (voices[v].vibratoDepth != 0)
	{
		if (voices[v].vibratoDelay <= 0)
		{
			voices[v].vibratoPeriod  = (vibratoTable[voices[v].vibratoCurrent] * voices[v].vibratoDepth) >> 7;
			voices[v].plantPeriod    = true;
			voices[v].vibratoCurrent = (voices[v].vibratoCurrent + voices[v].vibratoSpeed) & 0x3f;
		}
		else
			voices[v].vibratoDelay--;
	}

	// PList
	if ((voices[v].instrument != NULL) && (voices[v].perfCurrent < voices[v].instrument->playList.length))
	{
		if (--voices[v].perfWait <= 0)
		{
			int32 cur = voices[v].perfCurrent++;
			voices[v].perfWait = voices[v].perfSpeed;

			if (voices[v].perfList->entries[cur].waveform != 0)
			{
				voices[v].waveform              = voices[v].perfList->entries[cur].waveform - 1;
				voices[v].newWaveform           = true;
				voices[v].periodPerfSlideSpeed  = 0;
				voices[v].periodPerfSlidePeriod = 0;
			}

			// Holdwave
			voices[v].periodPerfSlideOn = false;

			for (int32 i = 0; i < 2; i++)
				PListCommandParse(v, voices[v].perfList->entries[cur].fx[i], voices[v].perfList->entries[cur].fxParam[i]);

			// GetNote
			if (voices[v].perfList->entries[cur].note != 0)
			{
				voices[v].instrPeriod = voices[v].perfList->entries[cur].note;
				voices[v].plantPeriod = true;
				voices[v].fixedNote   = voices[v].perfList->entries[cur].fixed;
			}
		}
	}
	else
	{
		if (voices[v].perfWait != 0)
			voices[v].perfWait--;
		else
			voices[v].periodPerfSlideSpeed = 0;
	}

	// PerfPortamento
	if (voices[v].periodPerfSlideOn)
	{
		voices[v].periodPerfSlidePeriod -= voices[v].periodPerfSlideSpeed;

		if (voices[v].periodPerfSlidePeriod != 0)
			voices[v].plantPeriod = true;
	}

	if ((voices[v].waveform == 3 - 1) && voices[v].squareOn)
	{
		if (--voices[v].squareWait <= 0)
		{
			int32 d1 = voices[v].squareLowerLimit;
			int32 d2 = voices[v].squareUpperLimit;
			int32 d3 = voices[v].squarePos;

			if (voices[v].squareInit)
			{
				voices[v].squareInit = false;

				if (d3 <= d1)
				{
					voices[v].squareSlidingIn = true;
					voices[v].squareSign      = 1;
				}
				else
				{
					if (d3 >= d2)
					{
						voices[v].squareSlidingIn = true;
						voices[v].squareSign      = -1;
					}
				}
			}

			// NoSquareInit
			if ((d1 == d3) || (d2 == d3))
			{
				if (voices[v].squareSlidingIn)
					voices[v].squareSlidingIn = false;
				else
					voices[v].squareSign = -voices[v].squareSign;
			}

			d3 += voices[v].squareSign;
			voices[v].squarePos   = d3;
			voices[v].plantSquare = true;
			voices[v].squareWait  = voices[v].instrument->squareSpeed;
		}
	}

	if (voices[v].filterOn && (--voices[v].filterWait <= 0))
	{
		int32 d1 = voices[v].filterLowerLimit;
		int32 d2 = voices[v].filterUpperLimit;
		int32 d3 = voices[v].filterPos;

		if (voices[v].filterInit)
		{
			voices[v].filterInit = false;

			if (d3 <= d1)
			{
				voices[v].filterSlidingIn = true;
				voices[v].filterSign      = 1;
			}
			else
			{
				if (d3 >= d2)
				{
					voices[v].filterSlidingIn = true;
					voices[v].filterSign      = -1;
				}
			}
		}

		// NoFilterInit
		int32 fMax = (voices[v].filterSpeed < 3) ? (5 - voices[v].filterSpeed) : 1;

		for (int32 i = 0; i < fMax; i++)
		{
			if ((d1 == d3) || (d2 == d3))
			{
				if (voices[v].filterSlidingIn)
					voices[v].filterSlidingIn = false;
				else
					voices[v].filterSign = -voices[v].filterSign;
			}

			d3 += voices[v].filterSign;
		}

		// Check by Thomas Neumann
		if (d3 > 63)
			d3 = 63;
		else if (d3 < 0)
			d3 = 0;

		voices[v].filterPos   = d3;
		voices[v].newWaveform = true;
		voices[v].filterWait  = voices[v].filterSpeed - 3;

		if (voices[v].filterWait < 1)
			voices[v].filterWait = 1;
	}

	if ((voices[v].waveform == 3 - 1) || voices[v].plantSquare)
	{
		// CalcSquare
		ASSERT((voices[v].filterPos >= 0) && (voices[v].filterPos <= 63));
		int8 *squarePtr = &waves->squares[(voices[v].filterPos - 0x20) * (0xfc + 0xfc + 0x80 * 0x1f + 0x80 + 0x280 * 3)];
		int32 x = voices[v].squarePos << (5 - voices[v].waveLength);

		if (x > 0x20)
		{
			x = 0x40 - x;
			voices[v].squareReverse = true;
		}

		// OkDownSquare
		if (--x)
			squarePtr += x << 7;

		int32 delta    = 32 >> voices[v].waveLength;
		waveformTab[2] = voices[v].squareTempBuffer;

		for (int32 i = 0; i < (1 << voices[v].waveLength) * 4; i++)
		{
			voices[v].squareTempBuffer[i] = *squarePtr;
			squarePtr += delta;
		}

		voices[v].newWaveform = true;
		voices[v].waveform    = 3 - 1;
		voices[v].plantSquare = false;
	}

	if (voices[v].waveform == 4 - 1)
		voices[v].newWaveform = true;

	if (voices[v].newWaveform)
	{
		int8 *audioSource = waveformTab[voices[v].waveform];

		if (voices[v].waveform != 3 - 1)
		{
			ASSERT((voices[v].filterPos >= 0) && (voices[v].filterPos <= 63));
			audioSource += (voices[v].filterPos - 0x20) * (0xfc + 0xfc + 0x80 * 0x1f + 0x80 + 0x280 * 3);
		}

		if (voices[v].waveform < 3 - 1)
		{
			// GetWLWaveformlor2
			audioSource += offsetsTable[voices[v].waveLength];
		}

		if (voices[v].waveform == 4 - 1)
		{
			// AddRandomMoving
			audioSource += (wnRandom & (2 * 0x280 - 1)) & ~1;

			// GoOnRandom
			wnRandom += 2239384;
			wnRandom  = ((((wnRandom >> 8) | (wnRandom << 24)) + 782323) ^ 75) - 6735;
		}

		voices[v].audioSource = audioSource;
	}

	// StillHoldWaveform
	// AudioInitPeriod
	voices[v].audioPeriod = voices[v].instrPeriod;

	if (!voices[v].fixedNote)
		voices[v].audioPeriod += voices[v].transpose + voices[v].trackPeriod - 1;

	if (voices[v].audioPeriod > 5 * 12)
		voices[v].audioPeriod = 5 * 12;

	if (voices[v].audioPeriod < 0)
		voices[v].audioPeriod = 0;

	voices[v].audioPeriod = periodTable[voices[v].audioPeriod];

	if (!voices[v].fixedNote)
		voices[v].audioPeriod += voices[v].periodSlidePeriod;

	voices[v].audioPeriod += voices[v].periodPerfSlidePeriod + voices[v].vibratoPeriod;

	if (voices[v].audioPeriod > 0x0d60)
		voices[v].audioPeriod = 0x0d60;

	if (voices[v].audioPeriod < 0x0071)
		voices[v].audioPeriod = 0x0071;

	// AudioInitVolume
	voices[v].audioVolume = ((((((((voices[v].adsrVolume >> 8) * voices[v].noteMaxVolume) >> 6) * voices[v].perfSubVolume) >> 6) * voices[v].trackMasterVolume) >> 6) * mainVolume) >> 6;
}



/******************************************************************************/
/* PListCommandParse()                                                        */
/******************************************************************************/
void AHX::PListCommandParse(int32 v, int32 fx, int32 fxParam)
{
	switch (fx)
	{
		case 0:
		{
			if ((song->revision > 0) && (fxParam != 0))
			{
				if (voices[v].ignoreFilter)
				{
					voices[v].filterPos    = 1;//voices[v].ignoreFilter;
					voices[v].ignoreFilter = false;
				}
				else
				{
					// Check by Thomas Neumann
					if (fxParam > 63)
						fxParam = 63;
					else if (fxParam < 0)
						fxParam = 0;

					voices[v].filterPos = fxParam;
				}

				voices[v].newWaveform = true;
			}
			break;
		}

		case 1:
		{
			voices[v].periodPerfSlideSpeed = fxParam;
			voices[v].periodPerfSlideOn    = true;
			break;
		}

		case 2:
		{
			voices[v].periodPerfSlideSpeed = -fxParam;
			voices[v].periodPerfSlideOn    = true;
			break;
		}

		case 3:		// Init Square Modulation
		{
			if (!voices[v].ignoreSquare)
				voices[v].squarePos = fxParam >> (5 - voices[v].waveLength);
			else
				voices[v].ignoreSquare = false;

			break;
		}

		case 4:		// Start/Stop Modulation
		{
			if ((song->revision == 0) || (fxParam == 0))
			{
				voices[v].squareOn   = voices[v].squareOn ? false : true;
				voices[v].squareInit = voices[v].squareOn;
				voices[v].squareSign = 1;
			}
			else
			{
				if ((fxParam & 0x0f) != 0x00)
				{
					voices[v].squareOn   = voices[v].squareOn ? false : true;
					voices[v].squareInit = voices[v].squareOn;
					voices[v].squareSign = 1;

					if ((fxParam & 0x0f) == 0x0f)
						voices[v].squareSign = -1;
				}

				if ((fxParam & 0xf0) != 0x00)
				{
					voices[v].filterOn   = voices[v].filterOn ? false : true;
					voices[v].filterInit = voices[v].filterOn;
					voices[v].filterSign = 1;

					if ((fxParam & 0xf0) == 0xf0)
						voices[v].filterSign = -1;
				}
			}
			break;
		}

		case 5:		// Jump to Step [xx]
		{
			voices[v].perfCurrent = fxParam;
			break;
		}

		case 6:		// Set Volume
		{
			if (fxParam > 0x40)
			{
				if ((fxParam -= 0x50) >= 0)
				{
					if (fxParam <= 0x40)
						voices[v].perfSubVolume = fxParam;
					else
					{
						if ((fxParam -= 0xa0 - 0x50) >= 0)
						{
							if (fxParam <= 0x40)
								voices[v].trackMasterVolume = fxParam;
						}
					}
				}
			}
			else
				voices[v].noteMaxVolume = fxParam;

			break;
		}

		case 7:		// Set speed
		{
			voices[v].perfSpeed = fxParam;
			voices[v].perfWait  = fxParam;
			break;
		}
	}
}



/******************************************************************************/
/* SetAudio()                                                                 */
/******************************************************************************/
void AHX::SetAudio(int32 v)
{
	if (!voices[v].trackOn)
	{
		voices[v].voiceVolume = 0;
		return;
	}

	voices[v].voiceVolume = voices[v].audioVolume;

	if (voices[v].plantPeriod)
	{
		voices[v].plantPeriod = false;
		voices[v].voicePeriod = voices[v].audioPeriod;
	}

	if (voices[v].newWaveform)
	{
		if (voices[v].waveform == 4 - 1)
			memcpy(voices[v].voiceBuffer, voices[v].audioSource, 0x280);
		else
		{
			int32 waveLoops = (1 << (5 - voices[v].waveLength)) * 5;

			for (int32 i = 0; i < waveLoops; i++)
				memcpy(&voices[v].voiceBuffer[i * 4 * (1 << voices[v].waveLength)], voices[v].audioSource, 4 * (1 << voices[v].waveLength));
		}

		voices[v].voiceBuffer[0x280] = voices[v].voiceBuffer[0];
	}
}
