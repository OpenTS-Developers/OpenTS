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

#include "speed.hh"


/****************************************************************************
**	Each type of terrain has certain characteristics. These are indicated
**	by the structure below. For every element of terrain there is a
**	corresponding GroundType structure.
*/
struct GroundType {
	float	Cost[SPEED_COUNT];			// Terrain effect cost (normal).
	bool				Build;			// Can build on this terrain?

	// Carries the terrain characteristics to or from a save game.
	template<typename S>
	void Serialize(S & stream)
	{
		stream.Serialize(Cost);
		stream.Serialize(Build);
	}
};
