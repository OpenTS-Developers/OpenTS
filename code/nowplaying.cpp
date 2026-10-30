/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "nowplaying.h"

#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_timer.h"
#include "data.h"
#include "dialog.h"
#include "font.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "init.h"
#include "language/language.h"
#include "rules.h"
#include "scheme.h"
#include "stimer.h"
#include "surface.h"
#include "tactical.h"
#include "theme.h"
#include "win.h"

#include <algorithm>
#include <cstdio>


namespace
{

TextPrintType const STYLE = TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW);

Rect Area;
char Text[320];
int Expires;
int CoveredWidth;
ThemeType Shown = THEME_NONE;
bool Visible;
bool WasScenario;
bool Announce;


int Line_Height(void)
{
	FontClass * font = Font_From_TPF(STYLE);
	if (font != NULL && font->Get_Height() > 0) {
		return(font->Get_Height());
	}
	return(14);
}


int Line_Width(void)
{
	FontClass * font = Font_From_TPF(STYLE);
	if (font == NULL || Text[0] == '\0') {
		return(0);
	}
	return(font->String_Pixel_Width(Text));
}


void Publish(int width)
{
	int mark = std::max(width, CoveredWidth);
	CoveredWidth = width;
	if (mark <= 0 || TacticalMap == NULL || Area.Width <= 0 || Area.Height <= 0) {
		return;
	}

	int height = Line_Height();
	int y = Area.Y + Area.Height - height;
	if (y < Area.Y) {
		y = Area.Y;
		height = Area.Height;
	}
	int pixels = std::min(mark + 2, Area.Width);
	TacticalMap->Register_Dirty_Area(Rect(Area.X, y, pixels, height + 1));
}


char const * Track_Name(ThemeType theme)
{
	char const * name = Theme.Full_Name(theme);
	if (name != NULL && name[0] != '\0') {
		return(name);
	}
	if ((unsigned)theme < (unsigned)Theme.Max_Themes()) {
		name = Theme.Base_Name(theme);
		if (name != NULL && name[0] != '\0') {
			return(name);
		}
	}
	return(NULL);
}


void Show(ThemeType theme)
{
	char const * name = Track_Name(theme);
	if (name == NULL || Rule == NULL) {
		if (Visible) {
			Visible = false;
			Text[0] = '\0';
			Publish(0);
		}
		return;
	}

	std::snprintf(Text, sizeof(Text), Fetch_String(TXT_NOW_PLAYING), name);
	Expires = (int)TickCount + int(Rule->MessageDelay * TICKS_PER_MINUTE);
	Visible = true;
	Publish(Line_Width());
}

}


void Now_Playing_Set_View(Rect const & area)
{
	Area = area;
}


void Now_Playing_AI(void)
{
	ThemeType const playing = Theme.What_Is_Playing();
#if defined(__APPLE__)
	Now_Playing_System_Keys(Options.NowPlaying && GameInFocus, playing >= THEME_FIRST ? Track_Name(playing) : NULL);
#endif

	bool active = Options.NowPlaying && ScenarioActive && Area.Width > 0 && Area.Height > 0;
	if (!active) {
		WasScenario = false;
		Announce = false;
		if (Visible) {
			Visible = false;
			Publish(0);
		}
		return;
	}

	// The track already playing is shown once the scenario is under way.
	if (!WasScenario) {
		WasScenario = true;
		Shown = THEME_NONE;
		Text[0] = '\0';
	}

	if (playing != Shown || Announce) {
		Announce = false;
		Shown = playing;
		if (playing >= THEME_FIRST) {
			Show(playing);
		} else if (Visible) {
			Visible = false;
			Text[0] = '\0';
			Publish(0);
		}
		return;
	}

	if (Text[0] != '\0' && (int)TickCount <= Expires) {
		if (!Visible) {
			Visible = true;
		}
		Publish(Line_Width());
		return;
	}

	if (Visible) {
		Visible = false;
		Publish(0);
	}
}


void Now_Playing_Draw(void)
{
	if (!Visible || Text[0] == '\0' || LogicalSurface == NULL) {
		return;
	}

	int color = 0;
	if (PlayerPtr != NULL && PlayerPtr->Scheme >= 0 && PlayerPtr->Scheme < ColorSchemes.Count() && ColorSchemes[PlayerPtr->Scheme] != NULL) {
		color = PlayerPtr->Scheme;
	} else if (ColorSchemes.Count() <= 0 || ColorSchemes[0] == NULL) {
		return;
	}

	int y = Area.Y + Area.Height - Line_Height();
	if (y < Area.Y) {
		y = Area.Y;
	}
	Simple_Text_Print(Text, *LogicalSurface, LogicalSurface->Get_Rect(), Point2D(Area.X, y), ColorSchemes[color], Options.TextBackgroundColor, STYLE, 1);
}


void Now_Playing_Announce(void)
{
	Announce = true;
}


bool Now_Playing_Key(int virtualkey, bool repeat)
{
	if (!Options.NowPlaying || repeat) {
		return(false);
	}
	if (virtualkey == VK_MEDIA_NEXT_TRACK) {
		Execute_Command("NextTheme");
		return(true);
	}
	if (virtualkey == VK_MEDIA_PREV_TRACK) {
		Execute_Command("PrevTheme");
		return(true);
	}
	return(false);
}
