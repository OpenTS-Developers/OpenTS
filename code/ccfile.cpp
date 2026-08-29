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

/* $Header: /CounterStrike/CCFILE.CPP 2     3/13/97 2:05p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CCFILE.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 8, 1994                                               *
 *                                                                                             *
 *                  Last Update : August 5, 1996 [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CCFileClass::CCFileClass -- Default constructor for file object.                          *
 *   CCFileClass::CCFileClass -- Filename based constructor for C&C file.                      *
 *   CCFileClass::Close -- Closes the file.                                                    *
 *   CCFileClass::Error -- Handles displaying a file error message.                            *
 *   CCFileClass::Is_Available -- Checks for existence of file on disk or in mixfile.          *
 *   CCFileClass::Is_Open -- Determines if the file is open.                                   *
 *   CCFileClass::Open -- Opens a file from either the mixfile system or the rawfile system.   *
 *   CCFileClass::Read -- Reads data from the file.                                            *
 *   CCFileClass::Seek -- Moves the current file pointer in the file.                          *
 *   CCFileClass::Size -- Determines the size of the file.                                     *
 *   CCFileClass::Write -- Writes data to the file (non mixfile files only).                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "ccfile.h"

#include "_mixfile.h"
#include "mixfile.h"

#include <cassert>
#include <new>

/***********************************************************************************************
 * CCFileClass::CCFileClass -- Filename based constructor for C&C file.                        *
 *                                                                                             *
 *    Use this constructor for a file when the filename is known at construction time.         *
 *                                                                                             *
 * INPUT:   filename -- Pointer to the filename to use for this file object.                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The filename pointer is presumed to be inviolate throughout the duration of     *
 *             the file object. If this is not guaranteed, then use the default constructor    *
 *             and then set the name manually.                                                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
CCFileClass::CCFileClass(char const * filename) :
	Position(0)
{
	CCFileClass::Set_Name(filename);
}


/***********************************************************************************************
 * CCFileClass::CCFileClass -- Default constructor for file object.                            *
 *                                                                                             *
 *    This is the default constructor for a C&C file object.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
CCFileClass::CCFileClass(void) :
	Position(0)
{
}


/// <summary>
/// Handles a file error reported by the file system.
/// This was the hook that asked the player for the disc a missing file lived on. Every file
/// is local now, so the caller's own failure path deals with it.
/// </summary>
/// <param name="error">The error number, using the DOSERR.H numbering.</param>
/// <param name="canretry">Is the caller able to retry, so that this routine may return?</param>
/// <param name="filename">The file the error refers to, or NULL if it is not known.</param>
void CCFileClass::Error(int , int , char const * )
{
}


/***********************************************************************************************
 * CCFileClass::Write -- Writes data to the file (non mixfile files only).                     *
 *                                                                                             *
 *    This routine will write data to the file, but NOT to a file that is part of a mixfile.   *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the buffer that holds the data to be written.               *
 *                                                                                             *
 *          size     -- The number of bytes to write.                                          *
 *                                                                                             *
 * OUTPUT:  Returns the number of bytes actually written.                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int CCFileClass::Write(void const * buffer, int size)
{
	/*
	**	If this is part of a mixfile, then writing is not allowed. Error out with a fatal
	**	message.
	*/
	if (Is_Resident()) {
		Error(EACCES, false, File_Name());
	}

	return(BASECLASS::Write(buffer, size));
}


/***********************************************************************************************
 * CCFileClass::Read -- Reads data from the file.                                              *
 *                                                                                             *
 *    This routine determines if the file is part of the mixfile system. If it is, then        *
 *    the file is copied from RAM if it is located there. Otherwise it is read from disk       *
 *    according to the correct position of the file within the parent mixfile.                 *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the buffer to place the read data.                          *
 *                                                                                             *
 *          size     -- The number of bytes to read.                                           *
 *                                                                                             *
 * OUTPUT:  Returns the actual number of bytes read (this could be less than requested).       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int CCFileClass::Read(void * buffer, int size)
{
	bool opened = false;

	/*
	**	If the file isn't currently open, then open it.
	*/
	if (!Is_Open()) {
		if (Open()) {
			opened = true;
		}
	}

	/*
	**	If the file is part of a loaded mixfile, then a mere copy is
	**	all that is required for the read.
	*/
	if (Is_Resident()) {
		int	maximum = Data.Get_Size() - Position;

		size = maximum < size ? maximum : size;
//		size = MIN(maximum, size);
		if (size) {
			memmove(buffer, (char *)Data + Position, size);
//			Mem_Copy((char *)Pointer + Position, buffer, size);
			Position += size;
		}
		if (opened) Close();
		return(size);
	}

	int s = BASECLASS::Read(buffer, size);

	/*
	**	If the file was opened by this routine, then close it at this time.
	*/
	if (opened) Close();

	/*
	**	Return with the number of bytes read.
	*/
	return(s);
}


/***********************************************************************************************
 * CCFileClass::Seek -- Moves the current file pointer in the file.                            *
 *                                                                                             *
 *    This routine will change the current file pointer to the position specified. It follows  *
 *    the same rules the a normal Seek() does, but if the file is part of the mixfile system,  *
 *    then only the position value needs to be updated.                                        *
 *                                                                                             *
 * INPUT:   pos      -- The position to move the file to relative to the position indicated    *
 *                      by the "dir" parameter.                                                *
 *                                                                                             *
 *          dir      -- The direction to affect the position change against. This can be       *
 *                      either SEEK_CUR, SEEK_END, or SEEK_SET.                                *
 *                                                                                             *
 * OUTPUT:  Returns with the position of the new location.                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int CCFileClass::Seek(int pos, int dir)
{
	/*
	**	When the file is resident, a mere adjustment of the virtual file position is
	**	all that is required of a seek.
	*/
	if (Is_Resident()) {
		switch (dir) {
			case SEEK_END:
				Position = Data.Get_Size();
				break;

			case SEEK_SET:
				Position = 0;
				break;

			case SEEK_CUR:
			default:
				break;
		}
		Position += pos;
		Position = Position < 0 ? 0 : Position;
		Position = Position > Data.Get_Size() ? Data.Get_Size() : Position;
//		Position = Bound(Position+pos, 0L, Length);
		return(Position);
	}
	return(BASECLASS::Seek(pos, dir));
}


/***********************************************************************************************
 * CCFileClass::Size -- Determines the size of the file.                                       *
 *                                                                                             *
 *    If the file is part of the mixfile system, then the size of the file is already          *
 *    determined and available. Otherwise, go to the low level system to find the file         *
 *    size.                                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the size of the file in bytes.                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1994 JLB : Created.                                                                 *
 *   08/05/1996 JLB : Handles returning size of embedded file.                                 *
 *=============================================================================================*/
int CCFileClass::Size(void)
{
	/*
	**	If the file is resident, the the size is already known. Just return the size in this
	**	case.
	*/
	if (Is_Resident()) return(Data.Get_Size());

	/*
	**	If the file is not available as a stand alone file, then search for it in the
	**	mixfiles in order to get its size.
	*/
	if (!BASECLASS::Is_Available()) {
		int length = 0;
		MFCD::Offset(File_Name(), NULL, NULL, NULL, &length);
		return(length);
	}

	return(BASECLASS::Size());
}


/***********************************************************************************************
 * CCFileClass::Is_Available -- Checks for existence of file on disk or in mixfile.            *
 *                                                                                             *
 *    This routine will examine the mixfile system looking for the file. If the file could     *
 *    not be found there, then the disk is examined directly.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the file available for opening?                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCFileClass::Is_Available(int )
{
	/*
	**	A file that is open is presumed available.
	*/
	if (Is_Open()) return(true);

	/*
	**	A file that is part of a mixfile is also presumed available.
	*/
	if (MFCD::Offset(File_Name())) {
		return(true);
	}

	/*
	**	Otherwise a manual check of the file system is required to
	**	determine if the file is actually available.
	*/
	return(BASECLASS::Is_Available());
}


/***********************************************************************************************
 * CCFileClass::Is_Open -- Determines if the file is open.                                     *
 *                                                                                             *
 *    A mixfile is open if there is a pointer to the mixfile data. In absence of this,         *
 *    the the file is open if the file handle is valid.                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the file open?                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCFileClass::Is_Open(void) const
{
	/*
	**	If the file is part of a cached file, then return that it is opened. A closed file
	**	doesn't have a valid pointer.
	*/
	if (Is_Resident()) return(true);

	/*
	**	Otherwise, go to a lower level to determine if the file is open.
	*/
	return(BASECLASS::Is_Open());
}


/***********************************************************************************************
 * CCFileClass::Close -- Closes the file.                                                      *
 *                                                                                             *
 *    If this is a mixfile file, then only the pointers need to be adjusted.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void CCFileClass::Close(void)
{
	new(&Data) ::Buffer;
	Position = 0;				// Starts at beginning offset.
	BASECLASS::Close();
}


/// <summary>
/// Assigns a name to this file object and opens it in one operation.
/// </summary>
/// <param name="filename">The name of the file to open.</param>
/// <param name="rights">The access rights to open the file with.</param>
/// <returns>int; Was the file opened successfully?</returns>
int CCFileClass::Open(char const * filename, int rights)
{
	// A file being written is looked for in the current directory alone, and never in a
	// mixfile, since nothing can be written into one.
	if ((rights & WRITE) != 0) {
		return(CDFileClass::Open(filename, rights));
	}

	Set_Name(filename);
	return(Open(rights));
}


/***********************************************************************************************
 * CCFileClass::Open -- Opens a file from either the mixfile system or the rawfile system.     *
 *                                                                                             *
 *    This routine will open the specified file. It examines the mixfile system to find a      *
 *    match. If one is found then the file is "opened" in a special cached way. Otherwise      *
 *    it is opened as a standard DOS file.                                                     *
 *                                                                                             *
 * INPUT:   rights   -- The access rights desired.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the file opened successfully?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int CCFileClass::Open(int rights)
{
	/*
	**	Always close the file if it was open.
	*/
	Close();

	/*
	**	Perform a preliminary check to see if the specified file
	**	exists on the disk. If it does, then open this file regardless
	**	of whether it also exists in RAM. This is slower, but allows
	**	upgrade files to work.
	*/
	if ((rights & WRITE) || BASECLASS::Is_Available()) {
		return(BASECLASS::Open(rights));
	}

	/*
	**	Check to see if file is part of a mixfile and that mixfile is currently loaded
	**	into RAM.
	*/
	MFCD * mixfile = NULL;
	void * pointer = NULL;
	int length = 0;
	int start = 0;
	if (MFCD::Offset(File_Name(), &pointer, &mixfile, &start, &length)) {

		assert(mixfile != NULL);

		/*
		**	If the mixfile is located on disk, then fake out the file system to read from
		**	the mixfile, but think it is reading from a solitary file.
		*/
		if (pointer == NULL && mixfile != NULL) {

			/*
			**	This is a legitimate open to the file. All access to the file through this
			**	file object will be appropriately adjusted for mixfile support however. Also
			**	note that the filename attached to this object is NOT the same as the file
			**	attached to the file handle.
			*/
			char * dupfile = strdup(File_Name());
			Open(mixfile->Filename, READ);
			Searching(false);				// Disable multi-drive search.
			Set_Name(dupfile);
			Searching(true);
			free(dupfile);
			Bias(0);
			Bias(start, length);
			Seek(0, SEEK_SET);
		} else {
			new (&Data) ::Buffer(pointer, length);
			Position = 0;
		}

	} else {

		/*
		**	The file cannot be found in any mixfile, so it must reside as
		**	an individual file on the disk. Or else it is just plain missing.
		*/
		return(BASECLASS::Open(rights));
	}
	return(true);
}


/***********************************************************************************************
 * CCFileClass::Get_Date_Time -- Gets the date and time the file was last modified.            *
 *                                                                                             *
 *    Use this routine to get the date and time of the file.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the file date and time as a long.                                     *
 *          Use the YEAR(long), MONTH(),....                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/14/1995 DRD : Created.                                                                 *
 *=============================================================================================*/
unsigned int CCFileClass::Get_Date_Time(void)
{
	unsigned int datetime;
	MFCD * mixfile = NULL;

	datetime = BASECLASS::Get_Date_Time();

	if ( !datetime ) {
		if (MFCD::Offset(File_Name(), NULL, &mixfile, NULL, NULL)) {
			//
			// check for nested MIX files
			//
			return( CCFileClass(mixfile->Filename).Get_Date_Time() );
		}
		// else return 0 indicating no file
	}

	return( datetime );
}


/***********************************************************************************************
 * CCFileClass::Set_Date_Time -- Sets the date and time the file was last modified.            *
 *                                                                                             *
 *    Use this routine to set the date and time of the file.                                   *
 *                                                                                             *
 * INPUT:   the file date and time as a long                                                   *
 *                                                                                             *
 * OUTPUT:  successful or not if the file date and time was changed.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/14/1995 DRD : Created.                                                                 *
 *=============================================================================================*/
bool CCFileClass::Set_Date_Time( unsigned int datetime )
{
	bool status;
	MFCD * mixfile = NULL;

	status = BASECLASS::Set_Date_Time( datetime );

	if ( !status ) {
		if (MFCD::Offset(File_Name(), NULL, &mixfile, NULL, NULL)) {
			//
			// check for nested MIX files
			//
			return( CCFileClass(mixfile->Filename).Set_Date_Time( datetime ) );
		}
		// else return 0 indicating no file
	}

	return( status );
}
