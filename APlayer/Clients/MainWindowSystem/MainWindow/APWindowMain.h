/******************************************************************************/
/* APWindowMain header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowMain_h
#define __APWindowMain_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PTime.h"
#include "PTimer.h"

// APlayerKit headers
#include "APGlobalData.h"

// Client headers
#include "APBackView.h"
#include "APInfoView.h"
#include "APToolTips.h"
#include "APPictureButton.h"
#include "APDiskButton.h"
#include "APVolSlider.h"
#include "APPosSlider.h"
#include "APKeyFilter.h"
#include "APWindowMainList.h"
#include "APWindowMainListItem.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define AP_MOD_PLAY_REFS_RECEIVED		'_mpr'
#define AP_MOD_ADD_REFS_RECEIVED		'_mar'
#define AP_APML_LOAD_REFS_RECEIVED		'_alr'
#define AP_APML_APPEND_REFS_RECEIVED	'_aar'
#define AP_APML_SAVE_REFS_RECEIVED		'_asr'

#define AP_INIT_CONTROLS				'_ict'
#define AP_UPDATE_INFORMATION			'_uif'
#define AP_UPDATE_SELECTION				'_usl'
#define AP_RESET_TIME					'_rti'
#define AP_CHANGE_PLAY_ITEM				'_cpi'

#define AP_MENU_ONLINEHELP				'_olh'
#define AP_MENU_SETTINGS				'_set'
#define AP_MENU_ADDONDISPLAYVIEW		'_adv'
#define AP_MENU_ADDONCONFIGVIEW			'_acv'

#define AP_MODULEINFO_TEXT				'_mit'
#define AP_MODULEINFO_BUTTON			'_mif'

#define AP_MUTE_BUTTON					'_mut'
#define AP_POS_SLIDER					'_pos'

#define AP_ADD_BUTTON					'_add'
#define AP_REMOVE_BUTTON				'_rem'
#define AP_SWAP_BUTTON					'_swp'
#define AP_SORT_BUTTON					'_sor'
#define AP_UP_BUTTON					'_up_'
#define AP_DOWN_BUTTON					'_dwn'
#define AP_SELECT_BUTTON				'_sel'
#define AP_DISK_BUTTON					'_dsk'

#define AP_SORTMENU_SORT_AZ				'_SAZ'
#define AP_SORTMENU_SORT_ZA				'_SZA'
#define AP_SORTMENU_SHUFFLE				'_SHU'

#define AP_SELECTMENU_ALL				'_ALL'
#define AP_SELECTMENU_NONE				'_NON'

#define AP_DISKMENU_LOAD				'_LOA'
#define AP_DISKMENU_APPEND				'_APP'
#define AP_DISKMENU_SAVE				'_SAV'

#define AP_PREVIOUSMOD_BUTTON			'_prm'
#define AP_PREVIOUSSONG_BUTTON			'_prs'
#define AP_REWIND_BUTTON				'_rwd'
#define AP_PLAY_BUTTON					'_ply'
#define AP_FORWARD_BUTTON				'_fwd'
#define AP_NEXTSONG_BUTTON				'_nxs'
#define AP_NEXTMOD_BUTTON				'_nxm'
#define AP_EJECT_BUTTON					'_ejc'
#define AP_PAUSE_BUTTON					'_pau'

#define AP_SAMPLE_BUTTON				'_sam'



/******************************************************************************/
/* Timer ids                                                                  */
/******************************************************************************/
#define AP_TIME_CLOCK			1
#define AP_TIME_NEVERENDING		2



/******************************************************************************/
/* Multi file structures                                                      */
/******************************************************************************/
class APWindowMain;

typedef struct APMultiFileAdd
{
	APWindowMain *window;
	BList tempList;
} APMultiFileAdd;



typedef struct APMultiFileSave
{
	APWindowMain *window;
	int32 index;
	int32 count;
} APMultiFileSave;



/******************************************************************************/
/* APWindowMain class                                                         */
/******************************************************************************/
class MainWindowSystem;

class APWindowMain : public BWindow
{
public:
	APWindowMain(MainWindowSystem *system, APGlobalData *global, BRect frame);
	virtual ~APWindowMain(void);

	void AddAddOnsToMenu(void);
	void AddAddOnToMenu(APAddOnInformation *addOn);
	void RemoveAddOnFromMenu(APAddOnInformation *addOn);

	void AddFilesToList(BMessage *message, bool skipList = false, int32 startIndex = -1);
	void AddFileToList(PString fileName, bool updateWindow = true);
	void UpdateList(void);

	void SetNormalCursor(void);
	void SetForbiddenCursor(void);
	void SetAppendCursor(void);
	void SetInsertCursor(void);
	void SetSleepCursor(void);

	void EnableListNumber(bool enable);
	void EnableToolTips(bool enable);
	void EnableShowNameInTitlebar(bool enable);

	APWindowMainListItem *GetListItem(int32 index);
	int32 GetPlayItem(void);
	int32 GetListCount(void);
	void SetTimeOnItem(APWindowMainListItem *item, PTimeSpan time);
	void RemoveItemTimeFromList(APWindowMainListItem *item);
	void RemoveListItem(int32 index);
	void EmptyList(bool lock);

	void InitWindowWhenPlayStarts(void);
	void StopAndFreeModule(void);
	void InitSubSongs(void);

	bool IsLoopOn(void);

	virtual bool QuitRequested(void);

protected:
	enum TimeFormat { apElapsed, apRemaining };

	friend void APPosSlider::MouseDown(BPoint);
	friend void APPosSlider::MouseUp(BPoint);

	virtual void MessageReceived(BMessage *msg);

	BPoint CalcMinSize(void);
	void SetPosAndSize(void);

	bool ParseKey(char key, int32 modifiers, int32 code);

	BCursor *CreateCursor(int32 id, bool merge);

	void AddAddOnListToMenu(APList<APAddOnInformation *> &list, BMenu *menu, uint32 menuMessage);

	void StartSong(uint16 newSong);
	void SwitchSubSong(char key);

	void StartTimers(void);
	void StopTimers(void);

	void UpdateTapeDeck(void);
	void UpdateTimes(void);
	void UpdateListCount(void);
	void UpdateListControls(void);

	void PrintInfo(void);
	void ShowNormalTitle(void);
	void ShowModuleName(void);

	void SetPosition(int16 position);
	void SetPositionTime(int16 position);

	void ShowModuleFilePanel(uint32 messageID, int32 buttonID);
	void ShowAPMLFilePanel(uint32 messageID);
	void ShowSaveAPMLFilePanel(void);
	void AddDirectoryToList(PString directory, BList &list);

	void AppendListFile(PString fileName, int32 index);
	void SaveAPMLFile(PString fileName);
	void RememberModuleList(void);

	void AppendMultiFile(PString fileName, int32 index = -1);
	void AppendMultiFile(PString fileName, BList &list);
	static bool MultiFileAdd(APMultiFiles::APMultiFileType *type, void *userData);
	static bool MultiFileSave(APMultiFiles::APMultiFileType *type, void *userData);

	static void SetMasterVolume(uintptr_t object, float vol);

	MainWindowSystem *windowSystem;
	APGlobalData *globalData;
	PResource *res;

	// Remember list variables. Only used when closing APlayer
	bool rememberFilled;
	int32 rememberSelected;
	int32 rememberPosition;
	int32 rememberSong;

	// Keyboard filter
	APKeyFilter *keyFilter;

	// Panels
	BMessenger *modMessenger;
	BFilePanel *modPanel;
	BMessenger *apmlMessenger;
	BFilePanel *apmlPanel;
	BMessenger *apmlSaveMessenger;
	BFilePanel *apmlSavePanel;
	entry_ref apmlLastEntry;

	// Timer variables
	TimeFormat timeFormat;
	PTimer *timer;
	PTimer *neverEndingTimer;
	PTime timeStart;
	PTimeSpan timeOccurred;
	PTimeSpan neverEndingTimeout;
	int16 prevPosition;

	// Module variables
	APWindowMainListItem *playItem;
	PTimeSpan songTotalTime;
	uint16 subSongMultiply;

	// List times
	PTimeSpan listTime;
	PTimeSpan selectedTime;

	// Window/Control status variables
	bool posSliderUpdate;
	bool neverEndingUpdate;
	bool showName;

	// Cursor variables
	BCursor *forbiddenCursor;
	BCursor *appendCursor;
	BCursor *insertCursor;
	BCursor *sleepCursor;

	// Misc.
	PList<int32> randomList;

	// Visual variables
	float fontHeight;

	BMenuBar *menuBar;
	BMenu *menuPlayers;
	BMenu *menuAgents;
	BMenu *menuClients;

	APBackView *topView;

	BBox *infoBox;
	APInfoView *infoView;
	APPictureButton *infoBut;

	APPictureButton *muteBut;
	APVolSlider *volSlider;
	BView* volSliderLayout;

	APWindowMainList *modList;
	BScrollView *modScrollView;

	BBox *listBox;
	APPictureButton *addBut;
	APPictureButton *removeBut;
	APPictureButton *swapBut;
	APPictureButton *sortBut;
	APPictureButton *upBut;
	APPictureButton *downBut;
	APPictureButton *selectBut;
	APDiskButton *diskBut;

	BBox *timeBox;
	BStringView *timeView;
	BStringView *totalView;

	APPosSlider *posSlider;

	BBox *tapeBox;
	APPictureButton *prevModBut;
	APPictureButton *prevSongBut;
	APPictureButton *rewBut;
	APPictureButton *playBut;
	APPictureButton *fwdBut;
	APPictureButton *nextSongBut;
	APPictureButton *nextModBut;
	APPictureButton *ejectBut;
	APPictureButton *pauseBut;

	BBox *loopSampleBox;
	APPictureButton *modLoopBut;
	APPictureButton *sampleBut;
};

#endif
