/******************************************************************************/
/* Vorbis registry functions.                                                 */
/******************************************************************************/


// Player headers
#include "Registry.h"


// Seems like overkill now; the backend numbers will grow into
// the infostructure soon enough
extern Vorbis_Func_Floor			floor0_exportBundle;
extern Vorbis_Func_Floor			floor1_exportBundle;
extern Vorbis_Func_Residue			residue0_exportBundle;
extern Vorbis_Func_Residue			residue1_exportBundle;
extern Vorbis_Func_Residue			residue2_exportBundle;
extern Vorbis_Func_Mapping			mapping0_exportBundle;



Vorbis_Func_Floor *floorP[] =
{
	&floor0_exportBundle,
	&floor1_exportBundle
};



Vorbis_Func_Residue *residueP[] =
{
	&residue0_exportBundle,
	&residue1_exportBundle,
	&residue2_exportBundle
};



Vorbis_Func_Mapping *mappingP[] =
{
	&mapping0_exportBundle
};
