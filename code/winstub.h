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

#include "win.h"

class Surface;
class PaletteClass;

bool Create_Main_Window(int command_show, int width, int height);

void Focus_Loss(void);
void Focus_Restore(void);

void Load_Title_Screen(char const * name, Surface * surface, PaletteClass * palette);

unsigned int Build_Number(void);
