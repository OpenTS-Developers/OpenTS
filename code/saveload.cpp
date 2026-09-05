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

/* $Header: /counterstrike/SAVELOAD.CPP 9     3/17/97 1:04a Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SAVELOAD.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 23, 1994                                              *
 *                                                                                             *
 *                  Last Update : July 8, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Code_All_Pointers -- Code all pointers.                                                   *
 *   Decode_All_Pointers -- Decodes all pointers.                                              *
 *   Get_Savefile_Info -- gets description, scenario #, house                                  *
 *   Load_Game -- loads a saved game                                                           *
 *   Load_MPlayer_Values -- Loads multiplayer-specific values                                  *
 *   Load_Misc_Values -- loads miscellaneous variables                                         *
 *   MPlayer_Save_Message -- pops up a "saving..." message                                     *
 *   Put_All -- Store all save game data to the pipe.                                          *
 *   Reconcile_Players -- Reconciles loaded data with the 'Players' vector                     *
 *   Save_Game -- saves a game to disk                                                         *
 *   Save_MPlayer_Values -- Saves multiplayer-specific values                                  *
 *   Save_Misc_Values -- saves miscellaneous variables                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "saveload.h"

#include "_logic.h"
#include "_map.h"
#include "_rect.h"
#include "_rules.h"
#include "_script.h"
#include "_tactica.h"
#include "_vanim.h"
#include "_warhead.h"
#include "_weapon.h"
#include "aircraft.h"
#include "airctype.h"
#include "aitrig.h"
#include "alphashp.h"
#include "anim.h"
#include "animtype.h"
#include "blight.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "data.h"
#include "dbgprint.h"
#include "empulse.h"
#include "enviro.h"
#include "factory.h"
#include "fog.h"
#include "gamedirs.h"
#include "globals.h"
#include "goptions.h"
#include "houstype.h"
#include "ilinkstm.h"
#include "infantry.h"
#include "infatype.h"
#include "init.h"
#include "ion.h"
#include "language/language.h"
#include "loaddlg.h"
#include "light.h"
#include "logic.h"
#include "overlay.h"
#include "overtype.h"
#include "ovrlight.h"
#include "particle.h"
#include "partsys.h"
#include "psystype.h"
#include "ptype.h"
#include "revent.h"
#include "rules.h"
#include "savemgr.h"
#include "savestream.h"
#include "savever.h"
#include "scenario.h"
#include "script.h"
#include "session.h"
#include "side.h"
#include "sidebar.h"
#include "smudtype.h"
#include "stimer.h"
#include "sun.h"
#include "super.h"
#include "suprtype.h"
#include "swizzle.h"
#include "syncrechook.h"
#include "syncreport.h"
#include "tactical.h"
#include "taction.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforc.h"
#include "team.h"
#include "teamtype.h"
#include "terrain.h"
#include "terrtype.h"
#include "tevent.h"
#include "tiberium.h"
#include "trigger.h"
#include "trigtype.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vanim.h"
#include "vanimtype.h"
#include "vein.h"
#include "vox.h"
#include "warhead.h"
#include "wave.h"
#include "waypoint.h"
#include "weapon.h"

#include "objheaps.hh"

#include <string>

//#define	SAVE_BLOCK_SIZE	512
#define	SAVE_BLOCK_SIZE	4096
//#define	SAVE_BLOCK_SIZE	1024

/*
********************************** Defines **********************************
*/
unsigned int ExpectedGameVersion = LoadOptionsClass::GAMEVER_OPENTS;

_COM_SMARTPTR_TYPEDEF(ILinkStream, __uuidof(ILinkStream));


/// <summary>
/// Loads a vector of persistent objects from the save game stream.
/// This routine reads the element count and then recreates each object through OLE. The
/// objects are not handed back -- each one reattaches itself to its own heap as it is
/// constructed, which is what refills the game's vectors.
/// </summary>
/// <returns>Returns with S_OK, or the failure code of the read that went wrong.</returns>
__forceinline HRESULT Load_Vector(IStream * stream)
{
	int count;
	int index;
	LPVOID obj;

	HRESULT result = stream->Read(&count, sizeof(count), NULL);
	if (FAILED(result)) {
		return(result);
	}
	for (index = 0; index < count; index++) {
		result = OleLoadFromStream(stream, IID_IUnknown, &obj);
		if (FAILED(result)) {
			return(result);
		}
	}
	return(S_OK);
}


/// <summary>
/// Saves a vector of persistent objects to the save game stream.
/// This routine writes the element count and then streams out each object in turn through
/// its IPersistStream interface.
/// </summary>
/// <returns>Returns with S_OK, or the failure code of the first object that refused to
/// save.</returns>
template<class T>
__forceinline HRESULT Save_Vector(IStream * stream, const DynamicVectorClass<T> &list)
{
	int count = list.Count();
	HRESULT result = stream->Write(&count, sizeof(count), NULL);
	if (SUCCEEDED(result)) {
		for (int index = 0; index < count; index++) {
			LPPERSISTSTREAM lpPS = NULL;
			result = list[index]->QueryInterface(IID_IPersistStream, (LPVOID *)&lpPS);
			if (FAILED(result)) {
				return(result);
			}
			result = OleSaveToStream(lpPS, stream);
			if (FAILED(result)) {
				return(result);
			}
			result = lpPS->Release();
			if (FAILED(result)) {
				return(result);
			}
		}
		result = S_OK;
	}
	return(result);
}
/// <summary>
/// Builds a checksum over the whole of the game object state.
/// This routine walks the scenario and every object and type heap, folding each one's own
/// contribution into a single engine. It is used to compare the state held by two
/// machines in a networked game, so that a desynchronization can be spotted.
/// </summary>
/// <returns>Returns with the checksum engine holding the accumulated state.</returns>
CRCEngine Object_CRCs(void)
{
	int i;
	CRCEngine crc;
	Scen->Compute_CRC(crc);

#define DO_OBJ_CRC(VECTOR) \
	for (i = 0; i < VECTOR.Count(); i++) { \
		VECTOR[i]->Compute_CRC(crc); \
	}

	OBJECT_HEAP_LIST(DO_OBJ_CRC)

#undef DO_OBJ_CRC

	if (PlayerPtr != NULL) {
		crc(PlayerPtr->HeapID);
	}
	crc((int)Frame);
	crc(CurrentObject.Count());
	return(crc);
}


/// <summary>
/// Writes a per-heap checksum table to the out-of-sync report: one summary line per heap, then
/// a row per live object keyed by its stable identifier. Unlike Object_CRCs this folds in no
/// per-machine state, so two peers' tables are directly comparable.
/// </summary>
void Print_Heap_CRCs(FILE * fp)
{
	int i;

	fprintf(fp, "\n----- Heap checksums -----\n");

#define HEAP_SUMMARY(VECTOR) \
	{ \
		CRCEngine heap_crc; \
		for (i = 0; i < VECTOR.Count(); i++) { \
			VECTOR[i]->Compute_CRC(heap_crc); \
		} \
		fprintf(fp, "%-28s count=%-5d crc=%08x\n", #VECTOR, VECTOR.Count(), heap_crc()); \
	}

	OBJECT_HEAP_LIST(HEAP_SUMMARY)

#undef HEAP_SUMMARY

#define HEAP_ROWS(VECTOR) \
	{ \
		CRCEngine heap_crc; \
		fprintf(fp, "\n--- %s ---\n", #VECTOR); \
		for (i = 0; i < VECTOR.Count(); i++) { \
			VECTOR[i]->Compute_CRC(heap_crc); \
			fprintf(fp, "%05d  ID:%-8d  %08x\n", i, VECTOR[i]->Fetch_ID(), heap_crc()); \
		} \
	}

	OBJECT_HEAP_LIST_INSTANCES(HEAP_ROWS)

#undef HEAP_ROWS
}

/***********************************************************************************************
 * Put_All -- Store all save game data to the pipe.                                            *
 *                                                                                             *
 *    This is the bulk processor of the game related save game data. All the game object       *
 *    and state data is stored to the pipe specified.                                          *
 *                                                                                             *
 * INPUT:   pipe  -- Reference to the pipe that will receive the save game data.               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool Put_All(IStream *stream, int save_net)
{
	/*
	**	Save the scenario global information.
	*/
	Scen->Save(stream);
	Environment.Save(stream);
	Rule->Save(stream);

	DebugString("Saving AnimTypes\n");
	if (FAILED(Save_Vector(stream, AnimTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	/*
	**	Save the map.  The map must be saved first, since it saves the Theater.
	*/
	DebugString("Saving Map\n");
	if (FAILED(Map.Save(stream))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	DebugString("Saving Tunnels\n");
	if (FAILED(Save_Vector(stream, Tubes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	/*
	**	Save miscellaneous variables.
	*/
	DebugString("Saving Misc. Values\n");
	if (FAILED(Save_Misc_Values(stream))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	/*
	**	Save the Logic & Map layers
	*/
	DebugString("Saving Logic\n");
	if (FAILED(Logic.Save(stream))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	DebugString("Saving TacticalMap\n");
	if (FAILED(OleSaveToStream(TacticalMap, stream))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	/*
	**	Save all game objects.  This code saves every object that's stored in a
	**	TFixedIHeap class.
	*/
	DebugString("Saving HouseTypes\n");
	if (FAILED(Save_Vector(stream, HouseTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Houses\n");
	if (FAILED(Save_Vector(stream, Houses))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Units\n");
	if (FAILED(Save_Vector(stream, Units))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving UnitTypes\n");
	if (FAILED(Save_Vector(stream, UnitTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving InfantryTypes\n");
	if (FAILED(Save_Vector(stream, InfantryTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Infantry\n");
	if (FAILED(Save_Vector(stream, Infantry))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving BuildingTypes\n");
	if (FAILED(Save_Vector(stream, BuildingTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Buildings\n");
	if (FAILED(Save_Vector(stream, Buildings))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving AircraftTypes\n");
	if (FAILED(Save_Vector(stream, AircraftTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Aircraft\n");
	if (FAILED(Save_Vector(stream, Aircraft))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Anims\n");
	if (FAILED(Save_Vector(stream, Anims))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving TaskForces\n");
	if (FAILED(Save_Vector(stream, TaskForces))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving TeamTypes\n");
	if (FAILED(Save_Vector(stream, TeamTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Teams\n");
	if (FAILED(Save_Vector(stream, Teams))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving ScriptTypes\n");
	if (FAILED(Save_Vector(stream, ScriptTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Scripts\n");
	if (FAILED(Save_Vector(stream, Scripts))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving TagTypes\n");
	if (FAILED(Save_Vector(stream, TagTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Tags\n");
	if (FAILED(Save_Vector(stream, Tags))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving TriggerTypes\n");
	if (FAILED(Save_Vector(stream, TriggerTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Triggers\n");
	if (FAILED(Save_Vector(stream, Triggers))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving AITriggerTypes\n");
	if (FAILED(Save_Vector(stream, AITriggerTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	DebugString("Saving Actions\n");
	if (FAILED(Save_Vector(stream, Actions))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Events\n");
	if (FAILED(Save_Vector(stream, Events))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Factories\n");
	if (FAILED(Save_Vector(stream, Factories))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving VoxelAnimTypes\n");
	if (FAILED(Save_Vector(stream, VoxelAnimTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving VoxelAnims\n");
	if (FAILED(Save_Vector(stream, VoxelAnims))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Warheads\n");
	if (FAILED(Save_Vector(stream, Warheads))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Weapons\n");
	if (FAILED(Save_Vector(stream, Weapons))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving ParticleTypes\n");
	if (FAILED(Save_Vector(stream, ParticleTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Particles\n");
	if (FAILED(Save_Vector(stream, Particles))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving ParticleSystemTypes\n");
	if (FAILED(Save_Vector(stream, ParticleSystemTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving ParticleSystems\n");
	if (FAILED(Save_Vector(stream, ParticleSystems))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving BulletTypes\n");
	if (FAILED(Save_Vector(stream, BulletTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Bullets\n");
	if (FAILED(Save_Vector(stream, Bullets))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving WaypointPaths\n");
	if (FAILED(Save_Vector(stream, WaypointPaths))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving SmudgeTypes\n");
	if (FAILED(Save_Vector(stream, SmudgeTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving OverlayTypes\n");
	if (FAILED(Save_Vector(stream, OverlayTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving LightSources\n");
	if (FAILED(Save_Vector(stream, LightSources))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving BuildingLights\n");
	if (FAILED(Save_Vector(stream, BuildingLights))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Sides\n");
	if (FAILED(Save_Vector(stream, Sides))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Tiberiums\n");
	if (FAILED(Save_Vector(stream, Tiberiums))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Empulses\n");
	if (FAILED(Save_Vector(stream, EMPulseClass::EMPulses))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving SuperWeaponTypes\n");
	if (FAILED(Save_Vector(stream, SuperWeaponTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving SuperWeapons\n");
	if (FAILED(Save_Vector(stream, SuperWeapons))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving TerrianTypes\n");
	if (FAILED(Save_Vector(stream, TerrainTypes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Terrains\n");
	if (FAILED(Save_Vector(stream, Terrains))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving FoggedObjects\n");
	if (FAILED(Save_Vector(stream, FoggedObjectClass::FoggyObjects))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving AlphaShapes\n");
	if (FAILED(Save_Vector(stream, AlphaShapes))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving Waves\n");
	if (FAILED(Save_Vector(stream, Waves))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving VeinholeMonster\n");
	if (!VeinholeMonsterClass::Save_All(stream)) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	DebugString("Saving RadarEvents\n");
	if (!RadarEventClass::Save(stream)) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	/*
	 * A campaign takes its options from the mission. Every other kind is given them at setup, so
	 * the save is the only place a resume can find them.
	 */
	if (Session.Type != GAME_NORMAL) {
		DebugString("Writing Session.Options\n");
		if (!Session.Options.Save(stream)) {
			DebugString("\t***** FAILED!\n");
			return(false);
		}
	}

	return(true);
}


/// <summary>
/// Restores all of the save game data from the stream.
/// This routine is the counterpart of Put_All. It tears down the current scenario, puts
/// back the addon, theater and rules that the game was saved under, rebuilds the display
/// surfaces to suit the saved options, and then recreates every object heap in the same
/// order they were written out.
/// </summary>
/// <returns>bool; Was the game state restored?</returns>
static bool Get_All(IStream *stream, bool save_net)
{
	Clear_Scenario();
	Scen->Load(stream);
	Disable_Addon(ADDON_ANY);
	Set_Required_Addon(Scen->RequiredAddOn);
	if (!Addon_Installed(Scen->RequiredAddOn)) {
		return(false);
	}
	Enable_Addon(Scen->RequiredAddOn);

	if (!Prep_For_Side(Scen->IsGDI ? SIDE_GDI : SIDE_NOD)) {
		return(false);
	}

	Rect temp = VisibleRect;
	temp.X = ((Options.IsSidebarOnRight || Debug_Map) ? 0 : SidebarClass::SIDE_WIDTH);
	temp.Y = 16;
	temp.Width -= SidebarClass::SIDE_WIDTH;
	temp.Height -= 16;

	Allocate_Surfaces(VisibleRect, Rect(0, 0, temp.Width, VisibleRect.Height), Rect(0, 0, temp.Width, VisibleRect.Height), Rect(0, 0, SidebarClass::SIDE_WIDTH, VisibleRect.Height));

	Map.Set_View_Dimensions(temp);

	Environment.Load(stream);

	Init_Theater(Scen->Theater);

	RulesClass::Load_Art_INI();

	if (Addon_Enabled(ADDON_FIRESTORM) == true) {
		CCFileClass artfs("ARTFS.INI");
		if (artfs.Is_Available() == true) {
			ArtINI.Load(artfs, false);
		}
	}

	Rule->Load(stream);

	if (Scen->SpeechSide != SIDE_NONE) {
		if (!Prep_Speech_For_Side(Scen->SpeechSide)) {
			return(false);
		}
	} else {
		if (!Prep_Speech_For_Side(Scen->IsGDI ? SIDE_GDI : SIDE_NOD)) {
			return(false);
		}
	}

	if (FAILED(Load_Vector(stream))) {	/// AnimTypes
		return(false);
	}

	Map.Load(stream);

	if (FAILED(Load_Vector(stream))) {	/// Tubes
		return(false);
	}

	if (FAILED(Load_Misc_Values(stream))) {
		return(false);
	}

	Map.Reset_All_Subzones();
	Logic.Load(stream);

	if (TacticalMap != NULL) {
		delete TacticalMap;
		TacticalMap = NULL;
	}
	Tactical * old_tactical;
	if (FAILED(OleLoadFromStream(stream, IID_IUnknown, (LPVOID *)&old_tactical))) {
		return(false);
	}

	if (FAILED(Load_Vector(stream))) {	/// HouseTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Houses
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Units
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// UnitTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// InfantryTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Infantry
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// BuildingTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Buildings
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// AircraftTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Aircraft
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Anims
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// TaskForces
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// TeamTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Teams
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// ScriptTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Scripts
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// TagTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Tags
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// TriggerTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Triggers
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// AITriggerTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Actions
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Events
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Factories
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// VoxelAnimTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// VoxelAnims
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Warheads
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Weapons
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// ParticleTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Particles
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// ParticleSystemTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// ParticleSystems
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// BulletTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Bullets
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// WaypointPaths
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// SmudgeTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// OverlayTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// LightSources
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// BuildingLights
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Sides
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Tiberiums
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// EMPulseClass::EMPulses
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// SuperWeaponTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// SuperWeapons
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// TerrainTypes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Terrains
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// FoggedObjectClass::FoggyObjects
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// AlphaShapes
		return(false);
	}
	if (FAILED(Load_Vector(stream))) {	/// Waves
		return(false);
	}
	if (!VeinholeMonsterClass::Load_All(stream)) {
		return(false);
	}
	if (!RadarEventClass::Load(stream)) {
		return(false);
	}

	if (Session.Type != GAME_NORMAL) {
		DebugString("Reading Session.Options\n");
		if (!Session.Options.Load(stream)) {
			DebugString("\t***** FAILED!\n");
			return(false);
		}
	}

	Map.Flag_To_Redraw(GS_REDRAW_ALL);

	return(true);
}

/***************************************************************************
 * Save_Game -- saves a game to disk                                       *
 *                                                                         *
 * Saving the Map:                                                         *
 *     DisplayClass::Save() invokes CellClass's Write() for every cell     *
 *     that needs to be saved.  A cell needs to be saved if it contains    *
 *     any special data at all, such as a TIcon, or an Occupier.           *
 *   The cell saves its own CellTrigger pointer, converted to a TARGET.    *
 *                                                                         *
 * Saving game objects:                                                    *
 *   - Any object stored in an ArrayOf class needs to be saved.  The ArrayOf*
 *     Save() routine invokes each object's Write() routine, if that       *
 *     object's IsActive is set.                                           *
 *                                                                         *
 * Saving the layers:                                                      *
 *   The Map's Layers (Ground, Air, etc) of things that are on the map,    *
 *     and the Logic's Layer of things to process both need to be saved.   *
 *     LayerClass::Save() writes the entire layer array to disk            *
 *                                                                         *
 * Saving the houses:                                                      *
 *   Each house needs to be saved, to record its Credits, Power, etc.      *
 *                                                                         *
 * Saving miscellaneous data:                                              *
 *   There are a lot of miscellaneous variables to save, such as the       *
 *     map's dimensions, the player's house, etc.                          *
 *                                                                         *
 * INPUT:                                                                  *
 *      id      numerical ID, for the file extension                       *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = OK, false = error                                           *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/28/1994 BR : Created.                                              *
 *   02/27/1996 JLB : Uses simpler game control value save operation.      *
 *=========================================================================*/
bool Save_Game(const char *file_name, char const * descr)
{
	WCHAR name[MAX_PATH];

	DebugString("\nSAVING GAME [%s - %s]\n", file_name, descr);

	MultiByteToWideChar(0,0, Saved_Game_Name(file_name).c_str(), -1, name, sizeof(name)/sizeof(WCHAR));

	/*
	**	Open the file
	*/
	DebugString("Creating DocFile\n");
	IStoragePtr storage;
	if (FAILED(StgCreateDocfile(name, STGM_CREATE|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &storage))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}


	/*
	**	Save the description, scenario #, and house
	**	(scenario # & house are saved separately from the actual Scenario &
	**	PlayerPtr globals for convenience; we can quickly find out which
	**	house & scenario this save-game file is for by reading these values.
	**	Also, PlayerPtr is stored in a coded form in Save_Misc_Values(),
	**	which may or may not be a HousesType number; so, saving 'house'
	**	here ensures we can always pull out the house for this file.)
	*/
	SaveVersionInfo info;
	info.Set_Internal_Version(ExpectedGameVersion);
	info.Set_Scenario_Description(descr);
	info.Set_Version(1);
	info.Set_Player_House(PlayerPtr->Class->GivenName);
	info.Set_Campaign_Number(Scen->Campaign);
	info.Set_Scenario_Number(Scen->Scenario);
	info.Set_Executable_Name("SUN.EXE");
	info.Set_Game_Type(Session.Type);
	FILETIME FileTime;
	CoFileTimeNow(&FileTime);
	info.Set_Last_Time(FileTime);
	info.Set_Start_Time(FileTime);
	info.Set_Play_Time(FileTime);

	/*
	**	Save the save-game version, for loading verification
	*/
	DebugString("Saving version information\n");
	if (FAILED(info.Save(storage))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	DebugString("Creating content stream\n");
	IStreamPtr content;
	if (FAILED(storage->CreateStream(L"CONTENTS", STGM_CREATE|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, 0, &content))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	DebugString("Linking content stream to compressor\n");
	ILinkStreamPtr link;
	link.CreateInstance(CLSID_CompressStream, NULL, CLSCTX_INPROC|CLSCTX_LOCAL_SERVER);
	if (FAILED(link->Link_Stream(content))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}
	IStreamPtr stream(link);

	/*
	**	Dump the save game data to the file. The data is compressed
	**	and then encrypted. The message digest is calculated in the
	**	process by using the data just as it is written to disk.
	*/
	DebugString("Calling Put_All()\n");
	bool res = Put_All(stream,0);

	DebugString("Unlinking content stream from compressor\n");
	if (FAILED(link->Unlink_Stream(NULL))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	DebugString("Releasing content stream\n");
	content.Release();

	DebugString("Closing DocFile\n");
	if (FAILED(storage->Commit(0))) {
		DebugString("\t***** FAILED!\n");
		return(false);
	}

	DebugString("SAVING GAME [%s - %s] - Complete\n\n", file_name, descr);

	if (res) {
		SaveManager.Autosave.Schedule(Frame);
	}

	return(res);
}


/***************************************************************************
 * Load_Game -- loads a saved game                                         *
 *                                                                         *
 * This routine loads the data in the same way it was saved out.           *
 *                                                                         *
 * Loading the Map:                                                        *
 *   - DisplayClass::Load() invokes CellClass's Load() for every cell      *
 *     that was saved.                                                     *
 * - The cell loads its own CellTrigger pointer.                           *
 *                                                                         *
 * Loading game objects:                                                   *
 * - IHeap's Load() routine loads the # of objects stored, and loads       *
 *   each object.                                                          *
 * - Triggers: Add themselves to the HouseTriggers if they're associated   *
 *   with a house                                                          *
 *                                                                         *
 * Loading the layers:                                                     *
 *     LayerClass::Load() reads the entire layer array to disk             *
 *                                                                         *
 * Loading the houses:                                                     *
 *   Each house is loaded in its entirety.                                 *
 *                                                                         *
 * Loading miscellaneous data:                                             *
 *   There are a lot of miscellaneous variables to load, such as the       *
 *     map's dimensions, the player's house, etc.                          *
 *                                                                         *
 * INPUT:                                                                  *
 *      id         numerical ID, for the file extension                    *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = OK, false = error                                           *
 *                                                                         *
 * WARNINGS:                                                               *
 *      If this routine returns false, the entire game will be in an       *
 *      unknown state, so the scenario will have to be re-initialized.     *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/28/1994 BR : Created.                                              *
 *   1/20/97  V.Grippi Added expansion CD check                            *
 *=========================================================================*/
bool Load_Game(const char *file_name)
{
	WCHAR name[MAX_PATH];

	DebugString("\nLOADING GAME [%s]\n", file_name);

	/*
	**	Read & discard the save-game's header info
	*/
	SaveVersionInfo info;
	if (!Get_Savefile_Info(file_name, &info)) {
		return(false);
	}

	/*
	 * The load dialog screens the saves it lists, but a network save reaches this routine
	 * without passing through it, so the stamp is checked here as well.
	 */
	if (info.Get_Internal_Version() != ExpectedGameVersion) {
		return(false);
	}
	LoadedSaveVersion = info.Get_Internal_Version();

	Session.Type = (GameType)info.Get_Game_Type();
	Swizzler.Discard();

	/*
	**	Open the file
	*/
	IStoragePtr storage;

	// Structured storage goes straight to Windows, so the saved game is named in full first.
	MultiByteToWideChar(0,0,Saved_Game_Name(file_name).c_str(), -1, name, (sizeof(name)/sizeof(WCHAR)));

	if (FAILED(StgOpenStorage(name, 0, STGM_SHARE_DENY_WRITE, 0, 0, &storage))) {
		return(false);
	}

	IStreamPtr content;
	if (FAILED(storage->OpenStream(L"CONTENTS", 0, STGM_SHARE_EXCLUSIVE, 0, &content))) {
		return(false);
	}

	IUnknown *pUnknown = NULL;
	ILinkStreamPtr link;
	link.CreateInstance(CLSID_CompressStream, pUnknown,CLSCTX_INPROC|CLSCTX_LOCAL_SERVER);
	if (FAILED(link->Link_Stream(content))) {
		return(false);
	}
	IStreamPtr stream(link);

	bool res = Get_All(stream, false);

	link->Unlink_Stream(NULL);

	if (!res) {
		return(false);
	}

	Swizzler.Resolve();

	/*
	**	Fixup any expediency data that can be inferred from the physical
	**	data loaded.
	*/
	Post_Load_Game();

	// The next mission of a resumed campaign is played at the pair the save carries.
	Session.CampaignDifficulty = Scen->Difficulty;
	Session.CampaignCDifficulty = Scen->CDifficulty;

	Map.Init_IO();
	Map.Activate(1);
	Map.Reposition_Sidebar();
	TiberiumClass::Init_Tiberium_Growth_System();
	TiberiumClass::Init_Tiberium_Spread_System();
	Map.Complete_Radar_Refresh();
	ScenarioActive = true;
	TacticalActive = true;
	Sync_Recorder_Arm();
	Sync_Report_Reset();
	SaveManager.Autosave.Schedule(Frame);
	DebugString("LOADING GAME [%s] - Complete\n\n", file_name);
	return(true);
}


/***************************************************************************
 * Save_Misc_Values -- saves miscellaneous variables                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      file      file to use for writing                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = success, false = failure                                    *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/29/1994 BR : Created.                                              *
 *   03/12/1996 JLB : Simplified.                                          *
 *=========================================================================*/
static void Serialize_Misc_Values(SaveStreamClass & stream)
{
	stream.Serialize(GasSystem);
	stream.Serialize(PlayerPtr);
	stream.Serialize(Frame);
	stream.Serialize(CurrentObject);
	stream.Serialize(Ground);

	IonStormClass::Serialize(stream);

	stream.Serialize(LogicTags);
	stream.Serialize(MapTags);
	stream.Serialize(CrateShares);
	stream.Serialize(CrateAnims);
	stream.Serialize(CrateData);
	stream.Serialize(MissionControl);
	stream.Serialize(Session.ObiWan);
	stream.Serialize(Session.AIOnly);

	/*
	 * Speech is reached through a pair of accessors rather than a variable of its own,
	 * so it travels through a local either way.
	 */
	int state = Get_Speech_State();
	stream.Serialize(state);
	if (stream.Is_Loading()) {
		Set_Speech_State(state != 0);
	}

	// The ring positions travel with every save, so a load continues where the save left off.
	int campaign_slot = SaveManager.Autosave.Campaign_Slot();
	int skirmish_slot = SaveManager.Autosave.Skirmish_Slot();
	stream.Serialize(campaign_slot);
	stream.Serialize(skirmish_slot);
	if (stream.Is_Loading()) {
		SaveManager.Autosave.Seed_Slots(campaign_slot, skirmish_slot);
	}
}


int Save_Misc_Values(IStream * stream)
{
	SaveStreamClass savestream(stream, SaveStreamClass::MODE_SAVE);
	Serialize_Misc_Values(savestream);
	return(savestream.Result());
}


/***********************************************************************************************
 * Load_Misc_Values -- Loads miscellaneous variables.                                          *
 *                                                                                             *
 * INPUT:   file  -- The file to load the misc values from.                                    *
 *                                                                                             *
 * OUTPUT:  Was the misc load process successful?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 BRR : Created.                                                                 *
 *   03/12/1996 JLB : Simplified.                                                              *
 *=============================================================================================*/
int Load_Misc_Values(IStream * stream)
{
	SaveStreamClass savestream(stream, SaveStreamClass::MODE_LOAD);
	savestream.Set_Context("Load_Misc_Values");
	Serialize_Misc_Values(savestream);
	return(savestream.Result());
}


/***************************************************************************
 * Get_Savefile_Info -- gets description, scenario #, house                *
 *                                                                         *
 * INPUT:                                                                  *
 *      id         numerical ID, for the file extension                    *
 *      buf      buffer to store description in                            *
 *      scenp      ptr to variable to hold scenario                        *
 *      housep   ptr to variable to hold house                             *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = OK, false = error (save-game file invalid)                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/12/1995 BR : Created.                                              *
 *=========================================================================*/
bool Get_Savefile_Info(char const * name, SaveVersionInfo * info)
{
	IStoragePtr storage;
	WCHAR wname[MAX_PATH];

	// Structured storage goes straight to Windows, so the saved game is named in full first.
	MultiByteToWideChar(0, 0, Saved_Game_Name(name).c_str(), -1, wname, sizeof(wname) / sizeof(WCHAR));

	HRESULT result = StgOpenStorage(wname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &storage);
	if (FAILED(result)) {
		return(false);
	}

	result = info->Load(storage);
	if (FAILED(result)) {
		return(false);
	}

	return(true);
}


/// <summary>
/// Gives each seated player the restored house carrying its name, so the connections formed
/// afterwards reach the right houses.
/// </summary>
/// <returns>bool; Do the seats and the saved houses agree?</returns>
bool Reconcile_Players(void)
{
	if (Session.Players.Count() == 0) {
		return(true);
	}

	for (int i = 0; i < Session.Players.Count(); i++) {
		HouseClass * found = NULL;

		for (int house = 0; house < Houses.Count(); house++) {
			if (Houses[house]->IsHuman && stricmp(Session.Players[i]->Name, Houses[house]->IniName) == 0) {
				found = Houses[house];
				break;
			}
		}

		if (found == NULL || found->IsObserver != Session.Players[i]->Player.IsObserver) {
			return(false);
		}

		Session.Players[i]->Player.ID = found->HeapID;
	}

	// The first seat is this machine, and PlayerPtr the house that wrote the save.
	if (Houses[Session.Players[0]->Player.ID] != PlayerPtr) {
		return(false);
	}

	for (int house = 0; house < Houses.Count(); house++) {
		HouseClass * housep = Houses[house];
		if (!housep->IsHuman) {
			continue;
		}

		bool seated = false;
		for (int i = 0; i < Session.Players.Count(); i++) {
			if (Session.Players[i]->Player.ID == housep->HeapID) {
				seated = true;
				break;
			}
		}

		// A player who did not return leaves their house fighting on under the computer. An
		// observer's house has nothing to hand over.
		if (!seated && !housep->IsObserver) {
			housep->IsHuman = false;
			housep->IsStarted = true;
			housep->IQ = Rule->MaxIQ;
		}
	}

	return(true);
}


/***************************************************************************
 * MPlayer_Save_Message -- pops up a "saving..." message                   *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/30/1995 BRR : Created.                                             *
 *=========================================================================*/
void MPlayer_Save_Message(void)
{
	//char *txt = Text_String(
}
