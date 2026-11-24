/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlfontfnt.h"

#include "utf8.h"

#include <algorithm>


static const std::size_t UI_FNT_HEADER_SIZE = 14;
static const int UI_FNT_INFO_MAX_HEIGHT = 4;
static const int UI_FNT_INFO_MAX_WIDTH = 5;
static const int UI_FNT_NEW_STYLE = 2;
static const int UI_FNT_SPACING_DIVISOR = 16;
static const int UI_FNT_ATLAS_COLUMNS = 16;


static int Read_Word(std::span<std::uint8_t const> data, std::size_t at)
{
	return(data[at] | (data[at + 1] << 8));
}


/// <summary>
/// Decodes a Westwood .FNT font, in either the nibble-packed or the byte-per-pixel layout.
/// </summary>
/// <returns>bool; Was the font read? Data that is short of any block or glyph it describes is
/// refused, and leaves the font empty.</returns>
bool UI_Read_Fnt(std::span<std::uint8_t const> data, UIFntFont & font)
{
	font = UIFntFont();

	if (data.size() < UI_FNT_HEADER_SIZE) {
		return(false);
	}

	bool const newstyle = data[2] == UI_FNT_NEW_STYLE;
	std::size_t const info = (std::size_t)Read_Word(data, 4);
	std::size_t const offsets = (std::size_t)Read_Word(data, 6);
	std::size_t const widths = (std::size_t)Read_Word(data, 8);
	std::size_t const pixels = (std::size_t)Read_Word(data, 10);
	std::size_t const heights = (std::size_t)Read_Word(data, 12);

	if (widths <= offsets || info + UI_FNT_INFO_MAX_WIDTH >= data.size()) {
		return(false);
	}

	std::size_t const count = (widths - offsets) / 2;
	if (count > 256 || offsets + count * 2 > data.size() || widths + count > data.size()
		|| heights + count * 2 > data.size()) {
		return(false);
	}

	UIFntFont read;
	read.Height = data[info + UI_FNT_INFO_MAX_HEIGHT];
	read.MaxWidth = data[info + UI_FNT_INFO_MAX_WIDTH];
	read.Windows1252 = count >= 0xFF;
	read.Glyphs.resize(count);

	for (std::size_t index = 0; index < count; index++) {
		UIFntGlyph & glyph = read.Glyphs[index];
		std::size_t const offset = (std::size_t)Read_Word(data, offsets + index * 2);
		int const height = Read_Word(data, heights + index * 2);

		glyph.Width = data[widths + index];
		glyph.Rows = height >> 8;
		glyph.FirstRow = height & 0xFF;
		glyph.Defined = index <= ' ' || offset != 0;

		std::size_t const stride = newstyle ? (std::size_t)glyph.Width : (std::size_t)(glyph.Width + 1) / 2;
		std::size_t const start = newstyle ? pixels + offset : offset;
		std::size_t const length = stride * (std::size_t)glyph.Rows;
		if (start + length > data.size()) {
			return(false);
		}

		glyph.Pixels.resize((std::size_t)glyph.Width * (std::size_t)glyph.Rows);
		for (int y = 0; y < glyph.Rows; y++) {
			for (int x = 0; x < glyph.Width; x++) {
				std::uint8_t value = 0;
				if (newstyle) {
					value = data[start + (std::size_t)y * stride + (std::size_t)x];
				} else {
					std::uint8_t const packed = data[start + (std::size_t)y * stride + (std::size_t)x / 2];
					value = (x & 1) ? (std::uint8_t)(packed >> 4) : (std::uint8_t)(packed & 0x0F);
				}
				glyph.Pixels[(std::size_t)y * glyph.Width + x] = (std::uint8_t)(value & 0x0F);
			}
		}
	}

	font = std::move(read);
	return(true);
}


/// <summary>
/// Finds the glyph the game draws for a character, using the font's code page.
/// </summary>
/// <returns>The glyph index, or -1 for a control character, which has no glyph and no width.
/// A character the font lacks gives the question mark.</returns>
int UI_Fnt_Glyph(UIFntFont const & font, char32_t code)
{
	if (code < U' ') {
		return(-1);
	}

	int index = 0;
	if (font.Windows1252) {
		index = UTF8::Windows_1252_Glyph(code);
	} else if (code == 0x0153) {
		// Westwood drew the 12-point metal font's oe ligature into an unused code page 437 slot.
		index = 0xCE;
	} else {
		index = UTF8::OEM_437_Glyph(code);
	}

	if (index < 0 || index >= (int)font.Glyphs.size() || (index > ' ' && !font.Glyphs[(std::size_t)index].Defined)) {
		return(((int)font.Glyphs.size() > '?') ? '?' : -1);
	}
	return(index);
}


/// <summary>
/// Gives the spacing the game's font class settles on for a spacing override, so text is laid
/// out at the same widths as the game draws it.
/// </summary>
/// <param name="raw">The font's largest glyph width, or its height for vertical spacing.</param>
/// <param name="spacing">The override the game sets for this font.</param>
/// <param name="outlined">Was the font loaded as outlined, which pulls its glyphs two pixels
/// closer?</param>
int UI_Fnt_Spacing(int raw, int spacing, bool outlined)
{
	int settled = spacing + (outlined ? -2 : 0);
	settled += (raw + std::max(settled, 0)) / UI_FNT_SPACING_DIVISOR;
	return(settled);
}


/// <summary>
/// Fills the colors the game prints a font in when no shadow is asked for: the text color for
/// pixel value 1, and for a gradient font for its shade values 4 to 15 as well. Every other
/// value is clear.
/// </summary>
void UI_Fnt_Default_Palette(std::uint32_t text, bool gradient, UIFntPalette & palette)
{
	palette.fill(0);
	palette[UI_FNT_TEXT] = text;
	if (gradient) {
		std::fill(palette.begin() + UI_FNT_FIRST_SHADE, palette.end(), text);
	}
}


/// <summary>
/// Places every glyph of a font in its own cell of an atlas, with a clear pixel between
/// neighbors.
/// </summary>
/// <returns>bool; Does the atlas fit within the limit on both sides?</returns>
bool UI_Fnt_Layout(UIFntFont const & font, int limit, std::vector<UIFntCell> & cells, int & width, int & height)
{
	cells.clear();
	width = 0;
	height = 0;

	if (font.Glyphs.empty() || font.Height <= 0) {
		return(false);
	}

	int widest = 0;
	for (UIFntGlyph const & glyph : font.Glyphs) {
		widest = std::max(widest, glyph.Width);
	}

	int const cellwidth = widest + 1;
	int const cellheight = font.Height + 1;
	int const rows = ((int)font.Glyphs.size() + UI_FNT_ATLAS_COLUMNS - 1) / UI_FNT_ATLAS_COLUMNS;
	int const atlaswidth = cellwidth * UI_FNT_ATLAS_COLUMNS + 1;
	int const atlasheight = cellheight * rows + 1;
	if (atlaswidth > limit || atlasheight > limit) {
		return(false);
	}

	cells.resize(font.Glyphs.size());
	for (std::size_t index = 0; index < cells.size(); index++) {
		cells[index].X = 1 + (int)(index % UI_FNT_ATLAS_COLUMNS) * cellwidth;
		cells[index].Y = 1 + (int)(index / UI_FNT_ATLAS_COLUMNS) * cellheight;
	}

	width = atlaswidth;
	height = atlasheight;
	return(true);
}


/// <summary>
/// Draws every glyph into a premultiplied RGBA atlas, coloring each pixel value from the
/// palette. Rows below the font's height are left out, as the game clips them.
/// </summary>
/// <returns>bool; Was the atlas drawn? It fails when the cells do not match the font.</returns>
bool UI_Fnt_Atlas(UIFntFont const & font, std::vector<UIFntCell> const & cells, int width, int height, UIFntPalette const & palette, std::vector<std::uint8_t> & rgba)
{
	rgba.clear();

	if (cells.size() != font.Glyphs.size() || width <= 0 || height <= 0) {
		return(false);
	}

	rgba.assign((std::size_t)width * (std::size_t)height * 4, 0);

	for (std::size_t index = 0; index < cells.size(); index++) {
		UIFntGlyph const & glyph = font.Glyphs[index];
		UIFntCell const & cell = cells[index];
		int const rows = std::min(glyph.Rows, font.Height - glyph.FirstRow);

		for (int y = 0; y < rows; y++) {
			int const ty = cell.Y + glyph.FirstRow + y;
			for (int x = 0; x < glyph.Width; x++) {
				int const tx = cell.X + x;
				if (tx < 0 || ty < 0 || tx >= width || ty >= height) {
					continue;
				}

				std::uint32_t const color = palette[glyph.Pixels[(std::size_t)y * glyph.Width + x]];
				std::uint8_t const alpha = (std::uint8_t)(color & 0xFF);
				std::uint8_t * out = rgba.data() + ((std::size_t)ty * width + tx) * 4;
				out[0] = (std::uint8_t)(((color >> 24) & 0xFF) * alpha / 255);
				out[1] = (std::uint8_t)(((color >> 16) & 0xFF) * alpha / 255);
				out[2] = (std::uint8_t)(((color >> 8) & 0xFF) * alpha / 255);
				out[3] = alpha;
			}
		}
	}

	return(true);
}
