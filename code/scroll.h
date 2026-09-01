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

/* $Header: /CounterStrike/SCROLL.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SCROLL.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 11/18/94                                                     *
 *                                                                                             *
 *                  Last Update : November 18, 1994 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "tab.h"


class ScrollClass: public TabClass
{
		typedef TabClass BASECLASS;

		/*
		 * The radar's tactical gadget invokes the right-click release handler directly, and
		 * that handler is protected here as it is in DisplayClass. RadarClass grants the
		 * gadget the same friendship for its own members.
		 */
		friend class RadarClass::RTacticalClass;

	private:
		/*
		 * The inertia ramp climbs or decays a step each time this count down timer reaches
		 * zero. The scroll distance itself is paced off the clock, not off this.
		 */
		static CDTimerClass<SystemTimerClass> Counter;

		/*
		**	Inertia control for scrolling
		*/
		int	Inertia;

		/*
		 * If the map is allowed to coast scroll under a right button drag, then this flag
		 * will be true. It is what lets a rubber band selection claim the drag first -- the
		 * coast scroll only takes over once no band is in progress.
		 */
		bool IsCoastScrollAllowed;

		/*
		 * This is the point, relative to the tactical view, at which the right mouse button
		 * was last pressed. A coast scroll takes both its direction and its speed from how
		 * far the mouse has been pulled away from it.
		 */
		Point2D RightPressPoint;

		/*
		 * If the right button has been pulled far enough from its press point to count as a
		 * drag rather than a click, then this flag will be true. It is what keeps the
		 * release that ends a coast scroll from being taken as an ordinary deselect click.
		 */
		bool IsDragOperation;

		/*
		 * If the map is allowed to scroll when the mouse rests against the edge of the
		 * tactical view, then this flag will be true.
		 */
		bool IsEdgeScrollAllowed;

		/*
		 * If a mouse button is being held down over the tactical map, then this flag will
		 * be true. While it is set the map hands movement to the held button's own handler
		 * rather than tracking the cursor, and it reports itself as scrolling.
		 */
		bool IsMouseDown;

	public:
		ScrollClass(void);

		virtual void Serialize(SaveStreamClass & stream) override;

		void Set_Scroll_Coasting_Allowed(bool coasting) { IsCoastScrollAllowed = coasting; }

		virtual void AI(KeyNumType &input, Point2D const & xy) override;
		virtual void Init_IO(void) override {/*Counter = 0;*/BASECLASS::Init_IO();};
		virtual bool Is_Scrolling(void) const override;
		virtual void Abort_Drag_Select(void) override;

		bool Resolve_Point(Point2D const & point, Cell & cell, Coord & coord, ObjectClass * & object, bool & fog, bool & shadow);

		ActionType What_Action(Cell const & cell, ObjectClass * object, bool check_fog);

		void Message_Handler(HWND hwnd, UINT & message, UINT & wParam, LONG & lParam);

	protected:
		virtual void Mouse_Right_Press(Point2D const & point = Point2D()) override;
		virtual void Mouse_Right_Release(Point2D const & point = Point2D()) override;

		void Scroll_AI(void);

		void Scroll_Edge(Point2D const & point);
		void Scroll_Coast(Point2D const & point);
};
