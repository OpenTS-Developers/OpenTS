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

/* $Header: /CounterStrike/OBJECT.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OBJECT.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 29, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 29, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstract.h"
#include "coord.h"
#include "face.h"
#include "globals.h"
#include "point.h"
#include "rect.h"
#include "tag.h"

#include "action.hh"
#include "layer.hh"
#include "mark.hh"
#include "mission.hh"
#include "move.hh"
#include "path.hh"
#include "pcp.hh"
#include "radio.hh"
#include "result.hh"
#include "tevent.hh"
#include "visual.hh"

#include <cassert>

class ObjectClass;
class TechnoClass;
class TechnoTypeClass;
class ObjectTypeClass;
class HouseClass;
class BuildingClass;
class RadioClass;
class TagClass;
class WarheadTypeClass;
class ShapeSet;
class MonoClass;


/**********************************************************************
**	Every game object (that can exist on the map) is ultimately derived from this object
**	class. It holds the common information between all objects. This is primarily the
**	object unique ID number and its location in the world. All common operations between
**	game objects are represented by virtual functions in this class.
*/
class ObjectClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		enum {
			PARACHUTE_MAX_FALL_RATE = -3,
			NO_PARACHUTE_MAX_FALL_RATE = -100,
		};

		/*
		 * This is the rate at which the object is falling, expressed in leptons of vertical
		 * movement per game frame -- only the Z component is meaningful. Gravity pulls it
		 * lower each frame, down to a terminal rate that is far gentler under a parachute.
		 */
		Coord Riser;

		/*
		**	Several objects could exist in the same cell list. This is a pointer to the
		**	next object in the cell list. The objects in this list are not in any
		**	significant order.
		*/
		ObjectClass * Next;

		/*
		 * Every object can be assigned a tag; the same tag can be assigned
		 * to multiple objects.
		 */
		TagClass * Tag;

		/*
		**	This is the current strength of this object.
		*/
		int Strength;

		/*
		**	The object can be in one of two states -- placed down on the map, or not. If the
		**	object is placed down on the map, then this flag will be true.
		*/
		bool IsDown;

		/*
		**	This is a support flag that is only used while building a list of objects to
		**	be damaged by a proximity affect (explosion). When this flag is set, this object
		**	will not be added to the list of units to damage. When damage is applied to the
		**	object, this flag is cleared again. This process ensures that an object is never
		**	subject to "double jeopardy".
		*/
		bool IsToDamage;

		/*
		**	Is this object flagged to be displayed during the next rendering process?  This
		**	flag could be set by many different circumstances. It is automatically cleared
		**	when the object is rerendered.
		*/
		mutable bool IsToDisplay;

		/*
		**	An object in the game may be valid yet held in a state of "limbo". Units are in such
		**	a state if they are being transported or are otherwise "inside" another unit. They can
		**	also be in limbo if they have been created but are being held until the proper time
		**	for delivery.
		*/
		bool IsInLimbo;

		/*
		**	When an object is "selected" it is given a floating bar graph or other graphic imagery
		**	to display this fact. When the player performs I/O, the actions may have a direct
		**	bearing on the actions of the currently selected object. For quick checking purposes,
		**	if this object is the one that is "selected", this flag will be true.
		*/
		bool IsSelected;

		/*
		**	If an animation is attached to this object, then this flag will be true.
		*/
		bool IsAnimAttached;

		/*
		 * If this object is on top of a bridge rather than on the ground underneath it, then
		 * this flag will be true. A cell spanned by a bridge has two levels an object can
		 * occupy, so zone lookups, pathfinding and cell occupancy all need to know which.
		 */
		bool IsOnBridge;

		/*
		**	If this object should process falling logic, then this flag will be true. Such
		**	objects might be ballistic projectiles, grenades, or parachuters.
		*/
		bool IsFalling;

		/*
		 * If this object is to be destroyed the moment it finishes falling, then this flag
		 * will be true. It is set when the object loses whatever was holding it up, both so
		 * that the object cannot simply walk away from the drop and so that its death is
		 * played as a fall -- a splash when it lands in water rather than ground debris.
		 */
		bool IsToExplode;

		/*
		 * If this object is still a live part of the game, then this flag will be true. It
		 * is cleared the moment the object is queued for deletion, which is what lets the
		 * object lists be walked safely while objects on them are being destroyed.
		 */
		bool IsActive;

		/*
		 * This is the display layer this object is currently submitted to, or LAYER_NONE if
		 * it is in none. It records where the object was actually placed so it can be pulled
		 * out of the same layer later, even if In_Which_Layer would now answer differently.
		 */
		LayerType Layer;

		/*
		 * If this object has been submitted to the logic layer, then this flag will be true.
		 * Only submitted objects are given their per-frame chance to think, and the flag
		 * keeps a double submission or a double removal from corrupting the logic list.
		 */
		bool IsSubmittedToLayer;

		/*
		**	The coordinate location of the unit. For vehicles, this is the center
		**	point. For buildings, it is the upper left corner.
		*/
		Coord Position;

		/*-----------------------------------------------------------------------------------
		**	Constructor & destructors.
		*/
		ObjectClass(void);
		virtual ~ObjectClass(void) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		bool operator < (ObjectClass const & object) const;
		bool operator > (ObjectClass const & object) const;

		/*
		**	Query functions.
		*/
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual bool Is_Players_Army(void) const {return(false);}
		virtual VisualType Visual_Character(bool raw = false, HouseClass const * = 0) const {return(VISUAL_NORMAL);}
		virtual void const * Get_Image_Data(void) const;
		virtual ActionType What_Action(ObjectClass const *, bool disallow_force = false) const;
		virtual ActionType What_Action(Cell const &, bool check_fog = false, bool disallow_force = false) const;
		virtual LayerType In_Which_Layer(void) const;
		virtual bool Not_Underground(void) const;
		virtual bool Considered_Vehicle(void) const {return(false);}
		virtual TechnoTypeClass const * Techno_Type_Class(void) const;
		__declspec( property( get=Techno_Type_Class) ) TechnoTypeClass const * TClass;
		virtual ObjectTypeClass const * Class_Of(void) const {return(0);}
		bool Is_Infantry(void) const {return(Fetch_RTTI() == RTTI_INFANTRY);}
		bool Is_Foot(void) const;
		virtual int Get_Ownable(void) const;
		virtual char const * Full_Name(void) const {return("");}
		virtual bool Can_Repair(void) const;
		virtual bool Can_Demolish(void) const;
		virtual bool Can_Player_Fire(void) const;
		virtual bool Can_Player_Move(void) const;

		/*
		**	Coordinate inquiry functions. These are used for both display and
		**	combat purposes.
		*/
		virtual Coord Target_Coord(void) const {return(Center_Coord());}
		virtual Coord Docking_Coord(void) const {return(Center_Coord());}
		virtual Coord Center_Coord(void) const override;
		virtual Coord Render_Coord(void) const {return(Center_Coord());}
		virtual Coord Fire_Coord(int which) const {return(Center_Coord() + Coord(0, 0, 50));}
		virtual Coord Exit_Coord(void) const {return(Center_Coord());}
		virtual int Sort_Y(void) const;
		virtual bool On_Ground(void) const override;
		virtual bool In_Air(void) const override;
		virtual bool Is_Moving_Onto_Bridge(void) const;
		virtual bool Occupies_Cells(void) const {return(true);}

		int Distance(AbstractClass const * target) const;
		int Distance(Coord const & coord) const {return(Center_Coord().Distance_To(coord));}
		int Distance_To(AbstractClass const * obj) const {return(Distance(obj->Center_Coord()));}
		int Planar_Distance(AbstractClass const * target) const;
		int Relative_Distance(AbstractClass const * target) const;
		int Relative_Distance(Coord const & coord) const;

		DirType Direction(AbstractClass const * object) const;

		/*
		**	Object entry and exit from the game system.
		*/
		virtual bool Limbo(void);
		virtual bool Unlimbo(Coord const & , Dir256 facing = DIR_N);
		virtual void Detach(AbstractClass const * target, bool all = true) override;
		virtual void Detach_All(bool all=true);
		virtual void Record_The_Kill(TechnoClass * );
		virtual bool Paradrop(Coord const & coord);
		bool Attach_Tag(TagClass * tag);
		virtual bool Is_Inactive(void) const override;

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		virtual void Fall_From_Height(void);
		virtual void Set_Occupy_Bit(Coord const & coord);
		virtual void Clear_Occupy_Bit(Coord const & coord);
		virtual void Delete_Me(void);
		virtual void Do_Shimmer(void);
		virtual int Exit_Object(TechnoClass *);
		virtual bool Render(Rect &, bool forced, bool extras_only) const;
		virtual Cell const * Occupy_List(bool placement=false) const;

		double Get_Health_Ratio(void) const;
		void Set_Health_Ratio(double health);

		__declspec( property( get=Get_Health_Ratio, put=Set_Health_Ratio ) ) double HealthRatio;

		virtual void Draw_Pre_Render(Point2D const & point, Rect const & cliprect) const { }
		virtual void Draw_Post_Render(Point2D const & point, Rect const & cliprect) const { }
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const { }
		virtual void Editor_Draw_It(Point2D const & point, Rect const & cliprect) const;
		virtual void Hidden(void);
		virtual void Look(bool incremental=false, bool=false);
		virtual bool Mark(MarkType=MARK_CHANGE);

		virtual Rect Get_Visual_Rect(void) const;
		virtual Rect Get_Render_Rect(void);
		virtual void Draw_Radial_Indicator(void) const {};

	private:
		virtual void Mark_For_Redraw(void);

	public:

		/*
		**	User I/O.
		*/
		virtual bool Active_Click_With(ActionType , ObjectClass *, bool);
		virtual bool Active_Click_With(ActionType , Cell const &, bool);
		virtual void Clicked_As_Target(int = 7);
		virtual bool Select(void);
		virtual void Unselect(void);

		/*
		**	Combat related.
		*/
		virtual bool In_Range(Coord const & coord, int which=0) const;
		virtual int Weapon_Range(int =0) const;
		virtual ResultType Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source=0, bool forced=false, bool=false);
		virtual void Scatter(Coord const &, bool forced=false, bool nokidding=false);
		virtual bool Catch_Fire(void);
		virtual void Fire_Out(void);
		virtual int Value(void) const;
		virtual MissionType Get_Mission(void) const;
		virtual void Assign_Mission(MissionType mission) {}

		__declspec( property( get=Get_Mission, put=Assign_Mission ) ) MissionType Mission;

		/*
		**	AI.
		*/
		virtual void Per_Cell_Process(PCPType) {}
		virtual BuildingClass * Who_Can_Build_Me(bool intheory, bool legal) const;
		virtual RadioMessageType Receive_Message(RadioClass * from, RadioMessageType message, intptr_t & param);
		virtual bool Revealed(HouseClass * house);
		virtual void Repair(int);
		virtual void Sell_Back(int);
		virtual void Set_Waypoint_Path(PathType path, char index);
		virtual void AI(void) override;

		virtual void Move(FacingType);
		virtual MoveType Can_Enter_Cell(CellClass const * cell, FacingType dir = FACING_NONE, int cell_height = -1, CellClass const * = 0, bool = true) const {return(MOVE_OK);}
		virtual MoveType Can_Reach(CellClass const * current_cell, FacingType facing, int & cell_height, bool & onto_bridge, CellClass const * adjacent_cell) const {return(MOVE_OK);}

		virtual Coord Get_Coord(void) const {return(Position);}
		virtual void Set_Coord(Coord const & coord);

		__declspec( property( get=Get_Coord, put=Set_Coord ) ) Coord PositionCoord;

		virtual Cell Get_Cell(void) const {return(Position.As_Cell());}

		__declspec( property( get=Get_Cell /*put=*/ ) ) Cell PositionCell;

		virtual CellClass * Get_Cell_Ptr(void) const;
		virtual Cell Get_Target_Cell(void) const;
		virtual CellClass * Get_Target_Cell_Ptr(void) const;

		int Get_Cell_Height(void) const;

		virtual int Get_Height_AGL(void) const;
		virtual void Set_Height_AGL(int);

		__declspec( property( get=Get_Height_AGL, put=Set_Height_AGL ) ) int HeightAGL;

		virtual int Get_Height(void) const;
		void Set_Height(int height);

		__declspec( property( get=Get_Height, put=Set_Height ) ) int Height;

		void Spring_Tag(TEventType event=TEVENT_ANY, ObjectClass * object=NULL, Cell const & cell=CELL_NONE, bool forced=false, TechnoClass *source=NULL)
		{
			if (IsActive && Tag != NULL) {
				Tag->Spring(event, object, cell, forced, source);
			}
		}

		/*
		**	Scenario and debug support.
		*/
#ifdef _DEBUG
		virtual void Debug_Dump(MonoClass *mono) const override;
#endif
};


inline ObjectClass * AbstractClass::As_ObjectClass(void)
{
	return(dynamic_cast<ObjectClass *>(this));
}


inline ObjectClass const * AbstractClass::As_ObjectClass(void) const
{
	return(dynamic_cast<ObjectClass const *>(this));
}


inline ObjectClass * As_Object(AbstractClass * target)
{
	return(dynamic_cast<ObjectClass *>(target));
}

Coord Vector_Center(DynamicVectorClass<ObjectClass *> const & list);
ObjectClass * Vector_Closest_Object(DynamicVectorClass<ObjectClass *> const & list, Coord const & coord);
