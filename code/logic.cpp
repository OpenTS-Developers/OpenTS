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

/* $Header: /CounterStrike/LOGIC.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : LOGIC.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 27, 1993                                           *
 *                                                                                             *
 *                  Last Update : July 30, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LogicClass::AI -- Handles AI logic processing for game objects.                           *
 *   LogicClass::Debug_Dump -- Displays logic class status to the mono screen.                 *
 *   LogicClass::Detach -- Detatch the specified target from the logic system.                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "logic.h"

#include "_bench.h"
#include "_map.h"
#include "_rules.h"
#include "_tactica.h"
#include "_vanim.h"
#include "aircraft.h"
#include "alphashp.h"
#include "anim.h"
#include "bench.h"
#include "blight.h"
#include "building.h"
#include "bullet.h"
#include "empulse.h"
#include "factory.h"
#include "globals.h"
#include "incdec.h"
#include "infantry.h"
#include "ion.h"
#include "ionblast.h"
#include "laser.h"
#include "light.h"
#include "mono.h"
#include "object.h"
#include "ovrlight.h"
#include "partsys.h"
#include "rules.h"
#include "scenario.h"
#include "session.h"
#include "tactical.h"
#include "tag.h"
#include "terrain.h"
#include "tiberium.h"
#include "unit.h"
#include "vanim.h"
#include "vein.h"
#include "wave.h"

#include "bench.hh"

#include <algorithm>


unsigned FramesThisSecond=0;
unsigned LastFramesPerSecond=0;
unsigned TotalFrames=0;
unsigned SecondsPassed=0;


#ifdef _DEBUG
/***********************************************************************************************
 * LogicClass::Debug_Dump -- Displays logic class status to the mono screen.                   *
 *                                                                                             *
 *    This is a debugging support routine. It displays the current state of the logic class    *
 *    to the monochrome monitor. It assumes that it is being called once per second.           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Call this routine only once per second.                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   01/26/1996 JLB : Prints game time value.                                                  *
 *=============================================================================================*/
void LogicClass::Debug_Dump(MonoClass * mono) const
{
	#define RECORDCOUNT	40
	#define RECORDHEIGHT 21
	static int _framecounter = 0;

	static bool first = true;
	if (first) {
		first = false;
		mono->Set_Cursor(0, 0);
	}

	_framecounter++;
	mono->Set_Cursor(0, 0);mono->Printf("%d", AllowVoice);
	mono->Set_Cursor(1, 1);mono->Printf("%ld", (int)Scen->MissionTimer);
	mono->Set_Cursor(10, 1);mono->Printf("%3d", LastFramesPerSecond);
	mono->Set_Cursor(1, 3);mono->Printf("%02d:%02d:%02d", Scen->MissionTimer / TICKS_PER_HOUR, (Scen->MissionTimer % TICKS_PER_HOUR)/TICKS_PER_MINUTE, (Scen->MissionTimer % TICKS_PER_MINUTE)/TICKS_PER_SECOND);

	mono->Set_Cursor(1, 11);mono->Printf("%3d", Units.Count());
	mono->Set_Cursor(1, 12);mono->Printf("%3d", Infantry.Count());
	mono->Set_Cursor(1, 13);mono->Printf("%3d", Aircraft.Count());
	mono->Set_Cursor(1, 15);mono->Printf("%3d", Buildings.Count());
	mono->Set_Cursor(1, 16);mono->Printf("%3d", Terrains.Count());
	mono->Set_Cursor(1, 17);mono->Printf("%3d", Bullets.Count());
	mono->Set_Cursor(1, 18);mono->Printf("%3d", Anims.Count());
	mono->Set_Cursor(1, 19);mono->Printf("%3d", Teams.Count());
	mono->Set_Cursor(1, 20);mono->Printf("%3d", Triggers.Count());
	mono->Set_Cursor(1, 21);mono->Printf("%3d", TriggerTypes.Count());
	mono->Set_Cursor(1, 22);mono->Printf("%3d", Factories.Count());

	SpareTicks = std::min((int)SpareTicks, (int)TIMER_SECOND);

	/*
	**	CPU utilization record.
	*/
	mono->Sub_Window(15, 1, 6, 11);
	mono->Scroll();
	mono->Set_Cursor(0, 10);
	mono->Printf("%3d%%", ((TIMER_SECOND-SpareTicks)*100) / TIMER_SECOND);

	/*
	**	Update the frame rate log.
	*/
	mono->Sub_Window(22, 1, 6, 11);
	mono->Scroll();
	mono->Set_Cursor(0, 10);

	/*
	**	Update the findpath calc record.
	*/
	mono->Sub_Window(50, 1, 6, 11);
	mono->Scroll();
	mono->Set_Cursor(0, 10);
	mono->Printf("%4d", PathCount);
	PathCount = 0;

	/*
	**	Update the cell redraw record.
	*/
	mono->Sub_Window(29, 1, 6, 11);
	mono->Scroll();
	mono->Set_Cursor(0, 10);
	mono->Printf("%5d", CellCount);
	CellCount = 0;

	/*
	**	Update the target scan record.
	*/
	mono->Sub_Window(36, 1, 6, 11);
	mono->Scroll();
	mono->Set_Cursor(0, 10);
	mono->Printf("%5d", TargetScan);
	TargetScan = 0;

	/*
	**	Sidebar redraw record.
	*/
	mono->Sub_Window(43, 1, 6, 11);
	mono->Scroll();
	mono->Set_Cursor(0, 10);
	mono->Printf("%5d", SidebarRedraws);
	SidebarRedraws = 0;

	/*
	**	Update the CPU utilization chart.
	*/
	mono->Sub_Window(15, 13, 63, 10);
	mono->Pan(1);
	mono->Sub_Window(15, 13, 64, 10);
	int graph = 0;//RECORDHEIGHT * fixed(TIMER_SECOND-SpareTicks, TIMER_SECOND);
	for (int row = 1; row < RECORDHEIGHT; row += 2) {
		static char _barchar[4] = {(char)' ', (char)220, (char)0, (char)219};
		char str[2];
		int index = 0;

		index |= (graph >= row) ? 0x01 : 0x00;
		index |= (graph >= row+1) ? 0x02: 0x00;

		str[1] = '\0';
		str[0] = _barchar[index];
		mono->Text_Print(str, 62, 9-(row/2));
	}
	mono->Sub_Window();

	SpareTicks = 0;
}
#endif


/***********************************************************************************************
 * LogicClass::AI -- Handles AI logic processing for game objects.                             *
 *                                                                                             *
 *    This routine is used to perform the AI processing for all game objects. This includes    *
 *    all houses, factories, objects, and teams.                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/29/1994 JLB : Created.                                                                 *
 *   12/17/1994 JLB : Must perform one complete pass rather than bailing early.                *
 *   12/23/1994 JLB : Ensures that no object gets skipped if it was deleted.                   *
 *=============================================================================================*/
void LogicClass::AI(void)
{
	int index;

	FramesThisSecond++;

	/*
	**	Handle any general timer trigger events.
	*/
	for (LogicTriggerID = 0; LogicTriggerID < LogicTags.Count(); LogicTriggerID++) {
		TagClass * trig = LogicTags[LogicTriggerID];

		if (Scen->IsCrateBeenPickedUp) {
			if (trig->Spring(TEVENT_PICKUP_CRATE_ANY)) continue;
		}

		/*
		**	Global changed trigger event might be triggered.
		*/
		if (Scen->IsGlobalChanged) {
			if (trig->Spring(TEVENT_GLOBAL_SET)) continue;
			if (trig->Spring(TEVENT_GLOBAL_CLEAR)) continue;
			if (trig->Spring(TEVENT_LOCAL_SET)) continue;
			if (trig->Spring(TEVENT_LOCAL_CLEAR)) continue;
		}

		if (Scen->IsAmbientLightChanged) {
			if (trig->Spring(TEVENT_AMBIENT_LESS_THAN)) continue;
			if (trig->Spring(TEVENT_AMBIENT_GREATER_THAN)) continue;
		}

		/*
		**	General time expire trigger events can be sprung without warning.
		*/
		if (trig->Spring(TEVENT_TIME)) continue;
		if (trig->Spring(TEVENT_RANDOM_TIME)) continue;

		/*
		**	The mission timer expiration trigger event might spring if the timer is active
		**	but at a value of zero.
		*/
		if (Scen->MissionTimer.Is_Active() && Scen->MissionTimer == 0) {
			if (trig->Spring(TEVENT_MISSION_TIMER_EXPIRED)) continue;
		}
	}

	/*
	**	Clean up any status values that were maintained only for logic trigger
	**	purposes.
	*/
	if (Scen->MissionTimer.Is_Active() && Scen->MissionTimer == 0) {
		Scen->MissionTimer.Stop();
		Map.Flag_To_Redraw(GS_REDRAW_ALL);			// Used only to cause tabs to redraw in new state.
	}

	Scen->IsGlobalChanged = false;
	Scen->IsBridgeChanged = false;
	Scen->IsAmbientLightChanged = false;
	Scen->IsCrateBeenPickedUp = false;

	/*
	**	Shadow creeping back over time is handled here.
	*/
	if (Rule->IsShroudGrow && Rule->ShroudRate != 0 && Scen->ShroudTimer == 0) {
		Scen->ShroudTimer = TICKS_PER_MINUTE * Rule->ShroudRate;
		Map.Encroach_Shadow();
	}

	if (Scen->Special.IsFogOfWar && Rule->FogRate != 0 && Scen->FogTimer == 0) {
		Scen->FogTimer = TICKS_PER_MINUTE * Rule->FogRate;
		Map.Encroach_Fog();
	}

	if (Scen->DesiredAmbientLight != Scen->CurrentAmbientLight && Rule->AmbientLightChangeRate != 0 && Scen->AmbientChangeTimer == 0) {
		Scen->AmbientChangeTimer = TICKS_PER_MINUTE * Rule->AmbientLightChangeRate;

		Scen->DesiredAmbientLight = std::max(Scen->DesiredAmbientLight, 0);

		int value = 100.0 * Rule->AmbientLightChangeStep;

		if (Scen->DesiredAmbientLight < Scen->CurrentAmbientLight) {
			Scen->CurrentAmbientLight -= value;
			Scen->CurrentAmbientLight = std::max(Scen->CurrentAmbientLight, Scen->DesiredAmbientLight);
		} else {
			Scen->CurrentAmbientLight += value;
			Scen->CurrentAmbientLight = std::min(Scen->CurrentAmbientLight, Scen->DesiredAmbientLight);
		}
		Scen->IsAmbientLightChanged = true;
		Map.Update_Cell_Colors();
		Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);

	}

	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth && Rule->IceGrowthRate != 0 && Scen->IceGrowthTimer == 0) {
		Scen->IceGrowthTimer = TICKS_PER_MINUTE * Rule->IceGrowthRate;
		Map.DirtyIceCells.Clear();
		if (Map.Ice_Growth_AI()) {
			Map.Recalc_Ice_Cells();
		}
	}

	VeinholeMonsterClass::Update_All();
	TiberiumClass::Tiberium_Growth();
	TiberiumClass::Tiberium_Spread();
	Map.Ice_Solidification_AI();

	DynamicVectorClass<TeamClass *> teams;
	for (index = 0; index < Teams.Count(); index++) {
		teams.Add(Teams[index]);
	}

	/*
	**	Team AI is processed.
	*/
	for (index = 0; index < teams.Count(); index++) {
		teams[index]->AI();
	}

	SpotLightClass::Update_All();
	LaserDrawClass::Update_All();
	IonStormClass::AI();
	LightSourceClass::Process_Lighting(6);
	EMPulseClass::Update_All();
	Map.Terrain_Deformation_AI();

	/*
	**	AI for all sentient objects is processed.
	*/
	for (index = 0; index < Count(); index++) {
		ObjectClass * obj = (*this)[index];

		BStart(BENCH_AI);
		obj->AI();
		BEnd(BENCH_AI);

		/*
		**	If the object was destroyed in the process of performing its AI, then
		**	adjust the index so that no object gets skipped.
		*/
		//if (obj != (*this)[index]) {
		//	index--;
		//}
	}
//	HouseClass::Recalc_Attributes();

	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
		for (index = 0; index < MoveFlashes.Count(); index++) {
			MoveFlashes[index]->AI();
		}
	}

	IonBlastClass::Update_All();
	AlphaShapeClass::Update_All();

	/*
	**	Map related logic is performed.
	*/
	Map.Logic();

	TacticalMap->AI();

	/*
	**	Factory processing is performed.
	*/
	for (index = 0; index < Factories.Count(); index++) {
		Factories[index]->AI();
	}

	/*
	**	House processing is performed.
	*/
	for (HousesType house = HOUSE_FIRST; house < Houses.Count(); house++) {
		HouseClass * hptr = Houses[house];
		if (hptr) {
			hptr->AI();
		}
	}

	if (Map.Object_To_Follow() != NULL) {
		TacticalMap->Set_Tactical_Position(Map.Object_To_Follow()->PositionCoord);
	}
}


/// <summary>
/// Handles the environmental side of the game logic frame.
/// This routine paces shroud regrowth, fog of war spread and ice growth against their
/// rule driven timers, lets the map settle its ice and terrain deformation, and then
/// gives each foot object's locomotor its chance to process.
/// </summary>
void LogicClass::Environment_AI(void)
{
	if (Rule->IsShroudGrow && Rule->ShroudRate != 0 && Scen->ShroudTimer == 0) {
		Scen->ShroudTimer = TICKS_PER_MINUTE * Rule->ShroudRate;
		Map.Encroach_Shadow();
	}

	if (Scen->Special.IsFogOfWar && Rule->FogRate != 0 && Scen->FogTimer == 0) {
		Scen->FogTimer = TICKS_PER_MINUTE * Rule->FogRate;
		Map.Encroach_Fog();
	}

	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth && Rule->IceGrowthRate != 0 && Scen->IceGrowthTimer == 0) {
		Scen->IceGrowthTimer = TICKS_PER_MINUTE * Rule->IceGrowthRate;
		Map.Ice_Growth_AI();
	}

	Map.Ice_Solidification_AI();
	Map.Terrain_Deformation_AI();

	for (int index = 0; index < Count(); index++) {
		FootClass *foot = (FootClass *)(*this)[index];
		if (foot->Is_Foot() && foot->Locomotion != NULL && !foot->IsSinking) {
			foot->Locomotion->Process();
		}
	}
}


/***********************************************************************************************
 * LogicClass::Detach -- Detatch the specified target from the logic system.                   *
 *                                                                                             *
 *    This routine is called when the specified target object is about to be removed from the  *
 *    game system and all references to it must be severed. The only thing that the logic      *
 *    system looks for in this case is to see if the target refers to a trigger and if so,     *
 *    it scans through the trigger list and removes all references to it.                      *
 *                                                                                             *
 * INPUT:   target   -- The target to remove from the sytem.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void LogicClass::Detach(AbstractClass const * target, bool all)
{
	/*
	**	Remove any triggers from the logic trigger list.
	*/
	if (Is_Target_Tag(target)) {
		for (int index = 0; index < LogicTags.Count(); index++) {
			if (target == (AbstractClass *)LogicTags[index]) {
				LogicTags.Delete_Index(index);
				index--;
			}
		}
	}
}


/// <summary>
/// Performs the per object thinking for one game logic frame.
/// This routine gives every object that has been submitted to a layer -- animations,
/// lights, bullets, buildings, aircraft, infantry, units, particle systems, terrain,
/// voxel animations and waves -- its chance to act. It is the companion to
/// LogicClass::AI, which handles the trigger and scenario side of the same frame.
/// </summary>
void LogicClass_AI_Logic_507470(void)
{
	int index;
	for (index = Anims.Count() - 1; index >= 0; index--) {
		AnimClass * anim = Anims[index];
		if (anim->IsSubmittedToLayer) anim->AI();
	}

	for (index = BuildingLights.Count() - 1; index >= 0; index--) {
		BuildingLightClass * blight = BuildingLights[index];
		if (blight->IsSubmittedToLayer) blight->AI();
	}

	for (index = Bullets.Count() - 1; index >= 0; index--) {
		BulletClass * bullet = Bullets[index];
		if (bullet->IsSubmittedToLayer) bullet->AI();
	}

	for (index = Buildings.Count() - 1; index >= 0; index--) {
		BuildingClass * building = Buildings[index];
		if (building->IsSubmittedToLayer) building->AI();
	}

	for (index = Aircraft.Count() - 1; index >= 0; index--) {
		AircraftClass * aircraft = Aircraft[index];
		if (aircraft->IsSubmittedToLayer) aircraft->AI();
	}

	for (index = Infantry.Count() - 1; index >= 0; index--) {
		InfantryClass * infantry = Infantry[index];
		if (infantry->IsSubmittedToLayer) infantry->AI();
	}

	for (index = Units.Count() - 1; index >= 0; index--) {
		UnitClass * unit = Units[index];
		if (unit->IsSubmittedToLayer) unit->AI();
	}

	for (index = ParticleSystems.Count() - 1; index >= 0; index--) {
		ParticleSystemClass * partsys = ParticleSystems[index];
		if (partsys->IsSubmittedToLayer) partsys->AI();
	}

	for (index = Terrains.Count() - 1; index >= 0; index--) {
		TerrainClass * terrain = Terrains[index];
		if (terrain->IsSubmittedToLayer) terrain->AI();
	}

	for (index = VoxelAnims.Count() - 1; index >= 0; index--) {
		VoxelAnimClass * voxelanim = VoxelAnims[index];
		if (voxelanim->IsSubmittedToLayer) voxelanim->AI();
	}

	for (index = Waves.Count() - 1; index >= 0; index--) {
		WaveClass * wave = Waves[index];
		if (wave->IsSubmittedToLayer) wave->AI();
	}
}


/// <summary>
/// Submits the object to the logic system.
/// This routine is used to add an object to the list that the game logic ticks each
/// frame. An object that has already been submitted is accepted without complaint.
/// </summary>
/// <param name="sort">Should the object be inserted in sorted order?</param>
/// <returns>bool; Is the object now submitted to the logic system?</returns>
bool LogicClass::Submit(ObjectClass const * object, bool sort)
{
	if (object->IsSubmittedToLayer) {
		return(true);
	}

	if (BASECLASS::Submit(object, sort)) {
		((ObjectClass *)object)->IsSubmittedToLayer = true;
		return(true);
	}

	return(false);
}


/// <summary>
/// Removes the object from the logic system.
/// This routine is called when an object should no longer be given its chance to think.
/// An object that was never submitted is quietly left alone.
/// </summary>
void LogicClass::Remove(ObjectClass * object)
{
	if (object->IsSubmittedToLayer) {
		Delete(object);
		object->IsSubmittedToLayer = false;
	}
}
