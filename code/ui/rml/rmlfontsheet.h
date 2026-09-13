/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Reading the dialog layer's bitmap font. It is a pair of PCX sheets of 256 cells in
// Windows-1252 order: one names a color per pixel and one carries the coverage, and the
// glyphs are shaded rather than flat, so asking for text in a color moves the whole palette
// toward it instead of tinting a white sheet. The work knows no toolkit, so a harness runs
// it over sheets it builds itself.

#pragma once

#include "ui/rml/rmlimage.h"

#include <cstdint>
#include <vector>


// How a font sits on its sheets: the blank margins around a cell, the inked area inside it,
// how many cells a row holds, and how far the pen moves after each character.
struct UISheetFontMetrics
{
	int TopMargin = 0;
	int LeftMargin = 0;
	int GlyphWidth = 0;
	int GlyphHeight = 0;
	int CellsPerRow = 0;
	int Advance[256] = {};

	int Cell_Width(void) const { return(GlyphWidth + LeftMargin); }
	int Cell_Height(void) const { return(GlyphHeight + TopMargin); }
};


// The coverage a pixel of the alpha sheet carries. The sheet is read through the red gun of
// its own palette, as the dialog layer reads it.
std::uint8_t UI_Sheet_Font_Coverage(UIImageIndexed const & alpha, int x, int y);

// Measures a font from its alpha sheet. False when the sheet holds no inked cell to measure.
bool UI_Sheet_Font_Metrics(UIImageIndexed const & alpha, UISheetFontMetrics & metrics);

// Where a character's cell sits on the sheets, and false for a character with nothing to
// draw: a space, anything below it, and anything past the sheet. The cell for a character
// is the one after it, because the sheets start with a cell the font does not draw.
bool UI_Sheet_Font_Cell(UISheetFontMetrics const & metrics, UIImageIndexed const & alpha, int character, int & x, int & y);

// The 256 colors the font's own palette becomes when its text is asked for in one color,
// as RGB triples. The shading survives because only the hue is carried over, with the
// saturation and value of each entry scaled by the color asked for.
void UI_Sheet_Font_Remap(std::uint8_t const * palette, std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t * remapped);

// One premultiplied RGBA sheet, colored through a remapped palette: the index sheet names
// the color of a pixel and the alpha sheet its coverage. False unless the two sheets are
// the same size.
bool UI_Sheet_Font_Atlas(UIImageIndexed const & index, UIImageIndexed const & alpha, std::uint8_t const * remapped, std::vector<std::uint8_t> & rgba);
