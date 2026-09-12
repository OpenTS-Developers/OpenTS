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


// Decodes a PNG or TGA image, named the way documents name their resources, into
// premultiplied RGBA8 rows from the top down. False when the file is missing, unreadable
// or of another kind; the output is then empty.
bool UI_Load_Image(char const * name, std::vector<unsigned char> & rgba, int & width, int & height);
