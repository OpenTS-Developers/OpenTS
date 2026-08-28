/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "smartdeform.h"

#include "_isotile.h"
#include "_map.h"
#include "_rect.h"
#include "_tactica.h"
#include "cell.h"
#include "inline.h"
#include "isotype.h"
#include "mapgen.h"
#include "object.h"
#include "smudtype.h"
#include "tactical.h"

#include "ramp.hh"

#include <algorithm>

#define MAX_TERRAIN_HEIGHT (12)

#define NUM_CORNERS 4

#define RAMP_FULL_HEIGHT (ISO_TILE_PIXEL_H)
#define RAMP_HALF_HEIGHT (RAMP_FULL_HEIGHT / 2)

/// A flat tile
static int _ramp_corners0[NUM_CORNERS] = {00, 00, 00, 00};

/// Standard (two corners high).
static int _ramp_corners1[NUM_CORNERS] = {00, RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT, 00};
static int _ramp_corners2[NUM_CORNERS] = {00, 00, RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT};
static int _ramp_corners3[NUM_CORNERS] = {RAMP_HALF_HEIGHT, 00, 00, RAMP_HALF_HEIGHT};
static int _ramp_corners4[NUM_CORNERS] = {RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT, 00, 00};

/// Tile outside corners (one corner high).
static int _ramp_corners5[NUM_CORNERS] = {00, 00, RAMP_HALF_HEIGHT, 00};
static int _ramp_corners6[NUM_CORNERS] = {00, 00, 00, RAMP_HALF_HEIGHT};
static int _ramp_corners7[NUM_CORNERS] = {RAMP_HALF_HEIGHT, 00, 00, 00};
static int _ramp_corners8[NUM_CORNERS] = {00, RAMP_HALF_HEIGHT, 00, 00};

/// Tile inside corners (three corners high).
static int _ramp_corners9[NUM_CORNERS] = {00, RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT};
static int _ramp_corners10[NUM_CORNERS] = {RAMP_HALF_HEIGHT, 00, RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT};
static int _ramp_corners11[NUM_CORNERS] = {RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT, 00, RAMP_HALF_HEIGHT};
static int _ramp_corners12[NUM_CORNERS] = {RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT, RAMP_HALF_HEIGHT, 00};

/// Steep (two corners high, one corner double high).
static int _ramp_corners13[NUM_CORNERS] = {00, RAMP_HALF_HEIGHT, RAMP_FULL_HEIGHT, RAMP_HALF_HEIGHT};
static int _ramp_corners14[NUM_CORNERS] = {RAMP_HALF_HEIGHT, 00, RAMP_HALF_HEIGHT, RAMP_FULL_HEIGHT};
static int _ramp_corners15[NUM_CORNERS] = {RAMP_FULL_HEIGHT, RAMP_HALF_HEIGHT, 00, RAMP_HALF_HEIGHT};
static int _ramp_corners16[NUM_CORNERS] = {RAMP_HALF_HEIGHT, RAMP_FULL_HEIGHT, RAMP_HALF_HEIGHT, 00};

/// Double ramps (two corners high, alternating).
static int _ramp_corners17[NUM_CORNERS] = {00, RAMP_HALF_HEIGHT, 00, RAMP_HALF_HEIGHT};
static int _ramp_corners18[NUM_CORNERS] = {RAMP_HALF_HEIGHT, 00, RAMP_HALF_HEIGHT, 00};
static int _ramp_corners19[NUM_CORNERS] = {00, RAMP_HALF_HEIGHT, 00, RAMP_HALF_HEIGHT};
static int _ramp_corners20[NUM_CORNERS] = {RAMP_HALF_HEIGHT, 00, RAMP_HALF_HEIGHT, 00};

static int * _ramp_corners[RAMP_COUNT] = {
	_ramp_corners0,
	_ramp_corners1,
	_ramp_corners2,
	_ramp_corners3,
	_ramp_corners4,
	_ramp_corners5,
	_ramp_corners6,
	_ramp_corners7,
	_ramp_corners8,
	_ramp_corners9,
	_ramp_corners10,
	_ramp_corners11,
	_ramp_corners12,
	_ramp_corners13,
	_ramp_corners14,
	_ramp_corners15,
	_ramp_corners16,
	_ramp_corners17,
	_ramp_corners18,
	_ramp_corners19,
	_ramp_corners20
};


struct DeformPointStruct {

	DeformPointStruct(void)
	{
		Height = 0;
		Rigid = false;
		Done = false;
	};

	DeformPointStruct(const DeformPointStruct & that)
	{
		Height = that.Height;
		Rigid = that.Rigid;
		Done = that.Done;
	};

	int Height;
	bool Rigid;
	bool Done;
	bool CanForce;
};


extern "C" {

DeformPointStruct * DeformPoints = NULL;

int DeformPointXAdd = 0;
int DeformPointYAdd = 0;

int DeformPointWidth = 0;
int DeformPointHeight = 0;

}


struct DeformUndoStruct {

	DeformUndoStruct(void)
	{
		CellPoint = Point2D(0, 0);
		OldHeight = 0;
	};

	DeformUndoStruct(const Point2D & point, int oldheight)
	{
		CellPoint = point;
		OldHeight = oldheight;
	};

	bool operator==(DeformUndoStruct const & rvalue) const { return(CellPoint == rvalue.CellPoint && OldHeight == rvalue.OldHeight); }
	bool operator!=(DeformUndoStruct const & rvalue) const { return(CellPoint != rvalue.CellPoint || OldHeight != rvalue.OldHeight); }

	Point2D CellPoint;
	int OldHeight;
};


DynamicVectorClass<DeformUndoStruct> UndoRecords;


/// <summary>
/// Records a smoothing point's height so it can be restored.
/// This routine is called just before a point is moved, so that Undo_Deform can put it
/// back if the height change turns out to be impossible.
/// </summary>
/// <param name="point">The smoothing point about to be changed.</param>
/// <param name="oldheight">The height the point holds at the moment.</param>
void Record_Deform_Undo(const Point2D & point, int oldheight)
{
	UndoRecords.Add(DeformUndoStruct(point, oldheight));
}


/// <summary>
/// Restores the smoothing points to their recorded heights.
/// This routine backs out a height change that could not be completed, putting every point
/// recorded since the operation began back the way it was found and emptying the undo record.
/// </summary>
void Undo_Deform(void)
{
	for (int i = UndoRecords.Count() - 1; i >= 0; i--) {
		DeformPointStruct & new_point = DeformPoints[UndoRecords[i].CellPoint.X + UndoRecords[i].CellPoint.Y * DeformPointWidth];
		new_point.Height = UndoRecords[i].OldHeight;
	}
	UndoRecords.Clear();
}


/// <summary>
/// Can this cell's terrain be reshaped by the generator?
/// This routine is the map generator's counterpart to Can_Deform_Cell. A cell beyond the playable
/// area, one that is occupied or carries an overlay, one the generator has already declared
/// inviolate, or one whose tile type is not morphable is left as it stands.
/// </summary>
/// <param name="cellptr">Pointer to the cell to examine.</param>
/// <returns>bool; May the cell be reshaped?</returns>
bool Can_Deform_Cell_RMG(CellClass * cellptr, bool)
{
	if (!My_In_Radar(cellptr->Fetch_CellID())) {
		return(false);
	}

	if (cellptr->Overlay != OVERLAY_NONE || cellptr->Cell_Occupier()) {
		return(false);
	}

	if (MapRegionClass::Get_Cell_Data(cellptr->Fetch_CellID()).Inviolate) {
		return(false);
	}

	IsometricTileType tile_type = cellptr->ITType;
	if (tile_type >= IsometricTileTypes.Count()) {
		tile_type = TILE_NONE;
	}

	if (tile_type != TILE_NONE) {
		IsometricTileTypeClass const * ittype = IsometricTileTypes[tile_type];
		return(ittype->IsMorphable);
	}
	return(true);
}


/// <summary>
/// Can this cell's terrain be reshaped?
/// This routine is used by the smoothing system to decide whether a cell may take part in a
/// height change. Anything anchored to the ground keeps the cell where it is -- a structure or
/// terrain object standing on it, a bridge above or below it, an overlay, ground outside the
/// visible map, or a tile type that is not morphable. Infantry and vehicles ride the change and
/// do not block it.
/// </summary>
/// <param name="cellptr">Pointer to the cell to examine.</param>
/// <returns>bool; May the cell be reshaped?</returns>
bool Can_Deform_Cell(CellClass * cellptr, bool)
{
	ObjectClass * obj = cellptr->Cell_Occupier();
	while (obj) {
		if (obj->IsActive && obj->RTTI != RTTI_INFANTRY && obj->RTTI != RTTI_UNIT) {
			return(false);
		}
		obj = obj->Next;
	}

	if ((cellptr->Cell_Bridge_Occupier() || cellptr->IsUnderBridge || cellptr->WasUnderBridge)) {
		return(false);
	}

	CellClass * adj_cellptr = &Map[Adjacent_Cell(cellptr->CellID, FACING_N)];
	if ((adj_cellptr->WasUnderBridge || adj_cellptr->IsUnderBridge) && adj_cellptr->IsBridgeEastWest) {
		return(false);
	}

	adj_cellptr = &Map[Adjacent_Cell(cellptr->CellID, FACING_W)];
	if ((adj_cellptr->WasUnderBridge || adj_cellptr->IsUnderBridge) && !adj_cellptr->IsBridgeEastWest) {
		return(false);
	}

	if (cellptr->Overlay != OVERLAY_NONE) {
		return(false);
	}

	if (!Map.In_Radar(cellptr->CellID)) return(false);

	IsometricTileType tile_type = cellptr->ITType;
	if (tile_type >= IsometricTileTypes.Count()) {
		tile_type = TILE_NONE;
	}

	if (tile_type != TILE_NONE) {
		IsometricTileTypeClass const * ittype = IsometricTileTypes[tile_type];
		return(ittype->IsMorphable);
	}
	return(true);
}


/// <summary>
/// Prepares the smoothing point array around a cell.
/// This routine allocates the corner point grid and seeds the points near the center cell from
/// the height and ramp of the cell beneath them, pinning those that sit on ground the terrain
/// will not let move. Points belonging to the center cell itself are flagged as forceable, so a
/// forced change can shift them even when they are pinned.
/// </summary>
/// <param name="center">The cell the height change will be centered on.</param>
/// <param name="forced">Should a cell that would refuse the change be treated as movable?</param>
/// <remarks>Commit_Deform_Grid releases the array this routine allocates.</remarks>
void Init_Deform_Grid(Cell center, bool forced)
{
	if (DeformPoints) {
		delete[] DeformPoints;
		DeformPoints = NULL;
	}

	DeformPoints = new DeformPointStruct[(Map.MapRect.Width + 1) * (Map.MapRect.Height + 1)];
	DeformPointWidth = Map.MapRect.Width + 1;
	DeformPointHeight = Map.MapRect.Height + 1;

	DeformPointXAdd = Map.MapRect.X;
	DeformPointYAdd = Map.MapRect.Y;

	int point_height;
	int height;
	int ramp;

	Cell top_left = center - Cell(MAX_TERRAIN_HEIGHT + 2, MAX_TERRAIN_HEIGHT + 2);
	Cell bottom_right = center + Cell(MAX_TERRAIN_HEIGHT + 2, MAX_TERRAIN_HEIGHT + 2);

	top_left.X = std::max<int>(0, top_left.X);
	top_left.Y = std::max<int>(0, top_left.Y);

	bottom_right.X = std::min<int>(Map.MapRect.Width - Map.MapRect.X, bottom_right.X);
	bottom_right.Y = std::min<int>(Map.MapRect.Height - Map.MapRect.Y, bottom_right.Y);

	for (int y = top_left.Y; y <= bottom_right.Y; y++) {
		for (int x = top_left.X; x <= bottom_right.X; x++) {

			DeformPointStruct & deform_point = DeformPoints[x + y * DeformPointWidth];
			deform_point.Rigid = false;
			deform_point.Done = false;
			deform_point.CanForce = false;

			point_height = -1;
			Cell deform_cell(x + DeformPointXAdd, y + DeformPointYAdd);

			if (deform_cell.X >= Map.MapRect.X && deform_cell.X <= Map.MapRect.X + Map.MapRect.Width) {
				if (deform_cell.Y >= Map.MapRect.Y && deform_cell.Y <= Map.MapRect.Y + Map.MapRect.Height) {
					ramp = Map[deform_cell].Ramp;
					height = Map[deform_cell].Height * RAMP_HALF_HEIGHT;

					int * ramptype = _ramp_corners[ramp];
					height += ramptype[0];

					point_height = height;

					if (!Can_Deform_Cell(&Map[deform_cell], forced)) {
						deform_point.Rigid = true;
					} else {
						if (!Can_Deform_Cell(&Map[deform_cell + Cell(0, -1)], forced)) {
							deform_point.Rigid = true;
						} else {
							if (!Can_Deform_Cell(&Map[deform_cell + Cell(-1, -1)], forced)) {
								deform_point.Rigid = true;
							} else {
								if (!Can_Deform_Cell(&Map[deform_cell + Cell(-1, 0)], forced)) {
									deform_point.Rigid = true;
								}
							}
						}
					}

					if (deform_cell == center || (deform_cell + Cell(0, -1)) == center || (deform_cell + Cell(-1, -1)) == center || (deform_cell + Cell(-1, 0)) == center) {
						deform_point.CanForce = true;
					}
				}
			}

			deform_point.Height = point_height;
		}
	}
}


/// <summary>
/// Prepares the smoothing point array for the map generator.
/// This routine allocates the corner point grid for the whole map and seeds every point from
/// the height and ramp of the cell beneath it, pinning the points that sit on ground the
/// generator is not allowed to reshape.
/// </summary>
/// <remarks>Commit_Deform_Grid_RMG releases the array this routine allocates.</remarks>
void Init_Deform_Grid_RMG(void)
{
	if (DeformPoints != NULL) {
		delete [] DeformPoints;
		DeformPoints = NULL;
	}

	DeformPoints = new DeformPointStruct[(Map.MapRect.Width + 1) * (Map.MapRect.Height + 1)];
	DeformPointWidth = Map.MapRect.Width + 1;
	DeformPointHeight = Map.MapRect.Height + 1;

	DeformPointXAdd = Map.MapRect.X;
	DeformPointYAdd = Map.MapRect.Y;

	int point_height;
	int height;
	int ramp;

	Cell radial_cell_array[3] = {
		Cell(0, -1),
		Cell(-1, -1),
		Cell(-1, 0)
	};

	for (int y = 0; y < DeformPointHeight; y++) {
		for (int x = 0; x < DeformPointWidth; x++) {

			DeformPointStruct & deform_point = DeformPoints[x + y * DeformPointWidth];
			deform_point.Rigid = false;
			deform_point.Done = false;

			point_height = 0;
			Cell cell(x + DeformPointXAdd, y + DeformPointYAdd);
			Cell loop_cell = cell;
			int i = 0;

			while (!My_In_Radar(loop_cell) && i < 3) {
				loop_cell = cell + radial_cell_array[i];
				i++;
			}

			if (My_In_Radar(loop_cell)) {
				ramp = Map[loop_cell].Ramp;
				height = Map[loop_cell].Height * RAMP_HALF_HEIGHT;

				int * ramptype = _ramp_corners[ramp];
				height += ramptype[0];

				point_height = height;

				if (My_In_Radar(cell + Cell(0, -1)) && !Can_Deform_Cell_RMG(&Map[cell + Cell(0, -1)], true)) deform_point.Rigid = true;
				if (My_In_Radar(cell + Cell(-1, -1)) && !Can_Deform_Cell_RMG(&Map[cell + Cell(-1, -1)], true)) deform_point.Rigid = true;
				if (My_In_Radar(cell + Cell(-1, 0)) && !Can_Deform_Cell_RMG(&Map[cell + Cell(-1, 0)], true)) deform_point.Rigid = true;
				if (My_In_Radar(cell) && !Can_Deform_Cell_RMG(&Map[cell], true)) deform_point.Rigid = true;
			} else {
				deform_point.Rigid = true;
			}
			deform_point.Height = point_height;
		}
	}
}


/// <summary>
/// Writes the smoothed points back to the map.
/// This routine settles each cell around the change onto the lowest of its four smoothing
/// points and gives it the ramp tile matching the slope those points describe. Objects standing
/// on a cell are lifted off and set back down around the alteration, smudges that no longer fit
/// the ground are scrubbed away, and the tactical map is told which areas need redrawing.
/// </summary>
/// <param name="center">The cell the height change was centered on.</param>
/// <param name="forced">Should a cell that would refuse the change be reshaped anyway?</param>
/// <remarks>The smoothing point array is released here, so it must be prepared again before the
/// next height change.</remarks>
bool Commit_Deform_Grid(Cell center, bool forced)
{
	Cell top_left = center - Cell(MAX_TERRAIN_HEIGHT + 2, MAX_TERRAIN_HEIGHT + 2);
	Cell bottom_right = center + Cell(MAX_TERRAIN_HEIGHT + 2, MAX_TERRAIN_HEIGHT + 2);

	top_left.X = std::max<int>(0, top_left.X);
	top_left.Y = std::max<int>(0, top_left.Y);

	bottom_right.X = std::min<int>(Map.MapRect.Width - Map.MapRect.X, bottom_right.X);
	bottom_right.Y = std::min<int>(Map.MapRect.Height - Map.MapRect.Y, bottom_right.Y);

	Rect bound_rect(0, 0, 0, 0);

	Cell least_cell(32767, 32767);
	Cell most_cell(0, 0);

	DynamicVectorClass<ObjectClass *> objects;

	for (int y = top_left.Y; y <= bottom_right.Y; y++) {
		for (int x = top_left.X; x <= bottom_right.X; x++) {

			CellClass * cellptr = &Map[Cell(x + DeformPointXAdd, y + DeformPointYAdd)];

			if (!Can_Deform_Cell(cellptr, forced)) continue;

			DeformPointStruct & deform_point1 = DeformPoints[x + y * DeformPointWidth];
			DeformPointStruct & deform_point2 = DeformPoints[x + 1 + y * DeformPointWidth];
			DeformPointStruct & deform_point3 = DeformPoints[x + 1 + (y + 1) * DeformPointWidth];
			DeformPointStruct & deform_point4 = DeformPoints[x + (y + 1) * DeformPointWidth];

			if (deform_point1.Done || deform_point2.Done || deform_point3.Done || deform_point4.Done) {

				int desired_corners[NUM_CORNERS];
				desired_corners[0] = deform_point1.Height;
				desired_corners[1] = deform_point2.Height;
				desired_corners[2] = deform_point3.Height;
				desired_corners[3] = deform_point4.Height;

				int min_height = 1000;
				int max_height = 0;

				for (int i = 0; i < NUM_CORNERS; i++) {
					if (desired_corners[i] < min_height) min_height = desired_corners[i];
					if (desired_corners[i] > max_height) max_height = desired_corners[i];
				}

				if (max_height - min_height > RAMP_HALF_HEIGHT) continue;

				objects.Clear();
				ObjectClass * obj = cellptr->Cell_Occupier();
				while (obj) {
					objects.Add(obj);
					obj = obj->Next;
				}
				int objindex;
				for (objindex = 0; objindex < objects.Count(); objindex++) {
					objects[objindex]->Mark(MARK_UP);
				}

				Rect cell_bound(0, 0, 0, 0);

				cell_bound = cellptr->Cell_Render_Rect();
				int oldheight = cellptr->Height;
				cellptr->Height = (char)(min_height / RAMP_HALF_HEIGHT);

				if (oldheight != cellptr->Height) {
					cellptr->Remove_Fogged_Objects();
				}
				cellptr->Recalc_Attributes();

				if (cellptr->Smudge != SMUDGE_NONE) {
					SmudgeTypeClass * smudge_type = SmudgeTypes[cellptr->Smudge];
					Cell smudge_cell = cellptr->CellID;
					smudge_cell -= Cell(cellptr->SmudgeData % smudge_type->Width, cellptr->SmudgeData / smudge_type->Width);
					for (int iindex = 0; iindex < smudge_type->Height; iindex++) {
						for (int jindex = 0; jindex < smudge_type->Width; jindex++) {
							Map[smudge_cell + Cell(jindex, iindex)].Smudge = SMUDGE_NONE;
						}
					}
				}

				Cell cur_cell(cellptr->Fetch_CellID());
				Map.Flag_Background_Update(cur_cell);

				if (cur_cell.X < least_cell.X) least_cell.X = cur_cell.X;
				if (cur_cell.X > most_cell.X) most_cell.X = cur_cell.X;
				if (cur_cell.Y < least_cell.Y) least_cell.Y = cur_cell.Y;
				if (cur_cell.Y > most_cell.Y) most_cell.Y = cur_cell.Y;

				for (int j = 0; j < NUM_CORNERS; j++) {
					desired_corners[j] -= min_height;
				}

				/// BUG? there's 21 ramps
				for (int ramp = 0; ramp < RAMP_COUNT - 2; ramp++) {
					int * ramptype = _ramp_corners[ramp];
					if (
						ramptype[0] == desired_corners[0] &&
						ramptype[1] == desired_corners[1] &&
						ramptype[2] == desired_corners[2] &&
						ramptype[3] == desired_corners[3]
					) {
						cellptr->Ramp = (char)ramp;
						if (ramp == RAMP_NONE) {
							cellptr->ITType = TILE_CLEAR;
						} else {
							cellptr->ITType = IsometricTileType(ramp - 1 + IsometricTileTypeClass::RampStart);
						}
						cellptr->Recalc_Attributes();
						Cell id(cellptr->Fetch_CellID());
						Map.Flag_Background_Update(id);

						cell_bound = Union(cell_bound, cellptr->Cell_Render_Rect());
						if (id.X < least_cell.X) least_cell.X = id.X;
						if (id.X > most_cell.X) most_cell.X = id.X;
						if (id.Y < least_cell.Y) least_cell.Y = id.Y;
						if (id.Y > most_cell.Y) most_cell.Y = id.Y;
						break;
					}
				}
				bound_rect = Union(bound_rect, cell_bound);

				for (objindex = 0; objindex < objects.Count(); objindex++) {
					objects[objindex]->Mark(MARK_DOWN);
					objects[objindex]->HeightAGL = 0;
				}
			}
		}
	}

	delete[] DeformPoints;
	DeformPoints = NULL;

	bound_rect.Y -= TacticalRect.Y;

	TacticalMap->Register_Dirty_Area(bound_rect);

	for (int i = least_cell.Y - 1; i <= most_cell.Y + 1; i++) {
		for (int j = least_cell.X - 1; j <= most_cell.X + 1; j++) {
			Cell cell(j, i);
			CellClass * cptr = &Map[cell];
			Rect lat_rect = cptr->Cell_Render_Rect();

			if (cptr->Fixup_LAT()) {
				lat_rect = Union(lat_rect, cptr->Cell_Render_Rect());
				lat_rect = lat_rect - TacticalRect.Top_Left();
				if (lat_rect.Is_Valid()) {
					TacticalMap->Register_Dirty_Area(lat_rect);
				}
			}
		}
	}

	return(true);
}


/// <summary>
/// Writes the smoothed points back to the map for the generator.
/// This routine settles every cell of the map onto the lowest of its four smoothing points and
/// gives it the ramp tile that matches the slope those points describe. Cells the generator is
/// not allowed to reshape keep what they have.
/// </summary>
/// <remarks>The smoothing point array is released here, so the generator must prepare it again
/// before the next round of height changes.</remarks>
bool Commit_Deform_Grid_RMG(void)
{
	Map.Reset_Iterator();
	CellClass * cellptr = NULL;

	for (;;) {

		cellptr = Map.Iterate();
		if (!cellptr) {
			break;
		}

		int x = cellptr->Fetch_CellID().X - DeformPointXAdd;
		int y = cellptr->Fetch_CellID().Y - DeformPointYAdd;

		DeformPointStruct & deform_point1 = DeformPoints[x + y * DeformPointWidth];
		DeformPointStruct & deform_point2 = DeformPoints[x + 1 + y * DeformPointWidth];
		DeformPointStruct & deform_point3 = DeformPoints[x + 1 + (y + 1) * DeformPointWidth];
		DeformPointStruct & deform_point4 = DeformPoints[x + (y + 1) * DeformPointWidth];

		if (deform_point1.Done || deform_point2.Done || deform_point3.Done || deform_point4.Done) {

			int desired_corners[NUM_CORNERS];
			desired_corners[0] = deform_point1.Height;
			desired_corners[1] = deform_point2.Height;
			desired_corners[2] = deform_point3.Height;
			desired_corners[3] = deform_point4.Height;

			int min_height = 1000;
			int max_height = 0;

			for (int i = 0; i < NUM_CORNERS; i++) {
				if (desired_corners[i] < min_height) min_height = desired_corners[i];
				if (desired_corners[i] > max_height) max_height = desired_corners[i];
			}

			if (max_height - min_height > RAMP_HALF_HEIGHT) continue;

			if (Can_Deform_Cell_RMG(cellptr, true)) {
				cellptr->Height = (char)(min_height / RAMP_HALF_HEIGHT);
				Cell cur_cell(cellptr->Fetch_CellID());
			} else {
				continue;
			}

			for (int j = 0; j < NUM_CORNERS; j++) {
				desired_corners[j] -= min_height;
			}

			bool found_matching_ramp = false;

			/// BUG? there's 21 ramps
			for (int ramp = 0; ramp < RAMP_COUNT - 2; ramp++) {
				int * ramptype = _ramp_corners[ramp];
				if (
					ramptype[0] == desired_corners[0] &&
					ramptype[1] == desired_corners[1] &&
					ramptype[2] == desired_corners[2] &&
					ramptype[3] == desired_corners[3]
				) {

					cellptr->Ramp = (char)ramp;
					if (ramp == RAMP_NONE) {
						cellptr->ITType = TILE_CLEAR;
					} else {
						cellptr->ITType = IsometricTileType(ramp - 1 + IsometricTileTypeClass::RampStart);
					}
					found_matching_ramp = true;
					break;
				}
			}
		}
	}

	delete[] DeformPoints;
	DeformPoints = NULL;

	return(true);
}


/// <summary>
/// Ripples a height change out from a smoothing point.
/// This routine pulls the points surrounding the start point into line, so that none of them
/// differs from it by more than half a height step, and then carries the change on outward
/// from every point it had to move. A point that is pinned in place and cannot conform stops
/// the operation, unless the caller is forcing the change through.
/// </summary>
/// <param name="start_point">The smoothing point the change radiates out from.</param>
/// <param name="general_direction">The direction of the height change; positive to raise.</param>
/// <param name="forced">Should a refusal to move be ignored?</param>
/// <returns>bool; Could the surrounding terrain take the change?</returns>
bool Ripple_Deform_Points(Point2D start_point, int general_direction, bool forced)
{
	DeformPointStruct & start_deform_point = DeformPoints[start_point.X + start_point.Y * DeformPointWidth];
	int startheight = start_deform_point.Height;

	for (int y = -1; y < 2; y++) {
		for (int x = -1; x < 2; x++) {

			if (!(x || y)) continue;

			Point2D test_point = Point2D(x, y) + start_point;

			if (test_point.X < 0 || test_point.Y < 0 || test_point.X >= DeformPointWidth || test_point.Y >= DeformPointHeight) {
				if (forced) continue;
				return(false);
			}

			DeformPointStruct & new_point = DeformPoints[test_point.X + test_point.Y * DeformPointWidth];

			if (!new_point.Rigid || (forced && new_point.CanForce)) {

				if (abs(new_point.Height - startheight) > RAMP_HALF_HEIGHT) {
					if (general_direction == 1) {
						if (new_point.Height < startheight) {
							Record_Deform_Undo(test_point, new_point.Height);
							new_point.Height = startheight - RAMP_HALF_HEIGHT;
							new_point.Done = true;
						}
					} else {
						if (new_point.Height > startheight) {
							Record_Deform_Undo(test_point, new_point.Height);
							new_point.Height = startheight + RAMP_HALF_HEIGHT;
							new_point.Done = true;
						}
					}

					if (!new_point.Rigid) {
						if (!Ripple_Deform_Points(test_point, general_direction, forced)) {
							if (!forced) {
								return(false);
							}
						}
					}
				}
			} else {
				if (abs(new_point.Height - startheight) > RAMP_HALF_HEIGHT) {
					return(false);
				}
			}
		}
	}
	return(true);
}


/// <summary>
/// Raises or lowers a cell and re-smooths the terrain around it.
/// This is the routine the terrain editing logic calls to change the height of a cell. The
/// requested corners are nudged by half a height step, the change is rippled out through the
/// neighboring points, and the result is written back to the map as new cell heights and ramp
/// tiles. If the surrounding terrain will not accept the change and it is not being forced,
/// the map is left alone.
/// </summary>
/// <param name="original_cell">The cell to raise or lower.</param>
/// <param name="general_direction">The direction of the height change; positive to raise.</param>
/// <param name="forced">Should a refusal to move be ignored?</param>
/// <param name="point_mask">Mask of the cell corner points to adjust.</param>
/// <returns>bool; Was the terrain changed?</returns>
bool Deform_Cell(Cell original_cell, int general_direction, bool forced, int point_mask)
{
	UndoRecords.Clear();

	Init_Deform_Grid(original_cell, forced);

	int new_point_mask = point_mask & 3;
	new_point_mask |= (point_mask & 4) ? 8 : 0;
	new_point_mask |= (point_mask & 8) ? 4 : 0;

	int x = original_cell.X - DeformPointXAdd;
	int y = original_cell.Y - DeformPointYAdd;
	int point_bits = 1;

	for (int yy = y; yy < y + 2; yy++) {
		for (int xx = x; xx < x + 2; xx++) {

			if (point_bits & new_point_mask) {
				DeformPointStruct & deform_point = DeformPoints[xx + yy * DeformPointWidth];
				if (!deform_point.Rigid || forced) {
					deform_point.Height += general_direction * RAMP_HALF_HEIGHT;
					if (deform_point.Height < 0 || deform_point.Height > MAX_TERRAIN_HEIGHT * RAMP_HALF_HEIGHT) deform_point.Height -= general_direction * RAMP_HALF_HEIGHT;
					deform_point.Done = true;
					if (!Ripple_Deform_Points(Point2D(xx, yy), general_direction, forced)) {
						if (!forced) {
							delete[] DeformPoints;
							DeformPoints = NULL;
							return(false);
						}
					}
				} else {
					if (!forced) {
						delete[] DeformPoints;
						DeformPoints = NULL;
						return(false);
					}
				}
			}

			point_bits = point_bits << 1;
		}
	}

	return(Commit_Deform_Grid(original_cell, forced));
}


/// <summary>
/// Raises or lowers the corner points of a cell for the map generator.
/// This routine nudges the requested corners of the cell by half a height step and then lets
/// the smoothing pass ripple that change out through the surrounding points. Unless the change
/// is being forced, a corner that refuses to move aborts the whole operation and every point
/// touched is put back the way it was found.
/// </summary>
/// <param name="original_cell">The cell whose corners are to be adjusted.</param>
/// <param name="general_direction">The direction of the height change; positive to raise.</param>
/// <param name="forced">Should a refusal to move be ignored?</param>
/// <param name="point_mask">Mask of the cell corner points to adjust.</param>
/// <returns>bool; Was the adjustment made?</returns>
/// <remarks>The generator must prepare the smoothing point array before calling this routine,
/// and write the points back once it has finished with them.</remarks>
bool Deform_Cell_RMG(Cell original_cell, int general_direction, bool forced, int point_mask)
{
	UndoRecords.Clear();

	int new_point_mask = point_mask & 3;
	new_point_mask |= (point_mask & 4) ? 8 : 0;
	new_point_mask |= (point_mask & 8) ? 4 : 0;

	int x = original_cell.X - DeformPointXAdd;
	int y = original_cell.Y - DeformPointYAdd;
	int point_bits = 1;

	for (int yy = y; yy < y + 2; yy++) {
		for (int xx = x; xx < x + 2; xx++) {

			if (point_bits & new_point_mask) {
				DeformPointStruct & deform_point = DeformPoints[xx + yy * DeformPointWidth];
				if (!deform_point.Rigid) {
					deform_point.Height += general_direction * RAMP_HALF_HEIGHT;
					if (deform_point.Height < 0 || deform_point.Height > MAX_TERRAIN_HEIGHT * RAMP_HALF_HEIGHT) {
						deform_point.Height -= general_direction * RAMP_HALF_HEIGHT;
					} else {
						Record_Deform_Undo(Point2D(xx, yy), deform_point.Height - general_direction * RAMP_HALF_HEIGHT);
					}
					deform_point.Done = true;
					if (!Ripple_Deform_Points(Point2D(xx, yy), general_direction, forced)) {
						if (!forced) {
							Undo_Deform();
							return(false);
						}
					}
				} else {
					if (!forced) {
						Undo_Deform();
						return(false);
					}
				}
			}
			point_bits = point_bits << 1;
		}
	}
	return(true);
}


/// <summary>
/// Fetches the height the cell would settle to.
/// This routine reports the lowest of the cell's four smoothing points, converted back into
/// terrain height units. A cell that the smoothing system is not allowed to reshape reports
/// the height it already has.
/// </summary>
/// <param name="cellptr">Pointer to the cell to examine.</param>
/// <returns>Returns with the terrain height the cell would take on.</returns>
/// <remarks>The smoothing point array must be prepared before calling this routine.</remarks>
int Get_Deformed_Cell_Height(CellClass * cellptr)
{
	int x = cellptr->Fetch_CellID().X - DeformPointXAdd;
	int y = cellptr->Fetch_CellID().Y - DeformPointYAdd;

	DeformPointStruct & deform_point1 = DeformPoints[x + y * DeformPointWidth];
	DeformPointStruct & deform_point2 = DeformPoints[x + 1 + y * DeformPointWidth];
	DeformPointStruct & deform_point3 = DeformPoints[x + 1 + (y + 1) * DeformPointWidth];
	DeformPointStruct & deform_point4 = DeformPoints[x + (y + 1) * DeformPointWidth];

	int desired_corners[NUM_CORNERS];
	desired_corners[0] = deform_point1.Height;
	desired_corners[1] = deform_point2.Height;
	desired_corners[2] = deform_point3.Height;
	desired_corners[3] = deform_point4.Height;

	int min_height = 1000;

	for (int i = 0; i < NUM_CORNERS; i++) {
		if (desired_corners[i] < min_height) min_height = desired_corners[i];
	}

	if (Can_Deform_Cell_RMG(cellptr, true)) {
		return(min_height / RAMP_HALF_HEIGHT);
	} else {
		return(cellptr->Height);
	}
}


/// <summary>
/// Fetches the mask of cell corner points that may be moved.
/// This routine is used by the terrain height adjustment logic to decide which of a cell's
/// four corners should take part in a raise or lower operation. Corners that are pinned in
/// place, and corners already sitting at the extreme the caller is heading toward, are left
/// out of the mask.
/// </summary>
/// <param name="cellptr">Pointer to the cell to examine.</param>
/// <param name="direction">The direction of the height change; positive to raise.</param>
/// <returns>Returns with the four bit mask of corner points that may be moved.</returns>
int Get_Deformable_Cell_Corners(CellClass * cellptr, int direction)
{
	int i;
	int x = cellptr->Fetch_CellID().X - DeformPointXAdd;
	int y = cellptr->Fetch_CellID().Y - DeformPointYAdd;

	DeformPointStruct cells[NUM_CORNERS] = {
		DeformPoints[x + y * DeformPointWidth],
		DeformPoints[x + 1 + y * DeformPointWidth],
		DeformPoints[x + 1 + (y + 1) * DeformPointWidth],
		DeformPoints[x + (y + 1) * DeformPointWidth]
	};

	int desired_corners[NUM_CORNERS];
	desired_corners[0] = cells[0].Height;
	desired_corners[1] = cells[1].Height;
	desired_corners[2] = cells[2].Height;
	desired_corners[3] = cells[3].Height;
	int min_height = 1000;
	int max_height = -1;
	int minindex = -1;
	int maxindex = -1;

	for (i = 0; i < NUM_CORNERS; i++) {
		if (desired_corners[i] < min_height) {
			minindex = i;
			min_height = desired_corners[i];
		}
		if (desired_corners[i] > max_height) {
			maxindex = i;
			max_height = desired_corners[i];
		}
	}

	int mask = 0;
	if (max_height == min_height) {
		for (i = 0; i < NUM_CORNERS; i++) {
			if (!cells[i].Rigid) {
				mask += (1 << i);
			}
		}
	} else if (direction > 0) {
		for (i = 0; i < NUM_CORNERS; i++) {
			if (desired_corners[i] < max_height && !cells[i].Rigid) {
				mask += (1 << i);
			}
		}
	} else {
		for (i = 0; i < NUM_CORNERS; i++) {
			if (desired_corners[i] > min_height && !cells[i].Rigid) {
				mask += (1 << i);
			}
		}
	}

	return(mask);
}
