/******************************************************************************/
/* Player information Interface.                                              */
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
#include "PSynchronize.h"
#include "PTime.h"
#include "PList.h"

// Client headers
#include "APPlayerInfo.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APPlayerInfo::APPlayerInfo(void) : varLock(false)
{
	// Initialize member variables
	ResetInfo();

	// Well, the volume isn't initialized in the previous
	// function call, so we do it here
	volume = 256;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APPlayerInfo::~APPlayerInfo(void)
{
}



/******************************************************************************/
/* Lock() will lock the information. Remember to call Unlock() when done.     */
/******************************************************************************/
void APPlayerInfo::Lock(void)
{
	// Lock the mutex
	varLock.Lock();
}



/******************************************************************************/
/* Unlock() unlocks the information after a lock.                             */
/******************************************************************************/
void APPlayerInfo::Unlock(void)
{
	// Unlock the mutex
	varLock.Unlock();
}



/******************************************************************************/
/* ResetInfo() clears all the information.                                    */
/******************************************************************************/
void APPlayerInfo::ResetInfo(void)
{
	PLock lock(&varLock);

	info        = false;
	playing     = false;
	currentSong = 0;
	maxSongNum  = 0;
	songLength  = 0;
	songPos     = 0;
	chanNum     = 0;
	moduleSize  = 0;
	changePos   = false;
	muted       = false;

	moduleName.MakeEmpty();
	author.MakeEmpty();
	totalTime.SetTimeSpan(0);
	posTimes.MakeEmpty();
	fileName.MakeEmpty();
	moduleFormat.MakeEmpty();
	playerName.MakeEmpty();
	modInfo.MakeEmpty();
}



/******************************************************************************/
/* SetCurrentSong() sets the current song.                                    */
/*                                                                            */
/* Input:   "newSong" is the new song.                                        */
/******************************************************************************/
void APPlayerInfo::SetCurrentSong(uint16 newSong)
{
	PLock lock(&varLock);

	currentSong = newSong;
}



/******************************************************************************/
/* SetMaxSongNumber() sets the maximum song.                                  */
/*                                                                            */
/* Input:   "newMaxSong" is the new maximum song.                             */
/******************************************************************************/
void APPlayerInfo::SetMaxSongNumber(uint16 newMaxSong)
{
	PLock lock(&varLock);

	maxSongNum = newMaxSong;
}



/******************************************************************************/
/* SetSongLength() sets the song length.                                      */
/*                                                                            */
/* Input:   "newLen" is the new song length.                                  */
/******************************************************************************/
void APPlayerInfo::SetSongLength(int16 newLen)
{
	PLock lock(&varLock);

	songLength = newLen;
}



/******************************************************************************/
/* SetSongPosition() sets the song position.                                  */
/*                                                                            */
/* Input:   "newPos" is the new song position.                                */
/******************************************************************************/
void APPlayerInfo::SetSongPosition(int16 newPos)
{
	PLock lock(&varLock);

	songPos = newPos;
}



/******************************************************************************/
/* SetModuleChannels() sets the number of channels used in the module.        */
/*                                                                            */
/* Input:   "newChanNum" is the new number of channels.                       */
/******************************************************************************/
void APPlayerInfo::SetModuleChannels(uint16 newChanNum)
{
	PLock lock(&varLock);

	chanNum = newChanNum;
}



/******************************************************************************/
/* SetModuleSize() sets the size of the module.                               */
/*                                                                            */
/* Input:   "newSize" is the new size.                                        */
/******************************************************************************/
void APPlayerInfo::SetModuleSize(uint32 newSize)
{
	PLock lock(&varLock);

	moduleSize = newSize;
}



/******************************************************************************/
/* SetModuleName() sets the module name.                                      */
/*                                                                            */
/* Input:   "newName" is the new module name.                                 */
/******************************************************************************/
void APPlayerInfo::SetModuleName(PString newName)
{
	PLock lock(&varLock);

	moduleName = newName;
}



/******************************************************************************/
/* SetAuthor() sets the author.                                               */
/*                                                                            */
/* Input:   "newName" is the new author.                                      */
/******************************************************************************/
void APPlayerInfo::SetAuthor(PString newName)
{
	PLock lock(&varLock);

	author = newName;
}



/******************************************************************************/
/* SetTotalTime() sets the total time.                                        */
/*                                                                            */
/* Input:   "newTotalTime" is the new total time.                             */
/******************************************************************************/
void APPlayerInfo::SetTotalTime(PTimeSpan newTotalTime)
{
	PLock lock(&varLock);

	totalTime = newTotalTime;
}



/******************************************************************************/
/* SetPositionTimes() sets the position times.                                */
/*                                                                            */
/* Input:   "newPosTimes" is the new position times.                          */
/******************************************************************************/
void APPlayerInfo::SetPositionTimes(const PList<PTimeSpan> &newPosTimes)
{
	PLock lock(&varLock);

	// Copy the items
	posTimes = newPosTimes;
}



/******************************************************************************/
/* SetFileName() sets the name of the file.                                   */
/*                                                                            */
/* Input:   "newName" is the new file name.                                   */
/******************************************************************************/
void APPlayerInfo::SetFileName(PString newName)
{
	PLock lock(&varLock);

	fileName = newName;
}



/******************************************************************************/
/* SetModuleFormat() sets the module format.                                  */
/*                                                                            */
/* Input:   "newFormat" is the new module format.                             */
/******************************************************************************/
void APPlayerInfo::SetModuleFormat(PString newFormat)
{
	PLock lock(&varLock);

	moduleFormat = newFormat;
}



/******************************************************************************/
/* SetPlayerName() sets the name of the player.                               */
/*                                                                            */
/* Input:   "newName" is the new player name.                                 */
/******************************************************************************/
void APPlayerInfo::SetPlayerName(PString newName)
{
	PLock lock(&varLock);

	playerName = newName;
}



/******************************************************************************/
/* SetOutputAgent() sets the name of the output agent.                        */
/*                                                                            */
/* Input:   "newAgent" is the new agent name.                                 */
/******************************************************************************/
void APPlayerInfo::SetOutputAgent(PString newAgent)
{
	PLock lock(&varLock);

	outputAgent = newAgent;
}



/******************************************************************************/
/* SetModuleInformation() sets the list of module information.                */
/*                                                                            */
/* Input:   "newInfo" is the new module information.                          */
/******************************************************************************/
void APPlayerInfo::SetModuleInformation(const PList<PString> &newInfo)
{
	PLock lock(&varLock);

	// Copy the items
	modInfo = newInfo;
}



/******************************************************************************/
/* SetVolume() sets the master volume.                                        */
/*                                                                            */
/* Input:   "newVol" is the new volume.                                       */
/******************************************************************************/
void APPlayerInfo::SetVolume(uint16 newVol)
{
	PLock lock(&varLock);

	volume = newVol;
}



/******************************************************************************/
/* SetChangePositionFlag() sets the change position flag.                     */
/*                                                                            */
/* Input:   "changeFlag" is what to set the flag to.                          */
/******************************************************************************/
void APPlayerInfo::SetChangePositionFlag(bool changeFlag)
{
	PLock lock(&varLock);

	changePos = changeFlag;
}



/******************************************************************************/
/* SetInfoFlag() sets the information flag.                                   */
/*                                                                            */
/* Input:   "infoFlag" is what to set the flag to.                            */
/******************************************************************************/
void APPlayerInfo::SetInfoFlag(bool infoFlag)
{
	PLock lock(&varLock);

	info = infoFlag;
}



/******************************************************************************/
/* SetPlayFlag() sets the playing flag.                                       */
/*                                                                            */
/* Input:   "playFlag" is what to set the flag to.                            */
/******************************************************************************/
void APPlayerInfo::SetPlayFlag(bool playFlag)
{
	PLock lock(&varLock);

	playing = playFlag;
}



/******************************************************************************/
/* SetMuteFlag() sets the mute flag.                                          */
/*                                                                            */
/* Input:   "muteFlag" is what to set the flag to.                            */
/******************************************************************************/
void APPlayerInfo::SetMuteFlag(bool muteFlag)
{
	PLock lock(&varLock);

	muted = muteFlag;
}



/******************************************************************************/
/* GetCurrentSong() returns the current playing song number.                  */
/*                                                                            */
/* Output:  The song number.                                                  */
/******************************************************************************/
uint16 APPlayerInfo::GetCurrentSong(void)
{
	PLock lock(&varLock);

	return (currentSong);
}



/******************************************************************************/
/* GetMaxSongNumber() returns the maximum number of sub-songs.                */
/*                                                                            */
/* Output:  The maximum number of sub-songs.                                  */
/******************************************************************************/
uint16 APPlayerInfo::GetMaxSongNumber(void)
{
	PLock lock(&varLock);

	return (maxSongNum);
}



/******************************************************************************/
/* GetSongLength() returns the length of the current playing song.            */
/*                                                                            */
/* Output:  The song length.                                                  */
/******************************************************************************/
int16 APPlayerInfo::GetSongLength(void)
{
	PLock lock(&varLock);

	return (songLength);
}



/******************************************************************************/
/* GetSongPosition() returns the current position of the playing song.        */
/*                                                                            */
/* Output:  The song position.                                                */
/******************************************************************************/
int16 APPlayerInfo::GetSongPosition(void)
{
	PLock lock(&varLock);

	return (songPos);
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels used in the module.     */
/*                                                                            */
/* Output:  The number of channels.                                           */
/******************************************************************************/
uint16 APPlayerInfo::GetModuleChannels(void)
{
	PLock lock(&varLock);

	return (chanNum);
}



/******************************************************************************/
/* GetModuleSize() returns the size of the module.                            */
/*                                                                            */
/* Output:  The module size.                                                  */
/******************************************************************************/
uint32 APPlayerInfo::GetModuleSize(void)
{
	PLock lock(&varLock);

	return (moduleSize);
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output:  The module name.                                                  */
/******************************************************************************/
PString APPlayerInfo::GetModuleName(void)
{
	PLock lock(&varLock);

	return (moduleName);
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output:  The author.                                                       */
/******************************************************************************/
PString APPlayerInfo::GetAuthor(void)
{
	PLock lock(&varLock);

	return (author);
}



/******************************************************************************/
/* GetTotalTime() returns the total time of the playing song.                 */
/*                                                                            */
/* Output:  The total time.                                                   */
/******************************************************************************/
PTimeSpan APPlayerInfo::GetTotalTime(void)
{
	PLock lock(&varLock);

	return (totalTime);
}



/******************************************************************************/
/* GetPositionTime() returns the time on the position given.                  */
/*                                                                            */
/* Input:   "position" is the song position you want the time on.             */
/*                                                                            */
/* Output:  The position time.                                                */
/******************************************************************************/
PTimeSpan APPlayerInfo::GetPositionTime(int16 position)
{
	PLock lock(&varLock);

	if (posTimes.IsEmpty() || (position < 0))
		return (-1);

	return (posTimes.GetItem(position));
}



/******************************************************************************/
/* GetFileName() returns the name of the file.                                */
/*                                                                            */
/* Output:  The file name.                                                    */
/******************************************************************************/
PString APPlayerInfo::GetFileName(void)
{
	PLock lock(&varLock);

	return (fileName);
}



/******************************************************************************/
/* GetModuleFormat() returns the module format.                               */
/*                                                                            */
/* Output:  The module format.                                                */
/******************************************************************************/
PString APPlayerInfo::GetModuleFormat(void)
{
	PLock lock(&varLock);

	return (moduleFormat);
}



/******************************************************************************/
/* GetPlayerName() returns the name of the player.                            */
/*                                                                            */
/* Output:  The player name.                                                  */
/******************************************************************************/
PString APPlayerInfo::GetPlayerName(void)
{
	PLock lock(&varLock);

	return (playerName);
}



/******************************************************************************/
/* GetOutputAgent() returns the name of the output agent.                     */
/*                                                                            */
/* Output:  The player name.                                                  */
/******************************************************************************/
PString APPlayerInfo::GetOutputAgent(void)
{
	PLock lock(&varLock);

	return (outputAgent);
}



/******************************************************************************/
/* GetModuleInformation() returns the module information on the line given.   */
/*                                                                            */
/* Input:   "line" is the line number you want information about.             */
/*          "description" is a reference where the description will be stored.*/
/*          "value" is a reference where the value will be stored.            */
/*                                                                            */
/* Output:  True if some information was returned, else false.                */
/******************************************************************************/
bool APPlayerInfo::GetModuleInformation(int32 line, PString &description, PString &value)
{
	PString tempStr;
	int32 index;
	PLock lock(&varLock);

	// Is the line number out of range?
	if (line >= modInfo.CountItems())
		return (false);

	// Get the information
	tempStr = modInfo.GetItem(line);

	// Find the spliter character between the description and value
	index = tempStr.Find('\t');
	if (index == -1)
		return (false);

	// Extract the information
	description = tempStr.Left(index);
	value       = tempStr.Mid(index + 1);

	return (true);
}



/******************************************************************************/
/* GetVolume() returns the master volume.                                     */
/*                                                                            */
/* Output:  The master volume.                                                */
/******************************************************************************/
uint16 APPlayerInfo::GetVolume(void)
{
	PLock lock(&varLock);

	return (volume);
}



/******************************************************************************/
/* CanChangePosition() returns if the player support position change or not.  */
/*                                                                            */
/* Output:  True if the player support position change, false if not.         */
/******************************************************************************/
bool APPlayerInfo::CanChangePosition(void)
{
	PLock lock(&varLock);

	return (changePos);
}



/******************************************************************************/
/* HaveInformation() tells if there is valid information in the object.       */
/*                                                                            */
/* Output:  True if the information is valid, false if not.                   */
/******************************************************************************/
bool APPlayerInfo::HaveInformation(void)
{
	PLock lock(&varLock);

	return (info);
}



/******************************************************************************/
/* IsPlaying() tells if the module is playing or not at the moment.           */
/*                                                                            */
/* Output:  True if the module plays, false if not.                           */
/******************************************************************************/
bool APPlayerInfo::IsPlaying(void)
{
	PLock lock(&varLock);

	return (playing);
}



/******************************************************************************/
/* IsMuted() tells if the volume is muted or not at the moment.               */
/*                                                                            */
/* Output:  True if the volume is muted, false if not.                        */
/******************************************************************************/
bool APPlayerInfo::IsMuted(void)
{
	PLock lock(&varLock);

	return (muted);
}
