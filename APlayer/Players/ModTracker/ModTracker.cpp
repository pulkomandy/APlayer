/******************************************************************************/
/* ModTracker Player Interface.                                               */
/*                                                                            */
/* Original players by:                                                       */
/*     SoundTracker15: Karsten Obarski.                                       */
/*     SoundTracker31: Unknown of D.O.C.                                      */
/*     NoiseTracker  : Mahoney & Kaktus.                                      */
/*     StarTrekker   : Bjorn Wesen (Exolon).                                  */
/*     StarTrekker 8 : Bjorn Wesen (Exolon).                                  */
/*     ProTracker    : Lars Hamre.                                            */
/*     FastTracker   : Fredrik Muss.                                          */
/*     TakeTracker   : Anders B. Ervik (Dr. Zon) & Oyvind Neuman (Twaddler).  */
/*     MultiTracker  : Daniel Goldstein.                                      */
/*                                                                            */
/* Converted to C++ by Thomas Neumann.                                        */
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
#include "PSystem.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APChannel.h"

// Player headers
#include "ModTracker.h"
#include "Tables.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.28f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ModTracker::ModTracker(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	samples     = NULL;
	tracks      = NULL;
	sequences   = NULL;
	amData      = NULL;
	channels    = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ModTracker::~ModTracker(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float ModTracker::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetCount() returns the number of add-ons in the add-on.                    */
/*                                                                            */
/* Output: Is the number of the add-ons.                                      */
/******************************************************************************/
int32 ModTracker::GetCount(void)
{
	return (9);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 ModTracker::GetSupportFlags(int32 index)
{
	return (appSetPosition);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString ModTracker::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_MOD_NAME + index);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString ModTracker::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_MOD_DESCRIPTION + index);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString ModTracker::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_MOD_MIME + index);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see which type of module it is.          */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to the file object with the file to check.     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result ModTracker::ModuleCheck(int32 index, PFile *file)
{
	ModType checkType;

	// Check the module
	checkType = TestModule(file);

	if (checkType == index)
		return (AP_OK);

	// We couldn't recognize it
	return (AP_UNKNOWN);
}



/******************************************************************************/
/* LoadModule() will load the module into the memory.                         */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a file object with the file to check.       */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result ModTracker::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	ap_result retVal;

	// Load the module
	if (index == modMultiTracker)
		retVal = LoadMultiTracker(file, errorStr);
	else
		retVal = LoadTracker((ModType)index, file, errorStr);

	return (retVal);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool ModTracker::InitPlayer(int32 index)
{
	SongTime *songTime;
	int16 pos, row, newPos, newRow, chan;
	int16 posCount;
	int16 startPos = 0, startRow = 0;
	uint16 *seqPoi;
	TrackLine *trackData;
	PosInfo posInfo;
	uint8 frameDelay;
	uint8 curSpeed;
	uint8 curTempo;
	uint8 *loopCount, *loopPos;
	bool *loopFlag;
	float total;
	bool pattBreak, posBreak, getOut;
	bool fullStop;

	// Remember the module type
	modType = (ModType)index;

	// Allocate temporary arrays used in the E6x effect calculations
	loopCount = new uint8[channelNum];
	if (loopCount == NULL)
		return (false);

	loopPos = new uint8[channelNum];
	if (loopPos == NULL)
	{
		delete[] loopCount;
		return (false);
	}

	loopFlag = new bool[channelNum];
	if (loopFlag == NULL)
	{
		delete[] loopPos;
		delete[] loopCount;
		return (false);
	}

	// Initialize channel structures
	channels = new Channel[channelNum];
	if (channels == NULL)
	{
		delete[] loopFlag;
		delete[] loopPos;
		delete[] loopCount;
		return (false);
	}

	do
	{
		// Allocate a new sub song structure
		songTime = new SongTime;
		if (songTime == NULL)
		{
			delete[] loopFlag;
			delete[] loopPos;
			delete[] loopCount;
			return (false);
		}

		// Set the start position
		songTime->startPos = startPos;

		// Initialize calculation variables
		curSpeed = 6;
		curTempo = 125;
		total    = 0.0f;
		fullStop = false;

		// Initialize loop arrays
		memset(loopCount, 0, channelNum * sizeof(uint8));
		memset(loopPos, 0, channelNum * sizeof(uint8));
		memset(loopFlag, 0, channelNum * sizeof(uint8));

		// Calculate the position times
		for (pos = startPos, posCount = startPos; pos < songLength; pos++, posCount++)
		{
			// Add the position information to the list
			posInfo.speed = curSpeed;
			posInfo.tempo = curTempo;
			posInfo.time.SetTimeSpan(total);

			if ((pos - startPos) >= songTime->posInfoList.CountItems())
				songTime->posInfoList.AddTail(posInfo);

			// Get pointer to next sequence
			seqPoi = &sequences[positions[pos] * 32];

			// Clear some flags
			pattBreak = false;
			posBreak  = false;
			getOut    = false;

			for (row = startRow; row < patternLength; row++)
			{
				// Reset the start row
				startRow   = 0;
				newPos     = -1;
				newRow     = -2;
				frameDelay = 1;

				for (chan = 0; chan < channelNum; chan++)
				{
					trackData = tracks[seqPoi[chan]] + row;

					// Should the speed/tempo be changed
					if (trackData->effect == effSetSpeed)
					{
						uint8 newSpeed = trackData->effectArg;

						if ((modType == modSoundTracker15) || (modType == modSoundTracker31) || (modType == modNoiseTracker) || (modType == modStarTrekker) || (modType == modStarTrekker8))
						{
							// Old trackers
							if (newSpeed != 0)
							{
								if (newSpeed > 32)
									newSpeed = 32;

								curSpeed = newSpeed;
							}
						}
						else
						{
							// New trackers
							if (newSpeed == 0)
							{
								curSpeed = 6;
								curTempo = 125;
								getOut   = true;
								fullStop = true;
							}
							else
							{
								if (newSpeed > 32)
									curTempo = newSpeed;
								else
									curSpeed = newSpeed;
							}
						}
					}

					// Should we break the current pattern
					if (trackData->effect == effPatternBreak)
					{
						startRow  = ((trackData->effectArg >> 4) & 0x0f) * 10 + (trackData->effectArg & 0x0f);
						pattBreak = true;
						getOut    = true;
					}

					// Should we change the position
					if (trackData->effect == effPosJump)
					{
						// Do we jump to a lower position
						if (trackData->effectArg < pos)
							fullStop = true;
						else
						{
							if (trackData->effectArg == pos)
								fullStop = true;

							newPos   = trackData->effectArg - 1;
							posBreak = true;
						}

						getOut = true;
					}

					// Check for ProTracker commands
					if ((modType == modProTracker) || (modType == modFastTracker) ||
						(modType == modTakeTracker) || (modType == modMultiTracker))
					{
						// Did we reach a pattern loop command
						if ((trackData->effect == effExtraEffect) && ((trackData->effectArg & 0xf0) == effJumpToLoop))
						{
							uint8 arg = trackData->effectArg & 0x0f;

							if (arg != 0)
							{
								// Ignore the effect if it hasn't been set
//								if (loopFlag[chan])		// Uncommented because of Breathless - Level 5
								{
									// Jump to the loop currently set
									if (loopCount[chan] == 0)
										loopCount[chan] = arg;
									else
										loopCount[chan]--;

									if (loopCount[chan] != 0)
									{
										// Set new row
										newRow = loopPos[chan] - 1;
									}
									else
										loopFlag[chan] = false;
								}
							}
							else
							{
								// Set the loop start point
								loopPos[chan]  = row;
								loopFlag[chan] = true;
							}
						}

						// Did we reach a pattern delay command
						if ((trackData->effect == effExtraEffect) && ((trackData->effectArg & 0xf0) == effPatternDelay))
						{
							// Get the delay count
							frameDelay = (trackData->effectArg & 0x0f) + 1;
						}
					}
				}

				// Change the position
				if (newPos != -1)
					pos = newPos;

				// If we both have a pattern break and position jump command
				// on the same line, ignore the full stop
				if (pattBreak && posBreak)
					fullStop = false;

				// Should the current line be calculated into the total time?
				if (!fullStop)
				{
					// Add the row time
					total += (frameDelay * 1000.0f * curSpeed / (curTempo / 2.5f));
				}

				if (getOut)
					break;

				// Should we jump to a new row?
				if (newRow != -2)
					row = newRow;
			}

			if (fullStop)
				break;
		}

		// Set the total time
		songTime->totalTime.SetTimeSpan(total);

		// And add the song time in the list
		songTimeList.AddTail(songTime);

		// Initialize the start position, in case we have more sub songs
		startPos = posCount + 1;
	}
	while (posCount < (songLength - 1));

	// Delete temporary arrays
	delete[] loopPos;
	delete[] loopCount;

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void ModTracker::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the current song.                                   */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void ModTracker::InitSound(int32 index, uint16 songNum)
{
	uint16 i;
	Channel *chan;
	SongTime *songTime;

	// Get the song time structure
	songTime = songTimeList.GetItem(songNum);

	// Initialize all the variables
	currentSong  = songNum;
	speed        = 6;
	tempo        = 125;
	patternPos   = 0;
	counter      = 0;
	songPos      = songTime->startPos;
	breakPos     = 0;
	posJumpFlag  = false;
	breakFlag    = false;
	gotJump      = false;
	gotBreak     = false;
	lowMask      = 0xff;
	pattDelTime  = 0;
	pattDelTime2 = 0;

	for (i = 0; i < channelNum; i++)
	{
		chan = &channels[i];

		chan->trackLine.note      = 0;
		chan->trackLine.sample    = 0;
		chan->trackLine.effect    = 0;
		chan->trackLine.effectArg = 0;
		chan->start               = NULL;
		chan->length              = 0;
		chan->loopStart           = NULL;
		chan->loopLength          = 0;
		chan->startOffset         = 0;
		chan->period              = 0;
		chan->fineTune            = 0;
		chan->volume              = 0;
		chan->tonePortDirec       = 0;
		chan->tonePortSpeed       = 0;
		chan->wantedPeriod        = 0;
		chan->vibratoCmd          = 0;
		chan->vibratoPos          = 0;
		chan->tremoloCmd          = 0;
		chan->tremoloPos          = 0;
		chan->waveControl         = 0;
		chan->glissFunk           = 0;
		chan->sampleOffset        = 0;
		chan->pattPos             = 0;
		chan->loopCount           = 0;
		chan->funkOffset          = 0;
		chan->waveStart           = 0;
		chan->realLength          = 0;
		chan->pick                = 0;
		chan->amSample            = false;
		chan->amTodo              = 0;
		chan->sampleNum           = 0;
		chan->curLevel            = 0;
		chan->vibDegree           = 0;
		chan->sustainCounter      = 0;

		if (index == modMultiTracker)
			chan->panning = panning[i] * 16;
		else
			chan->panning = 0;
	}
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void ModTracker::Play(void)
{
	if (speed != 0)					// Only play if speed <> 0
	{
		counter++;				// Count speed counter
		if (counter >= speed)	// Do we have to change pattern line?
		{
			counter = 0;
			if (pattDelTime2 != 0)	// Pattern delay active
				NoNewAllChan();
			else
				GetNewNote();

			// Get next pattern line
			patternPos++;
			if (pattDelTime != 0)	// New pattern delay time
			{
				// Activate the pattern delay
				pattDelTime2 = pattDelTime;
				pattDelTime  = 0;
			}

			// Pattern delay routine, jump one line back again
			if (pattDelTime2 != 0)
			{
				if (--pattDelTime2 != 0)
					patternPos--;
			}

			// Has the module ended?
			if (gotJump)
			{
				// If we got both a Bxx and Dxx command
				// on the same line, don't end the module
				if (!gotBreak)
					endReached = true;

				gotJump = false;
			}

			// Make sure that the break flag is always cleared
			gotBreak = false;

			// Pattern break
			if (breakFlag)
			{
				// Calculate new position in the next pattern
				breakFlag  = false;
				patternPos = breakPos;
				breakPos   = 0;
			}

			// Have we played the whole pattern?
			if (patternPos >= patternLength)
				NextPos();
		}
		else
			NoNewAllChan();

		if (posJumpFlag)
			NextPos();
	}
	else
	{
		NoNewAllChan();

		if (posJumpFlag)
			NextPos();
	}

	if (modType == modStarTrekker)
		AMHandler();

	// If we have reached the end of the module, reset speed and tempo
	if (endReached)
	{
		SongTime *songTime;
		PosInfo posInfo;

		songTime = songTimeList.GetItem(currentSong);

		if ((songPos < songTime->startPos) || (songPos >= songTime->posInfoList.CountItems()))
		{
			speed = 6;
			ChangeTempo(125);
		}
		else
		{
			posInfo = songTime->posInfoList.GetItem(songPos - songTime->startPos);
			speed   = posInfo.speed;
			ChangeTempo(posInfo.tempo);
		}
	}
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString ModTracker::GetModuleName(void)
{
	return (songName);
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 ModTracker::GetModuleChannels(void)
{
	return (channelNum);
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *ModTracker::GetSubSongs(void)
{
	songTab[0] = songTimeList.CountItems();	// Number of subsongs
	songTab[1] = 0;							// Default start song

	return (songTab);
}



/******************************************************************************/
/* GetSongLength() returns the length of the current song.                    */
/*                                                                            */
/* Output: Is the length of the current song.                                 */
/******************************************************************************/
int16 ModTracker::GetSongLength(void)
{
	return (songLength);
}



/******************************************************************************/
/* GetSongPosition() returns the current position of the playing song.        */
/*                                                                            */
/* Output: Is the current position.                                           */
/******************************************************************************/
int16 ModTracker::GetSongPosition(void)
{
	return (songPos);
}



/******************************************************************************/
/* SetSongPosition() sets the current position of the playing song.           */
/*                                                                            */
/* Input:  "pos" is the new position.                                         */
/******************************************************************************/
void ModTracker::SetSongPosition(int16 pos)
{
	SongTime *songTime;
	PosInfo posInfo;

	// Get the song time structure
	songTime = songTimeList.GetItem(currentSong);

	// Change the position
	songPos      = pos;
	patternPos   = 0;
	pattDelTime  = 0;
	pattDelTime2 = 0;

	// Change the speed
	if ((pos < songTime->startPos) || (pos >= songTime->posInfoList.CountItems()))
	{
		speed = 6;
		ChangeTempo(125);
	}
	else
	{
		posInfo = songTime->posInfoList.GetItem(pos - songTime->startPos);
		speed   = posInfo.speed;
		ChangeTempo(posInfo.tempo);
	}
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
PTimeSpan ModTracker::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	SongTime *songTime;
	int32 i, j, count;

	// Get the song time structure
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
	for (; i < songLength; i++)
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
bool ModTracker::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 4)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Song Length
		case 0:
		{
			description.LoadString(res, IDS_MOD_INFODESCLINE0);
			value.SetUNumber(songLength);
			break;
		}

		// Used Patterns
		case 1:
		{
			description.LoadString(res, IDS_MOD_INFODESCLINE1);
			value.SetUNumber(maxPattern);
			break;
		}

		// Supported/Used Samples
		case 2:
		{
			description.LoadString(res, IDS_MOD_INFODESCLINE2);
			value.SetUNumber(sampleNum);
			break;
		}

		// Actual Speed (BPM)
		case 3:
		{
			description.LoadString(res, IDS_MOD_INFODESCLINE3);
			value.SetUNumber(tempo);
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
bool ModTracker::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	Sample *sample;

	// First check the sample number for "out of range"
	if (num >= sampleNum)
		return (false);

	// Get the pointer to the sample data
	sample = &samples[num];

	// Find out the type of the sample
	info->type = apSample;
	if (amData != NULL)
	{
		if (amData[num].mark == 'AM')
			info->type = apSynth;
	}

	// Fill out the sample info structure
	info->name       = sample->sampleName;
	info->flags      = (sample->loopLength <= 1 ? 0 : APSAMP_LOOP);
	info->bitSize    = 8;
	info->middleC    = (uint32)(7093789.2 / (periods[sample->fineTune][3 * 12] * 2));
	info->volume     = sample->volume * 4;
	info->panning    = -1;
	info->address    = sample->start;
	info->length     = sample->length * 2;
	info->loopStart  = sample->loopStart * 2;
	info->loopLength = sample->loopLength * 2;

	return (true);
}



/******************************************************************************/
/* TestModule() tests the module to see which type of module it is.           */
/*                                                                            */
/* Input:  "file" is a pointer to the file object with the file to check.     */
/*                                                                            */
/* Output: modUnknown for unknown or the module type.                         */
/******************************************************************************/
ModType ModTracker::TestModule(PFile *file)
{
	uint32 mark;
	uint16 i;
	uint8 j, maxLen;
	uint8 buf[4];
	bool ok;
	ModType retVal = modUnknown;

	// First check to see if it's a MultiTracker module
	if (file->GetLength() < (int32)sizeof(MultiModule))
		return (modUnknown);

	// Check the signature
	file->SeekToBegin();
	file->Read(buf, 4);

	if ((buf[0] == 'M') && (buf[1] == 'T') && (buf[2] == 'M') && (buf[3] == 0x10))
		retVal = modMultiTracker;
	else
	{
		// Now check to see if it's a Noise- or ProTracker module
		if (file->GetLength() < (int32)sizeof(ProModule))
			return (modUnknown);

		// Check mark
		file->Seek(1080, PFile::pSeekBegin);
		mark = file->Read_B_UINT32();

		if ((mark == 'M.K.') || (mark == 'M!K!') || (mark == 'M&K!'))
		{
			// Now we know it's either a Noise- or ProTracker module, but we
			// need to know exactly which type it is
			retVal = modNoiseTracker;

			// Get the length byte
			file->Seek(950, PFile::pSeekBegin);
			maxLen = file->Read_UINT8();

			// Check the restart byte
			if (file->Read_UINT8() > 126)
				retVal = modProTracker;

			// Check the first 2 playing pattern for any BPM speed effects or
			// ExtraEffect effects just to be sure it's not a NoiseTracker module.
			//
			// Also check to see if it's a UNIC Tracker module. If so, don't
			// recognize it.
			buf[0] = file->Read_UINT8();
			buf[1] = file->Read_UINT8();

			maxLen = min(2, maxLen);
			for (j = 0; j < maxLen; j++)
			{
				file->Seek(1084 + buf[j] * 4 * 4 * 64, PFile::pSeekBegin);

				for (i = 0; i < 4 * 64; i++)
				{
					uint8 a, b, c, d;
					uint32 temp;

					a = file->Read_UINT8();
					b = file->Read_UINT8();
					c = file->Read_UINT8();
					d = file->Read_UINT8();

					// Check the data to see if it's not a UNIC Tracker module
					//
					// Is sample > 31 or pitch > 358
					if (a > 0x13)
					{
						retVal = modUnknown;
						break;
					}

					temp = ((a & 0x0f) * 256) + b;
					if ((temp > 0) && (temp < 0x1c))
					{
						retVal = modUnknown;
						break;
					}

					// Only check speed command on first line
					if (i < 4)
					{
						if ((c & 0x0f) == effSetSpeed)
						{
							if (d > 31)
							{
								retVal = modProTracker;
								break;
							}
						}
					}

					if ((c & 0x0f) == effExtraEffect)
					{
						if (d > 1)
						{
							retVal = modProTracker;
							break;
						}
					}
				}
			}

			if ((retVal != modUnknown) && (retVal != modProTracker))
			{
				// Well, now we want to be really really sure it's
				// not a NoiseTracker module, so we check the sample
				// information to see if some samples has a finetune.
				file->Seek(44, PFile::pSeekBegin);

				for (i = 0; i < 31; i++)
				{
					if ((file->Read_UINT8() & 0x0f) != 0)
					{
						retVal = modProTracker;
						break;
					}

					// Seek to the next sample
					file->Seek(30 - 1, PFile::pSeekCurrent);
				}
			}
		}
		else
		{
			if (mark == 'FLT4')
				retVal = modStarTrekker;
			else
			{
				if (mark == 'FLT8')
					retVal = modStarTrekker8;
				else
				{
					if ((mark == '6CHN') || (mark == '8CHN'))
						retVal = modFastTracker;
					else
					{
						if (((mark & 0x00ffffff) == '\0CHN') || ((mark & 0x0000ffff) == '\0\0CH') ||
							((mark & 0xffffff00) == 'TDZ\0'))
						{
							retVal = modTakeTracker;
						}
						else
						{
							// Check the mark to see if there is illegal character in it. There
							// has to be, else it isn't a SoundTracker module.
							ok = false;

							for (i = 0; i < 4; i++)
							{
								uint8 byte = (uint8)(mark & 0xff);
								if ((byte < 32) || (byte > 127))
								{
									ok = true;
									break;
								}

								mark = mark >> 8;
							}

							if (ok)
							{
								// Now we know it could be a SoundTracker module, but we have to
								// check the sample names, just to be sure.
								for (i = 0; i < 15; i++)
								{
									if (!CheckSampleName(file, i))
									{
										ok = false;
										break;
									}
								}

								if (ok)
								{
									// Check sample number 16 + 17. At least
									// one of them has to be illegal if it's a
									// SoundTracker15 module
									if ((!CheckSampleName(file, 15)) || (!CheckSampleName(file, 16)))
									{
										// And to be extra sure, the song length may not be 0
										file->Seek(470, PFile::pSeekBegin);

										if (file->Read_UINT8() != 0)
											retVal = modSoundTracker15;
									}
									else
									{
										// Well, we check the rest of the samples to see if it's
										// a SoundTracker31 module
										for (i = 15; i < 31; i++)
										{
											if (!CheckSampleName(file, i))
											{
												ok = false;
												break;
											}
										}

										if (ok)
										{
											// And to be extra sure, the song length may not be 0
											file->Seek(950, PFile::pSeekBegin);

											if (file->Read_UINT8() == 0)
												ok = false;

											if (ok)
											{
												// Okay, we know it could be a
												// SoundTracker31 module, but to be
												// extra extra sure, check the first
												// pattern to see if it's not a UNIC
												// Tracker module
												file->Seek(1080, PFile::pSeekBegin);

												for (i = 0; i < 4 * 64; i++)
												{
													uint8 a, b, c, d;
													uint32 temp;

													a = file->Read_UINT8();
													b = file->Read_UINT8();
													c = file->Read_UINT8();
													d = file->Read_UINT8();

													// Check the data to see if it's not
													// a UNIC Tracker module
													//
													// Is sample > 31 or pitch > 358
													if (a > 0x13)
													{
														ok = false;
														break;
													}

													temp = ((a & 0x0f) * 256) + b;
													if ((temp > 0) && (temp < 0x1c))
													{
														ok = false;
														break;
													}
												}

												if (ok)
													retVal = modSoundTracker31;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return (retVal);
}



/******************************************************************************/
/* CheckSampleName() checks a sample name for illegal characters.             */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*         "num" is the sample number to check starting from 0.               */
/*                                                                            */
/* Output: True if the sample name is ok, else false.                         */
/******************************************************************************/
bool ModTracker::CheckSampleName(PFile *file, uint16 num)
{
	unsigned char sampName[22];
	bool result = true;

	// Seek to the right position and read the name
	file->Seek(20 + num * 30, PFile::pSeekBegin);
	file->Read(sampName, 22);

	// Now check the name
	for (uint16 i = 0; i < 22; i++)
	{
		if (sampName[i] != 0x00)
		{
			if ((sampName[i] < 32) || (sampName[i] > 127))
			{
				result = false;
				break;
			}
		}
	}

	return (result);
}



/******************************************************************************/
/* LoadTracker() will load a tracker module into the memory.                  */
/*                                                                            */
/* Input:  "type" is the module type to load.                                 */
/*         "file" is a pointer to a file object with the file to check.       */
/*         "errorStr" is a reference to a string to any error messages.       */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result ModTracker::LoadTracker(ModType type, PFile *file, PString &errorStr)
{
	char buf[21];
	int32 i, j;
	uint32 mark;
	uint8 tempByte;
	ap_result retVal = AP_ERROR;

	try
	{
		// Find out the number of samples
		if (type == modSoundTracker15)
			sampleNum = 15;
		else
			sampleNum = 31;

		// Read the songname
		buf[20] = 0x00;
		file->Read(buf, 20);

		if ((type == modFastTracker) || (type == modTakeTracker))
			songName.SetString(buf, &dosCharSet);
		else
			songName.SetString(buf, &amiCharSet);

		// Allocate space to the samples
		samples = new Sample[sampleNum];
		if (samples == NULL)
		{
			errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
			throw PUserException();
		}

		// Read the samples
		for (i = 0; i < sampleNum; i++)
		{
			SampleInfo sampInfo;
			Sample *samp = &samples[i];

			// Read the sample info
			sampInfo.sampleName[22] = 0x00;
			file->Read(sampInfo.sampleName, 22);

			sampInfo.length       = file->Read_B_UINT16();
			sampInfo.fineTune     = file->Read_UINT8();
			sampInfo.volume       = file->Read_UINT8();
			sampInfo.repeatStart  = file->Read_B_UINT16();
			sampInfo.repeatLength = file->Read_B_UINT16();

			// Correct "funny" modules
			if (sampInfo.repeatStart > sampInfo.length)
			{
				sampInfo.repeatStart  = 0;
				sampInfo.repeatLength = 0;
			}

			if ((sampInfo.repeatStart + sampInfo.repeatLength) > sampInfo.length)
				sampInfo.repeatLength = sampInfo.length - sampInfo.repeatStart;

			// Do the recognized format support finetune?
			if ((type == modSoundTracker15) || (type == modSoundTracker31) ||
				(type == modNoiseTracker) || (type == modStarTrekker) || (type == modStarTrekker8))
			{
				sampInfo.fineTune = 0;
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_MOD_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			// Put the information into the sample structure
			if ((type == modFastTracker) || (type == modTakeTracker))
				samp->sampleName.SetString(sampInfo.sampleName, &dosCharSet);
			else
				samp->sampleName.SetString(sampInfo.sampleName, &amiCharSet);

			samp->start      = NULL;
			samp->length     = sampInfo.length;
			samp->loopStart  = sampInfo.repeatStart;
			samp->loopLength = sampInfo.repeatLength;
			samp->fineTune   = sampInfo.fineTune & 0xf;
			samp->volume     = sampInfo.volume;
		}

		// Read more header information
		songLength = file->Read_UINT8();

		if ((type == modNoiseTracker) || (type == modStarTrekker) || (type == modStarTrekker8))
		{
			tempByte   = 0;
			restartPos = file->Read_UINT8() & 0x7f;
		}
		else
		{
			tempByte   = file->Read_UINT8();
			restartPos = 0;
		}

		file->Read(positions, 128);

		if ((type != modSoundTracker15) && (type != modSoundTracker31))
			mark = file->Read_B_UINT32();
		else
			mark = 0;

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_MOD_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Find the missing information
		patternLength = 64;

		// Find the number of channels used
		if (type == modStarTrekker8)
			channelNum = 8;
		else
		{
			if ((mark & 0x00ffffff) == '\0CHN')
				channelNum = (uint16)(((mark & 0xff000000) >> 24) - 0x30);
			else
			{
				if ((mark & 0x0000ffff) == '\0\0CH')
					channelNum = (uint16)((((mark & 0xff000000) >> 24) - 0x30) * 10 + ((mark & 0x00ff0000) >> 16) - 0x30);
				else
				{
					if ((mark & 0xffffff00) == 'TDZ\0')
						channelNum = (uint16)((mark & 0x000000ff) - 0x30);
					else
						channelNum = 4;
				}
			}
		}

		// If we load a StarTrekker 8-voices module, divide all the
		// pattern numbers by 2
		if (type == modStarTrekker8)
		{
			for (i = 0; i < 128; i++)
				positions[i] /= 2;
		}

		// Find heighest pattern number
		maxPattern = 0;

		if ((type == modSoundTracker15) && (tempByte == 0xb8))
		{
			for (i = 0; i < songLength; i++)
			{
				if (positions[i] > maxPattern)
					maxPattern = positions[i];
			}
		}
		else
		{
			for (i = 0; i < 128; i++)
			{
				if (positions[i] > maxPattern)
					maxPattern = positions[i];
			}
		}

		maxPattern++;
		trackNum = maxPattern * channelNum;

		// Find the min and max periods
		if ((type == modFastTracker) || (type == modTakeTracker))
		{
			minPeriod = 28;
			maxPeriod = 3424;
		}
		else
		{
			minPeriod = 113;
			maxPeriod = 856;
		}

		// Allocate space for the patterns
		tracks = new TrackLine *[trackNum];
		if (tracks == NULL)
		{
			errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
			throw PUserException();
		}

		memset(tracks, 0, trackNum * sizeof(TrackLine *));

		// Read the tracks
		TrackLine **line = new TrackLine *[channelNum];

		for (i = 0; i < trackNum / channelNum; i++)
		{
			// Allocate memory to hold the tracks
			for (j = 0; j < channelNum; j++)
			{
				tracks[i * channelNum + j] = new TrackLine[64];
				if (tracks[i * channelNum + j] == NULL)
				{
					delete[] line;

					errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
					throw PUserException();
				}

				line[j] = tracks[i * channelNum + j];
			}

			// Now read the tracks
			if (type == modStarTrekker8)
			{
				LoadModTracks(file, &line[0], 4);
				LoadModTracks(file, &line[4], 4);
			}
			else
				LoadModTracks(file, &line[0], channelNum);

			if (file->IsEOF())
			{
				delete[] line;

				errorStr.LoadString(res, IDS_MOD_ERR_LOADING_PATTERNS);
				throw PUserException();
			}
		}

		delete[] line;

		// Allocate memory to hold the sequences
		sequences = new uint16[32 * maxPattern];
		if (sequences == NULL)
		{
			errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
			throw PUserException();
		}

		// Calculate the sequence numbers
		for (i = 0; i < maxPattern; i++)
		{
			for (j = 0; j < channelNum; j++)
				sequences[i * 32 + j] = (uint16)(i * channelNum + j);
		}

		// Read the samples
		for (i = 0; i < sampleNum; i++)
		{
			int8 *sampData;
			int32 length;

			// Allocate space to hold the sample
			length = samples[i].length * 2;

			if (length != 0)
			{
				sampData = new int8[length];
				if (sampData == NULL)
				{
					errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
					throw PUserException();
				}

				memset(sampData, 0, length);
				samples[i].start = sampData;

				// Check the see if we miss to much from the last sample
				if (file->GetLength() - file->GetPosition() < (length - 512))
				{
					errorStr.LoadString(res, IDS_MOD_ERR_LOADING_SAMPLES);
					throw PUserException();
				}

				// Read the sample
				file->Read(sampData, length);
			}
		}

		// Ok, we're done, load any extra files if needed
		if (type == modStarTrekker)
			retVal = ExtraLoad(file);
		else
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
/* LoadMultiTracker() will load a MultiTracker module into the memory.        */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*         "errorStr" is a reference to a string to any error messages.       */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result ModTracker::LoadMultiTracker(PFile *file, PString &errorStr)
{
	MultiModule mulMod;
	int32 i, j;
	ap_result retVal = AP_ERROR;

	try
	{
		// Read the header
		file->Read(mulMod.mark, 3);

		mulMod.version = file->Read_UINT8();

		mulMod.songName[20] = 0x00;
		file->Read(mulMod.songName, 20);

		mulMod.trackNum      = file->Read_L_UINT16();
		mulMod.patternNum    = file->Read_UINT8();
		mulMod.songLength    = file->Read_UINT8();
		mulMod.commentLength = file->Read_L_UINT16();
		mulMod.sampleNum     = file->Read_UINT8();
		mulMod.attributes    = file->Read_UINT8();
		mulMod.patternLength = file->Read_UINT8();
		mulMod.channels      = file->Read_UINT8();

		file->Read(panning, 32);

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_MOD_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Remember some of the information
		songName.SetString(mulMod.songName, &dosCharSet);
		maxPattern    = mulMod.patternNum + 1;
		channelNum    = mulMod.channels;
		sampleNum     = mulMod.sampleNum;
		songLength    = mulMod.songLength + 1;
		trackNum      = mulMod.trackNum + 1;	// Add one because track 0 is not written but considered empty
		patternLength = mulMod.patternLength;

		// Allocate the samples
		samples = new Sample[sampleNum];
		if (samples == NULL)
		{
			errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
			throw PUserException();
		}

		for (i = 0; i < sampleNum; i++)
		{
			MultiSampleInfo sampInfo;
			Sample *samp = &samples[i];

			// Read the sample information
			sampInfo.sampleName[22] = 0x00;

			file->Read(sampInfo.sampleName, 22);

			sampInfo.length      = file->Read_L_UINT32();
			sampInfo.repeatStart = file->Read_L_UINT32();
			sampInfo.repeatEnd   = file->Read_L_UINT32();
			sampInfo.fineTune    = file->Read_UINT8();
			sampInfo.volume      = file->Read_UINT8();
			sampInfo.attributes  = file->Read_UINT8();

			samp->sampleName.SetString(sampInfo.sampleName, &dosCharSet);
			samp->start      = NULL;
			samp->length     = (uint16)(sampInfo.length / 2);
			samp->loopStart  = (uint16)(sampInfo.repeatStart / 2);
			samp->loopLength = (uint16)(sampInfo.repeatEnd / 2 - samp->loopStart);
			samp->fineTune   = sampInfo.fineTune & 0xf;
			samp->volume     = sampInfo.volume;

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_MOD_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}
		}

		// Read the positions
		file->Read(positions, 128);

		// Allocate memory to hold all the tracks
		tracks = new TrackLine *[trackNum];
		if (tracks == NULL)
		{
			errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
			throw PUserException();
		}

		memset(tracks, 0, trackNum * sizeof(TrackLine *));

		// Generate an empty track
		tracks[0] = new TrackLine[patternLength];
		if (tracks[0] == NULL)
		{
			errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
			throw PUserException();
		}

		memset(tracks[0], 0, patternLength * sizeof(TrackLine));

		// Read the tracks. Use the number stored in the module
		for (i = 0; i < mulMod.trackNum; i++)
		{
			TrackLine *line;

			// Allocate memory to hold the track
			line = new TrackLine[patternLength];
			if (line == NULL)
			{
				errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
				throw PUserException();
			}

			tracks[i + 1] = line;

			// Now read the track
			for (j = 0; j < patternLength; j++)
			{
				uint8 a, b, c;

				a = file->Read_UINT8();
				b = file->Read_UINT8();
				c = file->Read_UINT8();

				line[j].note = a >> 2;
				if (line[j].note != 0)
					line[j].note += 13;

				line[j].sample    = (a & 0x03) << 4 | ((b & 0xf0) >> 4);
				line[j].effect    = b & 0x0f;
				line[j].effectArg = c;
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_MOD_ERR_LOADING_PATTERNS);
				throw PUserException();
			}
		}

		// Allocate memory to hold the sequences
		sequences = new uint16[32 * maxPattern];
		if (sequences == NULL)
		{
			errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
			throw PUserException();
		}

		// Read the sequence data
		file->ReadArray_L_UINT16s(sequences, 32 * maxPattern);

		// Skip the comment field
		file->Seek(mulMod.commentLength, PFile::pSeekCurrent);

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_MOD_ERR_LOADING_PATTERNS);
			throw PUserException();
		}

		// Read the samples
		for (i = 0; i < sampleNum; i++)
		{
			int8 *sampData;
			int32 length;

			// Allocate space to hold the sample
			length = samples[i].length * 2;

			if (length != 0)
			{
				sampData = new int8[length];
				if (sampData == NULL)
				{
					errorStr.LoadString(res, IDS_MOD_ERR_MEMORY);
					throw PUserException();
				}

				samples[i].start = sampData;

				// Read the sample
				file->Read(sampData, length);

				if (file->IsEOF())
				{
					errorStr.LoadString(res, IDS_MOD_ERR_LOADING_SAMPLES);
					throw PUserException();
				}

				// Convert the sample to signed
				for (j = 0; j < length; j++)
					*sampData++ = *sampData + 0x80;
			}
		}

		// Initialize the rest of the variables used
		minPeriod  = 45;
		maxPeriod  = 1616;
		restartPos = 0;

		// Ok, we're done
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
/* ExtraLoad() will load all extra files needed.                              */
/*                                                                            */
/* Input:  "file" is a pointer to the main module file.                       */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result ModTracker::ExtraLoad(PFile *file)
{
	PString fileName;
	PFile *extraFile;

	try
	{
		// Get the file name of the main module
		fileName = file->GetFullPath();

		// Try to open the extension file
		extraFile = OpenExtraFile(fileName, "nt");

		try
		{
			// Allocate memory to the AM structures
			amData = new AMSample[31];
			if (amData == NULL)
				throw PMemoryException();

			memset(amData, 0, 31 * sizeof(AMSample));

			// Load the AM data
			extraFile->Seek(24 + 120, PFile::pSeekBegin);

			for (int i = 0; i < 31; i++)
			{
				AMSample *samp = &amData[i];

				samp->mark         = extraFile->Read_B_UINT16();
				samp->pad00        = extraFile->Read_B_UINT32();
				samp->startAmp     = extraFile->Read_B_UINT16();
				samp->attack1Level = extraFile->Read_B_UINT16();
				samp->attack1Speed = extraFile->Read_B_UINT16();
				samp->attack2Level = extraFile->Read_B_UINT16();
				samp->attack2Speed = extraFile->Read_B_UINT16();
				samp->sustainLevel = extraFile->Read_B_UINT16();
				samp->decaySpeed   = extraFile->Read_B_UINT16();
				samp->sustainTime  = extraFile->Read_B_UINT16();
				samp->pad01        = extraFile->Read_B_UINT16();
				samp->releaseSpeed = extraFile->Read_B_UINT16();
				samp->waveform     = extraFile->Read_B_UINT16();
				samp->pitchFall    = extraFile->Read_B_UINT16();
				samp->vibAmp       = extraFile->Read_B_UINT16();
				samp->vibSpeed     = extraFile->Read_B_UINT16();
				samp->baseFreq     = extraFile->Read_B_UINT16();

				extraFile->Read(samp->reserved, 84);

				if (extraFile->IsEOF())
					break;
			}
		}
		catch(...)
		{
			CloseExtraFile(extraFile);
			throw;
		}

		CloseExtraFile(extraFile);
	}
	catch(...)
	{
		;
	}

	return (AP_OK);
}



/******************************************************************************/
/* LoadModTracks() will load x number of tracks in MOD format.                */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read from.                      */
/*         "tracks" is where to store the loaded tracks.                      */
/*         "channels" is the number of channels to load.                      */
/******************************************************************************/
void ModTracker::LoadModTracks(PFile *file, TrackLine **tracks, int32 channels)
{
	int32 i, j, n;

	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < channels; j++)
		{
			TrackLine *workLine;
			uint8 a, b, c, d;
			uint16 note;

			workLine = tracks[j];

			a = file->Read_UINT8();
			b = file->Read_UINT8();
			c = file->Read_UINT8();
			d = file->Read_UINT8();

			note = ((a & 0x0f) << 8) | b;

			if (note)		// Is there any note?
			{
				for (n = 0; n < NumberOfNotes; n++)
				{
					if (note >= periods[0][n])
						break;			// Found the note number
				}

				workLine[i].note = n + 1;
			}
			else
				workLine[i].note = 0;

			workLine[i].sample    = (a & 0xf0) | ((c & 0xf0) >> 4);
			workLine[i].effect    = c & 0x0f;
			workLine[i].effectArg = d;
		}
	}
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void ModTracker::Cleanup(void)
{
	int32 i, count;

	// Delete StarTrekker synthesis sounds
	delete[] amData;
	amData = NULL;

	// Delete the module
	delete channels;
	channels = NULL;

	delete[] sequences;
	sequences = NULL;

	if (tracks != NULL)
	{
		for (i = 0; i < trackNum; i++)
			delete[] tracks[i];
	}

	delete[] tracks;
	tracks = NULL;

	if (samples != NULL)
	{
		for (i = 0; i < sampleNum; i++)
			delete[] samples[i].start;
	}

	// Delete samples
	delete[] samples;
	samples = NULL;

	// Empty the song time list
	count = songTimeList.CountItems();
	for (i = 0; i < count; i++)
		delete songTimeList.GetItem(i);

	songTimeList.MakeEmpty();
}



/******************************************************************************/
/* NextPos() jumps to the next song postion.                                  */
/******************************************************************************/
void ModTracker::NextPos(void)
{
	// Initialize the position variables
	patternPos  = breakPos;
	breakPos    = 0;
	posJumpFlag = false;
	songPos    += 1;
	songPos    &= 0x7f;

	if (songPos >= songLength)
	{
		songPos = restartPos;

		// Position has changed
		ChangePosition();

		// And the module has repeated
		endReached = true;
	}
	else
	{
		// Position has changed
		ChangePosition();
	}
}



/******************************************************************************/
/* NoNewAllChan() checks all channels to see if any commands should run.      */
/******************************************************************************/
void ModTracker::NoNewAllChan(void)
{
	uint16 i;

	for (i = 0; i < channelNum; i++)
		CheckEFX(virtChannels[i], channels[i]);
}



/******************************************************************************/
/* GetNewNote() parses the next pattern line.                                 */
/******************************************************************************/
void ModTracker::GetNewNote(void)
{
	uint16 i;
	uint16 curSongPos, curPattPos;
	uint16 trackNum;
	TrackLine *trackData;

	// Get position information into temporary variables
	curSongPos = songPos;
	curPattPos = patternPos;

	for (i = 0; i < channelNum; i++)
	{
		// Find the track to use
		trackNum  = sequences[positions[curSongPos] * 32 + i];
		trackData = tracks[trackNum] + curPattPos;

		PlayVoice(trackData, virtChannels[i], channels[i]);
	}
}



/******************************************************************************/
/* PlayVoice() parses one pattern line for one channel.                       */
/*                                                                            */
/* Input:  "trackData" is a pointer to the pattern line.                      */
/*         "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::PlayVoice(TrackLine *trackData, APChannel *apChan, Channel &chan)
{
	Sample *sample;
	AMSample *amSamp;
	uint8 sampNum, cmd;

	// Check for any note or effect running
	if ((trackData->note == 0) && (trackData->sample == 0) && (trackData->effect == 0) && (trackData->effectArg == 0))
	{
		// Nothing runs, so set the period
		apChan->SetAmigaPeriod(chan.period);
	}

	// Copy pattern line to fields in our channel structure
	chan.trackLine = *trackData;

	sampNum = trackData->sample;
	if (sampNum)
	{
		// New sample
		sample = &samples[sampNum - 1];

		chan.sampleNum   = sampNum;
		chan.amSample    = false;

		if (modType == modStarTrekker)
		{
			if (amData != NULL)
			{
				amSamp = &amData[sampNum - 1];

				if (amSamp->mark == 'AM')
				{
					chan.volume   = amSamp->startAmp / 4;
					chan.amSample = true;
				}
			}
		}

		chan.start       = sample->start;
		chan.length      = sample->length;
		chan.realLength  = chan.length;
		chan.startOffset = 0;
		chan.fineTune    = sample->fineTune;

		if (!chan.amSample)
			chan.volume = sample->volume;

		// Check to see if we got a loop
		if ((sample->loopStart != 0) && (sample->loopLength > 1))
		{
			// We have, now check the player mode
			if ((modType == modSoundTracker15) || (modType == modSoundTracker31))
			{
				chan.start     += sample->loopStart;
				chan.loopStart  = chan.start;
				chan.waveStart  = chan.start;
				chan.length     = sample->loopLength;
				chan.loopLength = chan.length;
			}
			else
			{
				chan.loopStart  = chan.start + sample->loopStart * 2;
				chan.waveStart  = chan.loopStart;

				chan.length     = sample->loopStart + sample->loopLength;
				chan.loopLength = sample->loopLength;
			}
		}
		else
		{
			// No loop
			chan.loopStart  = chan.start + sample->loopStart;
			chan.waveStart  = chan.loopStart;
			chan.loopLength = sample->loopLength;
		}

		// Set volume
		if (!chan.amSample)
			apChan->SetVolume(chan.volume * 4);

		// Set panning
		if (modType == modMultiTracker)
			apChan->SetPanning(chan.panning);
	}

	// Check for some commands
	if (chan.trackLine.note)
	{
		// There is a new note to play
		cmd = chan.trackLine.effect;

		if (!chan.amSample)
		{
			// Check for SetFineTune
			if ((cmd == effExtraEffect) && ((chan.trackLine.effectArg & 0xf0) == effSetFineTune))
				SetFineTune(chan);
			else
			{
				switch (cmd)
				{
					case effTonePortamento:
					case effTonePort_VolSlide:
					{
						SetTonePorta(chan);
						CheckMoreEFX(apChan, chan);
						return;
					}

					case effSampleOffset:
					{
						CheckMoreEFX(apChan, chan);
						break;
					}
				}
			}
		}

		// Set the period
		chan.period = periods[chan.fineTune][chan.trackLine.note - 1];

		if (!((cmd == effExtraEffect) && ((chan.trackLine.effectArg & 0xf0) == effNoteDelay)))
		{
			if (!(chan.waveControl & 4))
				chan.vibratoPos = 0;

			if (!(chan.waveControl & 64))
				chan.tremoloPos = 0;

			if (chan.amSample)
			{
				// Setup AM sample
				amSamp = &amData[chan.sampleNum - 1];

				chan.start       = &amWaveforms[amSamp->waveform][0];
				chan.startOffset = 0;
				chan.length      = 16;
				chan.loopStart   = chan.start;
				chan.loopLength  = 16;

				chan.amTodo      = 1;
				chan.curLevel    = amSamp->startAmp;
				chan.vibDegree   = 0;
				chan.period      = chan.period << amSamp->baseFreq;
			}

			// Fill out the Channel
			if (chan.length > 0)
			{
				apChan->PlaySample(chan.start, chan.startOffset * 2, chan.length * 2);
				apChan->SetAmigaPeriod(chan.period);

				// Setup loop
				if (chan.loopLength > 0)
					apChan->SetLoop(chan.loopStart - chan.start, chan.loopLength * 2);
			}
			else
				apChan->Mute();
		}
	}

	CheckMoreEFX(apChan, chan);
}



/******************************************************************************/
/* CheckEFX() check one channel to see if there are some commands to run.     */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::CheckEFX(APChannel *apChan, Channel &chan)
{
	uint8 cmd;

	UpdateFunk(chan);

	cmd = chan.trackLine.effect;
	if ((cmd == 0) && (chan.trackLine.effectArg == 0))
		apChan->SetAmigaPeriod(chan.period);
	else
	{
		switch (cmd)
		{
			case effArpeggio:
			{
				Arpeggio(apChan, chan);			// Arpeggio or normal note
				break;
			}

			case effSlideUp:
			{
				PortUp(apChan, chan);
				break;
			}

			case effSlideDown:
			{
				PortDown(apChan, chan);
				break;
			}

			case effTonePortamento:
			{
				TonePortamento(apChan, chan);
				break;
			}

			case effVibrato:
			{
				Vibrato(apChan, chan);
				break;
			}

			case effTonePort_VolSlide:
			{
				TonePlusVol(apChan, chan);
				break;
			}

			case effVibrato_VolSlide:
			{
				VibratoPlusVol(apChan, chan);
				break;
			}

			case effExtraEffect:
			{
				ECommands(apChan, chan);
				break;
			}

			default:
			{
				apChan->SetAmigaPeriod(chan.period);

				if (cmd == effTremolo)
					Tremolo(apChan, chan);
				else
					if (cmd == effVolumeSlide)
						VolumeSlide(apChan, chan);

				break;
			}
		}
	}
}



/******************************************************************************/
/* CheckMoreEFX() check one channel to see if there are some commands to run. */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::CheckMoreEFX(APChannel *apChan, Channel &chan)
{
	if (modType != modMultiTracker)
	{
		switch (chan.trackLine.effect)
		{
			case effSampleOffset:
			{
				SampleOffset(chan);
				break;
			}

			case effPosJump:
			{
				PositionJump(chan);
				break;
			}

			case effSetVolume:
			{
				VolumeChange(apChan, chan);
				break;
			}

			case effPatternBreak:
			{
				PatternBreak(chan);
				break;
			}

			case effExtraEffect:
			{
				ECommands(apChan, chan);
				break;
			}

			case effSetSpeed:
			{
				SetSpeed(chan);
				break;
			}

			default:
			{
				apChan->SetAmigaPeriod(chan.period);
				break;
			}
		}
	}
	else
	{
		switch (chan.trackLine.effect)
		{
			case effSampleOffset:
			{
				SampleOffset(chan);
				break;
			}

			case effPosJump:
			{
				PositionJump(chan);
				break;
			}

			case effSetVolume:
			{
				VolumeChange(apChan, chan);
				break;
			}

			case effPatternBreak:
			{
				PatternBreak(chan);
				break;
			}

			case effExtraEffect:
			{
				ECommands(apChan, chan);
				break;
			}

			case effSetSpeed:
			{
				SetSpeed(chan);
				break;
			}

			case effSlideUp:
			{
				PortUp(apChan, chan);
				break;
			}

			case effSlideDown:
			{
				PortDown(apChan, chan);
				break;
			}

			case effTonePortamento:
			{
				TonePortamento(apChan, chan);
				break;
			}

			case effVibrato:
			{
				Vibrato(apChan, chan);
				break;
			}

			case effTonePort_VolSlide:
			{
				TonePlusVol(apChan, chan);
				break;
			}

			case effVibrato_VolSlide:
			{
				VibratoPlusVol(apChan, chan);
				break;
			}

			default:
			{
				apChan->SetAmigaPeriod(chan.period);
				break;
			}
		}
	}
}



/******************************************************************************/
/* ECommands() check one channel to see if there are some of the extra        */
/*         commands to run.                                                   */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::ECommands(APChannel *apChan, Channel &chan)
{
	if ((modType == modSoundTracker15) || (modType == modSoundTracker31) || (modType == modNoiseTracker) || (modType == modStarTrekker) || (modType == modStarTrekker8))
		Filter(chan);
	else
	{
		switch (chan.trackLine.effectArg & 0xf0)
		{
			case effSetFilter:
			{
				Filter(chan);
				break;
			}

			case effFineSlideUp:
			{
				FinePortUp(apChan, chan);
				break;
			}

			case effFineSlideDown:
			{
				FinePortDown(apChan, chan);
				break;
			}

			case effGlissandoCtrl:
			{
				SetGlissCon(chan);
				break;
			}

			case effVibratoWaveform:
			{
				SetVibCon(chan);
				break;
			}

			case effSetFineTune:
			{
				SetFineTune(chan);
				break;
			}

			case effJumpToLoop:
			{
				JumpLoop(chan);
				break;
			}

			case effTremoloWaveform:
			{
				SetTreCon(chan);
				break;
			}

			case effRetrig:
			{
				RetrigNote(apChan, chan);
				break;
			}

			case effFineVolSlideUp:
			{
				VolumeFineUp(apChan, chan);
				break;
			}

			case effFineVolSlideDown:
			{
				VolumeFineDown(apChan, chan);
				break;
			}

			case effNoteCut:
			{
				NoteCut(apChan, chan);
				break;
			}

			case effNoteDelay:
			{
				NoteDelay(apChan, chan);
				break;
			}

			case effPatternDelay:
			{
				PatternDelay(chan);
				break;
			}

			case effInvertLoop:
			{
				FunkIt(chan);
				break;
			}
		}
	}
}



/******************************************************************************/
/* SetTonePorta() sets the portamento frequency.                              */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::SetTonePorta(Channel &chan)
{
	uint16 period, i;

	period = periods[0][chan.trackLine.note - 1];

	for (i = 0; i < NumberOfNotes; i++)
	{
		if (periods[chan.fineTune][i] <= period)
		{
			i++;
			break;
		}
	}

	// Decrement counter so it have the right value.
	// This is because if the loop goes all the way through.
	i--;

	if ((chan.fineTune > 7) && (i != 0))
		i--;

	period = periods[chan.fineTune][i];

	chan.wantedPeriod  = period;
	chan.tonePortDirec = 0;

	if (chan.period == period)
		chan.wantedPeriod = 0;
	else
	{
		if (chan.period > period)
			chan.tonePortDirec = 1;
	}
}



/******************************************************************************/
/* UpdateFunk() updates funk?                                                 */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::UpdateFunk(Channel &chan)
{
	uint8 glissFunk;
	int8 *sample;

	glissFunk = chan.glissFunk >> 4;
	if (glissFunk != 0)
	{
		chan.funkOffset += funkTable[glissFunk];
		if (chan.funkOffset >= 128)
		{
			chan.funkOffset = 0;

			sample = chan.waveStart + 1;
			if (sample >= (chan.loopStart + chan.loopLength * 2))
				sample = chan.loopStart;

			chan.waveStart = sample;

			// Invert the sample data
			if (sample != NULL)
				*sample = ~*sample;
		}
	}
}



/******************************************************************************/
/* ChangeTempo() will change the tempo on the module.                         */
/*                                                                            */
/* Input:  "newTempo" is the new tempo value.                                 */
/******************************************************************************/
void ModTracker::ChangeTempo(uint8 newTempo)
{
	if (newTempo != tempo)
	{
		// BPM speed
		SetBPMTempo(newTempo);

		// Change the module info
		ChangeModuleInfo(3, PString::CreateUNumber(newTempo));

		// Remember the tempo
		tempo = newTempo;
	}
}



/******************************************************************************/
/*                                                                            */
/* Below are the functions to all the normal effects.                         */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Arpeggio() (0x00) plays arpeggio or normal note.                           */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::Arpeggio(APChannel *apChan, Channel &chan)
{
	uint16 period, i;
	uint8 modulus, arp;

	if (chan.trackLine.effectArg != 0)
	{
		modulus = counter % 3;

		switch (modulus)
		{
			case 1:
			{
				arp = chan.trackLine.effectArg >> 4;
				break;
			}

			case 2:
			{
				arp = chan.trackLine.effectArg & 0x0f;
				break;
			}

			default:
			{
				arp = 0;
				break;
			}
		}

		// Find the index into the period tables
		for (i = 0; i < NumberOfNotes; i++)
		{
			if (periods[chan.fineTune][i] <= chan.period)
				break;
		}

		// Get the period
		period = periods[chan.fineTune][i + arp];
	}
	else
	{
		// Normal note
		period = chan.period;
	}

	// Setup the NotePlayer registers
	apChan->SetAmigaPeriod(period);
}



/******************************************************************************/
/* PortUp() (0x01) slides the frequency up.                                   */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::PortUp(APChannel *apChan, Channel &chan)
{
	chan.period -= (chan.trackLine.effectArg & lowMask);
	if (chan.period < minPeriod)
		chan.period = minPeriod;

	lowMask = 0xff;

	apChan->SetAmigaPeriod(chan.period);
}



/******************************************************************************/
/* PortDown() (0x02) slides the frequency down.                               */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::PortDown(APChannel *apChan, Channel &chan)
{
	chan.period += (chan.trackLine.effectArg & lowMask);
	if (chan.period > maxPeriod)
		chan.period = maxPeriod;

	lowMask = 0xff;

	apChan->SetAmigaPeriod(chan.period);
}



/******************************************************************************/
/* TonePortamento() (0x03) slides the frequency to the current note.          */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/*         "skip" is true if it has to skip the initializing.                 */
/******************************************************************************/
void ModTracker::TonePortamento(APChannel *apChan, Channel &chan, bool skip)
{
	int32 period;
	uint16 i;

	if (!skip)
	{
		if (chan.trackLine.effectArg != 0)
		{
			// Set the slide speed
			chan.tonePortSpeed       = chan.trackLine.effectArg;
			chan.trackLine.effectArg = 0;
		}
	}

	// If slide mode enabled?
	if (chan.wantedPeriod != 0)
	{
		if (chan.tonePortDirec != 0)
		{
			// Slide up
			period = chan.period - chan.tonePortSpeed;
			if (chan.wantedPeriod >= period)
			{
				// Set to the final period and disable slide
				period            = chan.wantedPeriod;
				chan.wantedPeriod = 0;
			}

			chan.period = (uint16)period;
		}
		else
		{
			// Slide down
			period = chan.period + chan.tonePortSpeed;
			if (chan.wantedPeriod <= period)
			{
				// Set to final period and disable slide
				period            = chan.wantedPeriod;
				chan.wantedPeriod = 0;
			}

			chan.period = (uint16)period;
		}

		// Is glissando enabled?
		if (chan.glissFunk & 0x0f)
		{
			for (i = 0; i < NumberOfNotes; i++)
			{
				if (periods[chan.fineTune][i] <= period)
				{
					i++;
					break;
				}
			}

			period = periods[chan.fineTune][i - 1];
		}

		// Setup the NotePlayer structure
		apChan->SetAmigaPeriod(period);
	}
}



/******************************************************************************/
/* Vibrato() (0x04) vibrates the frequency.                                   */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/*         "skip" is true if it has to skip the initializing.                 */
/******************************************************************************/
void ModTracker::Vibrato(APChannel *apChan, Channel &chan, bool skip)
{
	uint8 effArg, vibCmd, vibPos, waveCtrl, addVal;
	uint16 period;

	// Get the effect argument
	effArg = chan.trackLine.effectArg;

	// Setup vibrato command
	if ((!skip) && (effArg != 0))
	{
		vibCmd = chan.vibratoCmd;

		if (effArg & 0x0f)
			vibCmd = (vibCmd & 0xf0) | (effArg & 0x0f);

		if (effArg & 0xf0)
			vibCmd = (vibCmd & 0x0f) | (effArg & 0xf0);

		chan.vibratoCmd = vibCmd;
	}

	// Calculate new position
	vibPos   = (chan.vibratoPos / 4) & 0x1f;
	waveCtrl = chan.waveControl & 0x03;

	if (waveCtrl != 0)
	{
		vibPos *= 8;
		if (waveCtrl != 1)
			addVal = 255;					// Square vibrato
		else
		{
			// Ramp down vibrato
			if (chan.vibratoPos < 0)
				addVal = 255 - vibPos;
			else
				addVal = vibPos;
		}
	}
	else
	{
		// Sine vibrato
		addVal = vibratoTable[vibPos];
	}

	// Set the vibrato
	addVal = addVal * (chan.vibratoCmd & 0x0f) / 128;
	period = chan.period;

	if (chan.vibratoPos < 0)
		period -= addVal;
	else
		period += addVal;

	// Put the new period into the NotePlayer structure
	apChan->SetAmigaPeriod(period);

	chan.vibratoPos += ((chan.vibratoCmd / 4) & 0x3c);
}



/******************************************************************************/
/* TonePlusVol() (0x05) is both effect 0x03 and 0x0a.                         */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::TonePlusVol(APChannel *apChan, Channel &chan)
{
	TonePortamento(apChan, chan, true);
	VolumeSlide(apChan, chan);
}



/******************************************************************************/
/* VibratoPlusVol() (0x06) is both effect 0x04 and 0x0a.                      */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::VibratoPlusVol(APChannel *apChan, Channel &chan)
{
	Vibrato(apChan, chan, true);
	VolumeSlide(apChan, chan);
}



/******************************************************************************/
/* Tremolo() (0x07) makes vibrato on the volume.                              */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::Tremolo(APChannel *apChan, Channel &chan)
{
	uint8 effArg, treCmd, trePos, waveCtrl, addVal;
	int16 volume;

	// Get the effect argument
	effArg = chan.trackLine.effectArg;

	// Setup tremolo command
	if (effArg != 0)
	{
		treCmd = chan.tremoloCmd;

		if (effArg & 0x0f)
			treCmd = (treCmd & 0xf0) | (effArg & 0x0f);

		if (effArg & 0xf0)
			treCmd = (treCmd & 0x0f) | (effArg & 0xf0);

		chan.tremoloCmd = treCmd;
	}

	// Calculate new position
	trePos   = (chan.tremoloPos / 4) & 0x1f;
	waveCtrl = (chan.waveControl >> 4) & 0x03;

	if (waveCtrl != 0)
	{
		trePos *= 8;
		if (waveCtrl != 1)
			addVal = 255;					// Square tremolo
		else
		{
			// Ramp down tremolo
			if (chan.tremoloPos < 0)
				addVal = 255 - trePos;
			else
				addVal = trePos;
		}
	}
	else
	{
		// Sine tremolo
		addVal = vibratoTable[trePos];
	}

	// Set the tremolo
	addVal = addVal * (chan.tremoloCmd & 0x0f) / 64;
	volume = chan.volume;

	if (chan.tremoloPos < 0)
	{
		volume -= addVal;
		if (volume < 0)
			volume = 0;
	}
	else
	{
		volume += addVal;
		if (volume > 64)
			volume = 64;
	}

	// Put the new volume into the NotePlayer structure
	apChan->SetVolume(volume * 4);

	chan.tremoloPos += ((chan.tremoloCmd / 4) & 0x3c);
}



/******************************************************************************/
/* SampleOffset() (0x09) starts the sample somewhere else, but the start.     */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::SampleOffset(Channel &chan)
{
	uint16 offset;

	// Check for initialize value
	if (chan.trackLine.effectArg != 0)
		chan.sampleOffset = chan.trackLine.effectArg;

	// Calculate the offset
	offset = chan.sampleOffset * 128;
	if (offset < chan.length)
		chan.startOffset = offset;
	else
	{
		chan.length      = chan.loopLength;
		chan.start       = chan.loopStart;
		chan.startOffset = 0;
	}
}



/******************************************************************************/
/* VolumeSlide() (0x0A) slides the volume.                                    */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::VolumeSlide(APChannel *apChan, Channel &chan)
{
	uint8 speed;

	speed = chan.trackLine.effectArg >> 4;
	if (speed != 0)
	{
		// Slide up
		chan.volume += speed;
		if (chan.volume > 64)
			chan.volume = 64;
	}
	else
	{
		// Slide down
		chan.volume -= (chan.trackLine.effectArg & 0x0f);
		if (chan.volume < 0)
			chan.volume = 0;
	}

	// Set the volume in the NotePlayer structure
	apChan->SetVolume(chan.volume * 4);
}



/******************************************************************************/
/* PositionJump() (0x0B) jumps to another position.                           */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::PositionJump(Channel &chan)
{
	uint8 pos;

	pos = chan.trackLine.effectArg;
	if (pos < songPos)
		endReached = true;	// Module has repeated

	if (pos == songPos)
		gotJump = true;		// Module jump to the same position, maybe end

	// Set the new position
	songPos     = pos - 1;
	breakPos    = 0;
	posJumpFlag = true;
}



/******************************************************************************/
/* VolumeChange() (0x0C) sets the sample volume.                              */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::VolumeChange(APChannel *apChan, Channel &chan)
{
	uint8 vol;

	vol = chan.trackLine.effectArg;
	if (vol > 64)
		vol = 64;

	chan.volume = vol;

	// Set the volume in the NotePlayer structure
	apChan->SetVolume(vol * 4);
}



/******************************************************************************/
/* PatternBreak() (0x0D) breaks the pattern and jump to the next position.    */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::PatternBreak(Channel &chan)
{
	uint8 arg;

	arg = chan.trackLine.effectArg;

	breakPos    = ((arg >> 4) & 0x0f) * 10 + (arg & 0x0f);
	posJumpFlag = true;
	gotBreak    = true;
}



/******************************************************************************/
/* SetSpeed() (0x0F) changes the speed of the module.                         */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::SetSpeed(Channel &chan)
{
	uint8 newSpeed;

	// Get the new speed
	newSpeed = chan.trackLine.effectArg;

	if ((modType == modSoundTracker15) || (modType == modSoundTracker31) || (modType == modNoiseTracker) || (modType == modStarTrekker) || (modType == modStarTrekker8))
	{
		// Old trackers
		if (newSpeed != 0)
		{
			if (newSpeed > 32)
				newSpeed = 32;

			// Set the new speed
			speed   = newSpeed;
			counter = 0;
		}
	}
	else
	{
		// New trackers
		if (newSpeed == 0)
		{
			// Reset the speed and other position variables
			speed       = 6;
			counter     = 0;
			breakPos    = 0;
			songPos     = restartPos - 1;
			posJumpFlag = true;
			ChangeTempo(125);

			// Module has stopped
			endReached = true;
		}
		else
		{
			if (newSpeed > 32)
				ChangeTempo(newSpeed);
			else
			{
				// Set the new speed
				speed   = newSpeed;
				counter = 0;
			}
		}
	}
}



/******************************************************************************/
/*                                                                            */
/* Below are the functions to all the extended effects.                       */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Filter() (0xE0) changes the filter.                                        */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::Filter(Channel &chan)
{
	amigaFilter = (chan.trackLine.effectArg & 0x01) == 0;
}



/******************************************************************************/
/* FinePortUp() (0xE1) fine slide the frequency up.                           */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::FinePortUp(APChannel *apChan, Channel &chan)
{
	if (counter == 0)
	{
		lowMask = 0x0f;
		PortUp(apChan, chan);
	}
}



/******************************************************************************/
/* FinePortDown() (0xE2) fine slide the frequency down.                       */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::FinePortDown(APChannel *apChan, Channel &chan)
{
	if (counter == 0)
	{
		lowMask = 0x0f;
		PortDown(apChan, chan);
	}
}



/******************************************************************************/
/* SetGlissCon() (0xE3) sets a new glissando control.                         */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::SetGlissCon(Channel &chan)
{
	chan.glissFunk &= 0x0f;
	chan.glissFunk |= (chan.trackLine.effectArg & 0x0f);
}



/******************************************************************************/
/* SetVibCon() (0xE4) sets a new vibrato waveform.                            */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::SetVibCon(Channel &chan)
{
	chan.waveControl &= 0xf0;
	chan.waveControl |= (chan.trackLine.effectArg & 0x0f);
}



/******************************************************************************/
/* SetFineTune() (0xE5) changes the finetune.                                 */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::SetFineTune(Channel &chan)
{
	chan.fineTune = chan.trackLine.effectArg & 0x0f;
}



/******************************************************************************/
/* JumpLoop() (0xE6) jump to pattern loop position.                           */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::JumpLoop(Channel &chan)
{
	uint8 arg;

	if (counter == 0)
	{
		arg = chan.trackLine.effectArg & 0x0f;

		if (arg != 0)
		{
			// Jump to the loop currently set
			if (chan.loopCount == 0)
				chan.loopCount = arg;
			else
				chan.loopCount--;

			if (chan.loopCount != 0)
			{
				breakPos  = chan.pattPos;
				breakFlag = true;
			}
		}
		else
		{
			// Set the loop start point
			chan.pattPos = (uint8)patternPos;
		}
	}
}



/******************************************************************************/
/* SetTreCon() (0xE7) sets a new tremolo waveform.                            */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::SetTreCon(Channel &chan)
{
	chan.waveControl &= 0x0f;
	chan.waveControl |= ((chan.trackLine.effectArg & 0x0f) << 4);
}



/******************************************************************************/
/* RetrigNote() (0xE9) retrigs the current note.                              */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::RetrigNote(APChannel *apChan, Channel &chan)
{
	uint8 arg;

	arg = chan.trackLine.effectArg & 0x0f;

	if (arg != 0)
	{
		if (!((counter == 0) && (chan.trackLine.note != 0)))
		{
			if ((counter % arg) == 0)
			{
				// Retrig the sample
				if (chan.length != 0)
				{
					apChan->PlaySample(chan.start, 0, chan.length * 2);

					if (chan.loopLength != 0)
						apChan->SetLoop(chan.loopStart - chan.start, chan.loopLength * 2);

					apChan->SetAmigaPeriod(chan.period);
				}
				else
					apChan->Mute();
			}
		}
	}
}



/******************************************************************************/
/* VolumeFineUp() (0xEA) fine slide the volume up.                            */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::VolumeFineUp(APChannel *apChan, Channel &chan)
{
	if (counter == 0)
	{
		chan.volume += (chan.trackLine.effectArg & 0x0f);
		if (chan.volume > 64)
			chan.volume = 64;
		
		// Set the volume in the NotePlayer structure
		apChan->SetVolume(chan.volume * 4);
	}
}



/******************************************************************************/
/* VolumeFineDown() (0xEB) fine slide the volume down.                        */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::VolumeFineDown(APChannel *apChan, Channel &chan)
{
	if (counter == 0)
	{
		chan.volume -= (chan.trackLine.effectArg & 0x0f);
		if (chan.volume < 0)
			chan.volume = 0;
	}

	// Set the volume in the NotePlayer structure
	apChan->SetVolume(chan.volume * 4);
}



/******************************************************************************/
/* NoteCut() (0xEC) stops the current note for playing.                       */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::NoteCut(APChannel *apChan, Channel &chan)
{
	if ((chan.trackLine.effectArg & 0x0f) == counter)
	{
		chan.volume = 0;
		apChan->SetVolume(0);
	}
}



/******************************************************************************/
/* NoteDelay() (0xED) waits a little while before playing.                    */
/*                                                                            */
/* Input:  "apChan" is a pointer to the Channel object.                       */
/*         "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::NoteDelay(APChannel *apChan, Channel &chan)
{
	if (((chan.trackLine.effectArg & 0x0f) == counter) && (chan.trackLine.note != 0))
	{
		// Retrig the sample
		if (chan.length != 0)
		{
			apChan->PlaySample(chan.start, 0, chan.length * 2);

			if (chan.loopLength != 0)
				apChan->SetLoop(chan.loopStart - chan.start, chan.loopLength * 2);

			apChan->SetAmigaPeriod(chan.period);
		}
		else
			apChan->Mute();
	}
}



/******************************************************************************/
/* PatternDelay() (0xEE) pauses the pattern for a little while.               */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::PatternDelay(Channel &chan)
{
	if ((counter == 0) && (pattDelTime2 == 0))
		pattDelTime = (chan.trackLine.effectArg & 0x0f) + 1;
}



/******************************************************************************/
/* FunkIt() (0xEF) inverts the loop.                                          */
/*                                                                            */
/* Input:  "chan" is a reference to the channel structure.                    */
/******************************************************************************/
void ModTracker::FunkIt(Channel &chan)
{
	uint8 arg;

	if (counter == 0)
	{
		arg = (chan.trackLine.effectArg & 0x0f) << 4;

		chan.glissFunk &= 0x0f;
		chan.glissFunk |= arg;

		if (arg != 0)
			UpdateFunk(chan);
	}
}



/******************************************************************************/
/* AMHandler() handles StarTrekker AM samples.                                */
/******************************************************************************/
void ModTracker::AMHandler(void)
{
	AMSample *amSamp;
 	Channel *chan;
 	uint16 i, degree;
 	int32 vibVal;
 	bool flag;

	for (i = 0; i < channelNum; i++)
	{
		chan = &channels[i];

		if (chan->amSample)
		{
			amSamp = &amData[chan->sampleNum - 1];

			switch (chan->amTodo)
			{
				// Do attack 1
				case 1:
				{
					if (chan->curLevel == amSamp->attack1Level)
						chan->amTodo = 2;
					else
					{
						if (chan->curLevel < amSamp->attack1Level)
						{
							chan->curLevel += amSamp->attack1Speed;
							if (chan->curLevel >= amSamp->attack1Level)
							{
								chan->curLevel = amSamp->attack1Level;
								chan->amTodo   = 2;
							}
						}
						else
						{
							chan->curLevel -= amSamp->attack1Speed;
							if (chan->curLevel <= amSamp->attack1Level)
							{
								chan->curLevel = amSamp->attack1Level;
								chan->amTodo   = 2;
							}
						}
					}
					break;
				}

				// Do attack 2
				case 2:
				{
					if (chan->curLevel == amSamp->attack2Level)
						chan->amTodo = 3;
					else
					{
						if (chan->curLevel < amSamp->attack2Level)
						{
							chan->curLevel += amSamp->attack2Speed;
							if (chan->curLevel >= amSamp->attack2Level)
							{
								chan->curLevel = amSamp->attack2Level;
								chan->amTodo   = 3;
							}
						}
						else
						{
							chan->curLevel -= amSamp->attack2Speed;
							if (chan->curLevel <= amSamp->attack2Level)
							{
								chan->curLevel = amSamp->attack2Level;
								chan->amTodo   = 3;
							}
						}
					}
					break;
				}

				// Do sustain
				case 3:
				{
					if (chan->curLevel == amSamp->sustainLevel)
						chan->amTodo = 4;
					else
					{
						if (chan->curLevel < amSamp->sustainLevel)
						{
							chan->curLevel += amSamp->decaySpeed;
							if (chan->curLevel >= amSamp->sustainLevel)
							{
								chan->curLevel = amSamp->sustainLevel;
								chan->amTodo   = 4;
							}
						}
						else
						{
							chan->curLevel -= amSamp->decaySpeed;
							if (chan->curLevel <= amSamp->sustainLevel)
							{
								chan->curLevel = amSamp->sustainLevel;
								chan->amTodo   = 4;
							}
						}
					}
					break;
				}

				// Do sustain delay
				case 4:
				{
					chan->sustainCounter--;
					if (chan->sustainCounter < 0)
						chan->amTodo = 5;

					break;
				}

				// Do release
				case 5:
				{
					chan->curLevel -= amSamp->releaseSpeed;
					if (chan->curLevel <= 0)
					{
						chan->amTodo   = 0;
						chan->curLevel = 0;
						chan->amSample = false;
					}
					break;
				}
			}

			// Set the volume
			virtChannels[i]->SetVolume(chan->curLevel);

			// Do pitch fall
			chan->period += amSamp->pitchFall;

			// Do vibrato
			vibVal = amSamp->vibAmp;
			if (vibVal)
			{
				flag   = false;
				degree = chan->vibDegree;
				if (degree >= 180)
				{
					degree -= 180;
					flag    = true;
				}

				vibVal = amSinus[degree] * amSamp->vibAmp / 128;
				if (flag)
					vibVal = -vibVal;
			}

			// Set new frequency
			virtChannels[i]->SetAmigaPeriod(chan->period + vibVal);

			chan->vibDegree += amSamp->vibSpeed;
			if (chan->vibDegree >= 360)
				chan->vibDegree -= 360;
		}
	}

	// Generate noise waveform
	for (i = 0; i < 32; i++)
		amWaveforms[3][i] = (int8)PSystem::Random(255);
}
