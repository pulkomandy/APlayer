/******************************************************************************/
/* SIDEmuEngine implementation file.                                          */
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
#include "SIDTune.h"
#include "SID6581.h"
#include "SID6510.h"
#include "SIDEmuEngine.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDEmuEngine::SIDEmuEngine(void) : sid6581(&sid6510)
{
	// Initialize member variables
	signedPanMix8  = NULL;
	signedPanMix16 = NULL;

	// Initialize mixer function pointers
	fillFunctions[0][0][0] = NULL;				// Fill8BitMono
	fillFunctions[0][0][1] = NULL;				// Fill8BitSplit
	fillFunctions[0][0][2] = NULL;				// Fill8BitMonoControl
	fillFunctions[0][0][3] = NULL;				// Fill8BitMonoControl
	fillFunctions[0][1][0] = NULL;				// Fill8BitStereo
	fillFunctions[0][1][1] = NULL;				// Fill8BitSplit
	fillFunctions[0][1][2] = NULL;				// Fill8BitStereoControl
	fillFunctions[0][1][3] = NULL;				// Fill8BitStereoSurround
	fillFunctions[1][0][0] = NULL;				// Fill16BitMono
	fillFunctions[1][0][1] = Fill16BitSplit;
	fillFunctions[1][0][2] = NULL;				// Fill16BitMonoControl
	fillFunctions[1][0][3] = NULL;				// Fill16BitMonoControl
	fillFunctions[1][1][0] = NULL;				// Fill16bitStereo
	fillFunctions[1][1][1] = Fill16BitSplit;
	fillFunctions[1][1][2] = NULL;				// Fill16BitStereoControl
	fillFunctions[1][1][3] = NULL;				// Fill16BitStereoSurround

	// Set the defaults
	config.frequency       = 44100;
	config.bitsPerSample   = SIDEMU_16BIT;
	config.sampleFormat    = SIDEMU_SIGNED_PCM;
	config.channels        = SIDEMU_STEREO;			//SIDEMU_MONO;
	config.sidChips        = 1;
	config.volumeControl   = SIDEMU_HWMIXING;		//SIDEMU_NONE;
	config.mos8580         = false;
	config.measuredVolume  = true;
	config.digiPlayerScans = 10 * 50;
	config.emulateFilter   = true;
	config.autoPanning     = SIDEMU_NONE;
	config.memoryMode      = MPU_BANK_SWITCHING;
	config.clockSpeed      = SIDTUNE_CLOCK_PAL;
	config.forceSongSpeed  = false;

	// Reset data counter
	bytesCountTotal = (bytesCountSong = 0);
	secondsTotal    = (secondsThisSong = 0);

	isThreeVoiceTune = false;

	sid6581.EmuResetAutoPanning(config.autoPanning);

	// Allocate memory for the interpreter
	sid6510.C64MemFree();
	mpuStatus = sid6510.C64MemAlloc();

	// Allocate memory for the SID emulator engine
	FreeMem();

	if (mpuStatus && AllocMem())
	{
		SetRandomSeed();
		MPUReset();
		ConfigureSID();
		InitMixerEngine();
		SetDefaultVoiceVolumes();
		SetDefaultFilterStrength();
		Reset();
		isReady = true;
	}
	else
		isReady = false;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDEmuEngine::~SIDEmuEngine(void)
{
	sid6510.C64MemFree();
	FreeMem();
}



/******************************************************************************/
/* GetConfig() copy the current configuration to the structure given.         */
/*                                                                            */
/* Input:  "outCfg" is a reference to the structure to copy the configuration */
/*         into.                                                              */
/******************************************************************************/
void SIDEmuEngine::GetConfig(sidEmuConfig &outCfg)
{
	outCfg = config;
}



/******************************************************************************/
/* SetConfig() will change the emulator settings.                             */
/*                                                                            */
/* Input:  "inCfg" is a reference to the structure with the new settings.     */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool SIDEmuEngine::SetConfig(const sidEmuConfig &inCfg)
{
	bool gotInvalidConfig = false;

	// Validate input value
	if ((inCfg.memoryMode == MPU_BANK_SWITCHING) || (inCfg.memoryMode == MPU_TRANSPARENT_ROM) || (inCfg.memoryMode == MPU_PLAYSID_ENVIRONMENT))
		config.memoryMode = inCfg.memoryMode;
	else
		gotInvalidConfig = true;	// Invalid settings

	// Validate input value
	// Check various settings before doing a single SID-config call
	bool newSIDconfig = false;
	bool newFilterInit = false;

	if ((inCfg.clockSpeed == SIDTUNE_CLOCK_PAL) || (inCfg.clockSpeed == SIDTUNE_CLOCK_NTSC))
	{
		if (inCfg.clockSpeed != config.clockSpeed)
		{
			config.clockSpeed = inCfg.clockSpeed;
			newSIDconfig = true;
		}
	}
	else
		gotInvalidConfig = true;	// Invalid settings

	if (inCfg.forceSongSpeed != config.forceSongSpeed)
		config.forceSongSpeed = (inCfg.forceSongSpeed == true);

	// Range-check the sample frequency
	if ((inCfg.frequency >= 4000) && (inCfg.frequency <= 48000))
	{
		// Has it changed? Then do the necessary initialization
		if (inCfg.frequency != config.frequency)
		{
			config.frequency = inCfg.frequency;
			newSIDconfig = true;
			newFilterInit = true;
		}
	}
	else
		gotInvalidConfig = true;	// Invalid settings

	if (inCfg.measuredVolume != config.measuredVolume)
	{
		config.measuredVolume = (inCfg.measuredVolume == true);
		newSIDconfig = true;
	}

	// The mixer mode, the sample format, the number of channels and bits per
	// sample all affect the mixing tables and settings.
	// Hence we define a handy flag here
	bool newMixerSettings = false;

	// Is the requested sample format valid?
	if ((inCfg.sampleFormat == SIDEMU_UNSIGNED_PCM) || (inCfg.sampleFormat == SIDEMU_SIGNED_PCM))
	{
		// Has it changed? Then do the necessary initialization
		if (inCfg.sampleFormat != config.sampleFormat)
		{
			config.sampleFormat = inCfg.sampleFormat;
			newMixerSettings = true;
		}
	}
	else
		gotInvalidConfig = true;	// Invalid settings

	// Is the requested number of channels valid?
	if ((inCfg.channels == SIDEMU_MONO) || (inCfg.channels == SIDEMU_STEREO))
	{
		// Has it changed? Then do the necessary initialization
		if (inCfg.channels != config.channels)
		{
			config.channels = inCfg.channels;
			SetDefaultVoiceVolumes();
			newMixerSettings = true;
		}
	}
	else
		gotInvalidConfig = true;	// Invalid settings

	// Is the requested sample precision valid?
	if ((inCfg.bitsPerSample == SIDEMU_8BIT) || (inCfg.bitsPerSample == SIDEMU_16BIT))
	{
		// Has it changed? Then do the necessary initialization
		if (inCfg.bitsPerSample != config.bitsPerSample)
		{
			config.bitsPerSample = inCfg.bitsPerSample;
			newMixerSettings = true;
		}
	}
	else
		gotInvalidConfig = false;	// Invalid settings

	// Is the requested mixing mode valid?
	if ((inCfg.volumeControl == SIDEMU_NONE) || (inCfg.volumeControl == SIDEMU_VOLCONTROL) ||
		(inCfg.volumeControl == SIDEMU_FULLPANNING) || (inCfg.volumeControl == SIDEMU_HWMIXING) ||
		(inCfg.volumeControl == SIDEMU_STEREOSURROUND))
	{
		// Has it changed? Then do the necessary initialization
		if (inCfg.volumeControl != config.volumeControl)
		{
			config.volumeControl = inCfg.volumeControl;
			SetDefaultVoiceVolumes();
			newMixerSettings = true;
		}
	}
	else
		gotInvalidConfig = true;	// Invalid settings

	if ((inCfg.autoPanning == SIDEMU_NONE) || (inCfg.autoPanning == SIDEMU_CENTEREDAUTOPANNING))
	{
		if (inCfg.autoPanning != config.autoPanning)
		{
			config.autoPanning = inCfg.autoPanning;
			if (config.autoPanning != SIDEMU_NONE)
			{
				if ((config.volumeControl != SIDEMU_FULLPANNING) && (config.volumeControl != SIDEMU_STEREOSURROUND))
				{
					config.autoPanning = false;
					gotInvalidConfig = true;	// Wrong mixing mode
				}
			}

			sid6581.EmuResetAutoPanning(config.autoPanning);
		}
	}
	else
		gotInvalidConfig = true;	// Invalid panning mode

	if (inCfg.emulateFilter != config.emulateFilter)
	{
		config.emulateFilter = (inCfg.emulateFilter == true);
		newSIDconfig = true;		// Filter
		newMixerSettings = true;	// Amplification
	}

	// Range-check the filter settings
	if ((inCfg.filterFs >= 1.0f) && (inCfg.filterFm != 0.0f))
	{
		// Have they changed? Then do the necessary initialization
		if ((inCfg.filterFs != config.filterFs) || (inCfg.filterFm != config.filterFm) || (inCfg.filterFt != config.filterFt))
		{
			config.filterFs = inCfg.filterFs;
			config.filterFm = inCfg.filterFm;
			config.filterFt = inCfg.filterFt;
			newFilterInit = true;
		}
	}
	else
		gotInvalidConfig = true;	// Invalid settings

	if (inCfg.digiPlayerScans != config.digiPlayerScans)
	{
		config.digiPlayerScans = inCfg.digiPlayerScans;
		newMixerSettings = true;	// Extra amplification
	}

	if ((config.channels==SIDEMU_MONO) && ((config.volumeControl==SIDEMU_STEREOSURROUND) || (config.autoPanning!=SIDEMU_NONE)))
		gotInvalidConfig = true;	// Invalid settings

	if (inCfg.mos8580 != config.mos8580)
	{
		config.mos8580 = (inCfg.mos8580 == true);
		newSIDconfig = true;
	}

	// Here re-initialize the SID, if required
	if (newSIDconfig)
		ConfigureSID();

	// Here re-initialize the mixer engine, if required
	if (newMixerSettings)
		InitMixerEngine();

	// Here re-initialize the filter settings, if required
	if (newFilterInit)
		FilterTableInit();

	// Return flag, whether input config was valid
	return (!gotInvalidConfig);
}



/******************************************************************************/
/* AmplifyThreeVoiceTunes() specifies whether a sidtune uses only three       */
/*      voices (here: no digis).                                              */
/******************************************************************************/
void SIDEmuEngine::AmplifyThreeVoiceTunes(bool inIsThreeVoiceTune)
{
	isThreeVoiceTune = inIsThreeVoiceTune;
}



/******************************************************************************/
/* ResetSecondsThisSong()                                                     */
/******************************************************************************/
void SIDEmuEngine::ResetSecondsThisSong(void)
{
	secondsThisSong = 0;
}



/******************************************************************************/
/* MPUReset()                                                                 */
/******************************************************************************/
void SIDEmuEngine::MPUReset(void)
{
	if (mpuStatus)
	{
		sid6510.InitInterpreter(config.memoryMode);
		sid6510.C64MemClear();
		sid6510.C64MemReset(config.clockSpeed, randomSeed);
	}
}



/******************************************************************************/
/* MPUReturnRAMBase()                                                         */
/******************************************************************************/
uint8 *SIDEmuEngine::MPUReturnRAMBase(void)
{
	if (mpuStatus)
		return (sid6510.c64Mem1);
	else
		return (NULL);
}



/******************************************************************************/
/* Reset()                                                                    */
/******************************************************************************/
void SIDEmuEngine::Reset(void)
{
	sid6581.EmuReset();
	ResetSampleEmu();
}



/******************************************************************************/
/* SetRandomSeed() sets a random (!) random seed value.                       */
/******************************************************************************/
void SIDEmuEngine::SetRandomSeed(void)
{
	time_t now = time(NULL);
	randomSeed = (uint8)now;
}



/******************************************************************************/
/* ResetSampleEmu()                                                           */
/******************************************************************************/
bool SIDEmuEngine::ResetSampleEmu(void)
{
	sid6581.samples.SampleEmuReset();
	return (true);
}



/******************************************************************************/
/* AllocMem()                                                                 */
/******************************************************************************/
bool SIDEmuEngine::AllocMem(void)
{
	// Keep track of memory allocation failures
	bool wasSuccess = true;

	// Seems as if both tables are needed for panning-mixing with 16-bit samples
	// 8-bit
	if ((sid6581.ampMod1x8 = new int8[256 * 256]) == NULL)
		wasSuccess = false;

	if ((signedPanMix8 = new int8[256 * 256]) == NULL)
		wasSuccess = false;

	// 16-bit
	if ((signedPanMix16 = new int16[256 * 256]) == NULL)
		wasSuccess = false;

	if (!wasSuccess)
		FreeMem();

	return (wasSuccess);
}



/******************************************************************************/
/* FreeMem()                                                                  */
/******************************************************************************/
bool SIDEmuEngine::FreeMem(void)
{
	delete[] sid6581.ampMod1x8;
	sid6581.ampMod1x8 = NULL;

	delete[] signedPanMix8;
	signedPanMix8 = NULL;

	delete[] signedPanMix16;
	signedPanMix16 = NULL;

	return (true);
}



/******************************************************************************/
/* ConfigureSID() initialize the SID chip and everything that depends on the  */
/*      frequency.                                                            */
/******************************************************************************/
void SIDEmuEngine::ConfigureSID(void)
{
	sid6581.EmuConfigure(config.frequency, config.measuredVolume, config.mos8580, config.emulateFilter, config.clockSpeed);
}



/******************************************************************************/
/* InitMixerEngine()                                                          */
/******************************************************************************/
void SIDEmuEngine::InitMixerEngine(void)
{
	uint16 uk;

	// If just three (instead of four) voices, do different amplification,
	// if that is desired (digiPlayerScans != 0)
	if ((config.digiPlayerScans !=0 ) && isThreeVoiceTune)
		isThreeVoiceAmplified = true;
	else
		isThreeVoiceAmplified = false;

	int32 si, sj;

	// 8-bit volume modulation tables
	float filterAmpl = 1.0f;

	if (config.emulateFilter)
		filterAmpl = 0.7f;

	uk = 0;
	for (si = 0; si < 256; si++)
	{
		for (sj = -128; sj < 128; sj++, uk++)
			sid6581.ampMod1x8[uk] = (int8)(((si * sj) / 255) * filterAmpl);
	}

	// Determine single-voice de-amplification
	float ampDiv;	// Logical voices per physical channel

	if (config.volumeControl == SIDEMU_HWMIXING)
		ampDiv = 1.0f;
	else if ((config.channels == SIDEMU_STEREO) &&
			 ((config.volumeControl == SIDEMU_NONE)
			  || (config.volumeControl == SIDEMU_VOLCONTROL)))
	{
		ampDiv = 2.0f;
	}
	else	// SIDEMU_MONO or SIDEMU_FULLPANNING or SIDEMU_STEREOSURROUND
	{
		if (isThreeVoiceAmplified)
			ampDiv = 3.0f;
		else
			ampDiv = 4.0f;
	}

	uk = 0;
	for (si = 0; si < 256; si++)
	{
		for (sj = -128; sj < 128; sj++, uk++)
		{
			// 8-bit mixing modulation tables
			signedPanMix8[uk] = (int8)(((si * sj) / 255) / ampDiv);

			// 16-bit mixing modulation tables
			signedPanMix16[uk] = (uint16)((si * sj) / ampDiv);
		}
	}

	int32 bitsIndex, monoIndex, controlIndex;

	// Define the ``zero'' sample for signed or unsigned samples
	uint8 zero8bit = 0x80;
	uint16 zero16bit = 0;

	if (config.bitsPerSample == SIDEMU_16BIT)
	{
		bitsIndex = 1;

		switch (config.sampleFormat)
		{
			// Waveform and amplification tables are signed samples,
			// so adjusting the sign should do the conversion
			case SIDEMU_SIGNED_PCM:
				zero16bit = 0;
				break;

			case SIDEMU_UNSIGNED_PCM:
			default:
				zero16bit = 0x8000;
				break;
		}
	}
	else	// if ( config.bitsPerSample == SIDEMU_8BIT )
	{
		bitsIndex = 0;

		switch (config.sampleFormat)
		{
			// Waveform and amplification tables are signed samples,
			// so adjusting the sign should do the conversion
			case SIDEMU_SIGNED_PCM:
				zero8bit = 0;
				break;

			case SIDEMU_UNSIGNED_PCM:
			default:
				zero8bit = 0x80;
				break;
		}
	}

	switch (config.channels)
	{
		case SIDEMU_MONO:
			monoIndex = 0;
			break;

		case SIDEMU_STEREO:
		default:
			monoIndex = 1;
			break;
	}

	if (config.volumeControl == SIDEMU_NONE)
		controlIndex = 0;
	else if (config.volumeControl == SIDEMU_HWMIXING)
		controlIndex = 1;
	else if (config.volumeControl == SIDEMU_STEREOSURROUND)
		controlIndex = 3;
	else
		controlIndex = 2;

	sidEmuFillFunc = fillFunctions[bitsIndex][monoIndex][controlIndex];

	// Call a function which inits more local tables
	MixerInit(isThreeVoiceAmplified, zero8bit, zero16bit);

	// Ensure that samplebuffer will be divided into
	// correct number of samples
	//  8-bit mono: buflen x = x samples
	//      stereo:          = x/2 samples
	// 16-bit mono: buflen x = x/2 samples
	//      stereo:          = x/4 samples
	sid6581.bufferScale = 0;

	// HWMIXING mode does not know about stereo
	if ((config.channels == SIDEMU_STEREO)
		&& !(config.volumeControl == SIDEMU_HWMIXING))
		sid6581.bufferScale++;

	if (config.bitsPerSample == SIDEMU_16BIT)
		sid6581.bufferScale++;
}



/******************************************************************************/
/* SetDefaultVoiceVolumes()                                                   */
/******************************************************************************/
void SIDEmuEngine::SetDefaultVoiceVolumes(void)
{ 
	// Adjust default mixing gain. Does not matter, whether this will be used.
	// Signed 8-bit samples will be added to base array index.
	// So middle must be 0x80.
	// [-80,-81,...,-FE,-FF,0,1,...,7E,7F]
	// Mono: left channel only used
	if (config.channels == SIDEMU_MONO)
	{
		SetVoiceVolume(1, 255, 0, 256);
		SetVoiceVolume(2, 255, 0, 256);
		SetVoiceVolume(3, 255, 0, 256);
		SetVoiceVolume(4, 255, 0, 256);
	}
	else	// if ( config.channels == SIDEMU_STEREO )
	{
		if (config.volumeControl == SIDEMU_STEREOSURROUND)
		{
			SetVoiceVolume(1, 255, 255, 256);
			SetVoiceVolume(2, 255, 255, 256);
			SetVoiceVolume(3, 255, 255, 256);
			SetVoiceVolume(4, 255, 255, 256);
		}
		else
		{
			SetVoiceVolume(1, 255, 0, 256);
			SetVoiceVolume(2, 0, 255, 256);
			SetVoiceVolume(3, 255, 0, 256);
			SetVoiceVolume(4, 0, 255, 256);
		}
	}
}



/******************************************************************************/
/* SetVoiceVolume()                                                           */
/******************************************************************************/
bool SIDEmuEngine::SetVoiceVolume(int32 voice, uint8 leftLevel, uint8 rightLevel, uint16 total)
{
	if (config.volumeControl == SIDEMU_NONE)
		return (false);

	if ((voice < 1) || (voice > 4) || (total > 256))
		return (false);

	if (config.channels == SIDEMU_MONO)
		rightLevel = 0;

	sid6581.EmuSetVoiceVolume(voice, leftLevel, rightLevel, total);
	return (true);
}



/******************************************************************************/
/* SetDefaultFilterStrength()                                                 */
/******************************************************************************/
void SIDEmuEngine::SetDefaultFilterStrength(void)
{
	config.filterFs = SIDEMU_DEFAULTFILTERFS;
	config.filterFm = SIDEMU_DEFAULTFILTERFM;
	config.filterFt = SIDEMU_DEFAULTFILTERFT;
	FilterTableInit();
}



/******************************************************************************/
/* FilterTableInit()                                                          */
/******************************************************************************/
void SIDEmuEngine::FilterTableInit(void)
{
	uint16 uk;

	// Parameter calculation has not been moved to a separate function
	// by purpose
	const float filterRefFreq = 44100.0f;

	float yMax = 1.0f;
	float yMin = 0.01f;
	uk = 0;

	for (float rk = 0; rk < 0x800; rk++)
	{
		sid6581.filterTable[uk] = (((exp(rk / 0x800 * log(config.filterFs)) / config.filterFm) + config.filterFt) * filterRefFreq) / config.frequency;

		if (sid6581.filterTable[uk] < yMin)
			sid6581.filterTable[uk] = yMin;

		if (sid6581.filterTable[uk] > yMax)
			sid6581.filterTable[uk] = yMax;

		uk++;
	}

	yMax = 0.22f;
	yMin = 0.05f;	// Less for some R1/R4 chips
	float yAdd = (yMax - yMin) / 2048.0f;
	float yTmp = yMin;
	uk = 0;

	// Some C++ compilers still have non-local scope!
	for (float rk2 = 0; rk2 < 0x800; rk2++)
	{
		sid6581.bandPassParam[uk] = (yTmp * filterRefFreq) / config.frequency;
		yTmp += yAdd;
		uk++;
	}

	float resDyMax = 1.0f;
	float resDyMin = 2.0f;
	float resDy = resDyMin;
	for (uk = 0; uk < 16; uk++)
	{
		sid6581.filterResTable[uk] = resDy;
		resDy -= ((resDyMin - resDyMax) / 15);
	}

	sid6581.filterResTable[0]  = resDyMin;
	sid6581.filterResTable[15] = resDyMax;
}



/******************************************************************************/
/* MixerInit()                                                                */
/******************************************************************************/
void SIDEmuEngine::MixerInit(bool threeVoiceAmplify, uint8 zero8, uint16 zero16)
{
	zero8Bit  = zero8;
	zero16Bit = zero16;

	int32 si;
	uint16 ui;

	int32 ampDiv = sidMaxLogicalVoices;
	if (threeVoiceAmplify)
		ampDiv = (sidMaxLogicalVoices - 1);

	// Mixing formulas are optimized by sample input value
	si = (-128 * sidMaxLogicalVoices);
	for (ui = 0; ui < sizeof(mix8Mono); ui++)
	{
		mix8Mono[ui] = (uint8)(si / ampDiv) + zero8Bit;
		si++;
	}

	si = (-128 * sidMaxLogicalVoices);		// Optimized by (/2 *2);
	for (ui = 0; ui < sizeof(mix8Stereo); ui++)
	{
		mix8Stereo[ui] = (uint8)(si / ampDiv) + zero8Bit;
		si += 2;
	}

	si = (-128 * sidMaxLogicalVoices) * 256;
	for (ui = 0; ui < sizeof(mix16Mono) / sizeof(uint16); ui++)
	{
		mix16Mono[ui] = (uint16)(si / ampDiv) + zero16Bit;
		si += 256;
	}

	si = (-128 * sidMaxLogicalVoices) * 256;	// Optimized by (/2 * 512)
	for (ui = 0; ui < sizeof(mix16Stereo) / sizeof(uint16); ui++)
	{
		mix16Stereo[ui] = (uint16)(si / ampDiv) + zero16Bit;
		si += 512;
	}
}



/******************************************************************************/
/* SyncEm()                                                                   */
/******************************************************************************/
inline void SIDEmuEngine::SyncEm(void)
{
	sid6581.optr1.cycleLenCount--;
	sid6581.optr2.cycleLenCount--;
	sid6581.optr3.cycleLenCount--;

	bool sync1 = (sid6581.optr1.modulator->cycleLenCount <= 0);
	bool sync2 = (sid6581.optr2.modulator->cycleLenCount <= 0);
	bool sync3 = (sid6581.optr3.modulator->cycleLenCount <= 0);

	if (sid6581.optr1.sync && sync1)
	{
		sid6581.optr1.cycleLenCount = 0;
		sid6581.optr1.outProc       = sid6581.optr1.WaveCalcNormal;
		sid6581.optr1.waveStep.l    = 0;
	}

	if (sid6581.optr2.sync && sync2)
	{
		sid6581.optr2.cycleLenCount = 0;
		sid6581.optr2.outProc       = sid6581.optr2.WaveCalcNormal;
		sid6581.optr2.waveStep.l    = 0;
	}

	if (sid6581.optr3.sync && sync3)
	{
		sid6581.optr3.cycleLenCount = 0;
		sid6581.optr3.outProc       = sid6581.optr3.WaveCalcNormal;
		sid6581.optr3.waveStep.l    = 0;
	}
}



/******************************************************************************/
/* Fill16BitSplit()                                                           */
/******************************************************************************/
void *SIDEmuEngine::Fill16BitSplit(SIDEmuEngine *obj, void *buffer, uint32 numberOfSamples)
{
	int16 *v1Buffer16Bit = (int16 *)buffer;
	int16 *v2Buffer16Bit = v1Buffer16Bit + obj->splitBufferLen;
	int16 *v3Buffer16Bit = v2Buffer16Bit + obj->splitBufferLen;
	int16 *v4Buffer16Bit = v3Buffer16Bit + obj->splitBufferLen;

	for ( ; numberOfSamples > 0; numberOfSamples--)
	{
		*v1Buffer16Bit++ = obj->zero16Bit + ((*obj->sid6581.optr1.outProc)(&obj->sid6581.optr1) << 8);
		*v2Buffer16Bit++ = obj->zero16Bit + ((*obj->sid6581.optr2.outProc)(&obj->sid6581.optr2) << 8);
		*v3Buffer16Bit++ = obj->zero16Bit + ((*obj->sid6581.optr3.outProc)(&obj->sid6581.optr3) << 8);
		*v4Buffer16Bit++ = obj->zero16Bit + ((*obj->sid6581.samples.sampleEmuRout)(&obj->sid6581.samples) << 8);
		obj->SyncEm();
	}

	return (v1Buffer16Bit);
}
