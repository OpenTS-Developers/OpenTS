/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <vector>


struct UIMapPreviewImage
{
	int Width = 0;
	int Height = 0;
	std::vector<std::uint8_t> Pixels;
	int Generation = 0;
};


bool UI_Map_Preview_Image(UIMapPreviewImage & image);

bool UI_Generated_Map_Preview_Image(UIMapPreviewImage & image);
