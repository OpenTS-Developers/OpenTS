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
 *                     $Archive:: /Commando/Code/Library/XPIPE.CPP                            $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/28/98 12:06p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   BufferPipe::Put -- Submit data to the buffered pipe segment.                              *
 *   FilePipe::Put -- Submit a block of data to the pipe.                                      *
 *   FilePipe::End -- End the file pipe handler.                                               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "xpipe.h"

#include <cstddef>
#include <cstring>


//---------------------------------------------------------------------------------------------------------
// BufferPipe
//---------------------------------------------------------------------------------------------------------


/***********************************************************************************************
 * BufferPipe::Put -- Submit data to the buffered pipe segment.                                *
 *                                                                                             *
 *    The buffered pipe is a pipe terminator. That is, the data never flows onto subsequent    *
 *    pipe chains. The data is stored into the buffer previously submitted to the pipe.        *
 *    If the buffer is full, no more data is output to the buffer.                             *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the data to submit.                                         *
 *                                                                                             *
 *          length   -- The number of bytes to be submitted.                                   *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes output to the destination buffer.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int BufferPipe::Put(void const * source, int slen)
{
	int total = 0;

	if (Is_Valid() && source != NULL && slen > 0) {
		int len = slen;
		if (BufferPtr.Get_Size() != 0) {
			int theoretical_max = BufferPtr.Get_Size() - Index;
			len = (slen < theoretical_max) ? slen : theoretical_max;
		}

		if (len > 0) {
			memmove(((char *)BufferPtr.Get_Buffer()) + Index, source, len);
		}

		Index += len;
//		Length -= len;
//		Buffer = ((char *)Buffer) + len;
		total += len;
	}
	return(total);
}


//---------------------------------------------------------------------------------------------------------
// StringPipe
//---------------------------------------------------------------------------------------------------------


int StringPipe::Put(void const * source, int slen)
{
	if (source != NULL && slen > 0) {
		Text.append((char const *)source, (size_t)slen);
	}
	return(slen);
}


//---------------------------------------------------------------------------------------------------------
// FilePipe
//---------------------------------------------------------------------------------------------------------

/// <summary>
/// Closes the attached file if this pipe was the one that opened it.
/// A file that was already open when it was submitted to the pipe is left untouched, since
/// whoever opened it stays responsible for closing it. A pipe chain should call End()
/// rather than rely on this routine, because destructor order between the chained pipes is
/// not easily controlled.
/// </summary>
FilePipe::~FilePipe(void)
{
	if (Valid_File() && HasOpened) {
		HasOpened = false;
		File->Close();
		File = NULL;
	}
}


/***********************************************************************************************
 * FilePipe::End -- End the file pipe handler.                                                 *
 *                                                                                             *
 *    This routine is called when there will be no more data sent through the pipe. It is      *
 *    responsible for cleaning up anything it needs to. This is not handled by the             *
 *    destructor, although it serves a similar purpose, because pipe are linked together and   *
 *    the destructor order is not easily controlled. If the destructors for a pipe chain were  *
 *    called out of order, the result might be less than pleasant.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes flushed out the final end of the pipe as a        *
 *          consequence of this routine.                                                       *
 *                                                                                             *
 * WARNINGS:   Don't send any more data through the pipe after this routine is called.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int FilePipe::End(void)
{
	int total = BASECLASS::End();
	if (Valid_File() && HasOpened) {
		HasOpened = false;
		File->Close();
	}
	return(total);
}


/***********************************************************************************************
 * FilePipe::Put -- Submit a block of data to the pipe.                                        *
 *                                                                                             *
 *    Takes the data block submitted and writes it to the file. If the file was not already    *
 *    open, this routine will open it for write.                                               *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the data to submit to the file.                             *
 *                                                                                             *
 *          length   -- The number of bytes to write to the file.                              *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes written to the file.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int FilePipe::Put(void const * source, int slen)
{
	if (Valid_File() && source != NULL && slen > 0) {
		if (!File->Is_Open()) {
			HasOpened = true;
			File->Open(FileClass::WRITE);
		}

		return(File->Write(source, slen));
	}
	return(0);
}
