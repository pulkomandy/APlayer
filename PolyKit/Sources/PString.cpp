/******************************************************************************/
/* PString implementation file.                                               */
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
#include "PSystem.h"
#include "PException.h"
#include "PSynchronize.h"
#include "PResource.h"
#include "PString.h"
#include "PStringTables.h"


/******************************************************************************/
/* CreateHostCharacterSet() creates an instance of the host character set.    */
/******************************************************************************/
PCharacterSet *CreateHostCharacterSet(void)
{
	return (new PCharSet_UTF8());
}





/******************************************************************************/
/* PCharacterSet class                                                        */
/******************************************************************************/

/******************************************************************************/
/* GetCharFromMultiTable() will convert an unicode character to any 8-bit     */
/*      character using a multi index table lookup.                           */
/*                                                                            */
/* Input:  "chr" is the unicode character to convert.                         */
/*         "indexTable" is a pointer to the index table to use.               */
/*                                                                            */
/* Output: The converted character.                                           */
/******************************************************************************/
char PCharacterSet::GetCharFromMultiTable(uint16 chr, const uint8 **indexTable)
{
	const uint8 *charTable;

	// Find the char table
	charTable = indexTable[chr >> 8];
	if (charTable == NULL)
		return (0x3f);		// Return '?' as an unknown character

	// Now get the new character
	return (charTable[chr & 0x00ff]);
}





/******************************************************************************/
/* PCharSet_Amiga class                                                       */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_Amiga::PCharSet_Amiga(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_Amiga::~PCharSet_Amiga(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_Amiga::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (Amiga_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in Amiga character set.                              */
/******************************************************************************/
const char *PCharSet_Amiga::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, Amiga_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_Amiga::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_Amiga::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new Amiga character set object.                    */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_Amiga::CreateObject(void) const
{
	return (new PCharSet_Amiga());
}





/******************************************************************************/
/* PCharSet_OEM_850 class                                                     */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_OEM_850::PCharSet_OEM_850(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_OEM_850::~PCharSet_OEM_850(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_OEM_850::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (OEM_850_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in codepage 850.                                     */
/******************************************************************************/
const char *PCharSet_OEM_850::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, OEM_850_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_OEM_850::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_OEM_850::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new OEM_850 character set object.                  */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_OEM_850::CreateObject(void) const
{
	return (new PCharSet_OEM_850());
}





/******************************************************************************/
/* PCharSet_Macintosh_Roman class                                             */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_Macintosh_Roman::PCharSet_Macintosh_Roman(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_Macintosh_Roman::~PCharSet_Macintosh_Roman(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_Macintosh_Roman::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (Mac_Roman_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in Macintosh Roman.                                  */
/******************************************************************************/
const char *PCharSet_Macintosh_Roman::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, Mac_Roman_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_Macintosh_Roman::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_Macintosh_Roman::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new Macintosh_Roman character set object.          */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_Macintosh_Roman::CreateObject(void) const
{
	return (new PCharSet_Macintosh_Roman());
}





/******************************************************************************/
/* PCharSet_MS_WIN_1250 class                                                 */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_MS_WIN_1250::PCharSet_MS_WIN_1250(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_MS_WIN_1250::~PCharSet_MS_WIN_1250(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_MS_WIN_1250::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (MS_WIN_1250_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in codepage 1250.                                    */
/******************************************************************************/
const char *PCharSet_MS_WIN_1250::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, MS_WIN_1250_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_MS_WIN_1250::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_MS_WIN_1250::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new MS_WIN_1250 character set object.              */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_MS_WIN_1250::CreateObject(void) const
{
	return (new PCharSet_MS_WIN_1250());
}





/******************************************************************************/
/* PCharSet_MS_WIN_1251 class                                                 */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_MS_WIN_1251::PCharSet_MS_WIN_1251(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_MS_WIN_1251::~PCharSet_MS_WIN_1251(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_MS_WIN_1251::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (MS_WIN_1251_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in codepage 1251.                                    */
/******************************************************************************/
const char *PCharSet_MS_WIN_1251::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, MS_WIN_1251_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_MS_WIN_1251::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_MS_WIN_1251::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new MS_WIN_1251 character set object.              */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_MS_WIN_1251::CreateObject(void) const
{
	return (new PCharSet_MS_WIN_1251());
}





/******************************************************************************/
/* PCharSet_MS_WIN_1252 class                                                 */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_MS_WIN_1252::PCharSet_MS_WIN_1252(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_MS_WIN_1252::~PCharSet_MS_WIN_1252(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_MS_WIN_1252::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (MS_WIN_1252_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in codepage 1252.                                    */
/******************************************************************************/
const char *PCharSet_MS_WIN_1252::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, MS_WIN_1252_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_MS_WIN_1252::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_MS_WIN_1252::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new MS_WIN_1252 character set object.              */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_MS_WIN_1252::CreateObject(void) const
{
	return (new PCharSet_MS_WIN_1252());
}





/******************************************************************************/
/* PCharSet_MS_WIN_1253 class                                                 */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_MS_WIN_1253::PCharSet_MS_WIN_1253(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_MS_WIN_1253::~PCharSet_MS_WIN_1253(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_MS_WIN_1253::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (MS_WIN_1253_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in codepage 1253.                                    */
/******************************************************************************/
const char *PCharSet_MS_WIN_1253::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, MS_WIN_1253_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_MS_WIN_1253::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_MS_WIN_1253::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new MS_WIN_1253 character set object.              */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_MS_WIN_1253::CreateObject(void) const
{
	return (new PCharSet_MS_WIN_1253());
}





/******************************************************************************/
/* PCharSet_MS_WIN_1254 class                                                 */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_MS_WIN_1254::PCharSet_MS_WIN_1254(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_MS_WIN_1254::~PCharSet_MS_WIN_1254(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_MS_WIN_1254::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (MS_WIN_1254_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in codepage 1254.                                    */
/******************************************************************************/
const char *PCharSet_MS_WIN_1254::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, MS_WIN_1254_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_MS_WIN_1254::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_MS_WIN_1254::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new MS_WIN_1254 character set object.              */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_MS_WIN_1254::CreateObject(void) const
{
	return (new PCharSet_MS_WIN_1254());
}





/******************************************************************************/
/* PCharSet_MS_WIN_1257 class                                                 */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_MS_WIN_1257::PCharSet_MS_WIN_1257(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_MS_WIN_1257::~PCharSet_MS_WIN_1257(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_MS_WIN_1257::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (MS_WIN_1257_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in codepage 1257.                                    */
/******************************************************************************/
const char *PCharSet_MS_WIN_1257::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, MS_WIN_1257_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_MS_WIN_1257::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_MS_WIN_1257::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new MS_WIN_1257 character set object.              */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_MS_WIN_1257::CreateObject(void) const
{
	return (new PCharSet_MS_WIN_1257());
}





/******************************************************************************/
/* PCharSet_MS_WIN_1258 class                                                 */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_MS_WIN_1258::PCharSet_MS_WIN_1258(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_MS_WIN_1258::~PCharSet_MS_WIN_1258(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the character.                               */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_MS_WIN_1258::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 1;

	// Return the character
	return (MS_WIN_1258_Table[(uint8)*chr]);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The character in codepage 1258.                                    */
/******************************************************************************/
const char *PCharSet_MS_WIN_1258::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
	charBuf = GetCharFromMultiTable(chr, MS_WIN_1258_HighByteIndex);

	// Store the length
	len = 1;

	return (&charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_MS_WIN_1258::GetCharLength(const char *chr)
{
	return (1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_MS_WIN_1258::IsValid(const char *chr, int8 len)
{
	if (len < 1)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new MS_WIN_1258 character set object.              */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_MS_WIN_1258::CreateObject(void) const
{
	return (new PCharSet_MS_WIN_1258());
}





/******************************************************************************/
/* PCharSet_UNICODE class                                                     */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_UNICODE::PCharSet_UNICODE(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_UNICODE::~PCharSet_UNICODE(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the unicode character.                       */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_UNICODE::ToUnicode(const char *chr, int8 &len)
{
	// Store the length
	len = 2;

	// Return the character
	return (*((uint16 *)chr));
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The unicode character.                                             */
/******************************************************************************/
const char *PCharSet_UNICODE::FromUnicode(uint16 chr, int8 &len)
{
	// Convert the character
#if __p_endian == __p_big

	charBuf[0] = chr >> 8;
	charBuf[1] = chr & 0x00ff;

#else

	charBuf[0] = chr & 0x00ff;
	charBuf[1] = chr >> 8;

#endif

	// Store the length
	len = 2;

	return ((char *)charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_UNICODE::GetCharLength(const char *chr)
{
	return (2);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_UNICODE::IsValid(const char *chr, int8 len)
{
	if (len < 2)
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new UNICODE character set object.                  */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_UNICODE::CreateObject(void) const
{
	return (new PCharSet_UNICODE());
}





/******************************************************************************/
/* PCharSet_UTF8 class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PCharSet_UTF8::PCharSet_UTF8(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PCharSet_UTF8::~PCharSet_UTF8(void)
{
}



/******************************************************************************/
/* ToUnicode() convert a character to unicode.                                */
/*                                                                            */
/* Input:  "chr" is a pointer to the UTF8 character.                          */
/*         "len" is a reference where the source character length is stored.  */
/*                                                                            */
/* Output: The unicode character.                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
uint16 PCharSet_UTF8::ToUnicode(const char *chr, int8 &len)
{
	// First find the length of the character
	len = GetCharLength(chr);

	// The length has to be between 1 and 3
	if ((len < 1) || (len > 3))
		throw PBoundsException(P_GEN_ERR_BAD_ARGUMENT, len);

	// 0x0000 - 0x007f
	if (!(chr[0] & 0x80))
		return (chr[0]);

	// 0x0080 - 0x07ff
	if ((chr[0] & 0xe0) == 0xc0)
	{
		if (len < 2)
			throw PBoundsException(P_GEN_ERR_INSUFFICIENT_BUFFER, len);

		return ((((uint16)(chr[0] & 0x1f)) << 6) | ((uint16)(chr[1] & 0x3f)));
	}

	// 0x0800 - 0xffff
	if ((chr[0] & 0xf0) == 0xe0)
	{
		if (len < 3)
			throw PBoundsException(P_GEN_ERR_INSUFFICIENT_BUFFER, len);

		return ((((uint16)(chr[0] & 0x0f)) << 12) | (((uint16)(chr[1] & 0x3f)) << 6) | ((uint16)(chr[2] & 0x3f)));
	}

	// Unknown - Return the '?' character
	return (0x003f);
}



/******************************************************************************/
/* FromUnicode() convert an unicode character to a character.                 */
/*                                                                            */
/* Input:  "chr" is the unicode character.                                    */
/*         "len" is where the converted character length should be stored.    */
/*                                                                            */
/* Output: The UTF8 character.                                                */
/******************************************************************************/
const char *PCharSet_UTF8::FromUnicode(uint16 chr, int8 &len)
{
	// 0x0000 - 0x007f
	if (chr < 0x0080)
	{
		charBuf[0] = (uint8)chr;
		len        = 1;
	}
	else
	{
		// 0x0080 - 0x07ff
		if (chr < 0x0800)
		{
			charBuf[0] = 0xc0 | ((chr & 0x07c0) >> 6);
			charBuf[1] = 0x80 | (chr & 0x003f);
			len        = 2;
		}
		else
		{
			// 0x0800 - 0xffff
			charBuf[0] = 0xe0 | ((chr & 0xf000) >> 12);
			charBuf[1] = 0x80 | ((chr & 0x0fc0) >> 6);
			charBuf[2] = 0x80 | (chr & 0x003f);
			len        = 3;
		}
	}

	return ((char *)charBuf);
}



/******************************************************************************/
/* GetCharLength() calculate the length of the character given and return it. */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*                                                                            */
/* Output: The length of the character.                                       */
/******************************************************************************/
int8 PCharSet_UTF8::GetCharLength(const char *chr)
{
	return (((0xE5000000 >> ((*chr >> 3) & 0x1E) & 3)) + 1);
}



/******************************************************************************/
/* IsValid() will check to see if the character given is valid.               */
/*                                                                            */
/* Input:  "chr" is the character in the current character set.               */
/*         "len" is the maximum length of the character.                      */
/*                                                                            */
/* Output: True if the character is valid, false if not.                      */
/******************************************************************************/
bool PCharSet_UTF8::IsValid(const char *chr, int8 len)
{
	if (len < GetCharLength(chr))
		return (false);

	return (true);
}



/******************************************************************************/
/* CreateObject() create a new UTF8 character set object.                     */
/*                                                                            */
/* Output: A pointer to the new object. When done, use delete on the pointer. */
/******************************************************************************/
PCharacterSet *PCharSet_UTF8::CreateObject(void) const
{
	return (new PCharSet_UTF8());
}





/******************************************************************************/
/* PChar class                                                                */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PChar::PChar(void)
{
	// Initialize the object
	Init();
}



/******************************************************************************/
/* Copy constructor.                                                          */
/*                                                                            */
/* Input:  "chr" is the character object to copy.                             */
/******************************************************************************/
PChar::PChar(const PChar &chr)
{
	// Make the character set pointer null
	charSet = NULL;

	// Initialize the object
	SetChar(chr);
}



/******************************************************************************/
/* Character constructor                                                      */
/*                                                                            */
/* Input:  "chr" is the character to initialize the object with.              */
/******************************************************************************/
PChar::PChar(char chr)
{
	// Make the character set pointer null
	charSet = NULL;

	// Initialize the object
	SetChar(chr);
}



/******************************************************************************/
/* Character with character set constructor                                   */
/*                                                                            */
/* Input:  "chr" is the character to initialize the object with.              */
/*         "characterSet" is the character set to use.                        */
/******************************************************************************/
PChar::PChar(char chr, const PCharacterSet *characterSet)
{
	// Make the character set pointer null
	charSet = NULL;

	// Initialize the object
	SetChar(chr, characterSet);
}



/******************************************************************************/
/* Character string constructor.                                              */
/*                                                                            */
/* Input:  "chr" is the character in host format to set.                      */
/*         "len" is the length of the character.                              */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
PChar::PChar(const char *chr, int8 len)
{
	// Make the character set pointer null
	charSet = NULL;

	// Initialize the object
	SetChar(chr, len);
}



/******************************************************************************/
/* Character string with character set constructor.                           */
/*                                                                            */
/* Input:  "chr" is the character to set.                                     */
/*         "len" is the length of the character.                              */
/*         "characterSet" is the character set to use.                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
PChar::PChar(const char *chr, int8 len, const PCharacterSet *characterSet)
{
	// Make the character set pointer null
	charSet = NULL;

	// Initialize the object
	SetChar(chr, len, characterSet);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PChar::~PChar(void)
{
	// Free the character set
	delete charSet;
}



/******************************************************************************/
/* IsNull() checks the current character to see if it's null.                 */
/*                                                                            */
/* Output: True if it's null, false if not.                                   */
/******************************************************************************/
bool PChar::IsNull(void) const
{
	return (length == 0);
}



/******************************************************************************/
/* GetLength() returns the length in bytes of the current character.          */
/*                                                                            */
/* Output: The length of the character in bytes.                              */
/******************************************************************************/
int8 PChar::GetLength(void) const
{
	return (length);
}



/******************************************************************************/
/* GetChar() returns a pointer to the current character.                      */
/*                                                                            */
/* Output: A pointer to the character.                                        */
/******************************************************************************/
const char *PChar::GetChar(void) const
{
	return ((char *)character);
}



/******************************************************************************/
/* SetChar() copy the character given into the current object.                */
/*                                                                            */
/* Input:  "chr" is the character object to copy.                             */
/******************************************************************************/
void PChar::SetChar(const PChar &chr)
{
	// First delete the current character set
	delete charSet;
	charSet = NULL;

	// Copy the object
	memcpy(character, chr.character, P_MAX_CHAR_LEN * 2);
	length = chr.length;
	charSet = chr.charSet->CreateObject();
	if (charSet == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* SetChar() set the character given in the host format.                      */
/*                                                                            */
/* Input:  "chr" is the character in host format to set.                      */
/******************************************************************************/
void PChar::SetChar(char chr)
{
	// First delete the current character set
	delete charSet;
	charSet = NULL;

	// Initialize the object
	Init();

	// Set the character
	character[0] = chr;
	length       = 1;

	// Null terminate the character
	memset(&character[1], 0, P_MAX_CHAR_LEN);
}



/******************************************************************************/
/* SetChar() set the character given in the character set given.              */
/*                                                                            */
/* Input:  "chr" is the character to set.                                     */
/*         "characterSet" is the character set to use.                        */
/******************************************************************************/
void PChar::SetChar(char chr, const PCharacterSet *characterSet)
{
	// Null pointer not allowed
	ASSERT(characterSet != NULL);

	// First delete the current character set
	delete charSet;
	charSet = NULL;

	// Allocate a new character set
	charSet = characterSet->CreateObject();
	if (charSet == NULL)
		throw PMemoryException();

	// Set the character
	character[0] = chr;
	length       = 1;

	// Null terminate the character
	memset(&character[1], 0, P_MAX_CHAR_LEN);
}



/******************************************************************************/
/* SetChar() set the character given in the host format.                      */
/*                                                                            */
/* Input:  "chr" is the character in host format to set.                      */
/*         "len" is the length of the character.                              */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
void PChar::SetChar(const char *chr, int8 len)
{
	// Check the length
	if ((len < 1) || (len > P_MAX_CHAR_LEN))
		throw PBoundsException(P_GEN_ERR_BAD_ARGUMENT, len);

	// First delete the current character set
	delete charSet;
	charSet = NULL;

	// Initialize the object
	Init();

	if (chr != NULL)
	{
		// Set the character
		memcpy(character, chr, len);
	}
	else
	{
		// Null pointer given
		len = 0;
	}

	// Remember the length
	length = len;

	// Null terminate the character
	memset(&character[len], 0, P_MAX_CHAR_LEN);
}



/******************************************************************************/
/* SetChar() set the character given in the character set given.              */
/*                                                                            */
/* Input:  "chr" is the character to set.                                     */
/*         "len" is the length of the character.                              */
/*         "characterSet" is the character set to use.                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
void PChar::SetChar(const char *chr, int8 len, const PCharacterSet *characterSet)
{
	// Check the length
	if ((len < 1) || (len > P_MAX_CHAR_LEN))
		throw PBoundsException(P_GEN_ERR_BAD_ARGUMENT, len);

	// First delete the current character set
	delete charSet;
	charSet = NULL;

	// Allocate a new character set
	charSet = characterSet->CreateObject();
	if (charSet == NULL)
		throw PMemoryException();

	if (chr != NULL)
	{
		// Set the character
		memcpy(character, chr, len);
	}
	else
	{
		// Null pointer given
		len = 0;
	}

	// Remember the length
	length = len;

	// Null terminate the character
	memset(&character[len], 0, P_MAX_CHAR_LEN);
}



/******************************************************************************/
/* GetCharacterSet() returns a pointer to the current character set.          */
/*                                                                            */
/* Output: A pointer to the character set.                                    */
/******************************************************************************/
PCharacterSet *PChar::GetCharacterSet(void) const
{
	return (charSet);
}



/******************************************************************************/
/* SetCharacterSet() set a new character set to the character. The current    */
/*      character will be converted to the new character set.                 */
/*                                                                            */
/* Input:  "characterSet" is the new character set to use.                    */
/******************************************************************************/
void PChar::SetCharacterSet(const PCharacterSet *characterSet)
{
	uint16 curChar;
	const char *newChar;
	int8 len;

	// Null pointer not allowed
	ASSERT(characterSet != NULL);

	try
	{
		// First convert to current character to unicode
		curChar = charSet->ToUnicode(character, len);
	}
	catch(PBoundsException e)
	{
		curChar = 0x003f;
	}

	// Delete the old character set
	delete charSet;
	charSet = NULL;

	// Allocate a new character set, which is a copy of the one given
	charSet = characterSet->CreateObject();
	if (charSet == NULL)
		throw PMemoryException();

	// Convert the current character back again
	newChar = charSet->FromUnicode(curChar, length);

	// Illegal length returned from the character set!
	ASSERT(length <= P_MAX_CHAR_LEN);

	// Copy the new character into the buffer
	memcpy(character, newChar, length);
}



/******************************************************************************/
/* MakeLower() convert the current character to lower case.                   */
/******************************************************************************/
void PChar::MakeLower(void)
{
	uint16 uniChar;
	const char *buffer;
	int8 len;

	try
	{
		// Convert the character to unicode
		uniChar = charSet->ToUnicode(character, len);

		// Convert it to lower case
		uniChar = MakeLower(uniChar);

		// Convert it back to a real character
		buffer = charSet->FromUnicode(uniChar, len);
		memcpy(character, buffer, len);

		// Null terminate the character
		memset(&character[len], 0, P_MAX_CHAR_LEN);
	}
	catch(PBoundsException e)
	{
		;
	}
}



/******************************************************************************/
/* MakeUpper() convert the current character to upper case.                   */
/******************************************************************************/
void PChar::MakeUpper(void)
{
	uint16 uniChar;
	const char *buffer;
	int8 len;

	try
	{
		// Convert the character to unicode
		uniChar = charSet->ToUnicode(character, len);

		// Convert it to upper case
		uniChar = MakeUpper(uniChar);

		// Convert it back to a real character
		buffer = charSet->FromUnicode(uniChar, len);
		memcpy(character, buffer, len);

		// Null terminate the character
		memset(&character[len], 0, P_MAX_CHAR_LEN);
	}
	catch(PBoundsException e)
	{
		;
	}
}



/******************************************************************************/
/* MakeLower() convert the character given to lower case.                     */
/*                                                                            */
/* Input:  "uniChar" is the character to convert.                             */
/*                                                                            */
/* Output: The character in lower case if it can be changed, otherwise it's   */
/*         the same character value as the input.                             */
/******************************************************************************/
uint16 PChar::MakeLower(uint16 uniChar)
{
	// First check to see if the character is already a lower case character
	if (IsLower(uniChar))
		return (uniChar);

	return (ChangeCase(uniChar));
}



/******************************************************************************/
/* MakeUpper() convert the character given to upper case.                     */
/*                                                                            */
/* Input:  "uniChar" is the character to convert.                             */
/*                                                                            */
/* Output: The character in upper case if it can be changed, otherwise it's   */
/*         the same character value as the input.                             */
/******************************************************************************/
uint16 PChar::MakeUpper(uint16 uniChar)
{
	// First check to see if the character is already an upper case character
	if (IsUpper(uniChar))
		return (uniChar);

	return (ChangeCase(uniChar));
}



/******************************************************************************/
/* IsAlpha() check the current character to see if it is an alphabetic        */
/*      character.                                                            */
/*                                                                            */
/* Output: True if it is an alphabetic character, false if not.               */
/******************************************************************************/
bool PChar::IsAlpha(void) const
{
	int8 len;
	bool result;

	try
	{
		result = IsAlpha(charSet->ToUnicode(character, len));
	}
	catch(PBoundsException e)
	{
		result = false;
	}

	return (result);
}



/******************************************************************************/
/* IsAlphaNum() check the current character to see if it is an alphanumeric   */
/*      character.                                                            */
/*                                                                            */
/* Output: True if it is an alphanumeric character, false if not.             */
/******************************************************************************/
bool PChar::IsAlphaNum(void) const
{
	int8 len;
	bool result;

	try
	{
		result = IsAlphaNum(charSet->ToUnicode(character, len));
	}
	catch(PBoundsException e)
	{
		result = false;
	}

	return (result);
}



/******************************************************************************/
/* IsAscii() check the current character to see if it is an ascii character.  */
/*                                                                            */
/* Output: True if it is an ascii character, false if not.                    */
/******************************************************************************/
bool PChar::IsAscii(void) const
{
	int8 len;
	bool result;

	try
	{
		result = IsAscii(charSet->ToUnicode(character, len));
	}
	catch(PBoundsException e)
	{
		result = false;
	}

	return (result);
}



/******************************************************************************/
/* IsControl() check the current character to see if it is a control          */
/*      character.                                                            */
/*                                                                            */
/* Output: True if it is a control character, false if not.                   */
/******************************************************************************/
bool PChar::IsControl(void) const
{
	int8 len;
	bool result;

	try
	{
		result = IsControl(charSet->ToUnicode(character, len));
	}
	catch(PBoundsException e)
	{
		result = false;
	}

	return (result);
}



/******************************************************************************/
/* IsDigit() check the current character to see if it is a decimal digit      */
/*      character.                                                            */
/*                                                                            */
/* Output: True if it is a decimal digit character, false if not.             */
/******************************************************************************/
bool PChar::IsDigit(void) const
{
	int8 len;
	bool result;

	try
	{
		result = IsDigit(charSet->ToUnicode(character, len));
	}
	catch(PBoundsException e)
	{
		result = false;
	}

	return (result);
}



/******************************************************************************/
/* IsLower() check the current character to see if it is in lower case.       */
/*                                                                            */
/* Output: True if it is in lower case, false if not.                         */
/******************************************************************************/
bool PChar::IsLower(void) const
{
	int8 len;
	bool result;

	try
	{
		result = IsLower(charSet->ToUnicode(character, len));
	}
	catch(PBoundsException e)
	{
		result = false;
	}

	return (result);
}



/******************************************************************************/
/* IsSpace() check the current character to see if it is a space character.   */
/*                                                                            */
/* Output: True if it is a space character, false if not.                     */
/******************************************************************************/
bool PChar::IsSpace(void) const
{
	int8 len;
	bool result;

	try
	{
		result = IsSpace(charSet->ToUnicode(character, len));
	}
	catch(PBoundsException e)
	{
		result = false;
	}

	return (result);
}



/******************************************************************************/
/* IsUpper() check the current character to see if it is in upper case.       */
/*                                                                            */
/* Output: True if it is in upper case, false if not.                         */
/******************************************************************************/
bool PChar::IsUpper(void) const
{
	int8 len;
	bool result;

	try
	{
		result = IsUpper(charSet->ToUnicode(character, len));
	}
	catch(PBoundsException e)
	{
		result = false;
	}

	return (result);
}



/******************************************************************************/
/* IsAlpha() check the character given to see if it is an alphabetic          */
/*      character.                                                            */
/*                                                                            */
/* Input:  "uniChar" is the character to test.                                */
/*                                                                            */
/* Output: True if it is an alphabetic character, false if not.               */
/******************************************************************************/
bool PChar::IsAlpha(uint16 uniCode)
{
	if (IsUpper(uniCode) || IsLower(uniCode))
		return (true);

	return (false);
}



/******************************************************************************/
/* IsAlphaNum() check the character given to see if it is an alphanumeric     */
/*      character.                                                            */
/*                                                                            */
/* Input:  "uniChar" is the character to test.                                */
/*                                                                            */
/* Output: True if it is an alphanumeric character, false if not.             */
/******************************************************************************/
bool PChar::IsAlphaNum(uint16 uniCode)
{
	if (IsAlpha(uniCode) || IsDigit(uniCode))
		return (true);

	return (false);
}



/******************************************************************************/
/* IsAscii() check the character given to see if it is an ascii character.    */
/*                                                                            */
/* Input:  "uniChar" is the character to test.                                */
/*                                                                            */
/* Output: True if it is an ascii character, false if not.                    */
/******************************************************************************/
bool PChar::IsAscii(uint16 uniCode)
{
	return (CheckCharacter(uniCode, UNIFLG_ASCII));
}



/******************************************************************************/
/* IsControl() check the character given to see if it is a control character. */
/*                                                                            */
/* Input:  "uniChar" is the character to test.                                */
/*                                                                            */
/* Output: True if it is a control character, false if not.                   */
/******************************************************************************/
bool PChar::IsControl(uint16 uniCode)
{
	return (CheckCharacter(uniCode, UNIFLG_CONTROL));
}



/******************************************************************************/
/* IsDigit() check the character given to see if it is a decimal digit        */
/*      character.                                                            */
/*                                                                            */
/* Input:  "uniChar" is the character to test.                                */
/*                                                                            */
/* Output: True if it is a decimal digit character, false if not.             */
/******************************************************************************/
bool PChar::IsDigit(uint16 uniCode)
{
	return (CheckCharacter(uniCode, UNIFLG_DIGIT));
}



/******************************************************************************/
/* IsLower() check the character given to see if it is in lower case.         */
/*                                                                            */
/* Input:  "uniChar" is the character to test.                                */
/*                                                                            */
/* Output: True if it is in lower case, false if not.                         */
/******************************************************************************/
bool PChar::IsLower(uint16 uniCode)
{
	return (CheckCharacter(uniCode, UNIFLG_LOWERCASE));
}



/******************************************************************************/
/* IsSpace() check the character given to see if it is a space character.     */
/*                                                                            */
/* Input:  "uniChar" is the character to test.                                */
/*                                                                            */
/* Output: True if it is a space character, false if not.                     */
/******************************************************************************/
bool PChar::IsSpace(uint16 uniCode)
{
	return (CheckCharacter(uniCode, UNIFLG_WHITESPACE));
}



/******************************************************************************/
/* IsUpper() check the character given to see if it is in upper case.         */
/*                                                                            */
/* Input:  "uniChar" is the character to test.                                */
/*                                                                            */
/* Output: True if it is in upper case, false if not.                         */
/******************************************************************************/
bool PChar::IsUpper(uint16 uniCode)
{
	return (CheckCharacter(uniCode, UNIFLG_UPPERCASE));
}



/******************************************************************************/
/* CompareNoCase() will compare the character given against the current       */
/*      character with a non-case sentitive compare.                          */
/*                                                                            */
/* Input:  "chr" is the character you want to compare with.                   */
/*                                                                            */
/* Output: < 0 if this < arg, == 0 if this = arg, or > 0 if this > arg.       */
/******************************************************************************/
int32 PChar::CompareNoCase(const PChar &chr) const
{
	uint16 char1, char2;
	int8 len;

	// Convert the two characters to unicode
	try
	{
		char1 = charSet->ToUnicode(character, len);
	}
	catch(PBoundsException e)
	{
		char1 = 0x003f;
	}

	try
	{
		char2 = chr.charSet->ToUnicode(chr.character, len);
	}
	catch(PBoundsException e)
	{
		char2 = 0x003f;
	}

	// Make the characters in lower case
	char1 = MakeLower(char1);
	char2 = MakeLower(char2);

	// And compare them
	if (char1 < char2)
		return (-1);

	if (char1 > char2)
		return (1);

	return (0);
}



/******************************************************************************/
/* operator = (PChar &) will set the character to the character given.        */
/*                                                                            */
/* Input:  "chr" is the new character.                                        */
/*                                                                            */
/* Output: The new character object.                                          */
/******************************************************************************/
const PChar & PChar::operator = (const PChar &chr)
{
	SetChar(chr);
	return (*this);
}



/******************************************************************************/
/* operator = (char) will set the character to the character given.           */
/*                                                                            */
/* Input:  "chr" is the character.                                            */
/*                                                                            */
/* Output: The new character object.                                          */
/******************************************************************************/
const PChar & PChar::operator = (char chr)
{
	SetChar(chr);
	return (*this);
}



/******************************************************************************/
/* operator == (PChar, PChar) will compare the two characters.                */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are equal, false if not.                */
/******************************************************************************/
bool operator == (const PChar &chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 == c2);
}



/******************************************************************************/
/* operator == (PChar, char) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are equal, false if not.                */
/******************************************************************************/
bool operator == (const PChar &chr1, char chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr1.ConvertCharacter(chr2);

	return (c1 == c2);
}



/******************************************************************************/
/* operator == (char, PChar) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are equal, false if not.                */
/******************************************************************************/
bool operator == (char chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr2.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 == c2);
}



/******************************************************************************/
/* operator != (PChar, PChar) will compare the two characters.                */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are different, false if not.            */
/******************************************************************************/
bool operator != (const PChar &chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 != c2);
}



/******************************************************************************/
/* operator != (PChar, char) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are different, false if not.            */
/******************************************************************************/
bool operator != (const PChar &chr1, char chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr1.ConvertCharacter(chr2);

	return (c1 != c2);
}



/******************************************************************************/
/* operator != (char, PChar) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are different, false if not.            */
/******************************************************************************/
bool operator != (char chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr2.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 != c2);
}



/******************************************************************************/
/* operator <= (PChar, PChar) will compare the two characters.                */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are <=, false if not.                   */
/******************************************************************************/
bool operator <= (const PChar &chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 <= c2);
}



/******************************************************************************/
/* operator <= (PChar, char) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are <=, false if not.                   */
/******************************************************************************/
bool operator <= (const PChar &chr1, char chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr1.ConvertCharacter(chr2);

	return (c1 <= c2);
}



/******************************************************************************/
/* operator <= (char, PChar) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are <=, false if not.                   */
/******************************************************************************/
bool operator <= (char chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr2.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 <= c2);
}



/******************************************************************************/
/* operator >= (PChar, PChar) will compare the two characters.                */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are >=, false if not.                   */
/******************************************************************************/
bool operator >= (const PChar &chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 >= c2);
}



/******************************************************************************/
/* operator >= (PChar, char) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are >=, false if not.                   */
/******************************************************************************/
bool operator >= (const PChar &chr1, char chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr1.ConvertCharacter(chr2);

	return (c1 >= c2);
}



/******************************************************************************/
/* operator >= (char, PChar) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are >=, false if not.                   */
/******************************************************************************/
bool operator >= (char chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr2.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 >= c2);
}



/******************************************************************************/
/* operator < (PChar, PChar) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are <, false if not.                    */
/******************************************************************************/
bool operator < (const PChar &chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 < c2);
}



/******************************************************************************/
/* operator < (PChar, char) will compare the two characters.                  */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are <, false if not.                    */
/******************************************************************************/
bool operator < (const PChar &chr1, char chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr1.ConvertCharacter(chr2);

	return (c1 < c2);
}



/******************************************************************************/
/* operator < (char, PChar) will compare the two characters.                  */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are <, false if not.                    */
/******************************************************************************/
bool operator < (char chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr2.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 < c2);
}



/******************************************************************************/
/* operator > (PChar, PChar) will compare the two characters.                 */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are >, false if not.                    */
/******************************************************************************/
bool operator > (const PChar &chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 > c2);
}



/******************************************************************************/
/* operator > (PChar, char) will compare the two characters.                  */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are >, false if not.                    */
/******************************************************************************/
bool operator > (const PChar &chr1, char chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr1.ConvertCharacter(chr1);
	c2 = chr1.ConvertCharacter(chr2);

	return (c1 > c2);
}



/******************************************************************************/
/* operator > (char, PChar) will compare the two characters.                  */
/*                                                                            */
/* Input:  "chr1" is the first character.                                     */
/*         "chr2" is the second character.                                    */
/*                                                                            */
/* Output: True if the two characters are >, false if not.                    */
/******************************************************************************/
bool operator > (char chr1, const PChar &chr2)
{
	uint16 c1, c2;

	// First convert the two characters to unicode
	c1 = chr2.ConvertCharacter(chr1);
	c2 = chr2.ConvertCharacter(chr2);

	return (c1 > c2);
}



/******************************************************************************/
/* Init() initialize the object.                                              */
/******************************************************************************/
void PChar::Init(void)
{
	// Set the length to 0
	length = 0;
	memset(character, 0, P_MAX_CHAR_LEN);

	// Allocate a host character set
	charSet = CreateHostCharacterSet();
	if (charSet == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* ConvertCharacter() convert the character given to unicode.                 */
/*                                                                            */
/* Input:  "chr" is the character to convert.                                 */
/*                                                                            */
/* Output: The character in unicode.                                          */
/******************************************************************************/
uint16 PChar::ConvertCharacter(const PChar &chr) const
{
	int8 len;
	uint16 uniChar;

	if (chr.IsNull())
		return (0);

	try
	{
		uniChar = chr.charSet->ToUnicode(chr.character, len);
	}
	catch(PBoundsException e)
	{
		uniChar = 0x003f;
	}

	return (uniChar);
}



/******************************************************************************/
/* ConvertCharacter() convert the character given to unicode.                 */
/*                                                                            */
/* Input:  "chr" is the character to convert.                                 */
/*                                                                            */
/* Output: The character in unicode.                                          */
/******************************************************************************/
uint16 PChar::ConvertCharacter(char chr) const
{
	int8 len;
	uint16 uniChar;
	PCharacterSet *charSet;

	// Create the host character set
	charSet = CreateHostCharacterSet();
	if (charSet == NULL)
		throw PMemoryException();

	try
	{
		uniChar = charSet->ToUnicode((const char *)&chr, len);
	}
	catch(PBoundsException e)
	{
		uniChar = 0x003f;
	}

	// Delete the character set
	delete charSet;

	return (uniChar);
}



/******************************************************************************/
/* CheckCharacter() will check the character given to see if it the type as   */
/*      the flag given.                                                       */
/*                                                                            */
/* Input:  "uniChar" is the character to check.                               */
/*         "flag" is what to check the character against.                     */
/*                                                                            */
/* Output: True if the character is the type given, false if not.             */
/******************************************************************************/
bool PChar::CheckCharacter(uint16 uniChar, uint8 flag)
{
	const uint8 *typeTable;

	// Find the type table
	typeTable = typeHighByteIndex[uniChar >> 8];
	if (typeTable == NULL)
		return (false);

	// Now check the type
	if (typeTable[uniChar & 0x00ff] & flag)
		return (true);

	return (false);
}



/******************************************************************************/
/* ChangeCase() will change the case on the character given.                  */
/*                                                                            */
/* Input:  "uniChar" is the character to change.                              */
/*                                                                            */
/* Output: The changed character.                                             */
/******************************************************************************/
uint16 PChar::ChangeCase(uint16 uniChar)
{
	const uint16 *caseTable;
	uint16 newChar;

	// Find the case table
	caseTable = caseHighByteIndex[uniChar >> 8];
	if (caseTable == NULL)
		return (uniChar);

	// Now get the new character
	newChar = caseTable[uniChar & 0x00ff];
	if (newChar == 0)
		return (uniChar);

	return (newChar);
}





/******************************************************************************/
/* PString class                                                              */
/******************************************************************************/

/******************************************************************************/
/* String structure                                                           */
/******************************************************************************/
typedef struct PString::PStringData
{
	int32	refCount;		// Number of objects that points to this string
	int32	stringLen;		// Length of the string in characters
	int32	allocatedLen;	// Number of characters allocated
	uint16 *string;			// Pointer to the string which is always stored in unicode
} PStringData;



/******************************************************************************/
/* Empty string                                                               */
/******************************************************************************/
static PStringData pEmptyString =
{
	-1, 0, 0, NULL
};



/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
PString::PString(void)
{
	Initialize();
	SetToHostCharacterSet();
	CreateEmptyString();
}



/******************************************************************************/
/* Constructor which sets the string to the PString given.                    */
/*                                                                            */
/* Input:  "string" is a reference to another PString object.                 */
/******************************************************************************/
PString::PString(const PString &string)
{
	// Initialize the object
	Initialize();

	// Set the character set to the host
	SetToHostCharacterSet();

	if (string.IsEmpty())
		CreateEmptyString();
	else
	{
		// The reference counter should at least have one reference
		ASSERT(string.stringData->refCount > 0);

		// Count up reference counter
		AtomicIncrement(&string.stringData->refCount);

		// Copy pointer
		stringData = string.stringData;
	}
}



/******************************************************************************/
/* Constructor which sets the string to the string given.                     */
/*                                                                            */
/* Input:  "string" is a pointer to a null terminated string.                 */
/******************************************************************************/
PString::PString(const char *string)
{
	int32 strLen;

	// Initialize the object
	Initialize();

	// Set the character set to the host
	SetToHostCharacterSet();

	// Find the length of the string in bytes
	strLen = CountStringLength(string, charSet);

	if (strLen == 0)
		CreateEmptyString();
	else
	{
		// Allocate the string buffer
		AllocBuffer(strLen);
		CopyString(string, charSet);
	}
}



/******************************************************************************/
/* Constructor which sets the string to the string given in a specific        */
/*      character set.                                                        */
/*                                                                            */
/* Input:  "string" is a pointer to a null terminated string.                 */
/*         "characterSet" is a pointer to the character set.                  */
/******************************************************************************/
PString::PString(const char *string, PCharacterSet *characterSet)
{
	int32 strLen;

	// Initialize the object
	Initialize();

	// Set the character set to the host
	SetToHostCharacterSet();

	// Find the length of the string in bytes
	strLen = CountStringLength(string, characterSet);

	if (strLen == 0)
		CreateEmptyString();
	else
	{
		// Allocate the string buffer
		AllocBuffer(strLen);
		CopyString(string, characterSet);
	}
}



/******************************************************************************/
/* Constructor which sets the string to the character given.                  */
/*                                                                            */
/* Input:  "chr" is a reference to the character.                             */
/******************************************************************************/
PString::PString(const PChar &chr)
{
	// Initialize the object
	Initialize();

	// Set the character set to the host
	SetToHostCharacterSet();

	if (chr.IsNull())
		CreateEmptyString();
	else
	{
		// Allocate the string buffer
		AllocBuffer(1);
		CopyString(chr.GetChar(), chr.GetCharacterSet());
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PString::~PString(void)
{
	// Free the string data
	Release();

	// Delete the characterset
	delete charSet;
}



/******************************************************************************/
/* GetString() will allocate a buffer that will contain the current string    */
/*      in the character set the object have.                                 */
/*                                                                            */
/* Input:  "length" is a pointer where you want the length of the buffer to   */
/*         be stored, exclusive the NULL terminator. If NULL, no length will  */
/*         be given.                                                          */
/*                                                                            */
/* Output: A pointer to the buffer holding the string. The string will be     */
/*         NULL terminated using the character sets NULL terminator.          */
/*         You need to call FreeBuffer() to delete the pointer after you're   */
/*         done with it.                                                      */
/******************************************************************************/
char *PString::GetString(int32 *length) const
{
	return (GetString(charSet, length));
}



/******************************************************************************/
/* GetString() will allocate a buffer that will contain the current string    */
/*      in the character set given.                                           */
/*                                                                            */
/* Input:  "characterSet" is the character set you want to use.               */
/*         "length" is a pointer where you want the length of the buffer to   */
/*         be stored, exclusive the NULL terminator. If NULL, no length will  */
/*         be given.                                                          */
/*                                                                            */
/* Output: A pointer to the buffer holding the string. The string will be     */
/*         NULL terminated using the character sets NULL terminator.          */
/*         You need to call FreeBuffer() to delete the pointer after you're   */
/*         done with it.                                                      */
/******************************************************************************/
char *PString::GetString(PCharacterSet *characterSet, int32 *length) const
{
	int32 allocLen, strLen;
	char *convString;

	// Find out how many bytes to allocate
	allocLen = stringData->stringLen * P_MAX_CHAR_LEN;

	// Allocate the buffer
	convString = new char[allocLen + P_MAX_CHAR_LEN];
	if (convString == NULL)
		throw PMemoryException();

	// Convert the string object to the character set assigned and
	// store it in the new buffer
	CreateString(convString, strLen, characterSet);

	// Should we store the length?
	if (length != NULL)
		*length = strLen;

	return (convString);
}



/******************************************************************************/
/* FreeBuffer() will free the buffer returned by the GetString() function.    */
/*                                                                            */
/* Input:  "buffer" is a pointer to the buffer to free. NULL is a valid       */
/*         pointer to give.                                                   */
/******************************************************************************/
void PString::FreeBuffer(char *buffer) const
{
	delete[] buffer;
}



/******************************************************************************/
/* SetString() will store the string given into the current object.           */
/*                                                                            */
/* Input:  "string" is the string to store in the object.                     */
/*         "characterSet" is the character set the string is in.              */
/******************************************************************************/
void PString::SetString(const char *string, PCharacterSet *characterSet)
{
	int32 strLen;
	PCharacterSet *charSet;

	// If no character set is given, use the host character set
	if (characterSet == NULL)
	{
		charSet = CreateHostCharacterSet();
		if (charSet == NULL)
			throw PMemoryException();
	}
	else
		charSet = characterSet;

	// Make make sure the current string is released
	Release();

	// Find the length of the string in bytes
	strLen = CountStringLength(string, charSet);

	if (strLen != 0)
	{
		// Allocate the string buffer
		AllocBuffer(strLen);
		CopyString(string, charSet);
	}

	// If we use the host character set, delete it
	if (characterSet == NULL)
		delete charSet;
}



/******************************************************************************/
/* SetString() will store the string given into the current object.           */
/*                                                                            */
/* Input:  "string" is the string to store in the object.                     */
/*         "length" is the length of the string in bytes.                     */
/*         "characterSet" is the character set the string is in.              */
/******************************************************************************/
void PString::SetString(const char *string, int32 length, PCharacterSet *characterSet)
{
	PCharacterSet *charSet;

	// If no character set is given, use the host character set
	if (characterSet == NULL)
	{
		charSet = CreateHostCharacterSet();
		if (charSet == NULL)
			throw PMemoryException();
	}
	else
		charSet = characterSet;

	// Make make sure the current string is released
	Release();

	if (length != 0)
	{
		// Allocate the string buffer
		AllocBuffer(length);
		CopyString(string, length, charSet);
	}

	// If we use the host character set, delete it
	if (characterSet == NULL)
		delete charSet;
}



/******************************************************************************/
/* SwitchCharacterSet() change the character set used in the current object.  */
/*                                                                            */
/* Input:  "characterSet" is the character set to use.                        */
/******************************************************************************/
void PString::SwitchCharacterSet(const PCharacterSet *characterSet)
{
	// First delete the old character set
	delete charSet;

	// Now allocate a new one
	charSet = characterSet->CreateObject();
	if (charSet == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* SwitchToHostCharacterSet() change the character set used in the current    */
/*      object to the host character set.                                     */
/******************************************************************************/
void PString::SwitchToHostCharacterSet(void)
{
	// Delete the old character set
	delete charSet;

	// Now allocate a new one
	SetToHostCharacterSet();
}



/******************************************************************************/
/* ForceStringCopy() will force a copy of the string in the object to a new   */
/*      buffer.                                                               */
/******************************************************************************/
void PString::ForceStringCopy(void)
{
	// Check to see if the string is empty
	if (stringData->string == NULL)
	{
		// It is, then make sure that we use our own empty structure
		CreateEmptyString();
	}
	else
		CopyBeforeWrite();
}



/******************************************************************************/
/* GetLength() returns the length of the string in characters.                */
/*                                                                            */
/* Output: The length in characters of the string.                            */
/******************************************************************************/
int32 PString::GetLength(void) const
{
	return (stringData->stringLen);
}



/******************************************************************************/
/* GetByteLength() returns the length of the string in bytes when encoded     */
/*      with the character set given.                                         */
/*                                                                            */
/* Input:  "characterSet" is a pointer to the character set to use.           */
/*                                                                            */
/* Output: The length of the string in bytes.                                 */
/******************************************************************************/
int32 PString::GetByteLength(PCharacterSet *characterSet) const
{
	int32 i, count;
	int8 len;
	int32 length = 0;

	// Null pointer not allowed
	ASSERT(characterSet != NULL);

	// Traverse the string and count the number of bytes
	count = stringData->stringLen;
	for (i = 0; i < count; i++)
	{
		// Get the length of the character
		characterSet->FromUnicode(stringData->string[i], len);

		// Add the length
		length += len;
	}

	return (length);
}



/******************************************************************************/
/* GetAt() will return the character at the index given.                      */
/*                                                                            */
/* Input:  "index" is the index in characters into the string.                */
/*                                                                            */
/* Output: The character at the index.                                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
PChar PString::GetAt(int32 index) const
{
	const char *string;
	int8 len;

	// Check for out of boundaries
	if ((index < 0) || (index > stringData->stringLen))
		throw PBoundsException();

	// Convert the character from unicode to the current character set
	// and return it in a PChar object
	string = charSet->FromUnicode(stringData->string[index], len);
	return (PChar(string, len, charSet));
}



/******************************************************************************/
/* SetAt() change a single character at the index given.                      */
/*                                                                            */
/* Input:  "index" is the index in characters into the string.                */
/*         "chr" is the character to store.                                   */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
void PString::SetAt(int32 index, PChar chr)
{
	int8 len;
	uint16 uniChar;

	// Check for out of boundaries
	if ((index < 0) || (index > stringData->stringLen))
		throw PBoundsException();

	// First make sure we have our own buffer
	CopyBeforeWrite();

	try
	{
		// Now change the character by converting it to unicode
		uniChar = chr.GetCharacterSet()->ToUnicode(chr.GetChar(), len);
	}
	catch(PBoundsException e)
	{
		uniChar = 0x003f;
	}

	// Store the new character in the string
	stringData->string[index] = uniChar;
}



/******************************************************************************/
/* IsEmpty() will check the string to see if it's empty.                      */
/*                                                                            */
/* Output: True if the string is empty, else false.                           */
/******************************************************************************/
bool PString::IsEmpty(void) const
{
	// Empty if the length of the string is 0
	if (stringData->stringLen == 0)
		return (true);

	// Nop, the string isn't empty
	return (false);
}



/******************************************************************************/
/* MakeEmpty() will force the string to be empty.                             */
/******************************************************************************/
void PString::MakeEmpty(void)
{
	// Release the buffer. This will also set the string to be empty
	Release();
}



/******************************************************************************/
/* MakeLower() will make the string lowercase.                                */
/******************************************************************************/
void PString::MakeLower(void)
{
	int32 i, count;

	// Well, make sure we can write to the string
	CopyBeforeWrite();

	// Get the number of characters
	count = stringData->stringLen;

	// Change all the characters to lower case
	for (i = 0; i < count; i++)
		stringData->string[i] = PChar::MakeLower(stringData->string[i]);
}



/******************************************************************************/
/* MakeUpper() will make the string uppercase.                                */
/******************************************************************************/
void PString::MakeUpper(void)
{
	int32 i, count;

	// Well, make sure we can write to the string
	CopyBeforeWrite();

	// Get the number of characters
	count = stringData->stringLen;

	// Change all the characters to upper case
	for (i = 0; i < count; i++)
		stringData->string[i] = PChar::MakeUpper(stringData->string[i]);
}



/******************************************************************************/
/* GetNumber() will try to read a signed number out of the string.            */
/*                                                                            */
/* Input:  "index" is where in the string to start reading.                   */
/*                                                                            */
/* Output: The number.                                                        */
/******************************************************************************/
int32 PString::GetNumber(int32 index) const
{
	return ((int32)GetNumber64(index));
}



/******************************************************************************/
/* GetUNumber() will try to read a number out of the string.                  */
/*                                                                            */
/* Input:  "index" is where in the string to start reading.                   */
/*                                                                            */
/* Output: The number.                                                        */
/******************************************************************************/
uint32 PString::GetUNumber(int32 index) const
{
	return ((uint32)GetUNumber64(index));
}



/******************************************************************************/
/* GetNumber64() will try to read a signed 64-bit number out of the string.   */
/*                                                                            */
/* Input:  "index" is where in the string to start reading.                   */
/*                                                                            */
/* Output: The number.                                                        */
/******************************************************************************/
int64 PString::GetNumber64(int32 index) const
{
	uint16 chr;
	int32 i;
	int64 number = 0;
	bool sign = false;

	// Check for out of bounds
	if ((index < 0) || (index >= stringData->stringLen))
		return (0);

	// Check the first character
	chr = stringData->string[index];
	if (chr == 0x002d)				// Minus
	{
		index++;
		sign = true;
	}
	else
	{
		if (chr == 0x002b)			// Plus
			index++;
	}

	// Take each character until we reach the end
	// or a non-number character is reached
	for (i = index; i < stringData->stringLen; i++)
	{
		// Get the character
		chr = stringData->string[i];

		// Is it a number
		if ((chr < 0x0030) || (chr > 0x0039))
			break;

		// Add the number
		number *= 10;
		number += (chr - 0x0030);
	}

	if (sign)
		number = -number;

	return (number);
}



/******************************************************************************/
/* GetUNumber64() will try to read a 64-bit number out of the string.         */
/*                                                                            */
/* Input:  "index" is where in the string to start reading.                   */
/*                                                                            */
/* Output: The number.                                                        */
/******************************************************************************/
uint64 PString::GetUNumber64(int32 index) const
{
	uint16 chr;
	int32 i;
	uint64 number = 0;

	// Check for out of bounds
	if ((index < 0) || (index >= stringData->stringLen))
		return (0);

	// Take each character until we reach the end
	// or a non-number character is reached
	for (i = index; i < stringData->stringLen; i++)
	{
		// Get the character
		chr = stringData->string[i];

		// Is it a number
		if ((chr < 0x0030) || (chr > 0x0039))
			break;

		// Add the number
		number *= 10;
		number += (chr - 0x0030);
	}

	return (number);
}



/******************************************************************************/
/* GetHexNumber() will try to read a hexadecimal number out of the string.    */
/*                                                                            */
/* Input:  "index" is where in the string to start reading.                   */
/*                                                                            */
/* Output: The number.                                                        */
/******************************************************************************/
uint32 PString::GetHexNumber(int32 index) const
{
	return (GetHexNumber64(index));
}



/******************************************************************************/
/* GetHexNumber64() will try to read a 64-bit hexadecimal number out of the   */
/*      string.                                                               */
/*                                                                            */
/* Input:  "index" is where in the string to start reading.                   */
/*                                                                            */
/* Output: The number.                                                        */
/******************************************************************************/
uint64 PString::GetHexNumber64(int32 index) const
{
	uint16 chr;
	int32 i;
	uint64 number = 0;

	// Check for out of bounds
	if ((index < 0) || (index >= stringData->stringLen))
		return (0);

	// Take each character until we reach the end
	// or an non-number character is reached
	for (i = index; i < stringData->stringLen; i++)
	{
		// Get the character
		chr = stringData->string[i];

		// Is it a valid character
		if (((chr < 0x0030) || (chr > 0x0039)) && ((chr < 0x0041) || (chr > 0x0046)) && ((chr < 0x0061) || (chr > 0x0066)))
			break;

		// Add the number
		if (chr >= 0x0061)
			chr -= 87;
		else
		{
			if (chr >= 0x0041)
				chr -= 55;
			else
				chr -= 48;
		}

		number *= 16;
		number += chr;
	}

	return (number);
}



/******************************************************************************/
/* SetNumber() will store the signed number given into the current string.    */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "thousandFix" is true if you want a separator for each thousands.  */
/******************************************************************************/
void PString::SetNumber(int32 number, bool thousandFix)
{
	// Use the unsigned function
	SetUNumber(number < 0 ? -number : number, thousandFix);

	// Is the number negative?
	if (number < 0)
	{
		// Insert the sign
		Insert(0, '-');
	}
}



/******************************************************************************/
/* SetNumber64() will store the signed 64-bit number given into the current   */
/*      string.                                                               */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "thousandFix" is true if you want a separator for each thousands.  */
/******************************************************************************/
void PString::SetNumber64(int64 number, bool thousandFix)
{
	// Use the unsigned function
	SetUNumber64(number < 0 ? -number : number, thousandFix);

	// Is the number negative?
	if (number < 0)
	{
		// Insert the sign
		Insert(0, '-');
	}
}



/******************************************************************************/
/* SetUNumber() will store the unsigned number given into the current string. */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "thousandFix" is true if you want a separator for each thousands.  */
/******************************************************************************/
void PString::SetUNumber(uint32 number, bool thousandFix)
{
	uint16 *buffer;
	uint16 tempNum;
	uint16 i;
	bool searching = true;

	// Start to release the string
	Release();

	// Now make sure we have a buffer big enough to hold the maximum number
	AllocBuffer(1 + 10 + 3);	// Sign + number + commas

	// Initialize the buffer
	buffer = stringData->string;

	// Begin to convert the number
	for (i = 0; i < 9; i++)
	{
		if ((!searching) || (number >= decimal[i]))
		{
			tempNum   = (uint16)(number / decimal[i]);
			number   -= tempNum * decimal[i];
			*buffer++ = tempNum + 0x0030;

			if (thousandFix && ((i % 3) == 0))
				*buffer++ = 0x0021;	// '!'

			searching = false;
		}
	}

	// Always store the 'ones'
	*buffer++ = (uint16)number + 0x0030;

	// Calculate the string length
	stringData->stringLen = (buffer - stringData->string);

	// Null terminate the string
	stringData->string[stringData->stringLen] = 0x0000;

	if (thousandFix)
	{
		// Now replace the '!' character with the right separator
		Replace("!", PSystem::GetSystemLocaleString(PSystem::pLocale_Thousand));
	}
}



/******************************************************************************/
/* SetUNumber64() will store the unsigned 64-bit number given into the        */
/*      current string.                                                       */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "thousandFix" is true if you want a separator for each thousands.  */
/******************************************************************************/
void PString::SetUNumber64(uint64 number, bool thousandFix)
{
	uint16 *buffer;
	uint64 checkNum;
	uint16 tempNum;
	uint16 i;
	bool searching = true;

	// Start to release the string
	Release();

	// Now make sure we have a buffer big enough to hold the maximum number
	AllocBuffer(1 + 20 + 6);	// Sign + number + commas

	// Initialize the buffer
	buffer = stringData->string;

	// Calculate the start number (compiler can't handle so big numbers)
	checkNum = 1;
	for (i = 0; i < 19; i++)
		checkNum *= 10;

	// Begin to convert the number
	for (i = 0; checkNum >= 10; checkNum /= 10, i++)
	{
		if ((!searching) || (number >= checkNum))
		{
			tempNum   = (uint16)(number / checkNum);
			number   -= tempNum * checkNum;
			*buffer++ = tempNum + 0x0030;

			if (thousandFix && ((i % 3) == 0))
				*buffer++ = 0x0021;	// '!'

			searching = false;
		}
	}

	// Always store the 'ones'
	*buffer++ = (uint16)number + 0x0030;

	// Calculate the string length
	stringData->stringLen = (buffer - stringData->string);

	// Null terminate the string
	stringData->string[stringData->stringLen] = 0x0000;

	if (thousandFix)
	{
		// Now replace the '!' character with the right separator
		Replace("!", PSystem::GetSystemLocaleString(PSystem::pLocale_Thousand));
	}
}



/******************************************************************************/
/* SetHexNumber() will store the number given into the current string.        */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "upper" indicates if you want the letters in upper or lower case.  */
/******************************************************************************/
void PString::SetHexNumber(uint32 number, bool upper)
{
	SetHexNumber64(number, upper);
}



/******************************************************************************/
/* SetHexNumber64() will store the 64-bit number given into the current       */
/*      string.                                                               */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "upper" indicates if you want the letters in upper or lower case.  */
/******************************************************************************/
void PString::SetHexNumber64(uint64 number, bool upper)
{
	PCharSet_UNICODE charSet;
	int32 len;
	uint16 buffer[17];

	// Build the number
	ToHex(buffer, len, number, upper);
	buffer[len] = 0x0000;
	SetString((char *)buffer, &charSet);
}



/******************************************************************************/
/* CreateNumber() will store the signed number given into a string and return */
/*      it.                                                                   */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "thousandFix" is true if you want a separator for each thousands.  */
/*                                                                            */
/* Output: A string with the number.                                          */
/******************************************************************************/
PString PString::CreateNumber(int32 number, bool thousandFix)
{
	PString result;

	// Build the number
	result.SetNumber(number, thousandFix);

	// And return it
	return (result);
}



/******************************************************************************/
/* CreateUNumber() will store the unsigned number given into a string and     */
/*      return it.                                                            */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "thousandFix" is true if you want a separator for each thousands.  */
/*                                                                            */
/* Output: A string with the number.                                          */
/******************************************************************************/
PString PString::CreateUNumber(uint32 number, bool thousandFix)
{
	PString result;

	// Build the number
	result.SetUNumber(number, thousandFix);

	// And return it
	return (result);
}



/******************************************************************************/
/* CreateNumber64() will store the 64-bit signed number given into a string   */
/*      and return it.                                                        */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "thousandFix" is true if you want a separator for each thousands.  */
/*                                                                            */
/* Output: A string with the number.                                          */
/******************************************************************************/
PString PString::CreateNumber64(int64 number, bool thousandFix)
{
	PString result;

	// Build the number
	result.SetNumber64(number, thousandFix);

	// And return it
	return (result);
}



/******************************************************************************/
/* CreateUNumber64() will store the 64-bit unsigned number given into a       */
/*      string and return it.                                                 */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "thousandFix" is true if you want a separator for each thousands.  */
/*                                                                            */
/* Output: A string with the number.                                          */
/******************************************************************************/
PString PString::CreateUNumber64(uint64 number, bool thousandFix)
{
	PString result;

	// Build the number
	result.SetUNumber64(number, thousandFix);

	// And return it
	return (result);
}



/******************************************************************************/
/* CreateHexNumber() will store the number given into a string in hexadecimal */
/*      and return it.                                                        */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "upper" indicates if you want the letters in upper or lower case.  */
/*                                                                            */
/* Output: A string with the number.                                          */
/******************************************************************************/
PString PString::CreateHexNumber(uint32 number, bool upper)
{
	PString result;

	// Build the number
	result.SetHexNumber(number, upper);

	// And return it
	return (result);
}



/******************************************************************************/
/* CreateHexNumber64() will store the 64-bit number given into a string in    */
/*      hexadecimal and return it.                                            */
/*                                                                            */
/* Input:  "number" is the number to store.                                   */
/*         "upper" indicates if you want the letters in upper or lower case.  */
/*                                                                            */
/* Output: A string with the number.                                          */
/******************************************************************************/
PString PString::CreateHexNumber64(uint64 number, bool upper)
{
	PString result;

	// Build the number
	result.SetHexNumber64(number, upper);

	// And return it
	return (result);
}



/******************************************************************************/
/* Format() will assign the string with the text given and format it. The     */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "..." is the arguments to put in the format string.                */
/******************************************************************************/
void PString::Format(PString formatString, ...)
{
	va_list argList;

	// Set argList to the first argument
	// TODO : I think this doesn't work.
	va_start(argList, formatString);

	// Format the string
	FormatIt(formatString, argList);

	// End argument list parsing
	va_end(argList);
}



/******************************************************************************/
/* Format() will assign the string with the text given and format it. The     */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "..." is the arguments to put in the format string.                */
/******************************************************************************/
void PString::Format(const char *formatString, ...)
{
	va_list argList;

	// Null pointer not allowed
	ASSERT(formatString != NULL);

	// Set argList to the first argument
	va_start(argList, formatString);

	// Format the string
	FormatIt(formatString, argList);

	// End argument list parsing
	va_end(argList);
}



/******************************************************************************/
/* Format() will assign the string with the text given and format it. The     */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource to use.                    */
/*         "id" is the string ID to retrieve.                                 */
/*         "..." is the arguments to put in the format string.                */
/******************************************************************************/
void PString::Format(PResource *resource, int32 id, ...)
{
	va_list argList;
	PString formatString;

	// Null pointer not allowed
	ASSERT(resource != NULL);

	// Set argList to the first argument
	va_start(argList, id);

	// Load the string from the resource
	formatString.LoadString(resource, id);

	// Format the string
	FormatIt(formatString, argList);

	// End argument list parsing
	va_end(argList);
}



/******************************************************************************/
/* Format_S1() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S1(PString formatString, PString str1)
{
	char *str1Poi = str1.GetString();

	Format(formatString, str1Poi);

	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S1() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S1(const char *formatString, PString str1)
{
	char *str1Poi = str1.GetString();

	Format(formatString, str1Poi);

	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S1() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource to use.                    */
/*         "id" is the string ID to retrieve.                                 */
/*         "str1" is the first string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S1(PResource *resource, int32 id, PString str1)
{
	char *str1Poi = str1.GetString();

	Format(resource, id, str1Poi);

	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S2() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/******************************************************************************/
void PString::Format_S2(PString formatString, PString str1, PString str2)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();

	Format(formatString, str1Poi, str2Poi);

	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S2() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/******************************************************************************/
void PString::Format_S2(const char *formatString, PString str1, PString str2)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();

	Format(formatString, str1Poi, str2Poi);

	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S2() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource to use.                    */
/*         "id" is the string ID to retrieve.                                 */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/******************************************************************************/
void PString::Format_S2(PResource *resource, int32 id, PString str1, PString str2)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();

	Format(resource, id, str1Poi, str2Poi);

	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S3() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S3(PString formatString, PString str1, PString str2, PString str3)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();

	Format(formatString, str1Poi, str2Poi, str3Poi);

	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S3() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S3(const char *formatString, PString str1, PString str2, PString str3)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();

	Format(formatString, str1Poi, str2Poi, str3Poi);

	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S3() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource to use.                    */
/*         "id" is the string ID to retrieve.                                 */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S3(PResource *resource, int32 id, PString str1, PString str2, PString str3)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();

	Format(resource, id, str1Poi, str2Poi, str3Poi);

	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S4() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/*         "str4" is the fourth string argument to put in the format string.  */
/******************************************************************************/
void PString::Format_S4(PString formatString, PString str1, PString str2, PString str3, PString str4)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();
	char *str4Poi = str4.GetString();

	Format(formatString, str1Poi, str2Poi, str3Poi, str4Poi);

	str4.FreeBuffer(str4Poi);
	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S4() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/*         "str4" is the fourth string argument to put in the format string.  */
/******************************************************************************/
void PString::Format_S4(const char *formatString, PString str1, PString str2, PString str3, PString str4)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();
	char *str4Poi = str4.GetString();

	Format(formatString, str1Poi, str2Poi, str3Poi, str4Poi);

	str4.FreeBuffer(str4Poi);
	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S4() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource to use.                    */
/*         "id" is the string ID to retrieve.                                 */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/*         "str4" is the fourth string argument to put in the format string.  */
/******************************************************************************/
void PString::Format_S4(PResource *resource, int32 id, PString str1, PString str2, PString str3, PString str4)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();
	char *str4Poi = str4.GetString();

	Format(resource, id, str1Poi, str2Poi, str3Poi, str4Poi);

	str4.FreeBuffer(str4Poi);
	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S5() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/*         "str4" is the fourth string argument to put in the format string.  */
/*         "str5" is the fifth string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S5(PString formatString, PString str1, PString str2, PString str3, PString str4, PString str5)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();
	char *str4Poi = str4.GetString();
	char *str5Poi = str5.GetString();

	Format(formatString, str1Poi, str2Poi, str3Poi, str4Poi, str5Poi);

	str5.FreeBuffer(str5Poi);
	str4.FreeBuffer(str4Poi);
	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S5() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/*         "str4" is the fourth string argument to put in the format string.  */
/*         "str5" is the fifth string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S5(const char *formatString, PString str1, PString str2, PString str3, PString str4, PString str5)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();
	char *str4Poi = str4.GetString();
	char *str5Poi = str5.GetString();

	Format(formatString, str1Poi, str2Poi, str3Poi, str4Poi, str5Poi);

	str5.FreeBuffer(str5Poi);
	str4.FreeBuffer(str4Poi);
	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* Format_S5() will assign the string with the text given and format it. The  */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource to use.                    */
/*         "id" is the string ID to retrieve.                                 */
/*         "str1" is the first string argument to put in the format string.   */
/*         "str2" is the second string argument to put in the format string.  */
/*         "str3" is the third string argument to put in the format string.   */
/*         "str4" is the fourth string argument to put in the format string.  */
/*         "str5" is the fifth string argument to put in the format string.   */
/******************************************************************************/
void PString::Format_S5(PResource *resource, int32 id, PString str1, PString str2, PString str3, PString str4, PString str5)
{
	char *str1Poi = str1.GetString();
	char *str2Poi = str2.GetString();
	char *str3Poi = str3.GetString();
	char *str4Poi = str4.GetString();
	char *str5Poi = str5.GetString();

	Format(resource, id, str1Poi, str2Poi, str3Poi, str4Poi, str5Poi);

	str5.FreeBuffer(str5Poi);
	str4.FreeBuffer(str4Poi);
	str3.FreeBuffer(str3Poi);
	str2.FreeBuffer(str2Poi);
	str1.FreeBuffer(str1Poi);
}



/******************************************************************************/
/* FormatV() will assign the string with the text given and format it. The    */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "argList" is the arguments to put in the format string.            */
/******************************************************************************/
void PString::FormatV(PString formatString, va_list argList)
{
	// Format the string
	FormatIt(formatString, argList);
}



/******************************************************************************/
/* FormatV() will assign the string with the text given and format it. The    */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "argList" is the arguments to put in the format string.            */
/******************************************************************************/
void PString::FormatV(const char *formatString, va_list argList)
{
	// Null pointer not allowed
	ASSERT(formatString != NULL);

	// Format the string
	FormatIt(formatString, argList);
}



/******************************************************************************/
/* FormatV() will assign the string with the text given and format it. The    */
/*      string given is the same type as with the sprintf() function.         */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource to use.                    */
/*         "id" is the string ID to retrieve.                                 */
/*         "argList" is the arguments to put in the format string.            */
/******************************************************************************/
void PString::FormatV(PResource *resource, int32 id, va_list argList)
{
	PString formatString;

	// Null pointer not allowed
	ASSERT(resource != NULL);

	// Load the string from the resource
	formatString.LoadString(resource, id);

	// Format the string
	FormatIt(formatString, argList);
}



/******************************************************************************/
/* LoadString() will get a string from the resources and store it in the      */
/*      current object.                                                       */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource to use.                    */
/*         "id" is the string ID to retrieve.                                 */
/******************************************************************************/
void PString::LoadString(PResource *resource, int32 id)
{
	int32 length;
	char *buffer;

	// Null pointer not allowed
	ASSERT(resource != NULL);

	// First make sure that the string resource is loaded
	resource->LoadResource(P_RES_STRING);

	// Get the length of the resource string
	length = resource->GetItemLength(P_RES_STRING, id);
	if (length == 0)
	{
		MakeEmpty();
		return;
	}

	// Allocate a temporary buffer to hold the string
	buffer = new char[length];
	if (buffer == NULL)
		throw PMemoryException();

	try
	{
		// Get the string
		resource->GetItem(P_RES_STRING, id, buffer, length);

		// Store the string in the object
		SetString(buffer);
	}
	catch(...)
	{
		delete[] buffer;
		throw;
	}

	// Delete the temporary buffer
	delete[] buffer;
}



/******************************************************************************/
/* Replace() search the current string after the search character given and   */
/*      replaces all instance of it with the replace character.               */
/*                                                                            */
/* Input:  "search" is the character to search after.                         */
/*         "replace" is what to replace the search character with.            */
/*                                                                            */
/* Output: Number of replaced instances.                                      */
/******************************************************************************/
int32 PString::Replace(PChar search, PChar replace)
{
	int32 replaceCount = 0;
	int32 i, count;
	uint16 searchChr, replaceChr;
	int8 len;

	try
	{
		// Get the characters as unicode
		searchChr  = search.GetCharacterSet()->ToUnicode(search.GetChar(), len);
		replaceChr = replace.GetCharacterSet()->ToUnicode(replace.GetChar(), len);

		// Should we replace anything at all?
		if (searchChr != replaceChr)
		{
			// First make sure we have write access to the string
			CopyBeforeWrite();

			// Find all search characters and replace them
			count = stringData->stringLen;
			for (i = 0; i < count; i++)
			{
				// Check the character
				if (stringData->string[i] == searchChr)
				{
					// Replace the character
					stringData->string[i] = replaceChr;
					replaceCount++;
				}
			}
		}
	}
	catch(PBoundsException e)
	{
		;
	}

	return (replaceCount);
}



/******************************************************************************/
/* Replace() search the current string after the search string given and      */
/*      replaces all instance of it with the replace string.                  */
/*                                                                            */
/* Input:  "search" is the string to search after.                            */
/*         "replace" is what to replace the search string with.               */
/*                                                                            */
/* Output: Number of replaced instances.                                      */
/******************************************************************************/
int32 PString::Replace(PString search, PString replace)
{
	int32 replaceCount = 0;
	int32 newStringLen;
	int32 searchLen, replaceLen;
	int32 i, j, count;

	// Should we search after anything at all?
	if (search.IsEmpty())
		return (0);

	// Get the work string lengths
	searchLen  = search.GetLength();
	replaceLen = replace.GetLength();

	// Loop through the string to see how many times
	// we need to make a replacement and count the new
	// string length
	count = stringData->stringLen - searchLen + 1;
	for (i = 0; i < count; i++)
	{
		// Check the first character
		if (stringData->string[i] == search.stringData->string[0])
		{
			// The first character is a match, now test the rest
			bool found = true;

			for (j = 1; j < searchLen; j++)
			{
				if (stringData->string[i + j] != search.stringData->string[j])
				{
					found = false;
					break;
				}
			}

			// Found the search string?
			if (found)
			{
				// Skip the search string and increment the replace counter
				i += (searchLen - 1);
				replaceCount++;
			}
		}
	}

	// Well, should we replace any strings?
	if (replaceCount > 0)
	{
		// Calculate the new string length
		newStringLen = stringData->stringLen - (replaceCount * searchLen) + (replaceCount * replaceLen);

		// Make sure we can write to the string
		EnlargeBeforeWrite(newStringLen);

		// Do the second time search and them replace the instances
		for (i = 0; i < count; i++)
		{
			// Check the first character
			if (stringData->string[i] == search.stringData->string[0])
			{
				// The first character is a match, now test the rest
				bool found = true;

				for (j = 1; j < searchLen; j++)
				{
					if (stringData->string[i + j] != search.stringData->string[j])
					{
						found = false;
						break;
					}
				}

				// Found the search string?
				if (found)
				{
					// Okay, now move the rest of the string to the
					// right place
					memmove(&stringData->string[i + replaceLen], &stringData->string[i + searchLen], (stringData->stringLen - i - searchLen) * sizeof(uint16));

					// Now copy the replace string into the right spot
					memcpy(&stringData->string[i], replace.stringData->string, replaceLen * sizeof(uint16));

					// Skip the replace string
					i     += (replaceLen - 1);
					count -= (searchLen - replaceLen);

					// Update the string length
					stringData->stringLen -= (searchLen - replaceLen);
				}
			}
		}

		// Null terminate the string
		stringData->string[newStringLen] = 0x0000;
	}

	return (replaceCount);
}



/******************************************************************************/
/* Remove() search the current string after the character given and removes   */
/*      any instances of it.                                                  */
/*                                                                            */
/* Input:  "chr" is the character to remove.                                  */
/*                                                                            */
/* Output: Number of removed characters.                                      */
/******************************************************************************/
int32 PString::Remove(PChar chr)
{
	int32 removeCount = 0;
	int32 i, count;
	uint16 unicodeChr;
	int8 len;
	uint16 *source, *dest;

	// Can we remove anything at all?
	if (IsEmpty())
		return (0);

	try
	{
		// Get the character in unicode
		unicodeChr = chr.GetCharacterSet()->ToUnicode(chr.GetChar(), len);

		// First make sure we have write access to the string
		CopyBeforeWrite();

		// Initialize pointers
		source = stringData->string;
		dest   = source;

		// Find all instances of the character given and replace them
		count = stringData->stringLen;
		for (i = 0; i < count; i++)
		{
			// Check the character
			if (*source != unicodeChr)
			{
				// Copy the character
				*dest++ = *source++;
			}
			else
			{
				// Skip the character
				source++;

				// Increment the counter and decrement the string length
				removeCount++;
				stringData->stringLen--;
			}
		}

		// Null terminate the string
		stringData->string[stringData->stringLen] = 0x0000;
	}
	catch(PBoundsException e)
	{
		;
	}

	return (removeCount);
}



/******************************************************************************/
/* Insert() inserts the character given into the current string.              */
/*                                                                            */
/* Input:  "index" is where in the current string you want to insert the      */
/*         character.                                                         */
/*         "chr" is the character to insert.                                  */
/******************************************************************************/
void PString::Insert(int32 index, PChar chr)
{
	int8 len;
	uint16 uniChar;

	// Fix out of bounds
	if (index < 0)
		index = 0;

	if (index > stringData->stringLen)
		index = stringData->stringLen;

	// Make sure the buffer is big enough
	EnlargeBeforeWrite(stringData->stringLen + 1);

	// Make room for the new character
	memmove(&stringData->string[index + 1], &stringData->string[index], (stringData->stringLen - index) * sizeof(uint16));

	try
	{
		// Convert the character to unicode
		uniChar = chr.GetCharacterSet()->ToUnicode(chr.GetChar(), len);
	}
	catch(PBoundsException e)
	{
		uniChar = 0x003f;
	}

	// Insert the new character
	stringData->string[index] = uniChar;

	// Adjust the string length
	stringData->stringLen++;

	// Null terminate the string
	stringData->string[stringData->stringLen] = 0x0000;
}



/******************************************************************************/
/* Insert() inserts the string given into the current string.                 */
/*                                                                            */
/* Input:  "index" is where in the current string you want to insert the      */
/*         string.                                                            */
/*         "string" is the string to insert.                                  */
/******************************************************************************/
void PString::Insert(int32 index, PString string)
{
	int32 strLen;

	// Fix out of bounds
	if (index < 0)
		index = 0;

	if (index > stringData->stringLen)
		index = stringData->stringLen;

	// Get the insert string length
	strLen = string.GetLength();

	// Make sure the buffer is big enough
	EnlargeBeforeWrite(stringData->stringLen + strLen);

	// Make room for the new character
	memmove(&stringData->string[index + strLen], &stringData->string[index], (stringData->stringLen - index) * sizeof(uint16));

	// Insert the new string
	memcpy(&stringData->string[index], string.stringData->string, strLen * sizeof(uint16));

	// Adjust the string length
	stringData->stringLen += strLen;

	// Null terminate the string
	stringData->string[stringData->stringLen] = 0x0000;
}



/******************************************************************************/
/* Delete() delete some characters in the current string.                     */
/*                                                                            */
/* Input:  "index" is the start position where you want to delete.            */
/*         "count" is the number of characters to delete.                     */
/******************************************************************************/
void PString::Delete(int32 index, int32 count)
{
	// Is the string empty?
	if (IsEmpty())
		return;

	// Should we delete anything at all?
	if (count <= 0)
		return;

	// Fix out of bounds
	if (index < 0)
		index = 0;

	if (index > stringData->stringLen)
		index = stringData->stringLen;

	if (count > (stringData->stringLen - index))
		count = stringData->stringLen - index;

	// Make sure we can write to the buffer
	CopyBeforeWrite();

	// Remove the part of the string
	memcpy(&stringData->string[index], &stringData->string[index + count], (stringData->stringLen - index - count) * sizeof(uint16));

	// Adjust the string length
	stringData->stringLen -= count;

	// Null terminate the string
	stringData->string[stringData->stringLen] = 0x0000;
}



/******************************************************************************/
/* Find() will search for a character in the string and return the position   */
/*      where it found it.                                                    */
/*                                                                            */
/* Input:  "chr" is the character to search for.                              */
/*         "startIndex" is the start position in the string to start the      */
/*         search. The character at the position given is included in the     */
/*         search.                                                            */
/*                                                                            */
/* Output: The position in the string where the character is found or -1 if   */
/*         not found.                                                         */
/******************************************************************************/
int32 PString::Find(PChar chr, int32 startIndex) const
{
	int32 i, count;
	uint16 uniChar;
	int8 len;

	// Check for "out of bounds"
	ASSERT(startIndex >= 0);
	ASSERT(startIndex <= stringData->stringLen);

	try
	{
		// Convert the character to search after to unicode
		uniChar = chr.GetCharacterSet()->ToUnicode(chr.GetChar(), len);

		count = stringData->stringLen;
		for (i = startIndex; i < count; i++)
		{
			if (stringData->string[i] == uniChar)
				return (i);	// Found it, return the position
		}
	}
	catch(PBoundsException e)
	{
		;
	}

	// If we get to this point, the character couldn't be found
	return (-1);
}



/******************************************************************************/
/* Find() will search for a substring in the string and return the position   */
/*      where it found it.                                                    */
/*                                                                            */
/* Input:  "subString" is the substring to search for.                        */
/*         "startIndex" is the start position in the string to start the      */
/*         search. The character at the position given is included in the     */
/*         search.                                                            */
/*                                                                            */
/* Output: The position in the string where the substring is found or -1 if   */
/*         not found.                                                         */
/******************************************************************************/
int32 PString::Find(PString subString, int32 startIndex) const
{
	int32 i, count;
	int32 j, subLen;

	// Check for "out of bounds"
	ASSERT(startIndex >= 0);
	ASSERT(startIndex <= stringData->stringLen);

	// Get the length of the substring
	subLen = subString.GetLength();
	if (subLen == 0)
		return (-1);	// Well, if the search string is empty, we can't find it

	// Begin to search for the substring
	count = stringData->stringLen - subLen + 1;
	for (i = startIndex; i < count; i++)
	{
		// Check the first character
		if (stringData->string[i] == subString.stringData->string[0])
		{
			// The first character is a match, now test the rest
			bool found = true;

			for (j = 1; j < subLen; j++)
			{
				if (stringData->string[i + j] != subString.stringData->string[j])
				{
					found = false;
					break;
				}
			}

			// Found the substring, return the index
			if (found)
				return (i);
		}
	}

	// If we get to this point, the substring couldn't be found
	return (-1);
}



/******************************************************************************/
/* ReverseFind() will search for a character in the string starting from the  */
/*      end of it and searching backwards and then return the position where  */
/*      it found it.                                                          */
/*                                                                            */
/* Input:  "chr" is the character to search for.                              */
/*                                                                            */
/* Output: The position in the string where the character is found or -1 if   */
/*         not found.                                                         */
/******************************************************************************/
int32 PString::ReverseFind(PChar chr) const
{
	// Just call the other version
	return (ReverseFind(chr, GetLength() - 1));
}



/******************************************************************************/
/* ReverseFind() will search for a character in the string starting from the  */
/*      index given and searching backwards and then return the position      */
/*      where it found it.                                                    */
/*                                                                            */
/* Input:  "chr" is the character to search for.                              */
/*         "startIndex" is the index to begin the search at. The character at */
/*         the position given is included in the search.                      */
/*                                                                            */
/* Output: The position in the string where the character is found or -1 if   */
/*         not found.                                                         */
/******************************************************************************/
int32 PString::ReverseFind(PChar chr, int32 startIndex) const
{
	int32 i;
	uint16 uniChar;
	int8 len;

	// Check for out of range
	if (startIndex >= GetLength())
		return (-1);

	try
	{
		// First convert the character to search after to unicode
		uniChar = chr.GetCharacterSet()->ToUnicode(chr.GetChar(), len);

		// Start the search
		for (i = startIndex; i >= 0; i--)
		{
			if (stringData->string[i] == uniChar)
				return (i);	// Found it, return the position
		}
	}
	catch(PBoundsException e)
	{
		;
	}

	// If we get to this point, the character couldn't be found
	return (-1);
}



/******************************************************************************/
/* FindOneOf() will search after one of the characters given in the string    */
/*      and return the position where it found a match.                       */
/*                                                                            */
/* Input:  "searchSet" is a set of characters to search for.                  */
/*         "startIndex" is the start position in the string to start the      */
/*         search. The character at the position given is included in the     */
/*         search.                                                            */
/*                                                                            */
/* Output: The position in the string where one of the characters in the      */
/*         search string are found or -1 if none could be found.              */
/******************************************************************************/
int32 PString::FindOneOf(PString searchSet, int32 startIndex) const
{
	int32 i, count;
	int32 j, searchLen;

	// Check for "out of bounds"
	ASSERT(startIndex >= 0);
	ASSERT(startIndex <= stringData->stringLen);

	// Get the length of the search set
	searchLen = searchSet.GetLength();
	if (searchLen == 0)
		return (-1);	// Well, if the search set string is empty, we can't find it

	// Begin to search
	count = stringData->stringLen;
	for (i = startIndex; i < count; i++)
	{
		// Now check for a match of one of the characters
		for (j = 0; j < searchLen; j++)
		{
			if (stringData->string[i] == searchSet.stringData->string[j])
			{
				// Found a match, return the index
				return (i);
			}
		}
	}

	// If we get to this point, none of the characters could be found
	return (-1);
}



/******************************************************************************/
/* Left() returns the left part of the current string.                        */
/*                                                                            */
/* Input:  "count" is the number of characters you want to cut.               */
/*                                                                            */
/* Output: The new string with the clipped context.                           */
/******************************************************************************/
PString PString::Left(int32 count) const
{
	PString newStr;

	// Fix out of bounds
	if (count < 0)
		count = 0;

	if (count > stringData->stringLen)
		count = stringData->stringLen;

	// Copy the clipped string to the new string
	AllocCopy(newStr, 0, count);

	return (newStr);
}



/******************************************************************************/
/* Mid() returns a middle part of the current string.                         */
/*                                                                            */
/* Input:  "first" is the character number from where you want to cut from.   */
/*                                                                            */
/* Output: The new string with the clipped context.                           */
/******************************************************************************/
PString PString::Mid(int32 first) const
{
	return (Mid(first, stringData->stringLen - first));
}



/******************************************************************************/
/* Mid() returns a middle part of the current string.                         */
/*                                                                            */
/* Input:  "first" is the character number from where you want to cut from.   */
/*         "count" is the number of characters you want to cut.               */
/*                                                                            */
/* Output: The new string with the clipped context.                           */
/******************************************************************************/
PString PString::Mid(int32 first, int32 count) const
{
	PString newStr;

	// Fix out of bounds
	if (first < 0)
		first = 0;

	if (count < 0)
		count = 0;

	if ((first + count) > stringData->stringLen)
		count = stringData->stringLen - first;

	if (first > stringData->stringLen)
		count = 0;

	// Copy the clipped string to the new string
	AllocCopy(newStr, first, count);

	return (newStr);
}



/******************************************************************************/
/* Right() returns the right part of the current string.                      */
/*                                                                            */
/* Input:  "count" is the number of characters you want to cut.               */
/*                                                                            */
/* Output: The new string with the clipped context.                           */
/******************************************************************************/
PString PString::Right(int32 count) const
{
	PString newStr;

	// Fix out of bounds
	if (count < 0)
		count = 0;

	if (count > stringData->stringLen)
		count = stringData->stringLen;

	// Copy the clipped string to the new string
	AllocCopy(newStr, stringData->stringLen - count, count);

	return (newStr);
}



/******************************************************************************/
/* TrimLeft() trims leading whitespaces.                                      */
/******************************************************************************/
void PString::TrimLeft(void)
{
	int32 i, count;

	// Do nothing is the string is empty
	if (IsEmpty())
		return;

	// Make sure we are the only one with this string
	CopyBeforeWrite();

	// Find the first non-whitespace character
	count = stringData->stringLen;
	for (i = 0; i < count; i++)
	{
		if (!PChar::IsSpace(stringData->string[i]))
			break;
	}

	if (i > 0)
	{
		// Minimum one character is a whitespace
		//
		// Calculate new string length
		stringData->stringLen -= i;

		if (stringData->stringLen == 0)
		{
			// All characters has been removed, make sure that
			// we got a valid empty string
			MakeEmpty();
		}
		else
		{
			// Move the characters
			memcpy(&stringData->string[0], &stringData->string[i], (stringData->stringLen + 1) * sizeof(uint16));
		}
	}
}



/******************************************************************************/
/* TrimRight() trims trailing whitespaces.                                    */
/******************************************************************************/
void PString::TrimRight(void)
{
	int32 i;

	// Do nothing is the string is empty
	if (IsEmpty())
		return;

	// Make sure we are the only one with this string
	CopyBeforeWrite();

	// Find the first non-whitespace character
	for (i = stringData->stringLen - 1; i >= 0; i--)
	{
		if (!PChar::IsSpace(stringData->string[i]))
			break;
	}

	if (i < (stringData->stringLen - 1))
	{
		// Minimum one character is a whitespace
		//
		// Calculate new string length
		stringData->stringLen = i + 1;

		if (stringData->stringLen == 0)
		{
			// All characters has been removed, make sure that
			// we got a valid empty string
			MakeEmpty();
		}
		else
		{
			// Null terminate the string
			stringData->string[stringData->stringLen] = 0x0000;
		}
	}
}



/******************************************************************************/
/* Compare() will compare the string given against the current string with a  */
/*      case sentitive compare.                                               */
/*                                                                            */
/* Input:  "string" is the string you want to compare with.                   */
/*                                                                            */
/* Output: < 0 if this < arg, == 0 if this = arg, or > 0 if this > arg.       */
/******************************************************************************/
int32 PString::Compare(PString string) const
{
	// Compare the strings
	return (CompareStrings(stringData, string.stringData));
}



/******************************************************************************/
/* CompareNoCase() will compare the string given against the current string   */
/*      with a non-case sentitive compare.                                    */
/*                                                                            */
/* Input:  "string" is the string you want to compare with.                   */
/*                                                                            */
/* Output: < 0 if this < arg, == 0 if this = arg, or > 0 if this > arg.       */
/******************************************************************************/
int32 PString::CompareNoCase(PString string) const
{
	PString tempStr(*this);

	// Make sure the two strings are in the same case
	tempStr.MakeUpper();
	string.MakeUpper();

	// Compare the strings
	return (CompareStrings(tempStr.stringData, string.stringData));
}



/******************************************************************************/
/* operator = (PString &) will set the string to the string given.            */
/*                                                                            */
/* Input:  "string" is the new string.                                        */
/*                                                                            */
/* Output: The pointer to the string buffer.                                  */
/******************************************************************************/
const PString & PString::operator = (const PString &string)
{
	// Check to see if the string to assign is empty
	if (string.IsEmpty())
	{
		// It is, so just release the current string
		Release();
	}
	else
	{
		// If the string pointer is the same, do nothing because the string is
		// already assigned to the string
		if (stringData != string.stringData)
		{
			// The reference counter should at least have one reference
			ASSERT(string.stringData->refCount > 0);

			// Count up reference counter
			AtomicIncrement(&string.stringData->refCount);

			// Release old string
			Release();

			// Copy the pointer
			stringData = string.stringData;
		}
	}

	return (*this);
}



/******************************************************************************/
/* operator = (char *) will set the string to the string given.               */
/*                                                                            */
/* Input:  "string" is the new string.                                        */
/*                                                                            */
/* Output: The pointer to the string buffer.                                  */
/******************************************************************************/
const PString & PString::operator = (const char *string)
{
	// If the pointer is NULL, create an empty string
	if (string == NULL)
		Release();
	else
		AssignCopy(string);

	return (*this);
}



/******************************************************************************/
/* operator = (char) will set the string to the character given.              */
/*                                                                            */
/* Input:  "chr" is the character.                                            */
/*                                                                            */
/* Output: The pointer to the string buffer.                                  */
/******************************************************************************/
const PString & PString::operator = (PChar chr)
{
	AssignCopy(chr);
	return (*this);
}



/******************************************************************************/
/* operator + (PString &, PString &) will append the two strings given.       */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: The new concatenated string.                                       */
/******************************************************************************/
PString operator + (const PString &string1, const PString &string2)
{
	PString str;

	// Append the strings
	str.ConcatCopy(string1.stringData, string2.stringData);

	// Return the new string
	return (str);
};



/******************************************************************************/
/* operator + (PString &, char *) will append the two strings given.          */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: The new concatenated string.                                       */
/******************************************************************************/
PString operator + (const PString &string1, const char *string2)
{
	ASSERT(string2 != NULL);

	PString str;
	PString str2(string2);

	// Append the strings
	str.ConcatCopy(string1.stringData, str2.stringData);

	// And return it
	return (str);
};



/******************************************************************************/
/* operator + (char *, PString &) will append the two strings given.          */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: The new concatenated string.                                       */
/******************************************************************************/
PString operator + (const char *string1, const PString &string2)
{
	ASSERT(string1 != NULL);

	PString str;
	PString str1(string1);

	// Append the strings
	str.ConcatCopy(str1.stringData, string2.stringData);

	// And return it
	return (str);
};



/******************************************************************************/
/* operator + (PString &, PChar &) will append a string and a character.      */
/*                                                                            */
/* Input:  "string1" is the string.                                           */
/*         "chr2" is the character.                                           */
/*                                                                            */
/* Output: The new concatenated string.                                       */
/******************************************************************************/
PString operator + (const PString &string1, const PChar &chr2)
{
	PString str;

	// Append the string and character
	str.ConcatCopy(string1.stringData, chr2);

	// And return it
	return (str);
};



/******************************************************************************/
/* operator + (PChar &, PString &) will append a character and a string.      */
/*                                                                            */
/* Input:  "chr1" is the character.                                           */
/*         "string2" is the string.                                           */
/*                                                                            */
/* Output: The new concatenated string.                                       */
/******************************************************************************/
PString operator + (const PChar &chr1, const PString &string2)
{
	PString str;

	// Append the character and string
	str.ConcatCopy(chr1, string2.stringData);

	// And return it
	return (str);
};



/******************************************************************************/
/* operator += (PString &) will append the string given to the current        */
/*         string.                                                            */
/*                                                                            */
/* Input:  "string" is the string to append.                                  */
/*                                                                            */
/* Output: The pointer to the string buffer.                                  */
/******************************************************************************/
const PString & PString::operator += (const PString &string)
{
	ConcatInPlace(string.stringData);
	return (*this);
}



/******************************************************************************/
/* operator += (char *) will append the string given to the current string.   */
/*                                                                            */
/* Input:  "string" is the string to append.                                  */
/*                                                                            */
/* Output: The pointer to the string buffer.                                  */
/******************************************************************************/
const PString & PString::operator += (const char *string)
{
	ASSERT(string != NULL);

	PString str(string);

	ConcatInPlace(str.stringData);
	return (*this);
}



/******************************************************************************/
/* operator += (PChar &) will append the character given to the current       */
/*      string.                                                               */
/*                                                                            */
/* Input:  "chr" is the character to append.                                  */
/*                                                                            */
/* Output: The pointer to the string buffer.                                  */
/******************************************************************************/
const PString & PString::operator += (const PChar &chr)
{
	ConcatInPlace(chr);
	return (*this);
}



/******************************************************************************/
/* operator == (PString, PString) will compare the two strings.               */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if they are equal, false if not.                              */
/******************************************************************************/
bool operator == (const PString &string1, const PString &string2)
{
	return (string1.CompareStrings(string1.stringData, string2.stringData) == 0 ? true : false);
}



/******************************************************************************/
/* operator == (PString, char) will compare the two strings.                  */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if they are equal, false if not.                              */
/******************************************************************************/
bool operator == (const PString &string1, const char *string2)
{
	PString str2(string2);

	return (string1.CompareStrings(string1.stringData, str2.stringData) == 0 ? true : false);
}



/******************************************************************************/
/* operator == (char, PString) will compare the two strings.                  */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if they are equal, false if not.                              */
/******************************************************************************/
bool operator == (const char *string1, const PString &string2)
{
	PString str1(string1);

	return (string2.CompareStrings(str1.stringData, string2.stringData) == 0 ? true : false);
}



/******************************************************************************/
/* operator != (PString, PString) will compare the two strings.               */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if they are different, false if not.                          */
/******************************************************************************/
bool operator != (const PString &string1, const PString &string2)
{
	return (string1.CompareStrings(string1.stringData, string2.stringData) != 0 ? true : false);
}



/******************************************************************************/
/* operator != (PString, char) will compare the two strings.                  */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if they are different, false if not.                          */
/******************************************************************************/
bool operator != (const PString &string1, const char *string2)
{
	PString str2(string2);

	return (string1.CompareStrings(string1.stringData, str2.stringData) != 0 ? true : false);
}



/******************************************************************************/
/* operator != (char, PString) will compare the two strings.                  */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if they are different, false if not.                          */
/******************************************************************************/
bool operator != (const char *string1, const PString &string2)
{
	PString str1(string1);

	return (string2.CompareStrings(str1.stringData, string2.stringData) != 0 ? true : false);
}



/******************************************************************************/
/* operator < (PString, PString) will compare the two strings.                */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is less than string2, false if not.                */
/******************************************************************************/
bool operator < (const PString &string1, const PString &string2)
{
	return (string1.CompareStrings(string1.stringData, string2.stringData) < 0 ? true : false);
}



/******************************************************************************/
/* operator < (PString, char) will compare the two strings.                   */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is less than string2, false if not.                */
/******************************************************************************/
bool operator < (const PString &string1, const char *string2)
{
	PString str2(string2);

	return (string1.CompareStrings(string1.stringData, str2.stringData) < 0 ? true : false);
}



/******************************************************************************/
/* operator < (char, PString) will compare the two strings.                   */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is less than string2, false if not.                */
/******************************************************************************/
bool operator < (const char *string1, const PString &string2)
{
	PString str1(string1);

	return (string2.CompareStrings(str1.stringData, string2.stringData) < 0 ? true : false);
}



/******************************************************************************/
/* operator > (PString, PString) will compare the two strings.                */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is higher than string2, false if not.              */
/******************************************************************************/
bool operator > (const PString &string1, const PString &string2)
{
	return (string1.CompareStrings(string1.stringData, string2.stringData) > 0 ? true : false);
}



/******************************************************************************/
/* operator > (PString, char) will compare the two strings.                   */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is higher than string2, false if not.              */
/******************************************************************************/
bool operator > (const PString &string1, const char *string2)
{
	PString str2(string2);

	return (string1.CompareStrings(string1.stringData, str2.stringData) > 0 ? true : false);
}



/******************************************************************************/
/* operator > (char, PString) will compare the two strings.                   */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is higher than string2, false if not.              */
/******************************************************************************/
bool operator > (const char *string1, const PString &string2)
{
	PString str1(string1);

	return (string2.CompareStrings(str1.stringData, string2.stringData) > 0 ? true : false);
}



/******************************************************************************/
/* operator <= (PString, PString) will compare the two strings.               */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is less or equal than string2, false if not.       */
/******************************************************************************/
bool operator <= (const PString &string1, const PString &string2)
{
	return (string1.CompareStrings(string1.stringData, string2.stringData) <= 0 ? true : false);
}



/******************************************************************************/
/* operator <= (PString, char) will compare the two strings.                  */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is less or equal than string2, false if not.       */
/******************************************************************************/
bool operator <= (const PString &string1, const char *string2)
{
	PString str2(string2);

	return (string1.CompareStrings(string1.stringData, str2.stringData) <= 0 ? true : false);
}



/******************************************************************************/
/* operator <= (char, PString) will compare the two strings.                  */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is less or equal than string2, false if not.       */
/******************************************************************************/
bool operator <= (const char *string1, const PString &string2)
{
	PString str1(string1);

	return (string2.CompareStrings(str1.stringData, string2.stringData) <= 0 ? true : false);
}



/******************************************************************************/
/* operator >= (PString, PString) will compare the two strings.               */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is higher or equal than string2, false if not.     */
/******************************************************************************/
bool operator >= (const PString &string1, const PString &string2)
{
	return (string1.CompareStrings(string1.stringData, string2.stringData) >= 0 ? true : false);
}



/******************************************************************************/
/* operator >= (PString, char) will compare the two strings.                  */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is higher or equal than string2, false if not.     */
/******************************************************************************/
bool operator >= (const PString &string1, const char *string2)
{
	PString str2(string2);

	return (string1.CompareStrings(string1.stringData, str2.stringData) >= 0 ? true : false);
}



/******************************************************************************/
/* operator >= (char, PString) will compare the two strings.                  */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: True if string1 is higher or equal than string2, false if not.     */
/******************************************************************************/
bool operator >= (const char *string1, const PString &string2)
{
	PString str1(string1);

	return (string2.CompareStrings(str1.stringData, string2.stringData) >= 0 ? true : false);
}



/******************************************************************************/
/* Initialize() reset the objects member variables.                           */
/******************************************************************************/
void PString::Initialize(void)
{
	stringData = NULL;
	charSet    = NULL;
}



/******************************************************************************/
/* SetToHostCharacterSet() set the string to the host character set.          */
/******************************************************************************/
void PString::SetToHostCharacterSet(void)
{
	// Allocate the character set
	charSet = CreateHostCharacterSet();
	if (charSet == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* CreateEmptyString() sets the string to an empty string. Notice that it     */
/*      will not free any previous string. Do that yourself before calling    */
/*      this function                                                         */
/******************************************************************************/
void PString::CreateEmptyString(void)
{
	stringData = &pEmptyString;
}



/******************************************************************************/
/* CopyString() will copy the string given into the object.                   */
/*                                                                            */
/* Input:  "string" is a pointer to a NULL terminated string to count.        */
/*         "characterSet" is a pointer to the character set to use.           */
/******************************************************************************/
void PString::CopyString(const char *string, PCharacterSet *characterSet)
{
	uint16 chr;
	int8 charLen;
	int32 i;

	// Null pointer not allowed
	ASSERT(string != NULL);
	ASSERT(characterSet != NULL);

	// Traverse the string
	for (i = 0; ; i++)
	{
		try
		{
			// Convert the character
			chr = characterSet->ToUnicode(string, charLen);
		}
		catch(PBoundsException e)
		{
			chr     = 0x003f;
			charLen = 1;
		}

		// Check for out of bounds
		ASSERT(i <= stringData->allocatedLen);

		// Store the character as unicode
		stringData->string[i] = chr;

		// Did we reach the NULL terminator?
		if (chr == 0x0000)
			break;

		// Go to the next character
		string += charLen;
	}
}



/******************************************************************************/
/* CopyString() will copy the string given into the object.                   */
/*                                                                            */
/* Input:  "string" is a pointer to a NULL terminated string to count.        */
/*         "length" is the number of bytes to copy.                           */
/*         "characterSet" is a pointer to the character set to use.           */
/******************************************************************************/
void PString::CopyString(const char *string, int32 length, PCharacterSet *characterSet)
{
	uint16 chr;
	int8 charLen;
	int32 i, j;

	// Null pointer not allowed
	ASSERT(string != NULL);
	ASSERT(characterSet != NULL);

	// Traverse the string
	for (i = 0, j = 0; j < length; i++)
	{
		try
		{
			// Convert the character
			chr = characterSet->ToUnicode(string, charLen);
		}
		catch(PBoundsException e)
		{
			chr     = 0x003f;
			charLen = 1;
		}

		// Check for out of bounds
		ASSERT(i <= stringData->allocatedLen);

		// Store the character as unicode
		stringData->string[i] = chr;

		// Go to the next character
		string += charLen;
		j      += charLen;
	}

	// Null terminate the string
	stringData->string[i] = 0x0000;
	stringData->stringLen = i;
}



/******************************************************************************/
/* CreateString() will convert the current object string to a string in the   */
/*      character set given.                                                  */
/*                                                                            */
/* Input:  "buffer" is a pointer to where to store the converted string.      */
/*         "length" is a reference where the string length will be stored.    */
/*         "characterSet" is the character set to use.                        */
/******************************************************************************/
void PString::CreateString(char *buffer, int32 &length, PCharacterSet *characterSet) const
{
	int32 i, count;
	int8 len;
	const char *tempBuf;

	// Null pointer not allowed
	ASSERT(buffer != NULL);
	ASSERT(characterSet != NULL);

	// Reset the length
	length = 0;

	// Get the string length
	count = stringData->stringLen;
	for (i = 0; i < count; i++)
	{
		// Convert the character
		tempBuf = characterSet->FromUnicode(stringData->string[i], len);

		// And copy it into the buffer
		memcpy(buffer, tempBuf, len);
		buffer += len;
		length += len;
	}

	// Store the NULL terminator
	tempBuf = characterSet->FromUnicode(0x0000, len);
	memcpy(buffer, tempBuf, len);
}



/******************************************************************************/
/* CountStringLength() will count the string length in characters.            */
/*                                                                            */
/* Input:  "string" is a pointer to a NULL terminated string to count.        */
/*         "characterSet" is a pointer to the character set to use.           */
/*                                                                            */
/* Output: The length in characters.                                          */
/******************************************************************************/
int32 PString::CountStringLength(const char *string, PCharacterSet *characterSet) const
{
	int8 charLen;
	int32 length = 0;

	// Null pointer not allowed
	ASSERT(characterSet != NULL);

	// Check for string null pointer
	if (string == NULL)
		return (0);

	// Traverse the string and count the number of characters
	for (;;)
	{
		try
		{
			if (characterSet->ToUnicode(string, charLen) == 0x0000)
				break;
		}
		catch(PBoundsException e)
		{
			charLen = 1;
		}

		length++;
		string += charLen;
	}

	return (length);
}



/******************************************************************************/
/* AllocBuffer() will allocate a new memory buffer to the string + data       */
/*      structure.                                                            */
/*                                                                            */
/* Input:  "length" is the number of characters to allocate.                  */
/******************************************************************************/
void PString::AllocBuffer(int32 length)
{
	if (length == 0)
		CreateEmptyString();	// Makes the string empty
	else
	{
		PStringData *data;

		// Allocate the data structure
		data = new PStringData;
		if (data == NULL)
			throw PMemoryException();

		try
		{
			// Fill out the structure
			data->refCount  = 1;
			data->stringLen = length;

			// Allocate bigger chunk of memory to avoid fragmentation
			if (length <= 64)
				length = 64;
			else if (length <= 128)
				length = 128;
			else if (length <= 256)
				length = 256;
			else if (length <= 512)
				length = 512;
			else if (length <= 1024)
				length = 1024;
			else if (length <= 2048)
				length = 2048;

			// Store how many characters we have allocated
			data->allocatedLen = length;

			// Allocate buffer to hold the string
			data->string = new uint16[length + 1];
			if (data->string == NULL)
				throw PMemoryException();

			// Assign the data structure to the string
			stringData = data;
		}
		catch(...)
		{
			// Delete the structure again
			delete data;
			throw;
		}
	}
}



/******************************************************************************/
/* Release() will count down the reference count and free the memory block if */
/*      necessary. It also makes sure that the string is empty after it has   */
/*      been released.                                                        */
/******************************************************************************/
void PString::Release(void)
{
	// Check for an empty string
	if (stringData->string != NULL)
	{
		int32 result;

		// The reference counter should at least have one reference
		ASSERT(stringData->refCount > 0);

		// Count down reference counter
		result = AtomicDecrement(&stringData->refCount);

		// If there isn't any references left, delete the buffer
		if (result == 0)
		{
			// Delete the old string
			delete[] stringData->string;
			delete stringData;
		}

		// Set the current object to an empty string
		CreateEmptyString();
	}
}



/******************************************************************************/
/* Release() will count down the reference count on the data structure given  */
/*      and free the memory block if necessary.                               */
/*                                                                            */
/* Input:  "data" is a pointer to the data structure to release.              */
/******************************************************************************/
void PString::Release(PStringData *data)
{
	if (data->string != NULL)
	{
		int32 result;

		// The reference counter should at least have one reference
		ASSERT(data->refCount > 0);

		// Count down reference counter
		result = AtomicDecrement(&data->refCount);

		// If there isn't any references left, delete the buffer
		if (result == 0)
		{
			// Delete the old string
			delete[] data->string;
			delete data;
		}
	}
}



/******************************************************************************/
/* CopyBeforeWrite() makes sure that the string buffer has it's own memory    */
/*      block.                                                                */
/******************************************************************************/
void PString::CopyBeforeWrite(void)
{
	// If there are more objects referring to the same string, make a
	// copy of it
	if (stringData->refCount > 1)
	{
		PStringData *oldData;

		// Remember old data pointer
		oldData = stringData;

		// Allocate new buffer
		AllocBuffer(oldData->stringLen);

		if (oldData->stringLen != 0)
		{
			// Copy the string to the new buffer
			memcpy(stringData->string, oldData->string, (oldData->stringLen + 1) * sizeof(uint16));
		}

		// Release the reference
		Release(oldData);
	}
}



/******************************************************************************/
/* AllocBeforeWrite() makes sure that there is a buffer allocated which is    */
/*      big enought to hold the new string.                                   */
/*                                                                            */
/* Input:  "length" is the length of the string.                              */
/******************************************************************************/
void PString::AllocBeforeWrite(int32 length)
{
	if ((stringData->refCount > 1) || (stringData->allocatedLen < length))
	{
		Release();
		AllocBuffer(length);
	}
}



/******************************************************************************/
/* EnlargeBeforeWrite() makes sure that there is a buffer allocated which is  */
/*      big enought to hold the length given. It won't change the current     */
/*      string.                                                               */
/*                                                                            */
/* Input:  "length" is the new length of the buffer.                          */
/******************************************************************************/
void PString::EnlargeBeforeWrite(int32 length)
{
	if ((stringData->refCount > 1) || (stringData->allocatedLen < length))
	{
		PStringData *oldData;

		// Remember old data pointer
		oldData = stringData;

		// Allocate new buffer
		AllocBuffer(max(length, oldData->allocatedLen));

		// Change the length back
		stringData->stringLen = oldData->stringLen;

		if (oldData->stringLen != 0)
		{
			// Copy the string to the new buffer
			memcpy(stringData->string, oldData->string, (oldData->stringLen + 1) * sizeof(uint16));
		}

		// Release the reference
		Release(oldData);
	}
}



/******************************************************************************/
/* AssignCopy() assigns a new string to the this object.                      */
/*                                                                            */
/* Input:  "string" is the new string to assign with.                         */
/*         "characterSet" is a pointer to the character set to use or NULL.   */
/******************************************************************************/
void PString::AssignCopy(const char *string, PCharacterSet *characterSet)
{
	PCharacterSet *hostCharSet;
	int32 strLen;

	// Create the host character set
	if (characterSet == NULL)
	{
		hostCharSet = CreateHostCharacterSet();
		if (hostCharSet == NULL)
			throw PMemoryException();
	}
	else
		hostCharSet = characterSet;

	// First find out how many characters the string is
	strLen = CountStringLength(string, hostCharSet);

	// Well, if it's an empty string, make the string empty
	if (strLen == 0)
		Release();
	else
	{
		// Make sure we have an allocated buffer
		AllocBeforeWrite(strLen);

		// Copy the string
		CopyString(string, hostCharSet);

		// Set the string length
		stringData->stringLen = strLen;
	}

	// Delete the host character set
	if (characterSet == NULL)
		delete hostCharSet;
}



/******************************************************************************/
/* AssignCopy() assigns a single character to the this object.                */
/*                                                                            */
/* Input:  "chr" is the character.                                            */
/******************************************************************************/
void PString::AssignCopy(PChar chr)
{
	int8 len;
	uint16 uniChar;

	// Make sure we have an allocated buffer
	AllocBeforeWrite(1);

	// Copy the character
	try
	{
		uniChar = chr.GetCharacterSet()->ToUnicode(chr.GetChar(), len);
	}
	catch(PBoundsException e)
	{
		uniChar = 0x003f;
	}

	stringData->string[0] = uniChar;

	// Set the string length
	stringData->stringLen = 1;
}


/******************************************************************************/
/* AllocCopy() will copy a clipped area from the current object to a new      */
/*      string object.                                                        */
/*                                                                            */
/* Input:  "dest" is a reference to where to copy the clipped data.           */
/*         "startIndex" is where to start the copy.                           */
/*         "length" is the number of bytes to copy.                           */
/******************************************************************************/
void PString::AllocCopy(PString &dest, int32 startIndex, int32 length) const
{
	// Should we just create an empty string?
	if (length == 0)
		dest.CreateEmptyString();
	else
	{
		// Allocate a buffer to hold the string
		dest.AllocBuffer(length);

		// Copy the string part
		memcpy(dest.stringData->string, &stringData->string[startIndex], length * sizeof(uint16));

		// Null terminate the string
		dest.stringData->string[length] = 0x0000;

		// Make sure the new string use the same character set
		dest.SwitchCharacterSet(charSet);
	}
}



/******************************************************************************/
/* ConcatInPlace() appends a string to the this object.                       */
/*                                                                            */
/* Input:  "string" is the string to append.                                  */
/******************************************************************************/
void PString::ConcatInPlace(const PStringData *string)
{
	// Append an empty string is a no-operation
	if (string->stringLen == 0)
		return;

	// Check to see if we need to allocate a new buffer
	if ((stringData->refCount > 1) || ((stringData->stringLen + string->stringLen) >= stringData->allocatedLen))
	{
		// We have to grow the buffer
		PStringData *oldData = stringData;
		ConcatCopy(stringData, string);
		Release(oldData);
	}
	else
	{
		// Just append the string, the buffer is big enough
		memcpy(&stringData->string[stringData->stringLen], string->string, string->stringLen * sizeof(uint16));

		// Set the new length
		stringData->stringLen += string->stringLen;

		// Null terminate the string
		stringData->string[stringData->stringLen] = 0x0000;
	}
}



/******************************************************************************/
/* ConcatInPlace() appends a character to the this object.                    */
/*                                                                            */
/* Input:  "chr" is the character to append.                                  */
/******************************************************************************/
void PString::ConcatInPlace(const PChar &chr)
{
	// Append a null character is a no-operation
	if (chr.IsNull())
		return;

	// Check to see if we need to allocate a new buffer
	if ((stringData->refCount > 1) || ((stringData->stringLen + 1) >= stringData->allocatedLen))
	{
		// We have to grow the buffer
		PStringData *oldData = stringData;
		ConcatCopy(stringData, chr);
		Release(oldData);
	}
	else
	{
		int8 len;
		uint16 uniChar;

		try
		{
			// Convert the character to unicode
			uniChar = chr.GetCharacterSet()->ToUnicode(chr.GetChar(), len);
		}
		catch(PBoundsException e)
		{
			uniChar = 0x003f;
		}

		// Just append the character, the buffer is big enough
		stringData->string[stringData->stringLen] = uniChar;

		// Set the new length
		stringData->stringLen++;

		// Null terminate the string
		stringData->string[stringData->stringLen] = 0x0000;
	}
}



/******************************************************************************/
/* ConcatCopy() append two strings and store it in the this object.           */
/*                                                                            */
/* Input:  "sourceString" is the start string.                                */
/*         "appendString" is the string to append.                            */
/******************************************************************************/
void PString::ConcatCopy(const PStringData *sourceString, const PStringData *appendString)
{
	int32 newLen;

	// Calculate the new length
	newLen = sourceString->stringLen + appendString->stringLen;

	if (newLen != 0)
	{
		// Allocate new buffer and append the strings
		AllocBuffer(newLen);
		memcpy(&stringData->string[0], sourceString->string, sourceString->stringLen * sizeof(uint16));
		memcpy(&stringData->string[sourceString->stringLen], appendString->string, appendString->stringLen * sizeof(uint16));

		// Null terminate the string
		stringData->string[newLen] = 0x0000;
	}
}



/******************************************************************************/
/* ConcatCopy() append a string and character and store it in the this object.*/
/*                                                                            */
/* Input:  "sourceString" is the start string.                                */
/*         "appendChar" is the character to append.                           */
/******************************************************************************/
void PString::ConcatCopy(const PStringData *sourceString, const PChar &appendChar)
{
	int32 newLen;
	int8 len;
	uint16 uniChar;

	// Calculate the new length
	newLen = sourceString->stringLen + 1;

	// Allocate new buffer and append the strings
	AllocBuffer(newLen);
	memcpy(&stringData->string[0], sourceString->string, sourceString->stringLen * sizeof(uint16));

	try
	{
		uniChar = appendChar.GetCharacterSet()->ToUnicode(appendChar.GetChar(), len);
	}
	catch(PBoundsException e)
	{
		uniChar = 0x003f;
	}

	// Set the character at the last position
	stringData->string[newLen - 1] = uniChar;

	// Null terminate the string
	stringData->string[newLen] = 0x0000;
}



/******************************************************************************/
/* ConcatCopy() append a character and string and store it in the this object.*/
/*                                                                            */
/* Input:  "sourceChar" is the start character.                               */
/*         "appendString" is the string to append.                            */
/******************************************************************************/
void PString::ConcatCopy(const PChar &sourceChar, const PStringData *appendString)
{
	int32 newLen;
	int8 len;

	// Calculate the new length
	newLen = 1 + appendString->stringLen;

	// Allocate new buffer and append the strings
	AllocBuffer(newLen);

	try
	{
		stringData->string[0] = sourceChar.GetCharacterSet()->ToUnicode(sourceChar.GetChar(), len);
	}
	catch(PBoundsException e)
	{
		stringData->string[0] = 0x003f;
	}

	memcpy(&stringData->string[1], appendString->string, appendString->stringLen * sizeof(uint16));

	// Null terminate the string
	stringData->string[newLen] = 0x0000;
}



/******************************************************************************/
/* CompareStrings() compares two strings.                                     */
/*                                                                            */
/* Input:  "string1" is the first string.                                     */
/*         "string2" is the second string.                                    */
/*                                                                            */
/* Output: -1, if str1 < str2, 0 if str1 == str2 and 1 if str1 > str2.        */
/******************************************************************************/
int8 PString::CompareStrings(const PStringData *string1, const PStringData *string2) const
{
	int32 count, i;
	register uint16 chr1, chr2;

	// Start to find the smallest string
	count = min(string1->stringLen, string2->stringLen);

	// Traverse each character in both strings and compare them
	for (i = 0; i < count; i++)
	{
		// Get the characters
		chr1 = string1->string[i];
		chr2 = string2->string[i];

		if (chr1 < chr2)
			return (-1);

		if (chr1 > chr2)
			return (1);
	}

	// Well, so far they are equal, but do they have the same length?
	if (string1->stringLen == string2->stringLen)
		return (0);

	if (string1->stringLen < string2->stringLen)
		return (-1);

	return (1);
}



/******************************************************************************/
/* FormatIt() is the real sprintf like format routine.                        */
/*                                                                            */
/* Input:  "formatString" is the format string.                               */
/*         "argList" is a pointer to the arguments.                           */
/*                                                                            */
/* Note: This function is based on the printf() function in the FREE-C        */
/*       runtime library by A. Maromaty.                                      */
/******************************************************************************/
void PString::FormatIt(PString formatString, va_list argList)
{
	int32 prevLen;
	uint16 *format, *formatEnd;
	uint16 *dest, *destEnd;
	uint16 *type, *typeEnd;
	bool runFlag;
	bool justify, sign, zeroPad, blank, hash;
	int32 width, precision;
	bool precisionUsed;
	bool hPrefix, lPrefix, l64Prefix;
	int32 sSize, dSize, maxLen;
	int32 padLen, zeroPadLen;
	bool gFormat, floatZeroFill;
	double floatNum, signedFloatNum;
	int32 floatPower, floatSign, floatDigit, floatPrec;
	uint16 signChar;
	uint16 typeBuffer[256];
	PString typeStr;
	PCharSet_UNICODE typeCharSet;

	// The format string may not be empty!
	ASSERT(!formatString.IsEmpty());

	// Release the string
	Release();

	// Make sure the type string use the same character set as
	// the current string
	typeStr.SwitchCharacterSet(charSet);

	// Make sure we have an unique buffer in the format string
	formatString.CopyBeforeWrite();

	// Initialize pointers
	format    = formatString.stringData->string;
	formatEnd = format + formatString.stringData->stringLen;
	dest      = stringData->string;
	destEnd   = stringData->string + stringData->stringLen;

	for (; format < formatEnd; format++)
	{
		// Did we found the '%' character?
		if (*format != 0x0025)
		{
			// Expand the buffer?
			if (dest == destEnd)
			{
				prevLen = stringData->stringLen;
				EnlargeBeforeWrite(stringData->stringLen + 63);
				dest    = stringData->string + prevLen;
				destEnd = stringData->string + stringData->allocatedLen;
			}

			*dest++ = *format;
			stringData->stringLen++;
			continue;
		}

		// Found a '%' character, so parse it like the sprintf function will do it
		//
		// Did we get a double %
		if (((format + 1) < formatEnd) && (*(format + 1) == 0x0025))
		{
			// Expand the buffer?
			if (dest == destEnd)
			{
				prevLen = stringData->stringLen;
				EnlargeBeforeWrite(stringData->stringLen + 63);
				dest    = stringData->string + prevLen;
				destEnd = stringData->string + stringData->allocatedLen;
			}

			*dest++ = 0x0025;
			stringData->stringLen++;
			format += 2;
			continue;
		}

		// Initialize flag variables
		runFlag       = true;
		justify       = false;
		sign          = false;
		zeroPad       = false;
		blank         = false;
		hash          = false;

		// Initialize width variables
		width         = 0;

		// Initialize precision variables
		precision     = 0;
		precisionUsed = false;

		// Initialize type prefix variables
		hPrefix       = false;
		lPrefix       = false;
		l64Prefix     = false;

		// Initialize type variables
		gFormat       = false;
		floatZeroFill = false;
		floatPrec     = 0;

		// Initialize other variables
		maxLen        = -1;
		signChar      = 0x0000;
		padLen        = 0;
		zeroPadLen    = 0;

		// Handle the flag field
		for (format++; (format < formatEnd) && runFlag; format++)
		{
			switch (*format)
			{
				// '-' - Left align the result within the given field width
				case 0x002d:
				{
					justify = true;
					break;
				}

				// '+' - Prefix output with a sign
				case 0x002b:
				{
					sign = true;
					break;
				}

				// '0' - Add zeros until the minimum width is reached
				case 0x0030:
				{
					zeroPad = true;
					break;
				}

				// ' ' - Prefix output with a space
				case 0x0020:
				{
					blank = true;
					break;
				}

				// '#' - Prefix output with 0, 0x or 0X
				case 0x0023:
				{
					hash = true;
					break;
				}

				default:
				{
					format--;
					runFlag = false;
					break;
				}
			}
		}

		// Do the width field
		if (*format == 0x002a)	// '*'
		{
			// Get the width of the field
			width = va_arg(argList, int);
			format++;
		}
		else
		{
			// Read until we don't have any digits left
			while ((format < formatEnd) && (PChar::IsDigit(*format)))
				width = (width * 10) + ((int32)(*format++ - 0x0030));
		}

		// Have we reached the end of the format string?
		if (format == formatEnd)
			return;

		// Do the precision field
		if (*format == 0x002e)	// '.'
		{
			format++;
			if (format == formatEnd)
				return;

			if (*format == 0x002a)	// '*'
			{
				// Get the precision value
				precision     = va_arg(argList, int);
				precisionUsed = true;
				format++;
			}
			else
			{
				precisionUsed = true;

				while ((format < formatEnd) && (PChar::IsDigit(*format)))
					precision = (precision * 10) + ((int32)(*format++ - 0x0030));
			}
		}
		
		// Have we reached the end of the format string?
		if (format == formatEnd)
			return;

		// Do the type prefix field
		if (*format == 0x0068)	// 'h'
		{
			hPrefix = true;
			format++;
		}
		else
		{
			if (*format == 0x006c)	// 'l'
			{
				lPrefix = true;
				format++;
			}
			else
			{
				if (*format == 0x004c)	// 'L'
				{
					l64Prefix = true;
					format++;
				}
			}
		}

		// Do the type field
		switch (*format)
		{
			// 'c'
			case 0x0063:
			{
				typeStr = (PChar)va_arg(argList, int);

				sign          = false;
				zeroPad       = false;
				blank         = false;
				hash          = false;
				precisionUsed = false;
				break;
			}

			// 'd' + 'i'
			case 0x0064:
			case 0x0069:
			{
				if (lPrefix)
					typeStr.SetNumber(va_arg(argList, long));
				else
				{
					if (hPrefix)
						typeStr.SetNumber(va_arg(argList, short));
					else
					{
						if (l64Prefix)
							typeStr.SetNumber64(va_arg(argList, int64));
						else
							typeStr.SetNumber(va_arg(argList, int));
					}
				}

				hash = false;
				break;
			}

			// 'g' + 'G'
			case 0x0067:
			case 0x0047:
			{
				gFormat        = true;
				floatNum       = va_arg(argList, double);

				signedFloatNum = floatNum;
				floatPower     = FloatP10(&signedFloatNum, &floatSign, -999);

				signedFloatNum = floatNum;
				if ((!precisionUsed) && (precision == 0))
					precision = 6;

				floatPower = FloatP10(&signedFloatNum, &floatSign, precision - floatPower);
				if ((floatNum != 0.0) && ((floatPower < -4) || (floatPower >= precision)))
				{
					if (*format == 0x0067)	// 'g'
						*format = 0x0065;	// 'e'
					else
						*format = 0x0045;	// 'E'
				}
				else
				{
					*format = 0x0066;	// 'f'
					precision -= (floatPower + 1);
					if (precision <= 0)
						precision = 1;
				}

				// Falling through!
			}

			// 'e' + 'E'
			case 0x0065:
			case 0x0045:
			{
				if (*format != 0x0066)	// 'f'
				{
					int32 workPos = 0;
					int32 tempPos = 0;
					PString tempStr;

					if (!gFormat)
						floatNum = va_arg(argList, double);

					if (floatNum == HUGE_VAL)
					{
						typeStr       = "infinity";
						zeroPad       = false;
						precisionUsed = false;
						break;
					}

					if (floatNum == -HUGE_VAL)
					{
						typeStr       = "-infinity";
						zeroPad       = false;
						precisionUsed = false;
						break;
					}

					signedFloatNum = floatNum;
					if ((!precisionUsed) && (precision == 0))
						precision = 6;

					floatPower = FloatP10(&signedFloatNum, &floatSign, -999);
					floatPower = FloatP10(&floatNum, &floatSign, precision - floatPower);

					if (floatSign < 0)
						typeBuffer[workPos++] = 0x002d;	// '-'

					floatDigit            = (int32)floatNum;
					typeBuffer[workPos++] = floatDigit + 0x0030;	// '0'

					floatNum -= floatDigit;
					floatNum *= 10.0;
					floatNum += (double)5e-15;

					if (gFormat)
						tempPos = workPos;

					if (precision != 0)
					{
						precisionUsed = false;
						typeBuffer[workPos++] = 0x002e;	// '.'

						while (precision != 0)
						{
							if (floatPrec < 16)
							{
								floatDigit            = (int32)floatNum;
								typeBuffer[workPos++] = floatDigit + 0x0030;	// '0'

								if (gFormat && (floatDigit > 0))
									tempPos = workPos;

								floatNum -= floatDigit;
								floatNum *= 10.0;
								floatNum += 5e-15;
								floatPrec++;
							}
							else
								typeBuffer[workPos++] = 0x0030;	// '0'

							precision--;
						}
					}

					if (gFormat)
						workPos = tempPos;

					typeBuffer[workPos++] = *format;

					if (floatPower >= 0)
						typeBuffer[workPos++] = 0x002b;	// '+'
					else
					{
						typeBuffer[workPos++] = 0x002d;	// '-'
						floatPower = -floatPower;
					}

					if (floatPower < 10)
						typeBuffer[workPos++] = 0x0030;	// '0'

					tempStr.SetNumber(floatPower);
					for (int32 i = 0; i < tempStr.stringData->stringLen; i++)
						typeBuffer[workPos++] = tempStr.stringData->string[i];

					typeBuffer[workPos] = 0x0000;
					typeStr.SetString((char *)typeBuffer, &typeCharSet);
					break;
				}

				// Falling through!
			}

			// 'f'
			case 0x0066:
			{
				int32 workPos = 0;
				int32 tempPos = 0;

				if (!gFormat)
					floatNum = va_arg(argList, double);

				if (floatNum == HUGE_VAL)
				{
					typeStr       = "infinity";
					zeroPad       = false;
					precisionUsed = false;
					break;
				}

				if (floatNum == -HUGE_VAL)
				{
					typeStr       = "-infinity";
					zeroPad       = false;
					precisionUsed = false;
					break;
				}

				if ((!precisionUsed) && (precision == 0))
					precision = 6;

				floatPower = FloatP10(&floatNum, &floatSign, precision);
				if (floatSign < 0)
					typeBuffer[workPos++] = 0x002d;	// '-'

				if (floatPower < 0)
				{
					typeBuffer[workPos++] = 0x0030;	// '0'
					floatPower++;
					floatZeroFill = true;
				}
				else
				{
					while (floatPower >= 0)
					{
						if (floatPrec < 16)
						{
							floatDigit            = (int32)floatNum;
							typeBuffer[workPos++] = floatDigit + 0x0030;	// '0'

							floatNum -= floatDigit;
							floatNum *= 10.0;
							floatNum += 5e-15;
							floatPrec++;
						}
						else
							typeBuffer[workPos++] = 0x0030;	// '0'

						floatPower--;
					}

					floatPower = 0;
				}

				if (gFormat)
					tempPos = workPos;

				if (precision != 0)
				{
					precisionUsed = false;
					typeBuffer[workPos++] = 0x002e;	// '.'

					while (precision != 0)
					{
						if (floatZeroFill && (floatPower < 0))
						{
							typeBuffer[workPos++] = 0x0030;	// '0'
							floatPower++;
						}
						else
						{
							if (floatPrec < 16)
							{
								floatDigit            = (int32)floatNum;
								typeBuffer[workPos++] = floatDigit + 0x0030;	// '0'

								if (gFormat && (floatDigit > 0))
									tempPos = workPos;

								floatNum -= floatDigit;
								floatNum *= 10.0;
								floatNum += 5e-15;
								floatPrec++;
							}
							else
								typeBuffer[workPos++] = 0x0030;	// '0'
						}

						precision--;
					}
				}

				if (gFormat)
					workPos = tempPos;

				typeBuffer[workPos] = 0x0000;
				typeStr.SetString((char *)typeBuffer, &typeCharSet);
				break;
			}

			// 'n'
			case 0x006e:
			{
				int *tempNum = va_arg(argList, int *);
				*tempNum = dest - stringData->string;

				typeStr.MakeEmpty();
				break;
			}

			// 'o'
			case 0x006f:
			{
				int32 typeLen;
				uint64 tempNum;

				if (lPrefix)
					tempNum = va_arg(argList, unsigned long);
				else
				{
					if (hPrefix)
						tempNum = va_arg(argList, unsigned short);
					else
					{
						if (l64Prefix)
							tempNum = va_arg(argList, uint64);
						else
							tempNum = va_arg(argList, unsigned int);
					}
				}

				// Convert the number to octal
				ToOctal(typeBuffer, typeLen, tempNum);

				// Put the number into a string
				typeBuffer[typeLen] = 0x0000;
				typeStr.SetString((char *)typeBuffer, &typeCharSet);
				break;
			}

			// 'p'
			case 0x0070:
			{
				int32 typeLen, missing;

				// Convert to hexadecimal
				ToHex(typeBuffer, typeLen, (uint32)va_arg(argList, void *), true);

				// Insert the ':' character in the middle and missing zeros
				// in the beginning
				missing = 8 - typeLen;
				memmove(&typeBuffer[missing], &typeBuffer[0], typeLen * sizeof(uint16));

				for (int32 i = 0; i < missing; i++)
					typeBuffer[i] = 0x0030;	// '0'

				memmove(&typeBuffer[5], &typeBuffer[4], 4);
				typeBuffer[4] = 0x003a;	// ':'

				// Put the number into a string
				typeBuffer[9] = 0x0000;
				typeStr.SetString((char *)typeBuffer, &typeCharSet);

				hash = false;
				break;
			}

			// 's'
			case 0x0073:
			{
				// Set the string
				typeStr.SetString(va_arg(argList, char *), typeStr.charSet);

				if (precisionUsed)
				{
					precisionUsed = false;
					maxLen        = precision;
				}

				sign    = false;
				zeroPad = false;
				blank   = false;
				hash    = false;
				break;
			}

			// 'u'
			case 0x0075:
			{
				if (lPrefix)
					typeStr.SetUNumber(va_arg(argList, unsigned long));
				else
				{
					if (hPrefix)
						typeStr.SetUNumber(va_arg(argList, unsigned short));
					else
					{
						if (l64Prefix)
							typeStr.SetUNumber64(va_arg(argList, uint64));
						else
							typeStr.SetUNumber(va_arg(argList, unsigned int));
					}
				}

				hash = false;
				break;
			}

			// 'x' + 'X'
			case 0x0078:
			case 0x0058:
			{
				int32 typeLen;
				uint64 tempNum;

				if (lPrefix)
					tempNum = va_arg(argList, unsigned long);
				else
				{
					if (hPrefix)
						tempNum = va_arg(argList, unsigned short);
					else
					{
						if (l64Prefix)
							tempNum = va_arg(argList, uint64);
						else
							tempNum = va_arg(argList, unsigned int);
					}
				}

				// Convert the number to hexadecimal
				ToHex(typeBuffer, typeLen, tempNum, *format == 0x0058 ? true : false);

				// Put the number into a string
				typeBuffer[typeLen] = 0x0000;
				typeStr.SetString((char *)typeBuffer, &typeCharSet);
				break;
			}

			default:
			{
				typeStr.MakeEmpty();
				sign    = false;
				justify = false;
				zeroPad = false;
				blank   = false;
				hash    = false;
				maxLen  = 1;
				break;
			}
		}

		if (!typeStr.IsEmpty())
		{
			type    = typeStr.stringData->string;
			typeEnd = type + typeStr.stringData->stringLen;
			sSize   = typeStr.GetLength();
			dSize   = sSize;

			if (justify && zeroPad)
				zeroPad = false;

			// 'c' + 's'
			if ((*format != 0x0063) && (*format != 0x0073))
			{
				// '0'
				if ((sSize == 1) && (*format == 0x0030))
				{
					if (precisionUsed && (precision == 0))
					{
						typeStr.MakeEmpty();
						sSize = 0;
						dSize = 0;
					}
				}
				else
				{
					if (hash)
					{
						// 'o'
						if ((*format == 0x006f) && (precision < sSize))
						{
							hash          = false;
							precisionUsed = true;
							precision     = sSize + 1;
						}
						else
						{
							// 'x' + 'X'
							if ((*format == 0x0078) || (*format == 0x0058))
							{
								sSize += 2;
								typeBuffer[32] = 0x0030;	// '0'
								typeBuffer[33] = *format;
								typeBuffer[34] = 0x0000;
							}
						}
					}
				}

				if (*type == 0x002d)	// '-'
				{
					type++;
					signChar = 0x002d;	// '-'
					dSize--;
				}
				else
				{
					if (sign)
					{
						signChar = 0x002b;	// '+'
						sSize++;
					}
					else
					{
						if (blank)
						{
							signChar = 0x0020;	// ' '
							sSize++;
						}
					}
				}
			}

			if (width > sSize)
			{
				if (precisionUsed && (precision > dSize))
					padLen = width - (sSize + (precision - dSize));
				else
					padLen = width - sSize;
			}

			if (precisionUsed && (precision > dSize))
				zeroPadLen = precision - dSize;
			else
			{
				if (zeroPad)
				{
					zeroPadLen = padLen;
					padLen     = 0;
				}
			}

			while (!justify && (padLen > 0))
			{
				// Expand the buffer?
				if (dest == destEnd)
				{
					prevLen = stringData->stringLen;
					EnlargeBeforeWrite(stringData->stringLen + 63);
					dest    = stringData->string + prevLen;
					destEnd = stringData->string + stringData->allocatedLen;
				}

				*dest++ = 0x0020;	// ' '
				stringData->stringLen++;
				padLen--;
			}

			if (hash)
			{
				uint16 *b = &typeBuffer[32];

				while (*b != 0x0000)
				{
					// Expand the buffer?
					if (dest == destEnd)
					{
						prevLen = stringData->stringLen;
						EnlargeBeforeWrite(stringData->stringLen + 63);
						dest    = stringData->string + prevLen;
						destEnd = stringData->string + stringData->allocatedLen;
					}

					*dest++ = *b++;
					stringData->stringLen++;
				}
			}
			else
			{
				if (signChar != 0x0000)
				{
					// Expand the buffer?
					if (dest == destEnd)
					{
						prevLen = stringData->stringLen;
						EnlargeBeforeWrite(stringData->stringLen + 63);
						dest    = stringData->string + prevLen;
						destEnd = stringData->string + stringData->allocatedLen;
					}

					*dest++ = signChar;
					stringData->stringLen++;
				}
			}

			while (zeroPadLen > 0)
			{
				// Expand the buffer?
				if (dest == destEnd)
				{
					prevLen = stringData->stringLen;
					EnlargeBeforeWrite(stringData->stringLen + 63);
					dest    = stringData->string + prevLen;
					destEnd = stringData->string + stringData->allocatedLen;
				}

				*dest++ = 0x0030;	// '0'
				stringData->stringLen++;
				zeroPadLen--;
			}

			while (type < typeEnd)
			{
				if (maxLen != -1)
				{
					if (!(maxLen--))
						break;
				}

				// Expand the buffer?
				if (dest == destEnd)
				{
					prevLen = stringData->stringLen;
					EnlargeBeforeWrite(stringData->stringLen + 63);
					dest    = stringData->string + prevLen;
					destEnd = stringData->string + stringData->allocatedLen;
				}

				*dest++ = *type++;
				stringData->stringLen++;
			}

			while (justify && (padLen > 0))
			{
				// Expand the buffer?
				if (dest == destEnd)
				{
					prevLen = stringData->stringLen;
					EnlargeBeforeWrite(stringData->stringLen + 63);
					dest    = stringData->string + prevLen;
					destEnd = stringData->string + stringData->allocatedLen;
				}

				*dest++ = 0x0020;	// ' '
				stringData->stringLen++;
				padLen--;
			}
		}
	}

	// Null terminate the string
	stringData->string[stringData->stringLen] = 0x0000;
}



/******************************************************************************/
/* FloatP10() gets the characteristic, mantissa and sign of a double          */
/*      precision floating point number such that:                            */
/*                                                                            */
/*      1 <= mantissa < 10                                                    */
/*      -308 <= characteristic <= 308                                         */
/*      sign: 1 for positive, -1 for negative.                                */
/*                                                                            */
/* Input:  "floatNum" is the double to check.                                 */
/*         "floatSign" is where to store the sign of the double.              */
/*         "precision" is the precision of the number.                        */
/*                                                                            */
/* Output: The mantissa.                                                      */
/******************************************************************************/
int32 PString::FloatP10(double *floatNum, int32 *floatSign, int32 precision)
{
	int32 i;
	int32 fPower = 0;
	int32 iPower = 256;
	int32 rPower = 256;
	double fRound = .5;

	// Find out the sign
	if (*floatNum < 0.0)
		*floatSign = -1;
	else
		*floatSign = 1;

	if (*floatNum != 0.0)
	{
		if (precision > 0)
		{
			if (precision < 309)
			{
				for (i = 0; i < 9; i++)
				{
					if (precision >= rPower)
					{
						fRound    /= posPower[i];
						precision -= rPower;
					}

					rPower >>= 1;
				}
			}
			else
				fRound = 0.0;
		}
		else
		{
			if (precision < 0)
			{
				if (precision > -310)
				{
					fRound = 5.0;
					for (i = 0; i < 9; i++)
					{
						if (precision >= rPower)
						{
							fRound    *= posPower[i];
							precision += rPower;
						}

						rPower >>= 1;
					}
				}
				else
					fRound = 0.0;
			}
		}

		*floatNum = fabs(*floatNum) + fRound;
		if (*floatNum < 1.0)
		{
			for (i = 0; i < 9; i++)
			{
				if (*floatNum <= negPower[i])
				{
					*floatNum *= posPower[i];
					fPower    -= iPower;
				}

				iPower >>= 1;
			}
		}
		else
		{
			if (*floatNum >= 10.0)
			{
				for (i = 0; i < 9; i++)
				{
					if (*floatNum >= posPower[i])
					{
						*floatNum /= posPower[i];
						fPower    += iPower;
					}

					iPower >>= 1;
				}
			}
		}

		if (*floatNum < 1.0)
		{
			*floatNum *= posPower[8];
			fPower--;
		}
	}

	return (fPower);
}



/******************************************************************************/
/* ToOctal() will store the unsigned number given into the buffer in octal.   */
/*                                                                            */
/* Input:  "buffer" is a pointer to the buffer where the number should be     */
/*         stored.                                                            */
/*         "length" is where the length of the string is stored.              */
/*         "number" is the number to convert.                                 */
/******************************************************************************/
void PString::ToOctal(uint16 *buffer, int32 &length, uint64 number)
{
	uint64 checkNum;
	uint16 tempNum;
	uint8 i;
	bool searching = true;

	// Initialize the length
	length = 0;

	// Begin to convert the number
	for (i = 21; i > 0; i--)
	{
		// Calculate the check number
		checkNum = (uint64)1 << (i * 3);

		if ((!searching) || (number >= checkNum))
		{
			tempNum   = (uint16)(number / checkNum);
			number   -= tempNum * checkNum;
			*buffer++ = tempNum + 0x0030;
			length++;

			searching = false;
		}
	}

	// Always store the 'ones'
	*buffer++ = (uint16)number + 0x0030;
	length++;
}



/******************************************************************************/
/* ToHex() will store the unsigned number given into the buffer in hex.       */
/*                                                                            */
/* Input:  "buffer" is a pointer to the buffer where the number should be     */
/*         stored.                                                            */
/*         "length" is where the length of the string is stored.              */
/*         "number" is the number to convert.                                 */
/*         "upper" is true if you want the letters in uppercase.              */
/******************************************************************************/
void PString::ToHex(uint16 *buffer, int32 &length, uint64 number, bool upper)
{
	uint64 checkNum;
	uint16 tempNum;
	uint8 i;
	bool searching = true;

	// Initialize the length
	length = 0;

	// Begin to convert the number
	for (i = 15; i > 0; i--)
	{
		// Calculate the check number
		checkNum = (uint64)1 << (i * 4);

		if ((!searching) || (number >= checkNum))
		{
			tempNum   = (uint16)(number / checkNum);
			number   -= tempNum * checkNum;

			if (tempNum >= 10)
				*buffer++ = tempNum - 10 + (upper ? 0x0041 : 0x0061);
			else
				*buffer++ = tempNum + 0x0030;

			length++;
			searching = false;
		}
	}

	// Always store the 'ones'
	if (number >= 10)
		*buffer++ = (uint16)number - 10 + (upper ? 0x0041 : 0x0061);
	else
		*buffer++ = (uint16)number + 0x0030;

	length++;
}
