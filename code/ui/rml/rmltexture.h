/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <vector>


// What became of a document's request for an image. Art the player does not have is told
// apart from art that is there and will not decode, because a screen may be missing a
// decoration and still be worth showing.
enum UIImageResult
{
	UI_IMAGE_LOADED,
	UI_IMAGE_MISSING,
	UI_IMAGE_UNREADABLE
};


// Decodes a PCX, PNG or TGA image, named the way documents name their resources, into
// premultiplied RGBA8 rows from the top down. The output is empty unless the result is
// UI_IMAGE_LOADED.
UIImageResult UI_Load_Image(char const * name, std::vector<unsigned char> & rgba, int & width, int & height);
