/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Turning the interface art into pixels a document can draw. The work knows no toolkit,
// renderer or file system, so a harness runs it over bytes it builds itself.

#pragma once

#include <cstdint>
#include <span>
#include <vector>


// An 8-bit picture as a PCX file holds it: one palette index per pixel, rows from the top
// down with no padding, and the 256 colors those indices name.
struct UIImageIndexed
{
	int Width = 0;
	int Height = 0;
	std::vector<std::uint8_t> Pixels;
	std::uint8_t Palette[768] = {};
};


// Decodes an 8-bit run-length PCX. False when the bytes are not one, or claim more than
// they hold; the picture is then empty.
bool UI_Decode_PCX(std::span<std::uint8_t const> encoded, UIImageIndexed & image);

// Premultiplied RGBA8 rows from the top down, as RmlUi composes them. Pure magenta is the
// color key this art is drawn with, so those pixels come back clear.
bool UI_Indexed_To_RGBA(UIImageIndexed const & image, std::vector<std::uint8_t> & rgba);

// Premultiplied RGBA8 from a 16-bit 565 picture, which is what every surface the engine
// draws into holds. Pitch is in pixels rather than bytes. The five and six bit channels are
// widened by repeating their top bits, the way the frame shows them.
bool UI_Hicolor_To_RGBA(std::span<std::uint16_t const> pixels, int width, int height, int pitch, std::vector<std::uint8_t> & rgba);

// Where a picture of the given size lands inside a box, kept in proportion and centred.
// Whichever side runs out first fills the box exactly. The dialog layer scaled a map preview
// in thousandths and halved both the box and the picture in whole pixels, which left it a
// pixel short of the frame and the gap all on one side; this centres it instead. False,
// leaving nothing placed, for a size that is not positive.
bool UI_Surface_Fit(int width, int height, int boxwidth, int boxheight, int & x, int & y, int & fitwidth, int & fitheight);

// A picture at another size, by repeating and dropping whole pixels rather than blending
// them, which is the stretch GDI gave the dialog layer in its COLORONCOLOR mode. False, with
// the result empty, for a size that is not positive or a picture short of its own.
bool UI_Scale_RGBA_Nearest(std::span<std::uint8_t const> pixels, int width, int height, int destwidth, int destheight, std::vector<std::uint8_t> & result);
