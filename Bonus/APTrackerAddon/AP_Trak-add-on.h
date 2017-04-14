// AP_Trak-add-on.h
#ifndef AP_TRAK_ADDON_H
#define AP_TRAK_ADDON_H

#include <app/Application.h>

#define APP_VERSION		"1.0.3"

#define APP_SIG			"application/x-vnd.Polycode.APlayer"
#define ADD_ON_SIG		"application/x-vnd.Polycode.APlayer-add-on"
#define T_ADDON_HELP	"file:///boot/beos/documentation/User's%20Guide/02_advanced/Advanced09_AddOns.html"

class APAddOn : public BApplication {

	public:
				APAddOn( void );
				~APAddOn( void );
};

#endif
