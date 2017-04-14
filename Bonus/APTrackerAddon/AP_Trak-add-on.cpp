// AP_Trak-add-on.cpp
#include <add-ons/tracker/TrackerAddOn.h>
#include <app/Roster.h>
#include <storage/Directory.h>
#include <interface/Alert.h>
#include <support/String.h>

#include "AP_Trak-add-on.h"


/* ---- Individual functions ----- */

#ifdef __POWERPC__
	BMessage *ProcessTheRef(entry_ref ref, BMessage *msg);
#endif

BMessage *ProcessTheRef(entry_ref ref, BMessage *msg)
{
	BEntry	anEntry(&ref, true);
	
	if(anEntry.IsDirectory()) {
		BDirectory *dir = new BDirectory(&ref);		
		while(dir->GetNextRef(&ref) == B_OK) {	// Scan Dir
			anEntry.SetTo(&ref, true);
			if(anEntry.IsDirectory()) {			// Subdir found
				msg = ProcessTheRef(ref, msg);
			} else {
				msg->AddRef("refs", &ref);		// Add Ref to msg
			}
		}
		delete dir;
	} else if(anEntry.IsFile()) {
		msg->AddRef("refs", &ref);				// Add Ref to msg
	}

	return msg;
}


/* ---- Tracker add-on ----- */

void process_refs(entry_ref dir_ref, BMessage *message, void *)
{
	entry_ref	ref;
	status_t	roster_stat = B_ERROR;
	BMessage	*msg = new BMessage(B_REFS_RECEIVED);
	
	if(message->FindRef("refs", &ref) == B_OK)
	{			/* --- got a fileref --- */
		for( int32 i = 0; message->FindRef("refs", i, &ref) == B_OK; i++ ) {
			msg = ProcessTheRef(ref, msg);
		}
	} else {	/* --- got only a dirref ---*/
		msg = ProcessTheRef(dir_ref, msg);
	}

	roster_stat = be_roster->Launch(APP_SIG, msg, NULL);
	delete msg;
	
	if( (roster_stat != B_OK) && (roster_stat != B_ALREADY_RUNNING) ) {
		const char	*alerttext = "APlayer Tracker add-on:\nCan not launch APlayer! Please check if it is installed properly.\n";

		BAlert		*alert = new BAlert("Add-On Error", alerttext, "Damn!", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
	}
}


/* ----- BApplication ----- */

int main( void )
{
	APAddOn *theApp;

    theApp = new APAddOn();
    theApp->Run();
	
    delete(theApp);
    return(0);
}


APAddOn::APAddOn( void ):BApplication(ADD_ON_SIG)
{
	int32		areply;
	BString		text = "Tracker add-on for APlayer.\n";
	
	/* --- Prepare the text to display --- */
	text << "Version " << APP_VERSION << "\n";
	text << "Built on " << __DATE__ << " for BeOS ";
	text << (int)(B_BEOS_VERSION >> 8) << "." << (int)((B_BEOS_VERSION >> 4) & 0xf) << "." << (int)(B_BEOS_VERSION & 0xf);
	#ifdef __INTEL__
		text << " x86\n\n";
	#elif defined __POWERPC__
		text << " PPC\n\n";
	#endif
	text << "It can be used for adding items to the playlist.\n";
	text << "To install, please move it to \"/boot/home/config/add-ons/Tracker\".\n\n";
	text << "This add-on can not be used without the installed main APlayer application.\n";
	
	/* --- Display alert --- */
	BAlert		*alert = new BAlert("APlayer add-on", text.String(), "What's a Tracker add-on??", "OK", NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert->SetShortcut(1, B_ESCAPE);
	areply = alert->Go();
	
	if(areply == 0) {
		BMessage	msg(B_ARGV_RECEIVED);
		msg.AddString("argv", "app");
		msg.AddString("argv", T_ADDON_HELP);
		msg.AddInt32("argc", 2);
		be_roster->Launch("text/html", &msg, NULL);
	}
	
	/* --- Quit BeApp --- */
  	be_app->PostMessage(B_QUIT_REQUESTED);
}


APAddOn::~APAddOn( void )
{

}
