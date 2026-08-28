/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Builds block pointer streams for the VQ delta decoders. A stream has to hand each decoder
// exactly the blocks one frame holds, because the decoders find the end of a row by comparing
// the destination against an exact address rather than by counting. Emitting a row that is a
// block short or a block long walks a decoder off the frame, so every builder here closes each
// row on the block that completes it.

#pragma once

namespace UnVQStream {

int const MAX_WORDS = 8192;

/*
 * The opcode mix each row cycles through. Every builder starts at the opcode that carries the
 * end-of-block-row correction, so a frame one row tall still reaches it.
 */
struct Builder {
	unsigned char * Data;
	int Bytes;

	void Reset(unsigned char * store)
	{
		Data = store;
		Bytes = 0;
	}

	void Word(unsigned int value)
	{
		Data[Bytes++] = (unsigned char)(value & 0xFF);
		Data[Bytes++] = (unsigned char)((value >> 8) & 0xFF);
	}

	void Byte(unsigned int value)
	{
		Data[Bytes++] = (unsigned char)(value & 0xFF);
	}
};


/*
 * Splits one row of blocks into runs. A run is never longer than what is left in the row, so
 * the last run always lands on the row boundary.
 */
inline int Run_Length(int remaining, int step)
{
	int run = (step % 3) + 1;

	if (run > remaining) {
		run = remaining;
	}

	return(run);
}


/// <summary>
/// Builds a stream for the 4x4 delta decoders that read a tag and a count from one word,
/// covering UnVQ2_C1_4x4 and the table decoder that shares its encoding.
/// Exercises tags 0x2000 and 0x3000, which carry the end-of-row correction.
/// </summary>
inline int Build_Tagged_4x4(unsigned char * store, int blocksperrow, int numrows)
{
	Builder out;
	out.Reset(store);

	int step = 0;

	for (int row = 0; row < numrows; row++) {
		int remaining = blocksperrow;

		while (remaining > 0) {
			int const run = Run_Length(remaining, step);

			switch (step % 4) {
				case 0:
					out.Word(0x2000 | run);		// codebook run, end-of-row correction
					out.Word(step % 1024);
					break;

				case 1:
					out.Word(0x3000 | run);		// masked codebook run, end-of-row correction
					out.Word(step % 1024);
					break;

				case 2:
					out.Word(0x0000 | run);		// solid fill
					out.Word(0x1234 + step);
					break;

				default:
					out.Word(0x1000 | run);		// skip
					break;
			}

			remaining -= run;
			step++;
		}
	}

	return(out.Bytes);
}


/// <summary>
/// Builds a stream for the 4x4 and 4x2 colour mode 4 keyframe decoders, whose word carries a
/// three bit type and a codebook index and always advances one block.
/// Exercises types 0x0000 and 0x2000, which carry the end-of-row correction.
/// </summary>
inline int Build_Keyframe_C4(unsigned char * store, int blocksperrow, int numrows)
{
	Builder out;
	out.Reset(store);

	int step = 0;

	for (int row = 0; row < numrows; row++) {
		for (int block = 0; block < blocksperrow; block++) {
			unsigned int const index = (unsigned int)(step % 512);

			switch (step % 3) {
				case 0:
					out.Word(0x0000 | index);	// codebook block, end-of-row correction
					break;

				case 1:
					out.Word(0x2000 | index);	// masked codebook block, end-of-row correction
					break;

				default:
					out.Word(0x4000 | index);	// skip one block
					break;
			}

			step++;
		}
	}

	return(out.Bytes);
}


/// <summary>
/// Builds a stream for the colour mode 4 delta decoders, which read their run lengths as a
/// byte that follows the word.
/// Exercises types 0x6000, 0x8000, 0xA000 and 0xC000, all of which carry the end-of-row
/// correction, plus the 0x0000 skip that does not.
/// </summary>
inline int Build_Delta_C4(unsigned char * store, int blocksperrow, int numrows)
{
	Builder out;
	out.Reset(store);

	int step = 0;

	for (int row = 0; row < numrows; row++) {
		int remaining = blocksperrow;

		while (remaining > 0) {
			unsigned int const index = (unsigned int)(step % 256);

			switch (step % 5) {
				case 0:
					out.Word(0x6000 | index);	// one block, end-of-row correction
					remaining -= 1;
					break;

				case 1:
					out.Word(0x8000 | index);	// one masked block, end-of-row correction
					remaining -= 1;
					break;

				case 2: {
					int const run = Run_Length(remaining, step);
					out.Word(0xC000 | index);	// masked vertical run, end-of-row correction
					out.Byte((unsigned int)run);
					remaining -= run;
					break;
				}

				case 3: {
					int const run = Run_Length(remaining, step);
					out.Word(0xA000 | index);	// vertical run, end-of-row correction
					out.Byte((unsigned int)run);
					remaining -= run;
					break;
				}

				default: {
					int const run = Run_Length(remaining, step);
					out.Word(0x0000 | (unsigned int)run);	// skip, no correction
					remaining -= run;
					break;
				}
			}

			step++;
		}
	}

	return(out.Bytes);
}


/// <summary>
/// Builds a stream for the 4x2 table keyframe decoder, whose types sit above 0x6000 and take
/// their run length from a byte after the word. Exercises types 0xA000 and 0xC000, which carry
/// the end-of-row correction.
///
/// Types 0x6000 and 0x8000 are left out. Both step the destination down four block rows and
/// then correct by two, so the blocks they claim do not add up to the row they sit in and a
/// stream built from them walks off the frame on Win32 as readily as anywhere else. That is a
/// question about those two paths rather than about the pointer arithmetic under test here.
/// </summary>
inline int Build_Table_Keyed_4x2(unsigned char * store, int blocksperrow, int numrows)
{
	Builder out;
	out.Reset(store);

	int step = 0;

	for (int row = 0; row < numrows; row++) {
		int remaining = blocksperrow;

		while (remaining > 0) {
			unsigned int const index = (unsigned int)(step % 256);
			int const run = Run_Length(remaining, step);

			switch (step % 3) {
				case 0:
					out.Word(0xA000 | index);	// scatter run, end-of-row correction
					out.Byte((unsigned int)run);
					break;

				case 1:
					out.Word(0xC000 | index);	// masked run, end-of-row correction
					out.Byte((unsigned int)run);
					break;

				default:
					out.Word(0x0000 | (unsigned int)run);	// skip, no correction
					break;
			}

			remaining -= run;
			step++;
		}
	}

	return(out.Bytes);
}

}	// namespace UnVQStream
