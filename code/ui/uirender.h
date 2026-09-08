/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <RmlUi/Core/RenderInterface.h>


// Draws RmlUi geometry through bgfx into the overlay view. Init needs the renderer running;
// Shutdown comes after Rml::Shutdown, which releases every texture and geometry through
// this object, and before the renderer stops. bgfx handles are kept as their indices so
// that no bgfx type appears here.
class UIRenderInterfaceClass : public Rml::RenderInterface
{
	public:
		UIRenderInterfaceClass(void);

		bool Init(void);
		void Shutdown(void);

		// Points the overlay view at the frame's destination rectangle, in window pixels.
		void Begin_Frame(int x, int y, int width, int height);

		// Writes the renderer's live texture and buffer counts to the debug log.
		void Log_Resource_Counts(char const * when) const;

		virtual Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
		virtual void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
		virtual void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

		virtual Rml::TextureHandle LoadTexture(Rml::Vector2i & dimensions, Rml::String const & source) override;
		virtual Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i dimensions) override;
		virtual void ReleaseTexture(Rml::TextureHandle texture) override;

		virtual void EnableScissorRegion(bool enable) override;
		virtual void SetScissorRegion(Rml::Rectanglei region) override;

	private:
		bool Apply_Scissor(void) const;

		bool IsReady;
		unsigned short Program;
		unsigned short Sampler;
		unsigned short WhiteTexture;

		int ViewX;
		int ViewY;
		int ViewWidth;
		int ViewHeight;

		bool ScissorEnabled;
		Rml::Rectanglei Scissor;
};
