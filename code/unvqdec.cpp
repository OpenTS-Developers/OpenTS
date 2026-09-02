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
#include <cstring>
#include <cstdint>

#include "_vqa.h"

/// <summary>
/// Assembles one block's codebook index from the two pointer planes.
/// </summary>
/// <param name="pointers">Base of the pointer data.</param>
/// <param name="entries">Total block count, which is where the high plane starts.</param>
/// <param name="block">Which block to read.</param>
/// <returns>uint32_t; The codebook index.</returns>
inline uint32_t Block_Index(uint8_t const * pointers, size_t entries, size_t block)
{
	return((static_cast<uint32_t>(pointers[entries + block]) << 8) | static_cast<uint32_t>(pointers[block]));
}


inline void Fill32(uint8_t * dest, uint32_t value)
{
	// memcpy folds to one store where MSVC won't optimize a byte-wise copy; assumes a little-endian host.
	std::memcpy(dest, reinterpret_cast<void*>(&value), 4);
}


/*
 * Spreads a 16 bit pixel across a doubleword so a solid block is filled four bytes at a time,
 * the way the assembly did it.
 */
inline uint32_t Pair16(uint16_t pixel)
{
	return((static_cast<uint32_t>(pixel) << 16) | pixel);
}


inline uint32_t Quad8(uint8_t colour)
{
	uint32_t const pair = (static_cast<uint32_t>(colour) << 8) | colour;
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
void __cdecl UnVQ1_C1_TABLE(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer,
	size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	size_t const pitch = bufwidth * 2;
	size_t const rowoffset = pitch * 4;
	size_t const entries = numrows * blocksperrow;

	uint8_t * rowstart = buffer;
	size_t block = 0;

	do {
		uint8_t * dest = rowstart;

		for (size_t i = 0; i < blocksperrow; i++) {
			uint32_t const index = Block_Index(pointers, entries, block);
			block++;

			if ((index & 0x8000) != 0) {
				uint32_t const pixels = Pair16(HicolorTable[index & 0x7FFF]);

				for (int row = 0; row < 4; row++) {
					Fill32(dest + row * pitch, pixels);
					Fill32(dest + row * pitch + 4, pixels);
				}
			} else {
				uint8_t const * word = codebook + index * 32;

				for (int row = 0; row < 4; row++) {
					std::memcpy(dest + row * pitch, word + row * 8, 8);
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
void __cdecl UnVQ1_C1_TABLE_ALT(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer,
	size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	size_t const pitch = bufwidth * 2;
	size_t const rowoffset = pitch * 4;
	size_t const entries = numrows * blocksperrow;

	uint8_t * rowstart = buffer;
	size_t block = 0;

	do {
		uint8_t * dest = rowstart;

		for (size_t i = 0; i < blocksperrow; i++) {
			uint32_t const index = Block_Index(pointers, entries, block);
			block++;

			if ((index & 0x8000) != 0) {
				uint32_t const pixels = Pair16(HicolorTable[index & 0x7FFF]);

				Fill32(dest, pixels);
				Fill32(dest + 4, pixels);
				Fill32(dest + pitch * 2, pixels);
				Fill32(dest + pitch * 2 + 4, pixels);
			} else {
				uint8_t const * word = codebook + index * 32;

				std::memcpy(dest, word, 8);
				std::memcpy(dest + pitch * 2, word + 16, 8);
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
void __cdecl UnVQ_4x2(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer,
	size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	size_t const rowoffset = bufwidth * 2;
	size_t const entries = numrows * blocksperrow;

	uint8_t * rowstart = buffer;
	size_t block = 0;

	do {
		uint8_t * dest = rowstart;

		for (size_t i = 0; i < blocksperrow; i++) {
			uint32_t const index = Block_Index(pointers, entries, block);
			block++;

			if ((index >> 8) == 0xFF) {
				uint32_t const pixels = Quad8(index & 0xFF);

				Fill32(dest, pixels);
				Fill32(dest + bufwidth, pixels);
			} else {
				uint8_t const * word = codebook + index * 8;

				std::memcpy(dest, word, 4);
				std::memcpy(dest + bufwidth, word + 4, 4);
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
void __cdecl UnVQ_4x4(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer,
	size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	size_t const rowoffset = bufwidth * 4;
	size_t const entries = numrows * blocksperrow;

	uint8_t * rowstart = buffer;
	size_t block = 0;

	do {
		uint8_t * dest = rowstart;

		for (size_t i = 0; i < blocksperrow; i++) {
			uint32_t const index = Block_Index(pointers, entries, block);
			block++;

			if ((index >> 8) == 0xFF) {
				uint32_t const pixels = Quad8(index & 0xFF);

				for (int row = 0; row < 4; row++) {
					Fill32(dest + row * bufwidth, pixels);
				}
			} else {
				uint8_t const * word = codebook + index * 16;

				for (int row = 0; row < 4; row++) {
					std::memcpy(dest + row * bufwidth, word + row * 4, 4);
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
void __cdecl UnVQ_4x4_HALF(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer,
	size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	size_t const rowoffset = bufwidth * 2;
	size_t const entries = numrows * blocksperrow;

	uint8_t * rowstart = buffer;
	size_t block = 0;

	do {
		uint8_t * dest = rowstart;

		for (size_t i = 0; i < blocksperrow; i++) {
			uint32_t const index = Block_Index(pointers, entries, block);
			block++;

			if ((index >> 8) == 0xFF) {
				uint8_t const colour = static_cast<uint8_t>(index & 0xFF);

				dest[0] = colour;
				dest[1] = colour;
				dest[bufwidth] = colour;
				dest[bufwidth + 1] = colour;
			} else {
				uint8_t const * word = codebook + index * 16;

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
void __cdecl UnVQ1_C1_4x4(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer,
	size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	if (blocksperrow == 0) {
		return;
	}

	size_t const pitch = bufwidth * 2;
	size_t const rowoffset = pitch * 4;
	size_t const entries = numrows * blocksperrow;

	uint8_t * rowstart = buffer;
	size_t block = 0;

	do {
		uint8_t * dest = rowstart;

		for (size_t i = 0; i < blocksperrow; i++) {
			uint32_t const index = Block_Index(pointers, entries, block);
			block++;

			if ((index & 0x8000) != 0) {
				uint32_t const pixels = Pair16(index & 0x7FFF);

				for (int row = 0; row < 4; row++) {
					Fill32(dest + row * pitch, pixels);
					Fill32(dest + row * pitch + 4, pixels);
				}
			} else {
				uint8_t const * word = codebook + index * 32;

				for (int row = 0; row < 4; row++) {
					std::memcpy(dest + row * pitch, word + row * 8, 8);
				}
			}

			dest += 8;
		}

		rowstart += rowoffset;
	} while (block < entries);
}
