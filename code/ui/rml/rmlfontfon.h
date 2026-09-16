/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Reading the raster face the Win32 dialogs drew their text with. A `.FON` is a 16-bit
// executable whose resources are `FNT` strikes, one per point size, and each strike holds a
// width and a one-bit picture for every character. Windows cuts the same face once per code
// page, so a strike names the page its bytes are in and the reader hands back code points,
// which lets the files be folded into one family. The work knows no toolkit and opens no
// file, so a harness runs it over bytes it builds itself.

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>


// No atlas laid out for a strike may pass this on either edge, magnification included.
constexpr int UI_RASTER_ATLAS_LIMIT = 4096;


// One character of a strike: the code point it shows, how far the pen moves after it, and a
// pixel per byte, row by row, either nothing or full coverage.
struct UIRasterGlyph
{
	char32_t Code = 0;
	int Advance = 0;
	std::vector<std::uint8_t> Coverage;
};


// One strike. Height is the cell the layer drew a line of text on, and Ascent how far the
// baseline sits below its top. Glyphs ascends by code and holds each code once; everything
// looks it up by binary search, and only the reader and the merge may put anything in it.
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


// Where a glyph sits on its strike's atlas.
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
