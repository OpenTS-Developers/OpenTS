/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlfontfon.h"

#include "utf8.h"

#include <algorithm>
#include <cmath>
#include <cstring>


static bool Word_At(std::span<std::uint8_t const> data, std::size_t offset, unsigned int & value)
{
	if (offset + 2 > data.size()) {
		return(false);
	}
	value = (unsigned int)data[offset] | ((unsigned int)data[offset + 1] << 8);
	return(true);
}


UIRasterGlyph const * UIRasterStrike::Find(char32_t code) const
{
	std::vector<UIRasterGlyph>::const_iterator at = std::lower_bound(Glyphs.begin(), Glyphs.end(), code,
		[](UIRasterGlyph const & glyph, char32_t wanted) { return(glyph.Code < wanted); });
	return((at != Glyphs.end() && at->Code == code) ? &*at : nullptr);
}


int UIRasterStrike::Advance(char32_t code) const
{
	UIRasterGlyph const * glyph = Find(code);
	return((glyph != nullptr) ? glyph->Advance : 0);
}


/// <summary>
/// Returns the ANSI code page for a Windows font charset, such as 1250 for
/// EASTEUROPE_CHARSET (238). A charset not listed reads as 1252.
/// </summary>
unsigned int UI_Raster_Charset_Page(unsigned int charset)
{
	switch (charset) {
		case 238: return(1250);
		case 204: return(1251);
		case 161: return(1253);
		case 162: return(1254);
		case 186: return(1257);
		default: break;
	}
	return(1252);
}


static bool Long_At(std::span<std::uint8_t const> data, std::size_t offset, unsigned int & value)
{
	unsigned int low = 0;
	unsigned int high = 0;
	if (!Word_At(data, offset, low) || !Word_At(data, offset + 2, high)) {
		return(false);
	}
	value = low | (high << 16);
	return(true);
}


/// <summary>
/// Reads one Windows 2.0 or 3.0 .FNT bitmap font, keying each glyph by the Unicode code point
/// its charset maps it to. Control characters and empty glyphs are left out.
/// </summary>
/// <returns>False when the data is not a readable .FNT font; the strike is then
/// unchanged.</returns>
bool UI_Read_Raster_Strike(std::span<std::uint8_t const> data, UIRasterStrike & strike)
{
	unsigned int version = 0;
	unsigned int points = 0;
	unsigned int ascent = 0;
	unsigned int height = 0;
	unsigned int first = 0;
	unsigned int last = 0;
	unsigned int face = 0;
	if (!Word_At(data, 0x00, version) || !Word_At(data, 0x44, points) || !Word_At(data, 0x4A, ascent)
		|| !Word_At(data, 0x58, height) || !Long_At(data, 0x69, face)) {
		return(false);
	}
	if (0x55 >= data.size()) {
		return(false);
	}
	unsigned int page = UI_Raster_Charset_Page(data[0x55]);
	if ((version != 0x200 && version != 0x300) || height == 0 || height > 256) {
		return(false);
	}
	if (0x60 >= data.size()) {
		return(false);
	}
	first = data[0x5F];
	last = data[0x60];
	if (last < first) {
		return(false);
	}

	std::size_t table = (version == 0x200) ? 0x76 : 0x94;
	std::size_t entry = (version == 0x200) ? 4 : 6;
	std::size_t count = (std::size_t)(last - first) + 2;
	if (table + count * entry > data.size()) {
		return(false);
	}

	UIRasterStrike reading;
	reading.Points = (int)points;
	reading.Height = (int)height;
	reading.Ascent = (int)ascent;

	for (std::size_t index = 0; index + 1 < count; index++) {
		unsigned int width = 0;
		unsigned int offset = 0;
		std::size_t at = table + index * entry;
		if (!Word_At(data, at, width)) {
			return(false);
		}
		if (version == 0x200) {
			if (!Word_At(data, at + 2, offset)) {
				return(false);
			}
		} else if (!Long_At(data, at + 2, offset)) {
			return(false);
		}

		char32_t code = UTF8::Windows_Code(page, (unsigned char)(first + index));
		if (code < U' ' || width == 0) {
			continue;
		}

		std::size_t columns = (width + 7) / 8;
		if ((std::size_t)offset + columns * height > data.size()) {
			return(false);
		}

		UIRasterGlyph glyph;
		glyph.Code = code;
		glyph.Advance = (int)width;
		glyph.Coverage.assign((std::size_t)width * height, 0);
		for (unsigned int x = 0; x < width; x++) {
			std::size_t column = (std::size_t)offset + (std::size_t)(x / 8) * height;
			for (unsigned int y = 0; y < height; y++) {
				if ((data[column + y] >> (7 - (x % 8))) & 1) {
					glyph.Coverage[(std::size_t)y * width + x] = 255;
				}
			}
		}
		reading.Glyphs.push_back(std::move(glyph));
	}

	std::sort(reading.Glyphs.begin(), reading.Glyphs.end(),
		[](UIRasterGlyph const & left, UIRasterGlyph const & right) { return(left.Code < right.Code); });
	reading.Glyphs.erase(std::unique(reading.Glyphs.begin(), reading.Glyphs.end(),
		[](UIRasterGlyph const & left, UIRasterGlyph const & right) { return(left.Code == right.Code); }),
		reading.Glyphs.end());

	if (face != 0 && face < data.size()) {
		std::size_t end = face;
		while (end < data.size() && data[end] != 0) {
			end++;
		}
		reading.Face.assign((char const *)data.data() + face, end - face);
	}

	strike = std::move(reading);
	return(true);
}


/// <summary>
/// Reads the fonts of a Windows .FON file, keeping the first strike of each height.
/// </summary>
/// <returns>False when the data is not a .FON file or holds no readable strike.</returns>
bool UI_Read_Raster_Font(std::span<std::uint8_t const> data, std::vector<UIRasterStrike> & strikes)
{
	strikes.clear();

	unsigned int header = 0;
	if (data.size() < 0x40 || data[0] != 'M' || data[1] != 'Z' || !Word_At(data, 0x3C, header)) {
		return(false);
	}
	if (header + 0x26 > data.size() || data[header] != 'N' || data[header + 1] != 'E') {
		return(false);
	}

	unsigned int table = 0;
	unsigned int shift = 0;
	if (!Word_At(data, header + 0x24, table)) {
		return(false);
	}
	table += header;
	if (!Word_At(data, table, shift) || shift > 16) {
		return(false);
	}

	std::size_t at = (std::size_t)table + 2;
	while (true) {
		unsigned int type = 0;
		unsigned int count = 0;
		if (!Word_At(data, at, type) || !Word_At(data, at + 2, count)) {
			return(false);
		}
		if (type == 0) {
			break;
		}
		at += 8;
		for (unsigned int index = 0; index < count; index++) {
			unsigned int offset = 0;
			unsigned int length = 0;
			if (!Word_At(data, at, offset) || !Word_At(data, at + 2, length)) {
				return(false);
			}
			at += 12;

			if (type != 0x8008) {
				continue;
			}

			std::size_t start = (std::size_t)offset << shift;
			std::size_t size = (std::size_t)length << shift;
			if (start > data.size() || size > data.size() - start) {
				continue;
			}

			UIRasterStrike strike;
			if (!UI_Read_Raster_Strike(data.subspan(start, size), strike)) {
				continue;
			}

			bool known = false;
			for (UIRasterStrike const & read : strikes) {
				known = known || read.Height == strike.Height;
			}
			if (!known) {
				strikes.push_back(std::move(strike));
			}
		}
	}

	return(!strikes.empty());
}


/// <summary>
/// Adds each extra strike's glyphs to the strike of the same height and ascent, keeping the
/// glyphs that strike already has. An extra strike with no such match is ignored.
/// </summary>
/// <returns>The number of glyphs added.</returns>
int UI_Merge_Raster_Strikes(std::vector<UIRasterStrike> & strikes, std::vector<UIRasterStrike> const & extra)
{
	int added = 0;

	for (UIRasterStrike const & donor : extra) {
		UIRasterStrike * into = nullptr;
		for (UIRasterStrike & strike : strikes) {
			if (strike.Height == donor.Height) {
				into = &strike;
				break;
			}
		}

		if (into == nullptr || into->Ascent != donor.Ascent) {
			continue;
		}

		std::vector<UIRasterGlyph> merged;
		merged.reserve(into->Glyphs.size() + donor.Glyphs.size());

		std::size_t mine = 0;
		std::size_t theirs = 0;
		while (mine < into->Glyphs.size() && theirs < donor.Glyphs.size()) {
			if (into->Glyphs[mine].Code < donor.Glyphs[theirs].Code) {
				merged.push_back(std::move(into->Glyphs[mine]));
				mine++;
			} else if (donor.Glyphs[theirs].Code < into->Glyphs[mine].Code) {
				merged.push_back(donor.Glyphs[theirs]);
				theirs++;
				added++;
			} else {
				merged.push_back(std::move(into->Glyphs[mine]));
				mine++;
				theirs++;
			}
		}
		for (; mine < into->Glyphs.size(); mine++) {
			merged.push_back(std::move(into->Glyphs[mine]));
		}
		for (; theirs < donor.Glyphs.size(); theirs++) {
			merged.push_back(donor.Glyphs[theirs]);
			added++;
		}

		into->Glyphs = std::move(merged);
	}

	return(added);
}


bool UI_Raster_Strike_Layout(UIRasterStrike const & strike, int limit,
	std::vector<UIRasterCell> & cells, int & width, int & height)
{
	cells.clear();
	width = 0;
	height = 0;

	if (strike.Glyphs.empty() || strike.Height <= 0 || limit < strike.Height) {
		return(false);
	}

	int widest = 1;
	double total = 0.0;
	for (UIRasterGlyph const & glyph : strike.Glyphs) {
		widest = (glyph.Advance > widest) ? glyph.Advance : widest;
		total += (double)glyph.Advance;
	}
	if (widest > limit) {
		return(false);
	}

	int across = (int)std::ceil(std::sqrt(total * (double)strike.Height));
	across = (across < widest) ? widest : across;

	while (true) {
		cells.clear();
		cells.reserve(strike.Glyphs.size());

		int x = 0;
		int y = 0;
		for (UIRasterGlyph const & glyph : strike.Glyphs) {
			if (x > 0 && x + glyph.Advance > across) {
				x = 0;
				y += strike.Height;
			}

			UIRasterCell cell;
			cell.Code = glyph.Code;
			cell.Advance = glyph.Advance;
			cell.X = x;
			cell.Y = y;
			cells.push_back(cell);
			x += glyph.Advance;
		}

		if (y + strike.Height <= limit) {
			width = across;
			height = y + strike.Height;
			return(true);
		}

		if (across >= limit) {
			cells.clear();
			return(false);
		}
		across = (across * 2 > limit) ? limit : across * 2;
	}
}
