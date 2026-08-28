/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "always.h"

#include "point.h"
#include "sun.h"
#include "win.h"

class ShapeSet;

class ProgressScreenClass
{
	public:
		ProgressScreenClass(void);
		~ProgressScreenClass(void);

		void Initialize(double progress, int count, bool usedialog = false);
		void End(void);

		void Set_Progress_Percent(int index, double value, Point2D pt=Point2D(-1,-1));
		void Add_Progress_Percent(int index, double value, Point2D pt=Point2D(-1,-1));

		double Get_Current_Progress(int index) const
		{
			//if (index < 8) {
				return(PlayerProgress[index] / MainProgress);
			//}
			//return MainProgress;
		}
		double Get_Current_Progress(void) const;

		int Get_Bar_Width(void) const;

		void Set_Graphic_Data(const char * progbar, const char * background = NULL, const char * string = NULL, Point2D pt=Point2D(-1,-1));
		void Display_Progress(Point2D pt = Point2D(-1,-1));

		void Begin_Dialog(void);
		void End_Dialog(void);
	private:
		static INT_PTR CALLBACK Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

	public:
		/*
		 * These are the amounts of the job that each player being tracked has finished so
		 * far, one slot per player. A slot is measured against MainProgress rather than
		 * against a hundred, so the bar drawn for it is the ratio of the two.
		 */
		double PlayerProgress[MAX_PLAYERS];

		/*
		 * This is the total amount of work that the job under way amounts to. Every player's
		 * progress is measured against it, so whatever units the caller counts its work in
		 * will do.
		 */
		double MainProgress;

		/*
		 * Pointer to the caption printed above the progress bars, or NULL if the screen
		 * carries no caption. The text is not copied, so it must outlive the job.
		 */
		const char * String;

		/*
		 * Pointer to the progress bar artwork. A bar is drawn by clipping the first frame of
		 * this shape down to the fraction of the job finished, so the frame's width is also
		 * the width of a full bar.
		 */
		ShapeSet * Shape;

		/*
		 * Pointer to the name of the title page loaded to sit behind the bars, or NULL if
		 * the caller asked for no backdrop.
		 */
		const char * Background;

		/*
		 * If a job is under way and progress updates should be drawn, then this flag will be
		 * true. It is cleared when the job ends, so that a stray progress request arriving
		 * outside a job is quietly ignored.
		 */
		bool IsActive;

		/*
		 * This is the number of players whose progress is being tracked, and so the number
		 * of bars drawn. It is never less than one -- a single player job draws one centered
		 * bar, while a multiplayer job stacks a bar per player with the names alongside.
		 */
		char PlayerCount;

		/*
		 * Handle of the progress dialog, or NULL when the progress is presented on the full
		 * screen instead. The dialog is used where the game must keep a window up while it
		 * works rather than take the screen over.
		 */
		HWND Dialog;

		/*
		 * This is the center of the progress bar display, expressed in screen pixels. A job
		 * that names no spot of its own is centered on the hidden surface.
		 */
		Point2D Pos;

		/*
		 * This is the highest loading message threshold already announced (0 - 100), or -1
		 * before any has been. Each message is printed and its notification sound played
		 * only when the progress first passes its threshold, so that none of them repeat.
		 */
		int Percentage;
};

extern ProgressScreenClass Progress;
