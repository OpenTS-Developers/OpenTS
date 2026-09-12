/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlrendermath.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>


bool UI_Render_Byte_Count(std::size_t count, std::size_t stride, std::uint32_t & bytes)
{
	bytes = 0;
	if (count == 0 || stride == 0 || count > std::numeric_limits<std::uint32_t>::max() / stride) {
		return(false);
	}
	bytes = (std::uint32_t)(count * stride);
	return(true);
}


bool UI_Render_Index_Range(std::span<int const> indices, std::size_t vertexcount)
{
	if (indices.empty() || indices.size() % 3 != 0 || vertexcount == 0) {
		return(false);
	}
	return(std::all_of(indices.begin(), indices.end(), [vertexcount](int index) {
		return(index >= 0 && (std::size_t)index < vertexcount);
	}));
}


bool UI_Render_Clip_Rect(float left, float top, float right, float bottom, int viewportx, int viewporty, int viewportwidth, int viewportheight, UIRenderClip & clip)
{
	clip = UIRenderClip();

	if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(right) || !std::isfinite(bottom)
		|| viewportx < 0 || viewporty < 0 || viewportwidth <= 0 || viewportheight <= 0
		|| (std::int64_t)viewportx + viewportwidth > UINT16_MAX
		|| (std::int64_t)viewporty + viewportheight > UINT16_MAX
		|| right <= left || bottom <= top) {
		return(false);
	}

	double x1 = std::clamp(std::floor((double)left), 0.0, (double)viewportwidth);
	double y1 = std::clamp(std::floor((double)top), 0.0, (double)viewportheight);
	double x2 = std::clamp(std::ceil((double)right), 0.0, (double)viewportwidth);
	double y2 = std::clamp(std::ceil((double)bottom), 0.0, (double)viewportheight);
	if (x2 <= x1 || y2 <= y1) {
		return(false);
	}

	clip.X = (std::uint16_t)(viewportx + x1);
	clip.Y = (std::uint16_t)(viewporty + y1);
	clip.Width = (std::uint16_t)(x2 - x1);
	clip.Height = (std::uint16_t)(y2 - y1);
	return(true);
}


bool UI_Render_Copy_RGBA_Rect(std::span<std::uint8_t const> pixels, int width, int height, int pitch, int x, int y, int rectwidth, int rectheight, std::vector<std::uint8_t> & result)
{
	result.clear();

	if (width <= 0 || height <= 0 || pitch <= 0 || x < 0 || y < 0 || rectwidth <= 0 || rectheight <= 0
		|| x > width || y > height || rectwidth > width - x || rectheight > height - y
		|| (std::uint64_t)width * 4 > (std::uint64_t)pitch
		|| (std::uint64_t)pitch * (height - 1) + (std::uint64_t)width * 4 > pixels.size()) {
		return(false);
	}

	std::uint32_t rowbytes = 0;
	std::uint32_t totalbytes = 0;
	if (!UI_Render_Byte_Count((std::size_t)rectwidth, 4, rowbytes) || !UI_Render_Byte_Count((std::size_t)rectheight, rowbytes, totalbytes)) {
		return(false);
	}

	result.resize(totalbytes);
	for (int row = 0; row < rectheight; row++) {
		std::size_t source = (std::size_t)(y + row) * pitch + (std::size_t)x * 4;
		std::memcpy(result.data() + (std::size_t)row * rowbytes, pixels.data() + source, rowbytes);
	}
	return(true);
}
