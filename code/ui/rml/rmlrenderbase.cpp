/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The part of the renderer that needs no bgfx: the refusal latch and the render effects
// outside the styling profile. The harness builds it under its recording renderer.

#include "ui/rml/rmlrender.h"

#include <cstdio>


bool UIRmlRenderClass::Fail(char const * message)
{
	if (ErrorText[0] == '\0') {
		std::snprintf(ErrorText, sizeof(ErrorText), "%s", message);
		Report(message);
	}
	return(false);
}


void UIRmlRenderClass::Report(char const *)
{
}


void UIRmlRenderClass::EnableClipMask(bool enable)
{
	if (enable) {
		Fail("clip masks are outside the supported styling profile");
	}
}


void UIRmlRenderClass::RenderToClipMask(Rml::ClipMaskOperation, Rml::CompiledGeometryHandle, Rml::Vector2f)
{
	Fail("clip masks are outside the supported styling profile");
}


void UIRmlRenderClass::SetTransform(Rml::Matrix4f const * transform)
{
	if (transform != nullptr) {
		Fail("transforms are outside the supported styling profile");
	}
}


Rml::LayerHandle UIRmlRenderClass::PushLayer(void)
{
	Fail("layers are outside the supported styling profile");
	return(0);
}


void UIRmlRenderClass::CompositeLayers(Rml::LayerHandle, Rml::LayerHandle, Rml::BlendMode, Rml::Span<const Rml::CompiledFilterHandle>)
{
	Fail("layers are outside the supported styling profile");
}


void UIRmlRenderClass::PopLayer(void)
{
	Fail("layers are outside the supported styling profile");
}


Rml::TextureHandle UIRmlRenderClass::SaveLayerAsTexture(void)
{
	Fail("layers are outside the supported styling profile");
	return(0);
}


Rml::CompiledFilterHandle UIRmlRenderClass::SaveLayerAsMaskImage(void)
{
	Fail("layers are outside the supported styling profile");
	return(0);
}


Rml::CompiledFilterHandle UIRmlRenderClass::CompileFilter(Rml::String const &, Rml::Dictionary const &)
{
	Fail("filters are outside the supported styling profile");
	return(0);
}


Rml::CompiledShaderHandle UIRmlRenderClass::CompileShader(Rml::String const &, Rml::Dictionary const &)
{
	Fail("shaders are outside the supported styling profile");
	return(0);
}


void UIRmlRenderClass::RenderShader(Rml::CompiledShaderHandle, Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle)
{
	Fail("shaders are outside the supported styling profile");
}
