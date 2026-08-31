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


/**********************************************************************
**	The houses that can be played are listed here. Each has their own
**	personality and strengths.
*/
#define HOUSE_NAME_MAX	20


enum HousesType {
	HOUSE_NONE=-1,

	HOUSE_GOOD,					// Global Defense Initiative
	HOUSE_BAD,					// Brotherhood of Nod
	HOUSE_NEUTRAL,				// Civilians
	HOUSE_MUTANT,

	HOUSE_COUNT,
	HOUSE_FIRST=0
};
