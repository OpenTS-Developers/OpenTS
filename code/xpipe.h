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
 *                     $Archive:: /G/wwlib/XPIPE.H                                            $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 12:01p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "buff.h"
#include "pipe.h"
#include "wwfile.h"

#include <string>

/*
**	This is a simple store-into-buffer pipe terminator. Use it as the final link in a pipe process
**	that needs to store the data into a memory buffer. This can only serve as the final
**	link in the chain of pipe segments.
*/
class BufferPipe : public Pipe
{
		typedef Pipe BASECLASS;

	public:
		BufferPipe(Buffer const & buffer) : BufferPtr(buffer), Index(0) {}
		BufferPipe(void * buffer, int length) : BufferPtr(buffer, length), Index(0) {}
		virtual int Put(void const * source, int slen) override;

	private:
		Buffer BufferPtr;
		int Index;

		bool Is_Valid(void) {return(BufferPtr.Is_Valid());}
		BufferPipe(BufferPipe & rvalue);
		BufferPipe & operator = (BufferPipe const & pipe);
};


// A pipe terminator that appends to the string it was given.
class StringPipe : public Pipe
{
		typedef Pipe BASECLASS;

	public:
		explicit StringPipe(std::string & text) : Text(text) {}
		virtual int Put(void const * source, int slen) override;

	private:
		std::string & Text;

		StringPipe(StringPipe & rvalue);
		StringPipe & operator = (StringPipe const & pipe);
};


/*
**	This is a store-to-file pipe terminator. Use it as the final link in a pipe process that
**	needs to store the data to a file. This can only serve as the last link in the chain
**	of pipe segments.
*/
class FilePipe : public Pipe
{
		typedef Pipe BASECLASS;

	public:
		FilePipe(FileClass * file) : File(file), HasOpened(false) {}
		FilePipe(FileClass & file) : File(&file), HasOpened(false) {}
		virtual ~FilePipe(void) override;

		virtual int Put(void const * source, int slen) override;
		virtual int End(void) override;

	private:
		FileClass * File;
		bool HasOpened;

		bool Valid_File(void) {return(File != NULL);}
		FilePipe(FilePipe & rvalue);
		FilePipe & operator = (FilePipe const & pipe);

};
