/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "coord.h"

#include "theme.hh"


class SaveStreamClass;
#include "win.h"

class ShapeSet;

class IonStormClass
{
	public:
		static void Init(void);
		static HRESULT Save(SaveStreamClass & stream);
		static HRESULT Load(SaveStreamClass & stream);

		static void Serialize(SaveStreamClass & stream);

		static void Do_Screen_Static(int shapenum);
		static void Lightning_Bolt(Cell cell);
		static void Set_Ion_Storm_Active(bool active);
		static bool Is_Ion_Storm_Active(void);
		static void Ion_Storm_Begin(int duration, int warning=0);
		static void Ion_Storm_End(void);
		static void AI(void);
		static void Apply_Secondary_Effect(bool do_static);

	private:
		/*
		 * If an ion storm is raging over the battlefield, then this flag will be true.
		 * While it is set, ion sensitive locomotors have no power and the world wears
		 * the ion tint.
		 */
		static bool IsActive;

		/*
		 * This is the game frame that the current storm broke on. Together with the
		 * duration it decides when the storm has blown itself out.
		 */
		static int StartFrame;

		/*
		 * This is how long the storm is to last, expressed in game frames. A storm that
		 * is still waiting to break carries its duration here as well, so that the
		 * deferment can hand it over when the wait is up. If -1, the storm never expires.
		 */
		static int Duration;

		/*
		 * This is the number of game frames still to wait before a scheduled storm
		 * breaks, counted down once per frame. The player is warned every fifteen seconds
		 * as it runs out. Zero means that no storm is pending.
		 */
		static int Deferment;

		/*
		 * This points to the screen static shapes that are tiled over the tactical view
		 * while the palettes are being tinted. They are fetched the first time they are
		 * needed and kept for the rest of the session.
		 */
		static ShapeSet const * StaticShape;

		/*
		 * This is the music that was playing when the storm rolled in, remembered so that
		 * it can be resumed once the storm passes. Between storms it holds
		 * THEME_PICK_ANOTHER.
		 */
		static ThemeType PreviousTheme;
};
