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

/* $Header: /CounterStrike/DISPLAY.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : DISPLAY.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 1, 1994                                                  *
 *                                                                                             *
 *                  Last Update : May 1, 1994   [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "coord.h"
#include "face.h"
#include "gadget.h"
#include "layer.h"
#include "map.h"
#include "rgb.h"
#include "types.h"

#include "action.hh"
#include "layer.hh"
#include "source.hh"
#include "super.hh"


class ObjectClass;
class ObjectTypeClass;
class CCINIClass;
class TechnoClass;
class WaypointClass;

#define	SIDE_BAR_TAC_WIDTH	10
#define  SIDE_BAR_TAC_HEIGHT	8


class DisplayClass: public MapClass
{
		typedef MapClass BASECLASS;

		friend class TabClass;
		friend class CellClass;
		friend class Tactical;

	public:
		virtual HRESULT Load(IStream * stream);
		virtual HRESULT Save(IStream * stream);

		virtual void Serialize(SaveStreamClass & stream) override;

	public:
		/*
		**	These layer control elements are used to group the displayable objects
		**	so that proper overlap can be obtained.
		*/
		static LayerClass Layer[LAYER_COUNT];

		/*
		**	This records the position and shape of a placement cursor to display
		**	over the map. This cursor is used when placing buildings and also used
		**	extensively by the scenario editor.
		*/
		Cell ZoneCell;
		Cell ZoneOffset;
		Cell const *CursorSize;
		bool ProximityCheck;				// Is proximity check ok?
		bool ShroudCheck;					/// Is shroud check ok?

		/*
		 * These record the object that the tactical view is following, if any. While the
		 * flag is set, the view is recentered on the object every game frame, and the
		 * object clears both of them as it leaves the map.
		 */
		bool FollowingObject;
		ObjectClass * FollowingObjectPtr;

		/*
		 * This records the object under the mouse on the tactical view. It backs the
		 * condition indicator drawn on hover, and the object clears it as it leaves the
		 * map.
		 */
		ObjectClass * HoverObject;

		/*
		**	This holds the building type that is about to be placed upon the map.
		**	It is only valid during the building placement state. The PendingLegal
		**	flag is updated as the cursor moves and it reflects the legality of
		**	placing the building at the desired location.
		*/
		ObjectClass * PendingObjectPtr;
		ObjectTypeClass const * PendingObject;
		HousesType PendingHouse;

		//-------------------------------------------------------------------------
		DisplayClass(void);

		// False when the terrain packs were damaged and only part of the map was read.
		virtual bool Read_INI(CCINIClass const & ini);
		void Write_INI(CCINIClass & ini);

		/*
		**	Initialization
		*/
		virtual void One_Time(void) override;                       // One-time inits
		virtual void Init_Clear(void) override;                     // Clears all to known state
		virtual void Init_IO(void) override;                        // Inits button list

		int Stash_Map_State(void * stash, int);
		void Restore_Map_State(void * stash);

		/*
		**	General display/map/interface support functionality.
		*/
		virtual void AI(KeyNumType &input, Point2D const & xy) override;

		/*
		**	Added functionality.
		*/
		void All_To_Look(bool units_only=false, bool=false);
		void Constrained_Look(Coord const & coord, LEPTON distance);
		void Shroud_Cell(Cell const & cell);
		void Encroach_Shadow(void);
		void Fog_Cell(Cell const & cell);
		void Encroach_Fog(void);
		void Center_Map(void);
		virtual char const * Help_Text(int id);
		virtual void Reposition_Sidebar(void);
		virtual void Abort_Drag_Select(void);
		virtual bool Map_Cell(Cell const & cell, HouseClass * house);
		virtual bool Fog_Map_Cell(Cell const & cell, HouseClass * house);
		virtual bool Shadow_Map_Cell(Cell const & cell, HouseClass * house);
		virtual MouseType Get_Mouse_Shape(void) const = 0;
		virtual bool Scroll_Map(FacingType facing, int & distance, bool really);
		virtual void Set_View_Dimensions(Rect const & dimensions);

		/*
		**	Pending object placement control.
		*/
		virtual void Put_Place_Back(TechnoClass * ) {}; // Affects 'pending' system.
		void Cursor_Mark(Cell const & pos, bool on);
		void Set_Cursor_Shape(Cell const * list);
		Cell Set_Cursor_Pos(Cell const & pos = CELL_NONE);
		Cell Get_Occupy_Dimensions(Cell const *list) const;

		/*
		**	Tactical map only functionality.
		*/
		void Remove(ObjectClass const * object);
		void Submit(ObjectClass const * object);
		Cell Calculated_Cell(SourceType dir, Cell const & waypoint=CELL_NONE, Cell const & cell=CELL_NONE, SpeedType loco=SPEED_FOOT, bool zonecheck=true, MZoneType mzone=MZONE_NORMAL) const;
		bool Passes_Proximity_Check(ObjectTypeClass const * object, HousesType house, Cell const * list, Cell const & trycell) const;
		bool Passes_Shroud_Check(ObjectTypeClass const * object, HousesType house, Cell const * list, Cell const & trycell) const;
		ObjectClass * Cell_Object(Cell const & cell, Point2D const & point) const;
		ObjectClass * Next_Object(ObjectClass * object) const;
		ObjectClass * Prev_Object(ObjectClass * object) const;
		bool Is_Spot_Free(Coord const & coord, bool bridge=false) const;
		Coord Closest_Free_Spot(Coord const & coord, bool any=false) const;
		void Sell_Mode_Control(int control);
		void Waypoint_Mode_Control(int control, bool edit_selected_path=false);
		void Power_Mode_Control(int control);
		void Repair_Mode_Control(int control);
		ObjectClass * Object_To_Follow(void) const;
		void Set_To_Follow(ObjectClass * object);
		void Break_Follow_Mode(void) { Set_To_Follow(NULL); }
		void Reinit_Cell_Drawers(void);
		void Update_Cell_Colors(void);

		LRESULT Windows_Message_Proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
		void Active_Click(ObjectClass * object, Cell cell, ActionType action);
		ActionType Action_To_Waypoint_Action(ActionType action, Cell const & cell);
		void Update_Waypoint_Color(int index);

		/*
		**	Computes starting position based on player's units' Coords.
		*/
		void Compute_Start_Pos(void);

	protected:
		virtual void Mouse_Right_Press(Point2D const & point = Point2D());
		virtual void Mouse_Left_Press(Point2D const & point);
		virtual void Mouse_Left_Up(Cell const & cell, bool shadow, ObjectClass * object, ActionType action, bool wsmall = false);
		virtual void Mouse_Left_Held(Point2D const & point);
		virtual void Mouse_Left_Release(Coord const & coord, Cell const & cell, ObjectClass * object, ActionType action, bool wsmall = false);
		virtual void Mouse_Right_Release(Point2D const & point = Point2D());

	public:
		/*
		**	If the player is currently wielding a wrench (to select buildings for repair),
		**	then this flag is true. In such a state, normal movement and combat orders
		**	are preempted.
		*/
		bool IsRepairMode;

		/*
		**	If the player is currently in "sell back" mode, then this flag will be
		**	true. While in this mode, anything clicked on will be sold back to the
		**	"factory".
		*/
		bool IsSellMode;

		/*
		 * If the player is currently in "power toggle" mode, then this flag will be
		 * true. While in this mode, anything clicked on will have its power state toggled.
		 */
		bool IsPowerMode;

		/*
		 * If the player is currently in "waypoint" mode, then this flag will be
		 * true. While in this mode, the player can manage waypoint paths.
		 */
		bool IsWaypointMode;

		/*
		**	If the player is currently in ion cannon targeting mode, then this
		**	flag will be true.  While in this mode, anything clicked on will be
		**	be destroyed by the ION cannon.
		*/
		SuperWeaponType IsTargettingMode;

		/*
		 * This is the waypoint that the player has picked up and is dragging about the map.
		 * While it is set, the waypoint follows the mouse from cell to cell and is left
		 * wherever the cursor was when the drag ends.
		 */
		WaypointClass * DraggedWaypoint;

		/*
		 * This is where the dragged waypoint stood before it was picked up. Units already
		 * traveling the path head for it rather than chase the waypoint about, and it is
		 * where the waypoint is put back if the drag is abandoned.
		 */
		Coord DraggedWaypointCoord;

		/*
		 * This is the mouse cursor color as it was before a waypoint path recolored it. It
		 * is remembered the first time a waypoint cursor is displayed and put back when the
		 * cursor moves off the waypoint actions.
		 */
		RGBClass WaypointColor;

	protected:

		/*
		**	If it is currently in rubber band mode (multi unit selection), then this
		**	flag will be true. While in such a mode, normal input is preempted while
		**	the extended selection is in progress.
		*/
		bool IsRubberBand;

		/*
		**	The moment the mouse is held down, this flag gets set. If the mouse is dragged
		**	a sufficient distance while held down, then true rubber band mode selection
		**	can begin. Using a minimum distance prevents accidental rubber band selection
		**	mode from being initiated.
		*/
		bool IsTentative;

		/*
		**	This gadget class is used for capturing input to the tactical map. All mouse input
		**	will be routed through this gadget.
		*/
		class TacticalClass : public GadgetClass {
			public:
				TacticalClass(void) : GadgetClass(0,0,0,0,LEFTPRESS|LEFTRELEASE|LEFTHELD|LEFTUP|RIGHTPRESS|RIGHTRELEASE|RIGHTHELD,true) {};

			protected:
				virtual int Action(unsigned flags, KeyNumType & key);
		};
		friend class TacticalClass;

		/*
		**	This is the "button" that tracks all input to the tactical map.
		**	It must be available to derived classes, for Save/Load purposes.
		*/
		static TacticalClass TacButton;

	private:

		/*
		**	This is a utility flag that is set during the icon draw process only if there
		**	was at least one shadow icon detected that should be redrawn. When the shadow
		**	drawing logic is to take place, but this flag is false, then the shadow drawing
		**	will be skipped since it would perform no function.
		*/
		bool IsShadowPresent;

		/*
		**	Rubber band mode consists of stretching a box from the anchor point (specified
		**	here) to the current cursor position.
		*/
		int BandX,BandY;
		int NewX,NewY;

		static void const *ShadowShapes;

		/*
		 * This is the artwork of the building placement cursor -- the tiles laid over the
		 * cells a pending structure would occupy, drawn with the clear frame where the
		 * ground will take the building and with the matching ramp frame where it will not.
		 */
		static void const *PlacementShapes;

		bool Good_Reinforcement_Cell(Cell const & outcell, Cell const & incell, SpeedType loco, int zone, MZoneType mzone) const;

};
