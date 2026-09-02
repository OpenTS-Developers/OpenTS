/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "ion.h"

#include "_convert.h"
#include "_map.h"
#include "_mixfile.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "aircraft.h"
#include "anim.h"
#include "building.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "convert.h"
#include "data.h"
#include "draw.h"
#include "foot.h"
#include "globals.h"
#include "gscreen.h"
#include "house.h"
#include "language/language.h"
#include "laser.h"
#include "lightcon.h"
#include "mixfile.h"
#include "mouse.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "scheme.h"
#include "session.h"
#include "sun.h"
#include "theme.h"
#include "vox.h"

#include "color.hh"

#define STATIC_SHAPE_NAME		"STATIC.SHP"
#define STATIC_SHAPE_WIDTH		56
#define STATIC_SHAPE_HEIGHT		48
#define STATIC_DELAY			7


bool IonStormClass::IsActive = false;
int IonStormClass::StartFrame = -1;
int IonStormClass::Duration = -1;
int IonStormClass::Deferment = 0;
ShapeSet const * IonStormClass::StaticShape = NULL;
ThemeType IonStormClass::PreviousTheme = THEME_PICK_ANOTHER;


/// <summary>
/// Clears the ion storm state back to its dormant condition.
/// This routine is called as a scenario is being set up, so that no storm or pending
/// storm is carried over from the previous mission.
/// </summary>
void IonStormClass::Init(void)
{
	IsActive = false;
	StartFrame = -1;
	Duration = -1;
	Deferment = 0;
}


/// <summary>
/// Saves the ion storm state to the save game stream.
/// </summary>
/// <returns>Returns with the result reported by the stream write.</returns>
HRESULT IonStormClass::Save(IStream * stream)
{
	SaveStreamClass savestream(stream, SaveStreamClass::MODE_SAVE);
	Serialize(savestream);
	return(savestream.Result());
}


/// <summary>
/// Loads the ion storm state from the save game stream.
/// </summary>
/// <returns>Returns with the result reported by the stream read.</returns>
/// <remarks>Only the bookkeeping is restored here. Post_Load_Game must still call
/// Apply_Secondary_Effect to put the world back into its storm bound state.</remarks>
HRESULT IonStormClass::Load(IStream * stream)
{
	SaveStreamClass savestream(stream, SaveStreamClass::MODE_LOAD);
	savestream.Set_Context("IonStormClass");
	Serialize(savestream);
	return(savestream.Result());
}


/// <summary>
/// Lists the ion storm bookkeeping the save game carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void IonStormClass::Serialize(SaveStreamClass & stream)
{
	stream.Serialize(IsActive);
	stream.Serialize(StartFrame);
	stream.Serialize(Duration);
	stream.Serialize(Deferment);
	// StaticShape -- artwork fetched by name when it is first needed.
	// PreviousTheme -- the score resumed after a storm is picked afresh.
}


/// <summary>
/// Fills the tactical view with a frame of screen static.
/// This routine is used while the storm is tinting or untinting the palettes, which
/// takes long enough to be noticeable. Rather than let the display sit frozen, the
/// static is tiled over the tactical view and flushed straight to the screen. Nothing
/// is drawn during scenario initialization.
/// </summary>
/// <param name="shapenum">The static shape frame to tile over the tactical view.</param>
void IonStormClass::Do_Screen_Static(int shapenum)
{
	if (!ScenarioInit) {
		if (StaticShape == NULL) {
			StaticShape = (ShapeSet const *)MFCD::Retrieve(STATIC_SHAPE_NAME);
		}
		for (int x = 0; x < TacticalRect.Width; x += STATIC_SHAPE_WIDTH) {
			for (int y = TacticalRect.Y; y < TacticalRect.Y + TacticalRect.Height; y += STATIC_SHAPE_HEIGHT) {
				Draw_Shape(*CompositeSurface, *NormalDrawer, StaticShape, shapenum, Point2D(x, y), Rect(0, 0, TacticalRect.Width, TacticalRect.Y + TacticalRect.Height));
			}
		}
		Update_Visible_Surface(CompositeSurface);
	}
}


/// <summary>
/// Records whether an ion storm is in progress.
/// This routine is used by the storm start and end handlers. The rest of the game asks
/// Is_Ion_Storm_Active rather than reaching for the flag itself.
/// </summary>
/// <remarks>This merely records the state. Use Ion_Storm_Begin and Ion_Storm_End to
/// actually raise or lift a storm.</remarks>
void IonStormClass::Set_Ion_Storm_Active(bool active)
{
	IsActive = active;
}


/// <summary>
/// Starts an ion storm, or schedules one to arrive later.
/// When a warning period is asked for, this routine merely arms the countdown and
/// returns; the storm itself begins when the AI process works the countdown down to
/// zero. Otherwise the storm breaks immediately -- ion sensitive locomotors lose power
/// (aircraft rather dramatically), the world takes on the ion tint, and the storm theme
/// replaces whatever was playing. A storm already in progress is left undisturbed.
/// </summary>
/// <param name="duration">The number of game frames the storm should last.</param>
/// <param name="warning">The number of game frames to wait before the storm breaks. Use
/// zero to start it right now.</param>
void IonStormClass::Ion_Storm_Begin(int duration, int warning)
{
	int i;
	static int static_frame = 0;

	if (!Is_Ion_Storm_Active()) {
		if (warning != 0) {
			if (Deferment == 0 || Deferment >= warning) {
				Deferment = warning;
			}
			Duration = duration;

		} else {

			Duration = duration;
			StartFrame = Frame;

			for (i = Feet.Count() - 1; i >= 0; i--) {
				FootClass * foot = Feet[i];
				if (!foot->IsInLimbo && foot->Locomotion != NULL && foot->Locomotion->Is_Ion_Sensitive()) {
					foot->Locomotion->Power_Off();
					if (foot->RTTI == RTTI_AIRCRAFT) {
						((AircraftClass *)foot)->Crash(NULL);
					}
				}
				/// Crashing an aircraft can shorten the list part way through the walk, so
				/// the index is pulled back; the loop's own decrement then brings it into
				/// range.
				if (i > Feet.Count()) {
					i = Feet.Count();
				}
			}

			Scen->DesiredAmbientLight = Scen->IonAmbientLight;
			Set_Ion_Storm_Active(true);
			PlayerPtr->RecalcRadar = true;
			PreviousTheme = Theme.What_Is_Playing();
			Theme.Stop();

			CDTimerClass<SystemTimerClass> static_timer;

			int red = NORMAL_LIGHT * Scen->IonRedTint / 100;
			int green = NORMAL_LIGHT * Scen->IonGreenTint / 100;
			int blue = NORMAL_LIGHT * Scen->IonBlueTint / 100;

			for (i = 0; i < TileDrawers.Count(); i++) {
				TileDrawers[i]->Apply_Tint(red, green, blue, true);
				if (static_timer == 0) {
					Do_Screen_Static(static_frame++);
					if (static_frame >= 30) {
						static_frame = 0;
					}
					static_timer = STATIC_DELAY;
				}
			}

			for (i = 0; i < ColorSchemes.Count(); i++) {
				ColorScheme * scheme = ColorSchemes[i];
				if (scheme->IntensityLevels > 1) {
					scheme->Converter->Apply_Tint(red, green, blue, true);
					if (static_timer == 0) {
						Do_Screen_Static(static_frame++);
						if (static_frame >= 30) {
							static_frame = 0;
						}
						static_timer = STATIC_DELAY;
					}
				}
			}

			Map.Update_Cell_Colors();
			Theme.Play_Song(Theme.From_Name("IONSTORM"));
			Session.Messages.Add_Message(NULL, 0, Fetch_String(TXT_ION_STORM), GREEN, TextPrintType(TPF_USE_GRAD_PAL|TPF_FULLSHADOW|TPF_LED|TPF_8POINT), TICKS_PER_SECOND * 10);
			Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
		}
	}
}


/// <summary>
/// Brings the ion storm to an end.
/// This routine restores everything the storm took away -- power comes back to the ion
/// sensitive locomotors, the ambient light and the color tinting return to normal, and
/// the music that was playing when the storm rolled in is resumed. The screen static is
/// shown while the palettes are rebuilt, since that is a lengthy business.
/// </summary>
void IonStormClass::Ion_Storm_End(void)
{
	int i;
	static int static_frame = 0;

	if (Is_Ion_Storm_Active()) {
		for (i = 0; i < Feet.Count(); i++) {
			FootClass * foot = Feet[i];
			if (foot->Locomotion != NULL && foot->Locomotion->Is_Ion_Sensitive()) {
				foot->Locomotion->Power_On();
			}
		}

		Scen->DesiredAmbientLight = Scen->AmbientLight;
		Set_Ion_Storm_Active(false);
		if (PlayerPtr) PlayerPtr->RecalcRadar = true;
		Theme.Stop();

		CDTimerClass<SystemTimerClass> static_timer;

		for (i = 0; i < TileDrawers.Count(); i++) {
			TileDrawers[i]->Apply_Tint(-1, -1, -1, false);
			if (static_timer == 0) {
				Do_Screen_Static(static_frame++);
				if (static_frame >= 30) {
					static_frame = 0;
				}
				static_timer = STATIC_DELAY;
			}
		}

		for (i = 0; i < ColorSchemes.Count(); i++) {
			ColorSchemes[i]->Converter->Apply_Tint(-1, -1, -1, false);
			if (static_timer == 0) {
				Do_Screen_Static(static_frame++);
				if (static_frame >= 30) {
					static_frame = 0;
				}
				static_timer = STATIC_DELAY;
			}
		}

		Map.Update_Cell_Colors();
		Theme.Play_Song(PreviousTheme);
		PreviousTheme = THEME_PICK_ANOTHER;
		Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
	}
}


/// <summary>
/// Determines if an ion storm is currently raging.
/// </summary>
/// <returns>bool; Is an ion storm in progress?</returns>
bool IonStormClass::Is_Ion_Storm_Active(void)
{
	return(IsActive);
}


/// <summary>
/// Strikes the specified cell with a bolt of lightning.
/// This routine handles the whole spectacle -- the thunderclap, the explosion animation,
/// the combat lighting flash, and the damage inflicted on whatever was unlucky enough to
/// be standing there. Debris is thrown up if something was destroyed or the terrain was
/// altered, and a jagged laser trail is drawn from the strike point up into the sky.
/// </summary>
void IonStormClass::Lightning_Bolt(Cell cell)
{
	int i;

	CellClass * cellptr = &Map[cell];
	int cell_height = cellptr->Height;
	int bheight = (cellptr->IsUnderBridge ? BRIDGE_LEPTON_HEIGHT : 0);
	Coord coord(cell, bheight);
	coord.Z += LEVEL_LEPTON_H * cell_height;
	if (coord != COORD_NONE) {
		Sound_Effect(Rule->LightningSound);
		int damage = Rule->LightningDamage;
		new AnimClass(Combat_Anim(damage, Rule->IonStormWarhead, cellptr->Land_Type(), coord), coord, 0, 1, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER|SHAPE_ZGRAD), Get_Explosion_Z(coord));

		bool debris = false;
		BuildingClass * building = cellptr->Cell_Building();
		TechnoClass * techno = cellptr->Cell_Techno();

		if (building == NULL && techno == NULL) {
			switch (cellptr->Land_Type()) {
				case LAND_ROAD:
				case LAND_ROCK:
				case LAND_WALL:
				case LAND_WEEDS:
					debris = true;
					break;
			}
		}

		Combat_Lighting(coord, damage, Rule->IonStormWarhead);
		Explosion_Damage(coord, damage, NULL, Rule->IonStormWarhead);

		if (building != cellptr->Cell_Building() || techno != cellptr->Cell_Techno() || cellptr->Height != cell_height || debris == true) {
			int count = Random_Pick(2, 6);
			for (i = 0; i < count; i++) {
				//new AnimClass(Rule->MetallicDebris.Random_Pick(Scen->RandomNumber), coord);
				new AnimClass(Rule->MetallicDebris[Random_Pick(0, Rule->MetallicDebris.Count()-1)], coord);
			}
		}

		Coord crd = coord;
		while (crd.Z < 200 * LEVEL_LEPTON_H) {
			crd.X += Random_Pick(-128, 128);
			crd.Y += Random_Pick(-128, 128);
			crd.Z += Random_Pick(0, 3 * LEVEL_LEPTON_H);
			new LaserDrawClass(coord, crd, 0, true, RGBClass(240, 240, 255), RGBClass(230, 230, 240), RGBClass(0, 0, 0), 10, true, false, 1, 0);
			coord = crd;
		}
	}
}


/// <summary>
/// Handles the per frame logic of the ion storm.
/// While a storm rages this routine expires it when its duration runs out and
/// occasionally calls down a lightning bolt, favoring objects that carry a lightning
/// rod. While no storm rages it counts down any pending deferment, warning the player
/// as the storm approaches and starting it when the wait is over.
/// </summary>
void IonStormClass::AI(void)
{
	if (Is_Ion_Storm_Active()) {
		if (Duration != -1 && StartFrame + Duration < Frame) {
			Ion_Storm_End();
			return;
		}

		if (Random_Pick(0, 1000) < Rule->LightningFrequency) {
			Cell cell(-1, -1);
			if (!Percent_Chance(Rule->LightningRandomness)) {
				int chance = -1;
				DynamicVectorClass<TechnoClass *> targets;
				for (int i = 0; i < Technos.Count(); i++) {
					TechnoClass * techno = Technos[i];

					if (!techno->IsActive) continue;
					if (techno->RTTI == RTTI_AIRCRAFT) continue;

					chance = 2;

					if (((TechnoTypeClass const *)techno->Class_Of())->IsLightningRod) {
						switch ((RTTIType)techno->RTTI) {
							case RTTI_BUILDING:
								if (((BuildingClass *)techno)->IsOn) {
									chance = 42;
								}
								break;

							case RTTI_UNIT:
							case RTTI_INFANTRY:
								if (((FootClass *)techno)->Locomotion->Is_Powered()) {
									chance = 12;
								}
								break;
						}
					}

					if ((!techno->Is_Foot() || ((FootClass *)techno)->Team == NULL || !((FootClass *)techno)->Team->Class->IsIonImmune || ((TechnoTypeClass const *)techno->Class_Of())->IsLightningRod) &&
						chance != -1 && Percent_Chance(chance)) {
						targets.Add(techno);
					}
				}

				if (targets.Count()) {
					cell = targets[Random_Pick(0, targets.Count() - 1)]->Center_Coord().As_Cell();
				}
			} else {
				while (!Map.In_Radar(cell)) {
					cell = Cell(Random_Pick(0, Map.MapRect.Width), Random_Pick(0, Map.MapRect.Height));
				}
			}

			if (cell != Cell(-1, -1)) {
				Lightning_Bolt(cell);
			}
		}
	} else {
		if (Deferment != 0) {
			Deferment--;
			if (Deferment == 0) {
				Ion_Storm_Begin(Duration, 0);
			} else {
				if (Deferment % (TICKS_PER_SECOND * 15) == 0) {
					Speak(VOX_ION_STORM_APPROACHING);
					Session.Messages.Add_Message(NULL, 0, Fetch_String(TXT_ION_STORM_APPROACHING), GREEN, TextPrintType(TPF_FULLSHADOW|TPF_LED|TPF_8POINT), TICKS_PER_SECOND * 10);
				}
			}
		}
	}
}


/// <summary>
/// Re-applies the visible effects of an ion storm.
/// This routine is used to put the world back into the state a raging storm leaves it
/// in -- ion sensitive locomotors shut down, the ambient light dimmed, and every tile
/// drawer and color scheme tinted. The save game loader calls this after restoring a
/// scenario that was saved while a storm was in progress.
/// </summary>
/// <param name="do_static">Should the screen static be shown while the tinting proceeds?</param>
void IonStormClass::Apply_Secondary_Effect(bool do_static)
{
	int i;
	static int static_frame = 0;

	if (Is_Ion_Storm_Active()) {
		for (i = 0; i < Feet.Count(); i++) {
			FootClass * foot = Feet[i];
			if (foot->Locomotion != NULL && foot->Locomotion->Is_Ion_Sensitive()) {
				foot->Locomotion->Power_Off();
			}
		}

		Scen->DesiredAmbientLight = Scen->IonAmbientLight;
		PlayerPtr->RecalcRadar = true;

		CDTimerClass<SystemTimerClass> static_timer;

		int red = NORMAL_LIGHT * Scen->IonRedTint / 100;
		int green = NORMAL_LIGHT * Scen->IonGreenTint / 100;
		int blue = NORMAL_LIGHT * Scen->IonBlueTint / 100;

		for (i = 0; i < TileDrawers.Count(); i++) {
			TileDrawers[i]->Apply_Tint(red, green, blue, true);
			if (do_static) {
				if (static_timer == 0) {
					Do_Screen_Static(static_frame++);
					if (static_frame >= 30) {
						static_frame = 0;
					}
					static_timer = STATIC_DELAY;
				}
			}
		}

		for (i = 0; i < ColorSchemes.Count(); i++) {
			ColorScheme * scheme = ColorSchemes[i];
			if (scheme->IntensityLevels > 1) {
				scheme->Converter->Apply_Tint(red, green, blue, true);
				if (do_static) {
					if (static_timer == 0) {
						Do_Screen_Static(static_frame++);
						if (static_frame >= 30) {
							static_frame = 0;
						}
						static_timer = STATIC_DELAY;
					}
				}
			}
		}

		Map.Update_Cell_Colors();
	}
}
