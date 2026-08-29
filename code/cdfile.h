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

/* $Header: /CounterStrike/CDFILE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Westwood LIbrary                                             *
 *                                                                                             *
 *                    File Name : CDFILE.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : October 18, 1994                                             *
 *                                                                                             *
 *                  Last Update : October 18, 1994   [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "bfiofile.h"

/*
 * This class is derived from the BufferIOFileClass, and adds the ability to search across
 * several directories for a file. A file this player's own game wrote is found first, then
 * the current directory, then every directory in the search list in turn.
 *
 * A file opened for writing, created or deleted is not searched for at all. It resolves to
 * the player's own directory, so that what a deployment ships is read from and never written
 * over. A name that already carries a directory of its own is left exactly as it was given.
 *
 * The search order is whatever order the directories were handed to Set_Search_Drives(),
 * which takes them in the same semicolon separated form the DOS PATH variable used.
 */
class CDFileClass : public BufferIOFileClass
{
		typedef BufferIOFileClass BASECLASS;

	public:
		CDFileClass(char const *filename);
		CDFileClass(void);
		virtual ~CDFileClass(void) override;

		virtual char const * Set_Name(char const *filename) override;
		virtual int Open(char const *filename, int rights=READ) override;
		virtual int Open(int rights=READ) override;
		virtual int Delete(void) override;

		void Searching(int on) {IsDisabled = !on;};

		static int Set_Search_Drives(char * pathlist);
		static void Add_Search_Drive(char const * path);
		static void Clear_Search_Drives(void);
		static char const * Search_Path(int index);

		static void Set_User_Path(char const * path);
		static char const * User_Path(void);

		static bool Find_First_File(char *buffer);
		static bool Find_Next_File(char *buffer);
		static void Find_Close(void);

	private:

		char const * Capture_Name(char const * filename);
		void Point_At_Own_Copy(void);

		static bool Has_Directory(char const * filename);
		static bool User_Path_For(char const * filename, char * buffer, int size);

		/*
		**	Is multi-drive searching disabled for this file object?
		*/
		bool IsDisabled;

		/*
		**	The name the game asked for. Every later decision is made from this rather than
		**	from the name the object carries, because that one records where a copy was
		**	found and answers a different question.
		*/
		char const * RequestedName;

		/*
		**	This is the control record for each of the drives specified in the search
		**	path. There can be many such search paths available.
		*/
		struct SearchDriveType {
			void * Next;        // Pointer to next search record.
			char const * Path;  // Pointer to path string.
		};

		/*
		**	This points to the first path record.
		*/
		static SearchDriveType * First;

		/*
		**	Where this player's own files are kept, ending in a separator. NULL when the
		**	player has no directory of their own and everything belongs together.
		*/
		static char const * UserPath;

		// A file object owns the name it captured, so it is not copied.
		CDFileClass(CDFileClass const & file);
		CDFileClass & operator = (CDFileClass const & file);
};
