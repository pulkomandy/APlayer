/******************************************************************************/
/* SIDViewFormular header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDViewFormular_h
#define __SIDViewFormular_h

// PolyKit headers
#include "POS.h"
#include "PResource.h"


/******************************************************************************/
/* SIDViewFormular class                                                      */
/******************************************************************************/
class SIDViewFormular : public BView
{
public:
	SIDViewFormular(PResource *resource, float par1, float par2, float par3);
	virtual ~SIDViewFormular(void);

	void UpdateCurve(float par1, float par2, float par3);

	virtual void GetPreferredSize(float *width, float *height);

protected:
	virtual void Draw(BRect updateRect);

	PResource *res;

	float filterFs;
	float filterFm;
	float filterFt;
};

#endif
