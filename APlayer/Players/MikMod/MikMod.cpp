/******************************************************************************/
/* MikMod Player Interface.                                                   */
/*                                                                            */
/* Original player by Divine Entertainment.                                   */
/* Maintained at the moment by Miodrag Vallet.                                */
/* Ported to APlayer by Thomas Neumann.                                       */
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
#include "PFile.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APChannel.h"

// Player headers
#include "MikMod.h"
#include "MikModTables.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.26f



/******************************************************************************/
/* Other defines                                                              */
/******************************************************************************/
#define HIGH_OCTAVE			2		// Number of above-range octaves



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MikMod::MikMod(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MikMod::~MikMod(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float MikMod::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 MikMod::GetSupportFlags(int32 index)
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
PString MikMod::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_MIK_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString MikMod::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_MIK_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString MikMod::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_MIK_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a MikMod module.             */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a file object with the file to check.       */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result MikMod::ModuleCheck(int32 index, PFile *file)
{
	// Check the module size
	if (file->GetLength() < 25)
		return (AP_UNKNOWN);

	// Check the mark
	file->SeekToBegin();

	if ((file->Read_B_UINT32() == 'APUN') && (file->Read_B_UINT16() == 0x0106))
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
ap_result MikMod::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	ap_result retVal;

	// Initialize the UNIMOD structure
	of.tracks      = NULL;
	of.patterns    = NULL;
	of.pattRows    = NULL;
	of.positions   = NULL;
	of.instruments = NULL;
	of.samples     = NULL;
	of.control     = NULL;
	of.voice       = NULL;

	// Parse the uni module and create structures to use
	retVal = CreateUniStructs(file, errorStr);

	if (retVal != AP_OK)
		FreeAll();

	return (retVal);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikMod::InitPlayer(int32 index)
{
	SongTime *songTime;
	uint16 pos, row, pattNum;
	int16 newPos, newRow;
	int16 startPos = 0;
	int16 posCount;
	uint16 startRow = 0;
	uint8 chan;
	uint8 **tracks;
	uint8 *loopCount;
	uint8 *loopPos;
	uint16 trackNum, rowNum;
	uint8 opcode, effArg;
	PosInfo posInfo;
	uint8 curSpeed;
	uint16 curTempo;
	uint8 frameDelay;
	float total;
	bool pattBreak, posBreak, getOut;
	bool fullStop;

	// Initialize mixer stuff
	md_sngChn = max(of.numChn, of.numVoices);

	// Initialize effect function table
	effects[0]  = DoNothing;		// 0
	effects[1]  = DoNothing;		// UNI_NOTE
	effects[2]  = DoNothing;		// UNI_INSTRUMENT
	effects[3]  = DoPTEffect0;		// UNI_PTEFFECT0
	effects[4]  = DoPTEffect1;		// UNI_PTEFFECT1
	effects[5]  = DoPTEffect2;		// UNI_PTEFFECT2
	effects[6]  = DoPTEffect3;		// UNI_PTEFFECT3
	effects[7]  = DoPTEffect4;		// UNI_PTEFFECT4
	effects[8]  = DoPTEffect5;		// UNI_PTEFFECT5
	effects[9]  = DoPTEffect6;		// UNI_PTEFFECT6
	effects[10] = DoPTEffect7;		// UNI_PTEFFECT7
	effects[11] = DoPTEffect8;		// UNI_PTEFFECT8
	effects[12] = DoPTEffect9;		// UNI_PTEFFECT9
	effects[13] = DoPTEffectA;		// UNI_PTEFFECTA
	effects[14] = DoPTEffectB;		// UNI_PTEFFECTB
	effects[15] = DoPTEffectC;		// UNI_PTEFFECTC
	effects[16] = DoPTEffectD;		// UNI_PTEFFECTD
	effects[17] = DoPTEffectE;		// UNI_PTEFFECTE
	effects[18] = DoPTEffectF;		// UNI_PTEFFECTF
	effects[19] = DoS3MEffectA;		// UNI_S3MEFFECTA
	effects[20] = DoS3MEffectD;		// UNI_S3MEFFECTD
	effects[21] = DoS3MEffectE;		// UNI_S3MEFFECTE
	effects[22] = DoS3MEffectF;		// UNI_S3MEFFECTF
	effects[23] = DoS3MEffectI;		// UNI_S3MEFFECTI
	effects[24] = DoS3MEffectQ;		// UNI_S3MEFFECTQ
	effects[25] = DoS3MEffectR;		// UNI_S3MEFFECTR
	effects[26] = DoS3MEffectT;		// UNI_S3MEFFECTT
	effects[27] = DoS3MEffectU;		// UNI_S3MEFFECTU
	effects[28] = DoKeyOff;			// UNI_KEYOFF
	effects[29] = DoKeyFade;		// UNI_KEYFADE
	effects[30] = DoVolEffects;		// UNI_VOLEFFECTS
	effects[31] = DoPTEffect4;		// UNI_XMEFFECT4
	effects[32] = DoXMEffect6;		// UNI_XMEFFECT6
	effects[33] = DoXMEffectA;		// UNI_XMEFFECTA
	effects[34] = DoXMEffectE1;		// UNI_XMEFFECTE1
	effects[35] = DoXMEffectE2;		// UNI_XMEFFECTE2
	effects[36] = DoXMEffectEA;		// UNI_XMEFFECTEA
	effects[37] = DoXMEffectEB;		// UNI_XMEFFECTEB
	effects[38] = DoXMEffectG;		// UNI_XMEFFECTG
	effects[39] = DoXMEffectH;		// UNI_XMEFFECTH
	effects[40] = DoXMEffectL;		// UNI_XMEFFECTL
	effects[41] = DoXMEffectP;		// UNI_XMEFFECTP
	effects[42] = DoXMEffectX1;		// UNI_XMEFFECTX1
	effects[43] = DoXMEffectX2;		// UNI_XMEFFECTX2
	effects[44] = DoITEffectG;		// UNI_ITEFFECTG
	effects[45] = DoITEffectH;		// UNI_ITEFFECTH
	effects[46] = DoITEffectI;		// UNI_ITEFFECTI
	effects[47] = DoITEffectM;		// UNI_ITEFFECTM
	effects[48] = DoITEffectN;		// UNI_ITEFFECTN
	effects[49] = DoITEffectP;		// UNI_ITEFFECTP
	effects[50] = DoITEffectT;		// UNI_ITEFFECTT
	effects[51] = DoITEffectU;		// UNI_ITEFFECTU
	effects[52] = DoITEffectW;		// UNI_ITEFFECTW
	effects[53] = DoITEffectY;		// UNI_ITEFFECTY
	effects[54] = DoNothing;		// UNI_ITEFFECTZ
	effects[55] = DoITEffectS0;		// UNI_ITEFFECTS0
	effects[56] = DoULTEffect9;		// UNI_ULTEFFECT9
	effects[57] = DoMEDSpeed;		// UNI_MEDSPEED
	effects[58] = DoMEDEffectF1;	// UNI_MEDEFFECTF1
	effects[59] = DoMEDEffectF2;	// UNI_MEDEFFECTF2
	effects[60] = DoMEDEffectF3;	// UNI_MEDEFFECTF3
	effects[61] = DoOktArp;			// UNI_OKTARP

	// Allocate a temporary array holding the track pointers
	tracks = new uint8 *[of.numChn];
	if (tracks == NULL)
		return (false);

	// Allocate temporary arrays used in the E6x effect calculations
	loopCount = new uint8[of.numChn];
	if (loopCount == NULL)
	{
		delete[] tracks;
		return (false);
	}

	loopPos = new uint8[of.numChn];
	if (loopPos == NULL)
	{
		delete[] loopCount;
		delete[] tracks;
		return (false);
	}

	do
	{
		// Allocate a new sub song structure
		songTime = new SongTime;
		if (songTime == NULL)
		{
			delete[] loopPos;
			delete[] loopCount;
			return (false);
		}

		// Set the start position
		songTime->startPos = startPos;

		// Initialize calculation variables
		if (of.initSpeed != 0)
			curSpeed = of.initSpeed < 32 ? of.initSpeed : 32;
		else
			curSpeed = 6;

		curTempo = of.initTempo < 32 ? 32 : of.initTempo;
		total    = 0.0f;
		fullStop = false;

		// Initialize loop arrays
		memset(loopCount, 0, of.numChn * sizeof(uint8));
		memset(loopPos, 0, of.numChn * sizeof(uint8));

		// Calculate the position times
		for (pos = startPos, posCount = startPos; pos < of.numPos; pos++, posCount++)
		{
			// Add the position information to the list
			posInfo.speed = curSpeed;
			posInfo.tempo = curTempo;
			posInfo.time.SetTimeSpan(total);

			if ((pos - startPos) >= songTime->posInfoList.CountItems())
				songTime->posInfoList.AddTail(posInfo);

			// Get the pattern number to play
			pattNum = of.positions[pos];
			if (pattNum == LAST_PATTERN)
			{
				// End of song
				break;
			}

			// Get pointers to all the tracks
			for (chan = 0; chan < of.numChn; chan++)
			{
				trackNum = of.patterns[pattNum * of.numChn + chan];
				if (trackNum < of.numTrk)
					tracks[chan] = of.tracks[trackNum];
				else
					tracks[chan] = NULL;
			}

			// Clear some flags
			pattBreak = false;
			posBreak  = false;
			getOut    = false;

			// Get number of rows in the current track
			rowNum = of.pattRows[pattNum];

			for (row = startRow; row < rowNum; row++)
			{
				// Reset the start row
				startRow   = 0;
				newRow     = -2;
				frameDelay = 1;
				newPos     = -1;

				for (chan = 0; chan < of.numChn; chan++)
				{
					// Did we have a valid track number?
					if (tracks[chan] == NULL)
						continue;

					// Set the row pointer
					uniTrk.UniSetRow(uniTrk.UniFindRow(tracks[chan], row));

					// Read and parse the opcodes for the entire row
					while ((opcode = uniTrk.UniGetByte()) != 0)
					{
						// Parse some of the opcodes
						switch (opcode)
						{
							// ProTracker set speed
							case UNI_PTEFFECTF:
							{
								// Get the speed
								effArg = uniTrk.UniGetByte();

								// Parse it
								if (effArg >= of.bpmLimit)
									curTempo = effArg;
								else
								{
									if (effArg != 0)
										curSpeed = (effArg >= of.bpmLimit) ? of.bpmLimit - 1 : effArg;
								}
								break;
							}

							// ScreamTracker set speed
							case UNI_S3MEFFECTA:
							{
								// Get the speed
								effArg = uniTrk.UniGetByte();

								if (effArg > 128)
									effArg -= 128;

								if (effArg != 0)
									curSpeed = effArg;
								break;
							}

							// ScreamTracker set tempo
							case UNI_S3MEFFECTT:
							{
								// Get the tempo
								effArg   = uniTrk.UniGetByte();
								curTempo = (effArg < 32) ? 32 : effArg;
								break;
							}

							// ProTracker pattern break
							case UNI_PTEFFECTD:
							{
								startRow = uniTrk.UniGetByte();

								if ((pos == of.numPos - 2) && (startRow == 0) && posBreak)
									posBreak = false;

								if (!posBreak)
								{
									if (newPos == -1)
										fullStop = false;

									pattBreak = true;
								}

								getOut = true;
								break;
							}

							// ProTracker pattern jump
							case UNI_PTEFFECTB:
							{
								// Get the new position
								effArg = uniTrk.UniGetByte();

								// Do we jump to a lower position
								if (effArg < pos)
								{
									newPos   = effArg - 1;
									fullStop = true;
								}
								else
								{
									if (effArg == pos)
										fullStop = true;
									else
										fullStop = false;

									newPos   = effArg - 1;
									posBreak = true;
								}

								getOut = true;
								break;
							}

							// ProTracker extra effects
							case UNI_PTEFFECTE:
							{
								// Get the effect
								effArg = uniTrk.UniGetByte();

								// Pattern loop?
								if ((effArg & 0xf0) == 0x60)
								{
									effArg &= 0x0f;

									if (effArg != 0)
									{
										// Jump to the loop currently set
										if (loopCount[chan] == 0)
											loopCount[chan] = effArg;
										else
											loopCount[chan]--;

										if (loopCount[chan] != 0)
										{
											// Set new row
											newRow = loopPos[chan] - 1;
										}
									}
									else
									{
										// Set the loop start point
										loopPos[chan] = row;
									}
									break;
								}

								// Pattern delay?
								if ((effArg & 0xf0) == 0xe0)
								{
									// Get the delay count
									frameDelay = (effArg & 0x0f) + 1;
								}
								break;
							}

							default:
							{
								// Just skip the opcode
								uniTrk.UniSkipOpcode();
								break;
							}
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
	while (posCount < (of.numPos - 1));

	// Delete temporary arrays
	delete[] loopPos;
	delete[] loopCount;
	delete[] tracks;

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void MikMod::EndPlayer(int32 index)
{
	int32 i, count;

	// Empty the song time list
	count = songTimeList.CountItems();
	for (i = 0; i < count; i++)
		delete songTimeList.GetItem(i);

	songTimeList.MakeEmpty();

	// Free the module
	FreeAll();
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void MikMod::InitSound(int32 index, uint16 songNum)
{
	SongTime *songTime;
	int32 t;

	// Get the song time structure
	songTime = songTimeList.GetItem(songNum);

	// Remember the subsong
	currentSong     = songNum;

	of.extSpd       = true;
	of.panFlag      = true;
	of.wrap         = true;
	of.loop         = true;
	of.fadeOut      = false;

	of.relSpd       = 0;

	of.sngTime      = 0;
	of.sngRemainder = 0;

	of.pat_repCrazy = 0;
	of.sngPos       = songTime->startPos;

	if (of.initSpeed != 0)
		of.sngSpd = of.initSpeed < 32 ? of.initSpeed : 32;
	else
		of.sngSpd = 6;

	of.volume       = of.initVolume > 128 ? 128 : of.initVolume;

	of.vbTick       = of.sngSpd;
	of.patDly       = 0;
	of.patDly2      = 0;
	of.bpm          = of.initTempo < 32 ? 32 : of.initTempo;
	of.realChn      = 0;

	of.patPos       = 0;
	of.posJmp       = 2;			// Make sure the player fetches the first note
	of.numRow       = 0xffff;
 	of.patBrk       = 0;

	// Allocate needed structures
	if ((of.control = new MP_CONTROL[of.numChn]) == NULL)
		return;

	if ((of.voice = new MP_VOICE[md_sngChn]) == NULL)
		return;

	// Make sure the player doesn't start with garbage
	memset(of.control, 0, of.numChn * sizeof(MP_CONTROL));
	memset(of.voice, 0, md_sngChn * sizeof(MP_VOICE));

	for (t = 0; t < of.numChn; t++)
	{
		of.control[t].main.chanVol = of.chanVol[t];
		of.control[t].main.panning = of.panning[t];
	}

	// Tell APlayer about the initial BPM tempo
	SetTempo(of.bpm);
	bpmTempo = of.bpm;
}



/******************************************************************************/
/* EndSound() ends the current song.                                          */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void MikMod::EndSound(int32 index)
{
	delete[] of.control;
	delete[] of.voice;
	of.control = NULL;
	of.voice   = NULL;
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void MikMod::Play(void)
{
	// Play the module
	Player_HandleTick();
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString MikMod::GetModuleName(void)
{
	return (of.songName);
}



/******************************************************************************/
/* GetVirtualChannels() returns the number of channels the module want to     */
/*      reserve.                                                              */
/*                                                                            */
/* Output: Is the number of required channels.                                */
/******************************************************************************/
uint16 MikMod::GetVirtualChannels(void)
{
	return (of.numVoices);
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 MikMod::GetModuleChannels(void)
{
	return (of.numChn);
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *MikMod::GetSubSongs(void)
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
int16 MikMod::GetSongLength(void)
{
	return (of.numPos);
}



/******************************************************************************/
/* GetSongPosition() returns the current position of the playing song.        */
/*                                                                            */
/* Output: Is the current position.                                           */
/******************************************************************************/
int16 MikMod::GetSongPosition(void)
{
	return (of.sngPos);
}



/******************************************************************************/
/* SetSongPosition() sets the current position of the playing song.           */
/*                                                                            */
/* Input:  "pos" is the new position.                                         */
/******************************************************************************/
void MikMod::SetSongPosition(int16 pos)
{
	SongTime *songTime;
	PosInfo posInfo;
	uint8 t;

	// Get the song time structure
	songTime = songTimeList.GetItem(currentSong);

	// Change the position
	if (pos >= of.numPos)
		pos = of.numPos;

	of.posJmp = 2;
	of.patBrk = 0;
	of.sngPos = pos;
	of.vbTick = 0;

	// Change the speed
	if ((pos < songTime->startPos) || (pos >= songTime->posInfoList.CountItems()))
	{
		of.sngSpd = of.initSpeed ? (of.initSpeed < 32 ? of.initSpeed : 32) : 6;
		of.bpm    = of.initTempo < 32 ? 32 : of.initTempo;
		SetTempo(of.bpm);
	}
	else
	{
		posInfo   = songTime->posInfoList.GetItem(pos - songTime->startPos);
		of.sngSpd = posInfo.speed;
		of.bpm    = posInfo.tempo;
		SetTempo(of.bpm);
	}

	for (t = 0; t < md_sngChn; t++)
	{
		VoiceStop(t);
		of.voice[t].main.i = NULL;
		of.voice[t].main.s = NULL;
	}

	for (t = 0; t < of.numChn; t++)
	{
		of.control[t].main.i = NULL;
		of.control[t].main.s = NULL;
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
PTimeSpan MikMod::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
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
	for (; i < of.numPos; i++)
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
bool MikMod::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 5)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Song Length
		case 0:
		{
			description.LoadString(res, IDS_MIK_INFODESCLINE0);
			value.SetUNumber(of.numPos);
			break;
		}

		// Used Patterns
		case 1:
		{
			description.LoadString(res, IDS_MIK_INFODESCLINE1);
			value.SetUNumber(of.numPat);
			break;
		}

		// Used Instruments
		case 2:
		{
			description.LoadString(res, IDS_MIK_INFODESCLINE2);
			value.SetUNumber(of.numIns);
			break;
		}

		// Used Samples
		case 3:
		{
			description.LoadString(res, IDS_MIK_INFODESCLINE3);
			value.SetUNumber(of.numSmp);
			break;
		}

		// Actual Speed (BPM)
		case 4:
		{
			description.LoadString(res, IDS_MIK_INFODESCLINE4);
			value.SetUNumber(of.bpm);
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* GetInstrumentInfo() fills out the APInstInfo structure given with the      */
/*      instrument information of the instrument number given.                */
/*                                                                            */
/* Input:  "num" is the instrument number starting from 0.                    */
/*         "info" is a pointer to an APInstInfo structure to fill out.        */
/*                                                                            */
/* Output: True if the information are returned, false if not.                */
/******************************************************************************/
bool MikMod::GetInstrumentInfo(uint32 num, APInstInfo *info)
{
	INSTRUMENT *inst;

	// Check to see if there is instruments at all in the module
	if (!(of.flags & UF_INST))
		return (false);

	// Now check to see if we have reached the maximum number of instruments
	if (num >= of.numIns)
		return (false);

	// Get pointer to the instrument
	inst = &of.instruments[num];

	// Fill out the instrument structure
	info->name  = inst->insName;
	info->flags = 0;

	// Fill out the note samples
	for (int8 i = 0; i < 10 * 12; i++)
		info->notes[i] = inst->sampleNumber[i];

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
bool MikMod::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	SAMPLE *samp;

	// Check to see if we have reached the maximum number of samples
	if (num >= of.numSmp)
		return (false);

	// Get the sample pointer
	samp = &of.samples[num];

	// Fill out the sample info structure
	info->name       = samp->sampleName;
	info->flags      = 0;
	info->type       = apSample;
	info->bitSize    = (samp->flags & SF_16BITS) ? 16 : 8;
	info->middleC    = (uint32)ceil(GetFrequency(of.flags, GetPeriod(of.flags, 96, samp->speed)));
	info->volume     = samp->volume * 4;
	info->panning    = (samp->panning == PAN_SURROUND) ? APPAN_SURROUND : samp->panning;
	info->address    = samp->handle;
	info->length     = samp->length;
	info->loopStart  = samp->loopStart;
	info->loopLength = samp->loopEnd - samp->loopStart;

	// Add extra loop flags if any
	if ((samp->flags & SF_LOOP) && (samp->loopStart < samp->loopEnd))
	{
		// Set loop flag
		info->flags |= APSAMP_LOOP;

		// Is the loop ping-pong?
		if (samp->flags & SF_BIDI)
			info->flags |= APSAMP_PINGPONG;
	}

	return (true);
}



/******************************************************************************/
/* CreateUniStructs() will create all the structs needed to play the module.  */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*         "errorStr" is a reference where to store the error string.         */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result MikMod::CreateUniStructs(PFile *file, PString &errorStr)
{
	uint32 v, w;
	SAMPLE *s;
	INSTRUMENT *i;
	PCharSet_UTF8 charSet;

	// Skip the mark and version
	file->Read_B_UINT32();
	file->Read_B_UINT16();

	// Read the header
	of.flags      = file->Read_B_UINT16();
	of.numChn     = file->Read_UINT8();
	of.numVoices  = file->Read_UINT8();
	of.numPos     = file->Read_B_UINT16();
	of.numPat     = file->Read_B_UINT16();
	of.numTrk     = file->Read_B_UINT16();
	of.numIns     = file->Read_B_UINT16();
	of.numSmp     = file->Read_B_UINT16();
	of.repPos     = file->Read_B_UINT16();
	of.initSpeed  = file->Read_UINT8();
	of.initTempo  = file->Read_UINT8();
	of.initVolume = file->Read_UINT8();
	of.bpmLimit   = file->Read_B_UINT16();

	if (file->IsEOF())
	{
		errorStr.LoadString(res, IDS_MIK_ERR_LOADING_HEADER);
		return (AP_ERROR);
	}

	of.songName = file->ReadString(&charSet);
	of.comment  = file->ReadString(&charSet);

	if (file->IsEOF())
	{
		errorStr.LoadString(res, IDS_MIK_ERR_LOADING_HEADER);
		return (AP_ERROR);
	}

	// Allocate memory to hold all the information
	if (!(AllocSamples()))
	{
		errorStr.LoadString(res, IDS_MIK_ERR_MEMORY);
		return (AP_ERROR);
	}

	if (!(AllocTracks()))
	{
		errorStr.LoadString(res, IDS_MIK_ERR_MEMORY);
		return (AP_ERROR);
	}

	if (!(AllocPatterns()))
	{
		errorStr.LoadString(res, IDS_MIK_ERR_MEMORY);
		return (AP_ERROR);
	}

	if (!(AllocPositions(of.numPos)))
	{
		errorStr.LoadString(res, IDS_MIK_ERR_MEMORY);
		return (AP_ERROR);
	}

	// Read arrays
	for (v = 0; v < of.numPos; v++)
		of.positions[v] = file->Read_B_UINT16();

	for (v = 0; v < of.numChn; v++)
		of.panning[v] = file->Read_B_UINT16();

	for (v = 0; v < of.numChn; v++)
		of.chanVol[v] = file->Read_UINT8();

	if (file->IsEOF())
	{
		errorStr.LoadString(res, IDS_MIK_ERR_LOADING_HEADER);
		return (AP_ERROR);
	}

	// Load sample headers
	s = of.samples;
	for (v = 0; v < of.numSmp; v++, s++)
	{
		s->flags      = file->Read_B_UINT16();
		s->speed      = file->Read_B_UINT32();
		s->volume     = file->Read_UINT8();
		s->panning    = file->Read_B_UINT16();
		s->length     = file->Read_B_UINT32();
		s->loopStart  = file->Read_B_UINT32();
		s->loopEnd    = file->Read_B_UINT32();
		s->susBegin   = file->Read_B_UINT32();
		s->susEnd     = file->Read_B_UINT32();

		s->globVol    = file->Read_UINT8();
		s->vibFlags   = file->Read_UINT8();
		s->vibType    = file->Read_UINT8();
		s->vibSweep   = file->Read_UINT8();
		s->vibDepth   = file->Read_UINT8();
		s->vibRate    = file->Read_UINT8();
		s->sampleName = file->ReadString(&charSet);

		// Reality check for loop settings
		if (s->loopEnd > s->length)
			s->loopEnd = s->length;

		if (s->loopStart >= s->loopEnd)
			s->flags &= ~SF_LOOP;

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_MIK_ERR_LOADING_SAMPLEINFO);
			return (AP_ERROR);
		}
	}

	// Load instruments
	if (of.flags & UF_INST)
	{
		if (!(AllocInstruments()))
		{
			errorStr.LoadString(res, IDS_MIK_ERR_MEMORY);
			return (AP_ERROR);
		}

		i = of.instruments;
		for (v = 0; v < of.numIns; v++, i++)
		{
			i->flags        = file->Read_UINT8();
			i->nnaType      = file->Read_UINT8();
			i->dca          = file->Read_UINT8();
			i->dct          = file->Read_UINT8();
			i->globVol      = file->Read_UINT8();
			i->panning      = file->Read_B_UINT16();
			i->pitPanSep    = file->Read_UINT8();
			i->pitPanCenter = file->Read_UINT8();
			i->rVolVar      = file->Read_UINT8();
			i->rPanVar      = file->Read_UINT8();

			i->volFade      = file->Read_B_UINT16();

			i->volFlg       = file->Read_UINT8();
			i->volPts       = file->Read_UINT8();
			i->volSusBeg    = file->Read_UINT8();
			i->volSusEnd    = file->Read_UINT8();
			i->volBeg       = file->Read_UINT8();
			i->volEnd       = file->Read_UINT8();

			for (w = 0; w < 32; w++)
			{
				i->volEnv[w].pos = file->Read_B_UINT16();
				i->volEnv[w].val = file->Read_B_UINT16();
			}

			i->panFlg       = file->Read_UINT8();
			i->panPts       = file->Read_UINT8();
			i->panSusBeg    = file->Read_UINT8();
			i->panSusEnd    = file->Read_UINT8();
			i->panBeg       = file->Read_UINT8();
			i->panEnd       = file->Read_UINT8();

			for (w = 0; w < 32; w++)
			{
				i->panEnv[w].pos = file->Read_B_UINT16();
				i->panEnv[w].val = file->Read_B_UINT16();
			}

			i->pitFlg       = file->Read_UINT8();
			i->pitPts       = file->Read_UINT8();
			i->pitSusBeg    = file->Read_UINT8();
			i->pitSusEnd    = file->Read_UINT8();
			i->pitBeg       = file->Read_UINT8();
			i->pitEnd       = file->Read_UINT8();

			for (w = 0; w < 32; w++)
			{
				i->pitEnv[w].pos = file->Read_B_UINT16();
				i->pitEnv[w].val = file->Read_B_UINT16();
			}

			for (w = 0; w < 120; w++)
				i->sampleNumber[w] = file->Read_B_UINT16();

			for (w = 0; w < 120; w++)
				i->sampleNote[w] = file->Read_UINT8();

			i->insName = file->ReadString(&charSet);

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_MIK_ERR_LOADING_INSTRUMENTINFO);
				return (AP_ERROR);
			}
		}
	}

	// Read patterns
	for (v = 0; v < of.numPat; v++)
		of.pattRows[v] = file->Read_B_UINT16();

	for (v = 0; v < (uint32)(of.numPat * of.numChn); v++)
		of.patterns[v] = file->Read_B_UINT16();

	// Read tracks
	for (v = 0; v < of.numTrk; v++)
		of.tracks[v] = TrkRead(file);

	if (file->IsEOF())
	{
		errorStr.LoadString(res, IDS_MIK_ERR_LOADING_TRACKS);
		return (AP_ERROR);
	}

	// Calculate the sample addresses and fix the samples
	return (FindSamples(file, errorStr));
}



/******************************************************************************/
/* TrkRead() allocates and read one track.                                    */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*                                                                            */
/* Output: A pointer to the tracker loaded.                                   */
/******************************************************************************/
uint8 *MikMod::TrkRead(PFile *file)
{
	uint8 *t;
	uint16 len, i;

	len = file->Read_B_UINT16();
	t   = new uint8[len];
	for (i = 0; i < len; i++)
		t[i] = file->Read_UINT8();

	return (t);
}



/******************************************************************************/
/* FindSamples() will find the sample addresses and fix them so all samples   */
/*      signed.                                                               */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*         "errorStr" is a reference where to store the error string.         */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
#define SLBUFSIZE		2048

ap_result MikMod::FindSamples(PFile *file, PString &errorStr)
{
	int32 v, w, length;
	SAMPLE *s;
	uint8 *samp;

	s = of.samples;
	for (v = 0; v < of.numSmp; v++, s++)
	{
		// Calculate the length of the sample
		length = s->length;

		if (length != 0)
		{
			int16 old = 0;
			int32 toDo;
			ITPACK status;
			uint16 inCnt;
			bool result;
			int32 cBlock = 0;
			uint8 *dest;

			if (s->flags & SF_16BITS)
				length *= 2;

			if (s->flags & SF_STEREO)
				length *= 2;

			// Allocate memory to hold the sample
			s->handle = new uint8[length];
			if (s->handle == NULL)
			{
				errorStr.LoadString(res, IDS_MIK_ERR_MEMORY);
				return (AP_ERROR);
			}

			// Get the length again, because the loop
			// is based on samples, not bytes
			length = s->length;
			dest   = (uint8 *)s->handle;

			while (length)
			{
				toDo = min(length, SLBUFSIZE);

				if (s->flags & SF_ITPACKED)
				{
					// Decompress the sample
					if (!cBlock)
					{
						status.bits = (s->flags & SF_16BITS) ? 17 : 9;
						status.last = status.bufBits = 0;

						// Read the compressed length
						inCnt = file->Read_L_UINT16();

						cBlock = (s->flags & SF_16BITS) ? 0x4000 : 0x8000;

						if (s->flags & SF_DELTA)
							old = 0;
					}

					if (s->flags & SF_16BITS)
						result = DecompressIT16(&status, file, (int16 *)dest, toDo, &inCnt);
					else
						result = DecompressIT8(&status, file, (int8 *)dest, toDo, &inCnt);

					if (!result)
					{
						// Well, some error occurred in the decompressing
						errorStr.LoadString(res, IDS_MIK_ERR_ITPACKING);
						return (AP_ERROR);
					}

					cBlock -= toDo;
				}
				else
				{
					// Read the sample into the memory
					if (s->flags & SF_16BITS)
					{
						if (s->flags & SF_BIG_ENDIAN)
							file->ReadArray_B_UINT16s((uint16 *)dest, toDo);
						else
							file->ReadArray_L_UINT16s((uint16 *)dest, toDo);
					}
					else
						file->Read(dest, toDo);
				}

				// Check for end of file
				if (file->IsEOF())
				{
					errorStr.LoadString(res, IDS_MIK_ERR_LOADING_SAMPLES);
					return (AP_ERROR);
				}

				// Dedelta the sample
				if (s->flags & SF_DELTA)
				{
					samp = dest;

					if (s->flags & SF_16BITS)
					{
						for (w = 0; w < toDo; w++)
						{
							*((int16 *)samp) += old;
							old = *((int16 *)samp);
							samp += 2;
						}
					}
					else
					{
						for (w = 0; w < toDo; w++)
						{
							*samp += old;
							old = *samp++;
						}
					}
				}

				// Convert the sample to signed
				if (!(s->flags & SF_SIGNED))
				{
					samp = dest;

					if (s->flags & SF_16BITS)
					{
						for (w = 0; w < toDo; w++)
						{
							*((int16 *)samp) += (int16)0x8000;
							samp += 2;
						}
					}
					else
					{
						for (w = 0; w < toDo; w++)
							*samp++ += 0x80;
					}
				}

				// Add number of samples to destination buffer
				dest += toDo;

				if (s->flags & SF_16BITS)
					dest += toDo;

				length -= toDo;
			}
		}
	}

	return (AP_OK);
}



/******************************************************************************/
/* DecompressIT8() decompress an 8-bit IT packed sample.                      */
/*                                                                            */
/* Input:  "status" is a pointer to the ITPACK structure.                     */
/*         "file" is a pointer to a file object to the file with the sample.  */
/*         "dest" is a pointer to where to store the unpacked data.           */
/*         "length" is the size of the destination buffer in samples.         */
/*         "inCnt" is a pointer the variable with the compressed length.      */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikMod::DecompressIT8(ITPACK *status, PFile *file, int8 *dest, uint32 length, uint16 *inCnt)
{
	int8 *end = dest + length;
	uint16 x, y, needBits, haveBits, newCount = 0;
	uint16 bits = status->bits;
	uint16 bufBits = status->bufBits;
	int8 last = (int8)status->last;
	uint8 buf = status->buf;

	while (dest < end)
	{
		needBits = newCount ? 3 : bits;
		x        = haveBits = 0;

		while (needBits)
		{
			// Feed buffer
			if (!bufBits)
			{
				if ((*inCnt)--)
					buf = file->Read_UINT8();
				else
					buf = 0;

				bufBits = 8;
			}

			// Get as many bits as necessary
			y     = needBits < bufBits ? needBits : bufBits;
			x    |= (buf & ((1 << y) - 1)) << haveBits;
			buf >>=y;

			bufBits  -= y;
			needBits -= y;
			haveBits += y;
		}

		if (newCount)
		{
			newCount = 0;

			if (++x >= bits)
				x++;

			bits = x;
			continue;
		}

		if (bits < 7)
		{
			if (x == (1 << (bits - 1)))
			{
				newCount = 1;
				continue;
			}
		}
		else if (bits < 9)
		{
			y = (0xff >> (9 - bits)) - 4;
			if ((x > y) && (x <= y + 8))
			{
				if ((x -= y) >= bits)
					x++;

				bits = x;
				continue;
			}
		}
		else if (bits < 10)
		{
			if (x >= 0x100)
			{
				bits = x - 0x100 + 1;
				continue;
			}
		}
		else
		{
			// Error in compressed data
			return (false);
		}

		if (bits < 8)	// Extend sign
			x = ((int8)(x << (8 - bits))) >> (8 - bits);

		*(dest++) = (last += x);
	}

	status->bits    = bits;
	status->bufBits = bufBits;
	status->last    = last;
	status->buf     = buf;

	return (true);
}



/******************************************************************************/
/* DecompressIT16() decompress an 16-bit IT packed sample.                    */
/*                                                                            */
/* Input:  "status" is a pointer to the ITPACK structure.                     */
/*         "file" is a pointer to a file object to the file with the sample.  */
/*         "dest" is a pointer to where to store the unpacked data.           */
/*         "length" is the size of the destination buffer in samples.         */
/*         "inCnt" is a pointer the variable with the compressed length.      */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikMod::DecompressIT16(ITPACK *status, PFile *file, int16 *dest, uint32 length, uint16 *inCnt)
{
	int16 *end = dest + length;
	int32 x, y, needBits, haveBits, newCount = 0;
	uint16 bits = status->bits;
	uint16 bufBits = status->bufBits;
	int16 last = status->last;
	uint8 buf = status->buf;

	while (dest < end)
	{
		needBits = newCount ? 4 : bits;
		x        = haveBits = 0;

		while (needBits)
		{
			// Feed buffer
			if (!bufBits)
			{
				if ((*inCnt)--)
					buf = file->Read_UINT8();
				else
					buf = 0;

				bufBits = 8;
			}

			// Get as many bits as necessary
			y     = needBits < bufBits ? needBits : bufBits;
			x    |= (buf & ((1 << y) - 1)) << haveBits;
			buf >>= y;

			bufBits  -= (uint16)y;
			needBits -= y;
			haveBits += y;
		}

		if (newCount)
		{
			newCount = 0;

			if (++x >= bits)
				x++;

			bits = (uint16)x;
			continue;
		}

		if (bits < 7)
		{
			if (x == (1 << (bits - 1)))
			{
				newCount = 1;
				continue;
			}
		}
		else if (bits < 17)
		{
			y = (0xffff >> (17 - bits)) - 8;
			if ((x > y) && (x <= y + 16))
			{
				if ((x -= y) >= bits)
					x++;

				bits = (uint16)x;
				continue;
			}
		}
		else if (bits < 18)
		{
			if (x >= 0x10000)
			{
				bits = x - 0x10000 + 1;
				continue;
			}
		}
		else
		{
			// Error in compressed data
			return (false);
		}

		if (bits < 16)		// Extend sign
			x = ((int16)(x << (16 - bits))) >> (16 - bits);

		*(dest++) = (last += (int16)x);
	}

	status->bits    = bits;
	status->bufBits = bufBits;
	status->last    = last;
	status->buf     = buf;

	return (true);
}



/******************************************************************************/
/* SetTempo() sets APlayer to the right BPM tempo.                            */
/*                                                                            */
/* Input:  "tempo" is the new tempo.                                          */
/******************************************************************************/
void MikMod::SetTempo(uint16 tempo)
{
	SetBPMTempo(tempo);
	ChangeModuleInfo(4, PString::CreateUNumber(tempo));
}



/******************************************************************************/
/* Player_HandleTick() is the main player function.                           */
/******************************************************************************/
void MikMod::Player_HandleTick(void)
{
	int16 channel;
	int32 max_Volume;

	if (of.sngPos >= of.numPos)
		return;

	// Update time counter (sngTime is in milliseconds (in fact 2^-10))
	of.sngRemainder += (1 << 9) * 5;	// Thus 2.5 * (1 << 10), since fps=0.4 * tempo
	of.sngTime      += of.sngRemainder / of.bpm;
	of.sngRemainder %= of.bpm;

	if (++of.vbTick >= of.sngSpd)
	{
		if (of.pat_repCrazy)
			of.pat_repCrazy = 0;		// Play 2 times row 0
		else
			of.patPos++;

		of.vbTick = 0;

		// Process pattern-delay. of.patDly2 is the counter and of.patDly
		// is the command memory
		if (of.patDly)
		{
			of.patDly2 = of.patDly;
			of.patDly  = 0;
		}

		if (of.patDly2)
		{
			// Patterndelay active
			if (--of.patDly2)
			{
				// So turn back of.patPos by 1
				if (of.patPos)
					of.patPos--;
			}
		}

		// Do we have to get a new patternpointer?
		// (when of.patPos reaches the pattern size or when
		// a patternbreak is active)
		if (((of.patPos >= of.numRow) && (of.numRow > 0)) && (!of.posJmp))
			of.posJmp = 3;

		if (of.posJmp)
		{
			of.patPos       = of.numRow ? (of.patBrk % of.numRow) : 0;
			of.pat_repCrazy = 0;
			of.sngPos      += (of.posJmp - 2);

			for (channel = 0; channel < of.numChn; channel++)
				of.control[channel].pat_repPos = -1;

			of.patBrk = of.posJmp = 0;

			// Tell APlayer we has changed position
			ChangePosition();

			// Handle the "---" (end of song) pattern since it can occur
			// *inside* the module in .IT and .S3M
			if ((of.sngPos >= of.numPos) || (of.positions[of.sngPos] == LAST_PATTERN))
			{
				if (!of.wrap)
					return;

				if (!(of.sngPos = of.repPos))
				{
					of.volume = of.initVolume > 128 ? 128 : of.initVolume;
					if (of.initSpeed != 0)
						of.sngSpd = of.initSpeed < 32 ? of.initSpeed : 32;
					else
						of.sngSpd = 6;

					of.bpm = of.initTempo < 32 ? 32 : of.initTempo;
				}

				// Tell APlayer we has restarted
				endReached = true;
			}

			if (of.sngPos < 0)
				of.sngPos = of.numPos - 1;
		}

		if (!of.patDly2)
			PT_Notes(&of);
	}

	// Fade global volume if enabled and we're playing the last pattern
	if (((of.sngPos == (of.numPos - 1)) || (of.positions[of.sngPos + 1] == LAST_PATTERN)) && (of.fadeOut))
		max_Volume = of.numRow ? ((of.numRow - of.patPos) * 128) / of.numRow : 0;
	else
		max_Volume = 128;

	PT_EffectsPass1(&of);

	if (of.flags & UF_NNA)
		PT_NNA(&of);

	PT_SetupVoices(&of);
	PT_EffectsPass2(&of);

	// Now set up the actual hardware channel playback information
	PT_UpdateVoices(&of, max_Volume);
}



/******************************************************************************/
/* PT_UpdateVoices() changes the voices, e.g. create the envelopes.           */
/*                                                                            */
/* Input:  "max_Volume" is the maximum volume.                                */
/******************************************************************************/
void MikMod::PT_UpdateVoices(MODULE *mod, int32 max_Volume)
{
	int16 envPan, envVol, envPit, channel;
	uint16 playPeriod;
	int32 vibVal, vibDpt;
	uint32 tmpVol;
	MP_VOICE *aout;
	INSTRUMENT *i;
	SAMPLE *s;

	mod->totalChn = mod->realChn = 0;

	for (channel = 0; channel < md_sngChn; channel++)
	{
		aout = &mod->voice[channel];
		i    = aout->main.i;
		s    = aout->main.s;

		if (!s || !s->length)
			continue;

		if (aout->main.period < 40)
			aout->main.period = 40;
		else
		{
			if (aout->main.period > 50000)
				aout->main.period = 50000;
		}

		if ((aout->main.kick == KICK_NOTE) || (aout->main.kick == KICK_KEYOFF))
		{
			VoicePlay(channel, s, (aout->main.start == -1) ? ((s->flags & SF_UST_LOOP) ? s->loopStart : 0) : aout->main.start);

			aout->main.fadeVol = 32768;
			aout->aSwpPos      = 0;
		}

		envVol = 256;
		envPan = PAN_CENTER;
		envPit = 32;

		if (i && ((aout->main.kick == KICK_NOTE) || (aout->main.kick == KICK_ENV)))
		{
			if (aout->main.volFlg & EF_ON)
				envVol = StartEnvelope(&aout->vEnv, aout->main.volFlg, i->volPts, i->volSusBeg, i->volSusEnd, i->volBeg, i->volEnd, i->volEnv, aout->main.keyOff, 256);
	
			if (aout->main.panFlg & EF_ON)
				envPan = StartEnvelope(&aout->pEnv, aout->main.panFlg, i->panPts, i->panSusBeg, i->panSusEnd, i->panBeg, i->panEnd, i->panEnv, aout->main.keyOff, PAN_CENTER);

			if (aout->main.pitFlg & EF_ON)
				envPit = StartEnvelope(&aout->cEnv, aout->main.pitFlg, i->pitPts, i->pitSusBeg, i->pitSusEnd, i->pitBeg, i->pitEnd, i->pitEnv, aout->main.keyOff, 32);

			if (aout->cEnv.flg & EF_ON)
				aout->masterPeriod = GetPeriod(mod->flags, (uint16)aout->main.note << 1, aout->master->speed);
		}
		else
		{
			if (aout->main.volFlg & EF_ON)
				envVol = ProcessEnvelope(aout, &aout->vEnv, 256);

			if (aout->main.panFlg & EF_ON)
				envPan = ProcessEnvelope(aout, &aout->pEnv, PAN_CENTER);

			if (aout->main.pitFlg & EF_ON)
				envPit = ProcessEnvelope(aout, &aout->cEnv, 32);
		}

		aout->main.kick = KICK_ABSENT;

		tmpVol  = aout->main.fadeVol;		// Max 32768
		tmpVol *= aout->main.chanVol;		// * max 64
		tmpVol *= aout->main.outVolume;		// * max 256
		tmpVol /= (256 * 64);				// tmpVol is max 32768 again

		aout->totalVol = tmpVol >> 2;		// Used to determine samplevolume

		tmpVol *= envVol;					// * max 256
		tmpVol *= mod->volume;				// * max 128
		tmpVol /= (128 * 256 * 128);

		// Fade out
		if (mod->sngPos >= mod->numPos)
			tmpVol = 0;
		else
			tmpVol = (tmpVol * max_Volume) / 128;

		if ((aout->masterChn != -1) && mod->control[aout->masterChn].muted)
			VoiceSetVolume(channel, 0);
		else
		{
			VoiceSetVolume(channel, tmpVol);

			if ((tmpVol) && (aout->master) && (aout->master->slave == aout))
				mod->realChn++;

			mod->totalChn++;
		}

		if (aout->main.panning == PAN_SURROUND)
			VoiceSetPanning(channel, PAN_SURROUND);
		else
		{
			if ((mod->panFlag) && (aout->pEnv.flg & EF_ON))
				VoiceSetPanning(channel, DoPan(envPan, aout->main.panning));
			else
				VoiceSetPanning(channel, aout->main.panning);
		}

		if (aout->main.period && s->vibDepth)
		{
			switch (s->vibType)
			{
				case 0:
				{
					vibVal = aVibTab[s->aVibPos & 127];
					if (aout->aVibPos & 0x80)
						vibVal = -vibVal;

					break;
				}

				case 1:
				{
					vibVal = 64;
					if (aout->aVibPos & 0x80)
						vibVal = -vibVal;

					break;
				}

				case 2:
				{
					vibVal = 63 - (((aout->aVibPos + 128) & 255) >> 1);
					break;
				}

				default:
				{
					vibVal = (((aout->aVibPos + 128) & 255) >> 1) - 64;
					break;
				}
			}
		}
		else
			vibVal = 0;

		if (s->vibFlags & AV_IT)
		{
			if ((aout->aSwpPos >> 8) < s->vibDepth)
			{
				aout->aSwpPos += s->vibSweep;
				vibDpt         = aout->aSwpPos;
			}
			else
				vibDpt = s->vibDepth << 8;

			vibVal = (vibVal * vibDpt) >> 16;

			if (aout->mFlag)
			{
				if (!(mod->flags & UF_LINEAR))
					vibVal >>= 1;

				aout->main.period -= vibVal;
			}
		}
		else
		{
			// Do XM style auto-vibrato
			if (!(aout->main.keyOff & KEY_OFF))
			{
				if (aout->aSwpPos < s->vibSweep)
				{
					vibDpt = (aout->aSwpPos * s->vibDepth) / s->vibSweep;
					aout->aSwpPos++;
				}
				else
					vibDpt = s->vibDepth;
			}
			else
			{
				// Key-off -> depth becomes 0 if final depth wasn't reached or
				// stays at final level if depth WAS reached
				if (aout->aSwpPos >= s->vibSweep)
					vibDpt = s->vibDepth;
				else
					vibDpt = 0;
			}

			vibVal             = (vibVal * vibDpt) >> 8;
			aout->main.period -= vibVal;
		}

		// Update vibrato position
		aout->aVibPos = (aout->aVibPos + s->vibRate) & 0xff;

		// Process pitch envelope
		playPeriod = aout->main.period;

		if ((aout->main.pitFlg & EF_ON) && (envPit != 32))
		{
			int32 p1;

			envPit -= 32;
			if (((aout->main.note << 1) + envPit) <= 0)
				envPit = -(aout->main.note << 1);

			p1 = GetPeriod(mod->flags, ((uint16)aout->main.note << 1) + envPit, aout->master->speed) - aout->masterPeriod;

			if (p1 > 0)
			{
				if ((uint16)(playPeriod + p1) <= playPeriod)
				{
					p1 = 0;
					aout->main.keyOff |= KEY_OFF;
				}
			}
			else
			{
				if (p1 < 0)
				{
					if ((uint16)(playPeriod + p1) >= playPeriod)
					{
						p1 = 0;
						aout->main.keyOff |= KEY_OFF;
					}
				}
			}

			playPeriod += p1;
		}

		if (!aout->main.fadeVol)	// Check for a dead note (fadeVol = 0)
		{
			VoiceStop(channel);
			mod->totalChn--;

			if ((tmpVol) && (aout->master) && (aout->master->slave == aout))
				mod->realChn--;
		}
		else
		{
			VoiceSetFrequency(channel, GetFrequency(mod->flags, playPeriod));

			// If keyFade, start subtracting FadeOutSpeed from fadeVol:
			if ((i) && (aout->main.keyOff & KEY_FADE))
			{
				if (aout->main.fadeVol >= i->volFade)
					aout->main.fadeVol -= i->volFade;
				else
					aout->main.fadeVol = 0;
			}
		}
	}

	// Do only update the tempo if it has changed
	if (bpmTempo != (mod->bpm + mod->relSpd))
	{
		bpmTempo = mod->bpm + mod->relSpd;
		if (bpmTempo < 32)
			bpmTempo = 32;
		else
		{
			if ((!(mod->flags & UF_HIGHBPM)) && (bpmTempo > 255))
				bpmTempo = 255;
		}

		SetTempo(bpmTempo);
	}
}



/******************************************************************************/
/* PT_Notes() handles new notes or instruments.                               */
/******************************************************************************/
void MikMod::PT_Notes(MODULE *mod)
{
	int16 channel;
	MP_CONTROL *a;
	uint8 c, inst;
	int32 tr, funky;	// Funky is set to indicate note or instrument change

	for (channel = 0; channel < mod->numChn; channel++)
	{
		a = &mod->control[channel];

		if (mod->sngPos >= mod->numPos)
		{
			tr          = mod->numTrk;
			mod->numRow = 0;
		}
		else
		{
			tr          = mod->patterns[(mod->positions[mod->sngPos] * mod->numChn) + channel];
			mod->numRow = mod->pattRows[mod->positions[mod->sngPos]];
		}

		a->row     = (tr < mod->numTrk) ? uniTrk.UniFindRow(mod->tracks[tr], mod->patPos) : NULL;
		a->newSamp = 0;

		if (!mod->vbTick)
			a->main.noteDelay = 0;

		if (!a->row)
			continue;

		uniTrk.UniSetRow(a->row);
		funky = 0;

		while ((c = uniTrk.UniGetByte()))
		{
			switch (c)
			{
				case UNI_NOTE:
				{
					funky        |= 1;
					a->oldNote    = a->aNote;
					a->aNote      = uniTrk.UniGetByte();
					a->main.kick  = KICK_NOTE;
					a->main.start = -1;
					a->sliding    = 0;

					// Retrig tremolo and vibrato waves?
					if (!(a->waveControl & 0x80))
						a->trmPos = 0;

					if (!(a->waveControl & 0x08))
						a->vibPos = 0;

					if (!a->panbWave)
						a->panbPos = 0;

					break;
				}

				case UNI_INSTRUMENT:
				{
					inst = uniTrk.UniGetByte();

					if (inst >= mod->numIns)
						break;				// Safety valve

					funky         |= 2;
					a->main.i      = (mod->flags & UF_INST) ? &mod->instruments[inst] : NULL;
					a->retrig      = 0;
					a->s3mTremor   = 0;
					a->ultOffset   = 0;
					a->main.sample = inst;
					break;
				}

				default:
				{
					uniTrk.UniSkipOpcode();
					break;
				}
			}
		}

		if (funky)
		{
			INSTRUMENT *i;
			SAMPLE *s;

			if ((i = a->main.i))
			{
				if (i->sampleNumber[a->aNote] >= mod->numSmp)
					continue;

				s            = &mod->samples[i->sampleNumber[a->aNote]];
				a->main.note = i->sampleNote[a->aNote];
			}
			else
			{
				a->main.note = a->aNote;
				s            = &mod->samples[a->main.sample];
			}

			if (a->main.s != s)
			{
				a->main.s  = s;
				a->newSamp = a->main.period;
			}

			// Channel or instrument determined panning?
			a->main.panning = mod->panning[channel];

			if (s->flags & SF_OWNPAN)
				a->main.panning = s->panning;
			else
			{
				if ((i) && (i->flags & IF_OWNPAN))
					a->main.panning = i->panning;
			}

			a->main.handle = s->handle;
			a->speed       = s->speed;

			if (i)
			{
				if ((mod->panFlag) && (i->flags & IF_PITCHPAN) && (a->main.panning != PAN_SURROUND))
				{
					a->main.panning += ((a->aNote - i->pitPanCenter) * i->pitPanSep) / 8;

					if (a->main.panning < PAN_LEFT)
						a->main.panning = PAN_LEFT;
					else
					{
						if (a->main.panning > PAN_RIGHT)
							a->main.panning = PAN_RIGHT;
					}
				}

				a->main.pitFlg = i->pitFlg;
				a->main.volFlg = i->volFlg;
				a->main.panFlg = i->panFlg;
				a->main.nna    = i->nnaType;
				a->dca         = i->dca;
				a->dct         = i->dct;
			}
			else
			{
				a->main.pitFlg = a->main.volFlg = a->main.panFlg = 0;
				a->main.nna    = a->dca = 0;
				a->dct         = DCT_OFF;
			}

			if (funky & 2)	// Instrument change
			{
				// IT random volume variations: 0:8 bit fixed, and one bit for sign
				a->volume = a->tmpVolume = s->volume;

				if ((s) && (i))
				{
					if (i->rVolVar)
					{
						a->volume = a->tmpVolume = s->volume + ((s->volume * ((int32)i->rVolVar * (int32)GetRandom(512))) / 25600);

						if (a->volume < 0)
							a->volume = a->tmpVolume = 0;
						else
						{
							if (a->volume > 64)
								a->volume = a->tmpVolume = 64;
						}
					}

					if ((mod->panFlag) && (a->main.panning != PAN_SURROUND))
					{
						a->main.panning += ((a->main.panning * ((int32)i->rPanVar * (int32)GetRandom(512))) / 25600);

						if (a->main.panning < PAN_LEFT)
							a->main.panning = PAN_LEFT;
						else
						{
							if (a->main.panning > PAN_RIGHT)
								a->main.panning = PAN_RIGHT;
						}
					}
				}
			}

			a->wantedPeriod = a->tmpPeriod = GetPeriod(mod->flags, (uint16)a->main.note << 1, a->speed);
			a->main.keyOff  = KEY_KICK;
		}
	}
}



/******************************************************************************/
/* PT_EffectsPass1() handles effects.                                         */
/******************************************************************************/
void MikMod::PT_EffectsPass1(MODULE *mod)
{
	int16 channel;
	MP_CONTROL *a;
	MP_VOICE *aout;
	int32 explicitSlides;

	for (channel = 0; channel < mod->numChn; channel++)
	{
		a = &mod->control[channel];

		if ((aout = a->slave))
		{
			a->main.fadeVol = aout->main.fadeVol;
			a->main.period  = aout->main.period;

			if (a->main.kick == KICK_KEYOFF)
				a->main.keyOff = aout->main.keyOff;
		}

		if (!a->row)
			continue;

		uniTrk.UniSetRow(a->row);

		a->ownPer = a->ownVol = 0;
		explicitSlides = PT_PlayEffects(mod, channel, a);

		// Continue volume slide if necessary for XM and IT
		if (mod->flags & UF_BGSLIDES)
		{
			if (!explicitSlides && a->sliding)
				DoS3MVolSlide(mod->vbTick, mod->flags, a, 0);
			else
			{
				if (a->tmpVolume)
					a->sliding = explicitSlides;
			}
		}

		if (!a->ownPer)
			a->main.period = a->tmpPeriod;

		if (!a->ownVol)
			a->volume = a->tmpVolume;

		if (a->main.s)
		{
			if (a->main.i)
				a->main.outVolume = (a->volume * a->main.s->globVol * a->main.i->globVol) >> 10;
			else
				a->main.outVolume = (a->volume * a->main.s->globVol) >> 4;

			if (a->main.outVolume > 256)
				a->main.outVolume = 256;
			else
			{
				if (a->main.outVolume < 0)
					a->main.outVolume = 0;
			}
		}
	}
}



/******************************************************************************/
/* PT_EffectsPass2() second effect pass.                                      */
/******************************************************************************/
void MikMod::PT_EffectsPass2(MODULE *mod)
{
	int16 channel;
	MP_CONTROL *a;
	uint8 c;

	for (channel = 0; channel < mod->numChn; channel++)
	{
		a = &mod->control[channel];

		if (!a->row)
			continue;

		uniTrk.UniSetRow(a->row);

		while ((c = uniTrk.UniGetByte()))
		{
			if (c == UNI_ITEFFECTS0)
			{
				c = uniTrk.UniGetByte();
				if ((c >> 4) == SS_S7EFFECTS)
					DoNNAEffects(mod, a, c & 0xf);
			}
			else
				uniTrk.UniSkipOpcode();
		}
	}
}



/******************************************************************************/
/* PT_NNA() manages the NNA.                                                  */
/******************************************************************************/
void MikMod::PT_NNA(MODULE *mod)
{
	int16 channel;
	MP_CONTROL *a;

	for (channel = 0; channel < mod->numChn; channel++)
	{
		a = &mod->control[channel];

		if (a->main.kick == KICK_NOTE)
		{
			bool kill = false;

			if (a->slave)
			{
				MP_VOICE *aout;

				aout = a->slave;
				if (aout->main.nna & NNA_MASK)
				{
					// Make sure the old MP_VOICE channel knows it has no master now!
					a->slave = NULL;

					// Assume the channel is taken by NNA
					aout->mFlag = 0;

					switch (aout->main.nna)
					{
						case NNA_CONTINUE:	// Continue note, do nothing
							break;

						case NNA_OFF:		// Note off
						{
							aout->main.keyOff |= KEY_OFF;

							if ((!(aout->main.volFlg & EF_ON)) || (aout->main.volFlg & EF_LOOP))
								aout->main.keyOff = KEY_KILL;

							break;
						}

						case NNA_FADE:
						{
							aout->main.keyOff |= KEY_FADE;
							break;
						}
					}
				}
			}

			if (a->dct != DCT_OFF)
			{
				int32 t;

				for (t = 0; t < md_sngChn; t++)
				{
					if ((!VoiceStopped(t)) && (mod->voice[t].masterChn == channel) && (a->main.sample == mod->voice[t].main.sample))
					{
						kill = false;

						switch (a->dct)
						{
							case DCT_NOTE:
							{
								if (a->main.note == mod->voice[t].main.note)
									kill = true;

								break;
							}

							case DCT_SAMPLE:
							{
								if (a->main.handle == mod->voice[t].main.handle)
									kill = true;

								break;
							}

							case DCT_INST:
							{
								kill = true;
								break;
							}
						}

						if (kill)
						{
							switch (a->dca)
							{
								case DCA_CUT:
								{
									mod->voice[t].main.fadeVol = 0;
									break;
								}

								case DCA_OFF:
								{
									mod->voice[t].main.keyOff |= KEY_OFF;
									if ((!(mod->voice[t].main.volFlg & EF_ON)) || (mod->voice[t].main.volFlg & EF_LOOP))
										mod->voice[t].main.keyOff = KEY_KILL;

									break;
								}

								case DCA_FADE:
								{
									mod->voice[t].main.keyOff |= KEY_FADE;
									break;
								}
							}
						}
					}
				}
			}
		}	// if (a->main.kick == KICK_NOTE)
	}
}



/******************************************************************************/
/* PT_SetupVoices() setup module and NNA voices.                              */
/******************************************************************************/
void MikMod::PT_SetupVoices(MODULE *mod)
{
	int16 channel;
	MP_CONTROL *a;
	MP_VOICE *aout;

	for (channel = 0; channel < mod->numChn; channel++)
	{
		a = &mod->control[channel];

		if (a->main.noteDelay)
			continue;

		if (a->main.kick == KICK_NOTE)
		{
			// If no channel was cut above, find an empty or quiet channel here
			if (mod->flags & UF_NNA)
			{
				if (!a->slave)
				{
					int32 newChn;

					if ((newChn = MP_FindEmptyChannel(mod)) != -1)
						a->slave = &mod->voice[a->slaveChn = newChn];
				}
			}
			else
				a->slave = &mod->voice[a->slaveChn = channel];

			// Assign parts of MP_VOICE only done for a KICK_NOTE
			if ((aout = a->slave))
			{
				if (aout->mFlag && aout->master)
					aout->master->slave = NULL;

				aout->master    = a;
				a->slave        = aout;
				aout->masterChn = channel;
				aout->mFlag     = 1;
			}
		}
		else
			aout = a->slave;

		if (aout)
			aout->main = a->main;

		a->main.kick = KICK_ABSENT;
	}
}



/******************************************************************************/
/* GetRandom() returns a random value between 0 and ceil - 1, ceil must be a  */
/*      power of two.                                                         */
/*                                                                            */
/* Input:  "ceil" the maximum number to get + 1.                              */
/*                                                                            */
/* Output: A random number.                                                   */
/******************************************************************************/
int32 MikMod::GetRandom(int32 ceil)
{
	return ((int32)((rand() * ceil) / (RAND_MAX + 1.0)));
}



/******************************************************************************/
/* GetPeriod() calculates the notes period and return it.                     */
/*                                                                            */
/* Input:  "flags" is the module flags that indicate which type of periods to */
/*         use.                                                               */
/*         "note" is the note to calculate the period on.                     */
/*         "speed" is the middle C speed.                                     */
/*                                                                            */
/* Output: The period.                                                        */
/******************************************************************************/
uint16 MikMod::GetPeriod(uint16 flags, uint16 note, uint32 speed)
{
	if (flags & UF_XMPERIODS)
	{
		if (flags & UF_LINEAR)
			return (GetLinearPeriod(note, speed));
		else
			return (GetLogPeriod(note, speed));
	}
	else
		return (GetOldPeriod(note, speed));
}



/******************************************************************************/
/* GetOldPeriod() calculates the notes period and return it.                  */
/*                                                                            */
/* Input:  "note" is the note to calculate the period on.                     */
/*         "speed" is the middle C speed.                                     */
/*                                                                            */
/* Output: The period.                                                        */
/******************************************************************************/
uint16 MikMod::GetOldPeriod(uint16 note, uint32 speed)
{
	uint16 n, o;

	// This happens sometimes on badly converted AMF, and old MOD
	if (!speed)
		return (4242);		// Prevent divide overflow.. (42 hehe)

	n = note % (2 * OCTAVE);
	o = note / (2 * OCTAVE);

	return ((uint16)(((8363L * (uint32)oldPeriods[n]) >> o) / speed));
}



/******************************************************************************/
/* GetLinearPeriod() calculates the notes period and return it.               */
/*                                                                            */
/* Input:  "note" is the note to calculate the period on.                     */
/*         "fine" is the middle C speed.                                      */
/*                                                                            */
/* Output: The period.                                                        */
/******************************************************************************/
uint16 MikMod::GetLinearPeriod(uint16 note, uint32 fine)
{
	uint16 t;

	t = ((20L + 2 * HIGH_OCTAVE) * OCTAVE + 2 - note) * 32L - (fine >> 1);

	return (t);
}



/******************************************************************************/
/* GetLogPeriod() calculates the notes period and return it.                  */
/*                                                                            */
/* Input:  "note" is the note to calculate the period on.                     */
/*         "fine" is the middle C speed.                                      */
/*                                                                            */
/* Output: The period.                                                        */
/******************************************************************************/
uint16 MikMod::GetLogPeriod(uint16 note, uint32 fine)
{
	uint16 n, o;
	uint16 p1, p2;
	uint32 i;

	n = note % (2 * OCTAVE);
	o = note / (2 * OCTAVE);
	i = (n << 2) + (fine >> 4);	// n * 8 + fine / 16

	p1 = logTab[i];
	p2 = logTab[i + 1];

	return (Interpolate(fine >> 4, 0, 15, p1, p2) >> o);
}



/******************************************************************************/
/* GetFrequency() XM linear period to frequency conversion.                   */
/*                                                                            */
/* Input:  "flags" is the module flags.                                       */
/*         "period" is the period to convert.                                 */
/*                                                                            */
/* Output: The frequency.                                                     */
/******************************************************************************/
uint32 MikMod::GetFrequency(uint16 flags, uint32 period)
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
/* Interpolate() interpolates?                                                */
/******************************************************************************/
int16 MikMod::Interpolate(int16 p, int16 p1, int16 p2, int16 v1, int16 v2)
{
	if ((p1 == p2) || (p == p1))
		return (v1);

	return (v1 + ((int32)((p - p1) * (v2 - v1)) / (p2 - p1)));
}



/******************************************************************************/
/* InterpolateEnv() interpolates the envelope.                                */
/******************************************************************************/
int16 MikMod::InterpolateEnv(int16 p, ENVPT *a, ENVPT *b)
{
	return (Interpolate(p, a->pos, b->pos, a->val, b->val));
}



/******************************************************************************/
/* DoPan() calculates the panning value.                                      */
/*                                                                            */
/* Input:  "envPan" is the envelope panning value.                            */
/*         "pan" is the channels panning.                                     */
/*                                                                            */
/* Output: The new panning.                                                   */
/******************************************************************************/
int16 MikMod::DoPan(int16 envPan, int16 pan)
{
	int32 newPan;

	newPan = pan + (((envPan - PAN_CENTER) * (128 - abs(pan - PAN_CENTER))) / 128);

	return ((newPan < PAN_LEFT) ? PAN_LEFT : ((newPan > PAN_RIGHT) ? PAN_RIGHT : newPan));
}



/******************************************************************************/
/* StartEnvelope() initialize and start the envelope.                         */
/******************************************************************************/
int16 MikMod::StartEnvelope(ENVPR *t, uint8 flg, uint8 pts, uint8 susBeg, uint8 susEnd, uint8 beg, uint8 end, ENVPT *p, uint8 keyOff, int16 v)
{
	t->flg    = flg;
	t->pts    = pts;
	t->susBeg = susBeg;
	t->susEnd = susEnd;
	t->beg    = beg;
	t->end    = end;
	t->env    = p;
	t->p      = 0;
	t->a      = 0;
	t->b      = ((t->flg & EF_SUSTAIN) && (!(keyOff & KEY_OFF))) ? 0 : 1;

	// TN: Check to see if an envelope is enabled, but is empty
	if (t->pts == 0)
		return (v);

	// Imago Orpheus sometimes stores an extra initial point in the envelope
	if ((t->pts >= 2) && (t->env[0].pos == t->env[1].pos))
	{
		t->a++;
		t->b++;
	}

	// Fit in the envelope, still
	if (t->a >= t->pts)
		t->a = t->pts - 1;

	if (t->b >= t->pts)
		t->b = t->pts - 1;

	return (t->env[t->a].val);
}



/******************************************************************************/
/* ProcessEnvelope() calculates the next envelope value.                      */
/*                                                                            */
/* This procedure processes all envelope types, include volume, pitch and     */
/* panning. Envelopes are defined by a set of points, each with a magnitude   */
/* [relating either to volume, panning position or pitch modifier] and a      */
/* tick position.                                                             */
/*                                                                            */
/* Envelopes work in the following manner:                                    */
/*                                                                            */
/* (a) Each tick the envelope is moved a point further in its progression.    */
/*   1. For an accurate progression, magnitudes between two envelope points   */
/*      are interpolated.                                                     */
/*                                                                            */
/* (b) When progression reaches a defined point on the envelope, values are   */
/*     shifted to interpolate between this point and the next, and checks for */
/*     loops or envelope end are done.                                        */
/*                                                                            */
/* Misc:                                                                      */
/*     Sustain loops are loops that are only active as long as the keyoff     */
/*     flag is clear. When a volume envelope terminates, so does the current  */
/*     fadeout.                                                               */
/******************************************************************************/
int16 MikMod::ProcessEnvelope(MP_VOICE *aout, ENVPR *t, int16 v)
{
	if (t->flg & EF_ON)
	{
		uint8 a, b;			// Actual points in the envelope
		uint16 p;			// The 'tick counter' - real point being played

		// TN: Check for an empty envelope
		if (t->pts == 0)
			return (v);

		a = t->a;
		b = t->b;
		p = t->p;

		// Sustain loop on one point (XM type).
		// Not processed if KEYOFF.
		// Don't move and don't interpolate when the point is reached
		if ((t->flg & EF_SUSTAIN) && (t->susBeg == t->susEnd) &&
			((!(aout->main.keyOff & KEY_OFF)) && (p == t->env[t->susBeg].pos)))
		{
			v = t->env[t->susBeg].val;
		}
		else
		{
			// All following situations will require interpolation between
			// two envelope points
			//
			// Sustain loop between two points (IT type).
			// Not processed if KEYOFF.
			//
			// If we were on a loop point, loop now
			if ((t->flg & EF_SUSTAIN) && (!(aout->main.keyOff & KEY_OFF)) && (a >= t->susEnd))
			{
				a = t->susBeg;
				b = (t->susBeg == t->susEnd) ? a : a + 1;
				p = t->env[a].pos;
				v = t->env[a].val;
			}
			else
			{
				// Regular loop
				// Be sure to correctly handle single point loops
				if ((t->flg & EF_LOOP) && (a >= t->end))
				{
					a = t->beg;
					b = (t->beg == t->end) ? a : a + 1;
					p = t->env[a].pos;
					v = t->env[a].val;
				}
				else
				{
					// Non looping situations.
					// Start to fade if the volume envelope is finished
					if (p >= t->env[t->pts - 1].pos)
					{
						v = t->env[a].val;
						if (t->flg & EF_VOLENV)
						{
							aout->main.keyOff |= KEY_FADE;

							if (!v)
								aout->main.fadeVol = 0;
						}
					}
					else
					{
						// Regular processing : compute value, progress one step
						if (a != b)
							v = InterpolateEnv(p, &t->env[a], &t->env[b]);
						else
							v = t->env[a].val;

						p++;

						// Did pointer reach point b?
						if (p >= t->env[b].pos)
							a = b++;	// Shift points a and b
					}
				}
			}

			t->a = a;
			t->b = b;
			t->p = p;
		}
	}

	return (v);
}



/******************************************************************************/
/* MP_FindEmptyChannel() returns MP_CONTROL index of free channel.            */
/*                                                                            */
/* New note action scoring system:                                            */
/* -------------------------------                                            */
/*  1) Total-volume (fadeVol, chanVol, volume) is the main scorer             */
/*  2) A looping sample is a bonus x2                                         */
/*  3) A forground channel is a bonus x4                                      */
/*  4) An active envelope with keyoff is a handicap -x2                       */
/*                                                                            */
/* Input:  "mod" is a pointer to the current module.                          */
/*                                                                            */
/* Output: The channel index to use.                                          */
/******************************************************************************/
int32 MikMod::MP_FindEmptyChannel(MODULE *mod)
{
	MP_VOICE *a;
	uint32 t, k, tVol, pp;

	for (t = 0; t < md_sngChn; t++)
	{
		if (((mod->voice[t].main.kick == KICK_ABSENT) || (mod->voice[t].main.kick == KICK_ENV)) && VoiceStopped(t))
			return (t);
	}

	tVol = 0xffffffUL;
	t    = 0;
	a    = mod->voice;

	for (k = 0; k < md_sngChn; k++, a++)
	{
		// Allow us to take over a nonexisting sample
		if (!a->main.s)
			return (k);

		if ((a->main.kick == KICK_ABSENT) || (a->main.kick == KICK_ENV))
		{
			pp = a->totalVol << ((a->main.s->flags & SF_LOOP) ? 1 : 0);
			if ((a->master) && (a == a->master->slave))
				pp <<= 2;

			if (pp < tVol)
			{
				tVol = pp;
				t    = k;
			}
		}
	}

	if (tVol > 8000 * 7)
		return (-1);

	return (t);
}



/******************************************************************************/
/* PT_PlayEffects() parse the effects.                                        */
/******************************************************************************/
int32 MikMod::PT_PlayEffects(MODULE *mod, int16 channel, MP_CONTROL *a)
{
	uint16 tick = mod->vbTick;
	uint16 flags = mod->flags;
	uint8 c;
	int32 explicitSlides = 0;
	effect_func f;

	while ((c = uniTrk.UniGetByte()))
	{
		f = effects[c];

		if (f != DoNothing)
			a->sliding = 0;

		explicitSlides |= f(this, tick, flags, a, mod, channel);
	}

	return (explicitSlides);
}



/******************************************************************************/
/* DoEEffects() parse the ProTracker Extra effects.                           */
/******************************************************************************/
void MikMod::DoEEffects(uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel, uint8 dat)
{
	uint8 nib = dat & 0xf;

	switch (dat >> 4)
	{
		// Hardware filter toggle
		case 0x0:
		{
			amigaFilter = (nib & 0x01) == 0;
			break;
		}

		// Fineslide up
		case 0x1:
		{
			if (a->main.period)
			{
				if (!tick)
					a->tmpPeriod -= (nib << 2);
			}
			break;
		}

		// Fineslide down
		case 0x2:
		{
			if (a->main.period)
			{
				if (!tick)
					a->tmpPeriod += (nib << 2);
			}
			break;
		}

		// Glissando control
		case 0x3:
		{
			a->glissando = nib;
			break;
		}

		// Set vibrato waveform
		case 0x4:
		{
			a->waveControl &= 0xf0;
			a->waveControl |= nib;
			break;
		}

		// Set finetune
		case 0x5:
		{
			if (a->main.period)
			{
				if (flags & UF_XMPERIODS)
					a->speed = nib + 128;
				else
					a->speed = fineTune[nib];

				a->tmpPeriod = GetPeriod(flags, (uint16)a->main.note << 1, a->speed);
			}
			break;
		}

		// Set patternloop
		case 0x6:
		{
			if (tick)
				break;

			if (nib)		// Set repPos or repCnt?
			{
				// Set repCnt, so check if repCnt already is set,
				// which means we are already looping
				if (a->pat_repCnt)
					a->pat_repCnt--;		// Already looping, decrease counter
				else
				{
#if 0
					// This would make walker.xm, shipped with Xsoundtracker,
					// play correctly, but it's better to remain compatible
					// with FT2
					if ((!(flags & UF_NOWRAP)) || (a->pat_repPos != POS_NONE))
#endif
						a->pat_repCnt = nib;	// Not yet looping, so set repCnt
				}

				if (a->pat_repCnt)			// Jump to repPos if repCnt > 0
				{
					if (a->pat_repPos == POS_NONE)
						a->pat_repPos = mod->patPos - 1;

					if (a->pat_repPos == -1)
					{
						mod->pat_repCrazy = 1;
						mod->patPos       = 0;
					}
					else
						mod->patPos = a->pat_repPos;
				}
				else
					a->pat_repPos = POS_NONE;
			}
			else
				a->pat_repPos = mod->patPos - 1;	// Set repPos - can be (-1)

			break;
		}

		// Set tremolo waveform
		case 0x7:
		{
			a->waveControl &= 0x0f;
			a->waveControl |= nib << 4;
			break;
		}

		// Set panning
		case 0x8:
		{
			if (mod->panFlag)
			{
				if (nib <= 8)
					nib <<= 4;
				else
					nib *= 17;

				a->main.panning = mod->panning[channel] = nib;
			}
			break;
		}

		// Retrig note
		case 0x9:
		{
			// Do not retrigger on tick 0, until we are emulating FT2 and
			// effect data is zero
			if (!tick && !((flags & UF_FT2QUIRKS) && (!nib)))
				break;

			// Only retrigger if data nibble > 0, or if tick 0 (FT2 compat)
			if (nib || !tick)
			{
				if (!a->retrig)
				{
					// When retrig counter reaches 0,
					// reset counter and restart the sample
					if (a->main.period)
						a->main.kick = KICK_NOTE;

					a->retrig = nib;
				}

				a->retrig--;	// Countdown
			}
			break;
		}

		// Fine volume slide up
		case 0xa:
		{
			if (tick)
				break;

			a->tmpVolume += nib;
			if (a->tmpVolume > 64)
				a->tmpVolume = 64;

			break;
		}

		// Fine volume slide down
		case 0xb:
		{
			if (tick)
				break;

			a->tmpVolume -= nib;
			if (a->tmpVolume < 0)
				a->tmpVolume = 0;

			break;
		}

		// Cut note
		case 0xc:
		{
			// When tick reaches the cut-note value,
			// turn the volume to zero (just like on the Amiga)
			if (tick >= nib)
				a->tmpVolume = 0;	// Just turn the volume down

			break;
		}

		// Note delay
		case 0xd:
		{
			// Delay the start of the sample until tick == nib
			if (!tick)
				a->main.noteDelay = nib;
			else
			{
				if (a->main.noteDelay)
					a->main.noteDelay--;
			}
			break;
		}

		// Pattern delay
		case 0xe:
		{
			if (!tick)
			{
				if (!mod->patDly2)
					mod->patDly = nib + 1;	// Only once, when tick = 0
			}
			break;
		}

		// Invert loop, not supported
		case 0xf:
		{
			break;
		}
	}
}



/******************************************************************************/
/* ProTracker effect helpers                                                  */
/******************************************************************************/

/******************************************************************************/
/* Arpeggio helper function.                                                  */
/******************************************************************************/
void MikMod::DoArpeggio(uint16 tick, uint16 flags, MP_CONTROL *a, uint8 style)
{
	uint8 note = a->main.note;

	if (a->arpMem)
	{
		switch (style)
		{
			// Mod style: N, N+x, N+y
			case 0:
			{
				switch (tick % 3)
				{
					// case 0: unchanged
					case 1:
					{
						note += (a->arpMem >> 4);
						break;
					}

					case 2:
					{
						note += (a->arpMem & 0xf);
						break;
					}
				}
				break;
			}

			// Okt arpeggio 3: N-x, N, N+y
			case 3:
			{
				switch (tick % 3)
				{
					case 0:
					{
						note -= (a->arpMem >> 4);
						break;
					}

					// case 1: unchanged
					case 2:
					{
						note += (a->arpMem & 0xf);
						break;
					}
				}
				break;
			}

			// Okt arpeggio 4: N, N+y, N, N-x
			case 4:
			{
				switch (tick % 4)
				{
					// case 0, case 2: unchanged
					case 1:
					{
						note += (a->arpMem & 0xf);
						break;
					}

					case 3:
					{
						note -= (a->arpMem >> 4);
						break;
					}
				}
				break;
			}

			// Okt arpeggio 5: N-x, N+y, N and nothing at tick 0
			case 5:
			{
				if (!tick)
					break;

				switch (tick % 3)
				{
					// case 0: unchanged
					case 1:
					{
						note -= (a->arpMem >> 4);
						break;
					}

					case 2:
					{
						note += (a->arpMem & 0xf);
						break;
					}
				}
				break;
			}
		}

		a->main.period = GetPeriod(flags, (uint16)note << 1, a->speed);
		a->ownPer      = 1;
	}
}



/******************************************************************************/
/* Tone slide helper function.                                                */
/******************************************************************************/
void MikMod::DoToneSlide(uint16 tick, MP_CONTROL *a)
{
	if (!a->main.fadeVol)
		a->main.kick = (a->main.kick == KICK_NOTE) ? KICK_NOTE : KICK_KEYOFF;
	else
		a->main.kick = (a->main.kick == KICK_NOTE) ? KICK_ENV : KICK_ABSENT;

	if (tick != 0)
	{
		int32 dist;

		// We have to slide a->main.period towards a->wantedPeriod, so
		// compute the difference between those two values
		dist = a->main.period - a->wantedPeriod;

		// If they are equal or if portamentoSpeed is too big...
		if ((dist == 0) || (a->portSpeed > abs(dist)))
		{
			// ...make tmpPeriod equal tperiod
			a->tmpPeriod = a->main.period = a->wantedPeriod;
		}
		else
		{
			if (dist > 0)
			{
				a->tmpPeriod   -= a->portSpeed;
				a->main.period -= a->portSpeed;	// dist > 0, slide up
			}
			else
			{
				a->tmpPeriod   += a->portSpeed;
				a->main.period += a->portSpeed;	// dist < 0, slide down
			}
		}
	}
	else
		a->tmpPeriod = a->main.period;

	a->ownPer = 1;
}



/******************************************************************************/
/* Vibrato helper function.                                                   */
/******************************************************************************/
void MikMod::DoVibrato(uint16 tick, MP_CONTROL *a)
{
	uint8 q;
	uint16 temp = 0;	// Silence warning

	if (!tick)
		return;

	q = (a->vibPos >> 2) & 0x1f;

	switch (a->waveControl & 3)
	{
		// Sine
		case 0:
		{
			temp = vibratoTable[q];
			break;
		}

		// Ramp down
		case 1:
		{
			q <<= 3;

			if (a->vibPos < 0)
				q = 255 - q;

			temp = q;
			break;
		}

		// Square wave
		case 2:
		{
			temp = 255;
			break;
		}

		// Random wave
		case 3:
		{
			temp = GetRandom(256);
			break;
		}
	}

	temp  *= a->vibDepth;
	temp >>= 7;
	temp <<= 2;

	if (a->vibPos >= 0)
		a->main.period = a->tmpPeriod + temp;
	else
		a->main.period = a->tmpPeriod - temp;

	a->ownPer = 1;

	if (tick != 0)
		a->vibPos += a->vibSpd;
}



/******************************************************************************/
/* Volume slide helper function.                                              */
/******************************************************************************/
void MikMod::DoVolSlide(MP_CONTROL *a, uint8 dat)
{
	if (dat & 0xf)
	{
		a->tmpVolume -= (dat & 0x0f);

		if (a->tmpVolume < 0)
			a->tmpVolume = 0;
	}
	else
	{
		a->tmpVolume += (dat >> 4);

		if (a->tmpVolume > 64)
			a->tmpVolume = 64;
	}
}



/******************************************************************************/
/* ScreamTracker 3 effect helpers                                             */
/******************************************************************************/

/******************************************************************************/
/* S3M volume slide helper function.                                          */
/******************************************************************************/
void MikMod::DoS3MVolSlide(uint16 tick, uint16 flags, MP_CONTROL *a, uint8 inf)
{
	uint8 lo, hi;

	if (inf)
		a->s3mVolSlide = inf;
	else
		inf = a->s3mVolSlide;

	lo = inf & 0xf;
	hi = inf >> 4;

	if (!lo)
	{
		if ((tick) || (flags & UF_S3MSLIDES))
			a->tmpVolume += hi;
	}
	else
	{
		if (!hi)
		{
			if ((tick) || (flags & UF_S3MSLIDES))
				a->tmpVolume -= lo;
		}
		else
		{
			if (lo == 0xf)
			{
				if (!tick)
					a->tmpVolume += (hi ? hi : 0xf);
			}
			else
			{
				if (hi == 0xf)
				{
					if (!tick)
						a->tmpVolume -= (lo ? lo : 0xf);
				}
				else
					return;
			}
		}
	}

	if (a->tmpVolume < 0)
		a->tmpVolume = 0;
	else
	{
		if (a->tmpVolume > 64)
			a->tmpVolume = 64;
	}
}



/******************************************************************************/
/* S3M slide down helper function.                                            */
/******************************************************************************/
void MikMod::DoS3MSlideDn(uint16 tick, MP_CONTROL *a, uint8 inf)
{
	uint8 hi, lo;

	if (inf)
		a->slideSpeed = inf;
	else
		inf = a->slideSpeed;

	hi = inf >> 4;
	lo = inf & 0xf;

	if (hi == 0xf)
	{
		if (!tick)
			a->tmpPeriod += (uint16)lo << 2;
	}
	else
	{
		if (hi == 0xe)
		{
			if (!tick)
				a->tmpPeriod += lo;
		}
		else
		{
			if (tick)
				a->tmpPeriod += (uint16)inf << 2;
		}
	}
}



/******************************************************************************/
/* S3M slide up helper function.                                              */
/******************************************************************************/
void MikMod::DoS3MSlideUp(uint16 tick, MP_CONTROL *a, uint8 inf)
{
	uint8 hi, lo;

	if (inf)
		a->slideSpeed = inf;
	else
		inf = a->slideSpeed;

	hi = inf >> 4;
	lo = inf & 0xf;

	if (hi == 0xf)
	{
		if (!tick)
			a->tmpPeriod -= (uint16)lo << 2;
	}
	else
	{
		if (hi == 0xe)
		{
			if (!tick)
				a->tmpPeriod -= lo;
		}
		else
		{
			if (tick)
				a->tmpPeriod -= (uint16)inf << 2;
		}
	}
}



/******************************************************************************/
/* ImpulseTracker effect helpers                                              */
/******************************************************************************/

/******************************************************************************/
/* IT tone slide helper function.                                             */
/******************************************************************************/
void MikMod::DoITToneSlide(uint16 tick, MP_CONTROL *a, uint8 dat)
{
	if (dat)
		a->portSpeed = dat;

	// If we don't come from another note, ignore the slide and play the note
	// as is
	if (!a->oldNote || !a->main.period)
		return;

	if ((!tick) && (a->newSamp))
	{
		a->main.kick  = KICK_NOTE;
		a->main.start = -1;
	}
	else
		a->main.kick = (a->main.kick == KICK_NOTE) ? KICK_ENV : KICK_ABSENT;

	if (tick)
	{
		int32 dist;

		// We have to slide a->main.period towards a->wantedPeriod,
		// compute the difference between those two values
		dist = a->main.period - a->wantedPeriod;

		// If they are equal or if portamentospeed is too big...
		if ((!dist) || ((a->portSpeed << 2) > abs(dist)))
		{
			// ...make tmpPeriod equal tPeriod
			a->tmpPeriod = a->main.period = a->wantedPeriod;
		}
		else
		{
			if (dist > 0)
			{
				a->tmpPeriod   -= a->portSpeed << 2;
				a->main.period -= a->portSpeed << 2;	// Dist > 0, slide up
			}
			else
			{
				a->tmpPeriod   += a->portSpeed << 2;
				a->main.period += a->portSpeed << 2;	// Dist < 0, slide down
			}
		}
	}
	else
		a->tmpPeriod = a->main.period;

	a->ownPer = 1;
}



/******************************************************************************/
/* IT vibrato helper function.                                                */
/******************************************************************************/
void MikMod::DoITVibrato(uint16 tick, MP_CONTROL *a, uint8 dat)
{
	uint8 q;
	uint16 temp = 0;

	if (!tick)
	{
		if (dat & 0x0f)
			a->vibDepth = dat & 0xf;

		if (dat & 0xf0)
			a->vibSpd = (dat & 0xf0) >> 2;
	}

	if (!a->main.period)
		return;

	q = (a->vibPos >> 2) & 0x1f;

	switch (a->waveControl & 3)
	{
		// Sine
		case 0:
		{
			temp = vibratoTable[q];
			break;
		}

		// Square wave
		case 1:
		{
			temp = 255;
			break;
		}

		// Ramp down
		case 2:
		{
			q <<= 3;

			if (a->vibPos < 0)
				q = 255 - q;

			temp = q;
			break;
		}

		// Random
		case 3:
		{
			temp = GetRandom(256);
			break;
		}
	}

	temp  *= a->vibDepth;
	temp >>= 8;
	temp <<= 2;

	if (a->vibPos >= 0)
		a->main.period = a->tmpPeriod + temp;
	else
		a->main.period = a->tmpPeriod - temp;

	a->ownPer = 1;

	a->vibPos += a->vibSpd;
}



/******************************************************************************/
/* IT Effect S7: NNA Effects                                                  */
/******************************************************************************/
void MikMod::DoNNAEffects(MODULE *mod, MP_CONTROL *a, uint8 dat)
{
	int32 t;
	MP_VOICE *aout;

	dat &= 0xf;
	aout = (a->slave) ? a->slave : NULL;

	switch (dat)
	{
		// Past note cut
		case 0x0:
		{
			for (t = 0; t < md_sngChn; t++)
			{
				if (mod->voice[t].master == a)
					mod->voice[t].main.fadeVol = 0;
			}
			break;
		}

		// Past note off
		case 0x1:
		{
			for (t = 0; t < md_sngChn; t++)
			{
				if (mod->voice[t].master == a)
				{
					mod->voice[t].main.keyOff |= KEY_OFF;
					if ((!(mod->voice[t].vEnv.flg & EF_ON)) || (mod->voice[t].vEnv.flg & EF_LOOP))
						mod->voice[t].main.keyOff = KEY_KILL;
				}
			}
			break;
		}

		// Past note fade
		case 0x2:
		{
			for (t = 0; t < md_sngChn; t++)
			{
				if (mod->voice[t].master == a)
					mod->voice[t].main.keyOff |= KEY_FADE;
			}
			break;
		}

		// Set NNA note cut
		case 0x3:
		{
			a->main.nna = (a->main.nna & ~NNA_MASK) | NNA_CUT;
			break;
		}

		// Set NNA note continue
		case 0x4:
		{
			a->main.nna = (a->main.nna & ~NNA_MASK) | NNA_CONTINUE;
			break;
		}

		// Set NNA note off
		case 0x5:
		{
			a->main.nna = (a->main.nna & ~NNA_MASK) | NNA_OFF;
			break;
		}

		// Set NNA note fade
		case 0x6:
		{
			a->main.nna = (a->main.nna & ~NNA_MASK) | NNA_FADE;
			break;
		}

		// Disable volume envelope
		case 0x7:
		{
			if (aout)
				aout->main.volFlg &= ~EF_ON;

			break;
		}

		// Enable volume envelope
		case 0x8:
		{
			if (aout)
				aout->main.volFlg |= EF_ON;

			break;
		}

		// Disable panning envelope
		case 0x9:
		{
			if (aout)
				aout->main.panFlg &= ~EF_ON;

			break;
		}

		// Enable panning envelope
		case 0xa:
		{
			if (aout)
				aout->main.panFlg |= EF_ON;

			break;
		}

		// Disable pitch envelope
		case 0xb:
		{
			if (aout)
				aout->main.pitFlg &= ~EF_ON;

			break;
		}

		// Enable pitch envelope
		case 0xc:
		{
			if (aout)
				aout->main.pitFlg |= EF_ON;

			break;
		}
	}
}



/******************************************************************************/
/* Special Specific Effects                                                   */
/******************************************************************************/

/******************************************************************************/
/* DoNothing effect.                                                          */
/******************************************************************************/
int32 MikMod::DoNothing(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	obj->uniTrk.UniSkipOpcode();

	return (0);
}



/******************************************************************************/
/* KeyOff effect.                                                             */
/******************************************************************************/
int32 MikMod::DoKeyOff(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	a->main.keyOff |= KEY_OFF;

	if ((!(a->main.volFlg & EF_ON)) || (a->main.volFlg & EF_LOOP))
		a->main.keyOff = KEY_KILL;

	return (0);
}



/******************************************************************************/
/* KeyFade effect.                                                            */
/******************************************************************************/
int32 MikMod::DoKeyFade(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if ((tick >= dat) || (tick == mod->sngSpd - 1))
	{
		a->main.keyOff = KEY_KILL;
		if (!(a->main.volFlg & EF_ON))
			a->main.fadeVol = 0;
	}

	return (0);
}



/******************************************************************************/
/* IT Volume/Panning column effects                                           */
/*                                                                            */
/* Impulse Tracker volume/pan column effects. All volume/pan column effects   */
/* share the same memory space.                                               */
/******************************************************************************/
int32 MikMod::DoVolEffects(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 c, inf;

	c   = obj->uniTrk.UniGetByte();
	inf = obj->uniTrk.UniGetByte();

	if ((!c) && (!inf))
	{
		c   = a->volEffect;
		inf = a->volData;
	}
	else
	{
		a->volEffect = c;
		a->volData   = inf;
	}

	if (c)
	{
		switch (c)
		{
			// Volume
			case VOL_VOLUME:
			{
				if (tick)
					break;

				if (inf > 64)
					inf = 64;

				a->tmpVolume = inf;
				break;
			}

			// Panning
			case VOL_PANNING:
			{
				if (mod->panFlag)
					a->main.panning = inf;

				break;
			}

			// Volume slide
			case VOL_VOLSLIDE:
			{
				obj->DoS3MVolSlide(tick, flags, a, inf);
				break;
			}

			// Pitch slide down
			case VOL_PITCHSLIDEDN:
			{
				if (a->main.period)
					obj->DoS3MSlideDn(tick, a, inf);

				break;
			}

			// Pitch slide up
			case VOL_PITCHSLIDEUP:
			{
				if (a->main.period)
					obj->DoS3MSlideUp(tick, a, inf);

				break;
			}

			// Tone portamento
			case VOL_PORTAMENTO:
			{
				obj->DoITToneSlide(tick, a, inf);
				break;
			}

			// Vibrato
			case VOL_VIBRATO:
			{
				obj->DoITVibrato(tick, a, inf);
				break;
			}
		}
	}

	return (0);
}



/******************************************************************************/
/* ProTracker Specific Effects                                                */
/******************************************************************************/

/******************************************************************************/
/* ProTracker effect 0: Arpeggio or normal note.                              */
/******************************************************************************/
int32 MikMod::DoPTEffect0(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (!dat && (flags & UF_ARPMEM))
			dat = a->arpMem;
		else
			a->arpMem = dat;
	}

	if (a->main.period)
		obj->DoArpeggio(tick, flags, a, 0);

	return (0);
}



/******************************************************************************/
/* ProTracker effect 1: Portamento up.                                        */
/******************************************************************************/
int32 MikMod::DoPTEffect1(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick && dat)
		a->slideSpeed = (uint16)dat << 2;

	if (a->main.period)
	{
		if (tick)
			a->tmpPeriod -= a->slideSpeed;
	}

	return (0);
}



/******************************************************************************/
/* ProTracker effect 2: Portamento down.                                      */
/******************************************************************************/
int32 MikMod::DoPTEffect2(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick && dat)
		a->slideSpeed = (uint16)dat << 2;

	if (a->main.period)
	{
		if (tick)
			a->tmpPeriod += a->slideSpeed;
	}

	return (0);
}



/******************************************************************************/
/* ProTracker effect 3: Toneportamento.                                       */
/******************************************************************************/
int32 MikMod::DoPTEffect3(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick && dat)
		a->portSpeed = (uint16)dat << 2;

	if (a->main.period)
		obj->DoToneSlide(tick, a);

	return (0);
}



/******************************************************************************/
/* ProTracker effect 4: Vibrato.                                              */
/******************************************************************************/
int32 MikMod::DoPTEffect4(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat & 0x0f)
			a->vibDepth = dat & 0x0f;

		if (dat & 0xf0)
			a->vibSpd = (dat & 0xf0) >> 2;
	}

	if (a->main.period)
		obj->DoVibrato(tick, a);

	return (0);
}



/******************************************************************************/
/* ProTracker effect 5: Tone + volume slide.                                  */
/******************************************************************************/
int32 MikMod::DoPTEffect5(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (a->main.period)
		obj->DoToneSlide(tick, a);

	if (tick)
		obj->DoVolSlide(a, dat);

	return (0);
}



/******************************************************************************/
/* ProTracker effect 6: Vibrato + volume slide.                               */
/******************************************************************************/
int32 MikMod::DoPTEffect6(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	if (a->main.period)
		obj->DoVibrato(tick, a);

	DoPTEffectA(obj, tick, flags, a, mod, channel);

	return (0);
}



/******************************************************************************/
/* ProTracker effect 7: Tremolo.                                              */
/******************************************************************************/
int32 MikMod::DoPTEffect7(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;
	uint8 q;
	uint16 temp = 0;	// Silence warning

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat & 0x0f)
			a->trmDepth = dat & 0x0f;

		if (dat & 0xf0)
			a->trmSpd = (dat & 0xf0) >> 2;
	}

	if (a->main.period)
	{
		q = (a->trmPos >> 2) & 0x1f;

		switch ((a->waveControl >> 4) & 3)
		{
			// Sine
			case 0:
			{
				temp = vibratoTable[q];
				break;
			}

			// Ramp down
			case 1:
			{
				q <<= 3;

				if (a->trmPos < 0)
					q = 255 - q;

				temp = q;
				break;
			}

			// Square wave
			case 2:
			{
				temp = 255;
				break;
			}

			// Random wave
			case 3:
			{
				temp = obj->GetRandom(256);
				break;
			}
		}

		temp  *= a->trmDepth;
		temp >>= 6;

		if (a->trmPos >= 0)
		{
			a->volume = a->tmpVolume + temp;

			if (a->volume > 64)
				a->volume = 64;
		}
		else
		{
			a->volume = a->tmpVolume - temp;

			if (a->volume < 0)
				a->volume = 0;
		}

		a->ownVol = 1;

		if (tick)
			a->trmPos += a->trmSpd;
	}

	return (0);
}



/******************************************************************************/
/* ProTracker effect 8: Panning.                                              */
/******************************************************************************/
int32 MikMod::DoPTEffect8(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (mod->panFlag)
		a->main.panning = mod->panning[channel] = dat;

	return (0);
}



/******************************************************************************/
/* ProTracker effect 9: Sample offset.                                        */
/******************************************************************************/
int32 MikMod::DoPTEffect9(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat)
			a->sOffset = (uint16)dat << 8;

		a->main.start = a->hiOffset | a->sOffset;

		if ((a->main.s) && ((uint32)a->main.start > a->main.s->length))
			a->main.start = a->main.s->flags & (SF_LOOP | SF_BIDI) ? a->main.s->loopStart : a->main.s->length;
	}

	return (0);
}



/******************************************************************************/
/* ProTracker effect A: Volume slide.                                         */
/******************************************************************************/
int32 MikMod::DoPTEffectA(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (tick)
		obj->DoVolSlide(a, dat);

	return (0);
}



/******************************************************************************/
/* ProTracker effect B: Position jump.                                        */
/******************************************************************************/
int32 MikMod::DoPTEffectB(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (tick || mod->patDly2)
		return (0);

	// Vincent Voois uses a nasty trick in "Universal Bolero"
	if ((dat == mod->sngPos) && (mod->patBrk == mod->patPos))
		return (0);

	if ((!mod->loop) && (!mod->patBrk) && ((dat < mod->sngPos) ||
		((mod->sngPos == (mod->numPos - 1)) && (!mod->patBrk)) ||
		((dat == mod->sngPos) && (flags & UF_NOWRAP))))
	{
		// If we don't loop, better not to skip the end of the
		// pattern, after all... so:
		mod->patBrk = 0;
		mod->posJmp = 3;
	}
	else
	{
		// If we were fading, adjust...
		if (mod->sngPos == (mod->numPos - 1))
			mod->volume = mod->initVolume > 128 ? 128 : mod->initVolume;

		if (dat <= mod->sngPos)
		{
			// Tell APlayer the module has ended
			obj->endReached = true;
		}
		else
		{
			// If more than one Bxx effect on the same line,
			// cancel the "module end"
			obj->endReached = false;
		}

		mod->sngPos = dat;
		mod->posJmp = 2;
		mod->patPos = 0;
	}

	return (0);
}



/******************************************************************************/
/* ProTracker effect C: Volume change.                                        */
/******************************************************************************/
int32 MikMod::DoPTEffectC(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (tick)
		return (0);

	if (dat == (uint8)-1)
		a->aNote = dat = 0;		// Note cut
	else
	{
		if (dat > 64)
			dat = 64;
	}

	a->tmpVolume = dat;

	return (0);
}



/******************************************************************************/
/* ProTracker effect D: Pattern break.                                        */
/******************************************************************************/
int32 MikMod::DoPTEffectD(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if ((tick) || (mod->patDly2))
		return (0);

	if ((mod->positions[mod->sngPos] != LAST_PATTERN) && (dat > mod->pattRows[mod->positions[mod->sngPos]]))
		dat = mod->pattRows[mod->positions[mod->sngPos]];

	mod->patBrk = dat;

	if (!mod->posJmp)
	{
		// Don't ask me to explain this code - it makes
		// backwards.s3m and children.xm (heretic's version) play
		// correctly, among others. Take that for granted, or write
		// the page of comments yourself... you might need some
		// aspirin - Miod
		if ((mod->sngPos == mod->numPos - 1) && (dat) && ((mod->loop) || (mod->positions[mod->sngPos] == (mod->numPat - 1) && !(flags & UF_NOWRAP))))
		{
//			mod->sngPos = 0;		// Uncommented by Thomas Neumann
			mod->posJmp = 2;
			obj->endReached = true;
		}
		else
			mod->posJmp = 3;
	}
	else
	{
		if ((mod->patBrk != 0) && (mod->posJmp == 2))
			obj->endReached = false;	// This is done to make Enantiodromia.xm to play at all!
	}

	return (0);
}



/******************************************************************************/
/* ProTracker effect E: Extra effects.                                        */
/******************************************************************************/
int32 MikMod::DoPTEffectE(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	obj->DoEEffects(tick, flags, a, mod, channel, obj->uniTrk.UniGetByte());

	return (0);
}



/******************************************************************************/
/* ProTracker effect F: Set speed.                                            */
/******************************************************************************/
int32 MikMod::DoPTEffectF(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (tick || mod->patDly2)
		return (0);

	if (mod->extSpd && (dat >= mod->bpmLimit))
		mod->bpm = dat;
	else
	{
		if (dat)
		{
			mod->sngSpd = (dat >= mod->bpmLimit) ? mod->bpmLimit - 1 : dat;
			mod->vbTick = 0;
		}
	}

	return (0);
}



/******************************************************************************/
/* ScreamTracker 3 Specific Effects                                           */
/******************************************************************************/

/******************************************************************************/
/* S3M Effect A: Set speed                                                    */
/******************************************************************************/
int32 MikMod::DoS3MEffectA(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 speed;

	speed = obj->uniTrk.UniGetByte();

	if (tick || mod->patDly2)
		return (0);

	if (speed > 128)
		speed -= 128;

	if (speed)
	{
		mod->sngSpd = speed;
		mod->vbTick = 0;
	}

	return (0);
}



/******************************************************************************/
/* S3M Effect D: Volume slide / Fine Volume slide                             */
/******************************************************************************/
int32 MikMod::DoS3MEffectD(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	obj->DoS3MVolSlide(tick, flags, a, obj->uniTrk.UniGetByte());

	return (1);
}



/******************************************************************************/
/* S3M Effect E: Slide / Fine Slide / Extra Fine Slide Down                   */
/******************************************************************************/
int32 MikMod::DoS3MEffectE(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (a->main.period)
		obj->DoS3MSlideDn(tick, a, dat);

	return (0);
}



/******************************************************************************/
/* S3M Effect F: Slide / Fine Slide / Extra Fine Slide Up                     */
/******************************************************************************/
int32 MikMod::DoS3MEffectF(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (a->main.period)
		obj->DoS3MSlideUp(tick, a, dat);

	return (0);
}



/******************************************************************************/
/* S3M Effect I: Tremor                                                       */
/******************************************************************************/
int32 MikMod::DoS3MEffectI(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf, on, off;

	inf = obj->uniTrk.UniGetByte();

	if (inf)
		a->s3mTrOnOf = inf;
	else
	{
		inf = a->s3mTrOnOf;
		if (!inf)
			return (0);
	}

	if (!tick)
		return (0);

	on  = (inf >> 4) + 1;
	off = (inf & 0xf) + 1;

	a->s3mTremor %= (on + off);
	a->volume = (a->s3mTremor < on) ? a->tmpVolume : 0;
	a->ownVol = 1;
	a->s3mTremor++;

	return (0);
}



/******************************************************************************/
/* S3M Effect Q: Retrig + VolumeSlide                                         */
/******************************************************************************/
int32 MikMod::DoS3MEffectQ(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf;

	inf = obj->uniTrk.UniGetByte();

	if (a->main.period)
	{
		if (inf)
		{
			a->s3mRtgSlide = inf >> 4;
			a->s3mRtgSpeed = inf & 0xf;
		}

		// Only retrigger if low nibble > 0
		if (a->s3mRtgSpeed > 0)
		{
			if (!a->retrig)
			{
				// When retrig counter reaches 0,
				// reset counter and restart the sample
				if (a->main.kick != KICK_NOTE)
					a->main.kick = KICK_KEYOFF;

				a->retrig = a->s3mRtgSpeed;

				if ((tick) || (flags & UF_S3MSLIDES))
				{
					switch (a->s3mRtgSlide)
					{
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						{
							a->tmpVolume -= (1 << (a->s3mRtgSlide - 1));
							break;
						}

						case 6:
						{
							a->tmpVolume = (2 * a->tmpVolume) / 3;
							break;
						}

						case 7:
						{
							a->tmpVolume >>= 1;
							break;
						}

						case 9:
						case 0xa:
						case 0xb:
						case 0xc:
						case 0xd:
						{
							a->tmpVolume += (1 << (a->s3mRtgSlide - 9));
							break;
						}

						case 0xe:
						{
							a->tmpVolume = (3 * a->tmpVolume) >> 1;
							break;
						}

						case 0xf:
						{
							a->tmpVolume = a->tmpVolume << 1;
							break;
						}
					}

					if (a->tmpVolume < 0)
						a->tmpVolume = 0;
					else
					{
						if (a->tmpVolume > 64)
							a->tmpVolume = 64;
					}
				}
			}

			a->retrig--;	// Countdown
		}
	}

	return (0);
}



/******************************************************************************/
/* S3M Effect R: Tremolo                                                      */
/******************************************************************************/
int32 MikMod::DoS3MEffectR(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat, q;
	uint16 temp = 0;	// Silence warning

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat & 0x0f)
			a->trmDepth = dat & 0xf;

		if (dat & 0xf0)
			a->trmSpd = (dat & 0xf0) >> 2;
	}

	q = (a->trmPos >> 2) & 0x1f;

	switch ((a->waveControl >> 4) & 3)
	{
		// Sine
		case 0:
		{
			temp = vibratoTable[q];
			break;
		}

		// Ramp down
		case 1:
		{
			q <<= 3;

			if (a->trmPos < 0)
				q = 255 - q;

			temp = q;
			break;
		}

		// Square wave
		case 2:
		{
			temp = 255;
			break;
		}

		// Random
		case 3:
		{
			temp = obj->GetRandom(256);
			break;
		}
	}

	temp  *= a->trmDepth;
	temp >>= 7;

	if (a->trmPos >= 0)
	{
		a->volume = a->tmpVolume + temp;
		if (a->volume > 64)
			a->volume = 64;
	}
	else
	{
		a->volume = a->tmpVolume - temp;
		if (a->volume < 0)
			a->volume = 0;
	}

	a->ownVol = 1;

	if (tick)
		a->trmPos += a->trmSpd;

	return (0);
}



/******************************************************************************/
/* S3M Effect T: Tempo                                                        */
/******************************************************************************/
int32 MikMod::DoS3MEffectT(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 tempo;

	tempo = obj->uniTrk.UniGetByte();

	if (tick || mod->patDly2)
		return (0);

	mod->bpm = (tempo < 32) ? 32 : tempo;

	return (0);
}



/******************************************************************************/
/* S3M Effect U: Fine vibrato                                                 */
/******************************************************************************/
int32 MikMod::DoS3MEffectU(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat, q;
	uint16 temp = 0;	// Silence warning

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat & 0x0f)
			a->vibDepth = dat & 0xf;

		if (dat & 0xf0)
			a->vibSpd = (dat & 0xf0) >> 2;
	}
	else
	{
		if (a->main.period)
		{
			q = (a->vibPos >> 2) & 0x1f;

			switch (a->waveControl & 3)
			{
				// Sine
				case 0:
				{
					temp = vibratoTable[q];
					break;
				}

				// Ramp down
				case 1:
				{
					q <<= 3;

					if (a->vibPos < 0)
						q = 255 - q;

					temp = q;
					break;
				}

				// Square wave
				case 2:
				{
					temp = 255;
					break;
				}

				// Random
				case 3:
				{
					temp = obj->GetRandom(256);
					break;
				}
			}

			temp  *= a->vibDepth;
			temp >>= 8;

			if (a->vibPos >= 0)
				a->main.period = a->tmpPeriod + temp;
			else
				a->main.period = a->tmpPeriod - temp;

			a->ownVol = 1;

			a->vibPos += a->vibSpd;
		}
	}

	return (0);
}



/******************************************************************************/
/* FastTracker II Specific Effects                                            */
/******************************************************************************/

/******************************************************************************/
/* XM Effect 6: Vibrato + volume slide.                                       */
/******************************************************************************/
int32 MikMod::DoXMEffect6(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	if (a->main.period)
		obj->DoVibrato(tick, a);

	return (DoXMEffectA(obj, tick, flags, a, mod, channel));
}



/******************************************************************************/
/* XM Effect A: Volume Slide                                                  */
/******************************************************************************/
int32 MikMod::DoXMEffectA(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf, lo, hi;

	inf = obj->uniTrk.UniGetByte();

	if (inf)
		a->s3mVolSlide = inf;
	else
		inf = a->s3mVolSlide;

	if (tick)
	{
		lo = inf & 0xf;
		hi = inf >> 4;

		if (!hi)
		{
			a->tmpVolume -= lo;

			if (a->tmpVolume < 0)
				a->tmpVolume = 0;
		}
		else
		{
			a->tmpVolume += hi;

			if (a->tmpVolume > 64)
				a->tmpVolume = 64;
		}
	}

	return (0);
}



/******************************************************************************/
/* XM Effect E1: Fine portamento slide up                                     */
/******************************************************************************/
int32 MikMod::DoXMEffectE1(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat)
			a->fPortUpSpd = dat;

		if (a->main.period)
			a->tmpPeriod -= (a->fPortUpSpd << 2);
	}

	return (0);
}



/******************************************************************************/
/* XM Effect E2: Fine portamento slide down                                   */
/******************************************************************************/
int32 MikMod::DoXMEffectE2(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat)
			a->fPortDnSpd = dat;

		if (a->main.period)
			a->tmpPeriod += (a->fPortDnSpd << 2);
	}

	return (0);
}



/******************************************************************************/
/* XM Effect EA: Fine volume slide up                                         */
/******************************************************************************/
int32 MikMod::DoXMEffectEA(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat)
			a->fSlideUpSpd = dat;
	}

	a->tmpVolume += a->fSlideUpSpd;

	if (a->tmpVolume > 64)
		a->tmpVolume = 64;

	return (0);
}



/******************************************************************************/
/* XM Effect EB: Fine volume slide down                                       */
/******************************************************************************/
int32 MikMod::DoXMEffectEB(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat)
			a->fSlideDnSpd = dat;
	}

	a->tmpVolume -= a->fSlideDnSpd;

	if (a->tmpVolume < 0)
		a->tmpVolume = 0;

	return (0);
}



/******************************************************************************/
/* XM Effect G: Set global volume                                             */
/******************************************************************************/
int32 MikMod::DoXMEffectG(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	mod->volume = obj->uniTrk.UniGetByte() << 1;

	if (mod->volume > 128)
		mod->volume = 128;

	return (0);
}



/******************************************************************************/
/* XM Effect H: Global Slide                                                  */
/******************************************************************************/
int32 MikMod::DoXMEffectH(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf;

	inf = obj->uniTrk.UniGetByte();

	if (tick)
	{
		if (inf)
			mod->globalSlide = inf;
		else
			inf = mod->globalSlide;

		if (inf & 0xf0)
			inf &= 0xf0;

		mod->volume = mod->volume + ((inf >> 4) - (inf & 0xf)) * 2;

		if (mod->volume < 0)
			mod->volume = 0;
		else
		{
			if (mod->volume > 128)
				mod->volume = 128;
		}
	}

	return (0);
}



/******************************************************************************/
/* XM Effect L: Set envelope position                                         */
/******************************************************************************/
int32 MikMod::DoXMEffectL(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if ((!tick) && (a->main.i))
	{
		uint16 points;
		INSTRUMENT *i = a->main.i;
		MP_VOICE *aout;

		if ((aout = a->slave))
		{
			if (aout->vEnv.env)
			{
				points = i->volEnv[i->volPts - 1].pos;
				aout->vEnv.p = aout->vEnv.env[(dat > points) ? points : dat].pos;
			}

			if (aout->pEnv.env)
			{
				points = i->panEnv[i->panPts - 1].pos;
				aout->pEnv.p = aout->pEnv.env[(dat > points) ? points : dat].pos;
			}
		}
	}

	return (0);
}



/******************************************************************************/
/* XM Effect P: Panning Slide                                                 */
/******************************************************************************/
int32 MikMod::DoXMEffectP(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf, lo, hi;
	int16 pan;

	inf = obj->uniTrk.UniGetByte();

	if (!mod->panFlag)
		return (0);

	if (inf)
		a->pansSpd = inf;
	else
		inf = a->pansSpd;

	if (tick)
	{
		lo = inf & 0xf;
		hi = inf >> 4;

		// Slide right has absolute priority
		if (hi)
			lo = 0;

		pan             = ((a->main.panning == PAN_SURROUND) ? PAN_CENTER : a->main.panning) + hi - lo;
		a->main.panning = (pan < PAN_LEFT) ? PAN_LEFT : (pan > PAN_RIGHT ? PAN_RIGHT : pan);
	}

	return (0);
}



/******************************************************************************/
/* XM Effect X1: Extra fine portamento up                                     */
/******************************************************************************/
int32 MikMod::DoXMEffectX1(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (dat)
		a->ffPortUpSpd = dat;
	else
		dat = a->ffPortUpSpd;

	if (a->main.period)
	{
		if (!tick)
		{
			a->main.period -= dat;
			a->tmpPeriod   -= dat;
			a->ownPer       = 1;
		}
	}

	return (0);
}



/******************************************************************************/
/* XM Effect X2: Extra fine portamento down                                   */
/******************************************************************************/
int32 MikMod::DoXMEffectX2(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat;

	dat = obj->uniTrk.UniGetByte();

	if (dat)
		a->ffPortDnSpd = dat;
	else
		dat = a->ffPortDnSpd;

	if (a->main.period)
	{
		if (!tick)
		{
			a->main.period += dat;
			a->tmpPeriod   += dat;
			a->ownPer       = 1;
		}
	}

	return (0);
}



/******************************************************************************/
/* ImpulseTracker Specific Effects                                            */
/******************************************************************************/

/******************************************************************************/
/* IT Effect G: Tone portamento                                               */
/******************************************************************************/
int32 MikMod::DoITEffectG(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	obj->DoITToneSlide(tick, a, obj->uniTrk.UniGetByte());

	return (0);
}



/******************************************************************************/
/* IT Effect H: Vibrato                                                       */
/******************************************************************************/
int32 MikMod::DoITEffectH(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	obj->DoITVibrato(tick, a, obj->uniTrk.UniGetByte());

	return (0);
}



/******************************************************************************/
/* IT Effect I: Tremor                                                        */
/******************************************************************************/
int32 MikMod::DoITEffectI(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf, on, off;

	inf = obj->uniTrk.UniGetByte();

	if (inf)
		a->s3mTrOnOf = inf;
	else
	{
		inf = a->s3mTrOnOf;
		if (!inf)
			return (0);
	}

	on  = (inf >> 4);
	off = (inf & 0xf);

	a->s3mTremor %= (on + off);
	a->volume     = (a->s3mTremor < on) ? a->tmpVolume : 0;
	a->ownVol     = 1;
	a->s3mTremor++;

	return (0);
}



/******************************************************************************/
/* IT Effect M: Set channel volume                                            */
/******************************************************************************/
int32 MikMod::DoITEffectM(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	a->main.chanVol = obj->uniTrk.UniGetByte();

	if (a->main.chanVol > 64)
		a->main.chanVol = 64;
	else
	{
		if (a->main.chanVol < 0)
			a->main.chanVol = 0;
	}

	return (0);
}



/******************************************************************************/
/* IT Effect N: Slide / Fine slide channel volume                             */
/******************************************************************************/
int32 MikMod::DoITEffectN(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf, lo, hi;

	inf = obj->uniTrk.UniGetByte();

	if (inf)
		a->chanVolSlide = inf;
	else
		inf = a->chanVolSlide;

	lo = inf & 0xf;
	hi = inf >> 4;

	if (!hi)
		a->main.chanVol -= lo;
	else
	{
		if (!lo)
			a->main.chanVol += hi;
		else
		{
			if (hi == 0xf)
			{
				if (!tick)
					a->main.chanVol -= lo;
			}
			else
			{
				if (lo == 0xf)
				{
					if (!tick)
						a->main.chanVol += hi;
				}
			}
		}
	}

	if (a->main.chanVol < 0)
		a->main.chanVol = 0;
	else
	{
		if (a->main.chanVol > 64)
			a->main.chanVol = 64;
	}

	return (0);
}



/******************************************************************************/
/* IT Effect P: Slide / Fine slide channel panning                            */
/******************************************************************************/
int32 MikMod::DoITEffectP(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf, lo, hi;
	int16 pan;

	inf = obj->uniTrk.UniGetByte();

	if (inf)
		a->pansSpd = inf;
	else
		inf = a->pansSpd;

	if (!mod->panFlag)
		return (0);

	lo = inf & 0xf;
	hi = inf >> 4;

	pan = (a->main.panning == PAN_SURROUND) ? PAN_CENTER : a->main.panning;

	if (!hi)
		pan += lo << 2;
	else
	{
		if (!lo)
			pan -= hi << 2;
		else
		{
			if (hi == 0xf)
			{
				if (!tick)
					pan += lo << 2;
			}
			else
			{
				if (lo == 0xf)
				{
					if (!tick)
						pan -= hi << 2;
				}
			}
		}
	}

	a->main.panning = (pan < PAN_LEFT) ? PAN_LEFT : (pan > PAN_RIGHT ? PAN_RIGHT : pan);

	return (0);
}



/******************************************************************************/
/* IT Effect S0: Special                                                      */
/*                                                                            */
/* Impulse/Scream Tracker Sxx effects. All Sxx effects share the same memory  */
/* space.                                                                     */
/******************************************************************************/
int32 MikMod::DoITEffectS0(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat, inf, c;

	dat = obj->uniTrk.UniGetByte();

	inf = dat & 0xf;
	c   = dat >> 4;

	if (!dat)
	{
		c   = a->ssEffect;
		inf = a->ssData;
	}
	else
	{
		a->ssEffect = c;
		a->ssData   = inf;
	}

	switch (c)
	{
		// S1x: Set glissando voice
		case SS_GLISSANDO:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0x30 | inf);
			break;
		}

		// S2x: Set finetune
		case SS_FINETUNE:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0x50 | inf);
			break;
		}

		// S3x: Set vibrato waveform
		case SS_VIBWAVE:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0x40 | inf);
			break;
		}

		// S4x: Set tremolo waveform
		case SS_TREMWAVE:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0x70 | inf);
			break;
		}

		// S5x: Set panbrello waveform
		case SS_PANWAVE:
		{
			a->panbWave = inf;
			break;
		}

		// S6x: Delay x number of frames (patDly)
		case SS_FRAMEDELAY:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0xe0 | inf);
			break;
		}

		// S7x: Instrument / NNA commands
		case SS_S7EFFECTS:
		{
			obj->DoNNAEffects(mod, a, inf);
			break;
		}

		// S8x: Set panning position
		case SS_PANNING:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0x80 | inf);
			break;
		}

		// S9x: Set surround sound
		case SS_SURROUND:
		{
			if (mod->panFlag)
				a->main.panning = mod->panning[channel] = PAN_SURROUND;

			break;
		}

		// SAy: Set high order sample offset yxx00h
		case SS_HIOFFSET:
		{
			if (!tick)
			{
				a->hiOffset   = inf << 16;
				a->main.start = a->hiOffset | a->sOffset;

				if ((a->main.s) && ((uint32)a->main.start > a->main.s->length))
					a->main.start = a->main.s->flags & (SF_LOOP | SF_BIDI) ? a->main.s->loopStart : a->main.s->length;
			}
			break;
		}

		// SBx: Pattern loop
		case SS_PATLOOP:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0x60 | inf);
			break;
		}

		// SCx: Notecut
		case SS_NOTECUT:
		{
			if (!inf)
				inf = 1;

			obj->DoEEffects(tick, flags, a, mod, channel, 0xc0 | inf);
			break;
		}

		// SDx: Notedelay
		case SS_NOTEDELAY:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0xd0 | inf);
			break;
		}

		// SEx: Pattern delay
		case SS_PATDELAY:
		{
			obj->DoEEffects(tick, flags, a, mod, channel, 0xe0 | inf);
			break;
		}
	}

	return (0);
}



/******************************************************************************/
/* IT Effect T: Set tempo                                                     */
/******************************************************************************/
int32 MikMod::DoITEffectT(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 tempo;
	int16 temp;

	tempo = obj->uniTrk.UniGetByte();

	if (mod->patDly2)
		return (0);

	temp = mod->bpm;

	if (tempo & 0x10)
		temp += (tempo & 0x0f);
	else
		temp -= tempo;

	mod->bpm = (temp > 255) ? 255 : (temp < 1 ? 1 : temp);

	return (0);
}



/******************************************************************************/
/* IT Effect U: Fine vibrato                                                  */
/******************************************************************************/
int32 MikMod::DoITEffectU(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat, q;
	uint16 temp = 0;	// Silence warning

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat & 0x0f)
			a->vibDepth = dat & 0xf;

		if (dat & 0xf0)
			a->vibSpd = (dat & 0xf0) >> 2;
	}

	if (a->main.period)
	{
		q = (a->vibPos >> 2) & 0x1f;

		switch (a->waveControl & 3)
		{
			// Sine
			case 0:
			{
				temp = vibratoTable[q];
				break;
			}

			// Square wave
			case 1:
			{
				temp = 255;
				break;
			}

			// Ramp down
			case 2:
			{
				q <<= 3;

				if (a->vibPos < 0)
					q = 255 - q;

				temp = q;
				break;
			}

			// Random
			case 3:
			{
				temp = obj->GetRandom(256);
				break;
			}
		}

		temp  *= a->vibDepth;
		temp >>= 8;

		if (a->vibPos >= 0)
			a->main.period = a->tmpPeriod + temp;
		else
			a->main.period = a->tmpPeriod - temp;

		a->ownPer = 1;

		a->vibPos += a->vibSpd;
	}

	return (0);
}



/******************************************************************************/
/* IT Effect W: Slide / Fine slide global volume                              */
/******************************************************************************/
int32 MikMod::DoITEffectW(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 inf, lo, hi;

	inf = obj->uniTrk.UniGetByte();

	if (inf)
		mod->globalSlide = inf;
	else
		inf = mod->globalSlide;

	lo = inf & 0xf;
	hi = inf >> 4;

	if (!lo)
	{
		if (tick)
			mod->volume += hi;
	}
	else
	{
		if (!hi)
		{
			if (tick)
				mod->volume -= lo;
		}
		else
		{
			if (lo == 0xf)
			{
				if (!tick)
					mod->volume += hi;
			}
			else
			{
				if (hi == 0xf)
					if (!tick)
						mod->volume -= lo;
			}
		}
	}

	if (mod->volume < 0)
		mod->volume = 0;
	else
	{
		if (mod->volume > 128)
			mod->volume = 128;
	}

	return (0);
}



/******************************************************************************/
/* IT Effect Y: Panbrello                                                     */
/******************************************************************************/
int32 MikMod::DoITEffectY(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat, q;
	int32 temp = 0;		// Silence warning

	dat = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (dat & 0x0f)
			a->panbDepth = (dat & 0xf);

		if (dat & 0xf0)
			a->panbSpd = (dat & 0xf0) >> 4;
	}

	if (mod->panFlag)
	{
		q = a->panbPos;

		switch (a->panbWave)
		{
			// Sine
			case 0:
			{
				temp = panbrelloTable[q];
				break;
			}

			// Square wave
			case 1:
			{
				temp = (q < 0x80) ? 64 : 0;
				break;
			}

			// Ramp down
			case 2:
			{
				q <<= 3;
				temp = q;
				break;
			}

			// Random
			case 3:
			{
				temp = obj->GetRandom(256);
				break;
			}
		}

		temp *= a->panbDepth;
		temp  = (temp / 8) + mod->panning[channel];

		a->main.panning = (temp < PAN_LEFT) ? PAN_LEFT : (temp > PAN_RIGHT) ? PAN_RIGHT : temp;
		a->panbPos     += a->panbSpd;
	}

	return (0);
}



/******************************************************************************/
/* UltraTracker Specific Effects                                              */
/******************************************************************************/

/******************************************************************************/
/* ULT Effect 9: Sample offset                                                */
/******************************************************************************/
int32 MikMod::DoULTEffect9(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint16 offset = obj->uniTrk.UniGetWord();

	if (offset)
		a->ultOffset = offset;

	a->main.start = a->ultOffset << 2;
	if ((a->main.s) && ((uint32)a->main.start > a->main.s->length))
		a->main.start = a->main.s->flags & (SF_LOOP | SF_BIDI) ? a->main.s->loopStart : a->main.s->length;

	return (0);
}



/******************************************************************************/
/* OctaMED Specific Effects                                                   */
/******************************************************************************/

/******************************************************************************/
/* OctaMED Speed Effect                                                       */
/******************************************************************************/
int32 MikMod::DoMEDSpeed(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint16 speed = obj->uniTrk.UniGetWord();

	mod->bpm = speed;

	return (0);
}



/******************************************************************************/
/* OctaMED Effect F1: Play note twice                                         */
/******************************************************************************/
int32 MikMod::DoMEDEffectF1(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	obj->DoEEffects(tick, flags, a, mod, channel, 0x90 | (mod->sngSpd / 2));

	return (0);
}



/******************************************************************************/
/* OctaMED Effect F2: Delay note                                              */
/******************************************************************************/
int32 MikMod::DoMEDEffectF2(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	obj->DoEEffects(tick, flags, a, mod, channel, 0xd0 | (mod->sngSpd / 2));

	return (0);
}



/******************************************************************************/
/* OctaMED Effect F3: Play note three times                                   */
/******************************************************************************/
int32 MikMod::DoMEDEffectF3(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	obj->DoEEffects(tick, flags, a, mod, channel, 0x90 | (mod->sngSpd / 3));

	return (0);
}



/******************************************************************************/
/* Octalyzer Specific Effects                                                 */
/******************************************************************************/

/******************************************************************************/
/* Octalyzer arpeggio effect                                                  */
/******************************************************************************/
int32 MikMod::DoOktArp(MikMod *obj, uint16 tick, uint16 flags, MP_CONTROL *a, MODULE *mod, int16 channel)
{
	uint8 dat, dat2;

	dat2 = obj->uniTrk.UniGetByte();	// Arpeggio style
	dat  = obj->uniTrk.UniGetByte();

	if (!tick)
	{
		if (!dat && (flags & UF_ARPMEM))
			dat = a->arpMem;
		else
			a->arpMem = dat;
	}

	if (a->main.period)
		obj->DoArpeggio(tick, flags, a, dat2);

	return (0);
}



/******************************************************************************/
/* VoicePlay() starts to play the sample.                                     */
/*                                                                            */
/* Input:  "voice" is the voice to play the sample in.                        */
/*         "s" is a pointer to the sample information.                        */
/*         "start" is where in the sample to start at.                        */
/******************************************************************************/
void MikMod::VoicePlay(uint8 voice, SAMPLE *s, uint32 start)
{
	uint32 repEnd;
	uint16 bits;
	AP_LoopType type;

	if ((voice >= md_sngChn) || (start >= s->length))
		return;

	// Play the sample
	if (s->flags & SF_16BITS)
		bits = 16;
	else
		bits = 8;

	virtChannels[voice]->PlaySample(s->handle, start, s->length, bits);

	// Setup the loop if any
	if ((s->flags & SF_LOOP) && (s->loopStart < s->loopEnd))
	{
		repEnd = s->loopEnd;

		if (repEnd > s->length)
			repEnd = s->length;		// RepEnd can't be bigger than size

		if (s->flags & SF_BIDI)
			type = APLOOP_PingPong;
		else
			type = APLOOP_Normal;

		virtChannels[voice]->SetLoop(s->loopStart, repEnd - s->loopStart, type);
	}
}



/******************************************************************************/
/* VoiceSetVolume() changes the volume in the channel.                        */
/*                                                                            */
/* Input:  "voice" is the voice to change.                                    */
/*         "vol" is the new volume.                                           */
/******************************************************************************/
void MikMod::VoiceSetVolume(uint8 voice, uint16 vol)
{
	if (voice >= md_sngChn)
		return;

	virtChannels[voice]->SetVolume(vol);
}



/******************************************************************************/
/* VoiceSetPanning() changes the panning in the channel.                      */
/*                                                                            */
/* Input:  "voice" is the voice to change.                                    */
/*         "pan" is the new panning.                                          */
/******************************************************************************/
void MikMod::VoiceSetPanning(uint8 voice, uint32 pan)
{
	if (voice >= md_sngChn)
		return;

	if (pan == PAN_SURROUND)
		pan = APPAN_SURROUND;

	virtChannels[voice]->SetPanning(pan);
}



/******************************************************************************/
/* VoiceSetFrequency() changes the frequency in the channel.                  */
/*                                                                            */
/* Input:  "voice" is the voice to change.                                    */
/*         "frq" is the new frequency.                                        */
/******************************************************************************/
void MikMod::VoiceSetFrequency(uint8 voice, uint32 frq)
{
	if (voice >= md_sngChn)
		return;

	virtChannels[voice]->SetFrequency(frq);
}



/******************************************************************************/
/* VoiceStop() stops the channel.                                             */
/*                                                                            */
/* Input:  "voice" is the voice to change.                                    */
/******************************************************************************/
void MikMod::VoiceStop(uint8 voice)
{
	if (voice >= md_sngChn)
		return;

	virtChannels[voice]->Mute();
}



/******************************************************************************/
/* VoiceStopped() returns true if the voice doesn't play anymore, else false. */
/*                                                                            */
/* Input:  "voice" is the voice to check on.                                  */
/*                                                                            */
/* Output: True if it's stopped, else false.                                  */
/******************************************************************************/
bool MikMod::VoiceStopped(uint8 voice)
{
	if (voice > md_sngChn)
		return (false);

	return (!virtChannels[voice]->IsActive());
}
