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
 *                     $Archive:: /G/wwlib/lcw.cpp                                            $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LCW_Comp -- Performes LCW compression on a block of data.                                 *
 *   LCW_Uncomp -- Decompress an LCW encoded data block.                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"lcw.h"

#include <cstddef>
#include <cstdint>


/// <summary>
/// Decompresses an LCW encoded data block.
///
/// A leading zero byte selects the relative form, where the two offset-carrying copies reach
/// back from the current position instead of forward from the start of the destination. VQA
/// stores its palettes, codebooks and pointer data that way, because an absolute offset is a
/// word and cannot address past 64K.
/// </summary>
/// <param name="source">Compressed data.</param>
/// <param name="dest">Buffer to decompress into.</param>
/// <param name="length">Size of the destination buffer, or zero to decode without a bound. A
/// bounded call clamps every command against the room left, so a stream claiming more than the
/// caller allowed is truncated instead of running past the buffer.</param>
/// <returns>uint32_t; The number of destination bytes written.</returns>
uint32_t LCW_Uncomp(void const * source, void * dest, unsigned long length)
{
	/*
	** Uncompress data to the following codes in the format b = byte, w = word
	** n = byte code pulled from compressed data.
	**
	**   Command code, n        |Description
	** -----------------------------------------------------------------------
	** n=0xxxyyyy,yyyyyyyy      |short copy back y bytes and run x+3 from dest
	** n=10xxxxxx,n1,n2,...,nx+1|med length copy the next x+1 bytes from source
	** n=11xxxxxx,w1            |med copy from dest x+3 bytes from offset w1
	** n=11111111,w1,w2         |long copy from dest w1 bytes from offset w2
	** n=11111110,w1,b1         |long run of byte b1 for w1 bytes
	** n=10000000               |end of data reached
	*/

	unsigned char * source_ptr, * dest_ptr, * copy_ptr;
	unsigned char op_code, data;
	unsigned count;

	/* Copy the source and destination ptrs. */
	source_ptr = (unsigned char*) source;
	dest_ptr   = (unsigned char*) dest;

	bool const relative = (*source_ptr == 0);
	bool const bounded = (length != 0);
	unsigned char * const end = (unsigned char*) dest + length;

	if (relative) {
		source_ptr++;
	}

	for (;;) {

		long maxlen = 0;

		if (bounded) {
			maxlen = (long)(end - dest_ptr);

			if (maxlen <= 0) {
				return((uint32_t) (dest_ptr - (unsigned char*) dest));
			}
		}

		/* Read in the operation code. */
		op_code = *source_ptr++;

		if (!(op_code & 0x80)) {

			/* Do a short copy from destination. */
			count = (op_code >> 4) + 3;
			copy_ptr = dest_ptr - ((unsigned) *source_ptr++ + (((unsigned) op_code & 0x0f) << 8));

			if (bounded && (count > (unsigned) maxlen)) {
				count = (unsigned) maxlen;
			} 

			while (count--) {
				*dest_ptr++ = *copy_ptr++;
			} 

		} else {

			if (!(op_code & 0x40)) {

				if (op_code == 0x80) {

					/* Return # of destination bytes written. */
					return((unsigned long) (dest_ptr - (unsigned char*) dest));

				} else {

					/* Do a medium copy from source. */
					count = op_code & 0x3f;

					if (bounded && (count > (unsigned) maxlen)) {
						count = (unsigned) maxlen;
					}

					while (count--) {
						*dest_ptr++ = *source_ptr++;
					}
				}

			} else {

				if (op_code == 0xfe) {

					/* Do a long run. */
					count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
					data = *(source_ptr + 2);
					source_ptr += 3;

					if (bounded && (count > (unsigned) maxlen)) {
						count = (unsigned) maxlen;
					}


					while (count--) { 
						*dest_ptr++ = data;
					}

				} else {

					if (op_code == 0xff) {

						/* Do a long copy from destination. */
						count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						size_t const offset = *(source_ptr + 2) + ((unsigned) *(source_ptr + 3) << 8);
						copy_ptr = relative ? (dest_ptr - offset) : ((unsigned char*)dest + offset);
						source_ptr += 4;

						if (bounded && (count > (unsigned) maxlen)) count = (unsigned) maxlen;

						while (count--) { 
							*dest_ptr++ = *copy_ptr++;
						}

					} else {

						/* Do a medium copy from destination. */
						count = (op_code & 0x3f) + 3;
						size_t const offset = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						copy_ptr = relative ? (dest_ptr - offset) : ((unsigned char*)dest + offset);
						source_ptr += 2;

						if (bounded && (count > (unsigned) maxlen)) count = (unsigned) maxlen;

						while (count--) { 
							*dest_ptr++ = *copy_ptr++;
						}
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * LCW_Comp -- Performes LCW compression on a block of data.                                   *
 *                                                                                             *
 *    This routine will compress a block of data using the LCW compression method. LCW has     *
 *    the primary characteristic of very fast uncompression at the expense of very slow        *
 *    compression times.                                                                       *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the source data to compress.                                *
 *                                                                                             *
 *          dest     -- Pointer to the destination location to store the compressed data       *
 *                      to.                                                                    *
 *                                                                                             *
 *          datasize -- The size (in bytes) of the source data to compress.                    *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes of output data stored into the destination        *
 *          buffer.                                                                            *
 *                                                                                             *
 * WARNINGS:   Be sure that the destination buffer is big enough. The maximum size required    *
 *             for the destination buffer is (datasize + datasize/128).                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/

/*
 * Compressed blocks reach save games, so this emits the same bytes the assembly it replaced
 * emitted, with one deliberate exception. The assembly wrote the first source byte before
 * testing for the end of the data, so a datasize of 1 read a second byte past the source and
 * encoded both. Those blocks expanded to two bytes. The end is tested before the loop here,
 * so a one byte block now holds one byte. Every other input encodes as it did.
 *
 * The search for a run reads sixty four bytes ahead of the current position without checking
 * that they belong to the source at all, so it can read past the end of the buffer. Only the
 * comparison result is used, and the run length that follows is measured properly.
 */
int LCW_Comp(void const * source, void * dest, int datasize)
{
	uint8_t const * const start = static_cast<uint8_t const *>(source);
	uint8_t * const first = static_cast<uint8_t *>(dest);
	uint8_t const * const end_of_data = start + datasize;

	uint8_t const * si = start;
	uint8_t * di = first;

	/*
	 * The first command is always a run of literals, opened here and extended in place as
	 * more of them are emitted.
	 */
	bool inlen = true;
	uint8_t * lenoff = di;

	*di++ = 0x81;
	*di++ = *si++;

	while (si < end_of_data) {
		uint8_t * ndest = di;
		uint8_t const * search = start;
		uint8_t const * matchoff = start;
		ptrdiff_t count = 1;

		/*
		 * Find the longest run of earlier data that repeats at the current position. A
		 * single byte repeated far enough is worth a command of its own and is emitted
		 * straight away, without disturbing the search.
		 */
		while (true) {
			uint8_t const value = *si;

			if (value == si[64]) {
				ptrdiff_t const left = end_of_data - si;
				ptrdiff_t matched = 0;

				while (matched < left && si[matched] == value) {
					matched++;
				}

				/*
				 * A run that reaches the end of the source is counted one short,
				 * because the scan it replaces stepped past the last byte it read.
				 * Callers hold the bytes that count produces, so it stays.
				 */
				ptrdiff_t const runlength = (matched < left) ? matched : (left - 1);

				if (runlength >= 65) {
					inlen = false;
					si += runlength;
					di = ndest;

					*di++ = 0xFE;
					*di++ = static_cast<uint8_t>(runlength & 0xFF);
					*di++ = static_cast<uint8_t>((runlength >> 8) & 0xFF);
					*di++ = value;

					ndest = di;
					continue;
				}
			}

			ptrdiff_t const window = si - search;

			if (window <= 0) {
				break;
			}

			/*
			 * Look for somewhere earlier the current byte appears.
			 */
			uint8_t const * found = nullptr;

			for (ptrdiff_t i = 0; i < window; i++) {
				uint8_t const candidate = *search++;

				if (candidate == value) {
					found = search;
					break;
				}
			}

			if (found == nullptr) {
				break;
			}

			/*
			 * Reject the candidate cheaply before measuring it: if the byte that would
			 * end a run at least as long as the best so far does not agree, it cannot
			 * beat it.
			 */
			if (si[count - 1] != search[count - 2]) {
				continue;
			}

			ptrdiff_t const room = end_of_data - si;
			ptrdiff_t length = 0;

			while (length < room && si[length] == (search - 1)[length]) {
				length++;
			}

			if (length < count) {
				continue;
			}

			count = length;
			matchoff = search - 1;
		}

		di = ndest;

		if (count > 2) {
			size_t const back = static_cast<size_t>(si - matchoff);

			if (count <= 10 && back <= 0x0FFF) {

				/*
				 * Short run: three bits of length and twelve of distance, packed into
				 * two bytes.
				 */
				*di++ = static_cast<uint8_t>((static_cast<size_t>(count - 3) << 4) | ((back >> 8) & 0x0F));
				*di++ = static_cast<uint8_t>(back & 0xFF);
			} else {
				if (count <= 64) {
					*di++ = static_cast<uint8_t>(0xC0 | (count - 3));
				} else {
					*di++ = 0xFF;
					*di++ = static_cast<uint8_t>(count & 0xFF);
					*di++ = static_cast<uint8_t>((count >> 8) & 0xFF);
				}

				/*
				 * The longer forms carry the match's position from the start of the
				 * data rather than its distance back from here.
				 */
				size_t const offset = static_cast<size_t>(matchoff - start);

				*di++ = static_cast<uint8_t>(offset & 0xFF);
				*di++ = static_cast<uint8_t>((offset >> 8) & 0xFF);
			}

			si += count;
			inlen = false;
		} else {

			/*
			 * Nothing worth referencing, so the byte goes out as a literal. A length
			 * command counts up to 0x3F bytes before another has to be opened.
			 */
			if (!inlen || *lenoff == 0xBF) {
				lenoff = di;
				*di++ = 0x80;
			}

			(*lenoff)++;
			*di++ = *si++;
			inlen = true;
		}
	}

	*di++ = 0x80;

	return(static_cast<int>(di - first));
}
