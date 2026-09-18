/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

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


void UI_Register_Surface_Element(void);
