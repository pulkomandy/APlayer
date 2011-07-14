/******************************************************************************/
/* LimVar header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __LimVar_h
#define __LimVar_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"


/******************************************************************************/
/* LimVar class                                                               */
/******************************************************************************/
template<class T, int32 min, int32 max> class LimVar
{
public:
	LimVar(const T &val)
	{
		value = val;
	}

	LimVar(void)
	{
		value = 0;
	}

	T & operator = (const T &newVal)
	{
		if (newVal <= min)		// I know the equal sign isn't neccessary, but it's there to remove a compiler warning
			value = min;
		else if (newVal > max)
			value = max;
		else
			value = newVal;

		return (value);
	}

	operator T() const
	{
		return (value);
	}

protected:
	T value;
};

#endif
