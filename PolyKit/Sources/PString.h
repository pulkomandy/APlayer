/******************************************************************************/
/* PString header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PString_h
#define __PString_h

// PolyKit headers
#include "POS.h"
#include "ImportExport.h"


/******************************************************************************/
/* Defines used in the classes                                                */
/******************************************************************************/
#define P_MAX_CHAR_LEN		3		// Max character length in all character sets. If you create a new character set, remember to check/update this



/******************************************************************************/
/* PCharacterSet class                                                        */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PCharacterSet
{
public:
	virtual uint16 ToUnicode(const char *chr, int8 &charLen) = 0;
	virtual const char *FromUnicode(uint16 chr, int8 &len) = 0;

	virtual int8 GetCharLength(const char *chr) = 0;
	virtual bool IsValid(const char *chr, int8 len) = 0;

	virtual PCharacterSet *CreateObject(void) const = 0;

protected:
	char GetCharFromMultiTable(uint16 chr, const uint8 **indexTable);
};



/******************************************************************************/
/* PCharSet_Amiga class                                                       */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_Amiga : public PCharacterSet
{
public:
	PCharSet_Amiga(void);
	virtual ~PCharSet_Amiga(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_OEM_850 class                                                     */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_OEM_850 : public PCharacterSet
{
public:
	PCharSet_OEM_850(void);
	virtual ~PCharSet_OEM_850(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_Machintosh_Roman class                                            */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_Macintosh_Roman : public PCharacterSet
{
public:
	PCharSet_Macintosh_Roman(void);
	virtual ~PCharSet_Macintosh_Roman(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_MS_WIN_1250 class                                                 */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_MS_WIN_1250 : public PCharacterSet
{
public:
	PCharSet_MS_WIN_1250(void);
	virtual ~PCharSet_MS_WIN_1250(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_MS_WIN_1251 class                                                 */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_MS_WIN_1251 : public PCharacterSet
{
public:
	PCharSet_MS_WIN_1251(void);
	virtual ~PCharSet_MS_WIN_1251(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_MS_WIN_1252 class                                                 */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_MS_WIN_1252 : public PCharacterSet
{
public:
	PCharSet_MS_WIN_1252(void);
	virtual ~PCharSet_MS_WIN_1252(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_MS_WIN_1253 class                                                 */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_MS_WIN_1253 : public PCharacterSet
{
public:
	PCharSet_MS_WIN_1253(void);
	virtual ~PCharSet_MS_WIN_1253(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_MS_WIN_1254 class                                                 */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_MS_WIN_1254 : public PCharacterSet
{
public:
	PCharSet_MS_WIN_1254(void);
	virtual ~PCharSet_MS_WIN_1254(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_MS_WIN_1257 class                                                 */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_MS_WIN_1257 : public PCharacterSet
{
public:
	PCharSet_MS_WIN_1257(void);
	virtual ~PCharSet_MS_WIN_1257(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_MS_WIN_1258 class                                                 */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_MS_WIN_1258 : public PCharacterSet
{
public:
	PCharSet_MS_WIN_1258(void);
	virtual ~PCharSet_MS_WIN_1258(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	char charBuf;
};



/******************************************************************************/
/* PCharSet_UNICODE class                                                     */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_UNICODE : public PCharacterSet
{
public:
	PCharSet_UNICODE(void);
	virtual ~PCharSet_UNICODE(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	uint8 charBuf[2];
};



/******************************************************************************/
/* PCharSet_UTF8 class                                                        */
/******************************************************************************/
class _IMPEXP_PKLIB PCharSet_UTF8 : public PCharacterSet
{
public:
	PCharSet_UTF8(void);
	virtual ~PCharSet_UTF8(void);

	virtual uint16 ToUnicode(const char *chr, int8 &charLen);
	virtual const char *FromUnicode(uint16 chr, int8 &len);

	virtual int8 GetCharLength(const char *chr);
	virtual bool IsValid(const char *chr, int8 len);

	virtual PCharacterSet *CreateObject(void) const;

protected:
	uint8 charBuf[3];
};



/******************************************************************************/
/* Define the operative system default character set                          */
/******************************************************************************/
#define PCharSet_BeOS		PCharSet_UTF8



/******************************************************************************/
/* PChar class                                                                */
/******************************************************************************/
class _IMPEXP_PKLIB PChar
{
public:
	PChar(void);
	PChar(const PChar &chr);
	PChar(char chr);
	PChar(char chr, const PCharacterSet *characterSet);
	PChar(const char *chr, int8 len);
	PChar(const char *chr, int8 len, const PCharacterSet *characterSet);
	virtual ~PChar(void);

	bool IsNull(void) const;
	int8 GetLength(void) const;

	const char *GetChar(void) const;

	void SetChar(const PChar &chr);
	void SetChar(char chr);
	void SetChar(char chr, const PCharacterSet *characterSet);
	void SetChar(const char *chr, int8 len);
	void SetChar(const char *chr, int8 len, const PCharacterSet *characterSet);

	PCharacterSet *GetCharacterSet(void) const;
	void SetCharacterSet(const PCharacterSet *characterSet);

	void MakeLower(void);
	void MakeUpper(void);

	static uint16 MakeLower(uint16 uniChar);
	static uint16 MakeUpper(uint16 uniChar);

	bool IsAlpha(void) const;
	bool IsAlphaNum(void) const;
	bool IsAscii(void) const;
	bool IsControl(void) const;
	bool IsDigit(void) const;
	bool IsLower(void) const;
	bool IsSpace(void) const;
	bool IsUpper(void) const;

	static bool IsAlpha(uint16 uniChar);
	static bool IsAlphaNum(uint16 uniChar);
	static bool IsAscii(uint16 uniChar);
	static bool IsControl(uint16 uniChar);
	static bool IsDigit(uint16 uniChar);
	static bool IsLower(uint16 uniChar);
	static bool IsSpace(uint16 uniChar);
	static bool IsUpper(uint16 uniChar);

	int32 CompareNoCase(const PChar &chr) const;

	const PChar & operator = (const PChar &chr);
	const PChar & operator = (char chr);

	_IMPEXP_PKLIB friend bool operator == (const PChar &chr1, const PChar &chr2);
	_IMPEXP_PKLIB friend bool operator == (const PChar &chr1, char chr2);
	_IMPEXP_PKLIB friend bool operator == (char chr1, const PChar &chr2);

	_IMPEXP_PKLIB friend bool operator != (const PChar &chr1, const PChar &chr2);
	_IMPEXP_PKLIB friend bool operator != (const PChar &chr1, char chr2);
	_IMPEXP_PKLIB friend bool operator != (char chr1, const PChar &chr2);

	_IMPEXP_PKLIB friend bool operator <= (const PChar &chr1, const PChar &chr2);
	_IMPEXP_PKLIB friend bool operator <= (const PChar &chr1, char chr2);
	_IMPEXP_PKLIB friend bool operator <= (char chr1, const PChar &chr2);

	_IMPEXP_PKLIB friend bool operator >= (const PChar &chr1, const PChar &chr2);
	_IMPEXP_PKLIB friend bool operator >= (const PChar &chr1, char chr2);
	_IMPEXP_PKLIB friend bool operator >= (char chr1, const PChar &chr2);

	_IMPEXP_PKLIB friend bool operator < (const PChar &chr1, const PChar &chr2);
	_IMPEXP_PKLIB friend bool operator < (const PChar &chr1, char chr2);
	_IMPEXP_PKLIB friend bool operator < (char chr1, const PChar &chr2);

	_IMPEXP_PKLIB friend bool operator > (const PChar &chr1, const PChar &chr2);
	_IMPEXP_PKLIB friend bool operator > (const PChar &chr1, char chr2);
	_IMPEXP_PKLIB friend bool operator > (char chr1, const PChar &chr2);

protected:
	void Init(void);
	uint16 ConvertCharacter(const PChar &chr) const;
	uint16 ConvertCharacter(char chr) const;

	static bool CheckCharacter(uint16 uniChar, uint8 flag);
	static uint16 ChangeCase(uint16 uniChar);

	char character[P_MAX_CHAR_LEN * 2];
	int8 length;
	PCharacterSet *charSet;
};



/******************************************************************************/
/* PString class                                                              */
/******************************************************************************/
class _IMPEXP_PKLIB PResource;

class _IMPEXP_PKLIB PString
{
public:
	PString(void);
	PString(const PString &string);
	PString(const char *string);
	PString(const char *string, PCharacterSet *characterSet);
	PString(const PChar &chr);
	virtual ~PString(void);

	char *GetString(int32 *length = NULL) const;
	char *GetString(PCharacterSet *characterSet, int32 *length = NULL) const;
	void FreeBuffer(char *buffer) const;
	void SetString(const char *string, PCharacterSet *characterSet = NULL);
	void SetString(const char *string, int32 length, PCharacterSet *characterSet = NULL);

	void SwitchCharacterSet(const PCharacterSet *characterSet);
	void SwitchToHostCharacterSet(void);

	void ForceStringCopy(void);

	int32 GetLength(void) const;
	int32 GetByteLength(PCharacterSet *characterSet) const;

	PChar GetAt(int32 index) const;
	void SetAt(int32 index, PChar chr);

	bool IsEmpty(void) const;
	void MakeEmpty(void);

	void MakeLower(void);
	void MakeUpper(void);

	int32 GetNumber(int32 index = 0) const;
	uint32 GetUNumber(int32 index = 0) const;
	int64 GetNumber64(int32 index = 0) const;
	uint64 GetUNumber64(int32 index = 0) const;
	uint32 GetHexNumber(int32 index = 0) const;
	uint64 GetHexNumber64(int32 index = 0) const;

	void SetNumber(int32 number, bool thousandFix = false);
	void SetNumber64(int64 number, bool thousandFix = false);
	void SetUNumber(uint32 number, bool thousandFix = false);
	void SetUNumber64(uint64 number, bool thousandFix = false);
	void SetHexNumber(uint32 number, bool upper = true);
	void SetHexNumber64(uint64 number, bool upper = true);

	static PString CreateNumber(int32 number, bool thousandFix = false);
	static PString CreateUNumber(uint32 number, bool thousandFix = false);
	static PString CreateNumber64(int64 number, bool thousandFix = false);
	static PString CreateUNumber64(uint64 number, bool thousandFix = false);
	static PString CreateHexNumber(uint32 number, bool upper = true);
	static PString CreateHexNumber64(uint64 number, bool upper = true);

	void Format(PString formatString, ...);
	void Format(const char *formatString, ...);
	void Format(PResource *resource, int32 id, ...);

	void Format_S1(PString formatString, PString str1);
	void Format_S1(const char *formatString, PString str1);
	void Format_S1(PResource *resource, int32 id, PString str1);
	void Format_S2(PString formatString, PString str1, PString str2);
	void Format_S2(const char *formatString, PString str1, PString str2);
	void Format_S2(PResource *resource, int32 id, PString str1, PString str2);
	void Format_S3(PString formatString, PString str1, PString str2, PString str3);
	void Format_S3(const char *formatString, PString str1, PString str2, PString str3);
	void Format_S3(PResource *resource, int32 id, PString str1, PString str2, PString str3);
	void Format_S4(PString formatString, PString str1, PString str2, PString str3, PString str4);
	void Format_S4(const char *formatString, PString str1, PString str2, PString str3, PString str4);
	void Format_S4(PResource *resource, int32 id, PString str1, PString str2, PString str3, PString str4);
	void Format_S5(PString formatString, PString str1, PString str2, PString str3, PString str4, PString str5);
	void Format_S5(const char *formatString, PString str1, PString str2, PString str3, PString str4, PString str5);
	void Format_S5(PResource *resource, int32 id, PString str1, PString str2, PString str3, PString str4, PString str5);

	void FormatV(PString formatString, va_list argList);
	void FormatV(const char *formatString, va_list argList);
	void FormatV(PResource *resource, int32 id, va_list argList);
	void LoadString(PResource *resource, int32 id);

	int32 Replace(PChar search, PChar replace);
	int32 Replace(PString search, PString replace);
	int32 Remove(PChar chr);
	void Insert(int32 index, PChar chr);
	void Insert(int32 index, PString string);
	void Delete(int32 index, int32 count = 1);

	int32 Find(PChar chr, int32 startIndex = 0) const;
	int32 Find(PString subString, int32 startIndex = 0) const;
	int32 ReverseFind(PChar chr) const;
	int32 ReverseFind(PChar chr, int32 startIndex) const;
	int32 FindOneOf(PString searchSet, int32 startIndex = 0) const;

	PString Left(int32 count) const;
	PString Mid(int32 first) const;
	PString Mid(int32 first, int32 count) const;
	PString Right(int32 count) const;

	void TrimLeft(void);
	void TrimRight(void);

	int32 Compare(PString string) const;
	int32 CompareNoCase(PString string) const;

	const PString & operator = (const PString &string);
	const PString & operator = (const char *string);
	const PString & operator = (PChar chr);

	_IMPEXP_PKLIB friend PString operator + (const PString &string1, const PString &string2);
	_IMPEXP_PKLIB friend PString operator + (const PString &string1, const char *string2);
	_IMPEXP_PKLIB friend PString operator + (const char *string1, const PString &string2);
	_IMPEXP_PKLIB friend PString operator + (const PString &string1, const PChar &chr2);
	_IMPEXP_PKLIB friend PString operator + (const PChar &chr1, const PString &string2);

	const PString & operator += (const PString &string);
	const PString & operator += (const char *string);
	const PString & operator += (const PChar &chr);

	_IMPEXP_PKLIB friend bool operator == (const PString &string1, const PString &string2);
	_IMPEXP_PKLIB friend bool operator == (const PString &string1, const char *string2);
	_IMPEXP_PKLIB friend bool operator == (const char *string1, const PString &string2);

	_IMPEXP_PKLIB friend bool operator != (const PString &string1, const PString &string2);
	_IMPEXP_PKLIB friend bool operator != (const PString &string1, const char *string2);
	_IMPEXP_PKLIB friend bool operator != (const char *string1, const PString &string2);

	_IMPEXP_PKLIB friend bool operator < (const PString &string1, const PString &string2);
	_IMPEXP_PKLIB friend bool operator < (const PString &string1, const char *string2);
	_IMPEXP_PKLIB friend bool operator < (const char *string1, const PString &string2);

	_IMPEXP_PKLIB friend bool operator > (const PString &string1, const PString &string2);
	_IMPEXP_PKLIB friend bool operator > (const PString &string1, const char *string2);
	_IMPEXP_PKLIB friend bool operator > (const char *string1, const PString &string2);

	_IMPEXP_PKLIB friend bool operator <= (const PString &string1, const PString &string2);
	_IMPEXP_PKLIB friend bool operator <= (const PString &string1, const char *string2);
	_IMPEXP_PKLIB friend bool operator <= (const char *string1, const PString &string2);

	_IMPEXP_PKLIB friend bool operator >= (const PString &string1, const PString &string2);
	_IMPEXP_PKLIB friend bool operator >= (const PString &string1, const char *string2);
	_IMPEXP_PKLIB friend bool operator >= (const char *string1, const PString &string2);

protected:
	struct PStringData;

	void Initialize(void);

	void SetToHostCharacterSet(void);
	void CreateEmptyString(void);
	void CopyString(const char *string, PCharacterSet *characterSet);
	void CopyString(const char *string, int32 length, PCharacterSet *characterSet);
	void CreateString(char *buffer, int32 &length, PCharacterSet *characterSet) const;

	int32 CountStringLength(const char *string, PCharacterSet *characterSet) const;

	void AllocBuffer(int32 length);
	void Release(void);
	void Release(PStringData *data);

	void CopyBeforeWrite(void);
	void AllocBeforeWrite(int32 length);
	void EnlargeBeforeWrite(int32 newLength);
	void AssignCopy(const char *string, PCharacterSet *characterSet = NULL);
	void AssignCopy(PChar chr);
	void AllocCopy(PString &dest, int32 startIndex, int32 length) const;

	void ConcatInPlace(const PStringData *string);
	void ConcatInPlace(const PChar &chr);
	void ConcatCopy(const PStringData *sourceString, const PStringData *appendString);
	void ConcatCopy(const PStringData *sourceString, const PChar &appendChar);
	void ConcatCopy(const PChar &sourceChar, const PStringData *appendString);

	int8 CompareStrings(const PStringData *string1, const PStringData *string2) const;

	void FormatIt(PString formatString, va_list argList);
	int32 FloatP10(double *floatNum, int32 *floatSign, int32 precision);
	void ToOctal(uint16 *buffer, int32 &length, uint64 number);
	void ToHex(uint16 *buffer, int32 &length, uint64 number, bool upper);

	PStringData *stringData;
	PCharacterSet *charSet;
};



/******************************************************************************/
/* Global helper functions                                                    */
/******************************************************************************/
PCharacterSet *CreateHostCharacterSet(void);

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
