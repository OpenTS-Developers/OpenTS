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
#include "rawfile.h"
#include "dbgprint.h"
#include "ui/rml/rmlimage.h"

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


// The whole file through the game's search, so a document reaches art inside a mix
// archive as readily as one beside the executable.
static bool Read_Whole_File(char const * name, std::vector<unsigned char> & bytes)
{
	bytes.clear();

	CCFileClass file(name);
	if (!file.Is_Available()) {
		return(false);
	}

	int size = file.Size();
	if (size <= 0) {
		return(false);
	}

	bytes.resize((size_t)size);
	if (!file.Open(FileClass::READ) || file.Read(bytes.data(), size) != size) {
		bytes.clear();
		return(false);
	}
	file.Close();

	return(true);
}


bool UI_Read_File(char const * path, std::vector<unsigned char> & bytes)
{
	bytes.clear();

	if (path == NULL) {
		return(false);
	}

	RawFileClass file(path);
	if (!file.Is_Available()) {
		return(false);
	}

	int size = file.Size();
	if (size <= 0) {
		return(false);
	}

	bytes.resize((size_t)size);
	if (!file.Open(FileClass::READ) || file.Read(bytes.data(), size) != size) {
		bytes.clear();
		return(false);
	}
	file.Close();

	return(true);
}


bool UI_Load_Indexed_Image(char const * name, UIImageIndexed & image)
{
	image = UIImageIndexed();

	if (name == NULL || !Has_Extension(name, ".pcx")) {
		return(false);
	}

	std::vector<unsigned char> encoded;
	if (!Read_Whole_File(name, encoded)) {
		return(false);
	}

	return(UI_Decode_PCX(std::span<std::uint8_t const>(encoded.data(), encoded.size()), image));
}


UIImageResult UI_Load_Image(char const * name, std::vector<unsigned char> & rgba, int & width, int & height)
{
	rgba.clear();
	width = 0;
	height = 0;

	if (name == NULL) {
		return(UI_IMAGE_MISSING);
	}

	bool pcx = Has_Extension(name, ".pcx");
	if (!pcx && !Has_Extension(name, ".png") && !Has_Extension(name, ".tga")) {
		return(UI_IMAGE_UNREADABLE);
	}

	std::vector<unsigned char> encoded;
	if (!Read_Whole_File(name, encoded)) {
		return(UI_IMAGE_MISSING);
	}

	if (pcx) {
		UIImageIndexed image;
		if (!UI_Decode_PCX(std::span<std::uint8_t const>(encoded.data(), encoded.size()), image)
			|| !UI_Indexed_To_RGBA(image, rgba)) {
			DebugString("UI: %s is not an 8-bit run-length PCX\n", name);
			rgba.clear();
			return(UI_IMAGE_UNREADABLE);
		}

		width = image.Width;
		height = image.Height;
		return(UI_IMAGE_LOADED);
	}

	int channels = 0;
	unsigned char * pixels = stbi_load_from_memory(encoded.data(), (int)encoded.size(), &width, &height, &channels, 4);
	if (pixels == NULL) {
		DebugString("UI: %s did not decode: %s\n", name, stbi_failure_reason());
		width = 0;
		height = 0;
		return(UI_IMAGE_UNREADABLE);
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

	return(UI_IMAGE_LOADED);
}
