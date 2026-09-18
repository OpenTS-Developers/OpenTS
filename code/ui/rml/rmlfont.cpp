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

#include <algorithm>
#include <cmath>
#include <span>
#include <unordered_map>


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
			float width = 0.0f;
			for (Rml::StringIteratorU8 character(string); character; ++character) {
				width += Sheet.Advance[Glyph(*character)] * Scale;
			}
			return((int)(width + 0.5f));
		}

		int Generate(Rml::RenderManager & manager, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied color, Rml::TexturedMeshList & meshes)
		{
			meshes.resize(1);
			meshes[0].texture = Atlas(manager, color);

			Rml::Mesh & mesh = meshes[0].mesh;
			Rml::Vector2f sheet((float)Index.Width, (float)Index.Height);
			Rml::Vector2f cell(Sheet.Cell_Width() * Scale, Sheet.Cell_Height() * Scale);

			Rml::ColourbPremultiplied opacity(color.alpha, color.alpha, color.alpha, color.alpha);

			float pen = position.x - Scale;
			float top = position.y - (Sheet.GlyphHeight + Sheet.TopMargin) * Scale;
			float width = 0.0f;

			for (Rml::StringIteratorU8 character(string); character; ++character) {
				int glyph = Glyph(*character);
				int x = 0;
				int y = 0;

				if (UI_Sheet_Font_Cell(Sheet, Alpha, glyph, x, y)) {
					Rml::Vector2f topleft((float)x / sheet.x, (float)y / sheet.y);
					Rml::Vector2f bottomright((float)(x + Sheet.Cell_Width()) / sheet.x, (float)(y + Sheet.Cell_Height()) / sheet.y);
					Rml::MeshUtilities::GenerateQuad(mesh, Rml::Vector2f(pen, top), cell, opacity, topleft, bottomright);
				}

				float advance = Sheet.Advance[glyph] * Scale;
				pen += advance;
				width += advance;
			}

			return((int)(width + 0.5f));
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

		Rml::Texture Atlas(Rml::RenderManager & manager, Rml::ColourbPremultiplied color)
		{
			int alpha = color.alpha;
			std::uint8_t red = (std::uint8_t)(alpha > 0 ? std::min(255, color.red * 255 / alpha) : 255);
			std::uint8_t green = (std::uint8_t)(alpha > 0 ? std::min(255, color.green * 255 / alpha) : 255);
			std::uint8_t blue = (std::uint8_t)(alpha > 0 ? std::min(255, color.blue * 255 / alpha) : 255);

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


class UIRasterFaceClass
{
	public:
		UIRasterFaceClass(UIRasterStrike const & strike, float scale, int magnification) :
			Strike(strike),
			Scale(scale),
			Magnification(magnification < 1 ? 1 : magnification)
		{
			UI_Raster_Strike_Layout(Strike, UI_RASTER_ATLAS_LIMIT, Cells, AtlasWidth, AtlasHeight);

			Measurements.size = (int)(Strike.Height * scale);
			Measurements.ascent = Strike.Ascent * scale;
			Measurements.descent = (Strike.Height - Strike.Ascent) * scale;
			Measurements.line_spacing = Strike.Height * scale;
			Measurements.x_height = Strike.Ascent * scale * 0.5f;
			Measurements.underline_position = 0.0f;
			Measurements.underline_thickness = 1.0f;
			Measurements.has_ellipsis = false;
		}

		Rml::FontMetrics const & Metrics(void) const { return(Measurements); }
		int Height(void) const { return(Strike.Height); }

		void Set_Magnification(int magnification)
		{
			magnification = magnification < 1 ? 1 : magnification;
			if (magnification != Magnification) {
				Magnification = magnification;
				Sheet.Release();
			}
		}

		int String_Width(Rml::StringView string) const
		{
			float width = 0.0f;
			for (Rml::StringIteratorU8 character(string); character; ++character) {
				int cell = Index(*character);
				width += ((cell >= 0) ? Cells[cell].Advance : 0) * Scale;
			}
			return((int)(width + 0.5f));
		}

		int Generate(Rml::RenderManager & manager, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied color, Rml::TexturedMeshList & meshes)
		{
			meshes.resize(1);
			meshes[0].texture = Atlas(manager);

			Rml::Mesh & mesh = meshes[0].mesh;
			float atlas = (float)AtlasWidth;
			float rows = (float)AtlasHeight;

			float pen = position.x;
			float top = std::floor(position.y - Strike.Ascent * Scale + 0.5f);
			float width = 0.0f;

			for (Rml::StringIteratorU8 character(string); character; ++character) {
				int cell = Index(*character);
				int advance = (cell >= 0) ? Cells[cell].Advance : 0;

				if (advance > 0 && Cells[cell].Code > U' ') {
					float x = (float)Cells[cell].X;
					float y = (float)Cells[cell].Y;
					Rml::Vector2f topleft(x / atlas, y / rows);
					Rml::Vector2f bottomright((x + advance) / atlas, (y + Strike.Height) / rows);
					Rml::Vector2f size(advance * Scale, Strike.Height * Scale);
					Rml::MeshUtilities::GenerateQuad(mesh, Rml::Vector2f(pen, top), size, color, topleft, bottomright);
				}

				pen += advance * Scale;
				width += advance * Scale;
			}

			return((int)(width + 0.5f));
		}

	private:
		int Find(char32_t code) const
		{
			std::vector<UIRasterCell>::const_iterator at = std::lower_bound(Cells.begin(), Cells.end(), code,
				[](UIRasterCell const & cell, char32_t wanted) { return(cell.Code < wanted); });
			return((at != Cells.end() && at->Code == code) ? (int)(at - Cells.begin()) : -1);
		}

		int Index(Rml::Character character) const
		{
			char32_t code = (char32_t)character;
			if (code < U' ') {
				return(-1);
			}

			int at = Find(code);
			if (at >= 0) {
				return(at);
			}

			int byte = UTF8::Windows_1252_Glyph(code);
			if (byte >= 0) {
				char32_t fitted = UTF8::Windows_Code(1252, (unsigned char)byte);
				if (fitted != 0 && fitted != code) {
					at = Find(fitted);
					if (at >= 0) {
						return(at);
					}
				}
			}
			return(Find(U'?'));
		}

		Rml::Texture Atlas(Rml::RenderManager & manager)
		{
			if (Sheet) {
				return(Sheet);
			}
			if (Cells.size() != Strike.Glyphs.size() || Cells.empty()) {
				return(Rml::Texture());
			}

			std::vector<std::uint8_t> rgba((std::size_t)AtlasWidth * AtlasHeight * 4, 0);

			for (std::size_t index = 0; index < Cells.size(); index++) {
				UIRasterGlyph const & picture = Strike.Glyphs[index];
				UIRasterCell const & cell = Cells[index];

				if (picture.Coverage.size() != (std::size_t)picture.Advance * Strike.Height) {
					continue;
				}

				for (int y = 0; y < Strike.Height; y++) {
					for (int x = 0; x < picture.Advance; x++) {
						std::uint8_t coverage = picture.Coverage[(std::size_t)y * picture.Advance + x];
						std::size_t at = ((std::size_t)(cell.Y + y) * AtlasWidth + cell.X + x) * 4;
						rgba[at] = coverage;
						rgba[at + 1] = coverage;
						rgba[at + 2] = coverage;
						rgba[at + 3] = coverage;
					}
				}
			}

			int factor = Magnification;
			while (factor > 1
				&& (AtlasWidth * factor > UI_RASTER_ATLAS_LIMIT || AtlasHeight * factor > UI_RASTER_ATLAS_LIMIT)) {
				factor--;
			}

			Rml::Vector2i dimensions(AtlasWidth, AtlasHeight);
			if (factor > 1) {
				std::vector<std::uint8_t> magnified;
				if (!UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(rgba.data(), rgba.size()), AtlasWidth, AtlasHeight, factor, magnified)) {
					return(Rml::Texture());
				}
				rgba.swap(magnified);
				dimensions = Rml::Vector2i(AtlasWidth * factor, AtlasHeight * factor);
			}

			Sheet = manager.MakeCallbackTexture([rgba, dimensions](Rml::CallbackTextureInterface const & interface) {
				return(interface.GenerateTexture(Rml::Span<const Rml::byte>(rgba.data(), rgba.size()), dimensions));
			});
			return(Sheet);
		}

		UIRasterStrike Strike;
		float Scale;
		int Magnification;
		std::vector<UIRasterCell> Cells;
		int AtlasWidth = 0;
		int AtlasHeight = 0;
		Rml::FontMetrics Measurements;
		Rml::CallbackTexture Sheet;
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


bool UIFontEngineClass::Load_Strikes(Rml::String const & family, std::vector<UIRasterStrike> const & strikes)
{
	std::unique_ptr<RasterFamily> raster = std::make_unique<RasterFamily>();
	raster->Name = Rml::StringUtilities::ToLower(family);

	for (UIRasterStrike const & strike : strikes) {
		if (strike.Height > 0 && strike.Advance(U'M') > 0) {
			raster->Strikes.push_back(strike);
		}
	}

	if (raster->Strikes.empty()) {
		return(false);
	}

	for (std::size_t entry = 0; entry < RasterFamilies.size(); entry++) {
		if (RasterFamilies[entry]->Name == raster->Name) {
			RasterFamilies[entry] = std::move(raster);
			return(true);
		}
	}

	RasterFamilies.push_back(std::move(raster));
	return(true);
}


bool UIFontEngineClass::Has_Family(Rml::String const & family) const
{
	return(Find_Family(family) != nullptr || Find_Raster_Family(family) != nullptr);
}


UIFontEngineClass::RasterFamily const * UIFontEngineClass::Find_Raster_Family(Rml::String const & family) const
{
	Rml::String wanted = Rml::StringUtilities::ToLower(family);

	for (std::unique_ptr<RasterFamily> const & raster : RasterFamilies) {
		if (raster->Name == wanted) {
			return(raster.get());
		}
	}

	return(nullptr);
}


UIRasterFaceClass * UIFontEngineClass::Find_Raster_Face(Rml::FontFaceHandle handle) const
{
	for (std::unique_ptr<UIRasterFaceClass> const & face : RasterFaces) {
		if ((Rml::FontFaceHandle)face.get() == handle) {
			return(face.get());
		}
	}

	return(nullptr);
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
	for (std::unique_ptr<UIRasterFaceClass> const & face : RasterFaces) {
		face->Set_Magnification(Magnification);
	}
}


void UIFontEngineClass::Initialize(void)
{
}


void UIFontEngineClass::Shutdown(void)
{
	Faces.clear();
	RasterFaces.clear();

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
	RasterFamily const * raster = UseStrikes ? Find_Raster_Family(family) : nullptr;
	if (raster != nullptr) {
		float wanted = (float)size / Reference;
		UIRasterStrike const * chosen = &raster->Strikes[0];
		for (UIRasterStrike const & strike : raster->Strikes) {
			if (std::abs(strike.Height - wanted) < std::abs(chosen->Height - wanted)) {
				chosen = &strike;
			}
		}

		float scale = (float)size / (float)chosen->Height;
		for (std::unique_ptr<UIRasterFaceClass> const & face : RasterFaces) {
			if (face->Height() == chosen->Height && face->Metrics().size == (int)(chosen->Height * scale)) {
				return((Rml::FontFaceHandle)face.get());
			}
		}

		RasterFaces.push_back(std::make_unique<UIRasterFaceClass>(*chosen, scale, Magnification));
		return((Rml::FontFaceHandle)RasterFaces.back().get());
	}

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
	if (Find_Face(handle) != nullptr || Find_Raster_Face(handle) != nullptr) {
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
	UIRasterFaceClass * strike = Find_Raster_Face(handle);
	if (strike != nullptr) {
		return(strike->Metrics());
	}
	return(Fallback->GetFontMetrics(handle));
}


int UIFontEngineClass::GetStringWidth(Rml::FontFaceHandle handle, Rml::StringView string, Rml::TextShapingContext const & shaping, Rml::Character prior)
{
	UISheetFaceClass * face = Find_Face(handle);
	if (face != nullptr) {
		return(face->String_Width(string));
	}
	UIRasterFaceClass * strike = Find_Raster_Face(handle);
	if (strike != nullptr) {
		return(strike->String_Width(string));
	}
	return(Fallback != nullptr ? Fallback->GetStringWidth(handle, string, shaping, prior) : 0);
}


int UIFontEngineClass::GenerateString(Rml::RenderManager & manager, Rml::FontFaceHandle handle, Rml::FontEffectsHandle effects, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied color, float opacity, Rml::TextShapingContext const & shaping, Rml::TexturedMeshList & meshes)
{
	UISheetFaceClass * face = Find_Face(handle);
	if (face != nullptr) {
		return(face->Generate(manager, string, position, color, meshes));
	}
	UIRasterFaceClass * strike = Find_Raster_Face(handle);
	if (strike != nullptr) {
		return(strike->Generate(manager, string, position, color, meshes));
	}
	return(Fallback != nullptr ? Fallback->GenerateString(manager, handle, effects, string, position, color, opacity, shaping, meshes) : 0);
}


int UIFontEngineClass::GetVersion(Rml::FontFaceHandle handle)
{
	if (Find_Face(handle) != nullptr || Find_Raster_Face(handle) != nullptr) {
		return(0);
	}
	return(Fallback != nullptr ? Fallback->GetVersion(handle) : 0);
}


void UIFontEngineClass::ReleaseFontResources(void)
{
	Faces.clear();
	RasterFaces.clear();

	if (Fallback != nullptr) {
		Fallback->ReleaseFontResources();
	}
}
