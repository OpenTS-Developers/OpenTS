/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The map preview on its way to a screen. It travels as bytes rather than as the surface the
// engine drew it on, so that no screen holds an engine pointer and the harness can build one.

#pragma once

#include <cstdint>
#include <vector>


// Premultiplied RGBA8, four bytes a pixel, rows from the top down. The generation moves
// whenever the picture does, which is how a view knows to hand the element new bytes.
struct UIMapPreviewImage
{
	int Width = 0;
	int Height = 0;
	std::vector<std::uint8_t> Pixels;
	int Generation = 0;
};


// Copies the preview the map dialogs share. False, with an empty picture at a fresh
// generation, when there is none: a map carrying no preview is not an error.
bool UI_Map_Preview_Image(UIMapPreviewImage & image);
