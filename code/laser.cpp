/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "laser.h"

#include "_map.h"
#include "_rect.h"
#include "_surface.h"
#include "_tactica.h"
#include "ccrand.h"
#include "coord.h"
#include "dsurface.h"
#include "globals.h"
#include "goptions.h"
#include "inline.h"
#include "mouse.h"
#include "rgb.h"
#include "scenario.h"
#include "tactical.h"

#include <algorithm>

DynamicVectorClass<LaserDrawClass *> LaserDrawClass::LaserDraws;


/// <summary>
/// Creates a laser beam between two points in the world.
/// This routine is used by the weapon code whenever a laser, railgun or similar beam
/// weapon fires. The new beam adds itself to the master list, so the caller may forget
/// about it entirely -- it will be updated, drawn and finally retired on its own.
/// </summary>
/// <param name="zadjust">The depth bias applied to the beam's starting end.</param>
/// <param name="outer_spread">The maximum random color deviation of the outer glow.</param>
/// <param name="duration">The lifetime of the beam, in game frames.</param>
/// <param name="blinks">Should the beam flicker on and off as it ages?</param>
/// <param name="fades">Should the beam's intensity slide from start to end over its life?</param>
LaserDrawClass::LaserDrawClass(Coord start, Coord end, int zadjust, RGBClass inner_color, RGBClass outer_color, RGBClass outer_spread, int duration, bool blinks, bool fades, float start_intensity, float end_intensity) :
	StageClass(),
	Start(start),
	End(end),
	ZAdjust(zadjust),
	InnerColor(),
	OuterColor(),
	OuterSpread(),
	Duration(duration),
	Blinks(blinks),
	BlinkState(false),
	Fades(fades),
	StartIntensity(start_intensity),
	EndIntensity(end_intensity)
{
	Set_Stage(0);
	Set_Rate(1);

	InnerColor = inner_color;
	OuterColor = outer_color;
	OuterSpread = outer_spread;

	LaserDraws.Add(this);
}


/// <summary>
/// Destroys the laser beam.
/// This routine removes the beam from the master list, so that it is no longer updated
/// or drawn.
/// </summary>
LaserDrawClass::~LaserDrawClass(void)
{
	LaserDraws.Delete(this);
}


/// <summary>
/// Removes every laser beam that is currently active.
/// This routine is used when the scenario is torn down, so that no beam survives into
/// the next game.
/// </summary>
void LaserDrawClass::All_Clear(void)
{
	while (LaserDraws.Count()) {
		delete LaserDraws[0];
	}
}


/// <summary>
/// Handles the per frame logic for this laser beam.
/// This routine advances the animation stage, flips the blink state for a blinking beam,
/// and retires the beam once it has lived out its duration.
/// </summary>
/// <remarks>The laser may delete itself here, so do not touch it after this call.</remarks>
void LaserDrawClass::AI(void)
{
	Graphic_Logic();
	if (Blinks) {
		BlinkState = BlinkState == false;
	}

	if (Fetch_Stage() >= Duration) {
		delete this;
	}
}


/// <summary>
/// Processes the logic for every laser beam that is currently active.
/// This routine is called once per game logic loop. Beams that have outlived their
/// duration remove themselves from the list during this call.
/// </summary>
void LaserDrawClass::Update_All(void)
{
	for (int i = LaserDraws.Count() - 1; i >= 0; i--) {
		LaserDraws[i]->AI();
	}
}


/// <summary>
/// Draws every laser beam that is currently active.
/// This routine is called by the tactical map rendering pass so that beams appear over
/// the objects they were fired between.
/// </summary>
void LaserDrawClass::Draw_All(void)
{
	for (int i = LaserDraws.Count() - 1; i >= 0; i--) {
		LaserDraws[i]->Draw_It();
	}
}


/// <summary>
/// Draws the laser beam onto the logical surface.
/// This routine draws the beam as a thin inner core and, when an outer color was
/// specified, a pair of parallel glow lines flanking it. A beam that lies under the fog
/// of war, or that is currently blinked off, draws nothing at all. At the lowest detail
/// level the beam degrades to a plain depth shaded line.
/// </summary>
void LaserDrawClass::Draw_It(void)
{
	if (!Scen->Special.IsFogOfWar || !Map.Is_Fogged(Start) || !Map.Is_Fogged(End)) {
		static Point2D _glow_offsets[16] = {
			Point2D(0, -1),
			Point2D(1, 0),
			Point2D(0, -1),
			Point2D(0, 1),
			Point2D(1, 0),
			Point2D(0, 1),
			Point2D(-1, 0),
			Point2D(1, 0),
			Point2D(-1, 0),
			Point2D(0, 1),
			Point2D(0, -1),
			Point2D(1, 0),
			Point2D(0, -1),
			Point2D(-1, 0),
			Point2D(-1, 0),
			Point2D(1, 0)
		};

		if (!BlinkState) {
			/// The "thin" core (InnerColor) is drawn once with no offset. The "thick"
			/// outer glow (OuterColor) is two extra lines, each a 1-pixel parallel copy
			/// of the core shifted by _glow_offsets[2*facing] / [2*facing+1].
			///
			/// The glow can look like it "leans" a slightly different way than the core at
			/// some angles. That is deliberate, not a defect here. Two reasons:
			/// 1. facing is the WORLD-space beam direction quantized to just 8 compass
			/// points (As_Dir8), so in-between angles get the nearest sector's offset.
			/// 2. facing is world-space but the offsets are applied in ISOMETRIC screen
			/// space (Y compressed ~2:1), so the offset is only approximately
			/// perpendicular to the on-screen beam.
			/// Per-facing the offset PAIR straddles the core symmetrically only for
			/// NE(1)/SE(3)/NW(7); for N(0)/E(2)/S(4)/SW(5)/W(6) the pair is an L-corner
			/// (both on one side), biasing the glow off the core axis -- this is the
			/// "thick and thin face different ways" artifact. Do NOT "fix" the table or
			/// switch to screen-space facing without opting into a deliberate deviation.
			DirType dir = Direction(Start, End);
			FacingType facing = dir.As_Facing();

			Point2D start_pixel;
			TacticalMap->Coord_To_Pixel(Start, start_pixel);
			Point2D end_pixel;
			TacticalMap->Coord_To_Pixel(End, end_pixel);

			int start_z = ZAdjust - TacticalMap->Z_Lepton_To_Pixel(Start.Z) - 2;
			int end_z = -TacticalMap->Z_Lepton_To_Pixel(End.Z) - 2;

			RGBClass outer_color;
			int outer_hicolor = 0;
			bool has_outer_glow = true;

			if (OuterColor == RGBClass(0, 0, 0)) {
				has_outer_glow = false;
			} else {
				int red = Sim_Random_Pick(-OuterSpread.Get_Red(), OuterSpread.Get_Red());
				int green = Sim_Random_Pick(-OuterSpread.Get_Green(), OuterSpread.Get_Green());
				int blue = Sim_Random_Pick(-OuterSpread.Get_Blue(), OuterSpread.Get_Blue());

				red += OuterColor.Get_Red();
				red = std::max(0, red);
				red = std::min(255, red);

				green += OuterColor.Get_Green();
				green = std::max(0, green);
				green = std::min(255, green);

				blue += OuterColor.Get_Blue();
				blue = std::max(0, blue);
				blue = std::min(255, blue);

				outer_hicolor = DSurface::Build_Hicolor_Pixel(red, green, blue);
				outer_color.Set_Red(red);
				outer_color.Set_Green(green);
				outer_color.Set_Blue(blue);
			}

			float current_intensity = 1.0;

			bool has_r = InnerColor.Get_Red() > 0;
			bool has_g = InnerColor.Get_Green() > 0;
			bool has_b = InnerColor.Get_Blue() > 0;

			if (Fades) {
				current_intensity = (StartIntensity - EndIntensity) * (Duration - Fetch_Stage()) / Duration + EndIntensity;
			}

			if (Options.DetailLevel != 0) {
				if (Blinks) {
					LogicalSurface->Draw_Depth_Antialiased_Line(TacticalRect, start_pixel, end_pixel, InnerColor, start_z, end_z, false, true, true, true, current_intensity);
					if (has_outer_glow) {
						LogicalSurface->Draw_Depth_Antialiased_Line(
							TacticalRect,
							start_pixel + _glow_offsets[2 * facing],
							end_pixel + _glow_offsets[2 * facing],
							outer_color,
							start_z, end_z, false, true, true, true, 1.0
						);

						LogicalSurface->Draw_Depth_Antialiased_Line(
							TacticalRect,
							start_pixel + _glow_offsets[2 * facing + 1],
							end_pixel + _glow_offsets[2 * facing + 1],
							outer_color,
							start_z, end_z, false, true, true, true, 1.0
						);
					}
				} else {
					LogicalSurface->Draw_Depth_Antialiased_Line(TacticalRect, start_pixel, end_pixel, InnerColor, start_z, end_z, false, has_r, has_g, has_b, current_intensity);
					if (has_outer_glow) {
						LogicalSurface->Draw_Depth_Antialiased_Line(
							TacticalRect,
							start_pixel + _glow_offsets[2 * facing],
							end_pixel + _glow_offsets[2 * facing],
							outer_color,
							start_z, end_z, false, true, false, false, 1.0
						);

						LogicalSurface->Draw_Depth_Antialiased_Line(
							TacticalRect,
							start_pixel + _glow_offsets[2 * facing + 1],
							end_pixel + _glow_offsets[2 * facing + 1],
							outer_color,
							start_z, end_z, false, true, false, false, 1.0
						);
					}
				}
			} else {
				LogicalSurface->Draw_Depth_Shaded_Line(TacticalRect, start_pixel, end_pixel,
					DSurface::Build_Hicolor_Pixel(InnerColor.Get_Red(), InnerColor.Get_Green(), InnerColor.Get_Blue()),
					start_z, end_z, false
				);

				if (has_outer_glow) {
					LogicalSurface->Draw_Depth_Shaded_Line(
						TacticalRect,
						start_pixel + _glow_offsets[2 * facing],
						end_pixel + _glow_offsets[2 * facing],
						outer_hicolor,
						start_z, end_z
					);

					LogicalSurface->Draw_Depth_Shaded_Line(
						TacticalRect,
						start_pixel + _glow_offsets[2 * facing + 1],
						end_pixel + _glow_offsets[2 * facing + 1],
						outer_hicolor,
						start_z, end_z
					);
				}
			}
		}
	}
}
