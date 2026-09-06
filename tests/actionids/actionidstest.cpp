/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include <cassert>
#include <cstdio>

#include "need.hh"
#include "taction.hh"

static_assert(TACTION_TALK_BUBBLE == 105, "the stock action set ends at 105");
static_assert(TACTION_GIVE_CREDITS == 106, "Give Credits is 106");
static_assert(TACTION_ENABLE_SHORT_GAME == 107, "Enable Short Game is 107");
static_assert(TACTION_DISABLE_SHORT_GAME == 108, "Disable Short Game is 108");
static_assert(TACTION_CREATE_BUILDING_AT == 109, "Create Building At is 109");
static_assert(TACTION_HOUSE_DESTROY_ALL == 110, "Destroy all of is 110");
static_assert(TACTION_MAKE_ELITE == 111, "Make Elite is 111");
static_assert(TACTION_ENABLE_ALLY_REVEAL == 112, "Enable Ally Reveal is 112");
static_assert(TACTION_DISABLE_ALLY_REVEAL == 113, "Disable Ally Reveal is 113");
static_assert(TACTION_CREATE_AUTOSAVE == 114, "Create Autosave is 114");
static_assert(TACTION_DELETE_OBJECT == 115, "Delete Attached Objects is 115");
static_assert(TACTION_ALL_ASSIGN_MISSION == 116, "All Assign Mission is 116");
static_assert(TACTION_MAKE_ALLY_ONE_WAY == 117, "Make Ally (One-Way) is 117");
static_assert(TACTION_MAKE_ENEMY_ONE_WAY == 118, "Make Enemy (One-Way) is 118");

static_assert(NEED_HOUSE_AND_CREDITS > NEED_TALK_BUBBLE && NEED_STRUCTURE_PLACEMENT > NEED_HOUSE_AND_CREDITS,
	"the payload types the new actions need follow the stock ones");


int main(void)
{
	std::printf("%-64s %s\n", "last stock and extended trigger action numbers are pinned", "ok");
	return 0;
}
