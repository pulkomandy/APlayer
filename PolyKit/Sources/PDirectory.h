/******************************************************************************/
/* PDirectory header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PDirectory_h
#define __PDirectory_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "ImportExport.h"


/******************************************************************************/
/* PDirectory class                                                           */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

/**
 *	This helper class is useful for creating and utilizing directory names, but
 *	also finding directories as for example the OS directory containing user
 *	settings. This class can also be used for creating new directories and
 *	check if a specific directory exists.
 */
class _IMPEXP_PKLIB PDirectory
{
public:
	/// Standard directory types
	enum PFindType
	{
		/// The launch directory of the application
		pLaunch,
		/// The setting directory where the user settings are stored
		pSettings,
		/// The library directory
		pLibs,
		/// The user home directory
		pUser
	};

	/// Directory entry type to return in enumeration
	enum PEntryType
	{
		/// Returns any entry
		pAny,
		/// Returns files only
		pFile,
		/// Returns directories only
		pDirectory,
		/// Returns links only (including Windows shortcuts)
		pLink
	};

	PDirectory(void);
	PDirectory(PString name);
	virtual ~PDirectory(void);

	void SetDirectory(PString name);
	PString GetDirectory(void) const;
	void Append(PString name);

	void FindDirectory(PFindType type);
	void CreateDirectory(void) const;

	bool Contains(PString name, PEntryType type = pAny) const;

	void InitEnum(PEntryType type);
	bool GetNextEntry(PString &name, PEntryType &type);
	void EndEnum(void);

	static PString EnsureDirectoryName(PString path);
	static PString GetDirectoryPart(PString path);
	static PString GetFilePart(PString path);

	static PString GetParentDirectory(PString path);
	static PString GetRootDirectory(PString path);

	static bool DirectoryExists(PString path);

protected:
	PString dirName;				// The directory name
	PEntryType enumType;			// The entry type to return in enumeration
	BDirectory *enumDir;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
