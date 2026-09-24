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


struct UIRenderStats
{
	unsigned int GeometryCount = 0;
	unsigned int TextureCount = 0;
	unsigned int DrawCalls = 0;
	std::uint64_t GeometryBytes = 0;
	std::uint64_t TextureBytes = 0;
};


class UIRmlRenderClass : public Rml::RenderInterface
{
	public:
		virtual ~UIRmlRenderClass(void) = default;

		virtual bool Init(void) = 0;
		virtual void Shutdown(void) = 0;

		virtual void Begin_Frame(int x, int y, int width, int height) = 0;
		virtual void Begin_Dev_Frame(int x, int y, int width, int height) = 0;

		virtual void Render_ImGui(ImDrawData * data) = 0;
		virtual void Destroy_ImGui_Textures(void) = 0;

		virtual int Texture_Limit(void) const = 0;
		virtual void Log_Resource_Counts(char const * when) const = 0;
		virtual void Set_Art_Magnification(int factor) { (void)factor; }

		char const * Error(void) const { return(ErrorText); }
		void Clear_Error(void) { ErrorText[0] = '\0'; }
		UIRenderStats const & Stats(void) const { return(Statistics); }

		virtual Rml::LayerHandle PushLayer(void) override;
		virtual void CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination, Rml::BlendMode mode, Rml::Span<const Rml::CompiledFilterHandle> filters) override;
		virtual void PopLayer(void) override;
		virtual Rml::TextureHandle SaveLayerAsTexture(void) override;
		virtual Rml::CompiledFilterHandle SaveLayerAsMaskImage(void) override;
		virtual Rml::CompiledFilterHandle CompileFilter(Rml::String const & name, Rml::Dictionary const & parameters) override;
		virtual Rml::CompiledShaderHandle CompileShader(Rml::String const & name, Rml::Dictionary const & parameters) override;
		virtual void RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;

	protected:
		bool Fail(char const * message);
		virtual void Report(char const * message);

		UIRenderStats Statistics;

	private:
		char ErrorText[512] = {};
};


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
		virtual void Set_Art_Magnification(int factor) override { ArtMagnification = factor < 1 ? 1 : factor; }

		virtual Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
		virtual void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
		virtual void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
		virtual Rml::TextureHandle LoadTexture(Rml::Vector2i & dimensions, Rml::String const & source) override;
		virtual Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i dimensions) override;
		virtual void ReleaseTexture(Rml::TextureHandle texture) override;
		virtual void EnableScissorRegion(bool enable) override;
		virtual void SetScissorRegion(Rml::Rectanglei region) override;
		virtual void SetTransform(Rml::Matrix4f const * transform) override;
		virtual void EnableClipMask(bool enable) override;
		virtual void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) override;

	protected:
		virtual void Report(char const * message) override;

	private:
		void Set_View(unsigned short view, int x, int y, int width, int height);
		bool Apply_Scissor(void) const;
		bool Draw_Available(void);
		bool Submit_Geometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, unsigned short texture, std::uint64_t state, std::uint32_t stencil);
		bool Clear_Clip_Mask(void);
		bool Record_Texture(unsigned short index, unsigned int bytes);
		void Forget_Texture(unsigned short index);
		void Update_ImGui_Texture(ImTextureData * texture);

		bool IsReady;
		unsigned short Program;
		unsigned short Sampler;
		unsigned short WhiteTexture;

		unsigned short ClearVertices;
		unsigned short ClearIndices;

		int ViewX;
		int ViewY;
		int ViewWidth;
		int ViewHeight;

		bool ScissorEnabled;
		Rml::Rectanglei Scissor;
		int ArtMagnification = 1;

		bool TransformEnabled;
		float Transform[16];

		bool ClipMaskEnabled;
		std::uint8_t StencilReference;

		std::unordered_map<unsigned short, unsigned int> TextureBytes;

		bool DevShortageLogged;
};
