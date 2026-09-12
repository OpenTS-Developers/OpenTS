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

/* $Header: /CounterStrike/MOUSE.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MOUSE.H                                                      *
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

class ShapeSet;

#include "scroll.h"
#include "stage.h"

class MouseClass: public ScrollClass
{
		typedef ScrollClass BASECLASS;

	public:
		virtual bool Load(SaveStreamClass & stream) override;
		virtual bool Save(SaveStreamClass & stream) override;

		virtual void Serialize(SaveStreamClass & stream) override;

	public:
		MouseClass(void);
		virtual ~MouseClass(void) override {}

		/*
		**	Initialization
		*/
		virtual void One_Time(void) override;   // One-time inits
		virtual void Init_Clear(void) override; // Clears all to known state

		virtual void AI(KeyNumType &input, Point2D const & xy) override;
		virtual bool Override_Mouse_Shape(MouseType mouse, bool wsmall=false) override;
		virtual void Revert_Mouse_Shape(void) override;
		virtual MouseType Get_Mouse_Shape(void) const override {return(NormalMouseShape);};
		virtual void Mouse_Small(bool wsmall) override;

		virtual void Set_Default_Mouse(MouseType mouse, bool wsmall = false) override;

		int Get_Mouse_Current_Frame(MouseType mouse, bool wsmall) const;
		Point2D Get_Mouse_Hotspot(MouseType mouse) const;
		int Get_Mouse_Start_Frame(MouseType mouse) const;
		int Get_Mouse_Frame_Count(MouseType mouse) const;

		/*
		**	This allows the tactical map input gadget access to change the
		**	mouse shapes.
		*/
		friend class TacticalClass;

		/*
		**	This points to the loaded mouse shapes.
		*/
		static ShapeSet const * MouseShapes;

	private:

		/*
		**	This type is used to control the frames and rates of the mouse
		**	pointer. Some mouse pointers are actually looping animations.
		*/
		struct MouseStruct
		{
			int StartFrame; // Starting frame number.
			int FrameCount; // Number of animation frames.
			int FrameRate;  // Frame delay between changing frames.
			int SmallFrame; // Start frame number for small version (if any).
			int X,Y;        // Hotspot X and Y offset.
		};

		/*
		**	The control frames and rates for the various mouse pointers are stored
		**	in this static array.
		*/
		static MouseStruct MouseControl[MOUSE_COUNT];

	public:
		/*
		**	If the small representation of the mouse is active, then this flag is true.
		*/
		bool IsSmall;


	private:
		/*
		**	The mouse shape is controlled by these variables. These
		**	hold the current mouse shape (so resetting won't be needlessly performed) and
		**	the normal default mouse shape (when arrow shapes are needed).
		*/
		MouseType CurrentMouseShape;
		MouseType NormalMouseShape;

		/*
		**	For animating mouse shapes, this controls the frame and animation rate.
		*/
		static CDTimerClass<SystemTimerClass> Timer;
		int Frame;
};
