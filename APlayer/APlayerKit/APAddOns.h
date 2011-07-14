/******************************************************************************/
/* APAddOns header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APAddOns_h
#define __APAddOns_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"
#include "PTime.h"
#include "PList.h"
#include "PSettings.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APChannel.h"


/******************************************************************************/
/* Current add-on version                                                     */
/******************************************************************************/
#define APLAYER_CURRENT_VERSION			4.01f



/******************************************************************************/
/* Private message IDs                                                        */
/******************************************************************************/
#define AP_POSITION_CHANGED				'_APO'
#define AP_MODULEINFO_CHANGED			'_AMI'
#define AP_REPORT_POSITION				'_ARP'
#define AP_MODULE_ENDED					'_AME'



/******************************************************************************/
/* Types used in the different functions.                                     */
/******************************************************************************/
enum ap_result { AP_OK, AP_ERROR, AP_UNKNOWN };



/******************************************************************************/
/* Support flags used in the GetSupportFlags() function. You can OR them      */
/* together to set what your add-on supports.                                 */
/******************************************************************************/
//
// Support flags for players
//
#define appSamplePlayer			0x00000001	// Set this is it's a sample player your create
#define appUseRingBuffer		0x00000004	// Set this if your player require a ring buffer
#define appDontCloseFile		0x00000008	// Set this if you want to use the file pointer in your player
#define appSetPosition			0x00000040	// Set this if your player can change to a certain position


//
// Support flags for agents
//
#define apaConverter			0x00000001	// Your agent can convert modules from one format to another
#define apaDecruncher			0x00000002	// Your agent can decrunch single files
#define apaVirtualMixer			0x10000000	// Your agent need a virtual mixer
#define apaSoundOutput			0x20000000	// Your agent output the sound to some device
#define apaDSP					0x40000000	// Your agent add some DSP effect to the sound data before it will be sent to an output agent
#define apaVisual				0x80000000	// Your agent will show the sound in a visual way


//
// Support flags for converters
//
#define apcLoader				0x00000001	// The converter has a loader
#define apcSaver				0x00000002	// The converter has a saver
#define apcSaverSettings		0x00010000	// There is a settings window for the saver
#define apcSupport8Bit			0x01000000	// The converter can write in 8-bit
#define apcSupport16Bit			0x02000000	// The converter can write in 16-bit
#define apcSupportMono			0x10000000	// The converter can write mono samples
#define apcSupportStereo		0x20000000	// The converter can write stereo samples



/******************************************************************************/
/* These structures can be used in any kind of add-on.                        */
/******************************************************************************/
/* Configuration information structure.                                       */
/* Fill out this structure in your GetConfigInfo() function in your add-on.   */
/******************************************************************************/
typedef struct APConfigInfo
{
	BView *view;						// A pointer to the view which shows the config panel
	PSettings *settings;				// A pointer to the settings object
	PString fileName;					// The file name to use when saving the settings
} APConfigInfo;



/******************************************************************************/
/* Display information structure.                                             */
/* Fill out this structure in your GetDisplayInfo() function in your add-on.  */
/******************************************************************************/
typedef struct APDisplayInfo
{
	BWindow *window;					// A pointer to the window that is opened
	bool openIt;						// Indicate if the window should be opened when APlayer starts
} APDisplayInfo;



/******************************************************************************/
/* Player specific structures.                                                */
/******************************************************************************/
/* Sample player information structure.                                       */
/* Fill out this structure in your GetSamplePlayerInfo() function in your     */
/* player.                                                                    */
/******************************************************************************/
typedef struct APSamplePlayerInfo
{
	uint16 bitSize;						// Number of bits the sample is (only 8 & 16 supported)
	uint32 frequency;					// The frequency to be played with
} APSamplePlayerInfo;



/******************************************************************************/
/* Instrument information structure.                                          */
/* Fill out this structure in the GetInstrumentInfo() function in your        */
/* player.                                                                    */
/******************************************************************************/
typedef struct APInstInfo
{
	PString name;						// The name of the sample
	uint32 flags;						// None at the moment. Write 0 in this field
	int16 notes[10 * 12];				// Sample numbers for each note (-1 means not used)
} APInstInfo;



/******************************************************************************/
/* Sample information structure.                                              */
/* Fill out this structure in the GetSampleInfo() function in your player.    */
/******************************************************************************/
enum APSampType { apSample, apSynth, apHybrid };

typedef struct APSampleInfo
{
	PString name;						// The name of the sample
	uint32 flags;						// Some flags about this sample (see below)
	APSampType type;					// The type of the sample (see above)
	uint8 bitSize;						// Number of bits the sample is (only 8 & 16 supported)
	uint32 middleC;						// The frequency of the middle C (C-4)
	uint16 volume;						// The volume of the sample (0-256)
	int16 panning;						// The panning value (0-255). -1 means no panning
	const void *address;				// Pointer to the sample data
	uint32 length;						// The length of the sample in samples
	uint32 loopStart;					// The start offset to the loop point in samples
	uint32 loopLength;					// The loop length in samples
} APSampleInfo;



/******************************************************************************/
/* Sample information flags.                                                  */
/* These flags are used in the flag field in the APSampleInfo structure above.*/
/******************************************************************************/
#define APSAMP_LOOP			0x00000001	// The sample is looping
#define APSAMP_PINGPONG		0x00000002	// The sample has ping-pong loop (set this together with APSAMP_LOOP)



/******************************************************************************/
/* Agent specific structures.                                                 */
/******************************************************************************/
/* Agent commands                                                             */
/******************************************************************************/

// Converter agent specific commands
#define APCA_CONVERT_MODULE				'CACM'

// Decruncher agent specific commands
#define APDA_DECRUNCH_FILE				'DADF'

// Virtual mixer agent specific commands
#define APMA_INIT_MIXER					'MAIM'
#define APMA_END_MIXER					'MAEM'
#define APMA_MIXING						'MAMX'

// Output agent specific commands
#define APOA_INIT_HARDWARE				'OAIH'
#define APOA_END_HARDWARE				'OAEH'
#define APOA_GET_OUTPUT_INFORMATION		'OAOI'
#define APOA_START_PLAYING				'OAPL'
#define APOA_STOP_PLAYING				'OASP'
#define APOA_RESUME_PLAYING				'OARE'
#define APOA_PAUSE_PLAYING				'OAPA'
#define APOA_SET_VOLUME					'OASV'

// DSP agent specific commands
#define APPA_DSP						'PADS'
#define APPA_STOP						'PAST'

// Visual agent specific commands
#define APVA_MIXED_DATA					'VAMD'
#define APVA_CHANNEL_CHANGE				'VACC'
#define APVA_STOP_SHOWING				'VASS'



/******************************************************************************/
/* Converter agent command structures                                         */
/******************************************************************************/
typedef struct APAgent_ConvertModule
{
	// Read only fields
	PFile *moduleFile;					// The module to load

	// If you convert the module, fill out these fields.
	// If you can not recognize the module, leave these fields untouched
	PFile *newModuleFile;				// A pointer to the file where the new module is stored
	PString modKind;					// The name of the module kind (player name) you converted from
	PString fileType;					// The file type you converted from (mime string)
} APAgent_ConvertModule;



/******************************************************************************/
/* Decruncher agent command structures                                        */
/******************************************************************************/
typedef struct APAgent_DecrunchFile
{
	// Read only fields
	PFile *file;						// The file to decrunch

	// If you decrunch the file, fill out these fields.
	// If you can not recognize the file format, leave these fields untouched
	PFile *decrunchedFile;				// A pointer to the file where the decrunched file is stored
} APAgent_DecrunchFile;



/******************************************************************************/
/* Virtual mixer agent command structures                                     */
/******************************************************************************/
typedef struct APAgent_InitMixer
{
	uint16 channels;					// Store the number of channels you want to be allocated here
} APAgent_InitMixer;


typedef struct APAgent_Mixing
{
	APChannel **channels;				// A pointer to a pointer table with all the channels
} APAgent_Mixing;



/******************************************************************************/
/* Output agent command structures                                            */
/******************************************************************************/
typedef int32 (*Mixer)(void *handle, int16 *buffer, int32 length);

typedef struct APAgent_InitHardware
{
	Mixer mixerFunc;					// Pointer to the main mixer function
	void *handle;						// A handle to use when calling the mixer function
	uint32 frequency;					// The output frequency
	PString fileName;					// The file name with full path of the module loaded
	PString moduleName;					// The name of the module
	PString author;						// The author of the module
} APAgent_InitHardware;


typedef struct APAgent_OutputInfo
{
	uint16 channels;					// Number of channels you want the output in (only 1 and 2 supported)
	uint32 bufferSize;					// Maximum buffer size in samples that will be given to the mixer
} APAgent_OutputInfo;


typedef struct APAgent_SetVolume
{
	uint16 volume;						// The new master volume
} APAgent_SetVolume;



/******************************************************************************/
/* DSP agent command structures                                               */
/******************************************************************************/
typedef struct APAgent_DSP
{
	int32 *buffer;						// Is a pointer to the buffer to add the effect on
	int32 todo;							// Is the size of the buffer in samples
	uint32 frequency;					// The mixer frequency used
	bool stereo;						// Is true if the buffer is in stereo, false if in mono
} APAgent_DSP;



/******************************************************************************/
/* Visual agent command structures                                            */
/******************************************************************************/
typedef struct APAgent_MixedData
{
	const int16 *buffer;				// Is a pointer to the buffer containing the sample data
	int32 length;						// Is the size of the buffer in samples per channel
	bool stereo;						// Is true if the buffer is in stereo, false if in mono
} APAgent_MixedData;


typedef struct APAgent_ChannelChange
{
	uint16 channels;					// Number of channels the module uses
	const APChannel **channelInfo;		// Pointers to each channel object with the current information
	uint32 *channelFlags;				// Use these flags instead of the ones in the APChannel objects
} APAgent_ChannelChange;



/******************************************************************************/
/* Converter specific structures.                                             */
/******************************************************************************/
/* Structure used to tell the converter about the sample format.              */
/******************************************************************************/
typedef struct APConverter_SampleFormat
{
	PString name;						// The name of the sample
	PString author;						// The author of the sample
	uint32 flags;						// Some flags about this sample (the APSAMP_xxxx defined above)
	uint16 bitSize;						// Number of bits the sample is in
	uint32 frequency;					// The frequency of the sample
	uint16 channels;					// The number of channels the sample is in
	uint16 volume;						// The volume of the sample (0-256)
	int16 panning;						// The panning value (0-255). -1 means no panning
	uint32 loopStart;					// The start offset to the loop point in samples
	uint32 loopLength;					// The loop length in samples
} APConverter_SampleFormat;



/******************************************************************************/
/* Add-on classes                                                             */
/******************************************************************************/
/* APAddOnBase class                                                          */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_APKIT APAddOnBase
{
public:
	APAddOnBase(APGlobalData *global);
	virtual ~APAddOnBase(void);

	// Override these functions if you want to change the default implementation
	virtual float GetVersion(void);

	virtual const APConfigInfo *GetConfigInfo(void);
	virtual const APDisplayInfo *GetDisplayInfo(void);

	virtual int32 GetCount(void);
	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index) = 0;
	virtual PString GetDescription(int32 index) = 0;

	float aplayerVersion;			// Set this variable to APLAYER_CURRENT_VERSION in your add-on

protected:
	APGlobalData *globalData;
};



/******************************************************************************/
/* APAddOnPlayer class                                                        */
/******************************************************************************/
class _IMPEXP_APKIT APAddOnPlayer : public APAddOnBase
{
public:
	APAddOnPlayer(APGlobalData *global);
	virtual ~APAddOnPlayer(void);

	// Override these functions if you want to change the default implementation
	virtual PString GetModTypeString(int32 index);

	virtual ap_result ModuleCheck(int32 index, PFile *file) = 0;
	virtual ap_result LoadModule(int32 index, PFile *file, PString &errorStr) = 0;

	virtual bool InitPlayer(int32 index);
	virtual void EndPlayer(int32 index);
	virtual void InitSound(int32 index, uint16 songNum);
	virtual void EndSound(int32 index);
	virtual void Play(void) = 0;

	virtual void GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo);
	virtual PString GetModuleName(void);
	virtual PString GetAuthor(void);
	virtual uint16 GetVirtualChannels(void);
	virtual uint16 GetModuleChannels(void);
	virtual const uint16 *GetSubSongs(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetInstrumentInfo(uint32 num, APInstInfo *info);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

	// Helper functions
	PFile *OpenExtraFile(PString fileName, PString extension);
	void CloseExtraFile(PFile *file);
	int32 GetExtraFilesSize(void);

	void ChangePosition(void);	// Call this everytime your player change it's position
	void ChangeModuleInfo(uint32 line, PString newValue);	// Call this when you have changed some of the module information

	void SetBPMTempo(uint16 bpm);
	uint16 GetBPMTempo(void) const;

	// Private functions. These functions are called from the server.
	// Do NOT call these functions from your add-on
	void SetLooper(BLooper *looper);

	// Public variables
	APChannel **virtChannels;	// A pointer to channel objects, one for each channel
	uint32 mixerFreq;			// This is the frequency of the mixer

	float playFreq;				// How often your play function has to be called in hertz

	bool amigaFilter;			// Set this to true to enable Amiga LED filter, false to disable it
	bool endReached;			// Set this to true, when your module has reached the end

private:
	void OpenFile(PFile *file, PString fileName);

	BLooper *serverLooper;
	int32 totalSize;
};



/******************************************************************************/
/* APAddOnAgent class                                                         */
/******************************************************************************/
class _IMPEXP_APKIT APAddOnAgent : public APAddOnBase
{
public:
	APAddOnAgent(APGlobalData *global);
	virtual ~APAddOnAgent(void);

	// Override these functions if you want to change the default implementation
	virtual int8 GetPluginPriority(uint32 pluginFlag);

	virtual bool InitAgent(int32 index);
	virtual void EndAgent(int32 index);
	virtual ap_result Run(int32 index, uint32 command, void *args);
};



/******************************************************************************/
/* APAddOnClient class                                                        */
/******************************************************************************/
class _IMPEXP_APKIT APAddOnClient : public APAddOnAgent
{
public:
	APAddOnClient(APGlobalData *global);
	virtual ~APAddOnClient(void);

	// Override these functions if you want to change the default implementation
	virtual bool InitClient(int32 index);
	virtual void EndClient(int32 index);
	virtual void Start(int32 index) = 0;
};



/******************************************************************************/
/* APAddOnConverter class                                                     */
/******************************************************************************/
class _IMPEXP_APKIT APAddOnConverter : public APAddOnBase
{
public:
	APAddOnConverter(APGlobalData *global);
	virtual ~APAddOnConverter(void);

	virtual PString GetExtension(int32 index) = 0;
	virtual PString GetTypeString(int32 index);

	// Loader functions
	virtual ap_result FileCheck(int32 index, PFile *file);
	virtual bool LoaderInit(int32 index);
	virtual void LoaderEnd(int32 index);

	virtual ap_result LoadHeader(PFile *file, APConverter_SampleFormat *convInfo, PString &errorStr);
	virtual uint32 LoadData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);

	virtual uint32 GetTotalSampleLength(const APConverter_SampleFormat *convInfo);
	virtual uint32 SetSamplePosition(PFile *file, uint32 position, const APConverter_SampleFormat *convInfo);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);

	// Saver functions
	virtual BWindow *ShowSaverSettings(void);

	virtual bool SaverInit(int32 index, const APConverter_SampleFormat *convInfo);
	virtual void SaverEnd(int32 index, const APConverter_SampleFormat *convInfo);

	virtual ap_result SaveHeader(PFile *file, const APConverter_SampleFormat *convInfo);
	virtual ap_result SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);
	virtual ap_result SaveTail(PFile *file, const APConverter_SampleFormat *convInfo);
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
