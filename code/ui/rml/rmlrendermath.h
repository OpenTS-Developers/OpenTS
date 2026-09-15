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


// What a document asks a clip mask to do: start one, start the area outside one, or narrow
// the one already there.
enum UIRenderMaskOperation
{
	UI_RENDER_MASK_SET,
	UI_RENDER_MASK_SET_INVERSE,
	UI_RENDER_MASK_INTERSECT
};


// What the stencil does for one mask operation: whether the whole target starts over,
// whether the shape counts up rather than writing a value, what it writes, and what the
// draws that follow test against.
struct UIRenderMaskStep
{
	bool Clear = false;
	bool Increment = false;
	std::uint8_t Write = 0;
	std::uint8_t Reference = 0;
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

// A picture `factor` times larger in each direction with every pixel repeated, so that
// drawing it smoothly keeps whole pixels whole. A factor of one copies. False, with the
// result empty, for a size or factor that is not positive or a picture short of its size.
bool UI_Render_Magnify_RGBA(std::span<std::uint8_t const> pixels, int width, int height, int factor, std::vector<std::uint8_t> & result);

// The sixteen floats for one draw: the document's transform, or none, with the fragment's
// translation applied before it. Both RmlUi and bgfx hold a matrix as four columns, with
// the translation last, so the result travels between them untouched.
void UI_Render_Model_Matrix(float const * transform, float translationx, float translationy, float * result);

// What the stencil does for one clip mask operation, given what the masks already there
// test against. False when they nest deeper than eight bits count.
bool UI_Render_Mask_Step(UIRenderMaskOperation operation, std::uint8_t reference, UIRenderMaskStep & step);
