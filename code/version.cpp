/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/* $Header: /CounterStrike/VERSION.CPP 14    3/16/97 10:16p Joe_b $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : VERSION.CPP                              *
 *                                                                         *
 *                   Programmer : Bill R. Randolph                         *
 *                                                                         *
 *                   Start Date : 10/26/95                                 *
 *                                                                         *
 *                  Last Update : September 17, 1996 [JLB]                 *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   VersionClass::VersionClass -- Class constructor                       *
 *   VersionClass::Version_Number -- Returns program version number        *
 *   VersionClass::Major_Version -- returns major version #                *
 *   VersionClass::Minor_Version -- returns minor version (revision) number*
 *   VersionClass::Version_Name -- returns version # as char string        *
 *   VersionClass::Read_Text_String -- reads version text string from disk *
 *   VersionClass::Version_Protocol -- returns default protocol for version*
 *   VersionClass::Init_Clipping -- Initializes version clipping           *
 *   VersionClass::Clip_Version -- "clips" the given version range         *
 *   VersionClass::Min_Version -- returns lowest version # to connect to   *
 *   VersionClass::Max_Version -- returns highest version # to connect to  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "version.h"

#include "globals.h"
#include "rawfile.h"
#include "utf8.h"
#include "winstub.h"


/****************************** Globals ************************************/
// This is a table of version numbers # the communications protocol used for
// that version number.  It's used by the game owner to determine the
// protocol to be used for a given session.
//
// This table needs to be updated every time a new communications protocol
// is implemented, not every time a new version is created.
//
// A given protocol is used from its corresponding version #, up to (but not
// including) the next version number in the table.  The last protocol in
// the table is the default protocol for this version.
//
// One entry covers every version, because OpenTS speaks one protocol. A threshold
// added here has to be an OpenTS version number: they start from zero again, so a
// threshold drawn from the original game's numbering would capture OpenTS versions
// that happen to reach it.
static VersionProtocolType VersionProtocol[] = {
	{0x00000000,COMM_PROTOCOL_MULTI_E_COMP},
};


/************************************************************************** *
 * VersionClass::VersionClass -- Class constructor                          *
 *                                                                          *
 * INPUT:                                                                   *
 *      none.                                                               *
 *                                                                          *
 * OUTPUT:                                                                  *
 *      none.                                                               *
 *                                                                          *
 * WARNINGS:                                                                *
 *      none.                                                               *
 *                                                                          *
 * HISTORY:                                                                 *
 *   10/26/1995 BRR : Created.                                              *
 *   09/17/1996 JLB : Converted to used initializer list.                   *
 *=========================================================================*/
VersionClass::VersionClass(void) :
	Version(0),
	MajorVer(0),
	MinorVer(0),
	MinClipVer(0),
	MaxClipVer(0),
	VersionInit(false),
	MajorInit(false),
	MinorInit(false),
	TextInit(false)
{
	VersionText[0] = '\0';
	VersionName[0] = '\0';
}


/***************************************************************************
 * VersionClass::Version_Number -- Returns program version number          *
 *                                                                         *
 * Version Number Format:                                                  *
 *   Byte 3,2: major version                                               *
 *   Byte 1:   minor version                                               *
 *   Byte 0:   patch version                                               *
 *   Thus, version 1.7.2 would appear as 0x0001 0702                       *
 *                                                                         *
 *   This format guarantees that a greater-than or less-than comparison    *
 *   will work on version numbers.                                         *
 *                                                                         *
 * The number should be printed in hex.                                    *
 *                                                                         *
 * This routine also fills in a text string (retrieved with Version_Text), *
 * which may contain a custom string (such as "Beta"); this string is      *
 * read from the file VERSION.TXT.                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      Version number                                                     *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Don't call this function until the file system has been init'd!    *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned int VersionClass::Version_Number(void)
{
	//------------------------------------------------------------------------
	// Read the text description, if there is one
	//------------------------------------------------------------------------
	if (!TextInit) {
		Read_Text_String();
		TextInit = 1;
	}

	//------------------------------------------------------------------------
	// If the version has already been set, just return it.
	//------------------------------------------------------------------------
	if (VersionInit) {
		return(Version);
	}

	//------------------------------------------------------------------------
	// Generate the version #
	//------------------------------------------------------------------------
	Version = ((Major_Version() << 16) | Minor_Version());
	VersionInit = 1;

	return(Version);

}	/* end of Version_Number */


/***************************************************************************
 * VersionClass::Major_Version -- returns major version #                  *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      Major Version number                                               *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Don't call this function until the file system has been init'd!    *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned short VersionClass::Major_Version(void)
{
	//------------------------------------------------------------------------
	// Read the text description, if there is one
	//------------------------------------------------------------------------
	if (!TextInit) {
		Read_Text_String();
		TextInit = 1;
	}

	//------------------------------------------------------------------------
	// If the major version # is already set, just return it.
	//------------------------------------------------------------------------
	if (MajorInit) {
		return(MajorVer);
	}

	MajorVer = MAJOR_VERSION;
	MajorInit = 1;

	return(MajorVer);

}	/* end of Major_Version */


/***************************************************************************
 * VersionClass::Minor_Version -- returns minor version (revision) number  *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      Minor Version number                                               *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Don't call this function until the file system has been init'd!    *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned short VersionClass::Minor_Version(void)
{
	//------------------------------------------------------------------------
	// Read the text description, if there is one
	//------------------------------------------------------------------------
	if (!TextInit) {
		Read_Text_String();
		TextInit = 1;
	}

	//------------------------------------------------------------------------
	// If the minor version # is already set, just return it.
	//------------------------------------------------------------------------
	if (MinorInit) {
		return(MinorVer);
	}

	MinorVer = MINOR_VERSION;
	MinorInit = 1;

	return(MinorVer);

}	/* end of Minor_Version */


/***************************************************************************
 * VersionClass::Version_Name -- returns version # as char string          *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      ptr to name                                                        *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/30/1995 BRR : Created.                                             *
 *=========================================================================*/
char * VersionClass::Version_Name(void)
{
	strncpy(VersionName, OPENTS_VERSION, sizeof(VersionName) - 1);
	VersionName[sizeof(VersionName) - 1] = '\0';

	return(VersionName);

}	/* end of Version_Name */


/***************************************************************************
 * VersionClass::Read_Text_String -- reads version # text string from disk *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Don't call this function until the file system has been init'd!    *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
void VersionClass::Read_Text_String(void)
{
	RawFileClass file("VERSION.TXT");

	if (file.Is_Available()) {
		file.Read(VersionText, sizeof(VersionText));
		VersionText[sizeof(VersionText)-1] = '\0';
		std::size_t bom = UTF8::BOM_Length(VersionText);
		memmove(VersionText, VersionText + bom, strlen(VersionText + bom) + 1);
		while (VersionText[strlen(VersionText)-1] == '\r') {
			VersionText[strlen(VersionText)-1] = '\0';
		}
	} else {
		VersionText[0] = '\0';
	}

}	/* end of Read_Text_String */


/***************************************************************************
 * VersionClass::Version_Protocol -- returns default protocol for version  *
 *                                                                         *
 * INPUT:                                                                  *
 *      version      version # to look up                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      protocol value to use for that version #                           *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
CommProtocolType VersionClass::Version_Protocol(unsigned int version)
{
	int i,j;

	//------------------------------------------------------------------------
	// Compute # entries in the VersionProtocol table
	//------------------------------------------------------------------------
	j = sizeof (VersionProtocol) / sizeof(VersionProtocolType);

	//------------------------------------------------------------------------
	// Search backwards through the table, finding the first entry for which
	// the given version # is >= the table's; this is the range containing
	// the given version number.
	//------------------------------------------------------------------------
	for (i = j - 1; i >= 0; i--) {
		if (version >= VersionProtocol[i].Version) {
			return(VersionProtocol[i].Protocol);
		}
	}

	//------------------------------------------------------------------------
	// If no range was found for the given version, return the highest
	// possible protocol.  (If version clipping is being done properly, this
	// case should never happen, but never say never.)
	//------------------------------------------------------------------------
	return(VersionProtocol[j-1].Protocol);

}	/* end of Version_Protocol */


/***************************************************************************
 * VersionClass::Init_Clipping -- Initializes version clipping             *
 *                                                                         *
 * Initializes the Min & Max clip version #'s to the min & max values      *
 * defined for this program.  This sets the initial range for use by       *
 * the Clip_Version routine.                                               *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
void VersionClass::Init_Clipping(void)
{
	MinClipVer = Min_Version();
	MaxClipVer = Max_Version();

}	/* end of Init_Clipping */


/***************************************************************************
 * VersionClass::Clip_Version -- "clips" the given version range           *
 *                                                                         *
 * This routine compares another program's supported min/max version       *
 * range with the range currently defined by 'MinClipVer' and 'MaxClipVer'.*
 * If there is overlap in the two ranges, Min & MaxClipVer are adjusted    *
 * to the bounds of the overlap. The routine returns the largest version   *
 * number shared by the ranges (MaxClipVer).                               *
 *                                                                         *
 * Thus, by calling Init_Clipping(), then a series of Clip_Version() calls,*
 * a mutually-acceptable range of version #'s may be negotiated between    *
 * different versions of this program.  The max shared version may then    *
 * be used to decide upon a communications protocol that all programs      *
 * support.                                                                *
 *                                                                         *
 * INPUT:                                                                  *
 *      minver      min version to clip to                                 *
 *      maxver      max version to clip to                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      highest clipped version #                                          *
 *      0 = given range is below our current range                         *
 *      0xFFFFFFFF = given range is above our current range                *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Be sure Init_Clipping() was called before performing a clipping    *
 *      session.                                                           *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned int VersionClass::Clip_Version(unsigned int minver,
	unsigned int maxver)
{
	//------------------------------------------------------------------------
	// If the given range is outside & above our own, return an error.
	//------------------------------------------------------------------------
	if (minver > MaxClipVer)
		return(0xffffffff);

	//------------------------------------------------------------------------
	// If the given range is outside & below our own, return an error.
	//------------------------------------------------------------------------
	if (maxver < MinClipVer)
		return(0);

	//------------------------------------------------------------------------
	// Clip the lower range value
	//------------------------------------------------------------------------
	if (minver > MinClipVer)
		MinClipVer = minver;

	//------------------------------------------------------------------------
	// Clip the upper range value
	//------------------------------------------------------------------------
	if (maxver < MaxClipVer)
		MaxClipVer = maxver;

	//------------------------------------------------------------------------
	// Return the highest version supported by the newly-adjusted range.
	//------------------------------------------------------------------------
	return(MaxClipVer);

}	/* end of Clip_Version */


/***************************************************************************
 * VersionClass::Min_Version -- returns lowest version # to connect to     *
 *                                                                         *
 * Returns the minimum version # this program will connect to.             *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      min version #                                                      *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned int VersionClass::Min_Version(void)
{
	return(MIN_VERSION);

}	/* end of Min_Version */


/***************************************************************************
 * VersionClass::Max_Version -- returns highest version # to connect to    *
 *                                                                         *
 * Returns the maximum version # this program will connect to.             *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      max version #                                                      *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/26/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned int VersionClass::Max_Version(void)
{
	return(MAX_VERSION);
}	/* end of Max_Version */


/// <summary>
/// Fetches the product version string of the running program.
/// This routine pulls the ProductVersion field out of the version resource of the executable
/// itself, so the number shown to the player always matches the build that shipped. Use this
/// routine for anything the player sees -- the menu version line, the crash report, and the
/// sync error log all print it. The string is looked up once and remembered thereafter.
/// </summary>
/// <returns>Returns with the product version text. A placeholder is returned if the program
/// carries no readable version resource.</returns>
char const * Version_Name(void)
{
	static char buffer[512] = {"zzz"};
	static bool empty = true;

	int size;
	void *block;
	unsigned int translate_len;
	DWORD handle;

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *translate;

	char query[128];
	char filename[MAX_PATH];

	if (empty) {

		empty = false;

		if (GetModuleFileName(ProgramInstance, filename, sizeof(filename)) > 0) {
			handle = 1;
			size = GetFileVersionInfoSize(filename, &handle);
			if (size > 0) {
				block = new BYTE[size];
				if (GetFileVersionInfo(filename, handle, size, block)) {
					VerQueryValue(block, "\\VarFileInfo\\Translation", (LPVOID *)&translate, &translate_len);
					if (translate_len > 0) {
						sprintf(query, "\\StringFileInfo\\%04X%04X\\ProductVersion", translate->wLanguage, translate->wCodePage);
						VerQueryValue(block, query, (LPVOID *)&translate, &translate_len);
						if (translate_len > 0) {
							strcpy(buffer, (const char *)translate);
						}
					}
				}

				delete [] block;
			}
		}
	}
	return(buffer);

}
