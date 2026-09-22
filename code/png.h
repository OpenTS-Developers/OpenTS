/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "wwfile.h"

// The pixels are the 16 bit 565 the surfaces hold, and the stride is in bytes.
bool Write_PNG_File(FileClass & file, int width, int height, int stride, unsigned short const * pixels);
