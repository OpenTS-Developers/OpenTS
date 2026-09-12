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

struct ImDrawData;
struct ImTextureData;


// The renderer the shell draws the documents and the developer overlays with. The engine's
// draws through bgfx; a test supplies one that records what it is asked.
class UIRmlRenderClass : public Rml::RenderInterface
{
	public:
		virtual ~UIRmlRenderClass(void) = default;

		virtual bool Init(void) = 0;
		virtual void Shutdown(void) = 0;

		// Points the document view at the frame's destination rectangle, in window pixels.
		virtual void Begin_Frame(int x, int y, int width, int height) = 0;

		// Points the developer view at the same rectangle.
		virtual void Begin_Dev_Frame(int x, int y, int width, int height) = 0;

		// Draws one Dear ImGui frame into the developer view, creating, updating and
		// destroying its textures as it asks.
		virtual void Render_ImGui(ImDrawData * data) = 0;

		// Destroys every texture Dear ImGui still holds. Called before its context goes.
		virtual void Destroy_ImGui_Textures(void) = 0;

		// The largest texture edge the renderer accepts.
		virtual int Texture_Limit(void) const = 0;

		// Writes the renderer's live texture and buffer counts to the debug log.
		virtual void Log_Resource_Counts(char const * when) const = 0;
};


// Draws RmlUi geometry and Dear ImGui frames through bgfx into the overlay views. Init
// needs the renderer running; Shutdown comes after Rml::Shutdown, which releases every
// texture and geometry through this object, and before the renderer stops. bgfx handles
// are kept as their indices so that no bgfx type appears here.
class UIRmlBgfxRenderClass : public UIRmlRenderClass
{
	public:
		UIRmlBgfxRenderClass(void);

		virtual bool Init(void) override;
		virtual void Shutdown(void) override;
		virtual void Begin_Frame(int x, int y, int width, int height) override;
		virtual void Begin_Dev_Frame(int x, int y, int width, int height) override;
		virtual void Render_ImGui(ImDrawData * data) override;
		virtual void Destroy_ImGui_Textures(void) override;
		virtual int Texture_Limit(void) const override;
		virtual void Log_Resource_Counts(char const * when) const override;

		virtual Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
		virtual void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
		virtual void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

		virtual Rml::TextureHandle LoadTexture(Rml::Vector2i & dimensions, Rml::String const & source) override;
		virtual Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i dimensions) override;
		virtual void ReleaseTexture(Rml::TextureHandle texture) override;

		virtual void EnableScissorRegion(bool enable) override;
		virtual void SetScissorRegion(Rml::Rectanglei region) override;

	private:
		void Set_View(unsigned short view, int x, int y, int width, int height);
		bool Apply_Scissor(void) const;
		void Update_ImGui_Texture(ImTextureData * texture);

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

		bool DevShortageLogged;
};
