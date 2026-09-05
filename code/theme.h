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

/* $Header: /CounterStrike/THEME.H 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : THEME.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 14, 1994                                              *
 *                                                                                             *
 *                  Last Update : August 14, 1994   [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "audio/audiohandle.h"
#include "stimer.h"
#include "vector.h"

#include "side.hh"
#include "theme.hh"

class CCINIClass;

struct ThemeControl {
	ThemeControl(void);
	bool Fill_In(CCINIClass const & ini);

	char Name[256];			// Filename of score.
	char Fullname[64];		// Text number for full score name.
	int Scenario;			// Scenario when it first becomes available.
	float Duration;			// Duration of theme in seconds.
	bool Normal;			// Allowed in normal game play?
	bool Repeat;			// Always repeat this score?
	bool Available;			// Is the score available?
	int Owner;				// What houses are allowed to play this theme (bit field)?
};

class ThemeClass
{
	private:
		char const * Theme_File_Name(ThemeType theme);

		AudioHandle Current;		// Handle to current score.
		ThemeType Score;			// Score number currently being played.
		ThemeType Pending;			// Score to play next.

		/*
		 * This is the volume the music is played at (0 - 255). At zero the theme handler
		 * does not bother starting a score at all.
		 */
		int Volume;

		/*
		 * If every score is to be played over again rather than moving on to the next one,
		 * then this flag will be true. A score can also ask to repeat on its own account
		 * through its Repeat setting.
		 */
		bool IsRepeat;

		/*
		 * If the next score is to be picked at random rather than in order, then this flag
		 * will be true. The score just played is never picked again immediately.
		 */
		bool IsShuffle;

		/*
		 * These are the scores the theme handler knows about, in the order they were read
		 * from the rules. A ThemeType is an index into this list.
		 */
		DynamicVectorClass<ThemeControl *> Themes;

		enum {
			THEME_FADE_MS = 1500		// The 60 maintenance ticks the old driver took to fade.
		};

	public:
		ThemeClass(void);

		ThemeType From_Name(char const * name) const;
		ThemeType Next_Song(ThemeType index) const;
		ThemeType What_Is_Playing(void) const {return(Score);}
		bool Is_Allowed(ThemeType index) const;
		bool Is_Regular(ThemeType theme) const {return(theme != THEME_NONE && Themes[theme]->Normal);}
		char const * Base_Name(ThemeType index) const;
		char const * Full_Name(ThemeType index) const;
		int Max_Themes(void) const {return(Themes.Count());}
		AudioHandle Play_Song(ThemeType index);
		bool Still_Playing(void) const;
		int Track_Length(ThemeType index) const;
		void Scan(void);
		void AI(void);
		void Fade_Out(void) {Queue_Song(THEME_QUIET);}
		void Queue_Song(ThemeType index);
		void Stop(bool fade = false);
		void Set_Shuffle(bool on) {IsShuffle = on;}
		void Set_Repeat(bool on) {IsRepeat = on;}
		bool Is_Shuffle(void) const {return(IsShuffle);}

		void Set_Volume(int volume);

		void Init_Themes(CCINIClass const & ini);
		void Free_Themes(void);
};

extern ThemeClass Theme;
