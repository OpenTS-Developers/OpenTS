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

#include "_rules.h"

#include "ccini.h"

/***************************************************************************
**	This holds the rules database. The rules database won't change during the
**	program's run, but may need to be referenced intermitently.
*/
CCINIClass ArtINI;
CCINIClass AIINI;

CCINIClass EditorINI;

CCINIClass ConfigINI;

CCINIClass FSRuleINI;
CCINIClass FSAIINI;

CCINIClass MPRuleINI;
CCINIClass FSMPRuleINI;

CCINIClass *RuleINI;

/***************************************************************************
**	General rules that control the game.
*/
RulesClass *Rule;
