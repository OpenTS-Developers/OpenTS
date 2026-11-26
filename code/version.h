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

/* $Header: /counterstrike/VERSION.H 2     3/10/97 6:22p Steve_tall $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : VERSION.H                                *
 *                                                                         *
 *                   Programmer : Bill R. Randolph                         *
 *                                                                         *
 *                   Start Date : 10/26/95                                 *
 *                                                                         *
 *                  Last Update : October 26, 1995 [BRR]                   *
 *                                                                         *
 * This class maintains version information, and communications protocol   *
 * information.                                                            *
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "opents_version.h"

enum CommProtocolType {
	COMM_PROTOCOL_SINGLE_NO_COMP = 0,   // single frame with no compression
	COMM_PROTOCOL_SINGLE_E_COMP,        // single frame with event compression
	COMM_PROTOCOL_MULTI_E_COMP,         // multiple frame with event compression
	COMM_PROTOCOL_COUNT,
	DEFAULT_COMM_PROTOCOL = COMM_PROTOCOL_MULTI_E_COMP
};

struct VersionProtocolType {
	unsigned int Version;
	CommProtocolType Protocol;
};

class VersionClass {
	public:
		//.....................................................................
		// Constructor/Destructor
		//.....................................................................
		VersionClass(void);
		virtual ~VersionClass(void) {};

		//.....................................................................
		// These routines return the current version number.  The long version
		// number contains the major version in the high word, and the minor
		// version in the low word.  They should be interpreted in hex.
		//.....................................................................
		unsigned int Version_Number(void);
		unsigned short Major_Version(void);
		unsigned short Minor_Version(void);

		//.....................................................................
		// Retrieves a pointer to the version # as a text string (#.#), with
		// the trailing 0's trimmed off.
		//.....................................................................
		char *Version_Name(void);

		//.....................................................................
		// Retrieves a pointer to the current version text.
		//.....................................................................
		char *Version_Text(void) {return(VersionText);}

		//.....................................................................
		// Returns the default comm protocol for a given version number.
		//.....................................................................
		CommProtocolType Version_Protocol(unsigned int version);

		//.....................................................................
		// These routines support "version clipping".
		//.....................................................................
		void Init_Clipping(void);
		unsigned int Clip_Version(unsigned int minver, unsigned int maxver);
		unsigned int Get_Clipped_Version(void) {return(MaxClipVer);}

		//.....................................................................
		// These routines return the theoretical lowest & highest version #'s
		// that this program will connect to; this does not take any previous
		// version clipping into account.
		//.....................................................................
		unsigned int Min_Version(void);
		unsigned int Max_Version(void);

	private:
		//.....................................................................
		// Fills in a 'VersionText' with a descriptive version name.
		//.....................................................................
		void Read_Text_String(void);

		// The version the program reports, taken from the project version so that the
		// number a build negotiates with matches the number it displays. The minor
		// field carries the SemVer minor and patch, one byte each, which makes the
		// composed version number the packed version. The save stamp adds a serial
		// above it, which this number leaves out.
		enum VersionEnum {
			MAJOR_VERSION = OPENTS_VERSION_MAJOR,
			MINOR_VERSION = (OPENTS_VERSION_MINOR << 8) | OPENTS_VERSION_PATCH
		};

		// The versions this program will connect to. A prerelease is not distinguished
		// here, so a prerelease and its final release connect to each other.
		enum VersionRangeEnum {
			// ajw - We can only play against same version.
			MIN_VERSION = OPENTS_VERSION_PACKED,
			MAX_VERSION = OPENTS_VERSION_PACKED
		};

		//.....................................................................
		// This is the program's version number, stored internally.
		//.....................................................................
		unsigned int Version;
		unsigned short MajorVer;
		unsigned short MinorVer;

		//.....................................................................
		// This array is used for formatting the version # as a string
		//.....................................................................
		char VersionName[30];

		//.....................................................................
		// This array contains special version labels (such as "Beta"), stored
		// in the file VERSION.TXT.  If the file isn't present, no label is
		// shown.
		//.....................................................................
		char VersionText[16];

		//.....................................................................
		// Values used for "Version Clipping"
		//.....................................................................
		unsigned int MinClipVer;
		unsigned int MaxClipVer;

		//.....................................................................
		// Bitfield Flags
		// IsInitialized: is set if the VERSION.TXT file has been read
		//.....................................................................
		unsigned VersionInit :		1;
		unsigned MajorInit :		1;
		unsigned MinorInit :		1;
		unsigned TextInit :			1;
};

char const * Version_Name(void);

/************************** end of version.h *******************************/
