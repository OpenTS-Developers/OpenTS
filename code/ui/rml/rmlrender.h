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

#include <cstdint>
#include <unordered_map>

struct ImDrawData;
struct ImTextureData;


// What the renderer holds for the documents and the overlays, and how often it drew in
// the frame under way.
struct UIRenderStats
{
	unsigned int GeometryCount = 0;
	unsigned int TextureCount = 0;
	unsigned int DrawCalls = 0;
	std::uint64_t GeometryBytes = 0;
	std::uint64_t TextureBytes = 0;
};


// The renderer the shell draws the documents and the developer overlays with. The engine's
// draws through bgfx; a test supplies one that records what it is asked. A request the
// renderer refuses is latched here with its reason, so the shell can tell a document that
// could not be drawn whole from one that could.
class UIRmlRenderClass : public Rml::RenderInterface
{
	public:
		virtual ~UIRmlRenderClass(void) = default;

		virtual bool Init(void) = 0;
		virtual void Shutdown(void) = 0;

		// Points the document view at the frame's destination rectangle, in window pixels,
		// and starts the frame's draw count.
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

		// Writes what the renderer holds to the debug log.
		virtual void Log_Resource_Counts(char const * when) const = 0;

		// The first refusal since the last clear, empty when there was none. The shell
		// clears it before preparing a document and reads it after.
		char const * Error(void) const { return(ErrorText); }
		void Clear_Error(void) { ErrorText[0] = '\0'; }
		UIRenderStats const & Stats(void) const { return(Statistics); }

		// The render effects outside the styling profile the documents keep to. A document
		// that reaches one draws without it and the refusal is latched.
		virtual void EnableClipMask(bool enable) override;
		virtual void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) override;
		virtual void SetTransform(Rml::Matrix4f const * transform) override;
		virtual Rml::LayerHandle PushLayer(void) override;
		virtual void CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination, Rml::BlendMode mode, Rml::Span<const Rml::CompiledFilterHandle> filters) override;
		virtual void PopLayer(void) override;
		virtual Rml::TextureHandle SaveLayerAsTexture(void) override;
		virtual Rml::CompiledFilterHandle SaveLayerAsMaskImage(void) override;
		virtual Rml::CompiledFilterHandle CompileFilter(Rml::String const & name, Rml::Dictionary const & parameters) override;
		virtual Rml::CompiledShaderHandle CompileShader(Rml::String const & name, Rml::Dictionary const & parameters) override;
		virtual void RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;

	protected:
		// Latches the first refusal, reports it once and returns false.
		bool Fail(char const * message);
		virtual void Report(char const * message);

		UIRenderStats Statistics;

	private:
		char ErrorText[512] = {};
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

	protected:
		virtual void Report(char const * message) override;

	private:
		void Set_View(unsigned short view, int x, int y, int width, int height);
		bool Apply_Scissor(void) const;
		bool Draw_Available(void);
		bool Record_Texture(unsigned short index, unsigned int bytes);
		void Forget_Texture(unsigned short index);
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

		// The bytes each live texture holds, by bgfx index, for the documents' and the
		// overlays' textures alike.
		std::unordered_map<unsigned short, unsigned int> TextureBytes;

		bool DevShortageLogged;
};
