/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "isotile.h"

#include "_isotile.h"
#include "_map.h"
#include "anim.h"
#include "cell.h"
#include "globals.h"
#include "isotype.h"
#include "overtype.h"
#include "savestream.h"
#include "tracker.h"
#include "vector.h"


/// <summary>
/// Marks this tile up or down on the map.
/// This routine stamps the tile's terrain -- tile index, subtile, ramp and height -- into
/// every cell it covers, or lifts that data back out again. Marking down also clears any
/// smudge or overlay those cells were carrying and repairs the neighboring LAT edges.
/// </summary>
/// <param name="mark">The marking operation to perform.</param>
/// <returns>bool; Was the tile marked?</returns>
/// <remarks>Marking the tile down destroys it. The terrain now lives in the cells, so the
/// tile must not be referred to again after such a call.</remarks>
bool IsometricTileClass::Mark(MarkType mark)
{
	IsoTileSet * set = (IsoTileSet *)Get_Image_Data();
	if (set != NULL && BASECLASS::Mark(mark)) {

		for (int y = 0; y < Class->Height; y++) {
			for (int x = 0; x < Class->Width; x++) {
				Cell cell = PositionCell;
				cell += Cell(x, y);

				if (Map.In_Radar(cell)) {
					CellClass *cptr = &Map[cell];
					int subtile = Class->SubTile_Index(x, y);
					IsoTileRecord const * record = set->Fetch_Record_Pointer_Unsafe(subtile);
					if (record != NULL) {
						if (mark == MARK_UP) {
							if (cptr->ITType == Class->HeapID && cptr->SubTile == subtile) {
								cptr->ITType = TILE_NONE;
								cptr->SubTile = 0;
								cptr->Height -= record->Height;
							}
						} else if (mark == MARK_DOWN || mark == MARK_DOWN_FORCED) {
							if (Class->HeapID == TILE_CLEAR) {
								cptr->ITType = TILE_NONE;
								cptr->SubTile = 0;
							} else {
								cptr->ITType = Class->HeapID;
								cptr->SubTile = subtile;
								cptr->Ramp = Class->Ramp_Type(subtile);
								if (cptr->ITType == IsometricTileTypeClass::SlopeSetPieces + 5) {
									switch (cptr->SubTile) {
										case 0:
										case 3:
										case 6:
										case 9:
											cptr->ITType = IsometricTileType(IsometricTileTypeClass::RampStart + 1);
											cptr->SubTile = 0;
											break;
										default:
											break;
									}
								}
								if (cptr->ITType == IsometricTileTypeClass::SlopeSetPieces2 + 5) {
									switch (cptr->SubTile) {
										case 0:
										case 3:
										case 6:
										case 9:
											cptr->ITType = IsometricTileType(IsometricTileTypeClass::MMRampBase + 1);
											cptr->SubTile = 0;
											break;
										default:
											break;
									}
								}
								if (cptr->ITType == IsometricTileTypeClass::SlopeSetPieces + 8 && cptr->SubTile <= 3) {
									cptr->ITType = IsometricTileTypeClass::RampStart;
									cptr->SubTile = 0;
								}
								if (cptr->ITType == IsometricTileTypeClass::SlopeSetPieces2 + 8 && cptr->SubTile <= 3) {
									cptr->ITType = IsometricTileTypeClass::MMRampBase;
									cptr->SubTile = 0;
								}
							}
							cptr->Smudge = SMUDGE_NONE;
							cptr->SmudgeData = 0;
							if (cptr->Overlay >= OVERLAY_LARGE_TIBERIUM01 && cptr->Overlay < OVERLAY_LARGE_TIBERIUM12) {
								cptr->IsAnimAttached = false;
								Coord ccoord = cptr->Center_Coord() + Coord(CELL_LEPTON_W + (CELL_LEPTON_W >> 1), CELL_LEPTON_H + (CELL_LEPTON_H >> 1), 0);
								for (int i = 0; i < Anims.Count(); i++) {
									AnimClass *anim = Anims[i];
									if (anim->Class == OverlayTypes[cptr->Overlay]->CellAnim) {
										if (anim->PositionCoord == ccoord) {
											delete anim;
											break;
										}
									}
								}
							}
							cptr->Overlay = OVERLAY_NONE;
							cptr->OverlayData = 0;
							cptr->Height += record->Height;
							cptr->Fixup_LAT();
							cptr->Adjacent_Cell(FACING_N).Fixup_LAT();
							cptr->Adjacent_Cell(FACING_E).Fixup_LAT();
							cptr->Adjacent_Cell(FACING_S).Fixup_LAT();
							cptr->Adjacent_Cell(FACING_W).Fixup_LAT();
						}
						cptr->Recalc_Attributes();
					}
				}
			}
		}

		if (mark == MARK_DOWN || mark == MARK_DOWN_FORCED) {
			IsDown = false;
			delete this;
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Creates an isometric tile of the type specified.
/// The tile is stamped onto the map straight away when a real cell is supplied, and is
/// added to the global isometric tile list either way.
/// </summary>
/// <param name="cell">The cell to place the tile at, or CELL_NONE to leave it in limbo.</param>
IsometricTileClass::IsometricTileClass(IsometricTileType type, Cell const & cell) :
	BASECLASS(),
	Class(IsometricTileTypes[type])
{
	if (cell != CELL_NONE) {
		Unlimbo(Coord(cell));
	}

	IsometricTiles.Add(this);
}


/// <summary>
/// Destroys this isometric tile.
/// The tile is detached from everything that refers to it, lifted back off the map, and
/// dropped from the isometric tile list.
/// </summary>
IsometricTileClass::~IsometricTileClass(void)
{
	Detach_This_From_All(this, true);
	Limbo();
	Class = NULL;
	IsometricTiles.Delete(this);
	AbstractTypePtrTracker.Delete(this);
}


/// <summary>
/// Lists the members this tile carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void IsometricTileClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
}


/// <summary>
/// Places this tile onto the map at the coordinate specified.
/// This routine will stamp the tile's terrain data down into every cell it covers.
/// </summary>
/// <returns>bool; Was the tile placed onto the map?</returns>
/// <remarks>Placing the tile down destroys it. Do not refer to the tile after this call.</remarks>
bool IsometricTileClass::Unlimbo(Coord const & coord, Dir256 dir)
{
	IsInLimbo = false;
	PositionCoord = coord;
	return(Mark(MARK_DOWN));
}


/// <summary>
/// Removes this tile from the map.
/// This routine will unselect the tile, break every attachment to it, and lift its
/// terrain data back out of the cells it was covering.
/// </summary>
/// <returns>bool; Was the tile removed from the map?</returns>
bool IsometricTileClass::Limbo(void)
{
	if (!GameActive || IsInLimbo) {
		return(false);
	}
	Unselect();
	Detach_All(true);
	Mark(MARK_UP);
	IsInLimbo = true;
	IsToDisplay = false;
	return(true);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_ISOTILE.</returns>
RTTIType IsometricTileClass::Fetch_RTTI(void) const
{
	return(RTTI_ISOTILE);
}


ClassID IsometricTileClass::Class_ID(void) const
{
	return(ClassID_IsometricTileClass);
}


/// <summary>
/// Fetches the object type class of this tile.
/// </summary>
/// <returns>Returns with a pointer to the isometric tile type this tile was built from.</returns>
ObjectTypeClass const * IsometricTileClass::Class_Of(void) const
{
	return(Class);
}


/// <summary>
/// Draws this isometric tile.
/// The tactical map renders the terrain itself as part of its own drawing pass, so an
/// isometric tile never draws itself through the normal object display path.
/// </summary>
void IsometricTileClass::Draw_It(Point2D const & point, Rect const & cliprect) const
{
	//nothing
}
