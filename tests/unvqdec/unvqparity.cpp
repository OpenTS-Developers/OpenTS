/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds the UnVQ1 block decoders in unvqdec.cpp to the output the assembly they replaced
// produced. The vectors in unvqgolden.h were recorded from that assembly before it was
// removed. Needs no game data.

#include <cstdio>
#include <cstring>

#include "vqalib/unvq.h"

#include "unvqgolden.h"

extern "C" unsigned short * HicolorTable = 0;

namespace {

int const CBMAX = 65536;
int const DSTMAX = 1048576;
int const PTRMAX = 8192;
unsigned char const GUARD = 0xA5;

unsigned char Codebook[CBMAX];
unsigned char Pointers[PTRMAX * 2];
unsigned char DestStore[DSTMAX];
unsigned short HicolorStore[32768];

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


/*
 * Must build byte for byte what the generator built, or the recorded output describes a
 * different frame than the one replayed here.
 */
void Build_Pointers(unsigned int seed, unsigned long entries, int maxindex, bool hicolour, int solid)
{
	Seed = seed;

	for (unsigned long i = 0; i < entries; i++) {
		unsigned int index = 0;

		if ((int)(Next_Random() % 10) < solid) {
			index = hicolour ? (0x8000 | (Next_Random() & 0x7FFF)) : (0xFF00 | (Next_Random() & 0xFF));
		} else {
			index = Next_Random() % (unsigned int)maxindex;

			if (hicolour) {
				index &= 0x7FFF;
			} else if ((index >> 8) == 0xFF) {
				index = 0;
			}
		}

		Pointers[i] = (unsigned char)(index & 0xFF);
		Pointers[entries + i] = (unsigned char)((index >> 8) & 0xFF);
	}
}


int Span(UnVQGoldenCase const & test, int bytesperpixel, int rowsperblock)
{
	return((int)(test.BufWidth * bytesperpixel * test.NumRows * rowsperblock) + 256);
}

}	// namespace


int main(void)
{
	HicolorTable = HicolorStore;

	Seed = 777;
	for (int i = 0; i < 32768; i++) {
		HicolorStore[i] = (unsigned short)(Next_Random() & 0xFFFF);
	}

	Seed = 555;
	for (int i = 0; i < CBMAX; i++) {
		Codebook[i] = (unsigned char)(Next_Random() & 0xFF);
	}

	for (int i = 0; i < UnVQGoldenCaseCount; i++) {
		UnVQGoldenCase const & test = UnVQGoldenCases[i];
		unsigned long const entries = test.BlocksPerRow * test.NumRows;

		std::memset(DestStore, GUARD, sizeof(DestStore));

		int span = 0;
		int const solid = test.Solid;

		switch (test.Which) {
			case 0:
				Build_Pointers(test.Seed, entries, 2048, true, solid);
				UnVQ1_C1_TABLE(Codebook, Pointers, DestStore, test.BlocksPerRow, test.NumRows, test.BufWidth);
				span = Span(test, 2, 4);
				break;

			case 1:
				Build_Pointers(test.Seed, entries, 2048, true, solid);
				UnVQ1_C1_TABLE_ALT(Codebook, Pointers, DestStore, test.BlocksPerRow, test.NumRows, test.BufWidth);
				span = Span(test, 2, 4);
				break;

			case 2:
				Build_Pointers(test.Seed, entries, 8192, false, solid);
				UnVQ_4x2(Codebook, Pointers, DestStore, test.BlocksPerRow, test.NumRows, test.BufWidth);
				span = Span(test, 1, 2);
				break;

			case 3:
				Build_Pointers(test.Seed, entries, 4096, false, solid);
				UnVQ_4x4(Codebook, Pointers, DestStore, test.BlocksPerRow, test.NumRows, test.BufWidth);
				span = Span(test, 1, 4);
				break;

			case 4:
				Build_Pointers(test.Seed, entries, 4096, false, solid);
				UnVQ_4x4_HALF(Codebook, Pointers, DestStore, test.BlocksPerRow, test.NumRows, test.BufWidth);
				span = Span(test, 1, 2);
				break;

			default:
				Build_Pointers(test.Seed, entries, 2048, true, solid);
				UnVQ1_C1_4x4(Codebook, Pointers, DestStore, test.BlocksPerRow, test.NumRows, test.BufWidth);
				span = Span(test, 2, 4);
				break;
		}

		unsigned long long const hash = Hash(DestStore, span);

		if (hash != test.Hash) {
			std::printf("FAILED decoder %d blocks %lu rows %lu width %lu: expected %llu, got %llu\n",
				test.Which, test.BlocksPerRow, test.NumRows, test.BufWidth, test.Hash, hash);
			Failures++;
		}

		Checked++;
	}

	std::printf("%-52s %s\n", "UnVQ block decode matches the recorded assembly", Failures == 0 ? "ok" : "FAILED");
	std::printf("checked %d cases, %d mismatches\n", Checked, Failures);

	return(Failures == 0 ? 0 : 1);
}
