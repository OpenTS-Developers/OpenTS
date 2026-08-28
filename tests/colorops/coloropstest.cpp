/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds the palette tinting and spot light brightening in colorops.cpp to the output the
// assembly they replaced produced. The vectors in colorgolden.h were recorded from that
// assembly before it was removed; the three hand-written paths through Adjust_Color agreed
// with one another when they were recorded, so one set of vectors covers all of them.
// Needs no game data.

#include <cstdio>
#include <cstring>

#include "colorgolden.h"

extern "C" {
void __cdecl Adjust_Color_565(void *pal, void *xlat, int r, int g, int b, int i, void *mask);
void __cdecl Adjust_Color_555(void *pal, void *xlat, int r, int g, int b, int i, void *mask);
void __cdecl Adjust_Color_556(void *pal, void *xlat, int r, int g, int b, int i, void *mask);
void __cdecl Adjust_Color_655(void *pal, void *xlat, int r, int g, int b, int i, void *mask);

void __cdecl Brighten_Color_565(unsigned char *mul, unsigned short *col, int mw, int cw, int w, int h);
void __cdecl Brighten_Color_555(unsigned char *mul, unsigned short *col, int mw, int cw, int w, int h);
void __cdecl Brighten_Color_556(unsigned char *mul, unsigned short *col, int mw, int cw, int w, int h);
void __cdecl Brighten_Color_655(unsigned char *mul, unsigned short *col, int mw, int cw, int w, int h);

void __cdecl MMX_Brighten_Color_565(unsigned char *mul, unsigned short *col, int mw, int cw, int w, int h, int *mmx);
void __cdecl MMX_Brighten_Color_555(unsigned char *mul, unsigned short *col, int mw, int cw, int w, int h, int *mmx);
void __cdecl MMX_Brighten_Color_556(unsigned char *mul, unsigned short *col, int mw, int cw, int w, int h, int *mmx);
void __cdecl MMX_Brighten_Color_655(unsigned char *mul, unsigned short *col, int mw, int cw, int w, int h, int *mmx);
}

namespace {

unsigned char Palette[256 * 3];
unsigned char Mask[256];
unsigned short Translator[256];

unsigned char MulBuffer[256 * 256];
unsigned short ColorBuffer[512 * 512];
int MmxBuffer[65536];

unsigned int Seed = 0;

int Failures = 0;
int Checked = 0;


unsigned int Next_Random(void)
{
	Seed = Seed * 1103515245u + 12345u;
	return(Seed >> 8);
}


unsigned long long Hash(void const * data, int size)
{
	unsigned char const * bytes = (unsigned char const *)data;
	unsigned long long hash = 1469598103934665603ULL;
	for (int i = 0; i < size; i++) {
		hash ^= (unsigned long long)bytes[i];
		hash *= 1099511628211ULL;
	}
	return(hash);
}


typedef void (__cdecl * AdjustFunc)(void *, void *, int, int, int, int, void *);
typedef void (__cdecl * BrightenFunc)(unsigned char *, unsigned short *, int, int, int, int);
typedef void (__cdecl * MmxBrightenFunc)(unsigned char *, unsigned short *, int, int, int, int, int *);

AdjustFunc const Adjusts[4] = {Adjust_Color_565, Adjust_Color_555, Adjust_Color_556, Adjust_Color_655};
BrightenFunc const Brightens[4] = {Brighten_Color_565, Brighten_Color_555, Brighten_Color_556, Brighten_Color_655};
MmxBrightenFunc const MmxBrightens[4] = {MMX_Brighten_Color_565, MMX_Brighten_Color_555, MMX_Brighten_Color_556, MMX_Brighten_Color_655};

char const * const ModeNames[4] = {"565", "555", "556", "655"};


// Must reproduce the generator's inputs exactly or every vector misses.
void Fill_Adjust_Inputs(unsigned int seed)
{
	Seed = seed;
	for (int i = 0; i < 256 * 3; i++) {
		Palette[i] = (unsigned char)(Next_Random() & 0xFF);
	}
	for (int i = 0; i < 256; i++) {
		Mask[i] = (unsigned char)((Next_Random() & 1) ? 1 : 0);
	}
}


struct Tint {
	int Red;
	int Green;
	int Blue;
	int Intensity;
};

Tint const Tints[] = {
	{0x10000, 0x10000, 0x10000, 0x10000},
	{0x08000, 0x0C000, 0x10000, 0x06000},
	{0x1FFFF, 0x18000, 0x04000, 0x1C000},
	{0x00000, 0x10000, 0x20000, 0x00000},
	{0x30000, 0x30000, 0x30000, 0x02000}
};

}	// namespace


int main(void)
{
	for (int i = 0; i < AdjustGoldenCaseCount; i++) {
		AdjustGoldenCase const & test = AdjustGoldenCases[i];

		Fill_Adjust_Inputs(test.Seed);
		std::memset(Translator, 0xA5, sizeof(Translator));

		Adjusts[test.Mode](Palette, Translator, Tints[test.Tint].Red, Tints[test.Tint].Green,
			Tints[test.Tint].Blue, Tints[test.Tint].Intensity, Mask);

		unsigned long long const hash = Hash(Translator, sizeof(Translator));

		if (hash != test.Hash) {
			std::printf("FAILED Adjust_Color_%s tint %d: expected %llu, got %llu\n",
				ModeNames[test.Mode], test.Tint, test.Hash, hash);
			Failures++;
		}

		Checked++;
	}

	for (int i = 0; i < BrightenGoldenCaseCount; i++) {
		BrightenGoldenCase const & test = BrightenGoldenCases[i];

		Seed = test.Seed;

		for (int j = 0; j < 256 * 256; j++) {
			MulBuffer[j] = (unsigned char)(Next_Random() & 0xFF);
		}
		for (int j = 0; j < 512 * 512; j++) {
			ColorBuffer[j] = (unsigned short)(Next_Random() & 0xFFFF);
		}
		for (int j = 0; j < 65536; j++) {
			MmxBuffer[j] = (int)(Next_Random() & 0x00FFFFFF);
		}

		if (test.Mmx != 0) {
			MmxBrightens[test.Mode](MulBuffer, ColorBuffer, 256, 512 * 2, test.Width, test.Height, MmxBuffer);
		} else {
			Brightens[test.Mode](MulBuffer, ColorBuffer, 256, 512 * 2, test.Width, test.Height);
		}

		unsigned long long const hash = Hash(ColorBuffer, 512 * 512 * 2);

		if (hash != test.Hash) {
			std::printf("FAILED %sBrighten_Color_%s %dx%d: expected %llu, got %llu\n",
				test.Mmx ? "MMX_" : "", ModeNames[test.Mode], test.Width, test.Height, test.Hash, hash);
			Failures++;
		}

		Checked++;
	}

	std::printf("%-52s %s\n", "Colour tinting and brightening match the assembly", Failures == 0 ? "ok" : "FAILED");
	std::printf("checked %d cases, %d mismatches\n", Checked, Failures);

	return(Failures == 0 ? 0 : 1);
}
