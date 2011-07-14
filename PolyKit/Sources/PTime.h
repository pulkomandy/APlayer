/******************************************************************************/
/* PTime header file.                                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PTime_h
#define __PTime_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "ImportExport.h"


/******************************************************************************/
/* PTimeSpan class                                                            */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PTimeSpan
{
public:
	PTimeSpan(void);
	PTimeSpan(int64 timeSpan);
	PTimeSpan(int32 days, int32 hours, int32 minutes, int32 seconds, int32 milliSeconds = 0);
	PTimeSpan(const PTimeSpan &timeSpan);
	virtual ~PTimeSpan(void);

	void SetTimeSpan(int64 timeSpan);
	void SetTimeSpan(int32 days, int32 hours, int32 minutes, int32 seconds, int32 milliSeconds = 0);
	void SetTimeSpan(const PTimeSpan &timeSpan);

	int32 GetDays(void) const;
	int32 GetHours(void) const;
	int32 GetMinutes(void) const;
	int32 GetSeconds(void) const;
	int32 GetMilliSeconds(void) const;

	int64 GetTotalHours(void) const;
	int64 GetTotalMinutes(void) const;
	int64 GetTotalSeconds(void) const;
	int64 GetTotalMilliSeconds(void) const;

	void AddDays(int32 days);
	void AddHours(int32 hours);
	void AddMinutes(int32 minutes);
	void AddSeconds(int32 seconds);
	void AddMilliSeconds(int32 milliSeconds);

	inline void SubDays(int32 days) { AddDays(-days); };
	inline void SubHours(int32 hours) { AddHours(-hours); };
	inline void SubMinutes(int32 minutes) { AddMinutes(-minutes); };
	inline void SubSeconds(int32 seconds) { AddSeconds(-seconds); };
	inline void SubMilliSeconds(int32 milliSeconds) { AddMilliSeconds(-milliSeconds); };

	PString Format(PString format) const;
	PString Format(PResource *resource, int32 strNum) const;

	// Overloadings
	const PTimeSpan & operator = (const PTimeSpan &timeSpan);

	PTimeSpan operator + (const PTimeSpan &timeSpan) const;
	PTimeSpan operator - (const PTimeSpan &timeSpan) const;

	const PTimeSpan & operator += (const PTimeSpan &timeSpan);
	const PTimeSpan & operator -= (const PTimeSpan &timeSpan);

	bool operator == (const PTimeSpan &timeSpan) const;
	bool operator != (const PTimeSpan &timeSpan) const;
	bool operator < (const PTimeSpan &timeSpan) const;
	bool operator > (const PTimeSpan &timeSpan) const;
	bool operator <= (const PTimeSpan &timeSpan) const;
	bool operator >= (const PTimeSpan &timeSpan) const;

protected:
	int64 curTimeSpan;
};



/******************************************************************************/
/* PTime class                                                                */
/******************************************************************************/
class _IMPEXP_PKLIB PTime
{
public:
	enum PTimeFormat
	{
		pAll				= 0x00000000,	// These values can be OR'ed together
		pNoSeconds			= 0x00000001,
		pNoMinutesOrSeconds	= 0x00000002,
		pNoTimeMarker		= 0x00000004
	};

	PTime(void);
	PTime(int32 newDay, int32 newMonth, int32 newYear, int32 newHour, int32 newMinute, int32 newSecond, int32 newMilliSeconds = 0);
	PTime(const PTime &time);
	virtual ~PTime(void);

	void Reset(void);
	bool IsValid(void) const;
	bool IsLeapYear(void);

	void SetTime(int32 newDay, int32 newMonth, int32 newYear, int32 newHour, int32 newMinute, int32 newSecond, int32 newMilliSeconds = 0);
	void SetTime(int32 time);
	void SetTime(const PTime &time);
	void SetTime(const struct tm &time);
	void SetTime(const bigtime_t &time);

	static PTime GetNow(void);
	void SetToNow(void);
	void SetToMidnight(void);

	void SetDay(int32 newDay);
	void SetMonth(int32 newMonth);
	void SetYear(int32 newYear);
	void SetHour(int32 newHour);
	void SetMinute(int32 newMinute);
	void SetSecond(int32 newSecond);
	void SetMilliSeconds(int32 newMilliSeconds);

	int32 GetDay(void);
	int32 GetMonth(void);
	int32 GetYear(void);
	int32 GetHour(void);
	int32 GetMinute(void);
	int32 GetSecond(void);
	int32 GetMilliSeconds(void);

	PTimeSpan GetTimeSpanSinceEpoch(void);
	PTimeSpan GetTimeDifference(PTime &time);

	int16 GetDayOfWeek(void);
	int16 GetDayOfYear(void);
	int16 GetDaysInMonth(void);
	int16 GetDaysInYear(void);

	double GetJulianDay(void);
	int32 GetJulianTime(void);

	void AddYears(int32 addYears);
	void AddMonths(int32 addMonths);
	void AddDays(int32 addDays);
	void AddHours(int32 addHours);
	void AddMinutes(int32 addMinutes);
	void AddSeconds(int32 addSeconds);
	void AddMilliSeconds(int32 addMilliSeconds);

	void SubYears(int32 subYears);
	void SubMonths(int32 subMonths);
	inline void SubDays(int32 subDays) { AddDays(-subDays); };
	inline void SubHours(int32 subHours) { AddHours(-subHours); };
	inline void SubMinutes(int32 subMinutes) { AddMinutes(-subMinutes); };
	inline void SubSeconds(int32 subSeconds) { AddSeconds(-subSeconds); };
	inline void SubMilliSeconds(int32 subMilliSeconds) { AddMilliSeconds(-subMilliSeconds); };

	PString MakeStringFromTime(void);
	bool MakeTimeFromString(PString dateStr);

	PString Format(PString format);
	PString Format(PResource *resource, int32 strNum);
	PString GetUserDate(bool longDate);
	PString GetUserTime(uint32 format = pAll);

	static int32 GetLocalTimeDifference(void);

	// Overloadings
	const PTime & operator = (const PTime &time);
	const PTime & operator = (const struct tm &time);
	operator struct tm (void);

	bool operator == (const PTime &time) const;
	bool operator != (const PTime &time) const;
	bool operator < (const PTime &time) const;
	bool operator <= (const PTime &time) const;
	bool operator > (const PTime &time) const;
	bool operator >= (const PTime &time) const;

	PTime operator + (const PTimeSpan &timeSpan) const;
	PTime operator - (const PTimeSpan &timeSpan) const;

	const PTime & operator += (const PTimeSpan &timeSpan);
	const PTime & operator -= (const PTimeSpan &timeSpan);

	PTimeSpan operator - (const PTime &time) const;

protected:
	enum State { pJulian, pValue, pBoth };

	void SetVariable(int32 &var, int32 value);
	void JulianToValue(void);
	void ValueToJulian(void);
	void Normalize(void);
	void PrepareTimeCheck(PTime &time1, PTime &time2) const;

	double julianDay;
	int32 julianTime;

	int32 day;
	int32 month;
	int32 year;

	int32 hour;
	int32 minute;
	int32 second;
	int32 milliSeconds;

	State state;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
