/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "ui/uipreview.h"

#include "mapgen.h"
#include "netshare.h"
#include "preview.h"
#include "ui/rml/rmlimage.h"
#include "xsurface.h"

#include <span>


bool UI_Map_Preview_Image(UIMapPreviewImage & image)
{
	int generation = image.Generation + 1;

	image = UIMapPreviewImage();
	image.Generation = generation;

	if (MultiplayerMapPreview == NULL) {
		return(false);
	}

	XSurface * surface = MultiplayerMapPreview->Get_Preview_Surface();
	if (surface == NULL || surface->Bytes_Per_Pixel() != 2) {
		return(false);
	}

	int width = surface->Get_Width();
	int height = surface->Get_Height();
	int pitch = surface->Stride() / 2;
	if (width <= 0 || height <= 0 || pitch < width) {
		return(false);
	}

	std::uint16_t const * pixels = (std::uint16_t const *)surface->Lock();
	if (pixels == NULL) {
		return(false);
	}

	std::size_t held = (std::size_t)pitch * (std::size_t)(height - 1) + (std::size_t)width;
	bool copied = UI_Hicolor_To_RGBA(std::span<std::uint16_t const>(pixels, held), width, height, pitch, image.Pixels);
	surface->Unlock();

	if (!copied) {
		image.Pixels.clear();
		return(false);
	}

	image.Width = width;
	image.Height = height;
	return(true);
}


bool UI_Generated_Map_Preview_Image(UIMapPreviewImage & image)
{
	int generation = image.Generation + 1;

	image = UIMapPreviewImage();
	image.Generation = generation;

	if (RandomMapGen.MapPreview == NULL) {
		return(false);
	}

	XSurface * surface = RandomMapGen.MapPreview->Get_Preview_Surface();
	if (surface == NULL || surface->Bytes_Per_Pixel() != 2) {
		return(false);
	}

	int width = surface->Get_Width();
	int height = surface->Get_Height();
	int pitch = surface->Stride() / 2;
	if (width <= 0 || height <= 0 || pitch < width) {
		return(false);
	}

	std::uint16_t const * pixels = (std::uint16_t const *)surface->Lock();
	if (pixels == NULL) {
		return(false);
	}

	std::size_t held = (std::size_t)pitch * (std::size_t)(height - 1) + (std::size_t)width;
	bool copied = UI_Hicolor_To_RGBA(std::span<std::uint16_t const>(pixels, held), width, height, pitch, image.Pixels);
	surface->Unlock();

	if (!copied) {
		image.Pixels.clear();
		return(false);
	}

	image.Width = width;
	image.Height = height;
	return(true);
}
