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

#pragma once

#include "coord.h"

class AStarClass;

/*
 * The most cells a path may run to. A buffer handed to Find_Path holds one more, because a
 * finished path of length L is padded out to entry L.
 */
inline constexpr int PATH_LENGTH_MAX = 2000;


/**************************************************************************
**	Find_Path returns with a pointer to this structure.
*/
struct PathStruct {
	Cell			Start;				// Starting cell.
	int				Cost;				// Accumulated terrain cost.
	int				Length;				// Command string length.
	FacingType		*Command;			// Pointer to command string.
	unsigned int	*Overlap;			// Pointer to overlap list
	unsigned int	*Height;			/// Cell height per command; tells a bridge deck from the ground.
	Cell			LastOverlap;		// stores position of last overlap
	Cell			LastFixup;			// stores position of last overlap
};


extern AStarClass Search;
