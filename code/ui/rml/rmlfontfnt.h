/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>


// The pixel values of a Westwood font, as the game's text printing colors them.
enum UIFntRole
{
	UI_FNT_CLEAR = 0,
	UI_FNT_TEXT = 1,
	UI_FNT_SHADOW = 2,
	UI_FNT_OUTLINE = 3,
	UI_FNT_FIRST_SHADE = 4,
	UI_FNT_ROLES = 16
};


struct UIFntGlyph
{
	int Width = 0;
	int FirstRow = 0;
	int Rows = 0;
	bool Defined = false;
	std::vector<std::uint8_t> Pixels;
};


struct UIFntFont
{
	int Height = 0;
	int MaxWidth = 0;
	bool Windows1252 = false;
	std::vector<UIFntGlyph> Glyphs;
};


struct UIFntCell
{
	int X = 0;
	int Y = 0;
};


// Straight colors packed as 0xRRGGBBAA, one for each pixel value.
using UIFntPalette = std::array<std::uint32_t, UI_FNT_ROLES>;


constexpr std::uint32_t UI_Fnt_Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha)
{
	return(((std::uint32_t)red << 24) | ((std::uint32_t)green << 16) | ((std::uint32_t)blue << 8) | alpha);
}


bool UI_Read_Fnt(std::span<std::uint8_t const> data, UIFntFont & font);

int UI_Fnt_Glyph(UIFntFont const & font, char32_t code);

int UI_Fnt_Spacing(int raw, int spacing, bool outlined);

void UI_Fnt_Default_Palette(std::uint32_t text, bool gradient, UIFntPalette & palette);

bool UI_Fnt_Layout(UIFntFont const & font, int limit, std::vector<UIFntCell> & cells, int & width, int & height);

bool UI_Fnt_Atlas(UIFntFont const & font, std::vector<UIFntCell> const & cells, int width, int height, UIFntPalette const & palette, std::vector<std::uint8_t> & rgba);
