/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2020, 2025 Electronic Arts Inc.
 * Copyright 2020-2024 Vanilla Conquer contributors
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code and from Vanilla
 * Conquer (https://github.com/TheAssemblyArmada/Vanilla-Conquer).
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
 *                     $Archive:: /Commando/Code/wwlib/rawfile.h                              $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/25/01 11:52a                                             $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RawFileClass::File_Name -- Returns with the filename associate with the file object.      *
 *   RawFileClass::RawFileClass -- Default constructor for a file object.                      *
 *   RawFileClass::~RawFileClass -- Default deconstructor for a file object.                   *
 *   RawFileClass::Is_Open -- Checks to see if the file is open or not.                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "wwfile.h"

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

#ifndef WWERROR
#define WWERROR	-1
#endif

/*
**	This is the definition of the raw file class. It is derived from the abstract base FileClass
**	and handles the interface to the low level DOS routines. This is the first class in the
**	chain of derived file classes that actually performs a useful function. With this class,
**	I/O is possible. More sophisticated features, such as packed files, CD-ROM support,
**	file caching, and XMS/EMS memory support, are handled by derived classes.
**
**	Of particular importance is the need to override the error routine if more sophisticated
**	error handling is required. This is more than likely if greater functionality is derived
**	from this base class.
*/
class RawFileClass : public FileClass
{
		typedef FileClass BASECLASS;

	public:

		/*
		**	This is a record of the access rights used to open the file. These rights are
		**	used if the file object is duplicated.
		*/
		int Rights;

		RawFileClass(char const *filename);
		RawFileClass(void);
		RawFileClass (RawFileClass const & f);
		RawFileClass & operator = (RawFileClass const & f);
		virtual ~RawFileClass(void) override;

		virtual char const * File_Name(void) const override;
		virtual char const * Set_Name(char const *filename) override;
		virtual int Create(void) override;
		virtual int Delete(void) override;
		virtual bool Is_Available(int forced=false) override;
		virtual bool Is_Open(void) const override;
		virtual int Open(char const *filename, int rights=READ) override;
		virtual int Open(int rights=READ) override;
		virtual int Read(void *buffer, int size) override;
		virtual int Seek(int pos, int dir=SEEK_CUR) override;
		virtual int Size(void) override;
		virtual int Write(void const *buffer, int size) override;
		virtual void Close(void) override;
		virtual unsigned int Get_Date_Time(void) override;
		virtual bool Set_Date_Time(unsigned int datetime);
		virtual void Error(int error, int canretry = false, char const * filename=NULL) override;
		void Bias(int start, int length=-1);
		FILE *Get_File_Handle(void) { return(Handle); };

		/*
		**	These bias values enable a sub-portion of a file to appear as if it
		**	were the whole file. This comes in very handy for multi-part files such as
		**	mixfiles.
		*/
		int BiasStart;
		int BiasLength;

	protected:

		/*
		**	This function returns the largest size a low level DOS read or write may
		**	perform. Larger file transfers are performed in chunks of this size or less.
		*/
		int Transfer_Block_Size(void) {return((int)((unsigned)UINT_MAX)-16L);};

		int Raw_Seek(int pos, int dir=SEEK_CUR);

	private:

		/*
		**	This is the file handle.  A nullptr indicates an empty condition.
		*/
		FILE *Handle;

		/*
		**	This points to a copy of the filename as a NULL terminated string.
		*/
		char *Filename;

		/*
		**	The type of the last file access operation.  Reset by fseek().
		*/
		int LastAccessType;

		//
		// file date and time are in the following formats:
		//
		//      date   bits 0-4   day (0-31)
		//             bits 5-8   month (1-12)
		//             bits 9-15  year (0-119 representing 1980-2099)
		//
		//      time   bits 0-4   second/2 (0-29)
		//             bits 5-10  minutes (0-59)
		//             bits 11-15 hours (0-23)
		//
		unsigned short Date;
		unsigned short Time;
};


/***********************************************************************************************
 * RawFileClass::File_Name -- Returns with the filename associate with the file object.        *
 *                                                                                             *
 *    Use this routine to determine what filename is associated with this file object. If no   *
 *    filename has yet been assigned, then this routing will return NULL.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the file name associated with this file object or NULL   *
 *          if one doesn't exist.                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
inline char const * RawFileClass::File_Name(void) const
{
	return(Filename);
}


/***********************************************************************************************
 * RawFileClass::RawFileClass -- Default constructor for a file object.                        *
 *                                                                                             *
 *    This constructs a null file object. A null file object has no file handle or filename    *
 *    associated with it. In order to use a file object created in this fashion it must be     *
 *    assigned a name and then opened.                                                         *
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
inline RawFileClass::RawFileClass(void) :
	Rights(READ),
	BiasStart(0),
	BiasLength(-1),
	Handle(nullptr),
	Filename(nullptr),
	Date(0),
	Time(0),
	LastAccessType(0)
{
}


/***********************************************************************************************
 * RawFileClass::Is_Open -- Checks to see if the file is open or not.                          *
 *                                                                                             *
 *    Use this routine to determine if the file is open. It returns true if it is.             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the file open?                                                            *
 *                                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
inline bool RawFileClass::Is_Open(void) const
{
	return(Handle != nullptr);
}
