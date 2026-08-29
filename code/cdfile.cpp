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

/* $Header: /CounterStrike/CDFILE.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Westwood Library                                             *
 *                                                                                             *
 *                    File Name : CDFILE.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : October 18, 1994                                             *
 *                                                                                             *
 *                  Last Update : September 22, 1995 [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CDFileClass::Clear_Search_Drives -- Removes all record of a search path.                  *
 *   CDFileClass::Open -- Opens the file object -- with path search.                           *
 *   CDFileClass::Open -- Opens the file wherever it can be found.                             *
 *   CDFileClass::Set_Name -- Performs a multiple directory scan to set the filename.          *
 *   CDFileClass::Set_Search_Drives -- Sets a list of search paths for file access.            *
 *   Is_Disk_Inserted -- Checks to see if a disk is inserted in specified drive.               *
 *   harderr_handler -- Handles hard DOS errors.                                               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "cdfile.h"

/*
**	Pointer to the first search path record.
*/
CDFileClass::SearchDriveType * CDFileClass::First = NULL;

/*
**	Where this player's own files are kept.
*/
char const * CDFileClass::UserPath = NULL;


/// <summary>
/// Constructs a CD file object for the file specified.
/// The name is searched for in the player's own directory, the current directory and every
/// configured path, so the object refers to the first matching local file.
/// </summary>
/// <param name="filename">The name of the file this object should refer to.</param>
CDFileClass::CDFileClass(char const *filename) :
	IsDisabled(false),
	RequestedName(NULL)
{
	CDFileClass::Set_Name(filename);
//	memset (RawPath, 0, sizeof(RawPath));
}


/// <summary>
/// Constructs a CD file object with no file attached to it yet.
/// Use Set_Name to give the object a file to work with before trying to open it.
/// </summary>
CDFileClass::CDFileClass(void) :
	IsDisabled(false),
	RequestedName(NULL)
{
}


CDFileClass::~CDFileClass(void)
{
	Capture_Name(NULL);
}


/***********************************************************************************************
 * CDFileClass::Open -- Opens the file object -- with path search.                             *
 *                                                                                             *
 *    This will open the file object, but since the file object could have been constructed    *
 *    with a pathname, this routine will try to find the file first. For files opened for      *
 *    writing, then use the existing filename without performing a path search.                *
 *                                                                                             *
 * INPUT:   rights   -- The access rights to use when opening the file                         *
 *                                                                                             *
 * OUTPUT:  bool; Was the open successful?                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int CDFileClass::Open(int rights)
{
	/*
	**	A file being written belongs to the player, so it is opened where the player's own
	**	files are kept rather than wherever a copy happened to be found. What a deployment
	**	ships is read from and never written over.
	*/
	if ((rights & WRITE) != 0) {
		Point_At_Own_Copy();
	}

	return(BASECLASS::Open(rights));
}


/// <summary>
/// Adds a list of directories to search when a file is not in the current directory.
/// The list is written the way DOS wrote a PATH -- entries separated by semicolons, with
/// or without a trailing backslash. Each entry is appended to the search chain in the
/// order given, so the first directory named is the first one tried.
/// </summary>
/// <param name="pathlist">The semicolon separated list of directories to add.</param>
/// <returns>int; Zero if at least one directory was added, or 1 if the list held none.</returns>
int CDFileClass::Set_Search_Drives(char * pathlist)
{
	bool found = false;

	/*
	**	If there is no pathlist to add, then just return.
	*/
	if (!pathlist) return(0);

	char *copy_pathlist = strdup(pathlist);

	char const * ptr = strtok(copy_pathlist, ";");
	while (ptr != NULL) {
		if (strlen(ptr) > 0) {

			char path[MAX_PATH];						// Working path buffer.

			// A directory with no room left for a trailing separator cannot become half
			// of a pathname, so it is passed over.
			if (strlen(ptr) + 1 >= sizeof(path)) {
				ptr = strtok(NULL, ";");
				continue;
			}

			/*
			**	Fixup the path to be legal. Legal is defined as all that is necessary to
			**	create a pathname is to append the actual filename submitted to the
			**	file system. This means that it must have either a trailing ':' or '\'
			**	character.
			*/
			strcpy(path, ptr);
			switch (path[strlen(path)-1]) {
				case ':':
				case '\\':
					break;

				default:
					strcat(path, "\\");
					break;
			}

			found	= true;
			Add_Search_Drive(path);
		}

		/*
		**	Find the next path string and resubmit.
		*/
		ptr = strtok(NULL, ";");
	}

	free(copy_pathlist);

	if (!found) return(1);
	return(0);
}


/***********************************************************************************************
 * CDFC::Add_Search_Drive -- Add a new path to the search path list                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    path                                                                              *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    5/22/96 10:12AM ST : Created                                                             *
 *=============================================================================================*/
void CDFileClass::Add_Search_Drive(char const * path)
{
	SearchDriveType *srch;					// Working pointer to path object.
	/*
	**	Allocate a record structure.
	*/
	srch	= new SearchDriveType;

	/*
	**	Attach the path to this structure.
	*/
	srch->Path = strdup(path);
	srch->Next = NULL;

	/*
	**	Attach this path record to the end of the path chain.
	*/
	if (!First) {
		First = srch;
	} else {
		SearchDriveType * chain = First;

		while (chain->Next) {
			chain = (SearchDriveType *)chain->Next;
		}
		chain->Next = srch;
	}
}


/// <summary>
/// Adds a path to the front of the search chain, so that it is tried before every path
/// already added. The current directory is still examined first.
/// </summary>
/// <param name="path">The path to search before all the others.</param>
void CDFileClass::Add_Search_Drive_Front(char const * path)
{
	SearchDriveType * srch = new SearchDriveType;

	srch->Path = strdup(path);
	srch->Next = First;

	First = srch;
}


/// <summary>
/// Records where this player's own files are kept.
/// A file the game writes, creates or deletes goes here, and a file it reads is looked for
/// here before anywhere else. Passing nothing puts the game back to keeping everything
/// together in the directory it is run from.
/// </summary>
/// <param name="path">The directory to keep the player's own files in.</param>
void CDFileClass::Set_User_Path(char const * path)
{
	if (UserPath != NULL) {
		free((char *)UserPath);
		UserPath = NULL;
	}

	if (path == NULL || *path == '\0') return;

	char terminated[MAX_PATH];

	// A directory with no room left for a trailing separator cannot become half of a
	// pathname, so it is refused rather than kept in a form nothing can be appended to.
	if (strlen(path) + 1 >= sizeof(terminated)) return;

	strcpy(terminated, path);
	switch (terminated[strlen(terminated)-1]) {
		case ':':
		case '/':
		case '\\':
			break;

		default:
			strcat(terminated, "\\");
			break;
	}

	UserPath = strdup(terminated);
}


char const * CDFileClass::User_Path(void)
{
	return(UserPath);
}


/*
**	A name that names a directory of its own has already said where it goes, so neither the
**	search nor the player's own directory touches it. The characters are the ones a
**	directory is allowed to end with.
*/
bool CDFileClass::Has_Directory(char const * filename)
{
	return(filename != NULL && strpbrk(filename, "\\/:") != NULL);
}


/*
**	Where a file belongs once it is the player's own. Fails when there is no such directory,
**	when the caller has already named one, or when the two will not make one pathname.
*/
bool CDFileClass::User_Path_For(char const * filename, char * buffer, int size)
{
	if (UserPath == NULL || filename == NULL) return(false);
	if (Has_Directory(filename)) return(false);
	if ((int)(strlen(UserPath) + strlen(filename)) >= size) return(false);

	strcpy(buffer, UserPath);
	strcat(buffer, filename);
	return(true);
}


/*
**	Keeps a copy of the name the game asked for. The copy is this object's own, so that the
**	name survives the object being pointed at a copy found elsewhere, and survives a mixfile
**	lookup writing over the name in place.
*/
char const * CDFileClass::Capture_Name(char const * filename)
{
	char * captured = (filename != NULL) ? strdup(filename) : NULL;

	if (RequestedName != NULL) {
		free((char *)RequestedName);
	}
	RequestedName = captured;

	return(RequestedName);
}


/*
**	Points the object at the file this player's own game owns, which is where a write and a
**	delete both belong. Worked out from the name that was asked for rather than from the one
**	the object carries, so that it lands in the same place however often it is done.
*/
void CDFileClass::Point_At_Own_Copy(void)
{
	if (IsDisabled || RequestedName == NULL) return;

	char path[_MAX_PATH];

	if (User_Path_For(RequestedName, path, sizeof(path))) {
		BASECLASS::Set_Name(path);
	} else {
		BASECLASS::Set_Name(RequestedName);
	}
}


/// <summary>
/// Reports the search path at a position in the chain, counting from zero in the order the
/// paths are tried. This is how a scan covers the same folders a file open would.
/// </summary>
/// <param name="index">The position in the search chain.</param>
/// <returns>The path at that position, or NULL once the end of the chain is passed.</returns>
char const * CDFileClass::Search_Path(int index)
{
	SearchDriveType const * srch = First;

	while (srch != NULL && index > 0) {
		srch = (SearchDriveType const *)srch->Next;
		index--;
	}

	return(srch != NULL ? srch->Path : NULL);
}


/***********************************************************************************************
 * CDFileClass::Clear_Search_Drives -- Removes all record of a search path.                    *
 *                                                                                             *
 *    Use this routine to clear out any previous path(s) set with Set_Search_Drives()          *
 *    function.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void CDFileClass::Clear_Search_Drives(void)
{
	SearchDriveType	* chain;			// Working pointer to path chain.

	chain = First;
	while (chain) {
		SearchDriveType	*next;

		next = (SearchDriveType *)chain->Next;
		if (chain->Path) {
			free((char *)chain->Path);
		}
		delete chain;

		chain = next;
	}
	First = 0;
}


/// <summary>
/// Searches the current directory and configured local data paths for a file.
/// The first match becomes this object's filename; if none is found, the raw filename is kept.
/// </summary>
/// <param name="filename">The file name to search for.</param>
/// <returns>The selected file name, including a configured path when one supplies the match.</returns>
char const * CDFileClass::Set_Name(char const *filename)
{
	/*
	**	Kept before anything else, because the name the object ends up carrying records
	**	where a copy was found, and a write has to go back to what was asked for.
	*/
	filename = Capture_Name(filename);

	/*
	**	Try to find the file in the current directory first. If it can be found, then
	**	just return with the normal file name setting process. Do the same if there is
	**	no multi-drive search path.
	*/
	BASECLASS::Set_Name(filename);
	if (IsDisabled || !First || filename == NULL || BASECLASS::Is_Available()) return(File_Name());

	/*
	**	Attempt to find the file first. Check the current directory. If not found there, then
	**	search all the path specifications available. If it still can't be found, then just
	**	fall into the normal raw file filename setting system.
	*/
	SearchDriveType * srch = First;

	while (srch) {
		char path[_MAX_PATH];

		// A directory and a name that will not make one pathname between them are passed
		// over rather than truncated into a different name.
		if (strlen(srch->Path) + strlen(filename) < sizeof(path)) {

			/*
			**	Build a pathname to search for.
			*/
			strcpy(path, srch->Path);
			strcat(path, filename);

			// Check this path. Is_Available returns false when the file cannot be opened,
			// allowing the search to continue with the next configured path.
			BASECLASS::Set_Name(path);
			if (BASECLASS::Is_Available()) {
				return(File_Name());
			}
		}

		/*
		**	It wasn't found, so try the next path entry.
		*/
		srch = (SearchDriveType *)srch->Next;
	}

	/*
	**	At this point, all path searching has failed. Just set the file name to the
	**	plain text passed to this routine and be done with it.
	*/
	BASECLASS::Set_Name(filename);
	return(File_Name());
}


/***********************************************************************************************
 * CDFileClass::Open -- Opens the file wherever it can be found.                               *
 *                                                                                             *
 *    This routine is similar to the RawFileClass open except that if the file is being        *
 *    opened only for READ access, it will search all specified directories looking for the    *
 *    file. If after a complete search the file still couldn't be found, then it is opened     *
 *    using the normal BufferIOFileClass system -- resulting in normal error procedures.       *
 *                                                                                             *
 * INPUT:   filename -- Pointer to the override filename to supply for this file object. It    *
 *                      would be the base filename (sans any directory specification).         *
 *                                                                                             *
 *          rights   -- The access rights to use when opening the file.                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the file opened successfully? If so then the filename may be different   *
 *                than requested. The location of the file can be determined by examining the  *
 *                filename of this file object. The filename will contain the complete         *
 *                pathname used to open the file.                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int CDFileClass::Open(char const *filename, int rights)
{
	CDFileClass::Close();

	/*
	**	Verify that there is a filename associated with this file object. If not, then this is a
	**	big error condition.
	*/
	if (!filename) {
		Error(ENOENT, false);
	}

	/*
	**	If writing is requested, then multiple drive searching is not performed.
	*/
	if (IsDisabled || (rights & WRITE) != 0) {

		BASECLASS::Set_Name( Capture_Name(filename) );
		return( CDFileClass::Open( rights ) );
	}

	/*
	**	Perform normal multiple drive searching for the filename and open
	**	using the normal procedure.
	*/
	Set_Name(filename);
	return(BASECLASS::Open(rights));
}


/// <summary>
/// Deletes this player's own copy of the file.
/// What a deployment ships is read from and never removed, so a file thrown away here falls
/// back to the copy it shipped with rather than disappearing altogether.
/// </summary>
/// <returns>int; Was a file deleted?</returns>
int CDFileClass::Delete(void)
{
	Point_At_Own_Copy();

	return(BASECLASS::Delete());
}


HANDLE FindFileHandle = INVALID_HANDLE_VALUE;

/// <summary>
/// Begins a search for the files matching the wildcard specified.
/// This routine will look in the current directory first and then work along the search
/// drive list, settling on the first drive that has a match. Only ordinary files qualify;
/// directories and system, hidden, or temporary files are passed over. Any search still
/// in progress is closed off first.
/// </summary>
/// <param name="fname">The wildcard to search for; filled in with the file found.</param>
/// <returns>bool; Was a matching file found?</returns>
/// <remarks>Be sure that the buffer is big enough to hold the filename returned.</remarks>
bool CDFileClass::Find_First_File(char *fname)
{
	WIN32_FIND_DATAA fb;
	char scan_path[MAX_PATH];
	SearchDriveType *entry;

	if (fname) {

		Find_Close();

		strcpy(scan_path, fname);

		HANDLE file_handle = ::FindFirstFile(scan_path, &fb);
		if (file_handle != INVALID_HANDLE_VALUE && !(fb.dwFileAttributes & (FILE_ATTRIBUTE_TEMPORARY|FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN))) {

			strcpy(fname, fb.cFileName);
			FindFileHandle = file_handle;

			return(true);
		}

		entry = First;

		if (entry != NULL) {

			while (true) {

				strcpy(scan_path, entry->Path);
				strcat(scan_path, fname);

				file_handle = ::FindFirstFile(scan_path, &fb);
				if (file_handle != INVALID_HANDLE_VALUE && !(fb.dwFileAttributes & (FILE_ATTRIBUTE_TEMPORARY|FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN))) {
					break;
				}

				entry = (SearchDriveType *)entry->Next;
				if (entry == NULL) {
					return(false);
				}
			}

			strcpy(fname, fb.cFileName);
			FindFileHandle = file_handle;

			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Fetches the next file that matches the search in progress.
/// This routine continues the scan begun by Find_First_File, working through the rest of
/// the matches on whichever drive that routine settled upon.
/// </summary>
/// <param name="buffer">Buffer to fill in with the name of the file found.</param>
/// <returns>bool; Was another matching file found?</returns>
/// <remarks>Be sure that the buffer is big enough to hold the filename returned.</remarks>
bool CDFileClass::Find_Next_File(char *buffer)
{
	WIN32_FIND_DATAA fb;

	if (buffer) {

		if (FindFileHandle != INVALID_HANDLE_VALUE && ::FindNextFile(FindFileHandle, &fb) == TRUE) {
			strcpy(buffer, fb.cFileName);
			return(true);
		}

		buffer[0] = '\0';
	}
	return(false);
}


/// <summary>
/// Closes off the file search that is in progress.
/// Call this routine when the results of a Find_First_File scan are no longer wanted, so
/// that the search handle held on the game's behalf is given back to the system.
/// </summary>
void CDFileClass::Find_Close(void)
{
	if (FindFileHandle != INVALID_HANDLE_VALUE) {
		FindClose(FindFileHandle);
		FindFileHandle = INVALID_HANDLE_VALUE;
	}
}
