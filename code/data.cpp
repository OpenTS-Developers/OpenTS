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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/data.cpp                                           $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/24/99 4:52p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Load_Alloc_Data -- Allocates a buffer and loads the file into it.                         *
 *   Load_Uncompress -- Loads and uncompresses data to a buffer.                               *
 *   Hires_Load -- Allocates memory for, and loads, a resolution dependant file.               *
 *   Fetch_String -- Fetches a string resource.                                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "data.h"

#include <new>

HINSTANCE LanguageResources;

/***********************************************************************************************
 * Load_Alloc_Data -- Allocates a buffer and loads the file into it.                           *
 *                                                                                             *
 *    This is the C++ replacement for the Load_Alloc_Data function. It will allocate the       *
 *    memory big enough to hold the file and then read the file into it.                       *
 *                                                                                             *
 * INPUT:   file  -- The file to read.                                                         *
 *                                                                                             *
 *          mem   -- The memory system to use for allocation.                                  *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the allocated and filled memory block.                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void * Load_Alloc_Data(FileClass & file)
{
	void * ptr = NULL;
	if (file.Is_Available()) {
		int size = file.Size();

		ptr = new char[size];
		if (ptr != NULL) {
			file.Read(ptr, size);
		}
	}
	return(ptr);
}


/***********************************************************************************************
 * Load_Uncompress -- Loads and uncompresses data to a buffer.                                 *
 *                                                                                             *
 *    This is the C++ counterpart to the Load_Uncompress function. It will load the file       *
 *    specified into the graphic buffer indicated and uncompress it.                           *
 *                                                                                             *
 * INPUT:   file     -- The file to load and uncompress.                                       *
 *                                                                                             *
 *          uncomp_buff -- The graphic buffer that initial loading will use.                   *
 *                                                                                             *
 *          dest_buff   -- The buffer that will hold the uncompressed data.                    *
 *                                                                                             *
 *          reserved_data  -- This is an optional pointer to a buffer that will hold any       *
 *                            reserved data the compressed file may contain. This is           *
 *                            typically a palette.                                             *
 *                                                                                             *
 * OUTPUT:  Returns with the size of the uncompressed data in the destination buffer.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int Load_Uncompress(FileClass & file, Buffer & uncomp_buff, Buffer & dest_buff, void * reserved_data)
{
	unsigned short	size;
	void	* sptr = uncomp_buff.Get_Buffer();
	void	* dptr = dest_buff.Get_Buffer();
	int	opened = false;
	CompHeaderType	header;

	/*
	**	The file must be opened in order to be read from. If the file
	**	isn't opened, then open it. Record this fact so that it can be
	**	restored to its closed state at the end.
	*/
	if (!file.Is_Open()) {
		if (!file.Open()) {
			return(0);
		}
		opened = true;
	}

	/*
	**	Read in the size of the file (supposedly).
	*/
	file.Read(&size, sizeof(size));

	/*
	**	Read in the header block. This block contains the compression type
	**	and skip data (among other things).
	*/
	file.Read(&header, sizeof(header));
	size -= (unsigned short)sizeof(header);

	/*
	**	If there are skip bytes then they must be processed. Either read
	**	them into the buffer provided or skip past them. No check is made
	**	to ensure that the reserved data buffer is big enough (watch out!).
	*/
	if (header.Skip) {
		size -= header.Skip;
		if (reserved_data) {
			file.Read(reserved_data, header.Skip);
		} else {
			file.Seek(header.Skip, SEEK_CUR);
		}
		header.Skip = 0;
	}

	/*
	**	Determine where is the proper place to load the data. If both buffers
	**	specified are identical, then the data should be loaded at the end of
	**	the buffer and decompressed at the beginning.
	*/
	if (uncomp_buff.Get_Buffer() == dest_buff.Get_Buffer()) {
		sptr = (char *)sptr + uncomp_buff.Get_Size()-(size+sizeof(header));
	}

	/*
	**	Read in the bulk of the data.
	*/
	memmove(sptr, &header, sizeof(header));
//	Mem_Copy(&header, sptr, sizeof(header));
	file.Read((char *)sptr + sizeof(header), size);

	/*
	**	Decompress the data.
	*/
	size = (unsigned short) Uncompress_Data(sptr, dptr);

	/*
	**	Close the file if necessary.
	*/
	if (opened) {
		file.Close();
	}
	return((int)size);
}


struct SRecord {
	int ID;						// ID number of the string resource.
	int TimeStamp;				// 'Time' that this string was last requested.
	char String[2048];			// Copy of string resource.

	SRecord(void) : ID(-1), TimeStamp(-1) {}
};


/***********************************************************************************************
 * Fetch_String -- Fetches a string resource.                                                  *
 *                                                                                             *
 *    Fetches a string resource and returns a pointer to its text.                             *
 *                                                                                             *
 * INPUT:   id -- The ID number of the string resource to fetch.                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the actual text of the string resource.                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/25/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * Fetch_String(int id)
{
	static SRecord _buffers[128];
	static int _time = 0;

	/*
	**	Determine if the string ID requested is valid. If not then return an empty string pointer.
	*/
	if (id == -1 || id == TXT_NONE) return("");

	/*
	**	Adjust the 'time stamp' tracking value. This is an artificial value used merely to track
	**	the relative age of the strings requested.
	*/
	_time = _time+1;

	/*
	**	Check to see if the requested string has already been fetched into a buffer. If so, then
	**	return a pointer to that string (update the time stamp as well).
	*/
	for (int index = 0; index < ARRAY_SIZE(_buffers); index++) {
		if (_buffers[index].ID == id) {
			_buffers[index].TimeStamp = _time;
			return(_buffers[index].String);
		}
	}

	/*
	**	Find a suitable buffer to hold the string to be fetched. The buffer should either be
	**	empty or have the oldest fetched string.
	*/
	int oldest = -1;
	int oldtime = -1;
	for (int text = 0; text < ARRAY_SIZE(_buffers); text++) {
		if (oldest == -1 || oldtime > _buffers[text].TimeStamp) {
			oldest = text;
			oldtime = _buffers[text].TimeStamp;
			if (oldtime == -1 || _buffers[text].ID == -1) break;
		}
	}

	/*
	**	A suitable buffer has been found so fetch the string resource and then return a pointer
	**	to the string.
	*/
	char * stringptr = _buffers[oldest].String;
	_buffers[oldest].ID = id;
	_buffers[oldest].TimeStamp = _time;

	if (LanguageResources == NULL) {
		Init_Language_Resources(false);
	}

	if (LoadString(LanguageResources, id, stringptr, sizeof(_buffers[oldest].String)) == 0) {
		return("");
	}
	stringptr[sizeof(_buffers[oldest].String)-1] = '\0';
	return(stringptr);
}


/// <summary>
/// Fetches a raw resource from the language library.
/// This routine locates the named resource and locks it down so that the caller may read
/// straight out of it. The data stays mapped for as long as the library is loaded, so
/// there is nothing to release afterwards.
/// </summary>
/// <param name="resname">Name or identifier of the resource to fetch.</param>
/// <param name="restype">Type of the resource to fetch.</param>
/// <returns>Returns with a pointer to the resource data. Otherwise, NULL is returned.</returns>
void const * Fetch_Resource(LPCSTR resname, LPCSTR restype)
{
	/// The superfluous MAKEINTRESOURCE cast is the game's, and the C4302 warning with it.
	HRSRC handle = FindResource(LanguageResources, MAKEINTRESOURCE(resname), restype);
	if (handle == NULL) {
		return(NULL);
	}

	HGLOBAL rhandle = LoadResource(LanguageResources, handle);
	if (rhandle == NULL) {
		return(NULL);
	}

	return(LockResource(rhandle));
}


/// <summary>
/// Loads a compressed picture into the buffer specified.
/// This routine is a convenience layer over Load_Uncompress for callers that deal in whole
/// screen images and would rather think in pages than in bytes.
/// </summary>
/// <param name="palette">Buffer to fill in with the picture's palette, or NULL to discard
/// it.</param>
/// <returns>Returns with the number of pages the loaded picture occupies.</returns>
int Load_Picture(FileClass & file, Buffer & scratchbuf, Buffer & destbuf, unsigned char * palette, PicturePlaneType )
{
	return(Load_Uncompress(file, scratchbuf, destbuf,  palette ) / 8000);
}


/***********************************************************************************************
 * Hires_Load -- Allocates memory for, and loads, a resolution dependant file.                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Name of file to load                                                              *
 *                                                                                             *
 * OUTPUT:   Ptr to loaded file                                                                *
 *                                                                                             *
 * WARNINGS: Caller is responsible for releasing the memory allocated                          *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    5/13/96 3:20PM ST : Created                                                              *
 *=============================================================================================*/
void * Hires_Load(FileClass & file)
{
	int 	length;
	void * return_ptr;

	if (file.Is_Available()) {

		length = file.Size();
		return_ptr = new char[length];
		file.Read(return_ptr, length);
		return(return_ptr);

	} else {
		return(NULL);
	}
}


/// <summary>
/// Loads the language resource library.
/// This routine brings in the library that every localized string and resource is fetched
/// from. It may be called as often as convenient -- the library is only loaded the first
/// time. If asked to, it will tell the player to reinstall when the library is missing.
/// </summary>
/// <param name="show_error">Should the player be told when the library cannot be loaded?</param>
/// <returns>bool; Are the language resources available?</returns>
bool Init_Language_Resources(bool show_error)
{
	if (LanguageResources == NULL) {

		LanguageResources = LoadLibrary("Language.dll");

		if (LanguageResources == NULL) {

			if (show_error == true) {
				MessageBox(NULL,
					"Unable to initialize Language.dll, please reinstall Tiberian Sun.\n"
					"Keine Initialisierung von Language.DLL m\xF6glich. Bitte installieren Sie Tiberian Sun erneut.\n"
					"Initialisation de Language.DLL impossible. Veuillez r\xE9installer Tiberian Sun.",
					"Tiberian Sun",
					MB_ICONERROR);

			}

			return(false);
		}
	}

	return(true);
}


/// <summary>
/// Fetches a description of the language pack in use.
/// This routine composes a readable line naming the localized data set and its version,
/// taken from the version resource of the language library. It is used when reporting the
/// build the player is running. The string is left empty if the library is absent or
/// carries no version information of its own.
/// </summary>
/// <param name="version_string">Buffer to fill in with the description.</param>
/// <remarks>Be sure that the destination buffer is big enough to hold the composed text.</remarks>
void Get_Language_Version(char *version_string)
{
	INT dwSize;
	LPVOID pFileInfo;
	UINT puInfoLen;
	LPCTSTR pcData;
	DWORD dwHandle;

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *pvInfo;

	char szQuery[128];
	char szFile[MAX_PATH];

	if (version_string != NULL) {
		version_string[0] = '\0';

		if (LanguageResources != NULL && GetModuleFileName(LanguageResources, szFile, sizeof(szFile)) > 0) {

			dwHandle = 1;
			dwSize = GetFileVersionInfoSize(szFile, &dwHandle);

			if (dwSize > 0) {
				pFileInfo = new char[dwSize];

				if (pFileInfo != NULL) {
					if (GetFileVersionInfo(szFile, dwHandle, dwSize, pFileInfo)) {

						VerQueryValue(pFileInfo, TEXT("\\VarFileInfo\\Translation"), (LPVOID *)&pvInfo, &puInfoLen);

						if (puInfoLen > 0) {

							sprintf(szQuery, TEXT("\\StringFileInfo\\%04X%04X\\InternalName"), pvInfo->wLanguage, pvInfo->wCodePage);
							VerQueryValue(pFileInfo, szQuery, (LPVOID *)&pcData, &puInfoLen);

							if (puInfoLen > 0) {

								sprintf(version_string, "Language: %s ", pcData);
								sprintf(szQuery, TEXT("\\StringFileInfo\\%04X%04X\\FileVersion"), pvInfo->wLanguage, pvInfo->wCodePage);

								VerQueryValue(pFileInfo, szQuery, (LPVOID *)&pcData, &puInfoLen);

								if (puInfoLen > 0) {
									strcat(version_string, pcData);
								}
							}
						}
					}

					delete [] pFileInfo;
				}
			}
		}
	}
}
