/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds Shape_Facing_Index to the facing a shape cut into 8, 16, 32 or 64 facings draws from,
// and to the first facing for any other count. Needs no game data.

#include <cstdio>

#include "face.h"

namespace {

int Failures = 0;

void Check(bool condition, char const * what)
{
	std::printf("%-70s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}

// A direction pointing exactly at one index of a grid of the given size.
DirType At(int index, int count)
{
	return(DirType(index * (65536 / count)));
}

// What the eight facing path drew before any other count was drawn per facing.
int Legacy_Eight(DirType dir)
{
	return((int)((dir.Round_To_8() + 1) & 7));
}

void Grid(int count, char const * label)
{
	bool ordered = true;
	bool wrapped = true;

	for (int index = 0; index < count; index++) {
		int drawn = Shape_Facing_Index(At(index, count), count);

		if (drawn != (index + count / 8) % count) {
			ordered = false;
		}
		if (drawn < 0 || drawn >= count) {
			wrapped = false;
		}
	}

	std::printf("  %s\n", label);
	Check(ordered, "    every facing lands on its own index, an eighth of a turn along");
	Check(wrapped, "    no facing leaves the range the artwork provides");

	Check(Shape_Facing_Index(At(count - count / 8, count), count) == 0,
		"    northwest draws index zero");
	Check(Shape_Facing_Index(At(0, count), count) == count / 8,
		"    north draws the index an eighth of a turn along");

	// A direction between two grid points belongs to the nearer of them.
	int step = 65536 / count;
	Check(Shape_Facing_Index(DirType(step / 2 - 16), count) == Shape_Facing_Index(At(0, count), count),
		"    a direction short of the halfway point keeps the index below it");
	Check(Shape_Facing_Index(DirType(step / 2 + 16), count) == Shape_Facing_Index(At(1, count), count),
		"    a direction past the halfway point takes the index above it");
}

}

int main(void)
{
	Grid(8, "cut into 8");
	Grid(16, "cut into 16");
	Grid(32, "cut into 32");
	Grid(64, "cut into 64");

	// The stock layout must come through untouched, whatever else the switch now admits.
	{
		bool same = true;
		for (int raw = 0; raw < 65536; raw += 7) {
			if (Shape_Facing_Index(DirType(raw), 8) != Legacy_Eight(DirType(raw))) {
				same = false;
			}
		}
		std::printf("  the eight facing layout\n");
		Check(same, "    draws exactly the facing it always drew");
	}

	// Anything else draws one facing, the way a vehicle declaring a single facing always has.
	{
		int const unsupported[] = {-8, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 12, 15, 24, 31, 33, 48, 63, 65, 128, 256};
		bool zero = true;

		for (int count : unsupported) {
			for (int raw = 0; raw < 65536; raw += 1021) {
				if (Shape_Facing_Index(DirType(raw), count) != 0) {
					zero = false;
				}
			}
		}
		std::printf("  a count no artwork is cut into\n");
		Check(zero, "    draws index zero whichever way the vehicle points");
	}

	// A direction from a save can carry anything in the upper half, and the rounding reads it.
	{
		int const supported[] = {8, 16, 32, 64};
		bool wrapped = true;

		for (int count : supported) {
			for (int high = 1; high < 8; high++) {
				DirType dir;
				dir.Raw = (high << 16) | 0x3FFF;
				int drawn = Shape_Facing_Index(dir, count);
				if (drawn < 0 || drawn >= count) {
					wrapped = false;
				}
			}
		}
		std::printf("  a direction carrying padding from a save\n");
		Check(wrapped, "    still lands inside the range the artwork provides");
	}

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
