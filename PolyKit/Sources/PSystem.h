/******************************************************************************/
/* PSystem header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PSystem_h
#define __PSystem_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "ImportExport.h"


/******************************************************************************/
/* PSystem class                                                              */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PSystem
{
public:
	enum POperativeSystem
	{
		// Numbers that can be returned by the GetOSVersion() function
		pBeOS_ppc					= 0x00000101,
		pBeOS_x86					= 0x00000102,

		// Mask numbers
		pmBeOS						= 0x00000100
	};

	enum PLocaleString
	{
		pLocale_LongDate			= 0x00000001,	// Long date in user format
		pLocale_ShortDate			= 0x00000002,	// Short date in user format
		pLocale_Time				= 0x00000003,	// Time in user format

		pLocale_Decimal				= 0x0000000E,	// Decimal separator
		pLocale_Thousand			= 0x0000000F,	// Thousand separator

		pLocale_1159				= 0x00000028,	// AM designator
		pLocale_2359				= 0x00000029,	// PM designator

		pLocale_DayName1			= 0x0000002A,	// Long name for Monday
		pLocale_DayName2			= 0x0000002B,	// Long name for Tuesday
		pLocale_DayName3			= 0x0000002C,	// Long name for Wednesday
		pLocale_DayName4			= 0x0000002D,	// Long name for Thursday
		pLocale_DayName5			= 0x0000002E,	// Long name for Friday
		pLocale_DayName6			= 0x0000002F,	// Long name for Saturday
		pLocale_DayName7			= 0x00000030,	// Long name for Sunday

		pLocale_AbbrevDayName1		= 0x00000031,	// Abbreviated name for Monday
		pLocale_AbbrevDayName2		= 0x00000032,	// Abbreviated name for Tuesday
		pLocale_AbbrevDayName3		= 0x00000033,	// Abbreviated name for Wednesday
		pLocale_AbbrevDayName4		= 0x00000034,	// Abbreviated name for Thursday
		pLocale_AbbrevDayName5		= 0x00000035,	// Abbreviated name for Friday
		pLocale_AbbrevDayName6		= 0x00000036,	// Abbreviated name for Saturday
		pLocale_AbbrevDayName7		= 0x00000037,	// Abbreviated name for Sunday

		pLocale_MonthName1			= 0x00000038,	// Long name for January
		pLocale_MonthName2			= 0x00000039,	// Long name for February
		pLocale_MonthName3			= 0x0000003A,	// Long name for March
		pLocale_MonthName4			= 0x0000003B,	// Long name for April
		pLocale_MonthName5			= 0x0000003C,	// Long name for May
		pLocale_MonthName6			= 0x0000003D,	// Long name for June
		pLocale_MonthName7			= 0x0000003E,	// Long name for July
		pLocale_MonthName8			= 0x0000003F,	// Long name for August
		pLocale_MonthName9			= 0x00000040,	// Long name for September
		pLocale_MonthName10			= 0x00000041,	// Long name for October
		pLocale_MonthName11			= 0x00000042,	// Long name for November
		pLocale_MonthName12			= 0x00000043,	// Long name for December

		pLocale_AbbrevMonthName1	= 0x00000044,	// Abbreviated name for January
		pLocale_AbbrevMonthName2	= 0x00000045,	// Abbreviated name for February
		pLocale_AbbrevMonthName3	= 0x00000046,	// Abbreviated name for March
		pLocale_AbbrevMonthName4	= 0x00000047,	// Abbreviated name for April
		pLocale_AbbrevMonthName5	= 0x00000048,	// Abbreviated name for May
		pLocale_AbbrevMonthName6	= 0x00000049,	// Abbreviated name for June
		pLocale_AbbrevMonthName7	= 0x0000004A,	// Abbreviated name for July
		pLocale_AbbrevMonthName8	= 0x0000004B,	// Abbreviated name for August
		pLocale_AbbrevMonthName9	= 0x0000004C,	// Abbreviated name for September
		pLocale_AbbrevMonthName10	= 0x0000004D,	// Abbreviated name for October
		pLocale_AbbrevMonthName11	= 0x0000004E,	// Abbreviated name for November
		pLocale_AbbrevMonthName12	= 0x0000004F	// Abbreviated name for December
	};

	static void PlayBeep(void);
	static void Sleep(uint32 milliSeconds);
	static uint32 Random(uint32 max);

	static uint32 ConvertOSError(uint32 osError);
	static PString GetErrorString(uint32 error);

	static POperativeSystem GetOSVersion(void);
	static PString GetSystemLocaleString(PLocaleString localeNum);

	static PResource polyResource;

protected:
	static bool rndInit;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
