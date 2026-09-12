/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "ionblast.h"

#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_zbuffer.h"
#include "anim.h"
#include "bsurface.h"
#include "building.h"
#include "cell.h"
#include "combat.h"
#include "foot.h"
#include "globals.h"
#include "goptions.h"
#include "infantry.h"
#include "rules.h"
#include "tactical.h"
#include "unit.h"
#include "vector.h"
#include "vector3.h"
#include "zbuffer.h"

#include <algorithm>

int SpiralIndexToSurfaceLUT[289];
Point2D SpiralIndexToScreenLUT[289];
int BlastSurfaceStride = 0;

BSurface * BlastSurfaces[80];

/*
 * Parameters of the ion blast shockwave ripple. The ripple is an elliptical
 * band that sweeps outward from the blast center over the lifetime of the
 * blast, displacing the pixels it covers by the height of a decaying sine
 * wave. The band starts as [WaveStartDistance - WaveBandWidth, WaveStartDistance]
 * and both edges advance by (a multiple of) WaveSpeed every frame. WaveRockScale
 * turns the slope of the wave into the angle a voxel unit riding it is rocked by.
 */
const double WaveStartDistance = 0.0;
const double WaveBandWidth = 57.0;
const double WaveFrequency = 0.11;
const double WaveAmplitudeFalloff = 25.0;
const double WaveRockScale = DEG_TO_RAD(360);
const double WaveSpeedScale = 0.0125;

double WaveSpeed = ((256.0 - WaveStartDistance) + WaveBandWidth) * WaveSpeedScale;
double WavePhaseSpeed = WaveSpeed;

DynamicVectorClass<IonBlastClass *> IonBlastClass::IonBlasts;

int Point_To_Spiral_Index(Point2D point);
void Calculate_Index_To_Surface_LUT(int stride);
Point2D Spiral_Index_To_Point(int index);


/// <summary>
/// Creates an ion cannon ground blast at the specified location.
/// The blast adds itself to the list of blasts in progress and is looked after from
/// there until its shockwave has expired. The ion cannon superweapon and the trigger
/// action of the same name are the two things that set one off.
/// </summary>
IonBlastClass::IonBlastClass(Coord coord) :
	Lifetime(0),
	Position(coord)
{
	IonBlasts.Add(this);
}


/// <summary>
/// Removes this blast from the list of blasts in progress.
/// </summary>
IonBlastClass::~IonBlastClass(void)
{
	IonBlasts.Delete(this);
}


/// <summary>
/// Advances this ion blast by a single frame.
/// On the frame the blast lands it throws up the explosion animations, damages whatever
/// is beneath it and lights the scene. From then on the expanding shockwave takes hold
/// of the infantry and vehicles it rolls over, bringing them to a halt and rocking them
/// away from the center. Once the wave has run its course the blast deletes itself.
/// </summary>
void IonBlastClass::AI(void)
{
	if (Lifetime >= 79) {
		delete this;
		return;
	}

	Point2D point;
	if (TacticalMap->Coord_To_Pixel(Position, point)) {
		if (Lifetime == 0) {
			Map.ScreenY = 10;
		}
	}

	if (Lifetime == 0) {
		Coord coord;

		if (Map[Position].Land_Type() == LAND_WATER) {
			new AnimClass(Rule->SplashList[Rule->SplashList.Count() - 1], Position + Coord(0,0,5), 0, 1, (ShapeFlags_Type)(SHAPE_CENTER|SHAPE_WIN_REL), 0);
		} else {
			new AnimClass(Rule->IonBlast, Position + Coord(0,0,5), 0, 1, (ShapeFlags_Type)(SHAPE_CENTER|SHAPE_WIN_REL), 0);
		}
		new AnimClass(Rule->IonBeam, Position + Coord(0,0,5), 0, 1, (ShapeFlags_Type)(SHAPE_CENTER|SHAPE_WIN_REL), 0);

		if (Map[Position].IsUnderBridge) {
			coord.X = Position.X;
			coord.Z = BRIDGE_LEPTON_HEIGHT + Position.Z;
			coord.Y = Position.Y;
			Explosion_Damage(coord, Rule->IonCannonDamage, NULL, Rule->IonCannonWarhead, true);
		}

		Explosion_Damage(Position, Rule->IonCannonDamage, NULL, Rule->IonCannonWarhead, true);
		Combat_Lighting(Position, Rule->IonCannonDamage, Rule->IonCannonWarhead, false);
	}

	Cell center(Position);

	for (int x_offset = -3; x_offset < 4; x_offset++) {
		for (int y_offset = -3; y_offset < 4; y_offset++) {

			FootClass * occupier = (FootClass *)Map[center + Cell(x_offset, y_offset)].Cell_Occupier();

			while (occupier != NULL) {

				if (dynamic_cast<InfantryClass *>((ObjectClass *)occupier) != NULL || dynamic_cast<UnitClass *>((ObjectClass *)occupier) != NULL) {

					Point2D occupier_point;
					TacticalMap->Coord_To_Pixel(occupier->Get_Coord(), occupier_point);

					Point2D diff = occupier_point - point;
					int screen_dist = diff.Length() + 8;

					if (screen_dist < 128) {

						char * displacement = (char *)BlastSurfaces[Lifetime]->Lock(Point2D(screen_dist + 128, 64));

						if (*displacement > 0) {

							/*
							 * Units talking to a weapons factory (i.e. currently driving
							 * out of one) are not stopped by the blast wave.
							 */
							bool stop = true;
							if (occupier->Contact_With_Whom() != NULL) {
								if (occupier->Contact_With_Whom()->Fetch_RTTI() == RTTI_BUILDING) {
									if (((BuildingClass *)occupier->Contact_With_Whom())->Class->IsWeaponsFactory) {
										stop = false;
									}
								}
							}

							/*
							 * Likewise, units standing on a weapons factory's exit cells
							 * are not stopped, so they cannot pile up on the bib.
							 */
							bool on_exit_cell = false;
							BuildingClass * building = Map[occupier->Get_Coord()].Cell_Building();
							if (building != NULL && building->Class->IsWeaponsFactory) {

								Cell building_cell(building->Get_Coord());
								Cell occupier_cell(occupier->Get_Coord());

								short cell_dx = occupier_cell.X - building_cell.X;
								short cell_dy = occupier_cell.Y - building_cell.Y;

								if (cell_dx == 0) {
									if (cell_dy == 1) {
										on_exit_cell = true;
									}
								} else if (cell_dx == 2) {
									if (cell_dy == 1) {
										on_exit_cell = true;
									}
								} else if (cell_dx == 3) {
									if (cell_dy == 1) {
										on_exit_cell = true;
									}
								}
							}

							if (!on_exit_cell && stop) {
								occupier->Set_Speed(0.0);
							}

							int draw_offset = Spiral_Index_To_Point(*displacement).Y;
							occupier->IonBlastYDrawOffset = 2 * draw_offset;
						}

						if (occupier->Techno_Type_Class()->Voxel.VoxLib != NULL
							&& !occupier->Techno_Type_Class()->Voxel.VoxLib->Load_Failed()
							&& *displacement >= 0) {

							/*
							 * Rock the voxel unit away from the blast: build the vector
							 * from the unit to the blast center, rotate it into the unit's
							 * facing frame, and scale it by the slope of the ripple at the
							 * unit's distance from the blast. The wave height there is
							 * height/decay, so its slope comes out by the quotient rule.
							 */
							Coord blast_coord = Position;
							Coord occupier_coord = occupier->Get_Coord();

							Vector3 to_blast((float)(blast_coord.X - occupier_coord.X), (float)(occupier_coord.Y - blast_coord.Y), (float)(blast_coord.Z - occupier_coord.Z));
							Vector3 flat = to_blast;

							double facing_rad = occupier->PrimaryFacing.Current().As_Radian();
							float facing_sin = (float)std::sin(facing_rad);
							float facing_cos = (float)std::cos(facing_rad);

							float blast_dist = to_blast.Length();

							if (fabs(blast_dist) > 0.00002) {

								flat.Z = 0.0;

								double flat_len = std::sqrt(flat.Y * flat.Y + flat.X * flat.X);

								Vector3 normal;
								if (flat_len != 0.0) {
									normal = flat / flat_len;
								} else {
									normal = flat;
								}

								float forwards = normal.X * facing_cos + normal.Y * facing_sin;
								float sideways_zsin = -(normal.Z * facing_sin);
								double sideways_zcos = normal.Z * facing_cos;
								double sideways_y = normal.X * facing_sin - normal.Y * facing_cos;
								float sideways = (float)std::sqrt(sideways_zsin * sideways_zsin + sideways_zcos * sideways_zcos + sideways_y * sideways_y);

								if (fabs(facing_cos * forwards - facing_sin * sideways - normal.X) > 0.0002
									|| fabs(facing_cos * sideways + facing_sin * forwards - normal.Y) > 0.0002) {
									sideways = -sideways;
								}

								double decay = blast_dist + WaveAmplitudeFalloff;
								double height = (std::sin((blast_dist - (double)Lifetime * WavePhaseSpeed + 38.0) * WaveFrequency) * 3.5 + 3.0) * WaveAmplitudeFalloff;
								double phase_cos = std::cos((blast_dist - (double)Lifetime * WavePhaseSpeed + 38.0) * WaveFrequency);
								double slope = ((phase_cos * WaveFrequency * WaveAmplitudeFalloff * 3.5) * decay - height) / (decay * decay);

								occupier->AngleRotatedSideways = (float)(sideways * slope * WaveRockScale);
								occupier->AngleRotatedForwards = (float)(-(forwards * slope * WaveRockScale));
							}
						}
					}
				}

				occupier = (FootClass *)occupier->Next;
			}
		}
	}

	Lifetime++;
}


/// <summary>
/// Processes the logic of every ion blast in progress.
/// This routine is called by the main game logic once per frame. Blasts that have
/// outlived their shockwave delete themselves along the way.
/// </summary>
void IonBlastClass::Update_All(void)
{
	for (int i = IonBlasts.Count() - 1; i >= 0; i--) {
		IonBlasts[i]->AI();
	}
}


/// <summary>
/// Precalculates the shockwave displacement surfaces.
/// One surface is built for each frame of the blast animation, holding the direction
/// the ripple pushes every pixel around the blast center. This is far too expensive to
/// work out while the game is running, so it is done once during startup and the
/// surfaces are kept until the game exits.
/// </summary>
/// <remarks>Harmless to call more than once -- only the first call does any work.</remarks>
void IonBlastClass::One_Time(void)
{
	static bool one_time = false;
	if (one_time) {
		return;
	}

	double min_dist = WaveStartDistance - WaveBandWidth;
	double max_dist = WaveStartDistance;

	for (int frame = 0; frame < ARRAY_SIZE(BlastSurfaces); frame++) {
		BlastSurfaces[frame] = new BSurface(256, 128, 1);
		BlastSurfaces[frame]->Fill(-1);

		/*
		 * Rasterize this frame's ripple band into the displacement surface.
		 * The surface is 256x128 with the blast center in the middle; only one
		 * quadrant is computed and the result is mirrored into the other three.
		 * The vertical axis counts double toward the distance because the
		 * shockwave ellipse is half as tall as it is wide.
		 */
		char * center = (char *)BlastSurfaces[frame]->Lock() + 0x4080;
		for (int y = 63; y >= 0; y--) {
			int y_dist_sq = y * y * 4;
			for (int x = 127; x >= 0; x--) {
				double dist = std::sqrt(x * x + y_dist_sq);
				if (dist >= min_dist && dist <= max_dist) {
					double wave_height = (std::sin((dist - frame * WavePhaseSpeed + 38.0) * WaveFrequency) * 3.5 + 3.0) / (dist / WaveAmplitudeFalloff + 1.0) + 0.5;
					char spiral_index = Point_To_Spiral_Index(Point2D(0, wave_height));
					int row = y << 8;
					center[row + x] = spiral_index;  /// Quadrant I
					center[row - x] = spiral_index;  /// Quadrant II
					char * mirror = center - row;
					mirror[x] = spiral_index;        /// Quadrant IV
					mirror[-x] = spiral_index;       /// Quadrant III
				}
			}
		}

		if (128.0 - WaveSpeed > max_dist) {
			max_dist += WaveSpeed;
		}

		min_dist = WaveSpeed * 1.2 + min_dist;

		if (min_dist > max_dist) {
			min_dist = max_dist;
		}
	}

	one_time = true;
}


/// <summary>
/// Frees the precalculated shockwave surfaces.
/// This routine is called as the game shuts down, to give back the memory One_Time
/// claimed for the wave animation.
/// </summary>
void IonBlastClass::Clear_All(void)
{
	for (int i = 0; i < ARRAY_SIZE(BlastSurfaces); i++) {
		delete BlastSurfaces[i];
		BlastSurfaces[i] = NULL;
	}
}


/// <summary>
/// Decides whether a displaced pixel may be fetched for the shockwave.
/// The neighbor a warped pixel is replaced by must itself lie within the view, or the
/// warp would carry interface pixels or memory past the frame into the blast.
/// </summary>
/// <param name="x">The view-relative column of the pixel being drawn.</param>
/// <param name="y">The view-relative row of the pixel being drawn.</param>
/// <param name="index">The spiral index of the displacement.</param>
/// <returns>bool; Does the displaced fetch stay inside the view?</returns>
static bool Fetch_In_View(int x, int y, int index)
{
	// The blast drawer works in view-relative coordinates — the pixel behind (x, y) sits
	// TacticalRect.Y rows lower on the surface — so the view's own extent is the bound.
	Point2D const & offset = SpiralIndexToScreenLUT[index];
	return(Rect(0, 0, TacticalRect.Width, TacticalRect.Height).Is_Point_Within(Point2D(x + offset.X, y + offset.Y)));
}


/// <summary>
/// Draws this blast's shockwave onto the tactical view.
/// Nothing of the wave is actually drawn; instead the scene already on the surface is
/// warped, each pixel the ripple band covers being replaced by a neighbor pulled from
/// the direction the wave is pushing. The depth buffer is consulted so that anything
/// standing nearer than the wave is left undisturbed. The effect is a luxury, so it
/// only appears at the highest detail level.
/// </summary>
/// <remarks>Call this only after the scene has been rendered, and only once the wave
/// surfaces have been built by One_Time.</remarks>
void IonBlastClass::Draw_It(void)
{
	Point2D point;
	if (Options.DetailLevel == 2 && TacticalMap->Coord_To_Pixel(Position, point)) {
		Surface * dest_surface = LogicalSurface;
		Rect dcliprect = TacticalRect;
		Surface * source_surface = BlastSurfaces[Lifetime];
		int stride = LogicalSurface->Stride();
		Rect srect(0, 0, 256, 128);
		Rect drect(point.X - 128, point.Y - 64, 256, 128);
		Rect scliprect(0, 0, 256, 128);
		bool overlapped = false;
		short * dbuffer;
		char * sbuffer;

		if (XSurface::Prep_For_Blit(*dest_surface, dcliprect, drect, *source_surface, scliprect, srect, overlapped, (void*&)dbuffer, (void*&)sbuffer)) {
			char * sptr = sbuffer;
			short * dest_row = dbuffer;
			char * source_row = sbuffer;

			int depth_z = TacticalMap->Z_Lepton_To_Pixel(Position.Z);
			short base_z = (short)DepthBuffer->Get_Scroll() - depth_z;
			unsigned short draw_z = base_z - drect.Y - 3;

			unsigned short * zbuffer_base = DepthBuffer->Get_Buffer_Offset(Point2D(0, drect.Y));
			int height = srect.Height + 1;
			int surface_width = LogicalSurface->Get_Width();
			int buffer_width = DepthBuffer->Get_Buffer_Width();

			if (zbuffer_base + surface_width + height * buffer_width < DepthBuffer->Get_Buffer_End()) {
				unsigned short * zptr = zbuffer_base + drect.X;
				int x;
				for (int y = 0; y < srect.Height; ++y) {
					short * dptr = dest_row;
					for (x = 0; x < srect.Width; ++x) {
						int index = *sptr++;
						if (index > 0 && *zptr > draw_z && Fetch_In_View(drect.X + x, drect.Y + y, index)) {
							*dptr = dptr[SpiralIndexToSurfaceLUT[index]];
						}
						dptr++;
						zptr++;
					}
					dest_row = (short *)((char *)dest_row + stride);
					source_row += 256;
					draw_z--;
					sptr = source_row;
					zptr += buffer_width - x;
				}
			} else {
				for (int y = 0; y < srect.Height; ++y) {
					short * dptr = dest_row;
					for (int x = 0; x < srect.Width; ++x) {
						int index = *sptr++;
						if (index > 0 && *DepthBuffer->Get_Buffer_Offset(Point2D(drect.X + x, drect.Y + y)) > draw_z && Fetch_In_View(drect.X + x, drect.Y + y, index)) {
							*dptr = dptr[SpiralIndexToSurfaceLUT[index]];
						}
						dptr++;
					}
					dest_row = (short *)((char *)dest_row + stride);
					source_row += 256;
					draw_z--;
					sptr = source_row;
				}
			}

			dest_surface->Unlock();
			source_surface->Unlock();
		}
	}
}


/// <summary>
/// Draws the shockwave of every ion blast in progress.
/// The tactical map calls this routine after the scene has been rendered, since the
/// blast distorts what is already on the surface rather than drawing anything of its
/// own.
/// </summary>
void IonBlastClass::Draw_All(void)
{
	if (LogicalSurface->Stride() != BlastSurfaceStride) {
		Calculate_Index_To_Surface_LUT(LogicalSurface->Stride());
	}
	for (int i = IonBlasts.Count() - 1; i >= 0; i--) {
		IonBlasts[i]->Draw_It();
	}
}


/// <summary>
/// Converts a spiral index back into a grid point.
/// Displacement offsets are kept as spiral indices so that each one fits into a single
/// byte of the wave surface. This routine unpacks one back into the offset it stands
/// for, counting outward from the center in concentric square rings.
/// </summary>
/// <returns>Returns with the grid point that the index refers to.</returns>
Point2D Spiral_Index_To_Point(int index)
{
	index--; /// Skip the center point, which corresponds to index 0.

	int layer = 1; /// Start at the first spiral layer.

	/*
	 * Determine which concentric layer the index falls into.
	 * Each layer has 8 * radius points.
	 */
	if (index >= 8) {
		for (int i = 8; index >= i; i += 8) {
			index -= i;
			layer++;
		}
	}

	/*
	 * Map the index within the layer to a point on one of its four sides:
	 * Right (X = +radius), Top (Y = +radius),
	 * Left (X = -radius), Bottom (Y = -radius).
	 */
	if (index < 2 * layer + 1) {
		return(Point2D(layer, index - layer)); /// Right edge
	} else if (index < 4 * layer + 1) {
		return(Point2D(3 * layer - index, layer)); // Top edge
	} else if (index < 6 * layer + 1) {
		return(Point2D(-layer, 5 * layer - index)); /// Left edge
	} else {
		return(Point2D(index - 7 * layer, -layer)); // Bottom edge
	}
}


/// <summary>
/// Converts a grid point into its index within the spiral.
/// This routine is the inverse of Spiral_Index_To_Point. The shockwave rasterizer uses
/// it to fold a displacement offset down into the single byte that is stored per pixel
/// of the wave surface.
/// </summary>
/// <returns>Returns with the spiral index of the point, zero being the center.</returns>
int Point_To_Spiral_Index(Point2D point)
{
	/*
	 * The center point (0, 0) is the origin of the spiral and has index 0.
	 */
	if (point == Point2D(0,0)) {
		return(0);
	}

	/*
	 * Determine which concentric square layer the point lies in.
	 * Each layer `n` surrounds the previous one and spans from -n to +n in both X and Y.
	 */
	int layer = std::max(abs(point.X), abs(point.Y));

	int index = 1;

	for (int i = 1; i < layer; i++) {
		index += 8 * i;
	}

	/*
	 * Add the offset of the point within its current layer's perimeter.
	 * Each layer has 8 * n positions, arranged clockwise starting from (n, -n+1).
	 * The positions proceed in the following order:
	 * - From (n, -n+1) up to (n, n)          -> Right edge (ascending Y)
	 * - From (n-1, n) to (-n, n)             -> Top edge (descending X)
	 * - From (-n, n-1) to (-n, -n)           -> Left edge (descending Y)
	 * - From (-n+1, -n) to (n, -n)           -> Bottom edge (ascending X)
	 */
	if (point.X == layer) {
		/// Right edge
		index += 1 * layer + point.Y;
	} else if (point.Y == layer) {
		// Top edge
		index += 3 * layer - point.X;
	} else if (point.X == -layer) {
		/// Left edge
		index += 5 * layer - point.Y;
	} else {
		// Bottom edge
		index += 7 * layer + point.X;
	}

	return(index);
}


/// <summary>
/// Rebuilds the spiral index to buffer offset table.
/// The blast drawer displaces a pixel by fetching a neighbor a short way off in the
/// direction the ripple is pushing. This routine works out where each of those
/// neighbors lives in a surface of the given stride, so that the drawer only has to
/// add. Draw_All calls it whenever the logical surface stride has changed.
/// </summary>
/// <param name="stride">The row stride of the surface the blast will be drawn onto.</param>
void Calculate_Index_To_Surface_LUT(int stride)
{
	/*
	 * Spiral center offset is always zero.
	 */
	SpiralIndexToSurfaceLUT[0] = 0;
	SpiralIndexToScreenLUT[0] = Point2D(0, 0);

	/*
	 * Store the stride used.
	 */
	BlastSurfaceStride = stride;

	for (int index = 1; index < ARRAY_SIZE(SpiralIndexToSurfaceLUT); index++) {
		Point2D point = Spiral_Index_To_Point(index);
		SpiralIndexToSurfaceLUT[index] = point.X + point.Y * stride;

		// The surface offset is built from the byte stride but applied to 16-bit pixels,
		// so the fetch truly lands twice the point's Y away; the screen table records that.
		SpiralIndexToScreenLUT[index] = Point2D(point.X, 2 * point.Y);
	}
}
