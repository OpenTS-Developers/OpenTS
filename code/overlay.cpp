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

/* $Header: /CounterStrike/OVERLAY.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OVERLAY.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 17, 1994                                                 *
 *                                                                                             *
 *                  Last Update : July 24, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   OverlayClass::Read_INI -- Reads the overlay data from an INI file.                        *
 *   OverlayClass::Init -- Resets the overlay object system.                                   *
 *   OverlayClass::Mark -- Marks the overlay down on the map.                                  *
 *   OverlayClass::OverlayClass -- Overlay object constructor.                                 *
 *   OverlayClass::delete -- Returns a overlay object to the pool.                             *
 *   OverlayClass::new -- Allocates a overlay object from pool                                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "overlay.h"

#include "_map.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "anim.h"
#include "bsurface.h"
#include "building.h"
#include "ccini.h"
#include "cell.h"
#include "dbgprint.h"
#include "draw.h"
#include "globals.h"
#include "house.h"
#include "ini.h"
#include "inline.h"
#include "lcwpipe.h"
#include "lcwstraw.h"
#include "lightcon.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "scheme.h"
#include "session.h"
#include "surface.h"
#include "tactical.h"
#include "tiberium.h"
#include "tracker.h"
#include "tube.h"
#include "vector.h"
#include "vein.h"
#include "xpipe.h"
#include "xstraw.h"

#include "overlay.hh"

#include <algorithm>


char const * const OverlayClass::INI_NAME = "OVERLAY";

HousesType OverlayClass::ToOwn = HOUSE_NONE;


/***********************************************************************************************
 * OverlayClass::OverlayClass -- Overlay object constructor.                                   *
 *                                                                                             *
 *    This is the constructor for a overlay object.                                            *
 *                                                                                             *
 * INPUT:   type  -- The overlay object this is to become.                                     *
 *                                                                                             *
 *          pos   -- The position on the map to place the object.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
OverlayClass::OverlayClass(OverlayTypeClass const * ttype, Cell const & pos, HousesType house) :
	Class((OverlayTypeClass *)ttype)
{
	assert(Class != NULL);

	Create_ID();

	Overlays.Add(this);

	if (pos != CELL_NONE) {
		ToOwn = house;
		Unlimbo(Coord(pos));
		ToOwn = HOUSE_NONE;
	}
}


/// <summary>
/// Removes the overlay from the game.
/// This routine detaches the overlay from anything that might still be referring to it
/// and then lifts it off the map.
/// </summary>
OverlayClass::~OverlayClass(void)
{
	Detach_This_From_All(this, true);
	Overlays.Delete(this);

	if (GameActive) {
		OverlayClass::Limbo();
	}
	Class = NULL;
}


/***********************************************************************************************
 * OverlayClass::Mark -- Marks the overlay down on the map.                                    *
 *                                                                                             *
 *    This routine will place the overlay onto the map. The overlay object is deleted by this  *
 *    operation. The map is updated to reflect the presence of the overlay.                    *
 *                                                                                             *
 * INPUT:   mark  -- The type of marking to perform. Only MARK_DOWN is supported.              *
 *                                                                                             *
 * OUTPUT:  bool; Was the overlay successfully marked? Failure occurs if it is not being       *
 *                marked down.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *   12/23/1994 JLB : Checks low level legality before proceeding.                             *
 *=============================================================================================*/
bool OverlayClass::Mark(MarkType mark)
{
	int i;

	if (BASECLASS::Mark(mark)) {
		if (mark == MARK_DOWN || mark == MARK_DOWN_FORCED) {
			Cell cell = PositionCell;
			CellClass * cellptr = &Map[cell];

			OverlayType type = Class->HeapID;

			if (cellptr->Ramp > 4 && type != OVERLAY_VEINHOLE_DUMMY) {
				return(false);
			}

			if (type == OVERLAY_BRIDGE1 || type == OVERLAY_BRIDGE2) {
				if (type == OVERLAY_BRIDGE1) {
					cellptr->Set_Under_Bridge(FACING_N);
				} else {
					cellptr->Set_Under_Bridge(FACING_W);
				}
			}

			if (type == OVERLAY_RAIL_BRIDGE1 || type == OVERLAY_RAIL_BRIDGE2) {
				if (type == OVERLAY_RAIL_BRIDGE1) {
					cellptr->Set_Under_Rail_Bridge(FACING_N);
				} else {
					cellptr->Set_Under_Rail_Bridge(FACING_W);
				}
			}

			if (type == OVERLAY_VEINHOLE) {
				if (VeinholeMonsterClass::Can_Monster_Go_Here(cellptr->CellID)) {
					for (int face = 0; face < FACING_COUNT; face++) {
						CellClass *adj = &cellptr->Adjacent_Cell(FacingType(face));
						adj->Overlay = OVERLAY_VEINHOLE_DUMMY;
						adj->OverlayData = 0;
					}
					cellptr->Overlay = OVERLAY_VEINHOLE;
					cellptr->OverlayData = 0;
					new VeinholeMonsterClass(cellptr->CellID);
				}
			} else if (type == OVERLAY_VEINS && ScenarioInit == 0) {
				cellptr->Place_Veins();
			} else if (Class->Land == LAND_RAILROAD) {
				cellptr->Overlay = Class->HeapID;
				cellptr->OverlayData = 0;
			}

			/*
			**	Walls have special logic when they are marked down.
			*/
			else if (Class->IsWall) {
				if (cellptr->Is_Clear_To_Build()) {
					cellptr->Overlay = Class->HeapID;
					cellptr->OverlayData = 0;
					cellptr->Wall_Update(true);
					if (!ScenarioInit) {
						Map.Update_Cell_Zone_Constructively(cellptr->CellID);
						Map.Update_Cell_Subzones(cellptr->CellID);
					}

					/*
					**	Flag ownership of the cell if the 'global' ownership flag indicates that this
					**	is necessary for the overlay.
					*/
					if (ToOwn != HOUSE_NONE) {
						cellptr->Owner = Map.PendingHouse;
					}

					for (int face = 0; face < FACING_COUNT; face++) {
						cellptr->Adjacent_Cell(FacingType(face)).AdjacentObjectCount++;
					}
				} else {
					Delete_Me();
					return(false);
				}
			}
			else if (Class->HeapID >= OVERLAY_LOWBRIDGE_FAKE_END1 && Class->HeapID <= OVERLAY_LOWBRIDGE_FAKE_END4)
			{
				int index = Class->HeapID - OVERLAY_LOWBRIDGE_FAKE_END1;

				static Cell _type_to_offset[4] = {Cell(0, -1), Cell(0, -1), Cell(-1, 0), Cell(-1, 0)};
				static FacingType _type_to_direction[4] = {FACING_S, FACING_S, FACING_E, FACING_E};
				static OverlayType _type_to_overlay[4] = {
					OVERLAY_LOWBRIDGE_19, OVERLAY_LOWBRIDGE_21, OVERLAY_LOWBRIDGE_23, OVERLAY_LOWBRIDGE_25
				};
				static FacingType _type_to_join_direction[4] = {FACING_W, FACING_E, FACING_S, FACING_N};
				static Cell _offset_fixup[2][3] = {
					Cell(-1, 0), Cell(0, 0), Cell(1, 0),
					Cell(0, -1), Cell(0, 0), Cell(0, 1)
				};
				static OverlayType _overlay_fixup[2] = {OVERLAY_LOWBRIDGE_10, OVERLAY_LOWBRIDGE_01};
				static OverlayType _direction_to_end[4] = {
					OVERLAY_LOWBRIDGE_23, OVERLAY_LOWBRIDGE_19, OVERLAY_LOWBRIDGE_25, OVERLAY_LOWBRIDGE_21
				};

				Cell start_cell = cell + _type_to_offset[index];
				Cell work_cell = start_cell;
				FacingType direction = _type_to_direction[index];
				bool clear = true;

				for (i = 0; i < 3; i++) {
					if (Map[work_cell].Overlay != OVERLAY_NONE) {
						clear = false;
					}
					work_cell = Adjacent_Cell(work_cell, direction);
				}

				work_cell = start_cell;
				if (clear) {
					for (i = 0; i < 3; i++) {
						CellClass *cptr = &Map[work_cell];
						cptr->Overlay = _type_to_overlay[index];
						cptr->OverlayData = i;
						cptr->Recalc_Attributes();
						work_cell = Adjacent_Cell(work_cell, direction);
					}
					FacingType join_direction = _type_to_join_direction[index];
					work_cell = Adjacent_Cell(cell, join_direction);
					bool found_end = false;
					OverlayType end_overlay = _direction_to_end[join_direction/2];

					while (Map.In_Radar(work_cell) && !found_end) {
						CellClass &check_cell_class = Map[work_cell];
						if (check_cell_class.Overlay == end_overlay && check_cell_class.OverlayData == 1) {
							found_end = true;
						} else {
							work_cell = Adjacent_Cell(work_cell, join_direction);
						}
					}
					if (found_end) {
						FacingType fixup_direction = Facing_Add(FACING_180, join_direction);
						work_cell = Adjacent_Cell(work_cell, fixup_direction);
						int length = std::max(abs(work_cell.X - start_cell.X), abs(work_cell.Y - start_cell.Y));
						for (i = 0; i < length; i++) {
							for (int j = 0; j < 3; j++) {
								CellClass *cptr = &Map[work_cell + _offset_fixup[(fixup_direction%4)/2][j]];
								cptr->Overlay = OverlayType(_overlay_fixup[(fixup_direction%4)/2] + (Scen->RandomNumber()&0x03));
								cptr->OverlayData = j;
								cptr->Recalc_Attributes();
							}
							work_cell = Adjacent_Cell(work_cell, fixup_direction);
						}
					}
				}
			}
			else
			{
				bool clear;
				if (!ScenarioInit) {
					if (Class == Rule->CrateImg || Class == Rule->WoodCrateImg) {
						clear = cellptr->Is_Clear_To_Move(SPEED_TRACK, false, false);
					} else {
						if (Class->HeapID == OVERLAY_BRIDGE1 || Class->HeapID == OVERLAY_BRIDGE2 ||
								Class->HeapID == OVERLAY_RAIL_BRIDGE1 || Class->HeapID == OVERLAY_RAIL_BRIDGE2) {
							clear = true;
						} else {
							if (Class->HeapID == OVERLAY_TIBERIUM01) {
								clear = cellptr->Is_Clear_To_Move(SPEED_TRACK, true, true);
								if (cellptr->Ramp > 4) clear = false;
							} else {
								clear = cellptr->Is_Clear_To_Move(SPEED_TRACK, true, true);
							}
						}
					}
				} else {
					clear = true;
				}

				if ((ScenarioInit || cellptr->Overlay == OVERLAY_NONE || !OverlayTypes[cellptr->Overlay]->IsOverrides) && clear) {

					cellptr->Overlay = Class->HeapID;
					if (type != OVERLAY_BRIDGE1 && type != OVERLAY_BRIDGE2 && type != OVERLAY_RAIL_BRIDGE1 && type != OVERLAY_RAIL_BRIDGE2) {
						cellptr->OverlayData = 0;
					}

					if (Class->Land == LAND_TIBERIUM) {
						cellptr->OverlayData = 1;
						cellptr->Tiberium_Adjust();
					}
				}

				if (Class->CellAnim != NULL) {
					Coord coord = Get_Coord();
					coord.Z = Map.Get_Height_GL(Get_Coord());
					AnimClass * aptr = new AnimClass(Class->CellAnim, coord + Coord(3 * CELL_LEPTON_W / 2, 3 * CELL_LEPTON_H / 2, 0));

					TiberiumType tibtype = cellptr->Tiberium_Type_Here();
					if (tibtype != TIBERIUM_NONE) {
						TiberiumClass *tiberium = Tiberiums[tibtype];
						if (tiberium != NULL) {
							aptr->AlternativeDrawer = ColorSchemes[tiberium->Color]->Converter;
							aptr->AlternativeBrightness = cellptr->Brightness;
						}
					}
				}
			}

			/*
			**	*****  Is this really needed?
			*/
			cellptr->Recalc_Attributes();

			/*
			**	Remove the overlay and make sure the system thinks it was never placed down!
			*/
			IsDown = false;
			IsInLimbo = true;

			Delete_Me();
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * OverlayClass::Read_INI -- Reads the overlay data from an INI file.                          *
 *                                                                                             *
 *    This routine is used to load a scenario's overlay data. The overlay objects are read     *
 *    from the INI file and then created on the map.                                           *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the INI file staging buffer.                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Requires that all the buildings be placed first, so the scan for assigning wall *
 *             ownership to the nearest building will work.                                    *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *   07/24/1995 JLB : Specifically forbid manual crates in multiplayer scenarios.              *
 *=============================================================================================*/
void OverlayClass::Read_INI(CCINIClass const & ini)
{
	if (NewINIFormat > 1) {

		BSurface temp_surface(640, 400, 2);
		temp_surface.Fill(0);

		int len = ini.Get_UUBlock("OverlayPack", temp_surface.Lock(), temp_surface.Get_Width() * temp_surface.Get_Height() * temp_surface.Bytes_Per_Pixel());

		if (len > 0) {
			BufferStraw bpipe(temp_surface.Lock(), len);
			LCWStraw uncomp(LCWStraw::DECOMPRESS);
			uncomp.Get_From(&bpipe);

			for (int y = 0; y < MAP_CELL_H; y++) {
				for (int x = 0; x < MAP_CELL_W; x++) {
					Cell cell(x, y);

					OverlayType classid = OVERLAY_NONE;

					uncomp.Get(&(char&)classid, sizeof(char));
					if (classid != OVERLAY_NONE) {
						classid = OverlayType(classid & 0x00FF);
					}

					if (classid != OVERLAY_NONE && (OverlayTypes[classid]->Get_Image_Data() != NULL || OverlayTypes[classid]->CellAnim)) {

						/*
						**	Don't allow placement of crates in the multiplayer scenarios.
						*/
						if (Session.Type == GAME_NORMAL || !OverlayTypes[classid]->IsCrate) {

							/*
							**	Don't allow placement of overlays on the top or bottom rows of
							**	the map.
							*/
							if (Map.In_Radar(cell)) {
								unsigned char old_overlay_data = (&Map[cell])->OverlayData;
								new OverlayClass(OverlayTypes[classid], cell);

								if ((int)classid == OVERLAY_BRIDGE1 || (int)classid == OVERLAY_BRIDGE2 ||
									(int)classid == OVERLAY_RAIL_BRIDGE1 || (int)classid == OVERLAY_RAIL_BRIDGE2) {
									(&Map[cell])->OverlayData = old_overlay_data;
								}
							}
						}
					}
				}
			}
			temp_surface.Unlock();
		}
		temp_surface.Unlock();

		len = ini.Get_UUBlock("OverlayDataPack", temp_surface.Lock(), temp_surface.Get_Width() * temp_surface.Get_Height());

		if (len > 0) {
			BufferStraw databpipe(temp_surface.Lock(), len);
			LCWStraw datauncomp(LCWStraw::DECOMPRESS);
			datauncomp.Get_From(&databpipe);

			for (int y = 0; y < MAP_CELL_H; y++) {
				for (int x = 0; x < MAP_CELL_W; x++) {
					Cell cell(x, y);
					unsigned char overlay_data = 0;
					datauncomp.Get(&(char &)overlay_data, sizeof(char));

					if (Map.In_Radar(cell)) {
						CellClass *cellptr = &Map[cell];
						cellptr->OverlayData = overlay_data;
					}
				}
			}
			temp_surface.Unlock();
		}
		temp_surface.Unlock();
	}
	Process_Deferred_Deletion();
}


/// <summary>
/// Stores the map's overlay layer into the INI database.
/// This routine is used when a scenario is saved. The overlay and overlay data of every
/// cell are compressed into the OverlayPack and OverlayDataPack sections, replacing
/// whatever the database held before.
/// </summary>
void OverlayClass::Write_INI(CCINIClass & ini)
{
	/*
	**	First, clear out all existing overlay data from the ini file.
	*/
	ini.Clear(INI_NAME);
	ini.Clear("OverlayPack");

	BufferPipe bpipe(AlternateSurface->Lock(), AlternateSurface->Get_Width() * AlternateSurface->Get_Height());
	LCWPipe comppipe(LCWPipe::COMPRESS);

	comppipe.Put_To(&bpipe);

	int total = 0;
	int y;
	for (y = 0; y < MAP_CELL_H; y++) {
		for (int x = 0; x < MAP_CELL_W; x++) {
			total += comppipe.Put(&(char&)(Map[Cell(x, y)].Overlay), sizeof(char));
		}
	}
	if (total) {
		ini.Put_UUBlock("OverlayPack", AlternateSurface->Lock(), total);
		AlternateSurface->Unlock();
	}
	AlternateSurface->Unlock();

	ini.Clear("OverlayDataPack");

	BufferPipe databpipe(AlternateSurface->Lock(), AlternateSurface->Get_Width() * AlternateSurface->Get_Height());
	LCWPipe datacomppipe(LCWPipe::COMPRESS);

	datacomppipe.Put_To(&databpipe);

	total = 0;
	for (y = 0; y < MAP_CELL_H; y++) {
		for (int x = 0; x < MAP_CELL_W; x++) {
			total += datacomppipe.Put(&(char&)(Map[Cell(x, y)].OverlayData), sizeof(char));
		}
	}
	if (total) {
		ini.Put_UUBlock("OverlayDataPack", AlternateSurface->Lock(), total);
		AlternateSurface->Unlock();
	}
	AlternateSurface->Unlock();

}


/// <summary>
/// Lists the members this overlay carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void OverlayClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
}


/// <summary>
/// Draws this overlay onto the tactical map.
/// This routine handles the three flavors of overlay separately. Tiberium is drawn with
/// the color scheme of the tiberium it belongs to, veins with the player's own scheme,
/// and every other overlay with the drawer of the cell it occupies.
/// </summary>
/// <param name="point">The pixel location to draw the overlay at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void OverlayClass::Editor_Draw_It(Point2D const & point, Rect const & cliprect) const
{
	ShapeSet const * shapefile = (ShapeSet const *)Class->Get_Image_Data();
	int yadjust = TacticalMap->Z_Lepton_To_Pixel(Map.Get_Height_GL(Get_Coord()));

	Point2D drawpoint = point + Overlay_Draw_Offset(OverlayType(OverlayTypes.ID(Class)));
	drawpoint.Y += TacticalRect.Y;

	if (Class->IsTiberium) {
		TiberiumClass const * tib = Tiberiums[Which_Tiberium_Type(OverlayType(OverlayTypes.ID(Class)))];
		ColorScheme const * scheme = ColorSchemes[tib->Color];
		int variety = 0;
		shapefile = (ShapeSet const *)OverlayTypes[tib->Overlay->HeapID + variety]->Get_Image_Data();
		if (!shapefile && Class->CellAnim) {
			shapefile = (ShapeSet const *)Class->CellAnim->Get_Image_Data();
		}
		Draw_Shape(*LogicalSurface, *(ConvertClass *)scheme->Converter, shapefile, 0, drawpoint, cliprect, (ShapeFlags_Type)(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA), NULL, -2-yadjust, ZGRAD_GROUND);
	} else if (Class->IsVeins) {
		if (Class->HeapID == OVERLAY_VEINHOLE) {
			drawpoint.Y -= 38;
		}
		Draw_Shape(*LogicalSurface, *ColorSchemes[PlayerPtr->Scheme]->Converter, shapefile, 0, drawpoint, cliprect, (ShapeFlags_Type)(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA), NULL, -2-yadjust, ZGRAD_GROUND, Map[Get_Coord()].TileBrightness);
	} else {
		CellClass *cellptr = NULL;
		if (Map.In_Radar(Get_Coord().As_Cell())) {
			cellptr = &Map[Get_Coord()];
		} else if (Map.In_Radar(TacticalMap->Pixel_To_Cell(drawpoint))) {
			cellptr = &Map[TacticalMap->Pixel_To_Cell(drawpoint)];
		} else {
			cellptr = &Map[Cell(1, Map.PlayRect.Width)];
		}
		Draw_Shape(*LogicalSurface, *(ConvertClass *)(cellptr->Drawer), shapefile, 0, drawpoint, cliprect, (ShapeFlags_Type)(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA), NULL, -2-yadjust, ZGRAD_GROUND, cellptr->TileBrightness);
	}
}


/// <summary>
/// Fetches the drawing offset that an overlay requires.
/// Not every overlay sits centered on its cell the way the generic draw code assumes.
/// This routine is used to nudge those overlays into place before their shape is drawn.
/// </summary>
/// <param name="overlay">The overlay type to fetch the offset for.</param>
/// <returns>Returns with the pixel offset to add to the overlay's draw point.</returns>
Point2D Overlay_Draw_Offset(OverlayType overlay)
{
	OverlayTypeClass const & otype = *OverlayTypes[overlay];
	Point2D offset(0, 0);

	if (otype.IsTiberium || otype.IsWall || otype.HeapID == OVERLAY_VEINS || otype.IsCrate) {
		offset.Y -= 12;
	}
	if (otype.Land == LAND_RAILROAD) {
		offset.Y -= 1;
	}
	if (overlay == OVERLAY_VEINS) {
		offset.Y -= 1;
	}

	return(offset);
}


/// <summary>
/// Determines which tiberium an overlay belongs to.
/// Each tiberium claims a block of overlay types for its growth and ramp stages. Use
/// this routine to get back from one of those overlays to the tiberium that owns it.
/// </summary>
/// <param name="overlay">The overlay type to identify.</param>
/// <returns>Returns with the tiberium type that owns the overlay. If the overlay is not
/// tiberium at all, then TIBERIUM_NONE is returned.</returns>
TiberiumType Which_Tiberium_Type(OverlayType overlay)
{
	if (overlay != OVERLAY_NONE) {
		OverlayTypeClass const * otype = OverlayTypes[overlay];
		if (otype->IsTiberium) {
			for (int index = 0; index < Tiberiums.Count(); index++) {
				TiberiumClass const * tiberium = Tiberiums[index];

				if (overlay >= tiberium->Overlay->HeapID && overlay < tiberium->Overlay->HeapID + tiberium->Variety) {
					return(TiberiumType(tiberium->HeapID));
				}

				if (overlay >= tiberium->Overlay->HeapID + tiberium->Variety && overlay < tiberium->Overlay->HeapID + tiberium->Variety+tiberium->RampVariety) {
					return(tiberium->HeapID);
				}
			}

			DebugString("Overlay %s not really tiberium\n", (char const *)otype->GivenName);
			return(TIBERIUM_RIPARIUS);
		}
	}
	return(TIBERIUM_NONE);
}


/// <summary>
/// Fetches the type class of this overlay.
/// </summary>
/// <returns>Returns with a pointer to the type class this overlay was made from.</returns>
ObjectTypeClass const * OverlayClass::Class_Of(void) const
{
	return(Class);
}


/// <summary>
/// Repairs the vein overlay after a scenario has been loaded.
/// This routine is called once the map cells are in place. The saved veins are
/// stripped back to bare ground and the solid ones are then placed again, so that the
/// vein network ends up consistent with the terrain it grew over.
/// </summary>
void OverlayClass::Post_Read_Vein_Fixups(void)
{
	int index;
	Map.Reset_Iterator();
	CellClass *cellptr = Map.Iterate();

	DynamicVectorClass<CellClass *> solid_vein_cells;
	solid_vein_cells.Set_Growth_Step(1000);

	while (cellptr) {
		if (cellptr->Overlay == OVERLAY_VEINS) {
			if (cellptr->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN) {
				solid_vein_cells.Add(cellptr);
			}
			cellptr->Overlay = OVERLAY_NONE;
			cellptr->OverlayData = 0;
		}
		cellptr = Map.Iterate();
	}

	for (index = solid_vein_cells.Count() - 1; index >= 0; index--) {
		CellClass *tmp = solid_vein_cells[index];
		if (tmp->Can_Place_Veins()) {
			tmp->Place_Veins();
		}
	}
}
