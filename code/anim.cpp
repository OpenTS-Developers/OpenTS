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

/* $Header: /CounterStrike/ANIM.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Dune                                                         *
 *                                                                                             *
 *                    File Name : ANIM.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : June 3, 1991                                                 *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AnimClass::AI -- This is the low level anim processor.                                    *
 *   AnimClass::AnimClass -- The constructor for animation objects.                            *
 *   AnimClass::Attach_To -- Attaches animation to object specified.                           *
 *   AnimClass::Sort_Above -- Sorts the animation above the target specified.                  *
 *   AnimClass::Center_Coord -- Determine center of animation.                                 *
 *   AnimClass::Detach -- Remove animation if attached to target.                              *
 *   AnimClass::Do_Atom_Damage -- Do atom bomb damage centered around the cell specified.      *
 *   AnimClass::Draw_It -- Draws the animation at the location specified.                      *
 *   AnimClass::In_Which_Layer -- Determines what render layer the anim should be in.          *
 *   AnimClass::Init -- Performs pre-scenario initialization.                                  *
 *   AnimClass::Mark -- Signals to map that redrawing is necessary.                            *
 *   AnimClass::Middle -- Processes any middle events.                                         *
 *   AnimClass::Occupy_List -- Determines the occupy list for the animation.                   *
 *   AnimClass::Overlap_List -- Determines the overlap list for the animation.                 *
 *   AnimClass::Render -- Draws an animation object.                                           *
 *   AnimClass::Sort_Y -- Returns with the sorting coordinate for the animation.               *
 *   AnimClass::Start -- Processes initial animation side effects.                             *
 *   AnimClass::delete -- Returns an anim object back to the free pool.                        *
 *   AnimClass::new -- Allocates an anim object from the pool.                                 *
 *   AnimClass::~AnimClass -- Destructor for anim objects.                                     *
 *   Anim_From_Name -- Given a name, this finds the corresponding anim type.                   *
 *   Shorten_Attached_Anims -- Reduces attached animation durations.                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "anim.h"

#include "_bench.h"
#include "_convert.h"
#include "_map.h"
#include "_palette.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "animtype.h"
#include "bench.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "conquer.h"
#include "crc.h"
#include "draw.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "incdec.h"
#include "inline.h"
#include "lightcon.h"
#include "overlay.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "scheme.h"
#include "session.h"
#include "shapeset.h"
#include "smudtype.h"
#include "sun.h"
#include "syncrechook.h"
#include "tactical.h"
#include "techno.h"
#include "tiberium.h"
#include "tracker.h"

#include "bench.hh"

#include <algorithm>
#include <intrin.h>


/***********************************************************************************************
 * AnimClass::AnimClass -- The constructor for animation objects.                              *
 *                                                                                             *
 *    This routine is used as the constructor of animation objects. It initializes and adds    *
 *    the animation object to the display and logic systems.                                   *
 *                                                                                             *
 * INPUT:   animnum  -- The animation number to start.                                         *
 *                                                                                             *
 *          coord    -- The location of the animation.                                         *
 *                                                                                             *
 *          timedelay-- The delay before the animation starts.                                 *
 *                                                                                             *
 *          loop     -- The number of times to loop this animation.                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   08/03/1994 JLB : Added a delayed affect parameter.                                        *
 *=============================================================================================*/
AnimClass::AnimClass(AnimTypeClass const * type, Coord const & coord, int timedelay, int loop, ShapeFlags_Type flags, int zadjust) :
	BASECLASS(),
	StageClass(),
	Class((AnimTypeClass *)type),
	xObject(NULL),
	OwnerHouse(HOUSE_NONE),
	AlternativeDrawer(NULL),
	AlternativeBrightness(NORMAL_LIGHT),
	ZAdjust(zadjust),
	YSortAdjust(0),
	FlamingGuyCoords(COORD_NONE),
	FlamingGuyRetries(0),
	IsBuildingAnim(false),
	Bounce(),
	Loops(1),
	IsBouncing(false),
	IsAttachedToCell(false),
	IsToDeleteOnOverpass(false),
	IsInert(false),
	IsFogged(false),
	IsFlamingGuyEnd(false),
	IsToDelete(false),
	IsBrandNew(true),
	IsInvisible(false),
	IsDisabled(false),
	Delay(timedelay),
	Accum(1),
	TranslucencyLevel(0),
	ShapeFlags(flags)
{
	Create_ID();
	Sync_Record_Anim(*this, coord, (unsigned)(uintptr_t)_ReturnAddress());
	Anims.Add(this);
	IsActive = true;

	YSortAdjust = Class->YSortAdjust;

	if (Class->Stages == -1) {
		Class->Stages = ((ShapeSet *)Class->Get_Image_Data())->Get_Count();
	}
	if (Class->LoopEnd == -1) {
		Class->LoopEnd = Class->Stages;
	}
	int delay = Class->Delay;
	if (Class->RandomRateMin != 0 || Class->RandomRateMax != 0) {
		if (Class->RandomRateMin <= Class->RandomRateMax) {
			delay = Random_Pick(Class->RandomRateMin, Class->RandomRateMax);
		}
	}
	if (Class->IsNormalized) {
		Set_Rate(Options.Normalize_Delay(delay));
	} else {
		Set_Rate(delay);
	}

	Set_Stage(0);

	if (!Class->IsGroundLayer) {
		Height = Rule->FlightLevel;
	} else {
		HeightAGL = 0;
	}

	if (Class->IsReverse) {
		Set_Stage(Class->LoopEnd);
		Set_Step(-Fetch_Step());
	}

	if (!Class->IsBouncer && !Class->IsMeteor) {
		BASECLASS::Unlimbo(coord);
	} else {
		IsBouncing = true;
		if (Class->IsMeteor) {
			Vector3 velocity((abs(Scen->RandomNumber) % int(Class->MaxXYVel * 2)) - Class->MaxXYVel,
							(abs(Scen->RandomNumber) % int(Class->MaxXYVel * 2)) - Class->MaxXYVel, Class->MinZVel);

			if (velocity.X < -velocity.Y) {
				velocity.X = -velocity.X;
				velocity.Y = -velocity.Y;
			}

			int time = 70 - abs(Scen->RandomNumber() % 20);
			int x = coord.X - time * velocity.X;
			int y = coord.Y - time * velocity.Y;
			int z = coord.Z - time * velocity.Z;
			Coord ucoord(x, y, z);
			BASECLASS::Unlimbo(ucoord);
			Bounce.Init(Center_Coord(), Class->Elasticity, 1.4f, 0.0, velocity, 0.0);
		} else {
			BASECLASS::Unlimbo(coord);
			Coord center = Center_Coord() + Coord(0, 0, 10);
			int r1 = Scen->RandomNumber, r2 = Scen->RandomNumber, r3 = Scen->RandomNumber;
			float x = (abs(r3) % int(Class->MaxXYVel * 2)) - Class->MaxXYVel;
			float y = (abs(r2) % int(Class->MaxXYVel * 2)) - Class->MaxXYVel;
			float z = (abs(r1) % int(Class->MaxZVel - Class->MinZVel + 1.0)) + Class->MinZVel;

			Vector3 velocity(x, y, z);
			Bounce.Init(center, Class->Elasticity, 1.4f, 0.0, velocity, 0.0);
		}
	}

	/*
	**	Drop zone smoke always reveals the map around itself.
	*/
	if (Class == Rule->FlareAnim) {
		Map.Sight_From(coord, Rule->DropZoneRadius / CELL_LEPTON_W, PlayerPtr, false);
	}

	loop = std::max(loop, 1) * Class->Loops;
	Loops = loop;
	Loops = std::max<int>(Loops, 1);

	/*
	**	If the animation starts immediately, then play the associated sound effect now.
	*/
	if (!Delay) {
		Start();
	}
}


/// <summary>
/// Constructs a blank animation object.
/// This constructor serves the load system, which creates an empty animation through the
/// class table and then fills it in from the save game. The animation joins the master
/// animation list but has no type and is nowhere on the map.
/// </summary>
AnimClass::AnimClass(void) :
	BASECLASS(),
	Class(NULL),
	xObject(NULL),
	OwnerHouse(HOUSE_NONE),
	Loops(1),
	IsToDelete(false),
	IsBrandNew(true),
	IsInvisible(false),
	IsDisabled(false),
	Delay(0),
	Accum(0),
	AlternativeDrawer(NULL),
	AlternativeBrightness(NORMAL_LIGHT),
	ZAdjust(0),
	YSortAdjust(0),
	IsBuildingAnim(false),
	IsBouncing(false),
	IsAttachedToCell(false),
	IsToDeleteOnOverpass(false)
{
	Anims.Add(this);
	IsActive = true;
}


/***********************************************************************************************
 * AnimClass::~AnimClass -- Destructor for anim objects.                                       *
 *                                                                                             *
 *    This destructor handles removing the animation object from the system. It might require  *
 *    informing any object this animation is attached to that it is no longer attached.        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
AnimClass::~AnimClass(void)
{
	Detach_This_From_All(this, true);
	if (GameActive) {

		/*
		**	If this anim is attached to another object
		**	then check to see if this is the last anim attached to it. If this
		**	is the case, then inform the object that it is no longer attached to
		**	an animation.
		*/
		if (xObject != NULL) {
			ObjectClass * to = xObject;

			/*
			**	Scan for any other animations that are attached to the object that
			**	this animation is attached to. If there are no others, then inform the
			**	attached object of this fact.
			*/
			int index;
			for (index = 0; index < Anims.Count(); index++) {
				if (Anims[index] != this && Anims[index]->xObject == xObject) break;
			}

			/*
			**	Tell the object that it is no longer being damaged.
			*/
			if (index == Anims.Count()) {
				to->Fire_Out();
				to->IsAnimAttached = false;
			}
			xObject = NULL;
		}
		Limbo();
	}

	AbstractTypePtrTracker.Delete(this);

	Class->Free_Image();

	xObject = NULL;
	Class = 0;

	if (Fetch_ID() == -2) {
		MoveFlashes.Delete(this);
	} else {
		Anims.Delete(this);
	}
}


/***********************************************************************************************
 * Anim_From_Name -- Given a name, this finds the corresponding anim type.                     *
 *                                                                                             *
 *    This routine will convert the supplied ASCII name into the animation type that it        *
 *    represents.                                                                              *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the ASCII name to convert.                                     *
 *                                                                                             *
 * OUTPUT:  Returns with the animation type that matches the name specified. If no match could *
 *          be found, then ANIM_NONE is returned.                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
AnimType Anim_From_Name(char const * name)
{
	if (name == NULL) return(ANIM_NONE);

	if (strcmpi(name, "<none>") == 0 || strcmpi(name, "none") == 0) return(ANIM_NONE);

	for (AnimType anim = ANIM_FIRST; anim < AnimTypes.Count(); anim++) {
		if (stricmp(AnimTypes[anim]->IniName, name) == 0) {
			return(anim);
		}
	}
	return(ANIM_NONE);
}


/***********************************************************************************************
 * Shorten_Attached_Anims -- Reduces attached animation durations.                             *
 *                                                                                             *
 *    This routine is used to reduce the amount of time any attached animations will process.  *
 *    Typical use of this is when an object is on fire and the object should now be destroyed  *
 *    but the attached animations are to run until completion before destruction can follow.   *
 *    This routine will make the animation appear to run its course, but in as short of time   *
 *    as possible. The shortening effect is achieved by reducing the number of times the       *
 *    animation will loop.                                                                     *
 *                                                                                             *
 * INPUT:   obj   -- Pointer to the object that all attached animations will be processed.     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/11/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void Shorten_Attached_Anims(ObjectClass * obj)
{
	if (obj != NULL) {
		for (int index = 0; index < Anims.Count(); index++) {
			AnimClass * anim = Anims[index];

			if (anim->xObject == obj) {
				anim->Loops = 0;
			}
		}
	}
}


/***********************************************************************************************
 * AnimClass::Sort_Y -- Returns with the sorting coordinate for the animation.                 *
 *                                                                                             *
 *    This routine is used by the sorting system. Animations that are located in the ground    *
 *    layer will be sorted by this the value returned from this function.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the sort coordinate to use for this animation.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   12/15/1994 JLB : Handles flat anims (infantry decay anims).                               *
 *=============================================================================================*/
int AnimClass::Sort_Y(void) const
{
	return(BASECLASS::Sort_Y() + YSortAdjust);
}


/***********************************************************************************************
 * AnimClass::Center_Coord -- Determine center of animation.                                   *
 *                                                                                             *
 *    This support function will return the "center" of the animation. The actual coordinate   *
 *    of the animation may be dependant on if the the animation is attached to an object.      *
 *    In such a case, it must factor in the object's location.                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the coordinate of the center of the animation. The coordinate is in real   *
 *          game coordinates -- taking into consideration if the animation is attached.        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *   02/02/1996 JLB : Coordinate based on visual center of object.                             *
 *=============================================================================================*/
Coord AnimClass::Center_Coord(void) const
{
	if (xObject != NULL) {
		return(xObject->Center_Coord() + BASECLASS::Center_Coord());
	}
	return(BASECLASS::Center_Coord());
}


/***********************************************************************************************
 * AnimClass::Render -- Draws an animation object.                                             *
 *                                                                                             *
 *    This is the working routine that renders the animation shape. It gets called once        *
 *    per animation per frame. It needs to be fast.                                            *
 *                                                                                             *
 * INPUT:   bool; Should the animation be rendered in spite of render flag?                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the animation rendered?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AnimClass::Render(Rect & rect, bool forced, bool extras_only) const
{
	if (Delay) return(false);
	return(BASECLASS::Render(rect, forced, extras_only));
}


/***********************************************************************************************
 * AnimClass::Draw_It -- Draws the animation at the location specified.                        *
 *                                                                                             *
 *    This routine is used to render the animation object at the location specified. This is   *
 *    how the map imagery gets updated.                                                        *
 *                                                                                             *
 * INPUT:   x,y      -- The pixel coordinates to draw the animation at.                        *
 *                                                                                             *
 *          window   -- The to base the draw coordinates upon.                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *   05/19/1995 JLB : Added white translucent effect.                                          *
 *=============================================================================================*/
void AnimClass::Draw_It(Point2D const & point, Rect const & cliprect) const
{
	if (!IsInvisible && Class->DetailLevel <= Options.DetailLevel && (!IsFogged || !Class->IsShouldFogRemove)) {
		BStart(BENCH_ANIMS);
		ShapeSet const * shapefile = (ShapeSet const *)Get_Image_Data();

		if (shapefile != NULL) {
			ShapeFlags_Type flags = ShapeFlags;
			int shapenum = Class->Start + Fetch_Stage();

			/*
			**	If the translucent table hasn't been determined yet, then check to see if it
			**	should use the white or normal translucent tables.
			*/
			if (Class->TranslucencyDetailLevel <= Options.DetailLevel) {
				if (Class->IsTranslucent) {
					if (TranslucencyLevel >= 15) return;
					if (Fetch_Stage() > Class->Stages * 0.6) {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT75);
					} else if (Fetch_Stage() > Class->Stages * 0.4) {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT50);
					} else if (Fetch_Stage() > Class->Stages * 0.2) {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT25);
					}
				} else if (Class->Translucency > 0) {
					if (TranslucencyLevel >= 15) return;
					if (Class->Translucency == 25) {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT25);
					} else if (Class->Translucency == 50) {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT50);
					} else if (Class->Translucency == 75) {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT75);
					}
				} else if (TranslucencyLevel != 0) {
					if (TranslucencyLevel > 15) return;
					if (TranslucencyLevel > 10) {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT50);
					} else if (TranslucencyLevel > 5) {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT50);
					} else {
						flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT25);
					}
				}
			}

			if ((flags & SHAPE_DARKEN) == 0) {
				flags = ShapeFlags_Type(flags|SHAPE_ALPHA);
			}

			int height = HeightAGL;
			int brightness = NORMAL_LIGHT;
			ConvertClass * convert;

			if (Class->IsVeins) {
				convert = ColorSchemes[PlayerPtr->Scheme]->Converter;
				if (!Class->IsUseNormalLight) {
					brightness = Map[Render_Coord()].TileBrightness;
				}
			} else if (IsAttachedToCell) {
				CellClass * cellptr = &Map[Render_Coord().As_Cell()];
				if (cellptr->Drawer == NULL) {
					cellptr->Init_Drawer();
				}
				convert = cellptr->Drawer;
				if (!Class->IsUseNormalLight) {
					brightness = cellptr->TileBrightness;
				}
			} else if (AlternativeDrawer != NULL) {
				convert = AlternativeDrawer;
				if (!Class->IsUseNormalLight) {
					brightness = AlternativeBrightness;
				}
			} else {
				convert = AnimDrawer;
				if (Class->IsAltPalette) {
					convert = ColorSchemes[0]->Converter;
				}
				if (!Class->IsUseNormalLight) {
					brightness = Map[Render_Coord().As_Cell()].Brightness;
				}
			}

			/*
			**	Draw the animation shape.
			*/
			if (IsBouncing) {
				Point2D drawpoint(point.X, point.Y + Class->YDrawOffset + TacticalMap->Z_Lepton_To_Pixel(height));
				Draw_Shape(*LogicalSurface, *convert, shapefile, shapenum, drawpoint, cliprect, ShapeFlags_Type(SHAPE_DARKEN|SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZGRAD), NULL, Class->YDrawOffset - TacticalMap->Z_Lepton_To_Pixel(Height));
			}

			if (Class->IsTiled) {
				int frameheight = shapefile->Get_Rect(0).Height;
				Point2D origin = point;
				Point2D drawpoint = origin - Point2D(0, frameheight / 2);
				bool done = false;
				int height_offset = ZAdjust + Class->YDrawOffset - TacticalMap->Z_Lepton_To_Pixel(Height) - 2;
				while (!done) {
					Draw_Shape(*LogicalSurface, *AnimDrawer, shapefile, shapenum, Point2D(origin.X, drawpoint.Y + Class->YDrawOffset), TacticalRect, flags, NULL, height_offset, ZGRAD_90DEG, brightness);
					if (drawpoint.Y < 0) done = true;
					height_offset -= frameheight;
					drawpoint.Y -= frameheight;
				}
			} else if (Class->IsFlat) {
				Draw_Shape(*LogicalSurface, *convert, shapefile, shapenum, Point2D(point.X, point.Y + Class->YDrawOffset), cliprect, ShapeFlags_Type(flags|SHAPE_ZGRAD), NULL, ZAdjust + Class->YDrawOffset - TacticalMap->Z_Lepton_To_Pixel(Height) - 2, ZGRAD_GROUND, brightness);
			} else {
				Draw_Shape(*LogicalSurface, *convert, shapefile, shapenum, Point2D(point.X, point.Y + Class->YDrawOffset), cliprect, ShapeFlags_Type(flags|SHAPE_ZGRAD), NULL, ZAdjust + Class->YDrawOffset - TacticalMap->Z_Lepton_To_Pixel(Height) - 2, ZGRAD_90DEG, brightness);
			}

			if (Class->IsFlamingGuy && !IsFalling) {
				shapenum += shapefile->Get_Count() / 2;
				Draw_Shape(*LogicalSurface, *convert, shapefile, shapenum, Point2D(point.X, point.Y + Class->YDrawOffset), cliprect, ShapeFlags_Type(SHAPE_DARKEN|SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZGRAD), NULL, ZAdjust + Class->YDrawOffset - TacticalMap->Z_Lepton_To_Pixel(Height) - 3, ZGRAD_GROUND, brightness);
			}
		}
		BEnd(BENCH_ANIMS);
	}
}


/***********************************************************************************************
 * AnimClass::Mark -- Signals to map that redrawing is necessary.                              *
 *                                                                                             *
 *    This routine is used by the animation logic system to inform the map that the cells      *
 *    under the animation must be rerendered.                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AnimClass::Mark(MarkType mark)
{
	if (BASECLASS::Mark(mark)) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * AnimClass::Occupy_List -- Determines the occupy list for the animation.                     *
 *                                                                                             *
 *    Animations always occupy only the cell that their center is located over. As such, this  *
 *    routine always returns a simple (center cell) occupation list.                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the occupation list for the animation.                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell const * AnimClass::Occupy_List(bool) const
{
	static Cell _simple[] = {REFRESH_EOL};

	return(_simple);
}


/// <summary>
/// Handles a frame of bounce physics for the animation.
/// Meteors and other tumbling animations travel by this routine. When the animation
/// strikes the ground it spawns its impact animation, plays its impact sound, and damages
/// anything standing close enough to the point of impact.
/// </summary>
/// <returns>Returns with the state of the bounce: BOUNCE_IMPACT when the animation has just
/// struck the ground and BOUNCE_SETTLED when it has finished bouncing and deleted itself.</returns>
BounceResultType AnimClass::Bounce_AI(void)
{
	BounceClass & bounce = Bounce;
	BounceResultType bounce_result = bounce.AI();

	if (Class->IsMeteor) {
		bounce.Velocity.Z += bounce.Gravity;
	}

	switch (bounce_result) {
		case BOUNCE_IMPACT: {
			if (Class->BounceAnim != NULL) {
				new AnimClass(Class->BounceAnim, Center_Coord());
			}
			if ((unsigned char)Class->BounceSound != UCHAR_MAX) {
				Sound_Effect(Class->BounceSound, Center_Coord());
			}
			ObjectClass * optr = Map[bounce.Get_Bounce_Coord()].Cell_Occupier();
			while (optr != NULL) {
				Coord cdiff = bounce.Get_Bounce_Coord() - optr->Center_Coord();
				int rad = abs(cdiff.X) + abs(cdiff.Y);
				if (rad <= Class->DamageRadius) {
					int damage = (int)Class->Damage;
					optr->Take_Damage(damage, TacticalMap->Z_Lepton_To_Pixel(rad), Class->Warhead);
				}
				optr = optr->Next;
			}
			break;
		}
		case BOUNCE_SETTLED:
			Delete_Me();
			break;
	}

	PositionCoord = bounce.Get_Bounce_Coord();

	return(bounce_result);
}


/***********************************************************************************************
 * AnimClass::AI -- This is the low level anim processor.                                      *
 *                                                                                             *
 *    This routine is called once per frame per animation. It handles transition between       *
 *    animation frames and marks the map for redraw as necessary.                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Speed is of upmost importance.                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimClass::AI(void)
{
	if (Class->IsFlamingGuy) {
		Flaming_Guy_AI();
		BASECLASS::AI();
	}

	if (IsBouncing) {
		BounceResultType bounce_result = Bounce_AI();
		if (bounce_result == BOUNCE_SETTLED || bounce_result == BOUNCE_IMPACT) {
			bool water = Map[(Coord const &)PositionCoord].Land_Type() == LAND_WATER;
			bool bridge = PositionCoord.Z >= Map.Get_Height_GL(PositionCoord) + BRIDGE_LEPTON_HEIGHT;

			if (water && !bridge) {
				if (Class->IsMeteor) {
					new AnimClass(Rule->SplashList[Rule->SplashList.Count()-1], PositionCoord + Coord(0, 0, 3));
				} else {
					new AnimClass(Rule->Wake, PositionCoord);
					new AnimClass(Rule->SplashList[0], PositionCoord + Coord(0, 0, 3));
				}
			} else {
				if (Class->ExpireAnim != NULL) {
					Vector3 bouncecoord = Bounce.MyCoord;
					new AnimClass(Class->ExpireAnim, Coord(bouncecoord.X, bouncecoord.Y, bouncecoord.Z), 0, 1, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZGRAD), -30);
					Explosion_Damage(Bounce.Get_Bounce_Coord(), Class->Damage, NULL, Class->Warhead);
					Combat_Lighting(Bounce.Get_Bounce_Coord(), Class->Damage, Class->Warhead);
				}
				if (Class->ExpireSound != VOC_NONE) {
					Sound_Effect(Class->ExpireSound, Center_Coord());
				}
			}

			if (!water || bridge) {
				Coord coord = Bounce.Get_Bounce_Coord();
				if (Class->Spawns != NULL && Class->SpawnCount > 0) {
					int count = Random_Pick(0, Class->SpawnCount) + Random_Pick(0, Class->SpawnCount);
					for (int i = 0; i < count; i++) {
						new AnimClass(Class->Spawns, coord);
					}
				}

				if (Rule->CraterLevel != 0 && Class->IsMeteor && !bridge) {
					Map.Deform_Terrain(coord.As_Cell(), false);
					if (Rule->CraterLevel > 1) {
						for (int dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
							if (dir % 2 != 0 || Rule->CraterLevel > 2) {
								Map.Deform_Terrain(Adjacent_Cell(Cell(coord), (FacingType)dir), false);
							}
						}
						if (Rule->CraterLevel > 3) {
							Map.Deform_Terrain(coord.As_Cell(), false);
						}
					}
				}

				Rect updaterect(0, 0, 0, 0);
				if (Class->IsTiberium && !bridge) {
					for (int x = -Class->TiberiumSpreadRadius; x <= Class->TiberiumSpreadRadius; x++) {
						for (int y = -Class->TiberiumSpreadRadius; y <= Class->TiberiumSpreadRadius; y++) {
							if ((int)std::sqrt((double)x * (double)x + (double)y * (double)y) <= Class->TiberiumSpreadRadius) {
								CellClass * cellptr = &Map[Adjacent_Cell(Cell(coord), FacingType(x))];
								if (cellptr->Can_Tiberium_Germinate(NULL) && Class->TiberiumSpawnType != NULL) {
									new OverlayClass(OverlayTypes[Class->TiberiumSpawnType->HeapID + Random_Pick(0, 3)], cellptr->Fetch_CellID());
									cellptr->OverlayData = Random_Pick(0, 2);
									Rect overlayrect = cellptr->Overlay_Render_Rect();
									overlayrect.Y -= TacticalRect.Y;
									updaterect = Union(updaterect, overlayrect);
								}
							}
						}
					}
					TacticalMap->Register_Dirty_Area(updaterect);
				}
			}

			Delete_Me();
			return;
		}
	}

	if (IsActive && !IsToDelete) {
		if (Class->TrailerAnim != NULL && (Class->TrailerSeperation == 1 || (Frame % Class->TrailerSeperation == 0))) {
			new AnimClass(Class->TrailerAnim, Center_Coord(), 1);
		}
	}

	/*
	**	Special case check to make sure that building on top of a smoke marker
	**	causes the smoke marker to vanish.
	*/
	if (Class == Rule->FlareAnim && Map[Center_Coord()].Cell_Building()) {
		IsToDelete = true;
	}

	/*
	**	Delete this animation and bail early if the animation is flagged to be deleted
	**	immediately.
	*/
	if (IsToDelete) {
		Delete_Me();
		return;
	}

	/*
	**	If this is a brand new animation, then don't process it the first logic pass
	**	since it might end up skipping the first animation frame before it has had a
	**	chance to draw it.
	*/
	if (IsBrandNew) {
		IsBrandNew = false;
		return;
	}

	if (Delay) {
		Delay--;
		if (!Delay) {
			Start();
		}
	} else if (IsActive) {
		if (Class->IsVeins) {
			Vein_Attack_AI();
		}

		if (Class->IsAnimatedTiberium) {
			OverlayType overlay = Map[Center_Coord() - Coord(CELL_LEPTON * 1.5, CELL_LEPTON * 1.5, 0)].Overlay;
			if (overlay == OVERLAY_NONE || OverlayTypes[overlay]->CellAnim != Class) {
				IsToDelete = true;
			}
		}

		if (Class->Stages == -1) {
			Class->Stages = ((ShapeSet const *)Class->Get_Image_Data())->Get_Count();
		}
		if (Class->LoopEnd == -1) {
			Class->LoopEnd = Class->Stages;
		}

		/*
		**	This is necessary because there is no recording of animations on the map
		**	and thus the animation cannot be intelligently flagged for redraw. Most
		**	animations move fast enough that they would need to be redrawn every
		**	game frame anyway so this isn't TOO bad.
		*/
		Mark(MARK_CHANGE);

		if (!IsDisabled && StageClass::Graphic_Logic()) {
			int stage = Fetch_Stage();

			/*
			**	If this animation is attached to another object and it is a
			**	damaging kind of animation, then do the damage to the other
			**	object.
			*/
			if (Class->Damage > 0 && !IsBouncing) {
				if (Is_Target_Terrain(xObject)) {
					Accum += Class->Damage * 5;
				} else {
					Accum += Class->Damage;
				}

				if (Accum >= 1 && !IsInert) {

					/*
					**	Administer the damage. If the object was destroyed by this anim,
					**	then the attached damaging anim is also destroyed.
					*/
					int damage = Accum;
					Accum -= damage;
					if (strcmp(Class->IniName, "INVISO") == 0) {
						Explosion_Damage(Center_Coord(), damage, NULL, Rule->C4Warhead);
					} else {
						Explosion_Damage(Center_Coord(), damage, NULL, Rule->FlameDamage2);
					}
					if (!IsActive) {
						return;
					}
				}
			}

			/*
			**	During the biggest stage (covers the most ground), perform any ground altering
			**	action required. This masks craters and scorch marks, so that they appear
			**	naturally rather than "popping" into existence while in plain sight.
			*/
			if (Class->Biggest && Class->Start+stage == Class->Biggest && !IsBouncing) {
				Middle();
			}

			if (Class->IsPingPong) {
				if ((Loops <= 1 && (stage >= Class->Stages || stage == 0)) || (Loops > 1 && (stage >= Class->LoopEnd-Class->Start || stage == Class->Start))) {
					Set_Step(-Fetch_Step());
					return;
				}
			}

			/*
			**	Check to see if the last frame has been displayed. If so, then the
			**	animation either ends or loops.
			*/
			if ((Loops <= 1 && stage >= Class->Stages) || (Loops > 1 && stage >= Class->LoopEnd-Class->Start) || (Class->IsReverse && stage <= Class->Start)) {

				/*
				**	Determine if this animation should loop another time. If so, then start the loop
				**	but if not, then proceed into the animation termination handler.
				*/
				if (Loops && Loops != UCHAR_MAX) Loops--;
				if (Loops) {
					if (Class->IsReverse) {
						Set_Stage(Class->LoopEnd);
					} else {
						Set_Stage(Class->LoopStart-Class->Start);
					}
					if (Class->RandomLoopDelayMin != 0 || Class->RandomLoopDelayMax != 0) {
						Delay = Random_Pick(Class->RandomLoopDelayMin, Class->RandomLoopDelayMax);
					}
				} else {

					/*
					**	The animation should end now, but first check to see if
					**	it needs to chain into another animation. If so, then the
					**	animation isn't technically over. It metamorphoses into the
					**	new form.
					*/
					if (Class->ChainTo != NULL) {

						Class = (AnimTypeClass *)Class->ChainTo;

						if (Class->Stages == -1) {
							Class->Stages = ((ShapeSet const *)Class->Get_Image_Data())->Get_Count();
						}
						if (Class->LoopEnd == -1) {
							Class->LoopEnd = Class->Stages;
						}

						IsToDelete = false;
						Loops = Class->Loops;
						Accum = 0;
						int delay = Class->Delay;
						if (Class->RandomRateMin != 0 || Class->RandomRateMax != 0) {
							delay = Random_Pick(Class->RandomRateMin, Class->RandomRateMax);
						}
						if (Class->IsNormalized) {
							Set_Rate(Options.Normalize_Delay(delay));
						} else {
							Set_Rate(delay);
						}
						Set_Stage(Class->Start);
						Start();
					} else {
						Delete_Me();
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * AnimClass::Attach_To -- Attaches animation to object specified.                             *
 *                                                                                             *
 *    An animation can be "attached" to an object. In such cases, the animation is rendered    *
 *    as an offset from the center of the object it is attached to. This allows affects such   *
 *    as fire or smoke to be consistently placed on the vehicle it is associated with.         *
 *                                                                                             *
 * INPUT:   obj   -- Pointer to the object to attach the animation to.                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimClass::Attach_To(ObjectClass * obj)
{
	/*
	**	If this anim is attached to another object
	**	then check to see if this is the last anim attached to it. If this
	**	is the case, then inform the object that it is no longer attached to
	**	an animation.
	*/
	if (xObject != NULL) {
		ObjectClass * to = xObject;

		bool down = IsDown;
		if (down) {
			Map.Remove(this);
		}

		/*
		**	Scan for any other animations that are attached to the object that
		**	this animation is attached to. If there are no others, then inform the
		**	attached object of this fact.
		*/
		int index;
		for (index = 0; index < Anims.Count(); index++) {
			if (Anims[index] != this && Anims[index]->xObject == xObject) break;
		}

		/*
		**	Tell the object that it is no longer being damaged.
		*/
		if (index == Anims.Count()) {
			to->Fire_Out();
			to->IsAnimAttached = false;
		}

		Coord center = Center_Coord();
		xObject = NULL;

		PositionCoord = center;

		if (down) {
			Map.Submit(this);
		}
	}

	if (obj == NULL) return;
	assert(obj->IsActive);

	Coord center = Center_Coord();
	Map.Remove(this);
	obj->IsAnimAttached = true;
	xObject = obj;
	PositionCoord = center - obj->Center_Coord();
	Map.Submit(this);
}


/***********************************************************************************************
 * AnimClass::In_Which_Layer -- Determines what render layer the anim should be in.            *
 *                                                                                             *
 *    Use this routine to find out which display layer (ground or air) that the animation      *
 *    should be in. This information is used to place the animation into the correct display   *
 *    list.                                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the layer that the animation should exist in.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/25/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
LayerType AnimClass::In_Which_Layer(void) const
{
	if (xObject != NULL || (Class != NULL && Class->IsGroundLayer)) {
		return(LAYER_GROUND);
	}
	return(LAYER_AIR);
}


/***********************************************************************************************
 * AnimClass::Start -- Processes initial animation side effects.                               *
 *                                                                                             *
 *    This routine is called when the animation first starts. Sometimes there are side effects *
 *    associated with this animation that must occur immediately. Typically, this is the       *
 *    sound effect assigned to this animation. If this animation is supposed to attach itself  *
 *    to any object at its location, then do so at this time as well.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/30/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimClass::Start(void)
{
	Mark(MARK_CHANGE);

	if (!IsInert && Class->Sound != VOC_NONE) {

		/*
		**	Play the sound effect for this animation.
		*/
		Sound_Effect(Class->Sound, Center_Coord());
	}

	/*
	**	If the stage where collateral effects occur is the first stage of the animation, then
	**	perform this action now. Subsequent checks against this stage value starts with the
	**	second frame of the animation.
	*/
	if (!Class->Biggest) {
		Middle();
	}

	if (!IsInert && Class->IsTiberiumChainReaction) {
		CellClass *cptr = &Map[Center_Coord()];
		TiberiumType tib = cptr->Tiberium_Type_Here();

		if (tib != TIBERIUM_NONE) {
			TiberiumClass * tiberium = Tiberiums[tib];
			cptr->Reduce_Tiberium(cptr->OverlayData + 1);

			if (tiberium->Debris.Count() > 0 && (abs(Scen->RandomNumber) % 3) == 0) {
				AnimClass * debris = new AnimClass(tiberium->Debris[Random_Pick(0, tiberium->Debris.Count()-1)], Center_Coord() + Coord(0, 0, 10));
				debris->AlternativeDrawer = ColorSchemes[tiberium->Color]->Converter;
				debris->AlternativeBrightness = cptr->Brightness;
			}

			Explosion_Damage(PositionCoord, Rule->TiberiumExplosionDamage, NULL, Rule->C4Warhead, false);

			cptr->Recalc_Attributes();
			Map.Update_Cell_Zone(cptr->CellID);
			Map.Update_Cell_Subzones(cptr->CellID);
		}
	}
}


/***********************************************************************************************
 * AnimClass::Middle -- Processes any middle events.                                           *
 *                                                                                             *
 *    This routine is called when the animation as reached its largest stage. Typically, this  *
 *    routine is used to cause scorches or craters to appear at a cosmetically pleasing        *
 *    moment.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/30/1995 JLB : Created.                                                                 *
 *   10/17/1995 JLB : Ion camera added.                                                        *
 *=============================================================================================*/
void AnimClass::Middle(void)
{
	Cell cell = Center_Coord().As_Cell();
	CellClass * cellptr = &Map[cell];

	int width = 30;
	int height = 30;

	ShapeSet const * shapefile = (ShapeSet const *)Get_Image_Data();
	if (shapefile != NULL) {
		width = shapefile->Get_Rect(Class->Biggest).Width;
		height = shapefile->Get_Rect(Class->Biggest).Height;
	}

	if (HeightAGL < 30) {

		/*
		**	If this animation leaves scorch marks (e.g., napalm), then do so at this time.
		*/
		if (Class->IsScorcher && (!Class->IsCraterForming || Random_Double(0.0, 1.0) < 0.5)) {
			SmudgeTypeClass::Scorch_The_Ground(Center_Coord(), width, height);
		}

		/*
		**	Some animations leave a crater when they occur. Artillery is a good example.
		**	Craters always remove the Tiberium where they occur.
		*/
		else if (Class->IsCraterForming) {

			/*
			**	Craters reduce the level of Tiberium in the cell.
			*/
			cellptr->Reduce_Tiberium(6);

			/*
			**	If there already is a crater in the cell, then just expand the
			**	crater.
			*/
			SmudgeTypeClass::Crater_The_Ground(Center_Coord(), width, height);
		}
	}

	AnimClass * newanim;

	/*
	**	If this animation spawns side effects during its lifetime, then
	**	do so now. Usually, these side effects are in the form of other
	**	animations.
	*/
	if (Class->IsFlameThrower) {
		new AnimClass(Rule->SmallFire, Map.Closest_Free_Spot(Coord_Scatter(Center_Coord(), CELL_LEPTON / 4), true), 0, Random_Pick(1, 2));
		if (Percent_Chance(50)) {
			new AnimClass(Rule->SmallFire, Map.Closest_Free_Spot(Coord_Scatter(Center_Coord(), 5 * CELL_LEPTON / 8), true), 0, Random_Pick(1, 2));
		}
		if (Percent_Chance(50)) {
			new AnimClass(Rule->LargeFire, Map.Closest_Free_Spot(Coord_Scatter(Center_Coord(), 7 * CELL_LEPTON / 16), true), 0, Random_Pick(1, 2));
		}
	} else if (Class->IsScorcher) {
		if (HeightAGL < 10) {
			LandType land = Map[(Coord const &)PositionCoord].Land_Type();
			if (land != LAND_WATER && land != LAND_BEACH && land != LAND_ICE && land != LAND_ROCK) {
				newanim = new AnimClass(Rule->SmallFire, Center_Coord(), 0, Random_Pick(1, 2));
				if (newanim != NULL && xObject != NULL) {
					newanim->Attach_To(xObject);
				}
			}
		}
	}
}


/***********************************************************************************************
 * AnimClass::Detach -- Remove animation if attached to target.                                *
 *                                                                                             *
 *    This routine is called when the specified target is being removed from the game. If this *
 *    animation happens to be attached to this object, then the animation must be remove as    *
 *    well.                                                                                    *
 *                                                                                             *
 * INPUT:   target   -- The target that is about to be destroyed.                              *
 *                                                                                             *
 *          all      -- Is the target being destroyed RIGHT NOW? If not, then it will be       *
 *                      destroyed soon. In that case, the animation should continue to remain  *
 *                      attached for cosmetic reasons.                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/30/1995 JLB : Created.                                                                 *
 *   07/02/1995 JLB : Detach is a precursor to animation destruction.                          *
 *=============================================================================================*/
void AnimClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);
	if (xObject == target) {
		Map.Remove(this);
		xObject = NULL;
		IsToDelete = true;
		Mark(MARK_UP);
	}
	if (Class == target) {
		Class = NULL;
	}
}


/***********************************************************************************************
 * AnimClass::Do_Atom_Damage -- Do atom bomb damage centered around the cell specified.        *
 *                                                                                             *
 *    This routine will apply damage around the ground-zero cell specified.                    *
 *                                                                                             *
 * INPUT:   ownerhouse  -- The owner of this atom bomb.                                        *
 *                                                                                             *
 *          cell        -- The ground zero location to apply the atom bomb damage.             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimClass::Do_Atom_Damage(HousesType ownerhouse, Cell const & cell)
{
	int radius;
	int rawdamage;
	if (Session.Type == GAME_NORMAL) {
		radius = 4;
		rawdamage = Rule->AtomDamage;
	} else {
		radius = 3;
		rawdamage = Rule->AtomDamage/5;
	}

	Wide_Area_Damage(cell, radius * CELL_LEPTON_W, rawdamage, NULL, Rule->NukeWarhead);
}


/// <summary>
/// Disables the animation so that it stops playing.
/// A disabled animation remains in the game and is still drawn, but it is frozen on
/// whatever stage it had reached.
/// </summary>
void AnimClass::Disable(void)
{
	IsDisabled = true;
}


/// <summary>
/// Enables the animation so that it plays again.
/// </summary>
void AnimClass::Enable(void)
{
	IsDisabled = false;
}


/// <summary>
/// Lists the members this animation carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void AnimClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);
	StageClass::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(xObject);
	stream.Serialize(OwnerHouse);
	// AlternativeDrawer -- a palette converter of the running session.
	stream.Serialize(AlternativeBrightness);
	stream.Serialize(ZAdjust);
	stream.Serialize(YSortAdjust);
	stream.Serialize(FlamingGuyCoords);
	stream.Serialize(FlamingGuyRetries);
	stream.Serialize(IsBuildingAnim);
	stream.Serialize(Bounce);
	stream.Serialize(TranslucencyLevel);
	stream.Serialize(Delay);
	stream.Serialize(Accum);
	stream.Serialize(ShapeFlags);
	stream.Serialize(IsBouncing);
	stream.Serialize(Loops);
	stream.Serialize(IsAttachedToCell);
	stream.Serialize(IsToDeleteOnOverpass);
	stream.Serialize(IsInert);
	stream.Serialize(IsFogged);
	stream.Serialize(IsFlamingGuyEnd);
	stream.Serialize(IsToDelete);
	stream.Serialize(IsBrandNew);
	stream.Serialize(IsInvisible);
	stream.Serialize(IsDisabled);
}


/// <summary>
/// Restores what the animation record could not carry.
/// The alternative drawer names a converter belonging to the running session, so it is
/// cleared here and picked up again by Post_Load_Game once the whole game is in place.
/// </summary>
void AnimClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	AlternativeDrawer = NULL;
}


/// <summary>
/// Adds the state of this animation to a running checksum.
/// The multiplayer code compares these checksums between machines so that it can tell
/// when two games have drifted out of sync.
/// </summary>
void AnimClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	if (xObject != NULL) {
		crc(xObject->Fetch_ID());
	}
	crc(OwnerHouse);
	crc((int)AlternativeBrightness);
	crc(ZAdjust);
	crc(YSortAdjust);
	crc(Delay);
	crc(Accum);
	crc(ShapeFlags);
	crc(IsBouncing);
	crc(Loops);
	crc(IsAttachedToCell);
	crc(IsToDeleteOnOverpass);
	crc(IsToDelete);
	crc(IsBrandNew);
	crc(IsInvisible);
	crc(IsDisabled);
}


/// <summary>
/// Fetches the number of stages this animation runs through.
/// </summary>
/// <returns>Returns with the stage count declared by the animation's type class.</returns>
int AnimClass::Stage_Count(void) const
{
	return(Class->Stages);
}


/// <summary>
/// Fetches the type class this animation was created from.
/// </summary>
/// <returns>Returns with a pointer to the animation's type class.</returns>
ObjectTypeClass const * AnimClass::Class_Of(void) const
{
	return(Class);
}


/// <summary>
/// Removes the animation from the map.
/// Any claim the animation had on its cell is given up and it is pulled from the display
/// list, but the animation object itself survives.
/// </summary>
/// <returns>bool; Was the animation removed from the map?</returns>
bool AnimClass::Limbo(void)
{
	if (Class->IsVeins) {
		Map[(Coord const &)PositionCoord].IsAnimAttached = false;
	}

	if (BASECLASS::Limbo()) {

		/*
		**	Remove the object from the appropriate display list.
		*/
		Map.Remove(this);
		return(true);
	}
	return(false);
}


/// <summary>
/// Handles the damage that an attacking vein animation deals out.
/// Anything standing in the veins that is not immune to them takes damage while this
/// animation plays. The animation marks itself for deletion once the veins or the victim
/// are no longer there.
/// </summary>
void AnimClass::Vein_Attack_AI(void)
{
	CellClass * cellptr = &Map[(Coord const &)PositionCoord];
	ObjectClass * optr = cellptr->Cell_Occupier();

	if (optr == NULL || optr->HeightAGL > 0 || cellptr->Overlay != OVERLAY_VEINS || cellptr->OverlayData < OVERLAYDATA_FIRST_SOLID_VEIN || cellptr->Ramp != 0) {
		IsToDelete = true;
	}

	if ((unsigned)Frame % 2 == 0) {
		while (optr != NULL) {
			ObjectClass * next = optr->Next;
			int damage = Rule->VeinDamage;
			TechnoClass * tech = Dynamic_Cast<TechnoClass *>(optr);
			if (tech != NULL && !tech->TClass->IsImmuneToVeins && !tech->Has_Ability(ABILITY_VEIN_PROOF) && tech->Strength > 0 && tech->IsActive && tech->HeightAGL <= 5) {
				tech->Take_Damage(damage, 0, Rule->VeinholeWarhead);
			}
			optr = next;
		}
	}
}


/// <summary>
/// Removes this animation from the game.
/// The animation is detached from whatever object it was riding before it goes away, so
/// that the object is not left pointing at an animation that no longer exists.
/// </summary>
void AnimClass::Delete_Me(void)
{
	Attach_To(NULL);
	BASECLASS::Delete_Me();
}


/// <summary>
/// Fetches the vertical position of the animation.
/// An animation attached to another object is carried at that object's height as well as
/// its own.
/// </summary>
/// <returns>Returns with the height of the animation, expressed in leptons.</returns>
int AnimClass::Get_Height(void) const
{
	int z = Get_Coord().Z;
	if (xObject != NULL) {
		z += xObject->Get_Coord().Z;
	}
	return(z);
}


/// <summary>
/// Handles the per frame logic for a burning victim animation.
/// This routine moves the animation along toward the water it is making for, keeps the
/// running frames pointed the way it is traveling, and switches it over to the death
/// sequence once it arrives or runs out of anywhere to go.
/// </summary>
/// <remarks>Only call this routine once per animation per game logic loop.</remarks>
void AnimClass::Flaming_Guy_AI(void)
{
	static const int _max_distance = 18;
	static const int _max_tries = 7;
	static const int _frame_divisor = 3;
	static const int _min_stage = 0;

	if (!IsFlamingGuyEnd) {
		Set_Rate(0);
	}

	if (FlamingGuyCoords != COORD_NONE && !IsFlamingGuyEnd) {
		Coord coord = PositionCoord;
		coord.Z = 0;
		Coord target = FlamingGuyCoords;
		target.Z = 0;
		if (coord.Distance_To(target) <= _max_distance) {
			if (!IsFalling) {
				Coord nextcoord = Next_Flaming_Guy_Coord();
				bool sink = Map[FlamingGuyCoords].Land_Type() == LAND_WATER && HeightAGL <= LEVEL_LEPTON_H;
				if (nextcoord != COORD_NONE && FlamingGuyRetries < _max_tries && !sink) {
					FlamingGuyRetries++;
					int z = PositionCoord.Z;
					Coord oldcoord = FlamingGuyCoords;
					if (z <= Map.Get_Height_GL(oldcoord) + 2 * LEVEL_LEPTON_H) {
						oldcoord.Z = Map.Get_Height_GL(oldcoord);
					} else {
						oldcoord.Z = Map.Get_Height_GL(oldcoord) + BRIDGE_LEPTON_HEIGHT;
					}
					PositionCoord = oldcoord;
					FlamingGuyCoords = nextcoord;
				} else {
					FlamingGuyCoords = COORD_NONE;
					IsFlamingGuyEnd = true;
					Set_Rate(1);
					Set_Stage(Class->RunningFrames * FACING_COUNT + 1);
					return;
				}
			}
		} else {
			int z = PositionCoord.Z;
			DirType direction = DirType().Direction(Center_Coord(), FlamingGuyCoords);
			Coord newcoord = Move_Coord(PositionCoord, direction, (float)_max_distance);
			if (z <= Map.Get_Height_GL(newcoord) + 2 * LEVEL_LEPTON_H) {
				newcoord.Z = Map.Get_Height_GL(newcoord);
			} else {
				newcoord.Z = Map.Get_Height_GL(newcoord) + BRIDGE_LEPTON_HEIGHT;
			}
			PositionCoord = newcoord;
		}

		if (!IsFalling) {
			Cell position = PositionCoord.As_Cell();
			if (PositionCoord.Z >= Map.Get_Height_GL(PositionCoord) + BRIDGE_LEPTON_HEIGHT) {
				if (!Map[Get_Coord()].IsUnderBridge && !Map[Adjacent_Cell(position, FACING_W)].IsUnderBridge && !Map[Adjacent_Cell(position, FACING_N)].IsUnderBridge) {
					IsFalling = true;
				}
			}
		}
	} else {
		if (!IsFalling) {
			if (!IsFlamingGuyEnd) {
				Coord nextcoord = Next_Flaming_Guy_Coord();
				if (nextcoord != COORD_NONE) {
					FlamingGuyCoords = nextcoord;
				} else {
					FlamingGuyCoords = COORD_NONE;
					IsFlamingGuyEnd = true;
					Set_Rate(1);
					Set_Stage(Class->RunningFrames * FACING_COUNT + 1);
				}
			}
		}
	}

	if (!IsFlamingGuyEnd) {
		int stage = _min_stage;
		if (FlamingGuyCoords != COORD_NONE) {
			stage = Class->RunningFrames * (Facing_Sub(-1, ::Direction(Center_Coord(), FlamingGuyCoords).As_Dir8()));
		}
		Set_Stage(stage + Frame / _frame_divisor % Class->RunningFrames);
		return;
	} else {
		int stage = ((ShapeSet const *)Get_Image_Data())->Get_Count() / 2 - 1;
		if (Fetch_Stage() >= stage) {
			Delete_Me();
		}
	}
}


/// <summary>
/// Fetches the next place for a burning victim to run to.
/// A man who has been set on fire heads for the nearest water in sight, and failing that
/// he takes any neighboring cell he can still get into.
/// </summary>
/// <returns>Returns with the coordinate to move to next, or COORD_NONE if the victim is
/// boxed in.</returns>
Coord AnimClass::Next_Flaming_Guy_Coord(void)
{
	bool any = false;

	Cell cell = PositionCoord.As_Cell();
	int mindist = 10000;
	Cell mincell = CELL_NONE;

	for (int x = cell.X - 5; x < cell.X + 5; x++) {
		any = true;
		for (int y = cell.Y - 5; y < cell.Y + 5; y++) {
			if (Map.In_Local_Radar(Cell(x, y))) {
				CellClass * cellptr = &Map[Cell(x, y)];
				if (cellptr->Land_Type() == LAND_WATER && !cellptr->IsUnderBridge && !cellptr->Adjacent_Cell(FACING_W).IsUnderBridge && !cellptr->Adjacent_Cell(FACING_N).IsUnderBridge) {
					int dist = cellptr->CellID != cell ? Point2D(x - cell.X, y - cell.Y).Length() : 0;
					if (x > cell.X || y > cell.Y) {
						dist -= 3;
					}
					if (dist < mindist) {
						mindist = dist;
						mincell = Cell(x, y);
					}
				}
			}
		}
	}

	if (any && mincell != CELL_NONE) {
		int dx = mincell.X - cell.X;
		int dy = mincell.Y - cell.Y;
		short ndy = 0;
		short ndx = 0;
		if (dx < 0) {
			ndx = -1;
		} else if (dx > 0) {
			ndx = 1;
		}
		if (dy < 0) {
			ndy = -1;
		} else if (dy > 0) {
			ndy = 1;
		}

		if (Is_Valid_Flaming_Guy_Cell(cell + Cell(ndx, ndy))) {
			return(Cell(cell.X + ndx, cell.Y + ndy));
		}

		if (Is_Valid_Flaming_Guy_Cell(cell + Cell(ndx, 0))) {
			return(Cell(cell.X + ndx, cell.Y).As_Coord());
		}

		if (Is_Valid_Flaming_Guy_Cell(cell + Cell(0, ndy))) {
			return(Cell(cell.X, cell.Y + ndy).As_Coord());
		}
	}

	if (CELL_NONE == Cell(0, 0)) {
		FacingType dir = Random_Pick(FACING_FIRST, FacingType(FACING_COUNT - 1));
		for (int i = 0; i < FACING_COUNT; i++) {
			Cell newcell = Adjacent_Cell(cell, FacingType((dir + i) % FACING_COUNT));
			if (Is_Valid_Flaming_Guy_Cell(newcell)) {
				return(newcell);
			}
		}
	}

	return(COORD_NONE);
}


/// <summary>
/// Can a burning victim run into the specified cell?
/// This routine is used while choosing the next step for a man who is on fire. Rock,
/// tunnels, walls and any cell already occupied by a building or vehicle turn him away.
/// </summary>
/// <returns>bool; May the animation move into this cell?</returns>
bool AnimClass::Is_Valid_Flaming_Guy_Cell(Cell const & cell)
{
	CellClass * cptr = &Map[cell];

	if (cptr->IsUnderBridge && Get_Coord().Z - LEVEL_LEPTON_H * cptr->Height > 2 * LEVEL_LEPTON_H) {
		if ((cptr->BridgeFlag.Occupy.Building || cptr->BridgeFlag.Occupy.Monolith || cptr->BridgeFlag.Occupy.Vehicle)) {
			return(false);
		}
	} else {

		if (cptr->Land_Type() == LAND_ROCK || cptr->Land_Type() == LAND_TUNNEL || cptr->Land_Type() == LAND_WALL) {
			return(false);
		}

		if (cptr->Overlay != OVERLAY_NONE && OverlayTypes[cptr->Overlay]->IsWall) {
			return(false);
		}
		if ((cptr->Flag.Occupy.Building || cptr->Flag.Occupy.Monolith || cptr->Flag.Occupy.Vehicle) || LEVEL_LEPTON_H * cptr->Height - Get_Coord().Z > 2 * LEVEL_LEPTON_H) {
			return(false);
		}
	}
	return(true);
}


/// <summary>
/// Restores the drawing state of every animation after a save game is loaded.
/// The alternative drawer is a pointer that cannot survive a save, so the tiberium and
/// flaming guy animations have theirs looked up again here.
/// </summary>
/// <remarks>This routine tends to the entire animation list, so it need only be called
/// once per load.</remarks>
void AnimClass::Post_Load_Game(void)
{
	for (int i = 0; i < Anims.Count(); i++) {
		AnimClass * anim = Anims[i];
		if (anim->Class->IsAnimatedTiberium) {
			CellClass * cptr = &Map[anim->Get_Coord()];
			TiberiumType tib = cptr->Tiberium_Type_Here();
			if (tib != TIBERIUM_NONE) {
				TiberiumClass * tptr = Tiberiums[tib];
				if (tptr != NULL) {
					anim->AlternativeDrawer = ColorSchemes[tptr->Color]->Converter;
				}
			}
		}
		if (anim->Class->IsFlamingGuy) {
			anim->AlternativeDrawer = ColorSchemes[PlayerPtr->Scheme]->Converter;
		}
	}
}


ClassID AnimClass::Class_ID(void) const
{
	return(ClassID_AnimClass);
}


/// <summary>
/// Fetches the RTTI identifier for this object.
/// The identifier lets the rest of the game recognize an animation without having to
/// perform a dynamic cast.
/// </summary>
RTTIType AnimClass::Fetch_RTTI(void) const
{
	return(RTTI_ANIM);
}
