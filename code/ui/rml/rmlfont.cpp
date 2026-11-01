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
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/FontEffect.h>
#include <RmlUi/Core/FontEffectInstancer.h>
#include <RmlUi/Core/FontGlyph.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/MeshUtilities.h>
#include <RmlUi/Core/PropertyDefinition.h>
#include <RmlUi/Core/RenderManager.h>
#include <RmlUi/Core/StringUtilities.h>
#include <algorithm>
#include <cmath>
#include <map>
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

		int Generate(Rml::RenderManager & manager, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied color, Rml::TexturedMeshList & meshes, std::size_t layer = 0)
		{
			if (meshes.size() <= layer) {
				meshes.resize(layer + 1);
			}
			meshes[layer].texture = Atlas(manager, color);

			Rml::Mesh & mesh = meshes[layer].mesh;
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

		int Generate(Rml::RenderManager & manager, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied color, Rml::TexturedMeshList & meshes, std::size_t layer = 0)
		{
			if (meshes.size() <= layer) {
				meshes.resize(layer + 1);
			}
			meshes[layer].texture = Atlas(manager);

			Rml::Mesh & mesh = meshes[layer].mesh;
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


class UIFntFaceClass
{
	public:
		UIFntFaceClass(void const * family, UIFntFont const & font, std::vector<UIFntCell> const & cells, int atlaswidth, int atlasheight, int advance, int lineheight, float scale, int magnification) :
			Family(family),
			Font(font),
			Cells(cells),
			AtlasWidth(atlaswidth),
			AtlasHeight(atlasheight),
			Spacing(advance),
			Scale(scale),
			Magnification(magnification < 1 ? 1 : magnification)
		{
			Measurements.size = (int)(Font.Height * scale);
			Measurements.ascent = Font.Height * scale;
			Measurements.descent = 0.0f;
			Measurements.line_spacing = lineheight * scale;
			Measurements.x_height = Font.Height * scale * 0.5f;
			Measurements.underline_position = 0.0f;
			Measurements.underline_thickness = 1.0f;
			Measurements.has_ellipsis = false;
		}

		Rml::FontMetrics const & Metrics(void) const { return(Measurements); }
		void const * Owner(void) const { return(Family); }

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
				width += Advance(*character) * Scale;
			}
			return((int)(width + 0.5f));
		}

		int Generate(Rml::RenderManager & manager, Rml::StringView string, Rml::Vector2f position, UIFntPalette const & palette, Rml::ColourbPremultiplied opacity, Rml::TexturedMeshList & meshes, std::size_t layer)
		{
			if (meshes.size() <= layer) {
				meshes.resize(layer + 1);
			}
			meshes[layer].texture = Atlas(manager, palette);

			Rml::Mesh & mesh = meshes[layer].mesh;
			float const atlas = (float)AtlasWidth;
			float const rows = (float)AtlasHeight;

			float pen = position.x;
			float const top = position.y - Font.Height * Scale;
			float width = 0.0f;

			for (Rml::StringIteratorU8 character(string); character; ++character) {
				int const glyph = UI_Fnt_Glyph(Font, (char32_t)*character);
				if (glyph < 0) {
					continue;
				}

				UIFntGlyph const & picture = Font.Glyphs[(std::size_t)glyph];
				if (picture.Width > 0) {
					float const x = (float)Cells[(std::size_t)glyph].X;
					float const y = (float)Cells[(std::size_t)glyph].Y;
					Rml::Vector2f topleft(x / atlas, y / rows);
					Rml::Vector2f bottomright((x + picture.Width) / atlas, (y + Font.Height) / rows);
					Rml::Vector2f size(picture.Width * Scale, Font.Height * Scale);
					Rml::MeshUtilities::GenerateQuad(mesh, Rml::Vector2f(pen, top), size, opacity, topleft, bottomright);
				}

				float const advance = (picture.Width + Spacing) * Scale;
				pen += advance;
				width += advance;
			}

			return((int)(width + 0.5f));
		}

	private:
		int Advance(Rml::Character character) const
		{
			int const glyph = UI_Fnt_Glyph(Font, (char32_t)character);
			return((glyph < 0) ? 0 : Font.Glyphs[(std::size_t)glyph].Width + Spacing);
		}

		Rml::Texture Atlas(Rml::RenderManager & manager, UIFntPalette const & palette)
		{
			std::map<UIFntPalette, Rml::CallbackTexture>::iterator found = Atlases.find(palette);
			if (found != Atlases.end()) {
				return(found->second);
			}

			std::vector<std::uint8_t> rgba;
			if (!UI_Fnt_Atlas(Font, Cells, AtlasWidth, AtlasHeight, palette, rgba)) {
				return(Rml::Texture());
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

			Rml::CallbackTexture texture = manager.MakeCallbackTexture([rgba, dimensions](Rml::CallbackTextureInterface const & interface) {
				return(interface.GenerateTexture(Rml::Span<const Rml::byte>(rgba.data(), rgba.size()), dimensions));
			});
			return(Atlases.emplace(palette, std::move(texture)).first->second);
		}

		void const * Family;
		UIFntFont const & Font;
		std::vector<UIFntCell> const & Cells;
		int AtlasWidth;
		int AtlasHeight;
		int Spacing;
		float Scale;
		int Magnification;
		Rml::FontMetrics Measurements;
		std::map<UIFntPalette, Rml::CallbackTexture> Atlases;
};


// Colors one range of a Westwood font's pixel values; only a .FNT face draws it.
class UIFntRoleEffectClass : public Rml::FontEffect
{
	public:
		UIFntRoleEffectClass(int first, int last) : First(first), Last(last) {}

		int First;
		int Last;
};


class UIFntRoleInstancerClass : public Rml::FontEffectInstancer
{
	public:
		UIFntRoleInstancerClass(int first, int last) :
			First(first),
			Last(last),
			IndexId(Rml::PropertyId::Invalid),
			ColorId(Rml::PropertyId::Invalid)
		{
			if (first == 0) {
				IndexId = RegisterProperty("index", "2").AddParser("number").GetId();
			}
			ColorId = RegisterProperty("color", "black").AddParser("color").GetId();
			RegisterShorthand("font-effect", (first == 0) ? "index, color" : "color", Rml::ShorthandType::FallThrough);
		}

		virtual Rml::SharedPtr<Rml::FontEffect> InstanceFontEffect(Rml::String const & name, Rml::PropertyDictionary const & properties) override
		{
			int first = First;
			int last = Last;
			if (first == 0) {
				first = properties.GetProperty(IndexId)->Get<int>();
				last = first;
				if (first < UI_FNT_SHADOW || first >= UI_FNT_ROLES) {
					return(nullptr);
				}
			}

			Rml::SharedPtr<UIFntRoleEffectClass> effect = Rml::MakeShared<UIFntRoleEffectClass>(first, last);
			effect->SetColour(properties.GetProperty(ColorId)->Get<Rml::Colourb>());
			return(effect);
		}

	private:
		int First;
		int Last;
		Rml::PropertyId IndexId;
		Rml::PropertyId ColorId;
};


/// <summary>
/// Registers the font effects that color a .FNT face's pixel values: ww-shadow(color) for value
/// 2, ww-outline(color) for value 3, ww-gradient(color) for values 4 to 15, and
/// ww-role(index color) for any one value from 2 to 15. A later effect overrides an earlier one
/// for the values both name, and every other face draws its text without them.
/// </summary>
void UI_Register_Fnt_Effects(void)
{
	static UIFntRoleInstancerClass role(0, 0);
	static UIFntRoleInstancerClass shadow(UI_FNT_SHADOW, UI_FNT_SHADOW);
	static UIFntRoleInstancerClass outline(UI_FNT_OUTLINE, UI_FNT_OUTLINE);
	static UIFntRoleInstancerClass gradient(UI_FNT_FIRST_SHADE, UI_FNT_ROLES - 1);

	Rml::Factory::RegisterFontEffectInstancer("ww-role", &role);
	Rml::Factory::RegisterFontEffectInstancer("ww-shadow", &shadow);
	Rml::Factory::RegisterFontEffectInstancer("ww-outline", &outline);
	Rml::Factory::RegisterFontEffectInstancer("ww-gradient", &gradient);
}


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


/// <summary>
/// Adds or replaces a family drawn from a Westwood .FNT font. A font-size equal to the font's
/// height in dp draws one font pixel per dp.
/// </summary>
/// <param name="spacing">The horizontal spacing override the game gives this font.</param>
/// <param name="outlined">Was the font loaded as outlined, which narrows its spacing?</param>
/// <param name="gradient">Does the game print shade values 4 to 15 in the text color?</param>
/// <returns>bool; Was the family added? A font whose glyphs do not fit one atlas is refused.</returns>
bool UIFontEngineClass::Load_Fnt(Rml::String const & family, UIFntFont const & font, int spacing, bool outlined, bool gradient)
{
	std::unique_ptr<FntFamily> fnt = std::make_unique<FntFamily>();
	fnt->Name = Rml::StringUtilities::ToLower(family);
	fnt->Font = font;
	fnt->Gradient = gradient;
	fnt->Advance = UI_Fnt_Spacing(font.MaxWidth, spacing, outlined);
	fnt->LineHeight = font.Height + std::max(UI_Fnt_Spacing(font.Height, 0, outlined) + font.MaxWidth / 16, 0);
	if (!UI_Fnt_Layout(fnt->Font, UI_RASTER_ATLAS_LIMIT, fnt->Cells, fnt->AtlasWidth, fnt->AtlasHeight)) {
		return(false);
	}

	for (std::size_t entry = 0; entry < FntFamilies.size(); entry++) {
		if (FntFamilies[entry]->Name == fnt->Name) {
			std::erase_if(FntFaces, [&](std::unique_ptr<UIFntFaceClass> const & face) { return(face->Owner() == FntFamilies[entry].get()); });
			FntFamilies[entry] = std::move(fnt);
			return(true);
		}
	}

	FntFamilies.push_back(std::move(fnt));
	return(true);
}


bool UIFontEngineClass::Has_Family(Rml::String const & family) const
{
	return(Find_Family(family) != nullptr || Find_Raster_Family(family) != nullptr || Find_Fnt_Family(family) != nullptr);
}


UIFontEngineClass::FntFamily const * UIFontEngineClass::Find_Fnt_Family(Rml::String const & family) const
{
	Rml::String wanted = Rml::StringUtilities::ToLower(family);

	for (std::unique_ptr<FntFamily> const & fnt : FntFamilies) {
		if (fnt->Name == wanted) {
			return(fnt.get());
		}
	}

	return(nullptr);
}


UIFntFaceClass * UIFontEngineClass::Find_Fnt_Face(Rml::FontFaceHandle handle) const
{
	for (std::unique_ptr<UIFntFaceClass> const & face : FntFaces) {
		if ((Rml::FontFaceHandle)face.get() == handle) {
			return(face.get());
		}
	}

	return(nullptr);
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
	for (std::unique_ptr<UIFntFaceClass> const & face : FntFaces) {
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
	FntFaces.clear();

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

	FntFamily const * fnt = Find_Fnt_Family(family);
	if (fnt != nullptr) {
		float const scale = (fnt->Font.Height > 0 && size > 0) ? (float)size / (float)fnt->Font.Height : 1.0f;
		for (std::unique_ptr<UIFntFaceClass> const & face : FntFaces) {
			if (face->Owner() == fnt && face->Metrics().size == (int)(fnt->Font.Height * scale)) {
				return((Rml::FontFaceHandle)face.get());
			}
		}

		FntFaces.push_back(std::make_unique<UIFntFaceClass>(fnt, fnt->Font, fnt->Cells, fnt->AtlasWidth, fnt->AtlasHeight, fnt->Advance, fnt->LineHeight, scale, Magnification));
		return((Rml::FontFaceHandle)FntFaces.back().get());
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


/// <summary>
/// Prepares the effects a game face can draw. A shadow, which moves the glyphs and recolors
/// them, is drawn on every face; an effect that builds glyphs of its own, such as an outline,
/// glow or blur, is drawn only by a TrueType face and leaves a game face's text plain. The
/// ww- effects recolor a .FNT face's pixel values and leave every other face unchanged.
/// </summary>
Rml::FontEffectsHandle UIFontEngineClass::PrepareFontEffects(Rml::FontFaceHandle handle, Rml::FontEffectList const & effects)
{
	if (Find_Face(handle) == nullptr && Find_Raster_Face(handle) == nullptr && Find_Fnt_Face(handle) == nullptr) {
		return(Fallback != nullptr ? Fallback->PrepareFontEffects(handle, effects) : 0);
	}

	EffectSet set;
	bool roles = false;
	for (Rml::SharedPtr<const Rml::FontEffect> const & effect : effects) {
		UIFntRoleEffectClass const * role = dynamic_cast<UIFntRoleEffectClass const *>(effect.get());
		if (role != nullptr) {
			for (int value = role->First; value <= role->Last; value++) {
				set.RoleSet[(std::size_t)value] = true;
				set.Roles[(std::size_t)value] = role->GetColour();
			}
			set.Fingerprints.push_back(role->GetFingerprint());
			roles = true;
			continue;
		}

		Rml::FontGlyph glyph;
		Rml::Vector2i origin(0, 0);
		Rml::Vector2i dimensions(1, 1);
		if (effect == nullptr || effect->HasUniqueTexture() || !effect->GetGlyphMetrics(origin, dimensions, glyph)) {
			continue;
		}

		Shadow shadow;
		shadow.Offset = Rml::Vector2f((float)origin.x, (float)origin.y);
		shadow.Colour = effect->GetColour();
		(effect->GetLayer() == Rml::FontEffect::Layer::Back ? set.Back : set.Front).push_back(shadow);
		set.Fingerprints.push_back(effect->GetFingerprint());
	}

	if (set.Back.empty() && set.Front.empty() && !roles) {
		return(0);
	}

	for (std::size_t index = 0; index < EffectSets.size(); index++) {
		if (EffectSets[index].Fingerprints == set.Fingerprints) {
			return((Rml::FontEffectsHandle)(index + 1));
		}
	}
	EffectSets.push_back(std::move(set));
	return((Rml::FontEffectsHandle)EffectSets.size());
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
	UIFntFaceClass * fnt = Find_Fnt_Face(handle);
	if (fnt != nullptr) {
		return(fnt->Metrics());
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
	UIFntFaceClass * fnt = Find_Fnt_Face(handle);
	if (fnt != nullptr) {
		return(fnt->String_Width(string));
	}
	return(Fallback != nullptr ? Fallback->GetStringWidth(handle, string, shaping, prior) : 0);
}


int UIFontEngineClass::GenerateString(Rml::RenderManager & manager, Rml::FontFaceHandle handle, Rml::FontEffectsHandle effects, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied color, float opacity, Rml::TextShapingContext const & shaping, Rml::TexturedMeshList & meshes)
{
	UISheetFaceClass * face = Find_Face(handle);
	UIRasterFaceClass * strike = Find_Raster_Face(handle);
	UIFntFaceClass * fnt = Find_Fnt_Face(handle);
	if (face == nullptr && strike == nullptr && fnt == nullptr) {
		return(Fallback != nullptr ? Fallback->GenerateString(manager, handle, effects, string, position, color, opacity, shaping, meshes) : 0);
	}

	EffectSet const * set = (effects > 0 && (std::size_t)effects <= EffectSets.size()) ? &EffectSets[(std::size_t)effects - 1] : nullptr;
	if (fnt != nullptr) {
		return(Generate_Fnt(*fnt, manager, set, string, position, color, opacity, meshes));
	}
	auto draw = [&](Rml::Vector2f at, Rml::ColourbPremultiplied tint, std::size_t layer) {
		return((face != nullptr) ? face->Generate(manager, string, at, tint, meshes, layer) : strike->Generate(manager, string, at, tint, meshes, layer));
	};

	// Each layer keeps its own mesh, so every line of a text adds to the same layers in the same order.
	std::size_t layer = 0;
	if (set != nullptr) {
		for (Shadow const & shadow : set->Back) {
			draw(position + shadow.Offset, shadow.Colour.ToPremultiplied(opacity), layer++);
		}
	}
	int const width = draw(position, color, layer++);
	if (set != nullptr) {
		for (Shadow const & shadow : set->Front) {
			draw(position + shadow.Offset, shadow.Colour.ToPremultiplied(opacity), layer++);
		}
	}
	return(width);
}


/// <summary>
/// Draws a string in a .FNT face. Each pixel value takes its color from the text color, the
/// family's defaults and the ww- effects, and each moved shadow draws every visible pixel in
/// the shadow's color.
/// </summary>
int UIFontEngineClass::Generate_Fnt(UIFntFaceClass & face, Rml::RenderManager & manager, EffectSet const * set, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied color, float opacity, Rml::TexturedMeshList & meshes)
{
	FntFamily const * family = static_cast<FntFamily const *>(face.Owner());
	float const visible = std::clamp(opacity, 0.0f, 1.0f);
	std::uint8_t const fade = (std::uint8_t)(visible * 255.0f + 0.5f);
	Rml::ColourbPremultiplied const tint(fade, fade, fade, fade);

	int const alpha = color.alpha;
	std::uint8_t const red = (std::uint8_t)(alpha > 0 ? std::min(255, color.red * 255 / alpha) : 0);
	std::uint8_t const green = (std::uint8_t)(alpha > 0 ? std::min(255, color.green * 255 / alpha) : 0);
	std::uint8_t const blue = (std::uint8_t)(alpha > 0 ? std::min(255, color.blue * 255 / alpha) : 0);
	std::uint8_t const textalpha = (std::uint8_t)(visible > 0.0f ? std::min(255.0f, alpha / visible + 0.5f) : 0.0f);

	UIFntPalette palette;
	UI_Fnt_Default_Palette(UI_Fnt_Color(red, green, blue, textalpha), family->Gradient, palette);
	if (set != nullptr) {
		for (std::size_t value = UI_FNT_SHADOW; value < UI_FNT_ROLES; value++) {
			if (set->RoleSet[value]) {
				Rml::Colourb const & role = set->Roles[value];
				palette[value] = UI_Fnt_Color(role.red, role.green, role.blue, role.alpha);
			}
		}
	}

	auto silhouette = [&](Rml::Colourb shade) {
		UIFntPalette moved;
		moved.fill(0);
		for (std::size_t value = UI_FNT_TEXT; value < UI_FNT_ROLES; value++) {
			if ((palette[value] & 0xFF) != 0) {
				moved[value] = UI_Fnt_Color(shade.red, shade.green, shade.blue, shade.alpha);
			}
		}
		return(moved);
	};

	std::size_t layer = 0;
	if (set != nullptr) {
		for (Shadow const & shadow : set->Back) {
			face.Generate(manager, string, position + shadow.Offset, silhouette(shadow.Colour), tint, meshes, layer++);
		}
	}
	int const width = face.Generate(manager, string, position, palette, tint, meshes, layer++);
	if (set != nullptr) {
		for (Shadow const & shadow : set->Front) {
			face.Generate(manager, string, position + shadow.Offset, silhouette(shadow.Colour), tint, meshes, layer++);
		}
	}
	return(width);
}


int UIFontEngineClass::GetVersion(Rml::FontFaceHandle handle)
{
	if (Find_Face(handle) != nullptr || Find_Raster_Face(handle) != nullptr || Find_Fnt_Face(handle) != nullptr) {
		return(0);
	}
	return(Fallback != nullptr ? Fallback->GetVersion(handle) : 0);
}


void UIFontEngineClass::ReleaseFontResources(void)
{
	Faces.clear();
	RasterFaces.clear();
	FntFaces.clear();

	if (Fallback != nullptr) {
		Fallback->ReleaseFontResources();
	}
}
