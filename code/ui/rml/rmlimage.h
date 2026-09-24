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
#include <span>
#include <vector>


struct UIImageIndexed
{
	int Width = 0;
	int Height = 0;
	std::vector<std::uint8_t> Pixels;
	std::uint8_t Palette[768] = {};
};


bool UI_Decode_PCX(std::span<std::uint8_t const> encoded, UIImageIndexed & image);

bool UI_Indexed_To_RGBA(UIImageIndexed const & image, std::vector<std::uint8_t> & rgba);

bool UI_Hicolor_To_RGBA(std::span<std::uint16_t const> pixels, int width, int height, int pitch, std::vector<std::uint8_t> & rgba);

bool UI_Surface_Fit(int width, int height, int boxwidth, int boxheight, int & x, int & y, int & fitwidth, int & fitheight);

bool UI_Scale_RGBA_Nearest(std::span<std::uint8_t const> pixels, int width, int height, int destwidth, int destheight, std::vector<std::uint8_t> & result);
