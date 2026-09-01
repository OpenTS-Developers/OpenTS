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
#include "always.h"

#include "vqalib/unvq.h"

#include "_vqa.h"

/// <summary>
/// Assembles one block's codebook index from the two pointer planes.
/// </summary>
/// <param name="pointers">Base of the pointer data.</param>
/// <param name="entries">Total block count, which is where the high plane starts.</param>
/// <param name="block">Which block to read.</param>
/// <returns>unsigned int; The codebook index.</returns>
inline unsigned int Block_Index(unsigned char const * pointers, unsigned long entries, unsigned long block)
{
	return(((unsigned int)pointers[entries + block] << 8) | (unsigned int)pointers[block]);
}


inline void Put32(unsigned char * dest, unsigned char const * source)
{
	dest[0] = source[0];
	dest[1] = source[1];
	dest[2] = source[2];
	dest[3] = source[3];
}


inline void Fill32(unsigned char * dest, unsigned int value)
{
	dest[0] = (unsigned char)(value & 0xFF);
	dest[1] = (unsigned char)((value >> 8) & 0xFF);
	dest[2] = (unsigned char)((value >> 16) & 0xFF);
	dest[3] = (unsigned char)((value >> 24) & 0xFF);
}


/*
 * Spreads a 16 bit pixel across a doubleword so a solid block is filled four bytes at a time,
 * the way the assembly did it.
 */
inline unsigned int Pair16(unsigned int pixel)
{
	return((pixel << 16) | pixel);
}


inline unsigned int Quad8(unsigned int colour)
{
	unsigned int const pair = (colour << 8) | colour;
	return((pair << 16) | pair);
}

/// <summary>
/// Draws a 16 bit frame from 4x4 codebook entries, taking a solid block's colour from
/// HicolorTable.
/// </summary>
/// <param name="codebook">Codebook the blocks are drawn from.</param>
/// <param name="pointers">Block pointer data, low plane then high plane.</param>
/// <param name="buffer">Destination pixel buffer.</param>
/// <param name="blocksperrow">Blocks across one row of the frame.</param>
/// <param name="numrows">Rows of blocks in the frame.</param>
/// <param name="bufwidth">Destination width in pixels.</param>
void __cdecl UnVQ1_C1_TABLE(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer,
	unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	unsigned long const pitch = bufwidth * 2;
	unsigned long const rowoffset = pitch * 4;
	unsigned long const entries = numrows * blocksperrow;

	unsigned char * rowstart = buffer;
	unsigned long block = 0;

	do {
		unsigned char * dest = rowstart;

		for (unsigned long i = 0; i < blocksperrow; i++) {
			unsigned int const index = Block_Index(pointers, entries, block);
			block++;

			if ((index & 0x8000) != 0) {
				unsigned int const pixels = Pair16(HicolorTable[index & 0x7FFF]);

				for (int row = 0; row < 4; row++) {
					Fill32(dest + row * pitch, pixels);
					Fill32(dest + row * pitch + 4, pixels);
				}
			} else {
				unsigned char const * word = codebook + index * 32;

				for (int row = 0; row < 4; row++) {
					Put32(dest + row * pitch, word + row * 8);
					Put32(dest + row * pitch + 4, word + row * 8 + 4);
				}
			}

			dest += 8;
		}

		rowstart += rowoffset;
	} while (block < entries);
}


/// <summary>
/// Draws a 16 bit frame from the first and third rows of 4x4 codebook entries, written two
/// screen rows apart. A solid block's colour comes from HicolorTable.
/// </summary>
/// <param name="codebook">Codebook the blocks are drawn from.</param>
/// <param name="pointers">Block pointer data, low plane then high plane.</param>
/// <param name="buffer">Destination pixel buffer.</param>
/// <param name="blocksperrow">Blocks across one row of the frame.</param>
/// <param name="numrows">Rows of blocks in the frame.</param>
/// <param name="bufwidth">Destination width in pixels.</param>
void __cdecl UnVQ1_C1_TABLE_ALT(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer,
	unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	unsigned long const pitch = bufwidth * 2;
	unsigned long const rowoffset = pitch * 4;
	unsigned long const entries = numrows * blocksperrow;

	unsigned char * rowstart = buffer;
	unsigned long block = 0;

	do {
		unsigned char * dest = rowstart;

		for (unsigned long i = 0; i < blocksperrow; i++) {
			unsigned int const index = Block_Index(pointers, entries, block);
			block++;

			if ((index & 0x8000) != 0) {
				unsigned int const pixels = Pair16(HicolorTable[index & 0x7FFF]);

				Fill32(dest, pixels);
				Fill32(dest + 4, pixels);
				Fill32(dest + pitch * 2, pixels);
				Fill32(dest + pitch * 2 + 4, pixels);
			} else {
				unsigned char const * word = codebook + index * 32;

				Put32(dest, word);
				Put32(dest + 4, word + 4);
				Put32(dest + pitch * 2, word + 16);
				Put32(dest + pitch * 2 + 4, word + 20);
			}

			dest += 8;
		}

		rowstart += rowoffset;
	} while (block < entries);
}


/// <summary>
/// Draws an 8 bit frame from 4x2 codebook entries.
/// </summary>
/// <param name="codebook">Codebook the blocks are drawn from.</param>
/// <param name="pointers">Block pointer data, low plane then high plane.</param>
/// <param name="buffer">Destination pixel buffer.</param>
/// <param name="blocksperrow">Blocks across one row of the frame.</param>
/// <param name="numrows">Rows of blocks in the frame.</param>
/// <param name="bufwidth">Destination width in pixels.</param>
void __cdecl UnVQ_4x2(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer,
	unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	unsigned long const rowoffset = bufwidth * 2;
	unsigned long const entries = numrows * blocksperrow;

	unsigned char * rowstart = buffer;
	unsigned long block = 0;

	do {
		unsigned char * dest = rowstart;

		for (unsigned long i = 0; i < blocksperrow; i++) {
			unsigned int const index = Block_Index(pointers, entries, block);
			block++;

			if ((index >> 8) == 0xFF) {
				unsigned int const pixels = Quad8(index & 0xFF);

				Fill32(dest, pixels);
				Fill32(dest + bufwidth, pixels);
			} else {
				unsigned char const * word = codebook + index * 8;

				Put32(dest, word);
				Put32(dest + bufwidth, word + 4);
			}

			dest += 4;
		}

		rowstart += rowoffset;
	} while (block < entries);
}


/// <summary>
/// Draws an 8 bit frame from 4x4 codebook entries.
/// </summary>
/// <param name="codebook">Codebook the blocks are drawn from.</param>
/// <param name="pointers">Block pointer data, low plane then high plane.</param>
/// <param name="buffer">Destination pixel buffer.</param>
/// <param name="blocksperrow">Blocks across one row of the frame.</param>
/// <param name="numrows">Rows of blocks in the frame.</param>
/// <param name="bufwidth">Destination width in pixels.</param>
void __cdecl UnVQ_4x4(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer,
	unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	unsigned long const rowoffset = bufwidth * 4;
	unsigned long const entries = numrows * blocksperrow;

	unsigned char * rowstart = buffer;
	unsigned long block = 0;

	do {
		unsigned char * dest = rowstart;

		for (unsigned long i = 0; i < blocksperrow; i++) {
			unsigned int const index = Block_Index(pointers, entries, block);
			block++;

			if ((index >> 8) == 0xFF) {
				unsigned int const pixels = Quad8(index & 0xFF);

				for (int row = 0; row < 4; row++) {
					Fill32(dest + row * bufwidth, pixels);
				}
			} else {
				unsigned char const * word = codebook + index * 16;

				for (int row = 0; row < 4; row++) {
					Put32(dest + row * bufwidth, word + row * 4);
				}
			}

			dest += 4;
		}

		rowstart += rowoffset;
	} while (block < entries);
}


/// <summary>
/// Draws an 8 bit frame at half size, reading 4x4 codebook entries but keeping every other
/// pixel of every other row.
/// </summary>
/// <param name="codebook">Codebook the blocks are drawn from.</param>
/// <param name="pointers">Block pointer data, low plane then high plane.</param>
/// <param name="buffer">Destination pixel buffer.</param>
/// <param name="blocksperrow">Blocks across one row of the frame.</param>
/// <param name="numrows">Rows of blocks in the frame.</param>
/// <param name="bufwidth">Destination width in pixels.</param>
void __cdecl UnVQ_4x4_HALF(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer,
	unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	unsigned long const rowoffset = bufwidth * 2;
	unsigned long const entries = numrows * blocksperrow;

	unsigned char * rowstart = buffer;
	unsigned long block = 0;

	do {
		unsigned char * dest = rowstart;

		for (unsigned long i = 0; i < blocksperrow; i++) {
			unsigned int const index = Block_Index(pointers, entries, block);
			block++;

			if ((index >> 8) == 0xFF) {
				unsigned char const colour = (unsigned char)(index & 0xFF);

				dest[0] = colour;
				dest[1] = colour;
				dest[bufwidth] = colour;
				dest[bufwidth + 1] = colour;
			} else {
				unsigned char const * word = codebook + index * 16;

				dest[0] = word[0];
				dest[1] = word[2];
				dest[bufwidth] = word[8];
				dest[bufwidth + 1] = word[10];
			}

			dest += 2;
		}

		rowstart += rowoffset;
	} while (block < entries);
}


/// <summary>
/// Draws a 16 bit frame from 4x4 codebook entries, taking a solid block's colour from the
/// pointer value itself rather than through HicolorTable.
/// </summary>
/// <param name="codebook">Codebook the blocks are drawn from.</param>
/// <param name="pointers">Block pointer data, low plane then high plane.</param>
/// <param name="buffer">Destination pixel buffer.</param>
/// <param name="blocksperrow">Blocks across one row of the frame.</param>
/// <param name="numrows">Rows of blocks in the frame.</param>
/// <param name="bufwidth">Destination width in pixels.</param>
void __cdecl UnVQ1_C1_4x4(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer,
	unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	unsigned long const pitch = bufwidth * 2;
	unsigned long const rowoffset = pitch * 4;
	unsigned long const entries = numrows * blocksperrow;

	unsigned char * rowstart = buffer;
	unsigned long block = 0;

	do {
		unsigned char * dest = rowstart;

		for (unsigned long i = 0; i < blocksperrow; i++) {
			unsigned int const index = Block_Index(pointers, entries, block);
			block++;

			if ((index & 0x8000) != 0) {
				unsigned int const pixels = Pair16(index & 0x7FFF);

				for (int row = 0; row < 4; row++) {
					Fill32(dest + row * pitch, pixels);
					Fill32(dest + row * pitch + 4, pixels);
				}
			} else {
				unsigned char const * word = codebook + index * 32;

				for (int row = 0; row < 4; row++) {
					Put32(dest + row * pitch, word + row * 8);
					Put32(dest + row * pitch + 4, word + row * 8 + 4);
				}
			}

			dest += 8;
		}

		rowstart += rowoffset;
	} while (block < entries);
}
