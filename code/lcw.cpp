/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 Vanilla-Conquer contributors
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

int LCW_Comp(const void* src, void* dst, unsigned int bytes)
{
    if (!bytes) {
        return 0;
    }

    const unsigned char* getp = (const unsigned char*)(src);
    unsigned char* putp = (unsigned char*)(dst);
    const unsigned char* getstart = getp;
    const unsigned char* getend = getp + bytes;
    unsigned char* putstart = putp;
    bool cmd_one;
    // Write a starting cmd1 and set bool to have cmd1 in progress
    unsigned char* cmd_onep = putp;
    *putp++ = 0x81;
    *putp++ = *getp++;
    cmd_one = true;

    // Compress data
    while (getp < getend) {
        // Is RLE encode (4bytes) worth evaluating?
        if (getend - getp > 64 && *getp == *(getp + 64)) {
            // RLE run length is encoded as a short so max is UINT16_MAX
            const unsigned char* rlemax = (getend - getp) < 0xFFFF ? getend : getp + 0xFFFF;
            const unsigned char* rlep;

            for (rlep = getp + 1; *rlep == *getp && rlep < rlemax; ++rlep)
                ;

            unsigned short run_length = rlep - getp;

            // If run length is long enough, write the command and start loop again
            if (run_length >= 0x41) {
                cmd_one = false;
                *putp++ = 0xFE;
                *putp++ = (unsigned char)run_length;
                *putp++ = run_length >> 8;
                *putp++ = *getp;
                getp = rlep;
                continue;
            }
        }

        // current block size for an offset copy
        int block_size = 0;
        const unsigned char* offstart;

        // Set where we start looking for matching runs.
        offstart = getstart;

        // Look for matching runs
        const unsigned char* offchk = offstart;
        const unsigned char* offsetp = getp;
        while (offchk < getp) {
            // Move offchk to next matching position
            while (offchk < getp && *offchk != *getp) {
                ++offchk;
            }

            // If the checking pointer has reached current pos, break
            if (offchk >= getp) {
                break;
            }

            // find out how long the run of matches goes for
            //<= because it can consider the current pixel as part of a run
            int i;
            for (i = 1; &getp[i] < getend; ++i) {
                if (offchk[i] != getp[i]) {
                    break;
                }
            }

            if (i >= block_size) {
                block_size = i;
                offsetp = offchk;
            }

            ++offchk;
        }

        // decide what encoding to use for current run
        if (block_size <= 2) {
            // short copy 0b10??????
            // check we have an existing 1 byte command and if its value is still
            // small enough to handle additional bytes
            // start a new command if current one doesn't have space or we don't
            // have one to continue
            if (cmd_one && *cmd_onep < 0xBF) {
                // increment command value
                ++*cmd_onep;
                *putp++ = *getp++;
            } else {
                cmd_onep = putp;
                *putp++ = 0x81;
                *putp++ = *getp++;
                cmd_one = true;
            }
        } else {
            unsigned short offset;
            unsigned short rel_offset = getp - offsetp;
            if (block_size > 0xA || (rel_offset > 0xFFF)) {
                // write 5 byte command 0b11111111
                if (block_size > 0x40) {
                    *putp++ = 0xFF;
                    *putp++ = block_size;
                    *putp++ = block_size >> 8;
                    // write 3 byte command 0b11??????
                } else {
                    *putp++ = (block_size - 3) | 0xC0;
                }

                offset = offsetp - getstart;
                // write 2 byte command? 0b0???????
            } else {
                offset = rel_offset << 8 | (16 * (block_size - 3) + (rel_offset >> 8));
            }
            *putp++ = (unsigned char)offset;
            *putp++ = offset >> 8;
            getp += block_size;
            cmd_one = false;
        }
    }

    // write final 0x80, this is why its also known as format80 compression
    *putp++ = 0x80;
    return putp - putstart;
}