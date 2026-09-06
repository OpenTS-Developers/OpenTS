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

/* $Header: /CounterStrike/TERRAIN.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TERRAIN.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 29, 1994                                               *
 *                                                                                             *
 *                  Last Update : October 4, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TerrainClass::AI -- Process the terrain object AI.                                        *
 *   TerrainClass::Can_Enter_Cell -- Determines if the terrain object can exist in the cell.   *
 *   TerrainClass::Catch_Fire -- Catches the terrain object on fire.                           *
 *   TerrainClass::Center_Coord -- Fetches the center point coordinate for terrain object.     *
 *   TerrainClass::Debug_Dump -- Displays the status of the terrain object.                    *
 *   TerrainClass::Draw_It -- Renders the terrain object at the location specified.            *
 *   TerrainClass::Fire_Out -- Handles when fire has gone out.                                 *
 *   TerrainClass::Heath_Ratio -- Determines the health ratio for the terrain object.          *
 *   TerrainClass::Init -- Initialize the terrain object tracking system.                      *
 *   TerrainClass::Limbo -- Handles terrain specific limbo action.                             *
 *   TerrainClass::Mark -- Marks the terrain object on the map.                                *
 *   TerrainClass::Radar_Icon -- Fetches pointer to radar icon to use.                         *
 *   TerrainClass::Read_INI -- Reads terrain objects from INI file.                            *
 *   TerrainClass::Start_To_Crumble -- Initiates crumbling of terrain (tree) object.           *
 *   TerrainClass::Take_Damage -- Damages the terrain object as specified.                     *
 *   TerrainClass::Target_Coord -- Returns with the target coordinate.                         *
 *   TerrainClass::TerrainClass -- This is the constructor for a terrain object.               *
 *   TerrainClass::Unlimbo -- Unlimbo terrain object onto the map.                             *
 *   TerrainClass::Write_INI -- Write all terrain objects to the INI database specified.       *
 *   TerrainClass::delete -- Deletes a terrain object.                                         *
 *   TerrainClass::new -- Creates a new terrain object.                                        *
 *   TerrainClass::~TerrainClass -- Default destructor for terrain class objects.              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "terrain.h"

#include "_convert.h"
#include "_map.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "anim.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "draw.h"
#include "findmake.h"
#include "globals.h"
#include "incdec.h"
#include "inline.h"
#include "lightcon.h"
#include "rect.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "shapeset.h"
#include "sun.h"
#include "tactical.h"
#include "terrtype.h"
#include "tracker.h"
#include "warhead.h"

#include "draw.hh"

#include <cstdio>


char const * const TerrainClass::INI_NAME = "Terrain";


/***********************************************************************************************
 * TerrainClass::~TerrainClass -- Default destructor for terrain class objects.                *
 *                                                                                             *
 *    This is the default destructor for terrain objects. It will remove the object from the   *
 *    map and tracking systems, but only if the game is running. Otherwise, it does nothing.   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TerrainClass::~TerrainClass(void)
{
	Detach_This_From_All(this);
	Terrains.Delete(this);

	if (GameActive && Class) {
		IsActive = true;
		TerrainClass::Limbo();
	}

	TargetTracker.Remove_Index(Fetch_ID());
}


/***********************************************************************************************
 * TerrainClass::Take_Damage -- Damages the terrain object as specified.                       *
 *                                                                                             *
 *    This routine is called when damage is to be inflicted upon the terrain object. It is     *
 *    through this routine that terrain objects are attacked and thereby destroyed. Not all    *
 *    terrain objects can be damaged by this routine however.                                  *
 *                                                                                             *
 * INPUT:   damage      -- The damage points to inflict (raw).                                 *
 *                                                                                             *
 *          warhead     -- The warhead type the indicates the kind of damage. This is used to  *
 *                         determine if the terrain object is damaged and if so, by how much.  *
 *                                                                                             *
 * OUTPUT:  bool; Was the terrain object destroyed by this damage?                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *   11/22/1994 JLB : Shares base damage handler for techno objects.                           *
 *   12/11/1994 JLB : Shortens attached burning animations.                                    *
 *=============================================================================================*/
ResultType TerrainClass::Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced, bool no_crew)
{
	ResultType res = RESULT_NONE;

	if (!warhead) return(RESULT_NONE);

	/*
	**	Small arms can never destroy a terrain element.
	*/
	if (warhead->IsWoodDestroyer && !Class->IsImmune) {

		res = BASECLASS::Take_Damage(damage, distance, warhead, source, forced, no_crew);

		if (res == RESULT_ALREADY_DESTROYED) {
			return(res);
		}

		if (!IsOnFire && damage > 0 && warhead->IsSparky) {
			Catch_Fire();
		}

		/*
		**	If the terrain object is destroyed by this damage, then only remove it if it
		**	currently isn't on fire and isn't in the process of crumbling.
		*/
		if (res == RESULT_DESTROYED) {

			if (Class->IsTiberiumSpawn) {

				static int const _damage = 100;

				new AnimClass(Combat_Anim(_damage, Rule->C4Warhead, Map[Get_Coord()].Land_Type(), Get_Coord()), Get_Coord(), 0, 1, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZGRAD), Get_Explosion_Z(Get_Coord()));
				Explosion_Damage(Get_Coord(), _damage, NULL, Rule->C4Warhead, true);
				Chain_Reaction_Damage(Get_Cell());
			} else if (IsOnFire) {

				/*
				**	Attached flame animation should be shortened as much as possible so that
				**	crumbling can begin soon.
				*/
				Shorten_Attached_Anims(this);
			} else {
				Start_To_Crumble();
			}

			TacticalMap->Register_Dirty_Area(Get_Render_Rect(), false);

			/*
			**	Remove this terrain object from the targeting computers of all other
			**	game objects. No use beating a dead horse.
			*/

			Detach_All();
			Delete_Me();
		}
	}
	return(res);
}


/***********************************************************************************************
 * TerrainClass::TerrainClass -- This is the constructor for a terrain object                  *
 *                                                                                             *
 *    This constructor for a terrain object will initialize the terrain                        *
 *    object with it's proper type and insert it into the access                               *
 *    tracking system.                                                                         *
 *                                                                                             *
 * INPUT:   type  -- The terrain object type.                                                  *
 *                                                                                             *
 *          cell  -- The location of the terrain object.                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TerrainClass::TerrainClass(TerrainTypeClass const * type, Cell const & cell) :
	BASECLASS(),
	Class((TerrainTypeClass *)type),
	IsOnFire(false),
	IsCrumbling(false),
	Unused1(0),
	Unused2(0),
	RenderPixelPos(0,0)
{
	Create_ID();
	Strength = Class->MaxStrength;
	if (cell != CELL_NONE) {
		if (!Unlimbo(Coord(cell))) {
			Delete_Me();
		}
	}

	Set_Rate(0);	// turn off animation
	Terrains.Add(this);
	TargetTracker.Add_Index(Fetch_ID(), this);
}


/// <summary>
/// Default constructor for a terrain object.
/// This routine is used when the object will be filled in from somewhere else, such as
/// by the load process. The object still adds itself to the terrain list and the target
/// tracker, exactly as a normally constructed one would.
/// </summary>
TerrainClass::TerrainClass(void) :
	BASECLASS(),
	Class(NULL),
	IsOnFire(false),
	IsCrumbling(false),
	Unused1(0),
	Unused2(0),
	RenderPixelPos(0,0)
{
	Terrains.Add(this);
	TargetTracker.Add_Index(Fetch_ID(), this);
}


/***********************************************************************************************
 * TerrainClass::Mark -- Marks the terrain object on the map.                                  *
 *                                                                                             *
 *    This routine will mark or remove the terrain object from the map                         *
 *    tracking system. This is typically called when the terrain object                        *
 *    is first created, when it is destroyed, and whenever it needs to be                      *
 *    redrawn.                                                                                 *
 *                                                                                             *
 * INPUT:   mark  -- The marking operation to perform.                                         *
 *                                                                                             *
 * OUTPUT:  bool; Was the terrain object successfully marked?                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/02/1994 JLB : Created.                                                                 *
 *   12/23/1994 JLB : Performs low level legality check before proceeding.                     *
 *=============================================================================================*/
bool TerrainClass::Mark(MarkType mark)
{
	if (BASECLASS::Mark(mark)) {
		Cell cell = Get_Cell();

		switch (mark) {
			case MARK_UP:
				Map.Pick_Up(cell, this);
				break;

			case MARK_DOWN:
			case MARK_DOWN_FORCED:
				Map.Place_Down(cell, this);
				break;

			default:
				break;
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the cell sub-positions this terrain object fills in the current theater.
/// The type carries a temperate and a snow figure, and the theater picks between them.
/// </summary>
/// <returns>Returns with the occupation bits, one per sub-position.</returns>
int TerrainClass::Occupation_Bits(void) const
{
	if (TheaterClass::As_Reference(Scen->Theater).IsArctic) {
		return(Class->SnowOccupationBits);
	}
	return(Class->TemperateOccupationBits);
}


/// <summary>
/// Clears the occupation bits for this terrain object in the cell.
/// This routine releases the sub-positions of the cell that the terrain object was
/// filling, so that infantry may stand there once the object is gone. Which
/// sub-positions are freed comes from the type class and differs between theaters.
/// </summary>
/// <param name="coord">The coordinate of the cell to clear.</param>
void TerrainClass::Clear_Occupy_Bit(Coord const & coord)
{
	int bits = Occupation_Bits();

	CellClass &cell = Map[coord.As_Cell()];

	if (bits & 1) {
		cell.Flag.Composite &= ~(1 << 2);
	}

	if (bits & 2) {
		cell.Flag.Composite &= ~(1 << 3);
	}

	if (bits & 4) {
		cell.Flag.Composite &= ~(1 << 4);
	}
}


/// <summary>
/// Sets the occupation bits for this terrain object in the cell.
/// This routine marks the sub-positions of the cell that the terrain object physically
/// fills, so that infantry cannot stand where the object is. Which sub-positions are
/// blocked comes from the type class and differs between theaters.
/// </summary>
/// <param name="coord">The coordinate of the cell to mark.</param>
void TerrainClass::Set_Occupy_Bit(Coord const & coord)
{
	int bits = Occupation_Bits();

	CellClass &cell = Map[coord.As_Cell()];

	if (bits & 1) {
		cell.Flag.Composite |= (1 << 2);
	}

	if (bits & 2) {
		cell.Flag.Composite |= (1 << 3);
	}

	if (bits & 4) {
		cell.Flag.Composite |= (1 << 4);
	}
}


/***********************************************************************************************
 * TerrainClass::Draw_It -- Renders the terrain object at the location specified.              *
 *                                                                                             *
 *    This routine is used to render the terrain object at the location specified and          *
 *    clipped to the window specified. This is the gruntwork drawing routine for the           *
 *    terrain objects as they are displayed on the map.                                        *
 *                                                                                             *
 * INPUT:   x,y      -- The coordinate to draw the terrain object at (centered).               *
 *                                                                                             *
 *          window   -- The clipping window to draw to.                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/27/1994 JLB : Created.                                                                 *
 *   11/09/1994 JLB : Changed selected terrain highlight method.                               *
 *=============================================================================================*/
void TerrainClass::Draw_It(Point2D const & point, Rect const & cliprect) const
{
	ShapeSet const * shapedata;

	Cell cell = Get_Cell();
	CellClass & cellptr = Map[cell];

	shapedata = (ShapeSet const *)Get_Image_Data();
	if (shapedata) {
		int	shapenum = 0;

		/*
		**	Determine the animation stage to render the terrain object. If it is crumbling, then
		**	it will display the crumbling animation.
		*/
		if (Class->IsAnimated) {
			shapenum = Fetch_Stage();
		} else if (IsCrumbling) {
			shapenum = Fetch_Stage() + IsCrumbling;
		} else {
			if (Strength < 2) {
				shapenum++;
			}
		}

		Point2D drawpoint = point;
		int zadjust = -TacticalMap->Z_Lepton_To_Pixel(Height);

		if (cellptr.Drawer == NULL) {
			cellptr.Init_Drawer();
		}

		drawpoint.Y += Class->YDrawFudge;
		zadjust += Class->YDrawFudge / 3;

		int tint;
		ConvertClass * drawer;

		if (Class->IsTiberiumSpawn) {
			drawer = TiberiumDrawer;
			tint = cellptr.Brightness;
			drawpoint -= Point2D(0,16);
		} else {
			drawer = cellptr.Drawer;
			tint = cellptr.TileBrightness;
		}

		ShapeFlags_Type flags = ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA);
		if (!Is_Animating()) {
			flags = ShapeFlags_Type(flags | SHAPE_ZWRITE);
		}
		Draw_Shape(*LogicalSurface, *drawer, shapedata, shapenum, drawpoint, cliprect, flags, NULL, zadjust - 12, ZGRAD_90DEG, tint);
		if (DrawShapeShadows) {
			Draw_Shape(*LogicalSurface, *cellptr.Drawer, shapedata, shapenum + shapedata->Get_Count() / 2, drawpoint, cliprect, ShapeFlags_Type(flags|SHAPE_DARKEN), NULL, zadjust - 2);
		}
	}
}


/// <summary>
/// Renders the terrain object without writing to the Z buffer.
/// This is the companion to Draw_It for objects that must not leave depth information
/// behind them, since a shape that changes from frame to frame would otherwise stamp a
/// stale silhouette into the buffer. The object's shadow is drawn along with it when
/// shape shadows are enabled.
/// </summary>
/// <param name="point">The screen location to draw the object at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void TerrainClass::Editor_Draw_It(Point2D const & point, Rect const & cliprect) const
{
	ShapeSet const * shapedata;

	Cell cell = Get_Cell();
	CellClass & cellptr = Map[cell];

	shapedata = (ShapeSet const *)Get_Image_Data();
	if (shapedata) {
		int	shapenum = 0;

		/*
		**	Determine the animation stage to render the terrain object. If it is crumbling, then
		**	it will display the crumbling animation.
		*/
		if (IsCrumbling) {
			shapenum = Fetch_Stage() + IsCrumbling;
		}

		Point2D drawpoint = point;
		int zadjust = -TacticalMap->Z_Lepton_To_Pixel(Height);

		if (cellptr.Drawer == NULL) {
			cellptr.Init_Drawer();
		}

		drawpoint.Y += Class->YDrawFudge;
		zadjust += Class->YDrawFudge;

		int tint;
		ConvertClass * drawer;

		if (Class->IsTiberiumSpawn) {
			drawer = TiberiumDrawer;
			tint = cellptr.Brightness;
			drawpoint -= Point2D(0,16);
		} else {
			drawer = cellptr.Drawer;
			tint = cellptr.TileBrightness;
		}

		ShapeFlags_Type flags = ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA);
		Draw_Shape(*LogicalSurface, *drawer, shapedata, shapenum, drawpoint, cliprect, flags, NULL, zadjust - 4, ZGRAD_90DEG, tint);
		if (DrawShapeShadows) {
			flags = ShapeFlags_Type(flags & ~SHAPE_ALPHA);
			Draw_Shape(*LogicalSurface, *cellptr.Drawer, shapedata, shapenum + shapedata->Get_Count() / 2, drawpoint, cliprect, ShapeFlags_Type(flags|SHAPE_DARKEN), NULL, zadjust - 2);
		}
	}
}


/***********************************************************************************************
 * TerrainClass::Can_Enter_Cell -- Determines if the terrain object can exist in the cell.     *
 *                                                                                             *
 *    This routine will examine the cell specified and determine if the the terrain object     *
 *    can legally exist there.                                                                 *
 *                                                                                             *
 * INPUT:   cell  -- The cell to examine.                                                      *
 *                                                                                             *
 * OUTPUT:  If the terrain object can be placed in the cell specified, then a value less than  *
 *          256 will be returned.                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *   01/01/1995 JLB : Actually works now.                                                      *
 *=============================================================================================*/
MoveType TerrainClass::Can_Enter_Cell(CellClass const * cell, FacingType, int cell_height, CellClass const *, bool) const
{
	Cell const * offset = Occupy_List();		// Pointer to cell offset list.
	Cell location = cell->CellID;

	while (*offset != REFRESH_EOL) {
		if (Class->IsWaterBased) {
			if (!Map[location + *offset++].Is_Clear_To_Build(SPEED_FLOAT)) {
				return(MOVE_NO);
			}
		} else {
			if (!Map[location + *offset++].Is_Clear_To_Build()) {
				return(MOVE_NO);
			}
		}
	}
	return(MOVE_OK);
}


/***********************************************************************************************
 * TerrainClass::Catch_Fire -- Catches the terrain object on fire.                             *
 *                                                                                             *
 *    This routine is called if the terrain object is supposed to catch on fire. The routine   *
 *    performs checking to make sure that only flammable terrain objects that aren't already   *
 *    on fire get caught on fire.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the terrain object caught on fire by this routine?                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1994 JLB : Created.                                                                 *
 *   12/11/1994 JLB : Don't catch fire if already on fire or crumbling.                        *
 *=============================================================================================*/
bool TerrainClass::Catch_Fire(void)
{
	if (!IsCrumbling && !IsOnFire && Class->Armor == ARMOR_WOOD && !Class->IsTiberiumSpawn) {
		int randomnum = Scen->RandomNumber();
		AnimClass * anim = new AnimClass(Rule->TreeFire[randomnum & 1], Center_Coord() + Coord(0, 0, 80), 0, 255);
		if (anim) {
			anim->Attach_To(this);
		}
		anim->ZAdjust -= 20;
		IsOnFire = true;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TerrainClass::Fire_Out -- Handles when fire has gone out.                                   *
 *                                                                                             *
 *    When the fire has gone out on a burning terrain object, this routine is called. The      *
 *    animation has already been terminated prior to calling this routine. All this routine    *
 *    needs to perform is any necessary local flag updating.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TerrainClass::Fire_Out(void)
{
	if (IsOnFire) {
		IsOnFire = false;
		if (!IsCrumbling && !Strength) {
			Detach_All();
			Mark(MARK_CHANGE);
			Start_To_Crumble();
		}
	}
}


/***********************************************************************************************
 * TerrainClass::AI -- Process the terrain object AI.                                          *
 *                                                                                             *
 *    This is used to handle any AI processing necessary for terrain objects. This might       *
 *    include animation effects.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1994 JLB : Created.                                                                 *
 *   09/28/1994 JLB : Crumbling animation.                                                     *
 *   08/12/1996 JLB : Reset map zone when terrain object destroyed.                            *
 *   10/04/1996 JLB : Growth speed regulated by rules.                                         *
 *=============================================================================================*/
void TerrainClass::AI(void)
{
	BASECLASS::AI();

	if (Class->IsAnimated) {
		if (Fetch_Rate() == 0) {
			double r = (abs(Scen->RandomNumber()) % 1000000) / 1000000.0;
			if (r < Class->AnimationProbability) {
				Set_Stage(0);
				Set_Rate(Class->AnimationRate);
			}
		}
	}

	if (StageClass::Graphic_Logic()) {

		/*
		**	If the terrain object is in the process of crumbling, then when at the
		**	last stage of the crumbling animation, delete the terrain object.
		*/
		if (IsCrumbling && Fetch_Stage() == (((ShapeSet const *)Class->Get_Image_Data())->Get_Count())-1) {
			Delete_Me();
			return;
		}

		if (Class->IsTiberiumSpawn && Class->IsAnimated && Fetch_Stage() == (((ShapeSet const *)Class->Get_Image_Data())->Get_Count() / 2)) {
			Set_Stage(0);
			Set_Rate(0);
			Map[Get_Coord()].Spread_Tiberium(true);
		}
	}

	if (IsOnFire) {
		static int const _interval = 100;

		if (abs(Scen->RandomNumber()) % _interval == 0) {
			CellClass & cellptr = Map[Get_Coord()];
			for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
				CellClass & adjacent = cellptr.Adjacent_Cell(facing);
				TerrainClass * terrain = adjacent.Cell_Terrain();
				if (terrain && !terrain->IsOnFire && Random_Double(0.0, 1.0) < Rule->TreeFlammability) {
					terrain->Catch_Fire();
				}
			}
		}
	}
}


#ifdef _DEBUG
/***********************************************************************************************
 * TerrainClass::Debug_Dump -- Displays the status of the terrain object.                      *
 *                                                                                             *
 *    This debugging support routine is used to display the status of the terrain object to    *
 *    the debug screen.                                                                        *
 *                                                                                             *
 * INPUT:   mono  -- The mono screen to display the status to.                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TerrainClass::Debug_Dump(MonoClass * mono) const
{
	BASECLASS::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * TerrainClass::Start_To_Crumble -- Initiates crumbling of terrain (tree) object.             *
 *                                                                                             *
 *    This routine is used to start the crumbling process for terrain object. This only        *
 *    applies to trees.                                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TerrainClass::Start_To_Crumble(void)
{
	if (!IsCrumbling) {
		IsCrumbling = true;
		Set_Rate(2);
		Set_Stage(0);
	}
}


/***********************************************************************************************
 * TerrainClass::Limbo -- Handles terrain specific limbo action.                               *
 *                                                                                             *
 *    This routine (called as a part of the limbo process) will remove the terrain occupation  *
 *    flag in the cell it occupies.                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the terrain object unlimboed?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TerrainClass::Limbo(void)
{
	if (!IsInLimbo) {
		Cell cell = Get_Cell();
		FacingType facing = FACING_FIRST;
		while (facing < FACING_COUNT) {
			CellClass & c = Map[Adjacent_Cell(cell, facing)];
			c.AdjacentObjectCount--;
			facing++;
		}
		Map[Get_Coord()].Flag.Occupy.Monolith = false;
	}
	Cell cell = Get_Cell();
	bool result = BASECLASS::Limbo();
	Map[cell].Recalc_Attributes();
	if (!ScenarioInit) {
		Map.Update_Cell_Zone(cell);
		Map.Update_Cell_Subzones(cell);
		Map.Radar_Background(cell);
	}
	return(result);
}


/***********************************************************************************************
 * TerrainClass::Read_INI -- Reads terrain objects from INI file.                              *
 *                                                                                             *
 *    This routine reads a scenario control INI file and creates all                           *
 *    terrain objects specified therein. Objects so created are placed                         *
 *    upon the map.                                                                            *
 *                                                                                             *
 *      INI entry format:                                                                      *
 *      cellnum = TypeName, Triggername                                                        *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the loaded scenario INI file data.                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TerrainClass::Read_INI(CCINIClass const & ini)
{
	TerrainClass * tptr;

	int len = ini.Entry_Count(INI_NAME);

	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);
		TerrainTypeClass const * terrain = TGet_Class<TerrainTypeClass>(ini, INI_NAME, entry, NULL);

		Cell cell;
		if (NewINIFormat >= 4) {
			int val = atoi(entry);
			cell = Cell(val % 1000, val / 1000);
		} else {
			int val = atoi(entry);
			cell = Cell(val % 128, val / 128);
		}

		if (terrain != NULL) {
			tptr = new TerrainClass(terrain, cell);
		}
	}
}


/***********************************************************************************************
 * TerrainClass::Write_INI -- Write all terrain objects to the INI database specified.         *
 *                                                                                             *
 *    This routine will clear out any old terrain data from the INI database and then          *
 *    fill it in with all the data from the terrain objects that currently exists.             *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to store the terrain objects in.            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TerrainClass::Write_INI(CCINIClass & ini)
{
	/*
	**	First, clear out all existing terrain data from the ini file.
	*/
	ini.Clear(INI_NAME);

	/*
	**	Write the terrain data out.
	*/
	for (int index = 0; index < Terrains.Count(); index++) {
		TerrainClass * terrain;

		terrain = Terrains[index];
		if (terrain != NULL && !terrain->IsInLimbo && terrain->IsActive) {
			char	uname[10];
			Cell cell = terrain->Get_Cell();
			sprintf(uname, "%d", cell.X + cell.Y * 1000);
			TPut_Class<TerrainTypeClass>(ini, INI_NAME, uname, terrain->Class);
		}
	}
}


/// <summary>
/// Draws this terrain object if it is due to be redrawn.
/// This routine is called by the display system for each terrain object in the redraw
/// list. It handles the visibility and clipping tests and then hands the actual drawing
/// off to Draw_It.
/// </summary>
/// <param name="cliprect">The clipping rectangle to draw within. It is narrowed to the
/// tactical map before use.</param>
/// <param name="forced">Should the object be drawn even if it is not flagged for
/// redraw?</param>
/// <returns>bool; Was the terrain object drawn?</returns>
bool TerrainClass::Render(Rect & cliprect, bool forced, bool extras_only) const
{
	assert(this != NULL);

	if (Debug_Map || !MainWindow || ((forced || IsToDisplay) && IsDown && !IsInLimbo)) {
		IsToDisplay = false;

		Point2D point;

		cliprect = Intersect(cliprect, TacticalRect);

		Rect rect = ((TerrainClass *)this)->Get_Render_Rect() + TacticalRect.Top_Left();

		if (cliprect.Is_Overlapping(rect)) {

			TacticalMap->Coord_To_Pixel(Render_Coord(), point);

			if (cliprect.X > TacticalRect.X) {
				point.X += TacticalRect.X - cliprect.X;
			}
			if (cliprect.Y > TacticalRect.Y) {
				point.Y += TacticalRect.Y - cliprect.Y;
			}

			/*
			**	Draw the object itself
			*/
			Draw_It(point, cliprect);

#ifdef _DEBUG
			/*
			**	Draw the trigger attached to the object. Draw_It is window-
			**	relative, so add the window's x-coord to 'x'.
			*/
			if (Debug_Map && Tag != NULL) {
				/*Fancy_Text_Print(Trigger->Class->IniName,
					x + (WinX), y,
					&ColorRemaps[PCOLOR_RED], TBLACK,
					TPF_CENTER | TPF_NOSHADOW | TPF_6POINT);*/
			}
#endif

			return(true);
		}
	}

	return(false);
}


/// <summary>
/// Loads this terrain object from the specified stream.
/// The object carries a different identity once it has been read, so its registration
/// under the identity it was constructed with is dropped before the members arrive.
/// </summary>
/// <param name="stream">The stream to read the object from.</param>
/// <returns>Returns with S_OK if the object was read successfully.</returns>
HRESULT TerrainClass::Load(SaveStreamClass & stream)
{
	TargetTracker.Remove_Index(Fetch_ID());

	return(BASECLASS::Load(stream));
}


/// <summary>
/// Re-registers this terrain object under its loaded identity.
/// </summary>
void TerrainClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	TargetTracker.Add_Index(Fetch_ID(), this);
}


/// <summary>
/// Lists the members this terrain object carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TerrainClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);
	StageClass::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(IsOnFire);
	stream.Serialize(IsCrumbling);
	stream.Serialize(Unused1);
	stream.Serialize(Unused2);
	stream.Serialize(RenderPixelPos);
}


/// <summary>
/// Is this terrain object animating?
/// A terrain type flagged as animated always is, and so is any object that has begun to
/// crumble. The draw routines consult this to decide whether the object is allowed to
/// write into the Z buffer.
/// </summary>
/// <returns>bool; Is the terrain object animating?</returns>
bool TerrainClass::Is_Animating(void) const
{
	return(Class->IsAnimated || IsCrumbling);
}


/// <summary>
/// Adds this terrain object's state to the running checksum.
/// The multiplayer sync checker uses this to notice when the games have drifted apart.
/// </summary>
/// <param name="crc">The checksum engine to submit this object's state to.</param>
void TerrainClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc((RTTIType)Class->RTTI);
	crc(Class->Fetch_ID());
	crc(IsOnFire);
	crc(IsCrumbling);
	crc(Unused1);
	crc(Unused2);
}


/// <summary>
/// Removes any reference this object has to the specified target.
/// This routine is called when some other object is about to disappear, so that this
/// terrain object is not left holding a pointer to it.
/// </summary>
/// <param name="target">Pointer to the object that is going away.</param>
/// <param name="all">Should every kind of reference be severed?</param>
void TerrainClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);
	if (Class == target) {
		Class = NULL;
	}
}


/***********************************************************************************************
 * TerrainClass::Unlimbo -- Unlimbo terrain object onto the map.                               *
 *                                                                                             *
 *    This routine is used to unlimbo the terrain object onto a location on the map. Normal    *
 *    unlimbo procedures are sufficient except that the coordinate location of a terrain       *
 *    object is based on the upper left corner of a cell rather than the center. Mask the      *
 *    coordinate value so that it snaps to the upper left corner and then proceed with a       *
 *    normal unlimbo process.                                                                  *
 *                                                                                             *
 * INPUT:   coord    -- The coordinate to mark as the terrain's location.                      *
 *                                                                                             *
 *          dir      -- unused                                                                 *
 *                                                                                             *
 * OUTPUT:  bool; Was the terrain object successful in the unlimbo process? Failure could be   *
 *                the result of illegal positioning.                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1994 JLB : Created.                                                                 *
 *   11/16/1994 JLB : Checks for theater legality.                                             *
 *=============================================================================================*/
bool TerrainClass::Unlimbo(Coord const & coord, Dir256 dir)
{
	if (BASECLASS::Unlimbo(coord, dir)) {
		Cell cell = coord.As_Cell();
		for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
			CellClass * cptr = &Map[Adjacent_Cell(cell, facing)];
			cptr->AdjacentObjectCount++;
		}

		TacticalMap->Coord_To_Pixel(Render_Coord(), RenderPixelPos);
		RenderPixelPos += Point2D(TacticalMap->TacPixelX, TacticalMap->TacPixelY);
		RenderPixelPos.Y += Class->YDrawFudge;

		if (Class->IsTiberiumSpawn) {
			Map[coord].Overlay = OVERLAY_NONE;
			Map[coord].OverlayData = 0;
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the type class of this terrain object.
/// </summary>
/// <returns>Returns with a pointer to the terrain type this object was created from.</returns>
ObjectTypeClass const * TerrainClass::Class_Of(void) const
{
	return(Class);
}


/// <summary>
/// Fetches the screen rectangle this terrain object draws into.
/// The rectangle covers the object's shape and its shadow both, so that the render
/// logic can tell whether any part of the object falls inside the clipping rectangle.
/// </summary>
/// <returns>Returns with the screen rectangle occupied. If the object has no shape data,
/// RECT_NONE is returned.</returns>
Rect TerrainClass::Get_Render_Rect(void)
{
	Point2D drawpoint = RenderPixelPos - Point2D(TacticalMap->TacPixelX, TacticalMap->TacPixelY);
	ShapeSet const * sdata = (ShapeSet const *)TerrainClass::Get_Image_Data();
	if (sdata == NULL) {
		return(RECT_NONE);
	}

	Rect rect1 = sdata->Get_Rect(0);
	Rect rect2 = sdata->Get_Rect(sdata->Get_Count() / 2);

	int width = sdata->Get_Width();
	int height = sdata->Get_Height();

	Rect a = Union(rect1, rect2);
	return(Rect(drawpoint.X + a.X - width / 2, drawpoint.Y + a.Y - height / 2, a.Width, a.Height));
}


/// <summary>
/// Fetches the run time type of this object.
/// </summary>
/// <returns>Returns with RTTI_TERRAIN.</returns>
RTTIType TerrainClass::Fetch_RTTI(void) const
{
	return(RTTI_TERRAIN);
}


/// <summary>
/// Fetches the class identifier for this object.
/// This routine is part of the IPersistent implementation. The save system records
/// the identifier so that it knows what to recreate when the game is loaded back in.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE TerrainClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TerrainClass;
	return(S_OK);
}
