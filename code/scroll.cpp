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

/* $Header: /CounterStrike/SCROLL.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SCROLL.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 01/08/95                                                     *
 *                                                                                             *
 *                  Last Update : August 25, 1995 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ScrollClass::AI -- Handles scroll AI processing.                                          *
 *   ScrollClass::ScrollClass -- Constructor for the scroll class object.                      *
 *   ScrollClass::Set_Autoscroll -- Turns autoscrolling on or off.                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "scroll.h"

#include "_keyboar.h"
#include "_map.h"
#include "_rect.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "building.h"
#include "cell.h"
#include "dbgprint.h"
#include "gametime.h"
#include "goptions.h"
#include "house.h"
#include "incdec.h"
#include "init.h"
#include "inline.h"
#include "misc.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "suprtype.h"
#include "surface.h"
#include "tactical.h"
#include "vidscale.h"
#include "waypoint.h"

#include "special.hh"

#include <algorithm>


// for some reason in wave..
extern FacingType Facing_Between_Points(Point2D const & pt1, Point2D const & pt2);

#define	SCROLL_DELAY	1

CDTimerClass<SystemTimerClass> ScrollClass::Counter;

// Steps from the scroll tables are paced off the clock at this rate, not off the frames drawn.
#define	SCROLL_STEPS_PER_SECOND	30

// Caps what a poll may cover, so a wait on a dialog or a load does not return as one long jump.
#define	MAX_SCROLL_STEPS_PER_POLL	2.0

static unsigned int _LastScrollPollTime = 0;
static double _ScrollFraction = 0.0;

// Sub-pixel parts of a scaled step, which truncation would otherwise drop at every poll.
static double _EdgeScrollRemainder = 0.0;
static double _CoastRemainderX = 0.0;
static double _CoastRemainderY = 0.0;


/***********************************************************************************************
 * ScrollClass::ScrollClass -- Constructor for the scroll class object.                        *
 *                                                                                             *
 *    This is the constructor for the scroll class object.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ScrollClass::ScrollClass(void) :
	Inertia(0),
	IsCoastScrollAllowed(false),
	RightPressPoint(0,0),
	IsDragOperation(false),
	IsEdgeScrollAllowed(true),
	IsMouseDown(false)
{
	//Counter = SCROLL_DELAY;
}


/// <summary>
/// Lists the members the scroll handler holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ScrollClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	// Counter -- it paces the inertia ramp off the system clock, so a saved value would carry the
	// time of the save into the loaded game.
	// Inertia -- the state of a drag on the tactical map, which no held button survives to
	// continue.
	// IsCoastScrollAllowed
	// RightPressPoint
	// IsDragOperation
	stream.Serialize(IsEdgeScrollAllowed);
	// IsMouseDown -- likewise the drag state, which no held button survives to continue.
}


/***********************************************************************************************
 * ScrollClass::AI -- Handles scroll AI processing.                                            *
 *                                                                                             *
 *    This routine is called on every input poll for purposes of input processing.             *
 *                                                                                             *
 * INPUT:   input    -- Reference to the keyboard/mouse event that just occurred.              *
 *                                                                                             *
 *          x,y      -- The mouse coordinates.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/10/1995 JLB : Created.                                                                 *
 *   08/10/1995 JLB : Revamped for free smooth scrolling.                                      *
 *   08/25/1995 JLB : Handles new scrolling option.                                            *
 *=============================================================================================*/
#define	EVA_WIDTH		80
void ScrollClass::AI(KeyNumType &input, Point2D const & xy)
{
	Scroll_AI();
	BASECLASS::AI(input, xy);
}


/// <summary>
/// Determines what lies underneath a point on the tactical view.
/// This routine is used by the mouse handling before it can decide on an action. Enemy
/// objects the player is not meant to see -- cloaked units and invisible types -- are not
/// reported, and neither is anything hidden behind shroud or fog.
/// </summary>
/// <param name="point">The mouse position, relative to the tactical view.</param>
/// <param name="cell">Receives the cell that lies under the point.</param>
/// <param name="coord">Receives the coordinate that lies under the point.</param>
/// <param name="object">Receives the object under the point, or NULL if there is none.</param>
/// <param name="fog">Receives whether the spot is covered by fog of war.</param>
/// <param name="shadow">Receives whether the spot is covered by shroud.</param>
/// <returns>bool; Does the point lie over the map?</returns>
bool ScrollClass::Resolve_Point(Point2D const & point, Cell & cell, Coord & coord, ObjectClass * & object, bool & fog, bool & shadow)
{
	object = NULL;
	cell = CELL_NONE;
	coord = COORD_NONE;
	fog = false;
	shadow = false;

	if (point.X < 0 || point.Y < 0) {
		return(false);
	}

	cell = (TacticalMap != NULL) ? TacticalMap->Pixel_To_Cell(point + TacticalRect.TopLeft) : CELL_NONE;
	coord = (TacticalMap != NULL) ? TacticalMap->Pixel_To_Coord(point + TacticalRect.TopLeft) : COORD_NONE;

	if (TacticalMap != NULL && Map.In_Radar(coord)) {
		Coord coord_height_adjusted = Coord(cell, Map.Get_Height_GL(Coord(cell)));

		shadow = Map.Is_Shrouded(coord_height_adjusted) && MainWindow != NULL;
		fog = false;

		if (Scen->Special.IsFogOfWar) {
			BuildingClass* building = Map[coord_height_adjusted].Cell_Building();
			if (building != NULL) {
				fog = building->IsFogged;
			} else {
				fog = Map.Is_Fogged(coord_height_adjusted) && MainWindow != NULL;
			}
		}

		/*
		**	Determine the object that the mouse is currently over.
		*/
		if (!shadow && !fog) {
			object = TacticalMap->Get_Selectable_Object(point);

			TechnoClass * techno = Dynamic_Cast<TechnoClass *>(object);
			if (techno != NULL) {

				/*
				**	Special case check to ignore cloaked object if not owned by the player.
				*/
				if (!techno->IsOwnedByPlayer && ((techno->Cloak == CLOAKED && !Map[techno->Center_Coord()].Is_Sensed(PlayerPtr->HeapID)) || techno->TClass->IsInvisible)) {
					object = NULL;
				}
			}

			if (object != NULL && object->RTTI == RTTI_BUILDING) {
				BuildingClass * building = (BuildingClass *)object;

				/*
				**	Special case check to ignore cloaked object if not owned by the player.
				*/
				if (!building->IsOwnedByPlayer && ((building->TranslucencyLevel == 15 && !Map[techno->Center_Coord()].Is_Sensed(PlayerPtr->HeapID)) || building->Class->IsInvisibleInGame)) {
					object = NULL;
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines what a left click at this spot would do.
/// This routine decides both the shape of the mouse cursor and what the click will actually
/// perform. When something is selected, the answer comes from the selected object. Otherwise
/// it reflects whichever special mode the map has been put into -- repair, sell, power,
/// waypoint or super weapon targeting -- and falls back to plain selection.
/// </summary>
/// <param name="cell">The cell the mouse is currently over.</param>
/// <param name="object">The object the mouse is over, or NULL if there is none.</param>
/// <param name="check_fog">Should the action be limited by what the fog of war hides?</param>
/// <returns>Returns with the action that a left click would perform.</returns>
ActionType ScrollClass::What_Action(Cell const & cell, ObjectClass * object, bool check_fog)
{
	ActionType action = ACTION_NONE;

	/*
	**	If there is a currently selected object, then the action to perform if
	**	the left mouse button were clicked must be determined.
	*/
	if (CurrentObject.Count()) {
		if (object != NULL) {
			action = Best_Selected_Object()->What_Action(object);
		} else {
			action = Best_Selected_Object()->What_Action(cell, check_fog);
		}

	} else {

		bool visible = true;
		TechnoClass const * techno = Dynamic_Cast<TechnoClass const *>((AbstractClass const *)object);

		/*
		**	Special case check to ignore cloaked object if not owned by the player.
		*/
		if (techno != NULL && !techno->IsOwnedByPlayer && ((techno->Cloak == CLOAKED && !Map[techno->Center_Coord()].Is_Sensed(PlayerPtr->HeapID)) || techno->TClass->IsInvisible)) {
			visible = false;
		}

		if (object != NULL && object->RTTI == RTTI_BUILDING) {
			BuildingClass * building = (BuildingClass *)object;

			/*
			**	Special case check to ignore cloaked object if not owned by the player.
			*/
			if (!building->IsOwnedByPlayer && (building->TranslucencyLevel == 15 && !Map[techno->Center_Coord()].Is_Sensed(PlayerPtr->HeapID))) {
				visible = false;
			}
		}

		if (visible && object != NULL && object->Class_Of() != NULL && object->Class_Of()->IsSelectable && (object->RTTI != RTTI_BUILDING || !((BuildingClass *)object)->IsFogged) && (techno == NULL || !techno->IsALoaner)) {
			action = ACTION_SELECT;
		}

		if (Map.IsRepairMode) {
			if (object != NULL && object->Owner_HouseClass() != NULL && object->Owner_HouseClass()->Is_Player_Control() && object->Can_Repair()) {
				action = ACTION_REPAIR;
			} else {
				action = ACTION_NO_REPAIR;
			}
		}

		if (Map.IsPowerMode) {
			action = ACTION_NO_TOGGLE_POWER;
			if (object != NULL && object->Owner_HouseClass() != NULL && object->Owner_HouseClass()->Is_Player_Control() && object->RTTI == RTTI_BUILDING && object->Class_Of()->IsSelectable && !object->Considered_Vehicle()) {
				BuildingTypeClass * bclass = ((BuildingClass *)object)->Class;
				if (bclass->IsCanTogglePower && (bclass->Drain > 0 || bclass->IsPowered)) {
					action = ACTION_TOGGLE_POWER;
				}
			}
		}

		WaypointClass * waypoint = PlayerPtr->Waypoint_At(cell);
		if (Map.IsWaypointMode) {

			bool shiftdown = Keyboard->Down(Options.KeySelect1) || Keyboard->Down(Options.KeySelect2);
			PathType path = PATH_NONE;
			char waypoint_id;

			if (waypoint != NULL) {
				PlayerPtr->Fetch_Waypoint_Data(waypoint, path, waypoint_id);
			}

			/*
			 * Dragging a waypoint takes priority. Otherwise, shift-clicking the end
			 * of the currently selected path loops it, clicking any waypoint selects
			 * it, and clicking an empty cell places a new waypoint if possible.
			 */
			if (Map.DraggedWaypoint != NULL) {
				action = ACTION_DRAG_WAYPOINT;

			} else if (shiftdown && waypoint != NULL && path == PlayerPtr->SelectedPath &&
					PlayerPtr->Can_Add_Waypoint_To_Path() && PlayerPtr->Paths[path]->Get_Next_Waypoint(waypoint) != NULL) {
				action = ACTION_LOOP_WAYPOINT_PATH;

			} else if (waypoint != NULL) {
				action = ACTION_SELECT_WAYPOINT;

			} else if (PlayerPtr->SelectedPath != PATH_NONE && PlayerPtr->Can_Add_Waypoint_To_Path() && Map.In_Local_Radar(cell)) {
				action = ACTION_PLACE_WAYPOINT;

			} else {
				action = ACTION_NO_PLACE_WAYPOINT;
			}
		}

		if (Map.IsSellMode) {
			if (object != NULL && object->Owner_HouseClass() != NULL && object->Owner_HouseClass()->Is_Player_Control() && object->Can_Demolish()) {
				if (object->RTTI == RTTI_BUILDING) {
					if (object->Considered_Vehicle()) {
						action = ACTION_NO_SELL;
					} else {
						action = ACTION_SELL;
					}
				} else {
					action = ACTION_SELL_UNIT;
				}
			} else {

				/*
				**	Check to see if the cursor is over an owned wall.
				*/
				Coord coord = cell;
				coord.Z = Map.Get_Height_GL(coord);
				if (!Map.Is_Shrouded(coord) && !Map.Is_Fogged(coord) &&
					Map[cell].Overlay != OVERLAY_NONE &&
					OverlayTypes[Map[cell].Overlay]->IsWall &&
					Map[cell].Owner != HOUSE_NONE && Houses[Map[cell].Owner]->Is_Player_Control()) {
					action = ACTION_SELL;
				} else {
					action = ACTION_NO_SELL;
				}
			}
		}

		if (Map.IsTargettingMode != SUPER_NONE) {
			ActionType super_action = SuperWeaponTypes[Map.IsTargettingMode]->What_Action(cell, object);
			if (super_action != ACTION_NONE) {
				action = super_action;
			}
		}

		if (action == ACTION_NONE && waypoint != NULL) {
			action = ACTION_ENTER_WAYPOINT_MODE;
		}

		if (Map.PendingObject != NULL) {
			action = ACTION_NONE;
		}
	}

	return(action);
}


/// <summary>
/// Handles scrolling the map when the mouse rests against a screen edge.
/// The direction is taken from where the cursor sits, biased toward the cardinal directions
/// so that the corners are not too easy to hit by accident. The cursor is set to the matching
/// scroll arrow, or its barred version when the map cannot travel that way. Scrolling gathers
/// speed the longer the cursor is held at the edge and coasts back down once it leaves.
/// </summary>
/// <param name="point">The current mouse position, relative to the tactical view.</param>
void ScrollClass::Scroll_Edge(Point2D const & point)
{
	/*
	 * If mouse is down, then don't allow edge scrolling of the tactical map.
	 */
	if (IsMouseDown != true) {
		/*
		**	Special check to not scroll within the special no-scroll regions.
		*/
		bool noscroll = false;

		if (!noscroll) {
			Point2D p = TacticalRect.Top_Left() + point;
			int x = p.X;
			int y = p.Y;
			int w = (CompositeSurface->Get_Width()+SidebarSurface->Get_Width()) - 1;
			int h = CompositeSurface->Get_Height() - 1;

			bool at_screen_edge = (y <= 0 || x == 0 || x >= w || y >= h);

			bool player_scrolled=false;

			static 	DirType	dir;
			static 	Dir256	direction;

			/*
			**	Verify that the mouse is over a scroll region.
			*/
			if (Inertia || at_screen_edge) {
				if (at_screen_edge) {

					player_scrolled=true;

					int width = VideoModeWidth;
					int height = VideoModeHeight;

					static double wratio = 0.16;
					static double hratio = 0.21;

					/*
					**	Adjust the mouse coordinates to emphasize the
					**	cardinal directions over the diagonals.
					*/
					int altx = x;
					if (altx < width * wratio) {
						altx = 0;
					} else if (altx > width * (1.0 - wratio)) {
						altx = width - 1;
					} else {
						altx = width / 2;
					}

					int alty = y;
					if (alty < height * hratio) {
						alty = 0;
					} else if (alty > height * (1.0 - hratio)) {
						alty = height - 1;
					} else {
						alty = height / 2;
					}

					dir = Direction(Point2D((VisibleRect.Width/2), (VisibleRect.Height/2)), Point2D(altx, alty));
					direction = dir.As_Dir256();
				}

				int control = Dir_Facing(direction);

				/*
				**	The mouse is over a scroll region so set the mouse shape accordingly if the map
				**	can be scrolled in the direction indicated.
				*/
				static int _rate[9] = {
					0x00E0*2,
					0x00C0*2,
					0x00A0*2,
					0x0080*2,
					0x0060*2,
					0x0040*2,
					0x0020*2,
					0x0010*2,
					0x0008*2
				};

				int rate = 8-Inertia;

				if (rate < Options.ScrollRate+1) {
					rate = Options.ScrollRate+1;
					Inertia = 8-rate;
				}

				/*
				**	Increase the scroll rate if the mouse button is held down.
				*/
				if (Keyboard->Down(KN_RMOUSE)) {
					rate = std::clamp(rate+1, 4, (int)(sizeof(_rate)/sizeof(_rate[0]))-1);
				}

				/*
				**	If options indicate that scrolling should be forced to
				**	one of the 8 facings, then adjust the direction value
				**	accordingly.
				*/
				FacingType facing = Dir_Facing(direction);

				int distance = 1;

				if (!Scroll_Map(facing, distance, false)) {
					Override_Mouse_Shape((MouseType)(MOUSE_NO_N+control), false);
				} else {
					Override_Mouse_Shape((MouseType)(MOUSE_N+control), false);

					int step = int(_rate[rate] * Rule->ScrollMultiplier);
					double scaled = step * _ScrollFraction + _EdgeScrollRemainder;
					distance = int(scaled);
					_EdgeScrollRemainder = scaled - distance;
					Scroll_Map(facing, distance, true);

					if (Counter == 0 && player_scrolled) {
						Counter = SCROLL_DELAY;
						Inertia++;
					}
				}
			}
			if (!player_scrolled) {
				if (!Counter) {
					Inertia--;
					if (Inertia<0) Inertia++;
					Counter = SCROLL_DELAY;
				}
			}
		}
	}
}


/// <summary>Handles the mouse tracking for the tactical map.
/// This routine is called on every input poll. It feeds the current mouse position to the map as
/// a held drag, a coast scroll, or a plain hover that keeps the action cursor up to date, and
/// lets the map scroll when the cursor rests against a screen edge.</summary>
void ScrollClass::Scroll_AI(void)
{
	unsigned int now = Get_Game_Time();
	_ScrollFraction = 0.0;
	if (_LastScrollPollTime != 0) {
		_ScrollFraction = std::min((now - _LastScrollPollTime) * (SCROLL_STEPS_PER_SECOND / 1000.0), MAX_SCROLL_STEPS_PER_POLL);
	}
	_LastScrollPollTime = now;

	if (!IgnoreInput) {
		Point2D tacti = TacticalRect.Top_Left();
		Point2D mouse = MouseCursor->Get_Mouse_Point();
		Point2D point = mouse - tacti;

		if (IsMouseDown == true) {
			if (Keyboard->Down(KN_LMOUSE)) {
				Map.Mouse_Left_Held(point);
			} else if (Keyboard->Down(KN_RMOUSE)) {
				Map.Scroll_Coast(point);
			}
			return;
		} else {
			Cell			cell;						/// cell click happened over
			Coord			coord;						/// coord click happened over
			ObjectClass *	object /*= 0*/;				// what object is in the cell
			bool			fog;						/// is the cell in fog or not
			bool			shadow;						// is the cell in shadow or not
			if (Resolve_Point(point, cell, coord, object, fog, shadow)) {
				Map.Mouse_Left_Up(cell, shadow, object, What_Action(cell, object, true));
			}
			if (IsEdgeScrollAllowed && !Debug_Map) {
				Scroll_Edge(point);
			}
		}
	}
}


/// <summary>
/// Is the tactical map currently in motion?
/// This routine is used to hold off work that would fight with a scroll already under way. A
/// drag operation counts as scrolling just as much as actual map movement does.
/// </summary>
/// <returns>bool; Is the map moving or being dragged about?</returns>
bool ScrollClass::Is_Scrolling(void) const
{
	if (IsMouseDown || IgnoreInput) {
		return(true);
	}
	if (TacticalMap != NULL && TacticalMap->MoveSpeed != 0) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Handles the mouse messages that drive the tactical map.
/// This routine is called from the main window procedure. It turns raw button presses and
/// releases into tactical map actions, and takes and releases the mouse capture so that a
/// drag survives the cursor leaving the window. Messages arriving while the game is not
/// running, or while input is being ignored, are quietly dropped.
/// </summary>
void ScrollClass::Message_Handler(HWND hwnd, UINT & message, UINT & wParam, LONG & lParam)
{
	if (!TacticalActive) {
		return;
	}

	if (!ScenarioActive && !Debug_Map) {
		return;
	}

	if (!GameActive) {
		return;
	}

	if (TacticalMap == NULL) {
		return;
	}

	if (MouseCursor == NULL) {
		return;
	}

	if (SpecialDialog != SDLG_NONE) {
		return;
	}

	if (IgnoreInput && message != WM_CAPTURECHANGED) {
		return;
	}

	Cell			cell;						/// cell click happened over
	Coord			coord;						/// coord click happened over
	ObjectClass *	object;						// what object is in the cell
	bool			fog;						/// is the cell in fog or not
	bool			shadow;						// is the cell in shadow or not
	Point2D			point;						/// click position relative to the tactical view

	switch (message) {

		case WM_LBUTTONDOWN:
			if (IsMouseDown == false) {

				POINTS pts = MAKEPOINTS(lParam);
				point.X = pts.x - TacticalRect.X;
				point.Y = pts.y - TacticalRect.Y;

				if (Resolve_Point(point, cell, coord, object, fog, shadow)) {
					Map.Mouse_Left_Up(cell, shadow, object, What_Action(cell, object, true));
					Map.Mouse_Left_Press(point);
					SetCapture(MainWindow);
					IsMouseDown = true;
				}
			}
			break;

		case WM_LBUTTONUP:
			if (IsMouseDown == true) {

				POINTS pts = MAKEPOINTS(lParam);
				point.X = pts.x - TacticalRect.X;
				point.Y = pts.y - TacticalRect.Y;

				Resolve_Point(point, cell, coord, object, fog, shadow);
				Map.Mouse_Left_Release(coord, cell, object, What_Action(cell, object, false));
				IsMouseDown = false;
				ReleaseCapture();
			}
			break;

		case WM_RBUTTONDOWN:
			if (IsMouseDown == false) {

				POINTS pts = MAKEPOINTS(lParam);
				point.X = pts.x - TacticalRect.X;
				point.Y = pts.y - TacticalRect.Y;

				if (Resolve_Point(point, cell, coord, object, fog, shadow)) {
					Map.Mouse_Right_Press(point);
					SetCapture(MainWindow);
					IsMouseDown = true;
				}
			}
			break;

		case WM_RBUTTONUP:
			if (IsMouseDown == true) {
				Map.Mouse_Right_Release(point);
				BASECLASS::Abort_Drag_Select();
				IsMouseDown = false;
				ReleaseCapture();
			}
			break;

		case WM_CAPTURECHANGED:
			if (((HWND &)lParam) != MainWindow && IsMouseDown == true) {
				Abort_Drag_Select();
			}
			break;
	}
}


/// <summary>
/// Handles coast scrolling while the right mouse button is dragged.
/// Once the mouse has travelled far enough from the press point to count as a drag rather
/// than a click, the map scrolls away from that point at a speed set by how far the mouse has
/// been pulled and by the player's scroll method and rate options. The cursor is updated to
/// show which directions the map can still travel in. Starting a coast scroll cancels any
/// rubber band selection that was under way.
/// </summary>
/// <param name="point">The current mouse position, relative to the tactical view.</param>
void ScrollClass::Scroll_Coast(Point2D const & point)
{
	if (!GameActive) {
		return;
	}

	if (!TacticalActive && !Debug_Map) {
		return;
	}

	if (!IsDragOperation) {
		if (abs(point.X - RightPressPoint.X) > GetSystemMetrics(SM_CXDRAG) * 2
		||	abs(point.Y - RightPressPoint.Y) > GetSystemMetrics(SM_CYDRAG) * 2) {
			IsDragOperation = true;
		}
	}

	if (IsDragOperation) {

		if (!IsCoastScrollAllowed) {
			/*
			**	If rubber band mode is in progress, then don't allow scrolling of the tactical map.
			*/
			if (IsRubberBand) {
				Mouse_Right_Release(Point2D(-1,-1));
			} else {
				IsCoastScrollAllowed = true;
			}
		}

		if (IsCoastScrollAllowed) {

			int posx = point.X - RightPressPoint.X;
			int posy = point.Y - RightPressPoint.Y;

			static FacingType _facing[FACING_COUNT+1] = {
				FACING_NONE,
				FACING_NE,
				FACING_E,
				FACING_SE,
				FACING_S,
				FACING_SW,
				FACING_W,
				FACING_NW,
				FACING_COUNT
			};

			FacingType face = Facing_Between_Points(RightPressPoint, point);
			if (face != _facing[0]) {
				_facing[0] = face;
			}

			int distx = 0;
			int disty = 0;

			switch (Options.Get_Scroll_Method()) {

				case 0: {
					// This offset stands until the hand moves; the methods below warp the
					// pointer back and so consume it once.
					int stepx = abs(int((double)posx * (1.0 / (double)(Options.ScrollRate + 1))));
					int stepy = abs(int((double)posy * (1.0 / (double)(Options.ScrollRate + 1))));

					double scaledx = stepx * _ScrollFraction + _CoastRemainderX;
					double scaledy = stepy * _ScrollFraction + _CoastRemainderY;
					distx = int(scaledx);
					disty = int(scaledy);
					_CoastRemainderX = scaledx - distx;
					_CoastRemainderY = scaledy - disty;
					break;
				}

				case 1:
					distx = abs(int((double)posx * (12.0 / (double)(Options.ScrollRate + 1))));
					disty = abs(int((double)posy * (12.0 / (double)(Options.ScrollRate + 1))));
					if (distx + disty > 0) {
						POINT pt;
						pt.x = RightPressPoint.X + TacticalRect.X;
						pt.y = RightPressPoint.Y + TacticalRect.Y;
						Game_Point_To_Screen(pt);
						SetCursorPos(pt.x, pt.y);
					}
					break;

				case 2:
					posx = -posx;
					posy = -posy;
					distx = abs(int((double)posx * (12.0 / (double)(Options.ScrollRate + 1))));
					disty = abs(int((double)posy * (12.0 / (double)(Options.ScrollRate + 1))));
					if (distx + disty > 0) {
						POINT pt;
						pt.x = RightPressPoint.X + TacticalRect.X;
						pt.y = RightPressPoint.Y + TacticalRect.Y;
						Game_Point_To_Screen(pt);
						SetCursorPos(pt.x, pt.y);
					}
					break;
			}

			int index = 0;
			for (int direction = FACING_FIRST; direction < FACING_COUNT; direction += FACING_90) {
				int dist = 1;
				if (!Map.Scroll_Map((FacingType)direction, dist, false)) {
					index |= 1 << (direction / 2);
				}
			}

			if (posx > 0) {
				Map.Scroll_Map(FACING_E, distx, true);
			} else if (posx < 0) {
				Map.Scroll_Map(FACING_W, distx, true);
			}
			if (posy > 0) {
				Map.Scroll_Map(FACING_S, disty, true);
			} else if (posy < 0) {
				Map.Scroll_Map(FACING_N, disty, true);
			}

			static MouseType _mouse[FACING_COUNT*2] = {
				MOUSE_SCROLL_COASTING,
				MOUSE_SCROLL_COASTING_N,
				MOUSE_SCROLL_COASTING_E,
				MOUSE_SCROLL_COASTING_NE,
				MOUSE_SCROLL_COASTING_S,
				MOUSE_SCROLL_COASTING,
				MOUSE_SCROLL_COASTING_SE,
				MOUSE_SCROLL_COASTING,
				MOUSE_SCROLL_COASTING_W,
				MOUSE_SCROLL_COASTING_NW,
				MOUSE_SCROLL_COASTING,
				MOUSE_SCROLL_COASTING,
				MOUSE_SCROLL_COASTING_SW,
				MOUSE_SCROLL_COASTING,
				MOUSE_SCROLL_COASTING,
				MOUSE_SCROLL_COASTING
			};

			Set_Default_Mouse(_mouse[index], true);
		}
	}
}


/// <summary>
/// Handles the right mouse button being released over the tactical map.
/// A release that ends a coast scroll drag merely restores the normal cursor. Otherwise the
/// base class gets the release and treats it as the ordinary deselect click.
/// </summary>
/// <param name="point">The mouse position, relative to the tactical view.</param>
void ScrollClass::Mouse_Right_Release(Point2D const & point)
{
	if (IsDragOperation) {
		Set_Default_Mouse(MOUSE_NORMAL, Map.IsSmall);
		IsDragOperation = false;
		return;
	}
	BASECLASS::Mouse_Right_Release(point);
}


/// <summary>
/// Handles the right mouse button being pressed over the tactical map.
/// The press point is remembered so that Scroll_Coast can measure any subsequent drag against
/// it. The mouse is shown again in case something had hidden it.
/// </summary>
/// <param name="point">The mouse position, relative to the tactical view.</param>
void ScrollClass::Mouse_Right_Press(Point2D const & point)
{
	if (!IgnoreInput) {
		BASECLASS::Mouse_Right_Press(point);
		RightPressPoint = point;
		IsDragOperation = false;
		Show_Mouse();
	}
}


/// <summary>
/// Aborts any drag operation currently in progress.
/// Besides letting the base class abandon the rubber band, this routine clears the mouse down
/// latch and hands back the mouse capture, so a drag interrupted by another window does not
/// leave the tactical map convinced the button is still held.
/// </summary>
void ScrollClass::Abort_Drag_Select(void)
{
	BASECLASS::Abort_Drag_Select();
	IsMouseDown = false;
	HWND hwnd = GetCapture();
	if (hwnd == MainWindow) {
		ReleaseCapture();
	}
	DebugString("Scroll: Abort_Drag_Select()\n");
}
