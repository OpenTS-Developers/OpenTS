/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The checks the overlay renderer makes before it hands anything to bgfx. They know no
// toolkit or renderer type, so a harness runs them without either.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>


// A scissor in the target's own pixels, as bgfx takes it.
struct UIRenderClip
{
	std::uint16_t X = 0;
	std::uint16_t Y = 0;
	std::uint16_t Width = 0;
	std::uint16_t Height = 0;
};


// count times stride as a 32-bit byte count; false when either is zero or the product
// does not fit.
bool UI_Render_Byte_Count(std::size_t count, std::size_t stride, std::uint32_t & bytes);

// True for whole triangles whose every index names one of the vertices.
bool UI_Render_Index_Range(std::span<int const> indices, std::size_t vertexcount);

// The viewport-relative rectangle rounded outward to whole pixels, clipped to the viewport
// and placed in the target. False when nothing is left or the target would not fit bgfx's
// 16-bit coordinates.
bool UI_Render_Clip_Rect(float left, float top, float right, float bottom, int viewportx, int viewporty, int viewportwidth, int viewportheight, UIRenderClip & clip);

// A tightly packed copy of a rectangle out of a pitched RGBA image. False when the
// rectangle or the image is not what it claims.
bool UI_Render_Copy_RGBA_Rect(std::span<std::uint8_t const> pixels, int width, int height, int pitch, int x, int y, int rectwidth, int rectheight, std::vector<std::uint8_t> & result);
