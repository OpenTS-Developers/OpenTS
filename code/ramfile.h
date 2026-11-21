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
 *                     $Archive:: /Commando/Code/wwlib/ramfile.h                              $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/31/01 2:02p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


#include "wwfile.h"

#include <cstdint>
#include <span>
#include <vector>


class RAMFileClass : public FileClass
{
		typedef FileClass BASECLASS;

	public:
		RAMFileClass(void);
		RAMFileClass(void * buffer, int len);
		virtual ~RAMFileClass(void) override;

		RAMFileClass(RAMFileClass const &) = delete;
		RAMFileClass & operator = (RAMFileClass const &) = delete;

		virtual char const * File_Name(void) const override {return("UNKNOWN");}
		virtual char const * Set_Name(char const * ) override {return(File_Name());}
		virtual int Create(void) override;
		virtual int Delete(void) override;
		virtual bool Is_Available(int forced=false) override;
		virtual bool Is_Open(void) const override;
		virtual int Open(char const * filename, int access=READ) override;
		virtual int Open(int access=READ) override;
		virtual int Read(void * buffer, int size) override;
		virtual int Seek(int pos, int dir=SEEK_CUR) override;
		virtual int Size(void) override;
		virtual int Write(void const * buffer, int size) override;
		virtual void Close(void) override;
		virtual unsigned int Get_Date_Time(void) override {return(0);}
		virtual bool Set_Date_Time(unsigned int ) override {return(true);}
		virtual void Error(int , int = false, char const * =NULL) override {}

		operator char const * () {return(File_Name());}

		// The file's bytes. A write that grows a growable file moves them.
		std::span<std::uint8_t const> Contents(void) const;

	private:

		void Grow(int length);

		/*
		**	Pointer to the buffer that the "file" will reside in.
		*/
		char * Buffer;

		/*
		**	The maximum size of the buffer. The file occupying the buffer
		**	may be smaller than this size.
		*/
		int MaxLength;

		/*
		**	The number of bytes in the sub-file occupying the buffer.
		*/
		int Length;

		/*
		**	The current file position offset within the buffer.
		*/
		int Offset;

		/*
		**	The file was opened with this access mode.
		*/
		int Access;

		/*
		**	Is the file currently open?
		*/
		bool IsOpen;

		/*
		**	Was the file buffer allocated during construction of this object?
		*/
		bool IsAllocated;

		// A growable file keeps its bytes here, and Buffer and MaxLength describe them.
		std::vector<char> Storage;
		bool IsGrowable;
};
