#include <Be.h>

int main(void)
{
	BFile file;
	BAppFileInfo appInfo;
	BMessage types;

	// Open the file
	file.SetTo("../APlayer/Server/BeOS/Server_x86.rsrc", B_READ_WRITE);

	// Set the app info
	appInfo.SetTo(&file);

	// Build supported types message
	types.AddString("types", "audio/x-669");		// 669 Tracker Module
	types.AddString("types", "audio/x-16sv");		// Amiga IFF 16-bit Sound File
	types.AddString("types", "audio/x-8svx");		// Amiga IFF Sound File
	types.AddString("types", "audio/x-ahx");		// AHX Module
	types.AddString("types", "audio/x-aiff");		// AudioIFF Sound File
	types.AddString("types", "audio/x-psid");		// C64 SID File
	types.AddString("types", "audio/x-dsm");		// DSIK Module
	types.AddString("types", "audio/x-amf");		// DSMI Module
	types.AddString("types", "audio/x-far");		// Farandole Module
	types.AddString("types", "audio/x-xm");			// FastTracker II Module
	types.AddString("types", "audio/x-fred");		// Fred Editor Module
	types.AddString("types", "audio/x-fc");			// Future Composer Module
	types.AddString("types", "audio/x-gdm");		// General DigiMusic Module
	types.AddString("types", "audio/x-imf");		// Imago Orpheus Module
	types.AddString("types", "audio/x-it");			// ImpulseTracker Module
	types.AddString("types", "audio/x-jam");		// JamCracker Module
	types.AddString("types", "audio/x-med");		// MED/OctaMED/OctaMED Professional/OctaMED Sound Studio
	types.AddString("types", "audio/x-wav");		// Microsoft .wav Sound File
	types.AddString("types", "audio/x-mikmod");		// MikMod Module
	types.AddString("types", "audio/x-mod");		// MOD Sound Module
	types.AddString("types", "audio/x-mpeg");		// MPEG Audio Format
	types.AddString("types", "audio/x-mtm");		// MultiTracker Module
	types.AddString("types", "audio/x-vorbis");		// OggVorbis Audio Format
	types.AddString("types", "audio/x-okt");		// Oktalyzer Module
	types.AddString("types", "audio/x-sawteeth");	// Sawteeth Module
	types.AddString("types", "audio/x-stm");		// ScreamTracker 2.x Module
	types.AddString("types", "audio/x-s3m");		// ScreamTracker 3.x Module
	types.AddString("types", "audio/x-sfx");		// SoundFX Module
	types.AddString("types", "audio/x-bp");			// SoundMonitor Module
	types.AddString("types", "audio/x-stx");		// STMIK Module
	types.AddString("types", "audio/x-tfmx");		// TFMX Module
	types.AddString("types", "audio/x-ult");		// UltraTracker Module
	types.AddString("types", "audio/x-umx");		// Unreal / Unreal Tournament Module

	// Types we already have, but with different mime-strings
	types.AddString("types", "audio/aiff");
	types.AddString("types", "audio/mpeg");
	types.AddString("types", "audio/riff");
	types.AddString("types", "audio/wav");
	types.AddString("types", "audio/x-riff");
	types.AddString("types", "audio/x-uni");

	// Different play list types 
	types.AddString("types", "text/x-aplayer-playlist");

	// Write the new resource
	appInfo.SetSupportedTypes(&types);

	return (0);
}
