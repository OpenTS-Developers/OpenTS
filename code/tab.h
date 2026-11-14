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

/* $Header: /CounterStrike/TAB.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TAB.H                                                        *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/15/94                                                     *
 *                                                                                             *
 *                  Last Update : December 15, 1994 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "credits.h"
#include "sidebar.h"

class TabClass: public SidebarClass
{
		typedef SidebarClass BASECLASS;

	public:
		TabClass(void);

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual void AI(KeyNumType &input, Point2D const & xy) override;
		virtual void Draw_It(bool complete=false) override;
		static void Draw_Credits_Tab(void);
		static void Hilite_Tab(int tab);
		void Flash_Money(void);

		virtual void One_Time(void) override;							// One-time inits
		void Redraw_Tab(void) {IsToRedraw = true;Flag_To_Redraw();};

		virtual void Init_For_House(void) override;

		CreditClass Credits;

		CDTimerClass<FrameTimerClass> FlasherTimer;

	protected:

		/*
		**	If the tab graphic is to be redrawn, then this flag is true.
		*/
		bool IsToRedraw;

	private:
		void Set_Active(int select);

		CDTimerClass<FrameTimerClass> MoneyFlashTimer;

		static ShapeSet const * TabShape;
};
