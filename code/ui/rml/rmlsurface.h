/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// A picture the engine drew while the game ran, in a document. The map preview is the only
// one so far. It is not a file the image loader could find, so the element is handed its
// pixels by the screen that owns them instead of naming a source of its own.

#pragma once

#include <RmlUi/Core/CallbackTexture.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Geometry.h>

#include <cstdint>
#include <vector>


class UIRmlSurfaceElementClass : public Rml::Element
{
	public:
		RMLUI_RTTI_DeclareWithParent(UIRmlSurfaceElementClass, Rml::Element)

		explicit UIRmlSurfaceElementClass(Rml::String const & tag);
		virtual ~UIRmlSurfaceElementClass(void) override;

		// Premultiplied RGBA8, four bytes a pixel, rows from the top down.
		void Set_Image(int width, int height, std::vector<std::uint8_t> pixels);

	protected:
		virtual void OnRender(void) override;
		virtual void OnResize(void) override;

	private:
		void Generate_Geometry(void);

		int Width;
		int Height;
		int DrawnWidth;
		int DrawnHeight;
		std::vector<std::uint8_t> Pixels;
		Rml::CallbackTexture Picture;
		Rml::Geometry Shape;
		bool Dirty;
};


// Adds the surface tag to the toolkit's factory. Call once, after Rml::Initialise.
void UI_Register_Surface_Element(void);
