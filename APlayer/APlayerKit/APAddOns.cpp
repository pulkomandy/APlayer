/******************************************************************************/
/* APlayer add-on classes.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_APLAYERKIT_

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"
#include "PDirectory.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* APAddOnBase class                                                          */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "global" is a pointer to the global data.                          */
/******************************************************************************/
APAddOnBase::APAddOnBase(APGlobalData *global)
{
	// Remember the global data pointer
	globalData = global;

	// Set the version to a uninitialized value
	aplayerVersion = 0.0;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APAddOnBase::~APAddOnBase(void)
{
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: Is the version of the add-on.                                      */
/******************************************************************************/
float APAddOnBase::GetVersion(void)
{
	return (0.0);
}



/******************************************************************************/
/* GetConfigInfo() will return a pointer to a config structure.               */
/*                                                                            */
/* Output: Is a pointer to a config structure.                                */
/******************************************************************************/
const APConfigInfo *APAddOnBase::GetConfigInfo(void)
{
	return (NULL);
}



/******************************************************************************/
/* GetShowInfo() will return a pointer to a display structure.                */
/*                                                                            */
/* Output: Is a pointer to a display structure.                               */
/******************************************************************************/
const APDisplayInfo *APAddOnBase::GetDisplayInfo(void)
{
	return (NULL);
}



/******************************************************************************/
/* GetCount() returns the number of add-ons in the library.                   */
/*                                                                            */
/* Output: The number of add-ons.                                             */
/******************************************************************************/
int32 APAddOnBase::GetCount(void)
{
	return (1);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 APAddOnBase::GetSupportFlags(int32 index)
{
	return (0);
}





/******************************************************************************/
/* APAddOnPlayer class                                                        */
/******************************************************************************/
static const uint16 noSubSongs[2] = { 1, 0 };



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "global" is a pointer to the global data.                          */
/******************************************************************************/
APAddOnPlayer::APAddOnPlayer(APGlobalData *global) : APAddOnBase(global)
{
	// Initialize public variables
	virtChannels = NULL;
	playFreq     = 50.0f;

	amigaFilter  = false;
	endReached   = false;

	serverLooper = NULL;

	// Initialize private variables
	totalSize = 0;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APAddOnPlayer::~APAddOnPlayer(void)
{
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString APAddOnPlayer::GetModTypeString(int32 index)
{
	return ("");
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APAddOnPlayer::InitPlayer(int32 index)
{
	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void APAddOnPlayer::EndPlayer(int32 index)
{
}



/******************************************************************************/
/* InitSound() initialize the current song.                                   */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void APAddOnPlayer::InitSound(int32 index, uint16 songNum)
{
}



/******************************************************************************/
/* EndSound() ends the current song.                                          */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void APAddOnPlayer::EndSound(int32 index)
{
}



/******************************************************************************/
/* GetSamplePlayerInfo() will fill out the sample info structure given.       */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to fill.      */
/******************************************************************************/
void APAddOnPlayer::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString APAddOnPlayer::GetModuleName(void)
{
	return ("");
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: Is the author.                                                     */
/******************************************************************************/
PString APAddOnPlayer::GetAuthor(void)
{
	return ("");
}



/******************************************************************************/
/* GetVirtualChannels() returns the number of channels the module want to     */
/*      reserve.                                                              */
/*                                                                            */
/* Output: Is the number of required channels.                                */
/******************************************************************************/
uint16 APAddOnPlayer::GetVirtualChannels(void)
{
	return (GetModuleChannels());
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 APAddOnPlayer::GetModuleChannels(void)
{
	return (4);
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *APAddOnPlayer::GetSubSongs(void)
{
	return (noSubSongs);
}



/******************************************************************************/
/* GetSongLength() returns the length of the current song.                    */
/*                                                                            */
/* Output: Is the length of the current song.                                 */
/******************************************************************************/
int16 APAddOnPlayer::GetSongLength(void)
{
	return (0);
}



/******************************************************************************/
/* GetSongPosition() returns the current position of the playing song.        */
/*                                                                            */
/* Output: Is the current position.                                           */
/******************************************************************************/
int16 APAddOnPlayer::GetSongPosition(void)
{
	return (-1);
}



/******************************************************************************/
/* SetSongPosition() sets the current position of the playing song.           */
/*                                                                            */
/* Input:  "pos" is the new position.                                         */
/******************************************************************************/
void APAddOnPlayer::SetSongPosition(int16 pos)
{
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
PTimeSpan APAddOnPlayer::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	return (0);
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
bool APAddOnPlayer::GetInfoString(uint32 line, PString &description, PString &value)
{
	return (false);
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
bool APAddOnPlayer::GetInstrumentInfo(uint32 num, APInstInfo *info)
{
	return (false);
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
bool APAddOnPlayer::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	return (false);
}



/******************************************************************************/
/* OpenExtraFile() will try to open the file and return a pointer to a file   */
/*      you have to use to load the file.                                     */
/*                                                                            */
/* Input:  "fileName" is the file you want to open.                           */
/*         "extension" is the extension/prefix you want to add to the file.   */
/*                                                                            */
/* Output: A pointer to a file object.                                        */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
PFile *APAddOnPlayer::OpenExtraFile(PString fileName, PString extension)
{
	PCacheFile *file;
	PString workFile, workExt, workDir;
	int32 index;

	// Allocate memory for the file object
	file = new PCacheFile();
	if (file == NULL)
		throw PMemoryException();

	try
	{
		if (extension.IsEmpty())
		{
			// No extension is given, so just try to
			// use the filename directly
			OpenFile(file, fileName);
			if (file->IsOpen())
				throw PUserException();

			throw PFileException(P_FILE_ERR_ENTRY_NOT_FOUND, fileName);
		}

		// First try directly with the extension
		OpenFile(file, fileName + "." + extension);
		if (file->IsOpen())
			throw PUserException();

		// Try with lowercase extension
		workExt = extension;
		workExt.MakeLower();
		OpenFile(file, fileName + "." + workExt);
		if (file->IsOpen())
			throw PUserException();

		// Try with uppercase extension
		workExt.MakeUpper();
		OpenFile(file, fileName + "." + workExt);
		if (file->IsOpen())
			throw PUserException();

		// Clip out the extension
		index = fileName.ReverseFind('.');
		if (index == -1)
			throw PFileException(P_FILE_ERR_ENTRY_NOT_FOUND, fileName + "." + extension);

		workFile = fileName.Left(index);

		// Try directly with the extension
		OpenFile(file, workFile + "." + extension);
		if (file->IsOpen())
			throw PUserException();

		// Try with lowercase extension
		workExt.MakeLower();
		OpenFile(file, workFile + "." + workExt);
		if (file->IsOpen())
			throw PUserException();

		// Try with uppercase extension
		workExt.MakeUpper();
		OpenFile(file, workFile + "." + workExt);
		if (file->IsOpen())
			throw PUserException();

		// Clip out prefix
		index = fileName.Find('.');
		if (index == -1)
			throw PFileException(P_FILE_ERR_ENTRY_NOT_FOUND, workFile + "." + extension);

		workDir  = PDirectory::GetDirectoryPart(fileName);
		workFile = fileName.Mid(index + 1);

		// Try directly with the prefix
		OpenFile(file, workDir + extension + "." + workFile);
		if (file->IsOpen())
			throw PUserException();

		// Try with lowercase prefix
		workExt.MakeLower();
		OpenFile(file, workDir + workExt + "." + workFile);
		if (file->IsOpen())
			throw PUserException();

		// Try with uppercase extension
		workExt.MakeUpper();
		OpenFile(file, workDir + workExt + "." + workFile);
		if (file->IsOpen())
			throw PUserException();

		throw PFileException(P_FILE_ERR_ENTRY_NOT_FOUND, extension + "." + workFile);
	}
	catch(PUserException e)
	{
		// Add the size to the total length variable
		totalSize += file->GetLength();
		return (file);
	}
	catch(...)
	{
		delete file;
		throw;
	}

	// Will never reach this line, but it's here just to make the compiler happy
	return (NULL);
}



/******************************************************************************/
/* CloseExtraFile() will close the file again and clean up.                   */
/*                                                                            */
/* Input:  "file" is the pointer you got from the OpenExtraFile() function.   */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APAddOnPlayer::CloseExtraFile(PFile *file)
{
	// Close the file and clean up
	file->Close();
	delete file;
}



/******************************************************************************/
/* GetExtraFilesSize() will return the total size of all the extra files      */
/*      loaded.                                                               */
/*                                                                            */
/* Output: The total size of all the files loaded.                            */
/******************************************************************************/
int32 APAddOnPlayer::GetExtraFilesSize(void)
{
	return (totalSize);
}



/******************************************************************************/
/* ChangePosition() will tell APlayer that your module has changed the        */
/*      position.                                                             */
/******************************************************************************/
void APAddOnPlayer::ChangePosition(void)
{
	ASSERT(serverLooper != NULL);
	serverLooper->PostMessage(AP_POSITION_CHANGED);
}



/******************************************************************************/
/* ChangeModuleInfo() will change the module info you give.                   */
/*                                                                            */
/* Input:  "line" is the line starting from 0.                                */
/*         "newValue" is the new value string.                                */
/******************************************************************************/
void APAddOnPlayer::ChangeModuleInfo(uint32 line, PString newValue)
{
	BMessage msg(AP_MODULEINFO_CHANGED);
	char *valStr;

	// Add the information to the message
	msg.AddInt32("line", line);
	msg.AddString("value", (valStr = newValue.GetString()));
	newValue.FreeBuffer(valStr);

	// Send the message
	ASSERT(serverLooper != NULL);
	serverLooper->PostMessage(&msg);
}



/******************************************************************************/
/* SetBPMTempo() calculates the frequency equal to the BPM you give and store */
/*      it in the playFreq variable.                                          */
/*                                                                            */
/* Input:  "bpm" is the BPM you want to run at.                               */
/******************************************************************************/
void APAddOnPlayer::SetBPMTempo(uint16 bpm)
{
	playFreq = ((float)bpm) / 2.5f;
}



/******************************************************************************/
/* GetBPMTempo() calculates the BPM from the frequency stored in the playFreq */
/*      variable.                                                             */
/*                                                                            */
/* Output: Is the calculated BPM.                                             */
/******************************************************************************/
uint16 APAddOnPlayer::GetBPMTempo(void) const
{
	return ((uint16)(playFreq * 2.5f));
}



/******************************************************************************/
/* SetLooper() sets the looper which the player uses to communicate with the  */
/*      server.                                                               */
/*                                                                            */
/* Input:  "looper" is a pointer to the server looper.                        */
/******************************************************************************/
void APAddOnPlayer::SetLooper(BLooper *looper)
{
	serverLooper = looper;
}



/******************************************************************************/
/* OpenFile() will try to open the file.                                      */
/*                                                                            */
/* Input:  "file" is the pointer you to the file object to use.               */
/*         "fileName" is the file you want to open.                           */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APAddOnPlayer::OpenFile(PFile *file, PString fileName)
{
	try
	{
		// Try to open the file
		file->Open(fileName, PFile::pModeRead);
	}
	catch(PFileException e)
	{
		if (e.errorNum != P_FILE_ERR_ENTRY_NOT_FOUND)
			throw;
	}
}





/******************************************************************************/
/* APAddOnAgent class                                                         */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "global" is a pointer to the global data.                          */
/******************************************************************************/
APAddOnAgent::APAddOnAgent(APGlobalData *global) : APAddOnBase(global)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APAddOnAgent::~APAddOnAgent(void)
{
}



/******************************************************************************/
/* GetPluginPriority() returns the priority the given plug-in place should    */
/*      have.                                                                 */
/*                                                                            */
/* Input:  "pluginFlag" is one of the plug-in flags to return the priority    */
/*         for.                                                               */
/*                                                                            */
/* Output: The priority.                                                      */
/******************************************************************************/
int8 APAddOnAgent::GetPluginPriority(uint32 pluginFlag)
{
	return (0);
}



/******************************************************************************/
/* InitAgent() will initialize the agent.                                     */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APAddOnAgent::InitAgent(int32 index)
{
	return (true);
}



/******************************************************************************/
/* EndAgent() will clean up the agent.                                        */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/******************************************************************************/
void APAddOnAgent::EndAgent(int32 index)
{
}



/******************************************************************************/
/* Run() will run one command in the agent.                                   */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*         "command" is the command to run.                                   */
/*         "args" is a pointer to a structure containing the arguments. The   */
/*         structure is different for each command.                           */
/*                                                                            */
/* Output: The result from the command.                                       */
/******************************************************************************/
ap_result APAddOnAgent::Run(int32 index, uint32 command, void *args)
{
	return (AP_UNKNOWN);
}





/******************************************************************************/
/* APAddOnClient class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "global" is a pointer to the global data.                          */
/******************************************************************************/
APAddOnClient::APAddOnClient(APGlobalData *global) : APAddOnAgent(global)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APAddOnClient::~APAddOnClient(void)
{
}



/******************************************************************************/
/* InitClient() initialize the client add-on.                                 */
/*                                                                            */
/* Input:  "index" is the client index number.                                */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APAddOnClient::InitClient(int32 index)
{
	return (true);
}



/******************************************************************************/
/* EndClient() cleanup the client add-on.                                     */
/*                                                                            */
/* Input:  "index" is the client index number.                                */
/******************************************************************************/
void APAddOnClient::EndClient(int32 index)
{
}





/******************************************************************************/
/* APAddOnConverter class                                                     */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "global" is a pointer to the global data.                          */
/******************************************************************************/
APAddOnConverter::APAddOnConverter(APGlobalData *global) : APAddOnBase(global)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APAddOnConverter::~APAddOnConverter(void)
{
}



/******************************************************************************/
/* GetTypeString() returns the sample type string.                            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The sample type string.                                            */
/******************************************************************************/
PString APAddOnConverter::GetTypeString(int32 index)
{
	return ("");
}



/******************************************************************************/
/* FileCheck() checks the file to see if it's a supported sample.             */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result APAddOnConverter::FileCheck(int32 index, PFile *file)
{
	return (AP_UNKNOWN);
}



/******************************************************************************/
/* LoaderInit() initialize the loader.                                        */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APAddOnConverter::LoaderInit(int32 index)
{
	return (true);
}



/******************************************************************************/
/* LoaderEnd() cleanup the loader.                                            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/******************************************************************************/
void APAddOnConverter::LoaderEnd(int32 index)
{
}



/******************************************************************************/
/* LoadHeader() loads the sample header.                                      */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "convInfo" is a pointer to the info structure you has to fill out. */
/*         "errorStr" is a reference to a string to store the error in if any.*/
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result APAddOnConverter::LoadHeader(PFile *file, APConverter_SampleFormat *convInfo, PString &errorStr)
{
	return (AP_OK);
}



/******************************************************************************/
/* LoadData() loads some part of the sample data.                             */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "buffer" is a pointer to the buffer you has to fill with the data. */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples filled in the buffer.                        */
/******************************************************************************/
uint32 APAddOnConverter::LoadData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	return (0);
}



/******************************************************************************/
/* GetTotalSampleLength() calculates how many samples that will be returned.  */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples the file is.                                 */
/******************************************************************************/
uint32 APAddOnConverter::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
{
	return (0);
}



/******************************************************************************/
/* SetSamplePosition() sets the file position in the sample file.             */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "position" is the start position from the start of the sample.     */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The real position in the sample.                                   */
/******************************************************************************/
uint32 APAddOnConverter::SetSamplePosition(PFile *file, uint32 position, const APConverter_SampleFormat *convInfo)
{
	return (position);
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
bool APAddOnConverter::GetInfoString(uint32 line, PString &description, PString &value)
{
	return (false);
}



/******************************************************************************/
/* ShowSaverSettings() will open a window showing the parameters to change in */
/*      the saver routine.                                                    */
/*                                                                            */
/* Output: Pointer to the window that have been opened.                       */
/******************************************************************************/
BWindow *APAddOnConverter::ShowSaverSettings(void)
{
	return (NULL);
}



/******************************************************************************/
/* SaverInit() initialize the saver.                                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APAddOnConverter::SaverInit(int32 index, const APConverter_SampleFormat *convInfo)
{
	return (true);
}



/******************************************************************************/
/* SaverEnd() cleanup the saver.                                              */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void APAddOnConverter::SaverEnd(int32 index, const APConverter_SampleFormat *convInfo)
{
}



/******************************************************************************/
/* SaveHeader() saves the sample header.                                      */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to save to.    */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result APAddOnConverter::SaveHeader(PFile *file, const APConverter_SampleFormat *convInfo)
{
	return (AP_OK);
}



/******************************************************************************/
/* SaveData() saves some part of the sample data.                             */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to save to.    */
/*         "buffer" is a pointer to the buffer with the sample data to save.  */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result APAddOnConverter::SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	return (AP_ERROR);
}



/******************************************************************************/
/* SaveTail() saves the sample tail.                                          */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to save to.    */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result APAddOnConverter::SaveTail(PFile *file, const APConverter_SampleFormat *convInfo)
{
	return (AP_OK);
}
