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
