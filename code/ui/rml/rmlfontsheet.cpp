/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlfontsheet.h"

#include "hsv.h"
#include "rgb.h"

#include <cstring>


// The hue the sheets are drawn around, and the window either side of a requested hue that
// the pull toward it is measured over. Both come from the dialog layer's own remap.
static const float UI_SHEET_FONT_SOURCE_HUE = 68.0f;
static const int UI_SHEET_FONT_HUE_WINDOW = 15;


// How far a hue is pulled back toward the sheets' own. The three hues below sit where the
// pull is weakest, and it eases off over sixteen steps either side of each.
static float Remap_Factor(int hue)
{
	static const int centers[3] = { 43, 128, 213 };
	float factor = 1.0f;

	for (int index = 0; index < 3; index++) {
		int center = centers[index];
		int distance = 0;

		if (hue > center - 16 && hue <= center) {
			distance = center - hue;
		} else if (hue > center && hue <= center + 16) {
			distance = hue - center;
		} else {
			continue;
		}

		factor = (float)distance * (1.0f / 16) * 0.6f + 0.4f;
	}

	return(factor);
}


std::uint8_t UI_Sheet_Font_Coverage(UIImageIndexed const & alpha, int x, int y)
{
	if (x < 0 || y < 0 || x >= alpha.Width || y >= alpha.Height) {
		return(0);
	}

	std::uint8_t index = alpha.Pixels[(std::size_t)y * alpha.Width + x];
	return(alpha.Palette[(std::size_t)index * 3]);
}


bool UI_Sheet_Font_Metrics(UIImageIndexed const & alpha, UISheetFontMetrics & metrics)
{
	metrics = UISheetFontMetrics();

	if (alpha.Width <= 4 || alpha.Height <= 0
		|| alpha.Pixels.size() != (std::size_t)alpha.Width * (std::size_t)alpha.Height) {
		return(false);
	}

	// The first cell is blank, so the margins are probed a few pixels in, where the second
	// cell's glyph is, exactly as the dialog layer probes them.
	while (metrics.TopMargin < alpha.Height && UI_Sheet_Font_Coverage(alpha, 4, metrics.TopMargin) == 0) {
		metrics.TopMargin++;
	}
	for (int y = metrics.TopMargin; y < alpha.Height && UI_Sheet_Font_Coverage(alpha, 4, y) != 0; y++) {
		metrics.GlyphHeight++;
	}

	while (metrics.LeftMargin < alpha.Width && UI_Sheet_Font_Coverage(alpha, metrics.LeftMargin, metrics.TopMargin) == 0) {
		metrics.LeftMargin++;
	}
	for (int x = metrics.LeftMargin; x < alpha.Width && UI_Sheet_Font_Coverage(alpha, x, metrics.TopMargin) != 0; x++) {
		metrics.GlyphWidth++;
	}

	if (metrics.GlyphWidth <= 0 || metrics.GlyphHeight <= 0) {
		return(false);
	}

	metrics.CellsPerRow = alpha.Width / metrics.Cell_Width();
	if (metrics.CellsPerRow <= 0) {
		return(false);
	}

	for (int character = 0; character < 256; character++) {
		int cellx = 0;
		int celly = 0;
		if (!UI_Sheet_Font_Cell(metrics, alpha, character, cellx, celly)) {
			metrics.Advance[character] = metrics.GlyphWidth / 3 + 1;
			continue;
		}

		int first = -1;
		int last = 0;
		for (int x = cellx + metrics.LeftMargin; x < cellx + metrics.Cell_Width(); x++) {
			for (int y = celly + metrics.TopMargin; y < celly + metrics.Cell_Height(); y++) {
				if (UI_Sheet_Font_Coverage(alpha, x, y) != 0) {
					last = x;
					if (first == -1) {
						first = x;
					}
					break;
				}
			}
		}

		// A cell with nothing in it is a space, and the pen still has to move.
		metrics.Advance[character] = (first != -1) ? (last - first + 1) : (metrics.GlyphWidth / 3 + 1);
	}

	return(true);
}


bool UI_Sheet_Font_Cell(UISheetFontMetrics const & metrics, UIImageIndexed const & alpha, int character, int & x, int & y)
{
	x = 0;
	y = 0;

	// A space is moved over rather than drawn, as the dialog layer moves over every code at
	// or below it. Its cell on the real sheets is not blank, so drawing it marks the page.
	if (character <= ' ' || character > 255 || metrics.CellsPerRow <= 0) {
		return(false);
	}

	int cell = character + 1;
	x = (cell % metrics.CellsPerRow) * metrics.Cell_Width();
	y = (cell / metrics.CellsPerRow) * metrics.Cell_Height();

	return(x + metrics.Cell_Width() <= alpha.Width && y + metrics.Cell_Height() <= alpha.Height);
}


void UI_Sheet_Font_Remap(std::uint8_t const * palette, std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t * remapped)
{
	HSVClass wanted = RGBClass(red, green, blue);
	int hue = wanted.Get_Hue();

	// The weakest pull anywhere near the requested hue is the one the whole palette moves
	// by, so a color close to one of the resting hues does not tear the shading apart.
	float pull = 1.0f;
	for (int step = hue - UI_SHEET_FONT_HUE_WINDOW; step <= hue + UI_SHEET_FONT_HUE_WINDOW; step++) {
		float factor = Remap_Factor(step);
		if (factor < pull) {
			pull = factor;
		}
	}

	int saturation = wanted.Get_Saturation();
	int value = wanted.Get_Value();

	for (int entry = 0; entry < 256; entry++) {
		std::uint8_t const * source = palette + (std::size_t)entry * 3;
		HSVClass shade = RGBClass(source[0], source[1], source[2]);

		HSVClass out;
		out.Set_Hue((unsigned char)(int)((float)hue - (int)(UI_SHEET_FONT_SOURCE_HUE - shade.Get_Hue()) * pull));
		out.Set_Saturation((unsigned char)((saturation * shade.Get_Saturation()) >> 8));
		out.Set_Value((unsigned char)((value * shade.Get_Value()) >> 8));

		RGBClass result = out;
		std::uint8_t * destination = remapped + (std::size_t)entry * 3;
		destination[0] = (std::uint8_t)result.Get_Red();
		destination[1] = (std::uint8_t)result.Get_Green();
		destination[2] = (std::uint8_t)result.Get_Blue();
	}
}


bool UI_Sheet_Font_Atlas(UIImageIndexed const & index, UIImageIndexed const & alpha, std::uint8_t const * remapped, std::vector<std::uint8_t> & rgba)
{
	rgba.clear();

	if (index.Width <= 0 || index.Height <= 0 || index.Width != alpha.Width || index.Height != alpha.Height
		|| index.Pixels.size() != (std::size_t)index.Width * (std::size_t)index.Height
		|| alpha.Pixels.size() != index.Pixels.size()) {
		return(false);
	}

	rgba.resize(index.Pixels.size() * 4);

	for (std::size_t pixel = 0; pixel < index.Pixels.size(); pixel++) {
		std::uint8_t coverage = alpha.Palette[(std::size_t)alpha.Pixels[pixel] * 3];
		std::uint8_t const * color = remapped + (std::size_t)index.Pixels[pixel] * 3;
		std::uint8_t * out = rgba.data() + pixel * 4;

		out[0] = (std::uint8_t)(color[0] * coverage / 255);
		out[1] = (std::uint8_t)(color[1] * coverage / 255);
		out[2] = (std::uint8_t)(color[2] * coverage / 255);
		out[3] = coverage;
	}

	return(true);
}
