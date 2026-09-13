/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmltexture.h"

#include "ccfile.h"
#include "dbgprint.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_TGA
#define STBI_NO_STDIO
#include <stb_image.h>

#include <cstring>


static bool Has_Extension(char const * name, char const * extension)
{
	size_t length = strlen(name);
	size_t extensionlength = strlen(extension);

	return(length >= extensionlength && _stricmp(name + length - extensionlength, extension) == 0);
}


bool UI_Load_Image(char const * name, std::vector<unsigned char> & rgba, int & width, int & height)
{
	rgba.clear();
	width = 0;
	height = 0;

	if (name == NULL || (!Has_Extension(name, ".png") && !Has_Extension(name, ".tga"))) {
		return(false);
	}

	CCFileClass file(name);
	if (!file.Is_Available()) {
		return(false);
	}

	int size = file.Size();
	if (size <= 0) {
		return(false);
	}

	std::vector<unsigned char> encoded((size_t)size);
	if (!file.Open(FileClass::READ) || file.Read(encoded.data(), size) != size) {
		return(false);
	}
	file.Close();

	int channels = 0;
	unsigned char * pixels = stbi_load_from_memory(encoded.data(), size, &width, &height, &channels, 4);
	if (pixels == NULL) {
		DebugString("UI: %s did not decode: %s\n", name, stbi_failure_reason());
		width = 0;
		height = 0;
		return(false);
	}

	rgba.assign(pixels, pixels + (size_t)width * (size_t)height * 4);
	stbi_image_free(pixels);

	// RmlUi composes premultiplied color.
	for (size_t index = 0; index < rgba.size(); index += 4) {
		unsigned int alpha = rgba[index + 3];
		if (alpha != 255) {
			rgba[index] = (unsigned char)(rgba[index] * alpha / 255);
			rgba[index + 1] = (unsigned char)(rgba[index + 1] * alpha / 255);
			rgba[index + 2] = (unsigned char)(rgba[index + 2] * alpha / 255);
		}
	}

	return(true);
}
