/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/rml/rmlimage.h"

#include <vector>


enum UIImageResult
{
	UI_IMAGE_LOADED,
	UI_IMAGE_MISSING,
	UI_IMAGE_UNREADABLE
};


UIImageResult UI_Load_Image(char const * name, std::vector<unsigned char> & rgba, int & width, int & height);

bool UI_Read_File(char const * path, std::vector<unsigned char> & bytes);

bool UI_Load_Indexed_Image(char const * name, UIImageIndexed & image);
