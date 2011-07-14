/******************************************************************************/
/* PTime implementation file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_POLYKIT_LIBRARY_

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSystem.h"
#include "PResource.h"
#include "PTime.h"
#include "PException.h"


/******************************************************************************/
/* PTimeSpan class                                                            */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PTimeSpan::PTimeSpan(void)
{
	// Clear the time span
	curTimeSpan = 0;
}



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "time" is the time span value to initialize the object with.       */
/******************************************************************************/
PTimeSpan::PTimeSpan(int64 timeSpan)
{
	SetTimeSpan(timeSpan);
}



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "days" is the number of days you want in the time span.            */
/*         "hours" is the number of hours you want in the time span.          */
/*         "minutes" is the number of minutes you want in the time span.      */
/*         "seconds" is the number of seconds you want in the time span.      */
/*         "milliSeconds" is the number of milliseconds you want in the time  */
/*         span.                                                              */
/******************************************************************************/
PTimeSpan::PTimeSpan(int32 days, int32 hours, int32 minutes, int32 seconds, int32 milliSeconds)
{
	SetTimeSpan(days, hours, minutes, seconds, milliSeconds);
}



/******************************************************************************/
/* Copy constructor                                                           */
/*                                                                            */
/* Input:  "timeSpan" is the time span to copy.                               */
/******************************************************************************/
PTimeSpan::PTimeSpan(const PTimeSpan &timeSpan)
{
	SetTimeSpan(timeSpan);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PTimeSpan::~PTimeSpan(void)
{
}



/******************************************************************************/
/* SetTimeSpan() sets the time span to the number of milliseconds given.      */
/*                                                                            */
/* Input:  "time" is the time span value you want to set the time span to     */
/******************************************************************************/
void PTimeSpan::SetTimeSpan(int64 timeSpan)
{
	// Copy the time span
	curTimeSpan = timeSpan;
}



/******************************************************************************/
/* SetTimeSpan() sets the time span to the arguments given.                   */
/*                                                                            */
/* Input:  "days" is the number of days you want in the time span.            */
/*         "hours" is the number of hours you want in the time span.          */
/*         "minutes" is the number of minutes you want in the time span.      */
/*         "seconds" is the number of seconds you want in the time span.      */
/*         "milliSeconds" is the number of milliseconds you want in the time  */
/*         span.                                                              */
/******************************************************************************/
void PTimeSpan::SetTimeSpan(int32 days, int32 hours, int32 minutes, int32 seconds, int32 milliSeconds)
{
	// Calculate the time span
	curTimeSpan = ((int64)days * 24 * 60 * 60 * 1000) + (hours * 60 * 60 * 1000) + (minutes * 60 * 1000) + (seconds * 1000) + milliSeconds;
}



/******************************************************************************/
/* SetTimeSpan() copies the time span given into the current time span.       */
/*                                                                            */
/* Input:  "timeSpan" is the time span to copy.                               */
/******************************************************************************/
void PTimeSpan::SetTimeSpan(const PTimeSpan &timeSpan)
{
	// Copy the time span
	curTimeSpan = timeSpan.curTimeSpan;
}



/******************************************************************************/
/* GetDays() returns the number of whole days in the time span.               */
/*                                                                            */
/* Output: The number of whole days.                                          */
/******************************************************************************/
int32 PTimeSpan::GetDays(void) const
{
	return ((int32)(curTimeSpan / (24 * 60 * 60 * 1000)));
}



/******************************************************************************/
/* GetHours() returns the number of hours of the current day.                 */
/*                                                                            */
/* Output: The number of hours in the current day, ranging from -23 to 23.    */
/******************************************************************************/
int32 PTimeSpan::GetHours(void) const
{
	return ((int32)(GetTotalHours() - (GetDays() * 24)));
}



/******************************************************************************/
/* GetMinutes() returns the number of minutes of the current day.             */
/*                                                                            */
/* Output: The number of minutes in the current day, ranging from -59 to 59.  */
/******************************************************************************/
int32 PTimeSpan::GetMinutes(void) const
{
	return ((int32)(GetTotalMinutes() - (GetTotalHours() * 60)));
}



/******************************************************************************/
/* GetSeconds() returns the number of seconds of the current day.             */
/*                                                                            */
/* Output: The number of seconds in the current day, ranging from -59 to 59.  */
/******************************************************************************/
int32 PTimeSpan::GetSeconds(void) const
{
	return ((int32)(GetTotalSeconds() - (GetTotalMinutes() * 60)));
}



/******************************************************************************/
/* GetMilliSeconds() returns the number of milliseconds of the current day.   */
/*                                                                            */
/* Output: The number of milliseconds in the current day, ranging from -999   */
/*         to 999.                                                            */
/******************************************************************************/
int32 PTimeSpan::GetMilliSeconds(void) const
{
	return ((int32)(GetTotalMilliSeconds() - (GetTotalSeconds() * 1000)));
}



/******************************************************************************/
/* GetTotalHours() returns the total number of hours in the time span.        */
/*                                                                            */
/* Output: The total number of hours.                                         */
/******************************************************************************/
int64 PTimeSpan::GetTotalHours(void) const
{
	return (curTimeSpan / (60 * 60 * 1000));
}



/******************************************************************************/
/* GetTotalMinutes() returns the total number of minutes in the time span.    */
/*                                                                            */
/* Output: The total number of minutes.                                       */
/******************************************************************************/
int64 PTimeSpan::GetTotalMinutes(void) const
{
	return (curTimeSpan / (60 * 1000));
}



/******************************************************************************/
/* GetTotalSeconds() returns the total number of seconds in the time span.    */
/*                                                                            */
/* Output: The total number of seconds.                                       */
/******************************************************************************/
int64 PTimeSpan::GetTotalSeconds(void) const
{
	return (curTimeSpan / 1000);
}



/******************************************************************************/
/* GetTotalMilliSeconds() returns the total number of milliseconds in the     */
/*      time span.                                                            */
/*                                                                            */
/* Output: The total number of milliseconds.                                  */
/******************************************************************************/
int64 PTimeSpan::GetTotalMilliSeconds(void) const
{
	return (curTimeSpan);
}



/******************************************************************************/
/* AddDays() adds the number of days given to the time span.                  */
/*                                                                            */
/* Input:  "days" is the number of days to add.                               */
/******************************************************************************/
void PTimeSpan::AddDays(int32 days)
{
	curTimeSpan += ((int64)days * 24 * 60 * 60 * 1000);
}



/******************************************************************************/
/* AddHours() adds the number of hours given to the time span.                */
/*                                                                            */
/* Input:  "hours" is the number of hours to add.                             */
/******************************************************************************/
void PTimeSpan::AddHours(int32 hours)
{
	curTimeSpan += ((int64)hours * 60 * 60 * 1000);
}



/******************************************************************************/
/* AddMinutes() adds the number of minutes given to the time span.            */
/*                                                                            */
/* Input:  "minutes" is the number of minutes to add.                         */
/******************************************************************************/
void PTimeSpan::AddMinutes(int32 minutes)
{
	curTimeSpan += ((int64)minutes * 60 * 1000);
}



/******************************************************************************/
/* AddSeconds() adds the number of seconds given to the time span.            */
/*                                                                            */
/* Input:  "seconds" is the number of seconds to add.                         */
/******************************************************************************/
void PTimeSpan::AddSeconds(int32 seconds)
{
	curTimeSpan += ((int64)seconds * 1000);
}



/******************************************************************************/
/* AddMilliSeconds() adds the number of milliseconds given to the time span.  */
/*                                                                            */
/* Input:  "milliSeconds" is the number of milliseconds to add.               */
/******************************************************************************/
void PTimeSpan::AddMilliSeconds(int32 milliSeconds)
{
	curTimeSpan += milliSeconds;
}



/******************************************************************************/
/* Format() creates a string with the information in the time span.           */
/*                                                                            */
/* Input:  "format" is the string to use to format the time span.             */
/*                                                                            */
/* Output: The formatted string.                                              */
/******************************************************************************/
PString PTimeSpan::Format(PString format) const
{
	int32 i, count;
	PString tempStr;
	PString retStr;

	// Add some extra spaces, so we don't get outside the string
	// array in some cases
	format += "    ";

	// Get the length of the string
	count = format.GetLength() - 4;

	for (i = 0; i < count; i++)
	{
		if (format.GetAt(i) == '%')
		{
			i++;		// Skip the percent character

			// Check the character
			switch (format.GetAt(i).GetChar()[0])
			{
				//
				// Total days
				//
				case 'D':
				{
					tempStr.Format("%d", GetDays());
					break;
				}

				//
				// Hours in current day
				//
				case 'H':
				{
					if (format.GetAt(i + 1) == 'H')
					{
						// With leading 0
						tempStr.Format("%02d", GetHours());
						i++;
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", GetHours());
					}
					break;
				}

				//
				// Minutes in current day
				//
				case 'm':
				{
					if (format.GetAt(i + 1) == 'm')
					{
						// With leading 0
						tempStr.Format("%02d", GetMinutes());
						i++;
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", GetMinutes());
					}
					break;
				}

				//
				// Seconds in current day
				//
				case 's':
				{
					if (format.GetAt(i + 1) == 's')
					{
						// With leading 0
						tempStr.Format("%02d", GetSeconds());
						i++;
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", GetSeconds());
					}
					break;
				}

				//
				// Milliseconds in current day
				//
				case 'i':
				{
					if ((format.GetAt(i + 1) == 'i') && (format.GetAt(i + 2) == 'i'))
					{
						// With leading 0's
						tempStr.Format("%03d", GetMilliSeconds());
						i += 2;
					}
					else
					{
						// Without leading 0's
						tempStr.Format("%d", GetMilliSeconds());
					}
					break;
				}

				//
				// Just copy the character
				//
				default:
				{
					tempStr = format.GetAt(i);
					break;
				}
			}
		}
		else
			tempStr = format.GetAt(i);

		// Add the character or formatted string to the result
		retStr += tempStr;
	}

	return (retStr);
}



/******************************************************************************/
/* Format() creates a string with the information in the time span.           */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource object to use.             */
/*         "strNum" is the resource string number.                            */
/*                                                                            */
/* Output: The formatted string.                                              */
/******************************************************************************/
PString PTimeSpan::Format(PResource *resource, int32 strNum) const
{
	PString str;

	str.LoadString(resource, strNum);
	return (Format(str));
}



/******************************************************************************/
/* operator = (PTimeSpan &) will set the time span to the time span given.    */
/*                                                                            */
/* Input:  "timeSpan" is the new time span.                                   */
/*                                                                            */
/* Output: The pointer to the new time span.                                  */
/******************************************************************************/
const PTimeSpan & PTimeSpan::operator = (const PTimeSpan &timeSpan)
{
	SetTimeSpan(timeSpan);
	return (*this);
}



/******************************************************************************/
/* operator + (PTimeSpan &) will add the time span given to the current one   */
/*      and return the new time span.                                         */
/*                                                                            */
/* Input:  "timeSpan" is the time span to add.                                */
/*                                                                            */
/* Output: The new time span.                                                 */
/******************************************************************************/
PTimeSpan PTimeSpan::operator + (const PTimeSpan &timeSpan) const
{
	return (PTimeSpan(curTimeSpan + timeSpan.curTimeSpan));
};



/******************************************************************************/
/* operator - (PTimeSpan &) will subtract the time span given from the        */
/*      current one and return the new time span.                             */
/*                                                                            */
/* Input:  "timeSpan" is the time span to subtract.                           */
/*                                                                            */
/* Output: The new time span.                                                 */
/******************************************************************************/
PTimeSpan PTimeSpan::operator - (const PTimeSpan &timeSpan) const
{
	return (PTimeSpan(curTimeSpan - timeSpan.curTimeSpan));
};



/******************************************************************************/
/* operator += (PTimeSpan &) will add the time span given to the current one. */
/*                                                                            */
/* Input:  "timeSpan" is the time span to add.                                */
/*                                                                            */
/* Output: The pointer to the current time span.                              */
/******************************************************************************/
const PTimeSpan & PTimeSpan::operator += (const PTimeSpan &timeSpan)
{
	curTimeSpan += timeSpan.curTimeSpan;
	return (*this);
};



/******************************************************************************/
/* operator -= (PTimeSpan &) will subtract the time span given to the current */
/*      one.                                                                  */
/*                                                                            */
/* Input:  "timeSpan" is the time span to subtract.                           */
/*                                                                            */
/* Output: The pointer to the current time span.                              */
/******************************************************************************/
const PTimeSpan & PTimeSpan::operator -= (const PTimeSpan &timeSpan)
{
	curTimeSpan -= timeSpan.curTimeSpan;
	return (*this);
};



/******************************************************************************/
/* operator == (PTimeSpan &) will compare the time span given with the        */
/*      current one.                                                          */
/*                                                                            */
/* Input:  "timeSpan" is the time span to compare with.                       */
/*                                                                            */
/* Output: True if they are equal, false if not.                              */
/******************************************************************************/
bool PTimeSpan::operator == (const PTimeSpan &timeSpan) const
{
	return (curTimeSpan == timeSpan.curTimeSpan);
};



/******************************************************************************/
/* operator != (PTimeSpan &) will compare the time span given with the        */
/*      current one.                                                          */
/*                                                                            */
/* Input:  "timeSpan" is the time span to compare with.                       */
/*                                                                            */
/* Output: True if they are not equal, false if they are.                     */
/******************************************************************************/
bool PTimeSpan::operator != (const PTimeSpan &timeSpan) const
{
	return (curTimeSpan != timeSpan.curTimeSpan);
};



/******************************************************************************/
/* operator < (PTimeSpan &) will compare the time span given with the         */
/*      current one.                                                          */
/*                                                                            */
/* Input:  "timeSpan" is the time span to compare with.                       */
/*                                                                            */
/* Output: True if the current is < than the given one.                       */
/******************************************************************************/
bool PTimeSpan::operator < (const PTimeSpan &timeSpan) const
{
	return (curTimeSpan < timeSpan.curTimeSpan);
};



/******************************************************************************/
/* operator > (PTimeSpan &) will compare the time span given with the         */
/*      current one.                                                          */
/*                                                                            */
/* Input:  "timeSpan" is the time span to compare with.                       */
/*                                                                            */
/* Output: True if the current is > than the given one.                       */
/******************************************************************************/
bool PTimeSpan::operator > (const PTimeSpan &timeSpan) const
{
	return (curTimeSpan > timeSpan.curTimeSpan);
};



/******************************************************************************/
/* operator <= (PTimeSpan &) will compare the time span given with the        */
/*      current one.                                                          */
/*                                                                            */
/* Input:  "timeSpan" is the time span to compare with.                       */
/*                                                                            */
/* Output: True if the current is <= than the given one.                      */
/******************************************************************************/
bool PTimeSpan::operator <= (const PTimeSpan &timeSpan) const
{
	return (curTimeSpan <= timeSpan.curTimeSpan);
};



/******************************************************************************/
/* operator >= (PTimeSpan &) will compare the time span given with the        */
/*      current one.                                                          */
/*                                                                            */
/* Input:  "timeSpan" is the time span to compare with.                       */
/*                                                                            */
/* Output: True if the current is >= than the given one.                      */
/******************************************************************************/
bool PTimeSpan::operator >= (const PTimeSpan &timeSpan) const
{
	return (curTimeSpan >= timeSpan.curTimeSpan);
};





/******************************************************************************/
/* PTime class                                                                */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PTime::PTime(void)
{
	Reset();
}



/******************************************************************************/
/* Constructor with values                                                    */
/*                                                                            */
/* Input:  "newDay" is the day of the month.                                  */
/*         "newMonth" is the month.                                           */
/*         "newYear" is the year.                                             */
/*         "newHour" is the hour.                                             */
/*         "newMinute" is the minute.                                         */
/*         "newSecond" is the second.                                         */
/*         "newMilliSeconds" is the milliseconds.                             */
/******************************************************************************/
PTime::PTime(int32 newDay, int32 newMonth, int32 newYear, int32 newHour, int32 newMinute, int32 newSecond, int32 newMilliSeconds)
{
	SetTime(newDay, newMonth, newYear, newHour, newMinute, newSecond, newMilliSeconds);
}



/******************************************************************************/
/* Copy constructor                                                           */
/*                                                                            */
/* Input:  "time" is the time to copy.                                        */
/******************************************************************************/
PTime::PTime(const PTime &time)
{
	SetTime(time);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PTime::~PTime(void)
{
}



/******************************************************************************/
/* Reset() will reset the time to the first date.                             */
/******************************************************************************/
void PTime::Reset(void)
{
	// Set time to 0
	julianDay  = 0.0;
	julianTime = 0;
	state      = pJulian;
}



/******************************************************************************/
/* IsValid() checks the object context to see if the class has a valid time.  */
/*                                                                            */
/* Output: True for ok, false for error.                                      */
/******************************************************************************/
bool PTime::IsValid(void) const
{
	switch (state)
	{
		case pJulian:
		case pBoth:
			return (true);

		case pValue:
		{
			PTime time(*this);

			// Convert to Julian and back again
			time.ValueToJulian();
			time.JulianToValue();

			// Now check to see if the time values are equal
			if ((time.GetDay() == day) && (time.GetMonth() == month) && (time.GetYear() == year) &&
				(time.GetHour() == hour) && (time.GetMinute() == minute) && (time.GetSecond() == second) &&
				(time.GetMilliSeconds() == milliSeconds))
			{
				return (true);
			}
			break;
		}
	}

	return (false);
}



/******************************************************************************/
/* IsLeapYear() checks the year to see if it's a leap year.                   */
/*                                                                            */
/* Output: True if the year is a leap year, false if not.                     */
/******************************************************************************/
bool PTime::IsLeapYear(void)
{
	int16 temp;
	bool leap = false;

	// Make sure we have values
	JulianToValue();

	temp = year / 4;
	if ((temp * 4) == year)
	{
		leap = true;

		// Well, make an extra check.
		// If the year is a centurial year and isn't divisible by 400,
		// it's a common year
		temp = year / 100;
		if ((temp * 100) == year)
		{
			temp = year / 400;
			if ((temp * 400) != year)
				leap = false;
		}
	}

	return (leap);
}



/******************************************************************************/
/* SetTime() sets the object time to the values given.                        */
/*                                                                            */
/* Input:  "newDay" is the day of the month.                                  */
/*         "newMonth" is the month.                                           */
/*         "newYear" is the year.                                             */
/*         "newHour" is the hour.                                             */
/*         "newMinute" is the minute.                                         */
/*         "newSecond" is the second.                                         */
/*         "newMilliSeconds" is the milliseconds.                             */
/******************************************************************************/
void PTime::SetTime(int32 newDay, int32 newMonth, int32 newYear, int32 newHour, int32 newMinute, int32 newSecond, int32 newMilliSeconds)
{
	ASSERT((newDay >= 1) && (newDay <= 31));
	ASSERT((newMonth >= 1) && (newMonth <= 12));
	ASSERT(newYear >= -4712);
	ASSERT((newHour >= 0) && (newHour <= 23));
	ASSERT((newMinute >= 0) && (newMinute <= 59));
	ASSERT((newSecond >= 0) && (newSecond <= 59));
	ASSERT((newMilliSeconds >= 0) && (newMilliSeconds <= 999));

	// Set the date
	day          = newDay;
	month        = newMonth;
	year         = newYear;

	// Set the time
	hour         = newHour;
	minute       = newMinute;
	second       = newSecond;
	milliSeconds = newMilliSeconds;

	// Set state
	state = pValue;

	// Make sure we have a valid date
	ValueToJulian();
	state = pJulian;
}



/******************************************************************************/
/* SetTime() sets the object time to the number of seconds given from         */
/*      00:00:00 January 1, 1970.                                             */
/*                                                                            */
/* Input:  "time" is the number of seconds elapsed.                           */
/******************************************************************************/
void PTime::SetTime(int32 time)
{
	// Start to set the time to the start date
	SetTime(1, 1, 1970, 0, 0, 0, 0);

	// Add the number of seconds given
	AddSeconds(time);
}



/******************************************************************************/
/* SetTime() sets the object time to another PTime.                           */
/*                                                                            */
/* Input:  "time" is a reference to the PTime object.                         */
/******************************************************************************/
void PTime::SetTime(const PTime &time)
{
	julianDay    = time.julianDay;
	julianTime   = time.julianTime;

	day          = time.day;
	month        = time.month;
	year         = time.year;

	hour         = time.hour;
	minute       = time.minute;
	second       = time.second;
	milliSeconds = time.milliSeconds;

	state        = time.state;
}



/******************************************************************************/
/* SetTime() sets the object time to the context of a tm structure.           */
/*                                                                            */
/* Input:  "time" is a reference to a tm structure.                           */
/******************************************************************************/
void PTime::SetTime(const struct tm &time)
{
	ASSERT((time.tm_mday >= 1) && (time.tm_mday <= 31));
	ASSERT((time.tm_mon >= 0) && (time.tm_mon <= 11));
	ASSERT(time.tm_year >= 0);
	ASSERT((time.tm_hour >= 0) && (time.tm_hour <= 23));
	ASSERT((time.tm_min >= 0) && (time.tm_min <= 59));
	ASSERT((time.tm_sec >= 0) && (time.tm_sec <= 59));

	day          = time.tm_mday;
	month        = time.tm_mon + 1;
	year         = time.tm_year + 1900;

	hour         = time.tm_hour;
	minute       = time.tm_min;
	second       = time.tm_sec;
	milliSeconds = 0;

	// Set the state
	state = pValue;
}



/******************************************************************************/
/* SetTime() sets the object time to the contexts of a bigtime_t structure.   */
/*                                                                            */
/* Input:  "time" is a reference to the bigtime_t structure.                  */
/******************************************************************************/
void PTime::SetTime(const bigtime_t &time)
{
	int64 addMilli = time / 1000;

	// Start to set the time to the start date
	SetTime(1, 1, 1970, 0, 0, 0, 0);

	// Add the milliseconds in step of 2060000000's
	// The number is 0x7fffffff - one day of milliseconds round down.
	while (addMilli > 2060000000)
	{
		AddMilliSeconds(2060000000);
		addMilli -= 2060000000;
	}

	// Add the rest
	AddMilliSeconds(addMilli);
}



/******************************************************************************/
/* GetNow() returns a new object holding the current local time.              */
/*                                                                            */
/* Output: A object holding the current time.                                 */
/******************************************************************************/
PTime PTime::GetNow(void)
{
	PTime time;

	// Set the time
	time.SetToNow();

	// And return it
	return (time);
}



/******************************************************************************/
/* SetToNow() sets the object time to the current local time.                 */
/******************************************************************************/
void PTime::SetToNow(void)
{
	time_t currentTime;
	struct tm *localTime;

	// Get the current time
	currentTime = time(NULL);

	// Convert it to local time
	localTime = localtime(&currentTime);

	// Set the time in the object
	SetTime(*localTime);
}



/******************************************************************************/
/* SetToMidnight() will set the time part to midnight.                        */
/******************************************************************************/
void PTime::SetToMidnight(void)
{
	ValueToJulian();

	// Reset the time
	julianTime = 0;
	state      = pJulian;
}



/******************************************************************************/
/* SetDay() will change the day.                                              */
/*                                                                            */
/* Input:  "newDay" is the new day.                                           */
/******************************************************************************/
void PTime::SetDay(int32 newDay)
{
	ASSERT((newDay >= 1) && (newDay <= 31));

	SetVariable(day, newDay);
}



/******************************************************************************/
/* SetMonth() will change the month.                                          */
/*                                                                            */
/* Input:  "newMonth" is the new month.                                       */
/******************************************************************************/
void PTime::SetMonth(int32 newMonth)
{
	ASSERT((newMonth >= 1) && (newMonth <= 12));

	SetVariable(month, newMonth);
}



/******************************************************************************/
/* SetYear() will change the year.                                            */
/*                                                                            */
/* Input:  "newYear" is the new year.                                         */
/******************************************************************************/
void PTime::SetYear(int32 newYear)
{
	ASSERT(newYear >= -4712);

	SetVariable(year, newYear);
}



/******************************************************************************/
/* SetHour() will change the hour.                                            */
/*                                                                            */
/* Input:  "newHour" is the new hour.                                         */
/******************************************************************************/
void PTime::SetHour(int32 newHour)
{
	ASSERT((newHour >= 0) && (newHour <= 23));

	SetVariable(hour, newHour);
}



/******************************************************************************/
/* SetMinute() will change the minute.                                        */
/*                                                                            */
/* Input:  "newMinute" is the new minute.                                     */
/******************************************************************************/
void PTime::SetMinute(int32 newMinute)
{
	ASSERT((newMinute >= 0) && (newMinute <= 59));

	SetVariable(minute, newMinute);
}



/******************************************************************************/
/* SetSecond() will change the second.                                        */
/*                                                                            */
/* Input:  "newSecond" is the new second.                                     */
/******************************************************************************/
void PTime::SetSecond(int32 newSecond)
{
	ASSERT((newSecond >= 0) && (newSecond <= 59));

	SetVariable(second, newSecond);
}



/******************************************************************************/
/* SetMilliSeconds() will change the milliseconds.                            */
/*                                                                            */
/* Input:  "newMilliSeconds" is the new milliseconds.                         */
/******************************************************************************/
void PTime::SetMilliSeconds(int32 newMilliSeconds)
{
	ASSERT((newMilliSeconds >= 0) && (newMilliSeconds <= 999));

	SetVariable(milliSeconds, newMilliSeconds);
}



/******************************************************************************/
/* GetDay() returns the objects day.                                          */
/*                                                                            */
/* Output: The day (1-31).                                                    */
/******************************************************************************/
int32 PTime::GetDay(void)
{
	JulianToValue();

	return (day);
}



/******************************************************************************/
/* GetMonth() returns the objects month.                                      */
/*                                                                            */
/* Output: The month (1-12).                                                  */
/******************************************************************************/
int32 PTime::GetMonth(void)
{
	JulianToValue();

	return (month);
}



/******************************************************************************/
/* GetYear() returns the objects year.                                        */
/*                                                                            */
/* Output: The year.                                                          */
/******************************************************************************/
int32 PTime::GetYear(void)
{
	JulianToValue();

	return (year);
}



/******************************************************************************/
/* GetHour() returns the objects hour.                                        */
/*                                                                            */
/* Output: The hour (0-23).                                                   */
/******************************************************************************/
int32 PTime::GetHour(void)
{
	JulianToValue();

	return (hour);
}



/******************************************************************************/
/* GetMinute() returns the objects minute.                                    */
/*                                                                            */
/* Output: The minute (0-59).                                                 */
/******************************************************************************/
int32 PTime::GetMinute(void)
{
	JulianToValue();

	return (minute);
}



/******************************************************************************/
/* GetSecond() returns the objects second.                                    */
/*                                                                            */
/* Output: The second (0-59).                                                 */
/******************************************************************************/
int32 PTime::GetSecond(void)
{
	JulianToValue();

	return (second);
}



/******************************************************************************/
/* GetMilliSeconds() returns the objects milliseconds.                        */
/*                                                                            */
/* Output: The milliseconds (0-999).                                          */
/******************************************************************************/
int32 PTime::GetMilliSeconds(void)
{
	JulianToValue();

	return (milliSeconds);
}



/******************************************************************************/
/* GetTimeSpanSinceEpoch() returns the time span from                         */
/*      00:00:00 January 1, 1970.                                             */
/*                                                                            */
/* Output: Is the time span elapsed.                                          */
/******************************************************************************/
PTimeSpan PTime::GetTimeSpanSinceEpoch(void)
{
	PTime tempTime;

	tempTime.SetTime(1, 1, 1970, 0, 0, 0, 0);
	return (tempTime.GetTimeDifference(*this));
}



/******************************************************************************/
/* GetTimeDifference() will calculate the time span between the time you give */
/*      and the current time.                                                 */
/*                                                                            */
/* Input:  "time" is the end time to calculate from.                          */
/*                                                                            */
/* Output: Is the time span.                                                  */
/******************************************************************************/
PTimeSpan PTime::GetTimeDifference(PTime &time)
{
	double days;
	int32 milli;
	int64 difference;

	// First make sure we have Julian Day
	ValueToJulian();
	time.ValueToJulian();

	// Calculate the time difference
	days  = time.julianDay - julianDay;
	milli = time.julianTime - julianTime;

	// Calculate the number of milliseconds
	difference = (int64)days * 24 * 60 * 60 * 1000 + milli;

	return (PTimeSpan(difference));
}



/******************************************************************************/
/* GetDayOfWeek() returns the day of week.                                    */
/*                                                                            */
/* Output: The day of week (0 = Sunday, 1 = Monday ...).                      */
/******************************************************************************/
int16 PTime::GetDayOfWeek(void)
{
	double dow;

	// First make sure we have Julian Day
	ValueToJulian();

	// Calculate the day of week
	dow = fmod((julianDay + 1.5), 7);

	return ((int16)dow);
}



/******************************************************************************/
/* GetDayOfYear() returns the day of the year.                                */
/*                                                                            */
/* Output: The day of the year (0-365).                                       */
/******************************************************************************/
int16 PTime::GetDayOfYear(void)
{
	int16 doy, k;

	// Make sure we have values
	JulianToValue();

	// Calculate the day of the year
	k   = IsLeapYear() ? 1 : 2;
	doy = ((275 * month) / 9) - k * ((month + 9) / 12) + day - 30;

	return (doy);
}



/******************************************************************************/
/* GetDaysInMonth() returns the number of days in the month.                  */
/*                                                                            */
/* Output: The number of days in the month (28-31).                           */
/******************************************************************************/
int16 PTime::GetDaysInMonth(void)
{
	static const int16 monthDay[12] = { 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	// Make sure we have values
	JulianToValue();

	if (month == 2)
	{
		if (IsLeapYear())
			return (29);
		else
			return (28);
	}

	return (monthDay[month - 1]);
}



/******************************************************************************/
/* GetDaysInYear() returns the number of days in the year.                    */
/*                                                                            */
/* Output: The number of days in the year (365/366).                          */
/******************************************************************************/
int16 PTime::GetDaysInYear(void)
{
	if (IsLeapYear())
		return (366);

	return (365);
}



/******************************************************************************/
/* GetJulianDay() returns the Julian Day value.                               */
/*                                                                            */
/* Output: The Julian Day.                                                    */
/******************************************************************************/
double PTime::GetJulianDay(void)
{
	// First make sure we have Julian Day
	ValueToJulian();

	return (julianDay);
}



/******************************************************************************/
/* GetJulianTime() returns the Julian Time value.                             */
/*                                                                            */
/* Output: The Julian Time.                                                   */
/******************************************************************************/
int32 PTime::GetJulianTime(void)
{
	// First make sure we have Julian Day
	ValueToJulian();

	return (julianTime);
}



/******************************************************************************/
/* AddYears() will add x years to the objects time.                           */
/*                                                                            */
/* Input:  "addYears" is the number of years to add.                          */
/******************************************************************************/
void PTime::AddYears(int32 addYears)
{
	// Start to convert the intern variables if needed
	JulianToValue();

	// Add the years
	year += addYears;
	state = pValue;

	// Check to see if we got an invalid day (can only occure in February)
	if (GetDaysInMonth() < day)
	{
		// We do, so round up to the 1. in the next month
		day = 1;
		month++;
	}
}



/******************************************************************************/
/* AddMonths() will add x months to the objects time.                         */
/*                                                                            */
/* Input:  "addMonths" is the number of months to add.                        */
/******************************************************************************/
void PTime::AddMonths(int32 addMonths)
{
	// Start to convert the intern variables if needed
	JulianToValue();

	// Add the months
	month += addMonths;
	state  = pValue;

	// Check to see if we got out of range
	while (month > 12)
	{
		month -= 12;
		year++;
	}

	// Check to see if we got an invalid day
	if (GetDaysInMonth() < day)
	{
		// We do, so round up to the 1. in the next month
		day = 1;
		month++;

		// Check again if we got out of range
		if (month == 13)
		{
			month = 1;
			year++;
		}
	}
}



/******************************************************************************/
/* AddDays() will add x days to the objects time.                             */
/*                                                                            */
/* Input:  "addDays" is the number of days to add.                            */
/******************************************************************************/
void PTime::AddDays(int32 addDays)
{
	// Start to convert the intern variables if needed
	ValueToJulian();

	julianDay += addDays;
	state      = pJulian;
}



/******************************************************************************/
/* AddHours() will add x hours to the objects time.                           */
/*                                                                            */
/* Input:  "addHours" is the number of hours to add.                          */
/******************************************************************************/
void PTime::AddHours(int32 addHours)
{
	int32 addValue;

	// Start to convert the intern variables if needed
	ValueToJulian();

	// Find the value to add
	addValue = (addHours < 0 ? -570 : 570);

	// Add the hours in step of 570
	// The number is calculated this way:
	// (0x7fffffff - one day of milliseconds (86400000) / 1000 / 60 / 60)
	// and then round down
	while (abs(addHours) > 570)
	{
		julianTime += (addValue * 60 * 60 * 1000);
		addHours   -= addValue;
		Normalize();
	}

	// Add the rest
	julianTime += (addHours * 60 * 60 * 1000);
	Normalize();

	// Change the state
	state = pJulian;
}



/******************************************************************************/
/* AddMinutes() will add x minutes to the objects time.                       */
/*                                                                            */
/* Input:  "addMinutes" is the number of minutes to add.                      */
/******************************************************************************/
void PTime::AddMinutes(int32 addMinutes)
{
	int32 addValue;

	// Start to convert the intern variables if needed
	ValueToJulian();

	// Find the value to add
	addValue = (addMinutes < 0 ? -34000 : 34000);

	// Add the hours in step of 34000
	// The number is calculated this way:
	// (0x7fffffff - one day of milliseconds (86400000) / 1000 / 60)
	// and then round down
	while (abs(addMinutes) > 34000)
	{
		julianTime += (addValue * 60 * 1000);
		addMinutes -= addValue;
		Normalize();
	}

	// Add the rest
	julianTime += (addMinutes * 60 * 1000);
	Normalize();

	// Change the state
	state = pJulian;
}



/******************************************************************************/
/* AddSeconds() will add x seconds to the objects time.                       */
/*                                                                            */
/* Input:  "addSeconds" is the number of seconds to add.                      */
/******************************************************************************/
void PTime::AddSeconds(int32 addSeconds)
{
	int32 addValue;

	// Start to convert the intern variables if needed
	ValueToJulian();

	// Find the value to add
	addValue = (addSeconds < 0 ? -2060000 : 2060000);

	// Add the seconds in step of 2060000
	// The number is calculated this way:
	// (0x7fffffff - one day of milliseconds (86400000) / 1000)
	// and then round down
	while (abs(addSeconds) > 2060000)
	{
		julianTime += (addValue * 1000);
		addSeconds -= addValue;
		Normalize();
	}

	// Add the rest
	julianTime += (addSeconds * 1000);
	Normalize();

	// Change the state
	state = pJulian;
}



/******************************************************************************/
/* AddMilliSeconds() will add x milliseconds to the objects time.             */
/*                                                                            */
/* Input:  "addMilliSeconds" is the number of milliseconds to add.            */
/******************************************************************************/
void PTime::AddMilliSeconds(int32 addMilliSeconds)
{
	long addValue;

	// Start to convert the intern variables if needed
	ValueToJulian();

	// Find the add value
	addValue = (addMilliSeconds < 0) ? -2061080000 : 2061080000;

	// Add the number of milliseconds in step of 2061080000
	// The number is calculated this way:
	// (0x7fffffff - one day of milliseconds (86400000))
	// and then round down
	while (abs(addMilliSeconds) > 2061080000)
	{
		julianTime      += addValue;
		addMilliSeconds -= addValue;
		Normalize();
	}

	// Add the rest
	julianTime += addMilliSeconds;
	Normalize();

	// Change the state
	state = pJulian;
}



/******************************************************************************/
/* SubYears() will subtract x years from the objects time.                    */
/*                                                                            */
/* Input:  "subYears" is the number of years to subtract.                     */
/******************************************************************************/
void PTime::SubYears(int32 subYears)
{
	// Start to convert the intern variables if needed
	JulianToValue();

	// Subtract the years
	year -= subYears;
	state = pValue;

	// Check to see if we got an invalid day (can only occure in February)
	if (GetDaysInMonth() < day)
	{
		// We do, so round down to the last day in the current month
		day = GetDaysInMonth();
	}
}



/******************************************************************************/
/* SubMonths() will subtract x months from the objects time.                  */
/*                                                                            */
/* Input:  "subMonths" is the number of months to subtract.                   */
/******************************************************************************/
void PTime::SubMonths(int32 subMonths)
{
	// Start to convert the intern variables if needed
	JulianToValue();

	// Subtract the months
	month -= subMonths;
	state  = pValue;

	// Check to see if we got out of range
	while (month < 1)
	{
		month += 12;
		year--;
	}

	// Check to see if we got an invalid day
	if (GetDaysInMonth() < day)
	{
		// We do, so round down to the last day in the current month
		day = GetDaysInMonth();
	}
}



/******************************************************************************/
/* MakeStringFromTime() will create a PTime date string.                      */
/*                                                                            */
/* Output: A date string.                                                     */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PString PTime::MakeStringFromTime(void)
{
	return (Format("%yyyy %MM %dd %HH %mm %ss"));
}



/******************************************************************************/
/* MakeTimeFromString() will parse the string given and place the date into   */
/*      the PTime object.                                                     */
/*                                                                            */
/* Input:  "dateString" is the string to parse.                               */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool PTime::MakeTimeFromString(PString dateString)
{
	int32 i;
	int32 oldIndex = -1;
	int32 newIndex;
	PString number;
	int16 dateFields[5];

	for (i = 0; i < 5; i++)
	{
		// Get the field
		if ((newIndex = dateString.Find(' ', oldIndex)) == -1)
		{
			// Unexpected end of date
			return (false);
		}

		// Convert the number
		number        = dateString.Mid(oldIndex + 1, newIndex - oldIndex - 1);
		dateFields[i] = number.GetNumber();

		// Continue with the search
		oldIndex = newIndex;
	}

	// Set the date into the object
	SetYear(dateFields[0]);
	SetMonth(dateFields[1]);
	SetDay(dateFields[2]);
	SetHour(dateFields[3]);
	SetMinute(dateFields[4]);
	SetSecond(dateString.GetNumber(newIndex + 1));
	SetMilliSeconds(0);

	return (true);
}



/******************************************************************************/
/* Format() returns the objects date and/or time in the format of the format  */
/*      string.                                                               */
/*                                                                            */
/* Input:  "format" is the string to use to format the time.                  */
/*                                                                            */
/* Output: The formatted string.                                              */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PString PTime::Format(PString format)
{
	int32 i, count;
	int32 tempVar;
	PString tempStr;
	PString retStr;
	PSystem::PLocaleString type;

	// We need values, so convert if the time isn't in it
	JulianToValue();

	// Add some extra spaces, so we don't get outside the string
	// array in some cases
	format += "    ";

	// Get the length of the string
	count = format.GetLength() - 4;

	for (i = 0; i < count; i++)
	{
		if (format.GetAt(i) == '%')
		{
			i++;		// Skip the percent character

			switch (format.GetAt(i).GetChar()[0])
			{
				//
				// 12 hour clock
				//
				case 'h':
				{
					tempVar = hour % 12;
					if (format.GetAt(i + 1) == 'h')
					{
						// With leading 0
						tempStr.Format("%02d", tempVar);
						i++;
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", tempVar);
					}
					break;
				}

				//
				// 24 hour clock
				//
				case 'H':
				{
					if (format.GetAt(i + 1) == 'H')
					{
						// With leading 0
						tempStr.Format("%02d", hour);
						i++;
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", hour);
					}
					break;
				}

				//
				// Minutes
				//
				case 'm':
				{
					if (format.GetAt(i + 1) == 'm')
					{
						// With leading 0
						tempStr.Format("%02d", minute);
						i++;
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", minute);
					}
					break;
				}

				//
				// Seconds
				//
				case 's':
				{
					if (format.GetAt(i + 1) == 's')
					{
						// With leading 0
						tempStr.Format("%02d", second);
						i++;
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", second);
					}
					break;
				}

				//
				// Milliseconds
				//
				case 'i':
				{
					if ((format.GetAt(i + 1) == 'i') && (format.GetAt(i + 2) == 'i'))
					{
						// With leading 0's
						tempStr.Format("%03d", milliSeconds);
						i += 2;
					}
					else
					{
						// Without leading 0's
						tempStr.Format("%d", milliSeconds);
					}
					break;
				}

				//
				// Time marker
				//
				case 't':
				{
					if (hour < 12)
						type = PSystem::pLocale_1159;	// AM
					else
						type = PSystem::pLocale_2359;	// PM

					if (format.GetAt(i + 1) == 't')
					{
						// Full string
						tempStr = PSystem::GetSystemLocaleString(type);
						i++;
					}
					else
					{
						// Only first character
						PString str = PSystem::GetSystemLocaleString(type);
						tempStr = str.GetAt(0);
					}
					break;
				}

				//
				// Day
				//
				case 'd':
				{
					if (format.GetAt(i + 1) == 'd')
					{
						if (format.GetAt(i + 2) == 'd')
						{
							tempVar = GetDayOfWeek();
							if (tempVar == 0)
								tempVar = 7;

							if (format.GetAt(i + 3) == 'd')
							{
								// Day of the week full string
								tempStr = PSystem::GetSystemLocaleString((PSystem::PLocaleString)(PSystem::pLocale_DayName1 + tempVar - 1));
								i += 3;
							}
							else
							{
								// Day of the week abbreviated
								tempStr = PSystem::GetSystemLocaleString((PSystem::PLocaleString)(PSystem::pLocale_AbbrevDayName1 + tempVar - 1));
								i += 2;
							}
						}
						else
						{
							// With leading 0
							tempStr.Format("%02d", day);
							i++;
						}
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", day);
					}
					break;
				}

				//
				// Month
				//
				case 'M':
				{
					if (format.GetAt(i + 1) == 'M')
					{
						if (format.GetAt(i + 2) == 'M')
						{
							if (format.GetAt(i + 3) == 'M')
							{
								// Month full string
								tempStr = PSystem::GetSystemLocaleString((PSystem::PLocaleString)(PSystem::pLocale_MonthName1 + month - 1));
								i += 3;
							}
							else
							{
								// Month abbreviated
								tempStr = PSystem::GetSystemLocaleString((PSystem::PLocaleString)(PSystem::pLocale_AbbrevMonthName1 + month - 1));
								i += 2;
							}
						}
						else
						{
							// With leading 0
							tempStr.Format("%02d", month);
							i++;
						}
					}
					else
					{
						// Without leading 0
						tempStr.Format("%d", month);
					}
					break;
				}

				//
				// Year
				//
				case 'y':
				{
					if (format.GetAt(i + 1) == 'y')
					{
						if ((format.GetAt(i + 2) == 'y') && (format.GetAt(i + 3) == 'y'))
						{
							// Four digits
							tempStr.Format("%04d", year);
							i += 3;
						}
						else
						{
							// Last two digits
							tempVar = year % 100;
							tempStr.Format("%02d", tempVar);
							i++;
						}
					}
					else
					{
						// Last digit
						tempVar = year % 10;
						tempStr.Format("%d", tempVar);
					}
					break;
				}

				//
				// Just copy the character
				//
				default:
				{
					tempStr = format.GetAt(i);
					break;
				}
			}
		}
		else
			tempStr = format.GetAt(i);

		// Add the character or formatted string to the result
		retStr += tempStr;
	}

	return (retStr);
}



/******************************************************************************/
/* Format() returns the objects date and/or time in the format of the format  */
/*      string.                                                               */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource object to use.             */
/*         "strNum" is the resource string number.                            */
/*                                                                            */
/* Output: The formatted string.                                              */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PString PTime::Format(PResource *resource, int32 strNum)
{
	PString str;

	str.LoadString(resource, strNum);
	return (Format(str));
}



/******************************************************************************/
/* GetUserDate() returns the objects date in user format.                     */
/*                                                                            */
/* Input:  "longDate" is true for long date format, false for short.          */
/*                                                                            */
/* Output: The string with the date.                                          */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PString PTime::GetUserDate(bool longDate)
{
	PString dateStr, formatStr;

	// Get the format string
	if (longDate)
		formatStr = PSystem::GetSystemLocaleString(PSystem::pLocale_LongDate);
	else
		formatStr = PSystem::GetSystemLocaleString(PSystem::pLocale_ShortDate);

	// Now format the date
	dateStr = Format(formatStr);

	// Return the formatted date string
	return (dateStr);
}



/******************************************************************************/
/* GetUserTime() returns the objects time in user format.                     */
/*                                                                            */
/* Input:  "format" is how you want the time returned.                        */
/*                                                                            */
/* Output: The string with the time.                                          */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PString PTime::GetUserTime(uint32 format)
{
	PString timeStr, formatStr;
	int32 index;

	// Get the format string
	formatStr = PSystem::GetSystemLocaleString(PSystem::pLocale_Time);

	// Clip out the things the caller don't want
	if (format & pNoSeconds)
	{
		index = formatStr.Find("%ss");
		if (index != -1)
			formatStr = formatStr.Left(index - 1) + formatStr.Mid(index + 3);

		index = formatStr.Find("%s");
		if (index != -1)
			formatStr = formatStr.Left(index - 1) + formatStr.Mid(index + 2);
	}

	if (format & pNoMinutesOrSeconds)
	{
		index = formatStr.Find("%mm");
		if (index != -1)
			formatStr = formatStr.Left(index - 1) + formatStr.Mid(index + 3);

		index = formatStr.Find("%m");
		if (index != -1)
			formatStr = formatStr.Left(index - 1) + formatStr.Mid(index + 2);

		index = formatStr.Find("%ss");
		if (index != -1)
			formatStr = formatStr.Left(index - 1) + formatStr.Mid(index + 3);

		index = formatStr.Find("%s");
		if (index != -1)
			formatStr = formatStr.Left(index - 1) + formatStr.Mid(index + 2);
	}

	if (format & pNoTimeMarker)
	{
		index = formatStr.Find("%tt");
		if (index != -1)
			formatStr = formatStr.Left(index - 1) + formatStr.Mid(index + 3);

		index = formatStr.Find("%t");
		if (index != -1)
			formatStr = formatStr.Left(index - 1) + formatStr.Mid(index + 2);
	}

	timeStr = Format(formatStr);

	// Return the string containing the time in user format
	return (timeStr);
}



/******************************************************************************/
/* GetLocalTimeDifference() returns the difference in minutes between the     */
/*      local time and UTC time seen from the local time view. To calculate   */
/*      the UTC time, use this formula:                                       */
/*          UTC = local time + x                                              */
/*      where x is the returned value from this function. If the local time   */
/*      is CET, this function will return -60.                                */
/*                                                                            */
/* Output: The difference in minutes.                                         */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
int32 PTime::GetLocalTimeDifference(void)
{
	return (timezone / 60);
}



/******************************************************************************/
/* PTime = PTime                                                              */
/******************************************************************************/
const PTime & PTime::operator = (const PTime &time)
{
	SetTime(time);
	return (*this);
}



/******************************************************************************/
/* PTime = struct tm                                                          */
/******************************************************************************/
const PTime & PTime::operator = (const struct tm &time)
{
	SetTime(time);
	return (*this);
}



/******************************************************************************/
/* struct tm = PTime                                                          */
/******************************************************************************/
PTime::operator struct tm (void)
{
	struct tm tmTime;

	// Convert Julian to Value if needed
	JulianToValue();

	// Fill out the structure
	tmTime.tm_sec   = second;
	tmTime.tm_min   = minute;
	tmTime.tm_hour  = hour;
	tmTime.tm_mday  = day;
	tmTime.tm_mon   = month - 1;
	tmTime.tm_year  = year - 1900;
	tmTime.tm_wday  = GetDayOfWeek();
	tmTime.tm_yday  = GetDayOfYear();
	tmTime.tm_isdst = -1;

	return (tmTime);
}



/******************************************************************************/
/* PTime == PTime                                                             */
/******************************************************************************/
bool PTime::operator == (const PTime &time) const
{
	PTime time1(time), time2(*this);

	PrepareTimeCheck(time1, time2);

	return ((time1.julianDay == time2.julianDay) && (time1.julianTime == time2.julianTime));
}



/******************************************************************************/
/* PTime != PTime                                                             */
/******************************************************************************/
bool PTime::operator != (const PTime &time) const
{
	PTime time1(time), time2(*this);

	PrepareTimeCheck(time1, time2);

	return ((time1.julianDay != time2.julianDay) || (time1.julianTime != time2.julianTime));
}



/******************************************************************************/
/* PTime < PTime                                                              */
/******************************************************************************/
bool PTime::operator < (const PTime &time) const
{
	PTime time1(time), time2(*this);

	PrepareTimeCheck(time1, time2);

	return ((time2.julianDay < time1.julianDay) ||
			((time2.julianDay == time1.julianDay) && (time2.julianTime < time1.julianTime)));
}



/******************************************************************************/
/* PTime <= PTime                                                             */
/******************************************************************************/
bool PTime::operator <= (const PTime &time) const
{
	PTime time1(time), time2(*this);

	PrepareTimeCheck(time1, time2);

	return ((time2.julianDay < time1.julianDay) ||
			((time2.julianDay == time1.julianDay) && (time2.julianTime <= time1.julianTime)));
}



/******************************************************************************/
/* PTime > PTime                                                              */
/******************************************************************************/
bool PTime::operator > (const PTime &time) const
{
	PTime time1(time), time2(*this);

	PrepareTimeCheck(time1, time2);

	return ((time1.julianDay < time2.julianDay) ||
			((time1.julianDay == time2.julianDay) && (time1.julianTime < time2.julianTime)));
}



/******************************************************************************/
/* PTime >= PTime                                                             */
/******************************************************************************/
bool PTime::operator >= (const PTime &time) const
{
	PTime time1(time), time2(*this);

	PrepareTimeCheck(time1, time2);

	return ((time1.julianDay < time2.julianDay) ||
			((time1.julianDay == time2.julianDay) && (time1.julianTime <= time2.julianTime)));
}



/******************************************************************************/
/* PTime + PTimeSpan                                                          */
/******************************************************************************/
PTime PTime::operator + (const PTimeSpan &timeSpan) const
{
	PTime time(*this);
	int64 totalSpan;

	// Add the time span
	totalSpan = timeSpan.GetTotalMilliSeconds();

	while (totalSpan > 0x7fffffff)
	{
		time.AddMilliSeconds(0x7fffffff);
		totalSpan -= 0x7fffffff;
	}

	time.AddMilliSeconds((int32)totalSpan);

	return (time);
}



/******************************************************************************/
/* PTime - PTimeSpan                                                          */
/******************************************************************************/
PTime PTime::operator - (const PTimeSpan &timeSpan) const
{
	PTime time(*this);
	int64 totalSpan;

	// Subtract the time span
	totalSpan = timeSpan.GetTotalMilliSeconds();

	while (totalSpan > 0x7fffffff)
	{
		time.SubMilliSeconds(0x7fffffff);
		totalSpan -= 0x7fffffff;
	}

	time.SubMilliSeconds((int32)totalSpan);

	return (time);
}



/******************************************************************************/
/* PTime += PTimeSpan                                                         */
/******************************************************************************/
const PTime & PTime::operator += (const PTimeSpan &timeSpan)
{
	int64 totalSpan;

	// Add the time span
	totalSpan = timeSpan.GetTotalMilliSeconds();

	while (totalSpan > 0x7fffffff)
	{
		AddMilliSeconds(0x7fffffff);
		totalSpan -= 0x7fffffff;
	}

	AddMilliSeconds((int32)totalSpan);

	return (*this);
}



/******************************************************************************/
/* PTime -= PTimeSpan                                                         */
/******************************************************************************/
const PTime & PTime::operator -= (const PTimeSpan &timeSpan)
{
	int64 totalSpan;

	// Subtract the time span
	totalSpan = timeSpan.GetTotalMilliSeconds();

	while (totalSpan > 0x7fffffff)
	{
		SubMilliSeconds(0x7fffffff);
		totalSpan -= 0x7fffffff;
	}

	SubMilliSeconds((int32)totalSpan);

	return (*this);
}



/******************************************************************************/
/* PTime - PTime                                                              */
/******************************************************************************/
PTimeSpan PTime::operator - (const PTime &time) const
{
	PTime time1(*this), time2(time);

	return (time2.GetTimeDifference(time1));
}



/******************************************************************************/
/* SetVariable() sets one of the variables to the right value.                */
/*                                                                            */
/* Input:  "var" is a reference to the variable to change.                    */
/*         "value" is the new value.                                          */
/******************************************************************************/
void PTime::SetVariable(int32 &var, int32 value)
{
	switch (state)
	{
		case pValue:
		{
			var = value;
			break;
		}

		case pJulian:
		{
			// Convert from Julian Day to values
			JulianToValue();

			var   = value;
			state = pValue;
			break;
		}

		case pBoth:
		{
			var   = value;
			state = pValue;
			break;
		}
	}
}



/******************************************************************************/
/* JulianToValue() converts the JulianDay variables to value variables.       */
/******************************************************************************/
void PTime::JulianToValue(void)
{
	double z, f;
	int32 a, alfa, b, c, d, e;
	int32 time;

	if (state == pJulian)
	{
		// Split the Julian Day into decimal and fraction part
		f = modf(julianDay + 0.5, &z);

		// Calculate temporary variables
		if (z < 2299161)
			a = (int32)z;
		else
		{
			alfa = (int32)((z - 1867216.25) / 36524.25);
			a    = (int32)(z + 1 + alfa - (int32)(alfa / 4));
		}

		b = a + 1524;
		c = (int32)((b - 122.1) / 365.25);
		d = (int32)(365.25 * c);
		e = (int32)((b - d) / 30.6001);

		// Begin to extract the values from the temporary variables
		day = (int32)(b - d - (int32)(30.6001 * e) + f);

		if (e < 14)
			month = e - 1;
		else
			month = e - 13;

		if (month > 2)
			year = c - 4716;
		else
			year = c - 4715;

		// Calculate time values
		hour         = julianTime / (60 * 60 * 1000);

		time         = julianTime - hour * (60 * 60 * 1000);
		minute       = time / (60 * 1000);

		time         = time - minute * (60 * 1000);
		second       = time / 1000;

		milliSeconds = time - second * 1000;

		// Set the state to both values and Julian Day are correct
		state = pBoth;
	}
}



/******************************************************************************/
/* ValueToJulian() converts the value variables to JulianDay variables.       */
/******************************************************************************/
void PTime::ValueToJulian(void)
{
	int32 a, b;
	int32 calcMonth, calcYear;

	if (state == pValue)
	{
		// Calculate number of milliseconds in the day
		julianTime = milliSeconds + (second * 1000) + (minute * 60 * 1000) + (hour * 60 * 60 * 1000);

		// Check to see if we need to change the month and year
		if (month > 2)
		{
			calcMonth = month;
			calcYear  = year;
		}
		else
		{
			calcMonth = month + 12;
			calcYear  = year - 1;
		}

		// Calculate temporary variables
		if (year < 1582)
			b = 0;			// Julian calendar
		else
		{
			if ((year == 1582) && (month <= 10) && (day <= 4))
				b = 0;		// Julian calendar
			else
			{
				// Gregorian calendar
				a = year / 100;
				b = 2 - a + (a / 4);
			}
		}

		// Calculate Julian day
		julianDay = ((int32)(365.25 * (calcYear + 4716))) +
					((int32)(30.6001 * (calcMonth + 1))) +
					day + b - 1524.5;

		// Set state to both values and Julian Day are correct
		state = pBoth;
	}
}



/******************************************************************************/
/* Normalize() normalizes the Julian variables. Will add the integer part of  */
/*      the time to the day and remove it from the time.                      */
/******************************************************************************/
void PTime::Normalize(void)
{
	// Remove whole days from the Julian time and store them
	// in the Julian day
	julianDay += julianTime / (24 * 60 * 60 * 1000);
	julianTime = julianTime % (24 * 60 * 60 * 1000);

	if (julianTime < 0)
	{
		julianDay--;
		julianTime += (24 * 60 * 60 * 1000);
	}
}



/******************************************************************************/
/* PrepareTimeCheck() prepares two time objects so they can be tested.        */
/*                                                                            */
/* Input:  "time1" is the first time object.                                  */
/*         "time2" is the second time object.                                 */
/******************************************************************************/
void PTime::PrepareTimeCheck(PTime &time1, PTime &time2) const
{
	time1.ValueToJulian();
	time2.ValueToJulian();
}
