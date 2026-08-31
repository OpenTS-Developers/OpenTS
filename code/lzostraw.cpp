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
 *                     $Archive:: /Commando/Library/LZOSTRAW.CPP                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LZOStraw::Get -- Fetch data through the LZO processor.                                    *
 *   LZOStraw::LZOStraw -- Constructor for LZO straw object.                                   *
 *   LZOStraw::~LZOStraw -- Destructor for the LZO straw.                                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "lzostraw.h"

#include "lzo.h"

#include <cassert>
#include <cstring>


/***********************************************************************************************
 * LZOStraw::LZOStraw -- Constructor for LZO straw object.                                     *
 *                                                                                             *
 *    This will initialize the LZO straw object. Whether the object is to compress or          *
 *    decompress and the block size to use is specified. The data is compressed in blocks      *
 *    that are sized to be quick to compress and yet still yield good compression ratios.      *
 *                                                                                             *
 * INPUT:   decrypt  -- Should the data be decompressed?                                       *
 *                                                                                             *
 *          blocksize-- The size of the blocks to process.                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   It takes two buffers of the blocksize specified if compression is to be         *
 *             performed.                                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
LZOStraw::LZOStraw(CompControl control, int blocksize) :
		Control(control),
		Counter(0),
		Buffer(NULL),
		Buffer2(NULL),
		BlockSize(blocksize)
{
	SafetyMargin = BlockSize;
	Buffer = new char[BlockSize+SafetyMargin];
	if (control == COMPRESS) {
		Buffer2 = new char[BlockSize+SafetyMargin];
	}
}


/***********************************************************************************************
 * LZOStraw::~LZOStraw -- Destructor for the LZO straw.                                        *
 *                                                                                             *
 *    The destructor will free up the allocated buffers that it allocated in the constructor.  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
LZOStraw::~LZOStraw(void)
{
	delete [] Buffer;
	Buffer = NULL;

	delete [] Buffer2;
	Buffer2 = NULL;
}


/***********************************************************************************************
 * LZOStraw::Get -- Fetch data through the LZO processor.                                      *
 *                                                                                             *
 *    This routine will fetch the data bytes specified. It does this by first accumulating     *
 *    a full block of data and then compressing or decompressing it as indicated. Subsequent   *
 *    requests for data will draw from this buffer of processed data until it is exhausted     *
 *    and another block must be fetched.                                                       *
 *                                                                                             *
 * INPUT:   destbuf  -- Pointer to the buffer to hold the data requested.                      *
 *                                                                                             *
 *          length   -- The number of data bytes requested.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with the actual number of bytes stored into the buffer. If this number     *
 *          is less than that requested, then this indicates that the data source has been     *
 *          exhausted.                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int LZOStraw::Get(void * destbuf, int slen)
{
	assert(Buffer != NULL);

	int total = 0;

	/*
	**	Verify parameters for legality.
	*/
	if (destbuf == NULL || slen < 1) {
		return(0);
	}

	while (slen > 0) {

		/*
		**	Copy as much data is requested and available into the desired
		**	destination buffer.
		*/
		if (Counter > 0) {
			int len = (slen < Counter) ? slen : Counter;
			if (Control == DECOMPRESS) {
				memmove(destbuf, &Buffer[BlockHeader.UncompCount-Counter], len);
			} else {
				memmove(destbuf, &Buffer2[(BlockHeader.CompCount+sizeof(BlockHeader))-Counter], len);
			}
			destbuf = ((char *)destbuf) + len;
			slen -= len;
			Counter -= len;
			total += len;
		}
		if (slen == 0) break;

		if (Control == DECOMPRESS) {
			int incount = BASECLASS::Get(&BlockHeader, sizeof(BlockHeader));
			if (incount != sizeof(BlockHeader)) break;

			char *staging_buffer = new char [BlockHeader.CompCount];
			incount = BASECLASS::Get(staging_buffer, BlockHeader.CompCount);
			if (incount != BlockHeader.CompCount) {
				delete [] staging_buffer;
 				break;
			}
			unsigned int length = sizeof(Buffer);
			lzo1x_decompress ((unsigned char*)staging_buffer, BlockHeader.CompCount, (unsigned char*)Buffer, &length, NULL);
			delete [] staging_buffer;
			Counter = BlockHeader.UncompCount;
		} else {
			BlockHeader.UncompCount = (unsigned short)BASECLASS::Get(Buffer, BlockSize);
			if (BlockHeader.UncompCount == 0) break;
			char *dictionary = new char [LZO1X_MEM_COMPRESS];
			unsigned int length = sizeof (Buffer2) - sizeof (BlockHeader);
			lzo1x_1_compress ((unsigned char*)Buffer, BlockHeader.UncompCount, (unsigned char*)(&Buffer2[sizeof(BlockHeader)]), &length, dictionary);
			BlockHeader.CompCount = (unsigned short)length;
			delete [] dictionary;
			memmove(Buffer2, &BlockHeader, sizeof(BlockHeader));
			Counter = BlockHeader.CompCount+sizeof(BlockHeader);
		}
	}

	return(total);
}
