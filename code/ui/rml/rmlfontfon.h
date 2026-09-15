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


/// <summary>
/// Returns the Windows code page a `FNT` charset byte names, and 1252 for one it does not,
/// which is the page every one of these files was read as before they named their own.
/// </summary>
unsigned int UI_Raster_Charset_Page(unsigned int charset);

/// <summary>
/// Reads one `FNT` strike, taking its characters through the code page its charset byte
/// names. A byte the page leaves undefined, and anything below a space, carries no glyph.
/// </summary>
/// <returns>False, leaving the strike untouched, for anything that is not a version two or
/// three strike, or whose character table or pictures fall outside the bytes given.</returns>
bool UI_Read_Raster_Strike(std::span<std::uint8_t const> data, UIRasterStrike & strike);

/// <summary>
/// Reads every strike a `.FON` carries, in the order its resource table names them. A file
/// holding a second strike of a height it already gave up is read once, so the height names
/// one strike.
/// </summary>
/// <returns>False, leaving nothing read, for bytes that are not a 16-bit image, and for one
/// carrying no strike this can read.</returns>
bool UI_Read_Raster_Font(std::span<std::uint8_t const> data, std::vector<UIRasterStrike> & strikes);

/// <summary>
/// Folds the strikes of another cut of the same face into strikes already read, matching them
/// by height. A code point the read strike already carries wins, so the file read first
/// decides what the family looks like whichever cuts a machine has. A strike whose height is
/// not already there is dropped rather than added, because a height carrying one page alone
/// would capture every document asking near it.
/// </summary>
/// <returns>How many characters the merge added.</returns>
int UI_Merge_Raster_Strikes(std::vector<UIRasterStrike> & strikes, std::vector<UIRasterStrike> const & extra);

/// <summary>
/// Lays a strike's glyphs into rows one strike-height tall, in the order the strike holds
/// them, and reports the atlas they need.
/// </summary>
/// <returns>False for a strike carrying nothing, and for one whose glyphs cannot be laid
/// inside limit either way.</returns>
bool UI_Raster_Strike_Layout(UIRasterStrike const & strike, int limit,
	std::vector<UIRasterCell> & cells, int & width, int & height);
