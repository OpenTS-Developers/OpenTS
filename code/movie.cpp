/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#include "always.h"

#include "movie.h"

#include "_keyboar.h"
#include "_map.h"
#include "_mixfile.h"
#include "_rect.h"
#include "_surface.h"
#include "ccfile.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "globals.h"
#include "goptions.h"
#include "gscreen.h"
#include "mixfile.h"
#include "movies.h"
#include "session.h"
#include "vector.h"
#include "vqa.h"

#include "vq.hh"


/// <summary>
/// Completes a half filled interpolated palette table.
/// The interpolation table is symmetric, so only one triangle of it needs to be stored on
/// disk. Use this routine to mirror the stored half across the diagonal after loading.
/// </summary>
/// <param name="interpal">Pointer to the 256 by 256 interpolation table to complete.</param>
void Rebuild_Interpolated_Palette(unsigned char * interpal)
{
	for (int y=0; y<SIZE_OF_PALETTE - 1; y++) {
		for (int x=y+1; x<SIZE_OF_PALETTE; x++) {
			*(interpal + (y*SIZE_OF_PALETTE+x)) = *(interpal + (x*SIZE_OF_PALETTE+y));
		}
	}
}


unsigned char 	* InterpolatedPalettes[100];
BOOL				PalettesRead;
unsigned			PaletteCounter;


/***********************************************************************************************
 * Play_Movie -- Plays a VQ movie.                                                             *
 *                                                                                             *
 *    Use this routine to play a VQ movie. It will dispatch the specified movie to the         *
 *    VQ player. The routine will not return until the movie has finished playing.             *
 *                                                                                             *
 * INPUT:   name  -- The name of the movie file (sans ".VQA").                                 *
 *                                                                                             *
 *          theme -- The identifier for an optional theme that should be played in the         *
 *                   background while this VQ plays.                                           *
 *                                                                                             *
 *          clrscrn -- 'true' if to clear the screen when the movie is over                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void Play_Movie(char const * name, ThemeType theme, bool clrscrn_after, bool stretch,
	bool clrscrn_before, MixFileSearchFilter const * filter)
{
	if (!CCFileClass(name).Is_Available()) {
		return;
	}

	/*
	**	Don't play movies in multiplayer mode
	*/
	if (Session.Type != GAME_NORMAL) {
		return;
	}

	Keyboard->Clear();

	VQHandle * vqa = Movie_Create(name, HiddenSurface, Rect(0,0,0,0), Rect(0,0,0,0),
		int(Options.SoundVolume * 255.0), 1, filter);

	if (vqa != NULL) {

		if (vqa->VQA->Get_VQA_Width() < 320 && vqa->VQA->Get_VQA_Height() < 200) {
			Movie_Destroy(vqa);
			delete vqa;
			return;
		}

		bool dostretch = (stretch == true && Options.StretchMovies == true);

		if (DSurface::AllowStretchBlits == true && dostretch == true) {
			if (vqa->InitialRect.Width < VisibleRect.Width && vqa->InitialRect.Height < VisibleRect.Height) {
				float ratio = ((float)VisibleRect.Width / (float)vqa->InitialRect.Width);
				vqa->StretchRect.Width = VisibleRect.Width;
				vqa->StretchRect.Height = (int)(vqa->InitialRect.Height * ratio);
				vqa->StretchRect.X = 0;
				vqa->StretchRect.Y = (VisibleRect.Height - vqa->StretchRect.Height) / 2;
				DebugString("Stretching movie %dx%d -> %dx%d\n", vqa->InitialRect.Width, vqa->InitialRect.Height, vqa->StretchRect.Width, vqa->StretchRect.Height);
			}
		}

		/*
		**	Prepare to play a movie. First hide the mouse and stop any score that is playing.
		**	While the score (if any) is fading to silence, fade the palette to black as well.
		**	When the palette has finished fading, wait until the score has finished fading
		**	before launching the movie.
		*/
		if (clrscrn_before) {
			HiddenSurface->Fill(0);
			Update_Visible_Surface(HiddenSurface);
		}

		Movie_Play(vqa, true, theme, false);
		Movie_Destroy(vqa);
		delete vqa;

		/*
		**	Presume that the screen is left in a garbage state as well as the palette
		**	being in an unknown condition. Recover from this by clearing the screen and
		**	forcing the palette to black.
		*/
		if (clrscrn_after == true) {
			HiddenSurface->Fill(0);
			Update_Visible_Surface(HiddenSurface);
		}

		Map.Flag_To_Redraw(GS_REDRAW_ALL);
		Keyboard->Clear();
	}
}


/// <summary>
/// Plays a VQ movie with none of the usual trimmings.
/// This is the plain variant of Play_Movie -- the screen is blanked, the movie runs at the
/// default volume, and no stretching is attempted. A missing movie is quietly ignored.
/// </summary>
/// <param name="name">The name of the movie file, including the ".VQA" extension.</param>
/// <param name="theme">The optional theme to play behind the movie.</param>
void _Play_Movie(char const * name, ThemeType theme)
{
	bool notavailable = CCFileClass(name).Is_Available() == false;
	if (!notavailable) {
		HiddenSurface->Fill(0);
		Update_Visible_Surface(HiddenSurface);
		Keyboard->Clear();
		VQHandle * vqa = Movie_Create(name, HiddenSurface, Rect(0,0,0,0), Rect(0,0,0,0), -1, true);
		if (vqa != NULL) {
			Movie_Play(vqa, true, theme, true);
			Movie_Destroy(vqa);
			delete vqa;
			Keyboard->Clear();
		}
	}
}


static bool Campaign_Intro_Archive(MixFileClass const * mixfile, void const * context)
{
	MixFileClass const * wanted = (MixFileClass const *)context;

	if (mixfile == MoviesMix) {
		return(mixfile == wanted);
	}
	for (int index = 0; index < MoviesMixLocal.Count(); index++) {
		if (mixfile == MoviesMixLocal[index]) {
			return(mixfile == wanted);
		}
	}
	return(true);
}


/// <summary>
/// Plays the opening film of a campaign.
/// </summary>
/// <param name="disc">The disc the campaign's files are on, as its CD number.</param>
/// <remarks>Both base-game movie archives contain INTRO.VQA. Normal overrides retain their
/// priority, but only the movie archive selected by the campaign is searched. If that archive
/// is not mounted, the ordinary search order is used.</remarks>
void Play_Campaign_Intro(int disc)
{
	char filename[64];

	sprintf(filename, "MOVIES%02d.MIX", disc + 1);

	MixFileClass * wanted = MFCD::Finder(filename);
	if (wanted != NULL) {
		MixFileSearchFilter const filter = {Campaign_Intro_Archive, wanted};
		Play_Movie("INTRO.VQA", THEME_NONE, true, true, true, &filter);
	} else {
		Play_Movie("INTRO.VQA", THEME_NONE, true);
	}
}


/// <summary>
/// Plays a VQ movie.
/// This routine is the convenient form that takes a movie identifier rather than a filename.
/// A movie identifier of VQ_NONE is quietly ignored.
/// </summary>
void Play_Movie(VQType vq, ThemeType theme, bool clrscrn, bool stretch)
{
	static char _buf[20];
	if (vq != VQ_NONE) {
		strcpy(_buf, Movies[vq]);
		strcpy(_buf + strlen(Movies[vq]), ".VQA");
		Play_Movie(_buf, theme, clrscrn, stretch, true);
	}
}


/// <summary>
/// Plays a movie on the sidebar.
/// This routine queues the movie up to play within the sidebar surface while the game
/// carries on around it. A missing movie, or a multiplayer game, is quietly ignored.
/// </summary>
/// <param name="name">The name of the movie file, including the ".VQA" extension.</param>
void Play_Ingame_Movie(const char * name)
{
	bool notavailable = CCFileClass(name).Is_Available() == false;
	if (!notavailable && Session.Type == GAME_NORMAL) {
		VQHandle * vqa = Movie_Create(name, SidebarSurface, Rect(0,0,0,0), SidebarSurface->Get_Rect(), int(Options.SoundVolume * 255.0), false);
		if (vqa != NULL) {
			Movie_Queue_Ingame(vqa);
		}
	}
}


/// <summary>
/// Plays a movie on the sidebar.
/// This routine is the convenient form that takes a movie identifier rather than a filename.
/// A movie identifier of VQ_NONE is quietly ignored.
/// </summary>
void Play_Ingame_Movie(VQType vq)
{
	static char _buf[20];
	if (vq != VQ_NONE) {
		strcpy(_buf, Movies[vq]);
		strcpy(_buf + strlen(Movies[vq]), ".VQA");
		Play_Ingame_Movie(_buf);
	}
}


/// <summary>
/// Shuts down any in game movies that are playing.
/// This routine tears down every queued sidebar movie and releases it. Call it whenever the
/// sidebar is going away or the scenario is ending.
/// </summary>
void Stop_Ingame_Movie(void)
{
	for (int i = 0; i < IngameVQ.Count(); i++) {
		VQHandle * vqa = IngameVQ[i];
		if (vqa != NULL) {
			Movie_Destroy(vqa);
			delete vqa;
		}
	}

	IngameVQ.Clear();
}


/// <summary>
/// Suspends or resumes the in game movie.
/// Use this routine when the game itself is being suspended, so that the sidebar movie does
/// not run on while everything else is stopped.
/// </summary>
/// <param name="pause">Should the movie be suspended rather than resumed?</param>
void Pause_Ingame_Movie(bool pause)
{
	if (Has_Ingame_Movies()) {
		VQHandle * vqa = IngameVQ[0];
		if (pause == true) {
			Movie_Pause(vqa);
			DebugString("IngameVQ Pause\n");
		} else {
			Movie_Resume(vqa);
			DebugString("IngameVQ Resume\n");
		}
	}
}


/// <summary>
/// Is there an in game movie playing on the sidebar?
/// </summary>
/// <returns>bool; Is at least one in game movie queued up?</returns>
bool Has_Ingame_Movies(void)
{
	return(IngameVQ.Count() > 0);
}
