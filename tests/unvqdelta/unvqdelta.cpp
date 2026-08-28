/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds the VQ delta and colour mode 4 block decoders in vqalib/unvq.cpp to the frame they
// decode on the supported Win32 target. The recorded hashes in unvqdeltagolden.h were taken
// from that target, so a build for another architecture that walks its destination pointer
// differently fails here rather than in a movie. Needs no game data.
//
// Run with --emit to print a replacement golden table.

#include <cstdio>
#include <cstring>

#include "vqalib/unvq.h"

// _vqa.h owns the declaration of the colour table the table decoders read through.
#include "_vqa.h"
#include "unvqtblc.h"

#include "unvqstream.h"
#include "unvqdeltagolden.h"

namespace {

int const CBMAX = 65536 + 64;
int const PTRMAX = 16384;
int const DSTMAX = 1048576;

/*
 * The frame is decoded into the front of this buffer and the rest is left holding the guard.
 * A decoder that runs past the frame is caught by the margin check rather than by corrupting
 * whatever the allocator happened to put next.
 */
int const MARGIN = 4096;

unsigned char const GUARD = 0xA5;

unsigned char Codebook[CBMAX];
unsigned char Pointers[PTRMAX];
unsigned char DestStore[DSTMAX];

unsigned int Seed = 0;

int Failures = 0;
int Checked = 0;


unsigned int Next_Random(void)
{
	Seed = Seed * 1103515245u + 12345u;
	return(Seed >> 8);
}


unsigned long long Hash(unsigned char const * data, int size)
{
	unsigned long long hash = 1469598103934665603ULL;

	for (int i = 0; i < size; i++) {
		hash ^= (unsigned long long)data[i];
		hash *= 1099511628211ULL;
	}

	return(hash);
}


/// <summary>
/// Decodes one case into the destination buffer and returns the number of frame bytes the
/// decoder was given, which is the span the hash covers.
/// </summary>
int Run_Case(UnVQDeltaCase const & test)
{
	unsigned long const blocksperrow = test.BlocksPerRow;
	unsigned long const numrows = test.NumRows;
	unsigned long const bufwidth = test.BufWidth;

	/*
	 * Every one of these decoders doubles the width it is handed, so a row of pixels is two
	 * bytes per pixel wide whatever the block height is.
	 */
	int const pitch = (int)(bufwidth * 2);
	int const rowsperblock = test.RowsPerBlock;
	int const span = pitch * (int)numrows * rowsperblock;

	std::memset(DestStore, GUARD, sizeof(DestStore));

	switch (test.Which) {
		case DELTA_C1_4X4:
			UnVQStream::Build_Tagged_4x4(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ2_C1_4x4(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;

		case KEY_C4_4X4:
			UnVQStream::Build_Keyframe_C4(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ1_C4_4x4(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;

		case DELTA_C4_4X4:
			UnVQStream::Build_Delta_C4(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ2_C4_4x4(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;

		case KEY_C4_4X2:
			UnVQStream::Build_Keyframe_C4(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ1_C4_4x2(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;

		case DELTA_C4_4X2:
			UnVQStream::Build_Delta_C4(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ2_C4_4x2(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;

		case TABLE_DELTA_4X4:
			UnVQStream::Build_Tagged_4x4(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ2_4x4_Table(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;

		case TABLE_DELTA_4X2:
			UnVQStream::Build_Tagged_4x4(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ2_4x2_Table(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;

		case TABLE_KEY_4X4:
			UnVQStream::Build_Keyframe_C4(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ1_4x4_Table(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;

		default:
			UnVQStream::Build_Table_Keyed_4x2(Pointers, (int)blocksperrow, (int)numrows);
			UnVQ1_4x2_Table(Codebook, Pointers, DestStore, blocksperrow, numrows, bufwidth);
			break;
	}

	return(span);
}


/// <summary>
/// Confirms the decoder left the bytes past the frame alone. A decoder whose destination walk
/// overshoots writes here before it writes anywhere interesting.
/// </summary>
bool Margin_Intact(int span)
{
	for (int i = span; i < span + MARGIN; i++) {
		if (DestStore[i] != GUARD) {
			return(false);
		}
	}

	return(true);
}


/*
 * The table decoders look a solid block's colour up rather than writing it straight out, so
 * the harness supplies its own table instead of the display dependent one the engine builds.
 */
unsigned short HicolorStore[65536];


void Fill_Inputs(void)
{
	Seed = 555;

	for (int i = 0; i < CBMAX; i++) {
		Codebook[i] = (unsigned char)(Next_Random() & 0xFF);
	}

	Seed = 777;

	for (int i = 0; i < (int)(sizeof(HicolorStore) / sizeof(HicolorStore[0])); i++) {
		HicolorStore[i] = (unsigned short)(Next_Random() & 0xFFFF);
	}

	HicolorTable = HicolorStore;
}

}	// namespace


int main(int argc, char ** argv)
{
	bool const emit = (argc > 1) && (std::strcmp(argv[1], "--emit") == 0);

	Fill_Inputs();

	if (emit) {
		std::printf("static UnVQDeltaCase const UnVQDeltaCases[] = {\n");
	}

	for (int i = 0; i < UnVQDeltaCaseCount; i++) {
		UnVQDeltaCase const & test = UnVQDeltaCases[i];

		int const span = Run_Case(test);
		unsigned long long const hash = Hash(DestStore, span);

		if (emit) {
			std::printf("\t{%d, %luul, %luul, %luul, %d, %lluULL},\n",
				test.Which, test.BlocksPerRow, test.NumRows, test.BufWidth,
				test.RowsPerBlock, hash);
			continue;
		}

		if (!Margin_Intact(span)) {
			std::printf("OVERRAN decoder %d blocks %lu rows %lu width %lu\n",
				test.Which, test.BlocksPerRow, test.NumRows, test.BufWidth);
			Failures++;
		} else if (hash != test.Hash) {
			std::printf("FAILED decoder %d blocks %lu rows %lu width %lu: expected %llu, got %llu\n",
				test.Which, test.BlocksPerRow, test.NumRows, test.BufWidth, test.Hash, hash);
			Failures++;
		}

		Checked++;
	}

	if (emit) {
		std::printf("};\n");
		return(0);
	}

	std::printf("%-52s %s\n", "VQ delta block decode matches the Win32 frame",
		Failures == 0 ? "ok" : "FAILED");
	std::printf("checked %d cases, %d mismatches\n", Checked, Failures);

	return(Failures == 0 ? 0 : 1);
}
