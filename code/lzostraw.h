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
 *                     $Archive:: /Commando/Library/lzostraw.h                                $*
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


#include "straw.h"

/*
**	This class handles LZO compression/decompression to the data stream that is drawn through
**	this class. Note that for compression, two internal buffers are required. For decompression
**	only one buffer is required. This changes the memory footprint of this class depending on
**	the process desired.
*/
class LZOStraw : public Straw
{
		typedef Straw BASECLASS;

	public:
		enum CompControl {
			COMPRESS,
			DECOMPRESS
		};

		LZOStraw(CompControl control, int blocksize=1024*8);
		virtual ~LZOStraw(void) override;

		virtual int Get(void * source, int slen) override;

		// Get's byte count is the same whether the stream ended or a block was rejected.
		bool Is_Damaged(void) const { return(IsDamaged); }

	private:

		/*
		**	This tells the pipe if it should be decompressing or compressing the data stream.
		*/
		CompControl Control;

		bool IsDamaged;

		/*
		**	The number of bytes accumulated into the staging buffer.
		*/
		int Counter;

		/*
		**	Pointer to the working buffer that compression/decompression will use.
		*/
		char * Buffer;
		char * Buffer2;

		// The compressor's scratch dictionary, allocated only when this straw compresses.
		char * Dictionary;

		/*
		**	The working block size. Data will be compressed in chunks of this size.
		*/
		int BlockSize;

		// The headroom the working buffers carry beyond a block. It bounds what the
		// decompressor may write, so a block claiming to expand past it is rejected.
		int SafetyMargin;

		/*
		**	Each block has a header of this format.
		*/
		struct {
			unsigned short CompCount;   // Size of data block (compressed).
			unsigned short UncompCount; // Bytes of uncompressed data it represents.
		} BlockHeader;

		LZOStraw(LZOStraw & rvalue);
		LZOStraw & operator = (LZOStraw const & pipe);
};
