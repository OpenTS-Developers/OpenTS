/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/rml/rmlfontfon.h"
#include "ui/rml/rmlfontsheet.h"

#include <RmlUi/Core/FontEngineInterface.h>

#include <memory>
#include <vector>


class UISheetFaceClass;
class UIRasterFaceClass;


// The size a document asks for to draw a bitmap family at the size its sheets were drawn.
// A bitmap font has one size of its own, so any other is that size magnified.
static const int UI_SHEET_FONT_NOMINAL_SIZE = 16;


// The font engine the shell installs. It answers for the bitmap families it was given
// sheets for and hands everything else to the engine RmlUi made, so the outline fonts a
// document names keep working. RmlUi allows one engine per process, and creates its own
// only when none is set, so this is installed after Rml::Initialise rather than before,
// with the engine it created as the one to fall back to.
class UIFontEngineClass : public Rml::FontEngineInterface
{
	public:
		UIFontEngineClass(void);
		virtual ~UIFontEngineClass(void);

		// Takes a bitmap family's two sheets, the one naming colors and the one carrying
		// coverage. False when they do not measure as a font; the family is then not
		// answered for and a document naming it falls back to an outline face.
		bool Load_Sheets(Rml::String const & family, UIImageIndexed const & index, UIImageIndexed const & alpha);

		// Takes a raster family's strikes. A strike is answered for at its height and
		// magnified at any other, so a document names the height of the strike it wants.
		// False for a family with no strike carrying a character.
		bool Load_Strikes(Rml::String const & family, std::vector<UIRasterStrike> const & strikes);

		// The pixels a dp comes to, which is what a document's size is multiplied by before
		// it reaches here. A raster family divides it back out to know which strike is meant.
		void Set_Reference_Scale(float scale) { Reference = (scale > 0.0f) ? scale : 1.0f; }

		// Whether a raster family may answer at all. A strike is drawn at the size it was cut
		// and cannot be resized well, so a frame that is not at one to one turns this off and
		// a document naming the family falls through to the outline face registered for it.
		void Set_Use_Strikes(bool use) { UseStrikes = use; }

		// The engine RmlUi made, which everything this one does not answer for goes to.
		// Taken after Rml::Initialise and dropped when RmlUi shuts down.
		void Set_Fallback(Rml::FontEngineInterface * fallback) { Fallback = fallback; }

		bool Has_Family(Rml::String const & family) const;

		// How many times over the sheets are magnified pixel for pixel before they become an
		// atlas, so the glyphs keep whole pixels when drawn smoothly at the frame's scale, as
		// the rest of the art does. The atlases are rebuilt at the next draw.
		void Set_Magnification(int factor);

		virtual void Initialize(void) override;
		virtual void Shutdown(void) override;

		virtual bool LoadFontFace(Rml::String const & filename, int faceindex, bool fallbackface, Rml::Style::FontWeight weight) override;
		virtual bool LoadFontFace(Rml::String const & filename, int faceindex, Rml::String const & family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallbackface) override;
		virtual bool LoadFontFace(Rml::Span<const Rml::byte> data, int faceindex, Rml::String const & family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallbackface) override;

		virtual Rml::FontFaceHandle GetFontFaceHandle(Rml::String const & family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size) override;
		virtual Rml::FontEffectsHandle PrepareFontEffects(Rml::FontFaceHandle handle, Rml::FontEffectList const & effects) override;
		virtual Rml::FontMetrics const & GetFontMetrics(Rml::FontFaceHandle handle) override;
		virtual int GetStringWidth(Rml::FontFaceHandle handle, Rml::StringView string, Rml::TextShapingContext const & shaping, Rml::Character prior) override;
		virtual int GenerateString(Rml::RenderManager & manager, Rml::FontFaceHandle handle, Rml::FontEffectsHandle effects, Rml::StringView string, Rml::Vector2f position, Rml::ColourbPremultiplied color, float opacity, Rml::TextShapingContext const & shaping, Rml::TexturedMeshList & meshes) override;
		virtual int GetVersion(Rml::FontFaceHandle handle) override;
		virtual void ReleaseFontResources(void) override;

	private:
		struct SheetFamily
		{
			Rml::String Name;
			UIImageIndexed Index;
			UIImageIndexed Alpha;
			UISheetFontMetrics Metrics;
		};

		struct RasterFamily
		{
			Rml::String Name;
			std::vector<UIRasterStrike> Strikes;
		};

		SheetFamily const * Find_Family(Rml::String const & family) const;
		RasterFamily const * Find_Raster_Family(Rml::String const & family) const;
		UISheetFaceClass * Find_Face(Rml::FontFaceHandle handle) const;
		UIRasterFaceClass * Find_Raster_Face(Rml::FontFaceHandle handle) const;

		Rml::FontEngineInterface * Fallback;
		int Magnification = 1;
		float Reference = 1.0f;
		bool UseStrikes = true;
		std::vector<std::unique_ptr<SheetFamily>> Families;
		std::vector<std::unique_ptr<UISheetFaceClass>> Faces;
		std::vector<std::unique_ptr<RasterFamily>> RasterFamilies;
		std::vector<std::unique_ptr<UIRasterFaceClass>> RasterFaces;
};
