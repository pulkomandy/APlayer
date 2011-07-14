/******************************************************************************/
/* SIDFile header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDFile_h
#define __SIDFile_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"
#include "PResource.h"

// APlayerKit headers
#include "APAddOns.h"	


/******************************************************************************/
/* Amiga icon structures                                                      */
/******************************************************************************/
typedef struct Border
{
	uint16 leftEdge;			// Initial offsets from the origin
	uint16 topEdge;
	uint8 frontPen;				// Pens numbers for rendering
	uint8 backPen;
	uint8 drawMode;				// Mode for rendering
	uint8 count;				// Number of XY pairs
	uint32 xy;					// int16 *xy: Vector coordinate pairs rel to leftTop
	uint32 nextBorder;			// Border *nextBorder: Pointer to any other Border too
} Border;



typedef struct Image
{
	uint16 leftEdge;			// Starting offset relative to some origin
	uint16 topEdge;
	uint16 width;				// Pixel size (though data is word-aligned)
	uint16 height;
	uint16 depth;				// >= 0, for images you create
	uint32 imageData;			// uint16 *imageData: Pointer to the actual word-aligned bits
	uint8 planePick;
	uint8 planeOnOff;
	uint32 nextImage;			// Image *nextImage
} Image;



typedef struct Gadget
{
	uint32 nextGadget;			// Gadget *nextGadget: Next gadget in the list
	uint16 leftEdge;			// "hit box" of gadget
	uint16 topEdge;
	uint16 width;				// "hit box" of gadget
	uint16 height;
	uint16 flags;				// See below for list of defines
	uint16 activation;
	uint16 gadgetType;			// See below for defines
	uint32 gadgetRender;		// Image *gadgetRender
	uint32 selectRender;		// Image *selectRender
	uint32 gadgetText;			// void *gadgetText
	uint32 mutualExclude;
	uint32 specialInfo;			// void *specialInfo
	uint16 gadgetID;
	uint32 userData;			// Pointer to general purpose user data
} Gadget;



typedef struct DiskObject
{
	uint16 magic;				// A magic number at the start of the file
	uint16 version;				// A version number, so we can change it
	Gadget gadget;				// A copy of in core gadget
	uint8 type;
	uint32 defaultTool;			// char *defaultTool
	uint32 toolTypes;			// char **toolTypes
	uint32 currentX;
	uint32 currentY;
	uint32 drawerData;			// char *drawerData
	uint32 toolWindow;			// char *toolWindow: Only applies to tools
	uint32 stackSize;			// Only applies to tools
} DiskObject;



// A magic number, not easily impersonated
#define WB_DISKMAGIC			0xE310

// Our current version number
#define WB_DISKVERSION			1

// Our current revision number
#define WB_DISKREVISION			1

// I only use the lower 8 bits of gadget.userData for the revision #
#define WB_DISKREVISIONMASK		0xFF

// The Workbench object types
#define	WB_DISK					1
#define	WB_DRAWER				2
#define	WB_TOOL					3
#define	WB_PROJECT				4
#define	WB_GARBAGE				5
#define	WB_DEVICE				6
#define	WB_KICK					7
#define	WB_APPICON				8

// --- gadget.flags values	---
// Combinations in these bits describe the highlight technique to be used
#define GFLG_GADGHIGHBITS		0x0003

// Complement the select box
#define GFLG_GADGHCOMP			0x0000

// Draw a box around the image
#define GFLG_GADGHBOX			0x0001

// Blast in this alternate image
#define GFLG_GADGHIMAGE			0x0002

// Don't highlight
#define GFLG_GADGHNONE			0x0003

// Set if gadgetRender and selectRender point to an Image structure,
// clear if they point to Border structures
#define GFLG_GADGIMAGE			0x0004



/******************************************************************************/
/* SIDFile class                                                              */
/******************************************************************************/
class SIDPlayer;

class SIDFile
{
public:
	SIDFile(PFile *file, PResource *resource, SIDPlayer *play);
	virtual ~SIDFile(void);

	ap_result TestIt(void);
	void FillInInfo(void);

	uint8 *modAdr;						// Address to the C64 data
	uint32 modLen;						// Length of the C64 data

	uint16 loadAdr;						// C64 address to load file to
	uint16 initAdr;						// C64 address of init subroutine
	uint16 playAdr;						// C64 address of play subroutine
	uint16 songs;						// Number of songs
	uint16 startSong;					// Start song (1-256)
	uint32 speed;						// Speed info (bit: 0 = 50 Hz, 1 = CIA 1 Timer A (default 60 Hz))
	PString name;						// Name of module
	PString author;						// The author
	PString copyright;					// Copyright string
	PString info[5];					// Extra information strings
	uint8 relocStartPage;
	uint8 relocPages;
	bool musPlay;						// True = Compute! mus player
	bool psidSpecific;
	uint8 clock;
	uint8 sidModel;

protected:
	enum SidType { apPSID, apMUS, apSID, apINFO };

	ap_result TestPSID(PFile *file);
	ap_result TestMUS(PFile *file);
	ap_result TestSID(PFile *file);
	ap_result TestINFO(PFile *file);

	uint32 ReadHex(PString line, int32 *nextPos);
	uint32 ReadDec(PString line, int32 *nextPos);

	SIDPlayer *player;
	PFile *curFile;
	PFile *descripFile;
	bool deleteDescrip;

	SidType type;
	PResource *res;
};

#endif
