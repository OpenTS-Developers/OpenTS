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
 *                     $Archive:: /Commando/Library/lzopipe.h                                 $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "pipe.h"


/*
**	Performs LZO compression/decompression on the data stream that is piped through this
**	class. The data is compressed in blocks so of small enough size to be compressed
**	quickly and large enough size to get decent compression rates.
*/
class LZOPipe : public Pipe
{
		typedef Pipe BASECLASS;

	public:
		enum CompControl {
			COMPRESS,
			DECOMPRESS
		};

		LZOPipe(CompControl, int blocksize=1024*8);
		virtual ~LZOPipe(void) override;

		virtual int Flush(void) override;
		virtual int Put(void const * source, int slen) override;

	private:
		/*
		**	This tells the pipe if it should be decompressing or compressing the data stream.
		*/
		CompControl Control;

		// Set once a block header claims more compressed data than the buffer accumulating it
		// can hold. Everything after such a header is dropped.
		bool IsBroken;

		/*
		**	The number of bytes accumulated into the staging buffer.
		*/
		int Counter;

		/*
		**	Pointer to the working buffer that compression/decompression will use.
		*/
		char * Buffer;
		char * Buffer2;

		// The compressor's scratch dictionary, allocated only when this pipe compresses.
		char * Dictionary;

		/*
		**	The working block size. Data will be compressed in chunks of this size.
		*/
		int BlockSize;

		// The headroom the working buffers carry beyond a block. It bounds both what the
		// decompressor may write and what a block header may claim.
		int SafetyMargin;

		/*
		**	Each block has a header of this format.
		*/
		struct {
			unsigned short CompCount;   // Size of data block (compressed).
			unsigned short UncompCount; // Bytes of uncompressed data it represents.
		} BlockHeader;

		LZOPipe(LZOPipe & rvalue);
		LZOPipe & operator = (LZOPipe const & pipe);
};
