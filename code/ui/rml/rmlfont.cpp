/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlfont.h"

#include "ui/rml/rmlrendermath.h"
#include "utf8.h"

#include <RmlUi/Core/CallbackTexture.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/MeshUtilities.h>
#include <RmlUi/Core/RenderManager.h>
#include <RmlUi/Core/StringUtilities.h>

#include <span>
#include <unordered_map>


// One size of one bitmap family. The sheets have a size of their own, so a document asking
// for another gets them magnified rather than redrawn, and an atlas is baked for each color
// the documents ask for, because the glyphs are shaded and cannot be tinted.
class UISheetFaceClass
{
	public:
		UISheetFaceClass(UIImageIndexed const & index, UIImageIndexed const & alpha, UISheetFontMetrics const & metrics, float scale, int magnification) :
			Index(index),
			Alpha(alpha),
			Sheet(metrics),
			Scale(scale),
			Magnification(magnification < 1 ? 1 : magnification)
		{
			Measurements.size = (int)(Sheet.GlyphHeight * scale);
			Measurements.ascent = Sheet.GlyphHeight * scale;
			Measurements.descent = 0.0f;
			Measurements.line_spacing = Sheet.Cell_Height() * scale;
			Measurements.x_height = Sheet.GlyphHeight * scale * 0.5f;
			Measurements.underline_position = 0.0f;
			Measurements.underline_thickness = 1.0f;
			Measurements.has_ellipsis = false;
		}

		Rml::FontMetrics const & Metrics(void) const { return(Measurements); }

		void Set_Magnification(int magnification)
		{
			magnification = magnification < 1 ? 1 : magnification;
			if (magnification != Magnification) {
				Magnification = magnification;
				Atlases.clear();
			}
		}

		int String_Width(Rml::StringView string) const
		{
			int width = 0;
			for (Rml::StringIteratorU8 character(string); character; ++character) {
				width += (int)(Sheet.Advance[Glyph(*character)] * Scale);
			}
			return(width);
		}

		int Generate(Rml::RenderManager & manager, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied colour, Rml::TexturedMeshList & meshes)
		{
			meshes.resize(1);
			meshes[0].texture = Atlas(manager, colour);

			Rml::Mesh & mesh = meshes[0].mesh;
			Rml::Vector2f sheet((float)Index.Width, (float)Index.Height);
			Rml::Vector2f cell(Sheet.Cell_Width() * Scale, Sheet.Cell_Height() * Scale);

			// The atlas already carries the color, so the vertices carry only how opaque
			// the text is.
			Rml::ColourbPremultiplied opacity(colour.alpha, colour.alpha, colour.alpha, colour.alpha);

			// The pen starts one pixel left of the text, as the dialog layer starts it, and
			// a cell hangs above the baseline by its glyph and the blank row over it.
			float pen = position.x - Scale;
			float top = position.y - (Sheet.GlyphHeight + Sheet.TopMargin) * Scale;
			int width = 0;

			for (Rml::StringIteratorU8 character(string); character; ++character) {
				int glyph = Glyph(*character);
				int x = 0;
				int y = 0;

				if (UI_Sheet_Font_Cell(Sheet, Alpha, glyph, x, y)) {
					Rml::Vector2f topleft((float)x / sheet.x, (float)y / sheet.y);
					Rml::Vector2f bottomright((float)(x + Sheet.Cell_Width()) / sheet.x, (float)(y + Sheet.Cell_Height()) / sheet.y);
					Rml::MeshUtilities::GenerateQuad(mesh, Rml::Vector2f(pen, top), cell, opacity, topleft, bottomright);
				}

				int advance = (int)(Sheet.Advance[glyph] * Scale);
				pen += (float)advance;
				width += advance;
			}

			return(width);
		}

	private:
		static int Glyph(Rml::Character character)
		{
			char32_t code = (char32_t)character;
			if (code < U' ') {
				return((int)code);
			}
			int index = UTF8::Windows_1252_Glyph(code);
			return(index < 0 ? '?' : index);
		}

		// An atlas per color, because a shaded glyph cannot be tinted. The dialogs use a
		// handful of colors, so this never grows far.
		Rml::Texture Atlas(Rml::RenderManager & manager, Rml::ColourbPremultiplied colour)
		{
			// RmlUi hands the color with the opacity already multiplied in; the atlas wants
			// the color the document asked for.
			int alpha = colour.alpha;
			std::uint8_t red = (std::uint8_t)(alpha > 0 ? std::min(255, colour.red * 255 / alpha) : 255);
			std::uint8_t green = (std::uint8_t)(alpha > 0 ? std::min(255, colour.green * 255 / alpha) : 255);
			std::uint8_t blue = (std::uint8_t)(alpha > 0 ? std::min(255, colour.blue * 255 / alpha) : 255);

			std::uint32_t key = ((std::uint32_t)red << 16) | ((std::uint32_t)green << 8) | blue;
			std::unordered_map<std::uint32_t, Rml::CallbackTexture>::iterator found = Atlases.find(key);
			if (found != Atlases.end()) {
				return(found->second);
			}

			std::uint8_t remapped[768];
			UI_Sheet_Font_Remap(Index.Palette, red, green, blue, remapped);

			std::vector<std::uint8_t> rgba;
			if (!UI_Sheet_Font_Atlas(Index, Alpha, remapped, rgba)) {
				return(Rml::Texture());
			}

			// The atlas is kept larger than the sheets while the glyph quads keep the sheets'
			// measure, so the sampler sees whole pixels however the frame is scaled.
			Rml::Vector2i dimensions(Index.Width, Index.Height);
			if (Magnification > 1) {
				std::vector<std::uint8_t> magnified;
				if (!UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(rgba.data(), rgba.size()), Index.Width, Index.Height, Magnification, magnified)) {
					return(Rml::Texture());
				}
				rgba.swap(magnified);
				dimensions = Rml::Vector2i(Index.Width * Magnification, Index.Height * Magnification);
			}

			Rml::CallbackTexture texture = manager.MakeCallbackTexture([rgba, dimensions](Rml::CallbackTextureInterface const & interface) {
				return(interface.GenerateTexture(Rml::Span<const Rml::byte>(rgba.data(), rgba.size()), dimensions));
			});

			return(Atlases.emplace(key, std::move(texture)).first->second);
		}

		UIImageIndexed Index;
		UIImageIndexed Alpha;
		UISheetFontMetrics Sheet;
		float Scale;
		int Magnification;
		Rml::FontMetrics Measurements;
		std::unordered_map<std::uint32_t, Rml::CallbackTexture> Atlases;
};


UIFontEngineClass::UIFontEngineClass(void) :
	Fallback(nullptr)
{
}


UIFontEngineClass::~UIFontEngineClass(void) = default;


bool UIFontEngineClass::Load_Sheets(Rml::String const & family, UIImageIndexed const & index, UIImageIndexed const & alpha)
{
	UISheetFontMetrics metrics;
	if (!UI_Sheet_Font_Metrics(alpha, metrics) || index.Width != alpha.Width || index.Height != alpha.Height) {
		return(false);
	}

	std::unique_ptr<SheetFamily> sheets = std::make_unique<SheetFamily>();
	sheets->Name = Rml::StringUtilities::ToLower(family);
	sheets->Index = index;
	sheets->Alpha = alpha;
	sheets->Metrics = metrics;

	for (std::size_t entry = 0; entry < Families.size(); entry++) {
		if (Families[entry]->Name == sheets->Name) {
			Families[entry] = std::move(sheets);
			return(true);
		}
	}

	Families.push_back(std::move(sheets));
	return(true);
}


bool UIFontEngineClass::Has_Family(Rml::String const & family) const
{
	return(Find_Family(family) != nullptr);
}


UIFontEngineClass::SheetFamily const * UIFontEngineClass::Find_Family(Rml::String const & family) const
{
	Rml::String wanted = Rml::StringUtilities::ToLower(family);

	for (std::unique_ptr<SheetFamily> const & sheets : Families) {
		if (sheets->Name == wanted) {
			return(sheets.get());
		}
	}

	return(nullptr);
}


UISheetFaceClass * UIFontEngineClass::Find_Face(Rml::FontFaceHandle handle) const
{
	for (std::unique_ptr<UISheetFaceClass> const & face : Faces) {
		if ((Rml::FontFaceHandle)face.get() == handle) {
			return(face.get());
		}
	}

	return(nullptr);
}


void UIFontEngineClass::Set_Magnification(int factor)
{
	Magnification = factor < 1 ? 1 : factor;
	for (std::unique_ptr<UISheetFaceClass> const & face : Faces) {
		face->Set_Magnification(Magnification);
	}
}


void UIFontEngineClass::Initialize(void)
{
}


// RmlUi calls this on whichever engine is installed, so the one it made is shut down from
// here or not at all. Its faces go first, because their atlases belong to a render manager
// RmlUi is about to release.
void UIFontEngineClass::Shutdown(void)
{
	Faces.clear();

	if (Fallback != nullptr) {
		Fallback->Shutdown();
		Fallback = nullptr;
	}
}


bool UIFontEngineClass::LoadFontFace(Rml::String const & filename, int faceindex, bool fallbackface, Rml::Style::FontWeight weight)
{
	return(Fallback != nullptr && Fallback->LoadFontFace(filename, faceindex, fallbackface, weight));
}


bool UIFontEngineClass::LoadFontFace(Rml::String const & filename, int faceindex, Rml::String const & family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallbackface)
{
	return(Fallback != nullptr && Fallback->LoadFontFace(filename, faceindex, family, style, weight, fallbackface));
}


bool UIFontEngineClass::LoadFontFace(Rml::Span<const Rml::byte> data, int faceindex, Rml::String const & family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallbackface)
{
	return(Fallback != nullptr && Fallback->LoadFontFace(data, faceindex, family, style, weight, fallbackface));
}


Rml::FontFaceHandle UIFontEngineClass::GetFontFaceHandle(Rml::String const & family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size)
{
	SheetFamily const * sheets = Find_Family(family);
	if (sheets == nullptr) {
		return(Fallback != nullptr ? Fallback->GetFontFaceHandle(family, style, weight, size) : 0);
	}

	float scale = (float)size / (float)UI_SHEET_FONT_NOMINAL_SIZE;
	if (scale <= 0.0f) {
		scale = 1.0f;
	}

	for (std::unique_ptr<UISheetFaceClass> const & face : Faces) {
		if (face->Metrics().size == (int)(sheets->Metrics.GlyphHeight * scale)) {
			return((Rml::FontFaceHandle)face.get());
		}
	}

	Faces.push_back(std::make_unique<UISheetFaceClass>(sheets->Index, sheets->Alpha, sheets->Metrics, scale, Magnification));
	return((Rml::FontFaceHandle)Faces.back().get());
}


Rml::FontEffectsHandle UIFontEngineClass::PrepareFontEffects(Rml::FontFaceHandle handle, Rml::FontEffectList const & effects)
{
	if (Find_Face(handle) != nullptr) {
		return(0);
	}
	return(Fallback != nullptr ? Fallback->PrepareFontEffects(handle, effects) : 0);
}


Rml::FontMetrics const & UIFontEngineClass::GetFontMetrics(Rml::FontFaceHandle handle)
{
	UISheetFaceClass * face = Find_Face(handle);
	if (face != nullptr) {
		return(face->Metrics());
	}
	return(Fallback->GetFontMetrics(handle));
}


int UIFontEngineClass::GetStringWidth(Rml::FontFaceHandle handle, Rml::StringView string, Rml::TextShapingContext const & shaping, Rml::Character prior)
{
	UISheetFaceClass * face = Find_Face(handle);
	if (face != nullptr) {
		return(face->String_Width(string));
	}
	return(Fallback != nullptr ? Fallback->GetStringWidth(handle, string, shaping, prior) : 0);
}


int UIFontEngineClass::GenerateString(Rml::RenderManager & manager, Rml::FontFaceHandle handle, Rml::FontEffectsHandle effects, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied colour, float opacity, Rml::TextShapingContext const & shaping, Rml::TexturedMeshList & meshes)
{
	UISheetFaceClass * face = Find_Face(handle);
	if (face != nullptr) {
		return(face->Generate(manager, string, position, colour, meshes));
	}
	return(Fallback != nullptr ? Fallback->GenerateString(manager, handle, effects, string, position, colour, opacity, shaping, meshes) : 0);
}


int UIFontEngineClass::GetVersion(Rml::FontFaceHandle handle)
{
	if (Find_Face(handle) != nullptr) {
		return(0);
	}
	return(Fallback != nullptr ? Fallback->GetVersion(handle) : 0);
}


void UIFontEngineClass::ReleaseFontResources(void)
{
	Faces.clear();

	if (Fallback != nullptr) {
		Fallback->ReleaseFontResources();
	}
}
