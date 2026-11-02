/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uifonts.h"

#include "ui/rml/rmlfont.h"
#include "ui/rml/rmltexture.h"
#include "ui/uihost.h"

#include <RmlUi/Core/Core.h>
#include <cmath>
#include <cstdarg>
#include <cstdio>


static char const * const UI_SHEET_FONT_FAMILY = "dlgsys";
static char const * const UI_SANS_FONT_FAMILY = "dlg-sans";
static char const * const UI_SANS_RASTER_FILES[] = {
	"sserife.fon",		// Western
	"sserifee.fon",		// Central European
	"sserifer.fon",		// Cyrillic
	"sserifeg.fon",		// Greek
	"sserifet.fon",		// Turkish
	"ssee1257.fon",		// Baltic
};
static char const * const UI_SANS_FONT_FILE = "micross.ttf";
static char const * const UI_PRINT_FONT_FAMILY = "fullfnt";
static char const * const UI_PRINT_FONT_FILE = "arial.ttf";

struct UIGameFontType
{
	char const * File;
	char const * Family;
	int Spacing;
	bool Outlined;
	bool Gradient;
};

// The spacing and outline settings the game loads each font with, and whether its text
// printing colors the font's shade values.
static UIGameFontType const UI_GAME_FONTS[] = {
	{ "KIA6PT.FNT", "wwfont-kia6pt", 1, false, false },
	{ "6POINT.FNT", "wwfont-6point", 1, true, false },
	{ "8POINT.FNT", "wwfont-8point", 1, true, false },
	{ "EDITFNT.FNT", "wwfont-editfnt", 1, true, true },
	{ "GRAD6FNT.FNT", "wwfont-grad6fnt", 2, true, true },
	{ "12METFNT.FNT", "wwfont-12metfnt", 1, false, true },
};


UIFontRegistryClass::UIFontRegistryClass(UIShellHostClass & host) :
	Host(host),
	Engine(std::make_unique<UIFontEngineClass>())
{
}


UIFontRegistryClass::~UIFontRegistryClass(void)
{
}


void UIFontRegistryClass::Log(char const * format, ...)
{
	char buffer[512];
	va_list args;

	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	Host.Log(buffer);
}


/// <summary>
/// Installs the font engine and loads the system and shipped faces. Call it once RmlUi is
/// initialized; the game's own fonts wait for Load_Game_Fonts.
/// </summary>
void UIFontRegistryClass::Register(void)
{
	Registered = true;
	UI_Register_Fnt_Effects();

	Engine->Set_Fallback(Rml::GetFontEngineInterface());
	Rml::SetFontEngineInterface(Engine.get());

	bool sansloaded = false;
	std::vector<UIRasterStrike> strikes;
	for (char const * const name : UI_SANS_RASTER_FILES) {
		std::string path = Host.System_Font_Path(name);
		std::vector<std::uint8_t> bytes;
		if (path.empty() || !UI_Read_File(path.c_str(), bytes)) {
			continue;
		}

		std::vector<UIRasterStrike> cut;
		if (!UI_Read_Raster_Font(std::span<std::uint8_t const>(bytes.data(), bytes.size()), cut)) {
			continue;
		}

		if (strikes.empty()) {
			strikes = std::move(cut);
			Log("UI: %s answers for %s with %d strikes\n", name, UI_SANS_FONT_FAMILY, (int)strikes.size());
			continue;
		}

		int added = UI_Merge_Raster_Strikes(strikes, cut);
		if (added > 0) {
			Log("UI: %s adds %d characters to %s\n", name, added, UI_SANS_FONT_FAMILY);
		}
	}

	if (!strikes.empty()) {
		sansloaded = Engine->Load_Strikes(UI_SANS_FONT_FAMILY, strikes);
	}

	bool sansface = false;
	std::string sans = Host.System_Font_Path(UI_SANS_FONT_FILE);
	if (!sans.empty() && UI_Read_File(sans.c_str(), SystemFontData)) {
		sansface = Rml::LoadFontFace(Rml::Span<const Rml::byte>(SystemFontData.data(), SystemFontData.size()), UI_SANS_FONT_FAMILY, Rml::Style::FontStyle::Normal);
		sansloaded = sansface || sansloaded;
	}

	// Stands in for FULLFNT3.SHP, which resembles Arial.
	bool printface = false;
	std::string print = Host.System_Font_Path(UI_PRINT_FONT_FILE);
	if (!print.empty() && UI_Read_File(print.c_str(), PrintFontData)
		&& Rml::LoadFontFace(Rml::Span<const Rml::byte>(PrintFontData.data(), PrintFontData.size()), UI_PRINT_FONT_FAMILY, Rml::Style::FontStyle::Normal)) {
		printface = true;
		Log("UI: %s answers for %s\n", UI_PRINT_FONT_FILE, UI_PRINT_FONT_FAMILY);
	} else {
		Log("UI: %s is not on this machine, so %s is the shipped face\n", UI_PRINT_FONT_FILE, UI_PRINT_FONT_FAMILY);
	}

	ShippedLoaded = Rml::LoadFontFace(UI_SHIPPED_FONT_FILE);
	if (!ShippedLoaded) {
		Log("UI: %s did not load, so no document can be shown\n", UI_SHIPPED_FONT_FILE);
		return;
	}

	if (!sansloaded) {
		Log("UI: neither %s nor %s is on this machine, so %s is the shipped face\n",
			UI_SANS_RASTER_FILES[0], UI_SANS_FONT_FILE, UI_SANS_FONT_FAMILY);
	}

	// Where a system face loaded, the shipped face adds only its bold weight, which the system
	// faces lack, since a weight loaded twice keeps the first face.
	Rml::LoadFontFace(UI_SHIPPED_FONT_FILE, UI_SANS_FONT_FAMILY, Rml::Style::FontStyle::Normal,
		sansface ? Rml::Style::FontWeight::Bold : Rml::Style::FontWeight::Auto);
	Rml::LoadFontFace(UI_SHIPPED_FONT_FILE, UI_PRINT_FONT_FAMILY, Rml::Style::FontStyle::Normal,
		printface ? Rml::Style::FontWeight::Bold : Rml::Style::FontWeight::Auto);

	Rml::LoadFontFace(UI_SHIPPED_FONT_FILE, UI_SHEET_FONT_FAMILY, Rml::Style::FontStyle::Normal);
}


void UIFontRegistryClass::Shutdown(void)
{
	Registered = false;
	ShippedLoaded = false;
}


/// <summary>
/// Loads the game's .FNT fonts and the dialog sheet font from the mounted archives, once
/// after Register and again after each Forget_Game_Fonts.
/// </summary>
void UIFontRegistryClass::Load_Game_Fonts(void)
{
	if (GameFontsTried || !Registered) {
		return;
	}

	GameFontsTried = true;
	Load_Fnt_Fonts();
	if (!Load_Sheet_Font(UI_SHEET_FONT_FAMILY)) {
		Log("UI: the %s sheets are not in the game's art, so %s stays the shipped face\n", UI_SHEET_FONT_FAMILY, UI_SHEET_FONT_FAMILY);
	}
}


void UIFontRegistryClass::Set_Scale(float ratio)
{
	Engine->Set_Reference_Scale(ratio);
	PixelRatio = ratio;
	Apply_Policy();
}


void UIFontRegistryClass::Set_Magnification(int factor)
{
	Engine->Set_Magnification(factor);
}


void UIFontRegistryClass::Apply_Policy(void)
{
	Engine->Set_Use_Strikes(Host.Bitmap_System_Font() && std::abs(PixelRatio - 1.0f) < 0.001f);
}


bool UIFontRegistryClass::Load_Sheet_Font(char const * family)
{
	std::string index = std::string(family) + "i.pcx";
	std::string alpha = std::string(family) + "a.pcx";

	UIImageIndexed indexsheet;
	UIImageIndexed alphasheet;
	if (!UI_Load_Indexed_Image(index.c_str(), indexsheet) || !UI_Load_Indexed_Image(alpha.c_str(), alphasheet)) {
		return(false);
	}

	return(Engine->Load_Sheets(family, indexsheet, alphasheet));
}


void UIFontRegistryClass::Load_Fnt_Fonts(void)
{
	for (UIGameFontType const & type : UI_GAME_FONTS) {
		std::vector<unsigned char> bytes;
		UIFntFont font;
		if (!UI_Read_Game_File(type.File, bytes)) {
			Log("UI: %s is not in the game's art, so %s is not available\n", type.File, type.Family);
			continue;
		}
		if (!UI_Read_Fnt(std::span<std::uint8_t const>(bytes.data(), bytes.size()), font)
			|| !Engine->Load_Fnt(type.Family, font, type.Spacing, type.Outlined, type.Gradient)) {
			Log("UI: %s could not be read, so %s is not available\n", type.File, type.Family);
		}
	}
}
