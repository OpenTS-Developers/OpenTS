/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <memory>
#include <vector>

class UIFontEngineClass;
class UIShellHostClass;


// Every document needs this face; nothing can be shown without it.
inline constexpr char const UI_SHIPPED_FONT_FILE[] = "Arimo.ttf";


class UIFontRegistryClass
{
	public:
		explicit UIFontRegistryClass(UIShellHostClass & host);
		~UIFontRegistryClass(void);

		UIFontRegistryClass(UIFontRegistryClass const &) = delete;
		UIFontRegistryClass & operator=(UIFontRegistryClass const &) = delete;

		void Register(void);
		void Shutdown(void);
		bool Shipped_Face_Loaded(void) const { return(ShippedLoaded); }

		void Load_Game_Fonts(void);
		void Forget_Game_Fonts(void) { GameFontsTried = false; }

		void Set_Scale(float ratio);
		void Set_Magnification(int factor);
		void Apply_Policy(void);

	private:
		void Log(char const * format, ...);
		bool Load_Sheet_Font(char const * family);
		void Load_Fnt_Fonts(void);

		UIShellHostClass & Host;
		std::unique_ptr<UIFontEngineClass> Engine;

		std::vector<unsigned char> SystemFontData;
		std::vector<unsigned char> PrintFontData;

		bool Registered = false;
		bool ShippedLoaded = false;
		bool GameFontsTried = false;
		float PixelRatio = 1.0f;
};
