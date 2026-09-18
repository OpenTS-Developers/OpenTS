/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>


struct UIRenderClip
{
	std::uint16_t X = 0;
	std::uint16_t Y = 0;
	std::uint16_t Width = 0;
	std::uint16_t Height = 0;
};


enum UIRenderMaskOperation
{
	UI_RENDER_MASK_SET,
	UI_RENDER_MASK_SET_INVERSE,
	UI_RENDER_MASK_INTERSECT
};


struct UIRenderMaskStep
{
	bool Clear = false;
	bool Increment = false;
	std::uint8_t Write = 0;
	std::uint8_t Reference = 0;
};


bool UI_Render_Byte_Count(std::size_t count, std::size_t stride, std::uint32_t & bytes);

bool UI_Render_Index_Range(std::span<int const> indices, std::size_t vertexcount);

bool UI_Render_Clip_Rect(float left, float top, float right, float bottom, int viewportx, int viewporty, int viewportwidth, int viewportheight, UIRenderClip & clip);

bool UI_Render_Copy_RGBA_Rect(std::span<std::uint8_t const> pixels, int width, int height, int pitch, int x, int y, int rectwidth, int rectheight, std::vector<std::uint8_t> & result);

bool UI_Render_Magnify_RGBA(std::span<std::uint8_t const> pixels, int width, int height, int factor, std::vector<std::uint8_t> & result);

void UI_Render_Model_Matrix(float const * transform, float translationx, float translationy, float * result);

bool UI_Render_Mask_Step(UIRenderMaskOperation operation, std::uint8_t reference, UIRenderMaskStep & step);
