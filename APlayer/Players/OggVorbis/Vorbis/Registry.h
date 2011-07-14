/******************************************************************************/
/* Registry header file.                                                      */
/******************************************************************************/


#ifndef __Registry_h
#define __Registry_h

// Player headers
#include "Backends.h"


#define VI_TRANSFORMB		1
#define VI_WINDOWB			1
#define VI_TIMEB			1
#define VI_FLOORB			2
#define VI_RESB				3
#define VI_MAPB				1

extern Vorbis_Func_Floor *floorP[];
extern Vorbis_Func_Residue *residueP[];
extern Vorbis_Func_Mapping *mappingP[];

#endif
