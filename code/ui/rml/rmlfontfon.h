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
#include <string>
#include <vector>


constexpr int UI_RASTER_ATLAS_LIMIT = 4096;


struct UIRasterGlyph
{
	char32_t Code = 0;
	int Advance = 0;
	std::vector<std::uint8_t> Coverage;
};


struct UIRasterStrike
{
	std::string Face;
	int Points = 0;
	int Height = 0;
	int Ascent = 0;
	std::vector<UIRasterGlyph> Glyphs;

	UIRasterGlyph const * Find(char32_t code) const;
	int Advance(char32_t code) const;
};


struct UIRasterCell
{
	char32_t Code = 0;
	int Advance = 0;
	int X = 0;
	int Y = 0;
};


unsigned int UI_Raster_Charset_Page(unsigned int charset);

bool UI_Read_Raster_Strike(std::span<std::uint8_t const> data, UIRasterStrike & strike);

bool UI_Read_Raster_Font(std::span<std::uint8_t const> data, std::vector<UIRasterStrike> & strikes);

int UI_Merge_Raster_Strikes(std::vector<UIRasterStrike> & strikes, std::vector<UIRasterStrike> const & extra);

bool UI_Raster_Strike_Layout(UIRasterStrike const & strike, int limit,
	std::vector<UIRasterCell> & cells, int & width, int & height);
