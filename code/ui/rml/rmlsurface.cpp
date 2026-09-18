/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlsurface.h"

#include "ui/rml/rmlimage.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/ElementInstancer.h>
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/MeshUtilities.h>
#include <RmlUi/Core/RenderManager.h>

#include <span>
#include <utility>


RMLUI_RTTI_Define(UIRmlSurfaceElementClass)


UIRmlSurfaceElementClass::UIRmlSurfaceElementClass(Rml::String const & tag) :
	Rml::Element(tag),
	Width(0),
	Height(0),
	DrawnWidth(0),
	DrawnHeight(0),
	Dirty(true)
{
}


UIRmlSurfaceElementClass::~UIRmlSurfaceElementClass(void)
{
}


void UIRmlSurfaceElementClass::Set_Image(int width, int height, std::vector<std::uint8_t> pixels)
{
	if (width <= 0 || height <= 0 || pixels.size() != (std::size_t)width * (std::size_t)height * 4) {
		Width = 0;
		Height = 0;
		Pixels.clear();
	} else {
		Width = width;
		Height = height;
		Pixels = std::move(pixels);
	}

	Picture.Release();
	DrawnWidth = 0;
	DrawnHeight = 0;
	Dirty = true;
}


void UIRmlSurfaceElementClass::OnRender(void)
{
	if (Dirty) {
		Generate_Geometry();
	}

	if (Picture) {
		Shape.Render(GetAbsoluteOffset(Rml::BoxArea::Border), Picture);
	}
}


void UIRmlSurfaceElementClass::OnResize(void)
{
	Dirty = true;
}


void UIRmlSurfaceElementClass::Generate_Geometry(void)
{
	Dirty = false;

	Rml::Mesh mesh = Shape.Release(Rml::Geometry::ReleaseMode::ClearMesh);
	Rml::RenderManager * manager = GetRenderManager();

	if (manager == nullptr || Pixels.empty()) {
		Picture.Release();
		return;
	}

	Rml::RenderBox box = GetRenderBox(Rml::BoxArea::Content);
	Rml::Vector2f offset = box.GetFillOffset();
	Rml::Vector2f size = box.GetFillSize();

	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	if (!UI_Surface_Fit(Width, Height, (int)size.x, (int)size.y, x, y, width, height)) {
		Picture.Release();
		return;
	}

	if (!Picture || DrawnWidth != width || DrawnHeight != height) {
		std::vector<std::uint8_t> drawn;
		if (!UI_Scale_RGBA_Nearest(std::span<std::uint8_t const>(Pixels.data(), Pixels.size()), Width, Height, width, height, drawn)) {
			Picture.Release();
			return;
		}

		Rml::Vector2i dimensions(width, height);
		Picture = manager->MakeCallbackTexture([drawn, dimensions](Rml::CallbackTextureInterface const & interface) {
			return(interface.GenerateTexture(Rml::Span<const Rml::byte>(drawn.data(), drawn.size()), dimensions));
		});
		DrawnWidth = width;
		DrawnHeight = height;
	}

	Rml::ComputedValues const & computed = GetComputedValues();
	Rml::ColourbPremultiplied color = computed.image_color().ToPremultiplied(computed.opacity());

	Rml::MeshUtilities::GenerateQuad(mesh, offset + Rml::Vector2f((float)x, (float)y),
		Rml::Vector2f((float)width, (float)height), color, Rml::Vector2f(0.0f, 0.0f), Rml::Vector2f(1.0f, 1.0f));
	Shape = manager->MakeGeometry(std::move(mesh));
}


void UI_Register_Surface_Element(void)
{
	static Rml::ElementInstancerGeneric<UIRmlSurfaceElementClass> instancer;
	Rml::Factory::RegisterElementInstancer("surface", &instancer);
}
