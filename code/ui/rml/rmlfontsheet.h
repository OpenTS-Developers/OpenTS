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

#include <cstdint>
#include <vector>


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


std::uint8_t UI_Sheet_Font_Coverage(UIImageIndexed const & alpha, int x, int y);

bool UI_Sheet_Font_Metrics(UIImageIndexed const & alpha, UISheetFontMetrics & metrics);

bool UI_Sheet_Font_Cell(UISheetFontMetrics const & metrics, UIImageIndexed const & alpha, int character, int & x, int & y);

void UI_Sheet_Font_Remap(std::uint8_t const * palette, std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t * remapped);

bool UI_Sheet_Font_Atlas(UIImageIndexed const & index, UIImageIndexed const & alpha, std::uint8_t const * remapped, std::vector<std::uint8_t> & rgba);
