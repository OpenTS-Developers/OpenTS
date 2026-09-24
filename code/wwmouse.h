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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Library/wwmouse.h                            $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/11/97 10:11a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "xmouse.h"

#include <vector>

class ShapeSet;
struct PlatformCursor;

/*
**	Handles the mouse as it relates to the C&C game engine. It is expected that only
**	one object of this type will be created during the lifetime of the game.
*/
class WWMouseClass : public Mouse {
		typedef Mouse BASECLASS;

	public:
		/*
		**	Private constructor.
		*/
		WWMouseClass(void);
		virtual ~WWMouseClass(void) override;

		/*
		**	Sets the game-drawn mouse imagery.
		*/
		virtual void Set_Cursor(Point2D const & hotspot, ShapeSet const * cursor, int shape) override;

		/*
		**	Controls visibility of the game-drawn mouse.
		*/
		virtual void Hide_Mouse(void) override;
		virtual void Show_Mouse(void) override;

		/*
		**	Takes control of and releases control of the mouse with
		**	respect to the operating system. The mouse must be released
		**	during operations with the operating system. When the mouse is
		**	relased, it may move outside of the confining rectangle and its
		**	shape is controlled by the operating sytem.
		*/
		virtual void Release_Mouse(void) override;
		virtual void Capture_Mouse(void) override;
		virtual bool Is_Captured(void) const override {return(IsCaptured);}

		/*
		**	Hide the mouse if it falls within this game screen region.
		*/
		virtual void Conditional_Hide_Mouse(Rect region) override;
		virtual void Conditional_Show_Mouse(void) override;

		/*
		**	Query about the mouse visiblity state and location.
		*/
		virtual int Get_Mouse_State(void) const override;

		// The position is asked of the system on demand.
		virtual int Get_Mouse_X(void) const override {int x; int y; Get_Bounded_Position(x, y); return(x);}
		virtual int Get_Mouse_Y(void) const override {int x; int y; Get_Bounded_Position(x, y); return(y);}
		virtual Point2D Get_Mouse_Point(void) const override {int x; int y; Get_Bounded_Position(x, y); return(Point2D(x, y));}

		/*
		**	Converts O/S screen coordinates into game coordinates.
		*/
		virtual void Convert_Coordinate(int & x, int & y) const override;

		/*
		**	Recalculate the confining rectangle from the window.
		*/
		void Calc_Confining_Rect(void);

		bool Show_Game_Pointer(void);
		void Refresh_Pointer_Scale(void);

	private:

		/*
		**	Mouse hide/show state. If zero or greater, the mouse is visible. Otherwise
		**	it is invisible.
		*/
		int MouseState;

		/*
		**	If the mouse is being managed by this class (for the game), then this flag
		**	will be true. When the mouse has been released to be managed by the operating
		**	system, this flag will be false. However, this class will still track the mouse
		**	position.
		*/
		bool IsCaptured;

		// The desktop rectangle of the game window's client area.
		Rect ConfiningRect;

		// The pointer's show count while the mouse is released, which hides the window's
		// arrow while it is below zero.
		int ReleasedState;

		// The pointer is a system cursor built from the game's shapes, so moving it needs no new frame.
		struct CachedCursor
		{
			int HotX;
			int HotY;
			PlatformCursor * Cursor;
		};

		// One slot per frame of CursorShape, holding the cursor once the game has asked for
		// that frame at CursorCacheScale.
		ShapeSet const * CursorShape;
		std::vector<CachedCursor> CursorCache;
		int CursorCacheScale;

		ShapeSet const * CurrentShape;
		int CurrentFrame;
		int CurrentHotX;
		int CurrentHotY;
		PlatformCursor * CurrentCursor;
		bool CursorVisible;

		void Get_Bounded_Position(int & x, int & y) const;
		void Client_To_Game(int & x, int & y) const;
		void Show_Released_Pointer(void) const;
		void Select_Cursor(ShapeSet const * shape, int frame, int hotx, int hoty, bool apply);
		void Set_Cursor_Visible(bool visible);
		void Flush_Cursor_Cache(void);

		virtual bool Is_Hidden(void) const override {return(MouseState < 0);}
};
