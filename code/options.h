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

/* $Header: /CounterStrike/OPTIONS.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OPTIONS.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : June 8, 1994                                                 *
 *                                                                                             *
 *                  Last Update : June 8, 1994   [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "keyboard.h"

#include "diff.hh"

class OptionsClass {
	public:
		enum {
			MAX_SCROLL_SETTING=7,
			MAX_SPEED_SETTING=7,
			MAX_DETAIL_SETTING=3,
			MAX_DIFFICULTY_SETTING=DIFF_COUNT
		};

		OptionsClass(void);

		void One_Time(void);

		void Fixup_Palette(void) const;
		void Set_Shuffle(bool on);
		void Set_Repeat(bool on);
		void Set_Score_Volume(float volume, bool feedback);
		void Set_Sound_Volume(float volume, bool feedback);
		void Set_Voice_Volume(float volume, bool feedback);
		int Normalize_Delay(int delay) const;
		int Normalize_Volume(int volume) const;
		int Get_Scroll_Method(void) const;

		bool Hotkey_Dialog(void);

		/*
		**	File I/O routines
		*/
		void Load_Settings(void);
		void Save_Settings(void);

		void Set(void);

		/*
		**	This is actually the delay between game frames expressed as 1/60 of
		**	a second. The default value is 4 (1/15 second).
		*/
		int GameSpeed;

		/*
		 * This is the campaign difficulty the player has picked, expressed as the slider
		 * position (0 - 4). It sets both the scenario's difficulty and the computer's when a
		 * single player game starts; multiplay takes its difficulty from the session options.
		 */
		int Difficulty;

		/*
		 * These control how the tactical map scrolls. The method (0 - 2) selects what holding
		 * the right mouse button does: 0 scrolls the map alone, while 1 and 2 also drag the
		 * mouse pointer along so that the map appears to move under it (2 reverses the sense).
		 */
		int ScrollMethod;
		int ScrollRate;					// Distance to scroll.
		bool AutoScroll;				// Does map autoscroll?

		/*
		 * This is how much detail the renderer is allowed to draw (0 - 2). Animations, particle
		 * systems and translucency carry a detail level of their own and are skipped when it
		 * exceeds this one, so a lower setting buys frame rate at the cost of eye candy.
		 */
		int DetailLevel;

		/*
		 * If the sidebar is to be drawn down the right hand edge of the screen, then this flag
		 * will be true. Tiberian Sun offers no way to move it, so it is always true.
		 */
		bool IsSidebarOnRight;

		/*
		 * If each sidebar cameo is to be captioned with the name of what it builds, then this
		 * flag will be true.
		 */
		bool SidebarCameoText;

		/*
		 * If the sidebar cameos are to be sorted rather than left in the order they were
		 * offered, then this flag will be true. Clearing it stops further sorting without
		 * undoing any already done.
		 */
		bool SidebarSorting;

		/*
		 * If selected objects are to draw a line to their target, then this flag will be true.
		 */
		bool ActionLines;

		/*
		 * If tooltips are to pop up over the user interface, then this flag will be true.
		 */
		bool ToolTips;

		/*
		 * The palette index drawn behind each glyph of the in-game message list, or zero for
		 * none. Twelve, black, is the value the CnCNet client's chat background option writes.
		 */
		int TextBackgroundColor;

		/*
		 * Frames between automatic saves of a game started from the menu, or zero for none.
		 * A launch file's own interval takes its place.
		 */
		int AutoSaveInterval;

		/*
		 * These are the dimensions of the game screen, expressed in pixels. The tactical
		 * view, the sidebar and the tabs are all laid out against them.
		 */
		int ScreenWidth;
		int ScreenHeight;

		/*
		 * If the movies are to be stretched to fill the screen, then this flag will be true.
		 */
		bool StretchMovies;

		/*
		 * If the game is to cover the whole screen rather than sit in a window, then this
		 * flag will be true. Either way the window is borderless and the desktop keeps its
		 * own resolution; the frame is scaled to fit.
		 */
		bool Fullscreen;

		/*
		 * These are the dimensions of the window the game runs in, expressed in pixels. A
		 * negative value tracks the size of the frame instead. They have no effect while
		 * the game covers the screen.
		 */
		int WindowWidth;
		int WindowHeight;

		/*
		 * This is how the frame is filtered when the window is larger than it, as one of
		 * the VideoScaleMode values.
		 */
		int ScaleMode;

		/*
		 * If the frame may only be enlarged by whole multiples, then this flag will be
		 * true. It keeps every game pixel the same size at the cost of a wider border.
		 */
		bool IntegerScaling;

		/*
		 * If presents are to wait for the display's refresh, then this flag will be true.
		 * Presents are already limited to the refresh rate without it.
		 */
		bool VSync;

		/*
		 * This is the graphics interface the renderer is asked for, as one of the
		 * BackendRenderer values. It exists to work around a driver, and is best left
		 * choosing for itself.
		 */
		int Renderer;

		/*
		 * This is how many times larger than its artwork the mouse pointer is drawn.
		 * Zero follows the size of the frame on screen, and a negative value leaves the
		 * pointer at the size the artwork was drawn at.
		 */
		int CursorScale;

		float SoundVolume;				// Volume for sound effects.
		float VoiceVolume;				/// Volume for voices.
		float ScoreVolume;				// Volume for scores.
		bool IsScoreRepeat;				// Score should repeat?
		bool IsScoreShuffle;			// Score list should shuffle?

		/*
		 * This is how far ahead of the movie the sound card is assumed to be running, expressed
		 * in VQA time ticks. The audio position is backed off by it so that a software emulated
		 * sound driver stays in step with the video; a real driver has it zeroed instead.
		 */
		unsigned short SoundLatency;

		/*
		**	These are the hotkeys used for keyboard control.
		*/
		KeyNumType KeyForceMove1;
		KeyNumType KeyForceMove2;
		KeyNumType KeyForceAttack1;
		KeyNumType KeyForceAttack2;
		KeyNumType KeySelect1;
		KeyNumType KeySelect2;
		KeyNumType KeyQueueMove1;
		KeyNumType KeyQueueMove2;

	private:

		static char const * const HotkeyName;
};
