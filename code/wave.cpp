/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#define INCLUDE_COM
#include "always.h"

#include "wave.h"

#include "_logic.h"
#include "_map.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_zbuffer.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "dsurface.h"
#include "globals.h"
#include "goptions.h"
#include "logic.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "sun.h"
#include "tactical.h"
#include "techno.h"
#include "tracker.h"
#include "vector.h"
#include "warhead.h"
#include "weapon.h"
#include "zbuffer.h"

#include <algorithm>


FacingType Facing_Between_Points(Point2D const & pt1, Point2D const & pt2);

enum {
	_RADIUS_TABLE_SIZE = 300,
	_SINE_TABLE_SIZE = 500,
	_INTENSITY_TABLE_SIZE = 13,
	_INTENSITY_TABLE_OFFSET = 110,
};

unsigned short RadiusTable[_RADIUS_TABLE_SIZE][_RADIUS_TABLE_SIZE];
short SineTable[_SINE_TABLE_SIZE];
int IntensityTable[_INTENSITY_TABLE_SIZE];

// How many whole strides the distortion lifts its replacement from, by ripple
// amplitude, and the screen-space step one stride takes in each facing.
int const _stride_counts[_INTENSITY_TABLE_SIZE] = {0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3};
int const _facing_row_steps[FACING_COUNT] = {-1, -1, 0, 1, 1, 1, 0, -1};
int const _facing_col_steps[FACING_COUNT] = {0, 1, 1, 1, 0, -1, -1, -1};

bool _tables_calculated = false;

const float WaveStep = 0.05f;


/// <summary>
/// Creates a wave between the firing object and its target.
/// The geometry is built, the wave is placed upon the map, and it is added to the global
/// wave list so that it will be processed and drawn. A sonic wave grows out from the firer,
/// while a laser wave springs up at its full length straight away.
/// </summary>
/// <param name="source">The object that is firing the wave.</param>
/// <param name="target">The object that the wave is aimed at.</param>
WaveClass::WaveClass(Coord const & source_coord, Coord const & target_coord, TechnoClass * source, WaveType type, TechnoClass * target) :
	BASECLASS(),
	Target(target),
	Type(type),
	IsWaveActive(true),
	WaveEC(100),
	WaveProgress(0),
	FadeProgress(0),
	LaserEC(0),
	Source(source),
	AffectedCells(5)
{
	Create_ID();
	Waves.Add(this);

	if (!_tables_calculated) {
		Init_Statics();
	}

	Build_Wave_Shape(source_coord, target_coord);
	DrawData.Points = NULL;
	Direction = Facing_Between_Points(WaveStartMiddle, WaveEndRight);
	Unlimbo(StartCoord);

	switch (Type) {
		case WAVE_SONIC:
			break;

		case WAVE_BIG_LASER:
		case WAVE_LASER:
			WaveProgress = 1.0;
			LaserEC = 160;
			WaveShape.Vertices[PolygonShapeStruct::END_LEFT] = WaveEndLeft;
			WaveShape.Vertices[PolygonShapeStruct::END_MIDDLE] = WaveEndMiddle;
			WaveShape.Vertices[PolygonShapeStruct::END_RIGHT] = WaveEndRight;
			break;
	}

	Wave_Shape_AI();
}


/// <summary>
/// Constructs a bare wave object.
/// This constructor is used by the save game loader, which fills the object in from the
/// stream afterwards. No geometry is built here.
/// </summary>
WaveClass::WaveClass(void) :
	BASECLASS(),
	Target(NULL),
	IsWaveActive(true),
	WaveEC(100),
	WaveProgress(0),
	FadeProgress(0),
	LaserEC(0),
	Source(NULL),
	AffectedCells(5)
{
	Waves.Add(this);

	if (!_tables_calculated) {
		Init_Statics();
	}
}


/// <summary>
/// Removes the wave from the game.
/// </summary>
WaveClass::~WaveClass(void)
{
	Detach_This_From_All(this, true);
	Waves.Delete(this);
	AffectedCells.Clear();
}


/// <summary>
/// Distorts one screen pixel for the sonic wave.
/// The pixel is replaced by one lifted from a short way along the wave's line of travel and
/// is tinted toward green and blue. This is what makes the terrain appear to ripple. How
/// far off the sample is taken depends upon where the pixel sits within the wave.
/// </summary>
/// <param name="x">The screen column of the pixel.</param>
/// <param name="xoff">The horizontal offset of the wave upon the screen.</param>
/// <param name="y">The row of the pixel within the wave.</param>
/// <param name="yscreen">The screen row of the pixel.</param>
/// <param name="buffer">Pointer to the screen pixel to distort.</param>
/// <param name="cliprect">The clipping rectangle the replacement may be lifted from.</param>
void WaveClass::Set_Sonic_Pixel(int x, int xoff, int y, int yscreen, unsigned short * buffer, Rect const & cliprect) const
{
	int radius = RadiusTable[y][abs(x - WaveStartMiddle.X - xoff)];
	int amplitude = abs(SineTable[WaveEC + radius]);

	// A replacement lifted from outside the view would carry interface pixels or memory
	// past the frame into the wave, so such a pixel keeps its own color and only tints.
	int count = _stride_counts[amplitude];
	int srow = yscreen + count * _facing_row_steps[Direction];
	int scol = x + count * _facing_col_steps[Direction];

	unsigned short color = *buffer;
	if (cliprect.Is_Point_Within(Point2D(scol, srow))) {
		color = buffer[WaveIntensityTable[amplitude]];
	}
	int mult = IntensityTable[amplitude];

	RGBClass rgb = DSurface::Deconstruct_Hicolor_Pixel(color);
	int red = rgb.Get_Red();
	int green = rgb.Get_Green();
	int blue = rgb.Get_Blue();
	green += ((green * mult) >> 8);
	blue += ((blue * mult) >> 8);

	*buffer = DSurface::Build_Hicolor_Pixel(red, std::min(255, green), std::min(255, blue));
}


/// <summary>
/// Brightens one screen pixel for the laser wave.
/// Only the red component of the pixel is raised, which is what gives the laser its color
/// over whatever terrain happens to lie underneath it.
/// </summary>
/// <param name="buffer">Pointer to the screen pixel to brighten.</param>
/// <param name="mult">The strength of the brightening. The wave fades this out
/// over its life.</param>
void WaveClass::Set_Laser_Pixel(unsigned short * buffer, int mult) const
{
	RGBClass rgb = DSurface::Deconstruct_Hicolor_Pixel(*buffer);
	int red = rgb.Get_Red();
	int green = rgb.Get_Green();
	int blue = rgb.Get_Blue();
	red += ((red * mult) >> 8);
	*buffer = DSurface::Build_Hicolor_Pixel(std::min(255, red), green, blue);
}


/// <summary>
/// Builds the lookup tables that every sonic wave shares.
/// The radius, sine and intensity tables are the same for all waves, so the first wave to
/// come along builds them and every wave after that makes free with them.
/// </summary>
void WaveClass::Init_Statics(void)
{
	const double _sine_duration = 0.125;
	const double _sine_amplitude = 12;
	const double _offset = 0.49;

	_tables_calculated = true;

	for (int x = 0; x < _RADIUS_TABLE_SIZE; x++) {
		for (int y = 0; y < _RADIUS_TABLE_SIZE; y++) {
			RadiusTable[x][y] = std::sqrt(x * x + y * y);
		}
	}

	int i;
	for (i = 0; i < _SINE_TABLE_SIZE; i++) {
		SineTable[i] = (short)(std::sin(i * _sine_duration) * _sine_amplitude + _offset);
	}

	for (i = 0; i < _INTENSITY_TABLE_SIZE; i++) {
		IntensityTable[i] = _INTENSITY_TABLE_OFFSET + (i * 8);
	}
}


/// <summary>
/// Builds the pixel offset tables that the sonic wave is drawn with.
/// The distortion lifts its pixels from a short way along the wave's line of travel, so the
/// offsets depend both upon the pitch of the surface being drawn to and upon the direction
/// the wave is heading in.
/// </summary>
/// <remarks>The drawing routine reads these tables, so build them first.</remarks>
void WaveClass::Init_Offset_Tables(void)
{
	int stride = LogicalSurface->Stride() >> 1;

	for (int facing = 0; facing < FACING_COUNT; facing++) {
		DirectionStrides[facing] = _facing_row_steps[facing] * stride + _facing_col_steps[facing];
	}

	for (int i = 0; i < _INTENSITY_TABLE_SIZE; i++) {
		WaveIntensityTable[i] = _stride_counts[i] * DirectionStrides[Direction];
	}
}


/// <summary>
/// Determines the facing from one screen point toward another.
/// This routine works in screen space, so the facing it returns is the direction the wave
/// appears to travel in rather than the direction it travels upon the map.
/// </summary>
/// <returns>Returns with the nearest of the eight facings.</returns>
FacingType Facing_Between_Points(Point2D const & pt1, Point2D const & pt2)
{
	int xdiff = pt2.X - pt1.X;
	int ydiff = pt1.Y - pt2.Y;

	double tangent = std::sin(DEG_TO_RAD(22.5)) / std::cos(DEG_TO_RAD(22.5));

	if (xdiff != 0) {
		double slope = (double)ydiff / (double)xdiff;

		if (slope < 1.0 / tangent && slope >= tangent) {
			return((xdiff > 0) ? FACING_NE : FACING_SW);
		}

		if (slope < tangent && slope >= -tangent) {
			return((xdiff > 0) ? FACING_E : FACING_W);
		}

		if (slope < -tangent && slope >= -1.0 / tangent) {
			return((xdiff > 0) ? FACING_SE : FACING_NW);
		}
	}
	return((ydiff > 0) ? FACING_N : FACING_S);
}


/// <summary>
/// Inflicts the wave's damage upon one cell.
/// Everything standing in the cell takes the firing weapon's ambient damage, walls are worn
/// down, chain reactive overlay is set off, and a destroyable cliff may be brought down.
/// The object that fired the wave is never harmed by its own weapon.
/// </summary>
/// <param name="coord">The coordinate of the cell to damage.</param>
void WaveClass::Sonic_Damage(Coord const & coord)
{
	if (Source != NULL) {
		CellClass *cptr = &Map[coord.As_Cell()];

		WeaponTypeClass * weapon = Source->PrimaryWeapon;
		WarheadTypeClass const * warhead = weapon->WarheadPtr;

		int damage = weapon->AmbientDamage;

		bool isbridge = (cptr->IsUnderBridge && StartCoord.Z >= LEVEL_LEPTON_H * (cptr->Height + BRIDGE_CELL_HEIGHT));

		ObjectClass * obj = cptr->Cell_Occupier(isbridge);
		while (obj != NULL) {
			if (obj != Source) {
				if (obj->IsActive && obj->IsDown && !obj->IsInLimbo && obj->Strength > 0) {
					obj->Take_Damage(damage, 0, warhead, Source);
				}
			}
			obj = obj->Next;
		}

		if (cptr->Overlay != OVERLAY_NONE) {
			OverlayTypeClass *optr = OverlayTypes[cptr->Overlay];
			if (optr->IsChainReaction) {
				Cell cell = cptr->CellID;
				Chain_Reaction_Damage(cell);
			}
			if (optr->IsWall) {
				cptr->Reduce_Wall(weapon->AmbientDamage);
			}
		}

		if (cptr->Is_Tile_Destroyable_Cliff()) {
			int chance = Rule->CollapseChance;
			if (Random_Pick(0, 99) < chance) {
				Map.Collapse_Cliff(cptr);
			}
		}
	}
}


/// <summary>
/// Adds a cell to the list of cells this wave is affecting.
/// A cell is only recorded once, however many times the wave sweeps across it.
/// </summary>
void WaveClass::Sonic_Add_Cell(Cell const & cell)
{
	CellClass * cptr = &Map[cell];
	if (AffectedCells.ID(cptr) == -1) {
		AffectedCells.Add(cptr);
	}
}


/// <summary>
/// Interpolates between two coordinates.
/// </summary>
/// <param name="t">The fraction of the way from the first coordinate to the second.</param>
/// <returns>Returns with the coordinate that lies that far along the line between them.</returns>
Coord Lerp(Coord const & a, Coord const & b, float t)
{
	// std::lerp returns the second endpoint exactly when t is one, which the
	// a*(1-t) + b*t form does not.
	return(Coord(int(std::lerp((double)a.X, (double)b.X, (double)t)),
				 int(std::lerp((double)a.Y, (double)b.Y, (double)t)),
				 int(std::lerp((double)a.Z, (double)b.Z, (double)t))));
}


/// <summary>
/// Interpolates between two screen points.
/// </summary>
/// <param name="t">The fraction of the way from the first point to the second.</param>
/// <returns>Returns with the point that lies that far along the line between them.</returns>
Point2D Lerp(Point2D const & a, Point2D const & b, float t)
{
	int x = int(std::lerp((double)a.X, (double)b.X, (double)t));
	int y = int(std::lerp((double)a.Y, (double)b.Y, (double)t));
	return(Point2D(x,y));
}


/// <summary>
/// Removes the specified object from this wave's records.
/// This routine is called when an object is about to leave the game, so that the wave is
/// not left pointing at it.
/// </summary>
/// <param name="target">The object that is going away.</param>
void WaveClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (Source == target) {
		Source = NULL;
	}
	if (Target == target) {
		Target = NULL;
	}
}


/// <summary>
/// Lists the members this wave carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void WaveClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Target);
	stream.Serialize(Type);
	stream.Serialize(StartCoord);
	stream.Serialize(EndCoord);
	stream.Serialize(WaveStartMiddle);
	stream.Serialize(WaveEndMiddle);
	stream.Serialize(WaveEndLeft);
	stream.Serialize(WaveEndRight);
	stream.Serialize(WaveStartLeft);
	stream.Serialize(WaveStartRight);
	stream.Serialize(WaveEndLeftCoord);
	stream.Serialize(WaveEndRightCoord);
	stream.Serialize(WaveStartLeftCoord);
	stream.Serialize(WaveStartRightCoord);
	stream.Serialize(IsWaveActive);
	stream.Serialize(WaveEC);
	stream.Serialize(WaveProgress);
	stream.Serialize(FadeProgress);

	/*
	 * The outline of the wave is drawn from the active corners below, so only the number
	 * of them travels; Post_Load points the shape back at them.
	 */
	stream.Serialize(WaveShape.Count);

	stream.Serialize(ActiveWaveStartMiddle);
	stream.Serialize(ActiveWaveEndMiddle);
	stream.Serialize(ActiveWaveEndLeft);
	stream.Serialize(ActiveWaveEndRight);
	stream.Serialize(ActiveWaveStartLeft);
	stream.Serialize(ActiveWaveStartRight);
	// DrawData -- the pixel offsets the wave is swept with, rebuilt before every draw.
	// DirectionStrides
	stream.Serialize(Direction);
	stream.Serialize(LaserEC);
	stream.Serialize(Source);
	stream.Serialize(Facing);
	stream.Serialize(AffectedCells);
	// WaveIntensityTable -- the rasterized wave, which belongs to the surface being drawn to and
	// is rebuilt before every draw.
	// WaveShape.Vertices
}


/// <summary>
/// Points the wave outline back at the corners it is built from.
/// </summary>
void WaveClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	WaveShape.Vertices = &ActiveWaveStartMiddle;
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is part of the persistence interface. The save game loader uses the
/// identifier to recreate the object as the right kind of class.
/// </summary>
/// <param name="retval">Pointer to the buffer to store the class identifier in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no buffer was supplied.</returns>
HRESULT STDMETHODCALLTYPE WaveClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_WaveClass;
	return(S_OK);
}


/// <summary>
/// Determines which display layer the wave belongs to.
/// </summary>
/// <returns>Returns with the air layer, so that the wave draws over ground objects.</returns>
LayerType WaveClass::In_Which_Layer(void) const
{
	return(LAYER_AIR);
}


/// <summary>
/// Fetches the object type class that this object was built from.
/// </summary>
/// <returns>Returns with NULL, since a wave has no type class behind it.</returns>
ObjectTypeClass const * WaveClass::Class_Of(void) const
{
	return(NULL);
}


/// <summary>
/// Places the wave into the game world.
/// This routine puts the wave onto the map and submits it to the display and logic layers
/// so that it will be drawn and processed from now on.
/// </summary>
/// <param name="coord">The coordinate to place the wave at.</param>
/// <returns>bool; Was the wave successfully placed?</returns>
bool WaveClass::Unlimbo(Coord const & coord, Dir256 facing)
{
	assert(this != NULL);

	if (GameActive && IsInLimbo && !IsDown) {
		if (ScenarioInit || Can_Enter_Cell(&Map[coord], FACING_NONE, coord.Z) == MOVE_OK) {
			IsInLimbo = false;
			IsToDisplay = false;
			PositionCoord = coord;

			if (Mark(MARK_DOWN)) {
				if (IsActive) {
					if (In_Which_Layer() != LAYER_NONE) {
						Map.Submit(this);
					}
					Logic.Submit(this);
				}
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Removes the wave from the game world.
/// This routine takes the wave back off the map and out of the display and logic layers so
/// that it is neither drawn nor processed any longer.
/// </summary>
/// <returns>bool; Was the wave removed?</returns>
bool WaveClass::Limbo(void)
{
	assert(this != NULL);

	if (GameActive && !IsInLimbo) {

		Detach_All();
		Mark(MARK_UP);

		Map.Remove(this);
		Logic.Remove(this);

		Hidden();
		IsInLimbo = true;
		IsToDisplay = false;
		return(true);
	}
	return(false);
}


/// <summary>
/// Draws the wave to the screen.
/// This routine is called by the display layer and dispatches to the drawing handler for
/// this wave's type. A wave that lies entirely under the fog of war is not drawn.
/// </summary>
/// <param name="point">The screen point that the wave's origin lands upon.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void WaveClass::Draw_It(Point2D const & point, Rect const & cliprect) const
{
	if (Scen->Special.IsFogOfWar && Map.Is_Fogged(StartCoord) && Map.Is_Fogged(EndCoord)) return;

	switch (Type) {
		case WAVE_SONIC:
			((WaveClass *)this)->Draw_Sonic(point, cliprect);
			break;
		case WAVE_BIG_LASER:
		case WAVE_LASER:
			((WaveClass *)this)->Draw_Laser(point, cliprect);
			break;
	}
}


/// <summary>
/// Draws the sonic wave.
/// The wave distorts the screen pixels it covers rather than replacing them, so that the
/// terrain appears to ripple as the wave passes over it. Pixels hidden behind nearer ground
/// or objects are left alone.
/// </summary>
/// <param name="point">The screen point that the wave's origin lands upon.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void WaveClass::Draw_Sonic(Point2D const & point, Rect const & cliprect)
{
	int zpix = Tactical::Z_Lepton_To_Pixel(StartCoord.Z);

	unsigned short * surfptr = (unsigned short *)LogicalSurface->Lock();
	if (surfptr != NULL) {

		// The walk may draw to the clip's very edge because a replacement pixel that
		// would come from outside the view is refused where it is sampled.
		int top = cliprect.Y;
		int bottom = cliprect.Y + cliprect.Height - 1;
		int left = cliprect.X;
		int right = cliprect.X + cliprect.Width - 1;

		int xoff = point.X - WaveStartMiddle.X;
		int yoff = point.Y - WaveStartMiddle.Y;

		Rasterize_Polygon(WaveShape, DrawData);

		if (DrawData.Points != NULL) {

			int starty = yoff + TacticalRect.Y + DrawData.BaseY;
			int endy = std::min(DrawData.Count + starty - 1, cliprect.Height + cliprect.Y);

			Init_Offset_Tables();

			if (Direction > FACING_NE && Direction < FACING_W) {
				unsigned short base_z = DepthBuffer->Get_Scroll_Delta(zpix);
				unsigned short zval = base_z - starty - 2;
				unsigned int zoffset = DepthBuffer->Get_Buffer_Offset(Point2D(0, starty - TacticalRect.Y));
				int width = LogicalSurface->Get_Width();
				int zwidth = DepthBuffer->Get_Buffer_Width();

				if (zoffset + 2 * (width + zwidth * (endy - starty + 1)) < DepthBuffer->Get_Buffer_End()) {

					int stride = LogicalSurface->Stride() >> 1;
					unsigned short * dest = surfptr + starty * stride;
					Point2D * points = DrawData.Points;

					for (int y = starty; y <= endy; y++) {
						if (y >= top && y <= bottom) {
							int xstop = xoff + points->Y;
							if (xstop >= right) {
								xstop = right;
							}
							int xstart = left;
							if (points->X + xoff > left) {
								xstart = points->X + xoff;
							}

							zoffset += 2 * xstart;
							dest += xstart;

							int x;
							int ypos = abs(y - yoff - WaveStartMiddle.Y);
							for (x = xstart; x <= xstop; x++) {
								if (*(unsigned short *)zoffset > zval) {
									Set_Sonic_Pixel(x, xoff, ypos, y, dest, cliprect);
								}
								zoffset += 2;
								dest++;
							}
							zoffset -= 2 * x;
							dest -= x;
						}
						dest += stride;
						zoffset += 2 * zwidth;
						zval--;
						points++;
					}

				} else {

					int index = 0;
					for (int y = starty; y <= endy; y++) {
						if (y >= top && y <= bottom) {
							Point2D * entry = &DrawData.Points[index];
							int xstop = xoff + entry->Y;
							if (xstop >= right) {
								xstop = right;
							}
							int xstart = left;
							if (entry->X + xoff > left) {
								xstart = entry->X + xoff;
							}

							int xpos = xstart;
							unsigned short * dest = surfptr + xstart + y * (LogicalSurface->Stride() >> 1);
							int zy = y - TacticalRect.Y;
							int ypos = abs(y - yoff - WaveStartMiddle.Y);
							for (int x = xstart; x <= xstop; x++) {
								if (*(unsigned short *)DepthBuffer->Get_Buffer_Offset(Point2D(xpos, zy)) > zval) {
									Set_Sonic_Pixel(x, xoff, ypos, y, dest, cliprect);
								}
								xpos++;
								dest++;
							}
						}
						zval--;
						index++;
					}
				}

			} else {

				unsigned int zoffset = DepthBuffer->Get_Buffer_Offset(Point2D(0, starty - TacticalRect.Y));
				int width = LogicalSurface->Get_Width();
				int rows = endy - starty;
				int zwidth = DepthBuffer->Get_Buffer_Width();

				if (zoffset + 2 * (width + zwidth * (rows + 1)) < DepthBuffer->Get_Buffer_End()) {

					int stride = LogicalSurface->Stride() >> 1;
					unsigned short * dest = surfptr + endy * stride;
					Point2D * points = &DrawData.Points[rows];
					unsigned short base_z = DepthBuffer->Get_Scroll_Delta(endy);
					unsigned short zval = base_z - zpix - 2;
					unsigned short * zptr = (unsigned short *)DepthBuffer->Get_Buffer_Offset(Point2D(0, endy - TacticalRect.Y));

					for (int y = endy; y >= starty; y--) {
						if (y >= top && y <= bottom) {
							int xstart = xoff + points->Y;
							if (xstart >= right) {
								xstart = right;
							}
							int xstop = xoff + points->X;
							if (xstop <= left) {
								xstop = left;
							}

							dest += xstart;
							zptr += xstart;

							int x;
							int ypos = abs(y - yoff - WaveStartMiddle.Y);
							for (x = xstart; x >= xstop; x--) {
								if (*zptr > zval) {
									Set_Sonic_Pixel(x, xoff, ypos, y, dest, cliprect);
								}
								zptr--;
								dest--;
							}
							zptr -= x;
							dest -= x;
						}
						dest -= stride;
						zval++;
						points--;
						zptr -= zwidth;
					}

				} else {

					unsigned short base_z = DepthBuffer->Get_Scroll_Delta(endy);
					unsigned short zval = base_z - zpix - 2;
					int index = rows;
					for (int y = endy; y >= starty; y--) {
						if (y >= top && y <= bottom) {
							Point2D * entry = &DrawData.Points[index];
							int xstart = entry->Y + xoff;
							if (xstart >= right) {
								xstart = right;
							}
							int xstop = xoff + entry->X;
							if (xstop <= left) {
								xstop = left;
							}

							unsigned short * dest = surfptr + xstart + y * (LogicalSurface->Stride() >> 1);
							int zy = y - TacticalRect.Y;
							int xpos = xstart;
							int ypos = abs(y - yoff - WaveStartMiddle.Y);
							for (int x = xstart; x >= xstop; x--) {
								if (*(unsigned short *)DepthBuffer->Get_Buffer_Offset(Point2D(xpos, zy)) > zval) {
									Set_Sonic_Pixel(x, xoff, ypos, y, dest, cliprect);
								}
								xpos--;
								dest--;
							}
						}
						zval++;
						index--;
					}
				}
			}
		}

		LogicalSurface->Unlock();

		if (DrawData.Points != NULL) {
			delete [] DrawData.Points;
			DrawData.Points = NULL;
		}
	}
}


/// <summary>
/// Draws the laser wave.
/// The wave brightens the screen pixels it covers rather than replacing them, so that the
/// beam reads as a glow lying over the terrain. Pixels hidden behind nearer ground or
/// objects are left alone. Nothing is drawn at all unless the player is running at the
/// highest detail level.
/// </summary>
/// <param name="point">The screen point that the wave's origin lands upon.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void WaveClass::Draw_Laser(Point2D const & point, Rect const & cliprect)
{
	if (Options.DetailLevel == 2) {
		int zpix = Tactical::Z_Lepton_To_Pixel(StartCoord.Z);

		unsigned short * buffer = (unsigned short *)LogicalSurface->Lock();
		if (buffer != NULL) {

			int top = cliprect.Y;
			int bottom = cliprect.Y + cliprect.Height - 1;
			int left = cliprect.X;
			int right = cliprect.X + cliprect.Width - 1;

			int xoff = point.X - WaveStartMiddle.X;
			int yoff = point.Y - WaveStartMiddle.Y + TacticalRect.Y;

			Rasterize_Polygon(WaveShape, DrawData);

			if (DrawData.Points != NULL) {

				int ystart = yoff + DrawData.BaseY;
				int yend = std::min(DrawData.Count + ystart - 1, bottom);

				unsigned short base_z = DepthBuffer->Get_Scroll_Delta(zpix);
				unsigned short depth = base_z - ystart - 2;
				unsigned int zoff = DepthBuffer->Get_Buffer_Offset(Point2D(0, ystart - TacticalRect.Y));
				int surfwidth = LogicalSurface->Get_Width();
				int zwidth = DepthBuffer->Get_Buffer_Width();

				if (zoff + 2 * (surfwidth + (yend - ystart + 1) * zwidth) < DepthBuffer->Get_Buffer_End()) {

					/*
					 * The wave lies entirely within the depth buffer, so the depth buffer
					 * can be walked with a running cursor.
					 */
					int stride = LogicalSurface->Stride() >> 1;
					unsigned short * scrptr = &buffer[ystart * stride];
					Point2D * points = DrawData.Points;

					for (int y = ystart; y <= yend; y++) {
						if (y >= top && y <= bottom) {

							int xstop = right;
							if (points->Y + xoff < right) {
								xstop = points->Y + xoff;
							}
							int xstart = points->X + xoff;
							if (xstart <= left) {
								xstart = left;
							}

							zoff += 2 * xstart;
							scrptr += xstart;

							int x = xstart;
							for (; x <= xstop; x++) {
								if (*(unsigned short *)zoff > depth) {
									Set_Laser_Pixel(scrptr, LaserEC);
								}
								zoff += 2;
								scrptr++;
							}

							zoff -= 2 * x;
							scrptr -= x;
						}
						scrptr += stride;
						zoff += 2 * zwidth;
						depth--;
						points++;
					}

				} else {

					/*
					 * The wave extends beyond the end of the depth buffer, so every pixel
					 * must fetch its (wrapped) depth buffer position the slow way.
					 */
					int i = 0;
					for (int y = ystart; y <= yend; y++) {
						if (y >= top && y <= bottom) {

							int xstop = DrawData.Points[i].Y + xoff;
							if (xstop >= right) {
								xstop = right;
							}
							int xstart = left;
							if (DrawData.Points[i].X + xoff > left) {
								xstart = DrawData.Points[i].X + xoff;
							}

							unsigned short * scrptr = &buffer[xstart + y * (LogicalSurface->Stride() >> 1)];
							int ypos = y - TacticalRect.Y;

							for (int x = xstart; x <= xstop; x++) {
								if (*(unsigned short *)DepthBuffer->Get_Buffer_Offset(Point2D(x, ypos)) > depth) {
									Set_Laser_Pixel(scrptr, LaserEC);
								}
								scrptr++;
							}
						}
						depth--;
						i++;
					}
				}
			}

			LogicalSurface->Unlock();

			if (DrawData.Points != NULL) {
				delete [] DrawData.Points;
				DrawData.Points = NULL;
			}
		}
	}
}


/// <summary>
/// Handles the per frame logic for the wave.
/// This routine is called by the logic layer and merely dispatches to the handler for this
/// wave's type.
/// </summary>
void WaveClass::AI(void)
{
	switch (Type) {
		case WAVE_SONIC:
			Sonic_AI();
			break;
		case WAVE_BIG_LASER:
		case WAVE_LASER:
			Laser_AI();
			break;
	}
}


/// <summary>
/// Handles the per frame logic for a sonic wave.
/// The wave geometry is brought up to date, every cell currently lying under the wave is
/// damaged, and the wave removes itself from the game once it has run its course.
/// </summary>
void WaveClass::Sonic_AI(void)
{
	Wave_Shape_AI();
	for (int i = 0; i < AffectedCells.Count(); i++) {
		Sonic_Damage(AffectedCells[i]->Cell_Coord());
	}
	WaveEC -= 1;
	if (WaveEC < 0) {
		Delete_Me();
	} else {
		BASECLASS::AI();
	}
}


/// <summary>
/// Handles the per frame logic for a laser wave.
/// A laser wave inflicts no damage of its own. It merely dims until it is too faint to be
/// worth drawing, at which point it removes itself from the game.
/// </summary>
void WaveClass::Laser_AI(void)
{
	LaserEC -= 6;
	if (LaserEC < 32) {
		Delete_Me();
	}
}


/// <summary>
/// Rebuilds the list of cells the wave covers.
/// This routine walks the length of the wave and collects the cells it passes over. The
/// sonic damage pass works from that list, so it is refreshed as the wave grows and fades.
/// </summary>
void WaveClass::Wave_Recalc_Affected_Cells(void)
{
	AffectedCells.Clear();

	if (IsWaveActive) {
		Cell start = StartCoord.As_Cell();

		for (double t = WaveStep; t <= ((WaveStep / 2.0) + WaveProgress); t += WaveStep) {
			Coord coord = Lerp(StartCoord, EndCoord, t);
			Cell cell = coord.As_Cell();
			if (start != cell) {
				start = cell;
				Sonic_Add_Cell(start);
			}
		}
	} else {
		Cell left = WaveEndLeftCoord.As_Cell();
		Cell right = WaveEndRightCoord.As_Cell();
		Cell middle = EndCoord.As_Cell();

		for (double t = FadeProgress; t <= ((WaveStep / 2.0) + WaveProgress); t += WaveStep) {
			Coord coord = COORD_NONE;

			if (Direction < FACING_S) {
				coord = Lerp(WaveStartLeftCoord, WaveEndLeftCoord, t);
				if (left != coord.As_Cell()) {
					left = coord.As_Cell();
					Sonic_Add_Cell(left);
				}
			}

			coord = Lerp(StartCoord, EndCoord, t);
			if (middle != coord.As_Cell()) {
				middle = coord.As_Cell();
				Sonic_Add_Cell(middle);
			}

			if (Direction >= FACING_S) {
				coord = Lerp(WaveStartRightCoord, WaveEndRightCoord, t);
				if (right != coord.As_Cell()) {
					right = coord.As_Cell();
					Sonic_Add_Cell(right);
				}
			}
		}
	}
}


static const float WaveLengthFactor[] = { 1.0f, 1.05f, 1.05f };


/// <summary>
/// Builds the geometry of the wave.
/// This routine spans the wave between the firing point and the target, sizing it from the
/// template for its wave type, rotating it to lie along the line of fire, and converting
/// each corner into the screen points that the drawing routines rasterize from.
/// </summary>
void WaveClass::Build_Wave_Shape(Coord const & source_coord, Coord const & target_coord)
{
	static Vector3 WaveSize[WAVE_COUNT][4] = {
		{
			Vector3(-30.0, -100.0, 0.0),	/// END_LEFT
			Vector3(-30.0, 100.0, 0.0),		/// END_RIGHT
			Vector3(30.0, -100.0, 0.0),		/// START_LEFT
			Vector3(30.0, 100.0, 0.0)		/// START_RIGHT
		},
		{
			Vector3(-34.0, -44.0, 0.0),		/// END_LEFT
			Vector3(-34.0, 44.0, 0.0),		/// END_RIGHT
			Vector3(34.0, -44.0, 0.0),		/// START_LEFT
			Vector3(34.0, 44.0, 0.0)		/// START_RIGHT
		},
		{
			Vector3(-27.0, -34.0, 0.0),		/// END_LEFT
			Vector3(-27.0, 34.0, 0.0),		/// END_RIGHT
			Vector3(27.0, -34.0, 0.0),		/// START_LEFT
			Vector3(27.0, 34.0, 0.0)		/// START_RIGHT
		}
	};

	Coord start_coord = source_coord;
	Coord end_coord = Lerp(start_coord, target_coord, WaveLengthFactor[Type]);
	if (Type == WAVE_SONIC) {
		end_coord.Z += 50;
	}

	start_coord = Lerp(target_coord, start_coord, WaveLengthFactor[Type]);

	int distance2d = ((TPoint2D<int> &)start_coord - (TPoint2D<int> &)end_coord).Length(); /// 2D distance (X,Y only)

	/// Set initial coordinates
	StartCoord = start_coord;
	PositionCoord = StartCoord;
	EndCoord = end_coord;

	WaveShape.Count = 6;
	WaveShape.Vertices = &ActiveWaveStartMiddle;

	/// Calculate wave points
	TacticalMap->Coord_To_Pixel(start_coord, WaveStartMiddle);
	TacticalMap->Coord_To_Pixel(end_coord, WaveEndMiddle);

	WaveShape.Vertices[PolygonShapeStruct::END_MIDDLE]   = WaveEndMiddle;
	WaveShape.Vertices[PolygonShapeStruct::START_MIDDLE] = WaveStartMiddle;

	double distance2d_f = distance2d;

	Vector3 end_left 	= WaveSize[Type][0] + Vector3(distance2d_f, 0, 0);
	Vector3 end_right 	= WaveSize[Type][1] + Vector3(distance2d_f, 0, 0);
	Vector3 start_left 	= WaveSize[Type][2] + Vector3(0, 0, 0);
	Vector3 start_right = WaveSize[Type][3] + Vector3(0, 0, 0);

	double dz = end_coord.Z - start_coord.Z;

	end_left.Z 		= end_left.X 	* dz / distance2d_f + start_coord.Z;
	end_right.Z 	= end_right.X 	* dz / distance2d_f + start_coord.Z;
	start_left.Z 	= start_left.X 	* dz / distance2d_f + start_coord.Z;
	start_right.Z 	= start_right.X * dz / distance2d_f + start_coord.Z;

	double dx = end_coord.X - start_coord.X;
	double dy = end_coord.Y - start_coord.Y;
	float dx_f = dx;
	double dist = std::sqrt(dy * dy + dx_f * dx_f);

	if (dist < dx) dx = dist;
	if (-dx > dist) dx = -dist;

	double theta = std::acos(dx / dist);
	if (start_coord.Y > end_coord.Y) {
		theta = -theta;
	}

	/// Calculate wave vectors
	Matrix3D matrix;
	matrix.Make_Identity();
	matrix.Rotate_Z(theta);

	end_left 	= matrix * end_left;
	end_right 	= matrix * end_right;
	start_left 	= matrix * start_left;
	start_right = matrix * start_right;

	/// Set wave coordinates
	WaveEndLeftCoord 	= Coord(start_coord.X + end_left.X,    start_coord.Y + end_left.Y,    end_left.Z);
	WaveEndRightCoord 	= Coord(start_coord.X + end_right.X,   start_coord.Y + end_right.Y,   end_right.Z);
	WaveStartLeftCoord 	= Coord(start_coord.X + start_left.X,  start_coord.Y + start_left.Y,  start_left.Z);
	WaveStartRightCoord = Coord(start_coord.X + start_right.X, start_coord.Y + start_right.Y, start_right.Z);

	/// Convert coordinates to pixels
	TacticalMap->Coord_To_Pixel(WaveEndLeftCoord, WaveEndLeft);
	TacticalMap->Coord_To_Pixel(WaveEndRightCoord, WaveEndRight);
	TacticalMap->Coord_To_Pixel(WaveStartLeftCoord, WaveStartLeft);
	TacticalMap->Coord_To_Pixel(WaveStartRightCoord, WaveStartRight);

	/// Set wave points
	WaveShape.Vertices[PolygonShapeStruct::END_LEFT] = WaveEndLeft;
	WaveShape.Vertices[PolygonShapeStruct::END_RIGHT] = WaveEndRight;
	WaveShape.Vertices[PolygonShapeStruct::START_RIGHT] = WaveStartRight;
	WaveShape.Vertices[PolygonShapeStruct::START_LEFT] = WaveStartLeft;

	/// Copy facing
	Facing = Source->SecondaryFacing;
}


/// <summary>
/// Handles the per frame update of the wave geometry.
/// This routine keeps the wave anchored to the object that fired it and to the object it
/// was aimed at. While that link holds, the shape is rebuilt and its leading edge is drawn
/// further out until the wave has reached its full length. Once the link is broken the
/// trailing edge is drawn in after it, and the wave removes itself when nothing of it is
/// left. The list of cells the wave covers is refreshed before returning.
/// </summary>
void WaveClass::Wave_Shape_AI(void)
{
	if (Target != NULL && Source != NULL && WaveEC != (int)(1.0 / WaveStep) && Source->TarCom == Target) {
		if ((Source->PositionCoord - Target->As_Coord()).Length() > CELL_LEPTON_DIAG * 6.0) {
			IsWaveActive = false;
		}
	} else {
		IsWaveActive = false;
	}

	if (IsWaveActive) {
		Build_Wave_Shape(Source->Fire_Coord(0), Target->As_Coord());
	}

	if (WaveProgress < 1.0 && IsWaveActive) {
		WaveProgress += WaveStep;
		if (WaveProgress > 0.98) {
			WaveProgress = 1.0;
		}
		WaveShape.Vertices[PolygonShapeStruct::END_LEFT] = Lerp(WaveStartLeft, WaveEndLeft, WaveProgress);
		WaveShape.Vertices[PolygonShapeStruct::END_MIDDLE] = Lerp(WaveStartMiddle, WaveEndMiddle, WaveProgress);
		WaveShape.Vertices[PolygonShapeStruct::END_RIGHT] = Lerp(WaveStartRight, WaveEndRight, WaveProgress);
	} else if (!IsWaveActive) {
		if (WaveStep * 0.5 + WaveProgress >= FadeProgress) {
			FadeProgress += WaveStep;
			if (WaveProgress <= FadeProgress) {
				Delete_Me();
				return;
			} else {
				WaveShape.Vertices[PolygonShapeStruct::START_RIGHT] = Lerp(WaveStartRight, WaveEndRight, FadeProgress);
				WaveShape.Vertices[PolygonShapeStruct::START_MIDDLE] = Lerp(WaveStartMiddle, WaveEndMiddle, FadeProgress);
				WaveShape.Vertices[PolygonShapeStruct::START_LEFT] = Lerp(WaveStartLeft, WaveEndLeft, FadeProgress);
			}
		}
	}

	Wave_Recalc_Affected_Cells();
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with the RTTI identifier that marks this object as a wave.</returns>
RTTIType WaveClass::Fetch_RTTI(void) const
{
	return(RTTI_WAVE);
}
