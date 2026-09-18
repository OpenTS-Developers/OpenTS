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


static const int UI_SHEET_FONT_NOMINAL_SIZE = 16;


class UIFontEngineClass : public Rml::FontEngineInterface
{
	public:
		UIFontEngineClass(void);
		virtual ~UIFontEngineClass(void);

		bool Load_Sheets(Rml::String const & family, UIImageIndexed const & index, UIImageIndexed const & alpha);
		bool Load_Strikes(Rml::String const & family, std::vector<UIRasterStrike> const & strikes);
		void Set_Reference_Scale(float scale) { Reference = (scale > 0.0f) ? scale : 1.0f; }

		void Set_Use_Strikes(bool use) { UseStrikes = use; }
		void Set_Fallback(Rml::FontEngineInterface * fallback) { Fallback = fallback; }
		bool Has_Family(Rml::String const & family) const;
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
