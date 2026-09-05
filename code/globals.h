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

#pragma once

#include "coord.h"
#include "ftimer.h"

#include "action.hh"
#include "anim.hh"
#include "armor.hh"
#include "crate.hh"
#include "ground.hh"
#include "house.hh"
#include "land.hh"
#include "mission.hh"
#include "source.hh"
#include "special.hh"
#include "speed.hh"
#include "theater.hh"
#include "vox.hh"

#include <deque>

/*
**	Forward declarations.
*/
template<class T> class DynamicVectorClass;
template<class I, class T> class IndexClass;
template<class T> class CDTimerClass;
class AITriggerTypeClass;
class AbstractTypeClass;
class AircraftClass;
class AircraftTypeClass;
class AnimClass;
class AnimTypeClass;
class BuildingClass;
class BuildingLightClass;
class BuildingTypeClass;
class BulletClass;
class BulletTypeClass;
class CampaignClass;
class FactoryClass;
class HouseClass;
class HouseTypeClass;
class InfantryClass;
class InfantryTypeClass;
class IsometricTileTypeClass;
class ObjectClass;
class OverlayClass;
class OverlayTypeClass;
class ParticleClass;
class ParticleSystemClass;
class ParticleSystemTypeClass;
class ParticleTypeClass;
class ScriptTypeClass;
class SideClass;
class SmudgeClass;
class SmudgeTypeClass;
class SuperClass;
class SuperWeaponTypeClass;
class TaskForceClass;
class TeamClass;
class TeamTypeClass;
class TechnoClass;
class TechnoTypeClass;
class TerrainClass;
class TerrainTypeClass;
class TriggerClass;
class TriggerTypeClass;
class TubeClass;
class UnitClass;
class UnitTypeClass;
class VoxelAnimTypeClass;
class WaveClass;
class MissionControlClass;
class RulesClass;
class TagClass;
class ScenarioClass;

class RandomStraw;
class VersionClass;
class GameOptionsClass;
class ThemeClass;
class SessionClass;
class IPXManagerClass;
class Buffer;
class SpecialClass;
class EventClass;
class FrameTimerClass;

extern bool ScenarioActive;
extern bool Debug_ForceScenario;
extern bool Debug_Create_Maps;
extern bool Debug_SpeedBuild;
extern bool Debug_Inert;
extern bool Debug_MotionCapture;
extern bool Debug_Quiet;
extern bool Debug_Cheat;
extern bool Debug_Remap;
extern bool Debug_Flag;
extern bool Debug_Lose;
extern bool Debug_Map;
extern bool Debug_Win;
extern bool Debug_Icon;
extern bool Debug_Passable;
extern bool Debug_Unshroud;
extern bool Debug_Threat;
extern bool Debug_Find_Path;
extern bool Debug_Check_Map;
extern bool Debug_Playtest;
extern bool Debug_Trap_Check_Heap;
extern bool Debug_Print_Events;
extern bool Debug_Console;

extern ParticleSystemClass *GasSystem;

extern int NewINIFormat;

/*
**	Dynamic global variables (these change or are initialized at run time).
*/
extern MissionControlClass			MissionControl[MISSION_COUNT];
extern IndexClass<int, char *> 		TutorialText;
extern int							MapTriggerID;
extern int							LogicTriggerID;
extern RandomStraw					CryptRandom;
extern ScenarioClass *				Scen;
extern VersionClass					VerNum;
extern bool							ScoresPresent;
extern bool							DrawShapeShadows;
extern int							TournamentTime;
extern bool							AllowVoice;
extern bool							PlayerWins;
extern bool							PlayerLoses;
extern bool							PlayerRestarts;
extern bool							PlayerAborts;
extern int							Frame;
extern GameOptionsClass 			Options;
extern ThemeClass 					Theme;
extern SpecialClass 				Special;

/*
**	Game object allocation and tracking classes.
*/
extern DynamicVectorClass<AITriggerTypeClass *>			AITriggerTypes;
extern DynamicVectorClass<TriggerTypeClass *>			TriggerTypes;
extern DynamicVectorClass<TriggerClass *>				Triggers;
extern DynamicVectorClass<BuildingLightClass *>			BuildingLights;
extern DynamicVectorClass<ParticleSystemClass *>		ParticleSystems;
extern DynamicVectorClass<TubeClass *>					Tubes;
extern DynamicVectorClass<WaveClass *>					Waves;
extern DynamicVectorClass<BulletClass *>				Bullets;
extern DynamicVectorClass<OverlayClass *>				Overlays;
extern DynamicVectorClass<ParticleClass *>				Particles;
extern DynamicVectorClass<SmudgeClass *>				Smudges;
extern DynamicVectorClass<TerrainClass *>				Terrains;
extern DynamicVectorClass<InfantryClass *>				Infantry;
extern DynamicVectorClass<BuildingClass *>				Buildings;
extern DynamicVectorClass<AircraftClass *>				Aircraft;
extern DynamicVectorClass<UnitClass *>					Units;
extern DynamicVectorClass<HouseClass *>					Houses;
extern DynamicVectorClass<AnimClass *>					Anims;
extern DynamicVectorClass<TeamClass *>					Teams;
extern DynamicVectorClass<FactoryClass *>				Factories;
extern DynamicVectorClass<CampaignClass *>				Campaigns;
extern DynamicVectorClass<SideClass *>					Sides;
extern DynamicVectorClass<HouseTypeClass *>				HouseTypes;
extern DynamicVectorClass<TaskForceClass *>				TaskForces;
extern DynamicVectorClass<TeamTypeClass *>				TeamTypes;
extern DynamicVectorClass<ScriptTypeClass *>			ScriptTypes;
extern DynamicVectorClass<OverlayTypeClass *>			OverlayTypes;
extern DynamicVectorClass<SmudgeTypeClass *>			SmudgeTypes;
extern DynamicVectorClass<TerrainTypeClass *>			TerrainTypes;
extern DynamicVectorClass<BuildingTypeClass *>			BuildingTypes;
extern DynamicVectorClass<AircraftTypeClass *>			AircraftTypes;
extern DynamicVectorClass<UnitTypeClass *>				UnitTypes;
extern DynamicVectorClass<AnimTypeClass *>				AnimTypes;
extern DynamicVectorClass<InfantryTypeClass *>			InfantryTypes;
extern DynamicVectorClass<BulletTypeClass *>			BulletTypes;
extern DynamicVectorClass<VoxelAnimTypeClass *>			VoxelAnimTypes;
extern DynamicVectorClass<IsometricTileTypeClass *>		IsometricTileTypes;
extern DynamicVectorClass<ParticleTypeClass *>			ParticleTypes;
extern DynamicVectorClass<ParticleSystemTypeClass *>	ParticleSystemTypes;
extern DynamicVectorClass<SuperWeaponTypeClass *>		SuperWeaponTypes;
extern DynamicVectorClass<SuperClass *>					SuperWeapons;
extern DynamicVectorClass<TechnoClass *>				Technos;
extern DynamicVectorClass<TechnoTypeClass *>			TechnoTypes;
extern DynamicVectorClass<ObjectClass *>				Objects;
extern DynamicVectorClass<AbstractTypeClass *>			AbstractTypes;

extern DynamicVectorClass<AnimClass *>					MoveFlashes;

extern std::deque<EventClass>							OutList;
extern std::deque<EventClass>							DoList;

extern DynamicVectorClass<ObjectClass *>				CurrentObject;
extern DynamicVectorClass<TagClass *>					LogicTags;
extern DynamicVectorClass<TagClass *>					MapTags;

/*
**	Miscellaneous globals.
*/
extern HousesType					Whom;

//extern _VQAConfig					AnimControl;
extern int							SpareTicks;
extern int							PathCount;
extern int							CellCount;
extern int							TargetScan;
extern int							SidebarRedraws;
//extern DMonoType					MonoPage;
extern bool							GameActive;
extern bool							SpecialFlag;
extern int							ScenarioInit;
extern HouseClass *					PlayerPtr;

extern int							BuildLevel;
extern unsigned int				ScenarioCRC;

#ifdef _DEBUG
extern Cell 						CurrentCell;
#endif

extern SessionClass				Session;
extern IPXManagerClass			Ipx;

#if (TIMING_FIX)
extern int										NewMaxAheadFrame1;
extern int										NewMaxAheadFrame2;
#endif

extern bool VisceroidsAsSnoBees;
extern bool Just4Fun;

extern int 							_dialog_count;
extern int							Seed;
extern int							CustomSeed;
extern bool							IgnoreInput;
extern bool							drag_select_aborted;

extern GroundType  				Ground[LAND_COUNT];

extern int LParam;

/*
**	Constant externs (data is not modified during game play).
*/
extern char const *							LandName[LAND_COUNT];
extern char const *							SpeedName[SPEED_COUNT];
extern double								CrateData[CRATE_COUNT];
extern char const * const					CrateNames[CRATE_COUNT];
extern int									CrateShares[CRATE_COUNT];
extern AnimType								CrateAnims[CRATE_COUNT];
extern char const * const					ArmorName[ARMOR_COUNT];
extern char const * const					ActionName[ACTION_COUNT];
extern Coord const 							StoppingCoordAbs[5];

extern SpecialDialogType	SpecialDialog;

extern int LogLevel;
extern unsigned int LogLevelTime[ MAX_LOG_LEVEL ];
extern unsigned int LogLastTime;

extern TheaterType LastTheater;

extern bool _special_dialog_flag;

/*
 * These used to be defined in the header, polluting every module. Moved to globals.cpp to not do that.
 */
extern const double ONE_RAD;
extern const double RAD_45;
extern const double RAD_60;
extern const double RAD_90;

extern const double CELL_LEPTON_DIAG;

extern const double ISO_TILE_SIZE;
extern const int ISO_TILE_PIXEL_W;
extern const int ISO_TILE_PIXEL_H;

extern const int LEVEL_LEPTON_H;
extern const int LEVEL_PIXEL_H;
extern const double CELL_SLOPE_ANGLE;
extern const double CELL_DIAG_SLOPE_ANGLE;
extern const int LEVEL_PIXEL_H_1;
extern const int BRIDGE_LEPTON_HEIGHT;


extern const Cell CELL_NONE;
extern const Coord COORD_NONE;


/************************************************************
**	Win32 specific externs
*/
extern int ReadyToQuit;							//Are we about to exit cleanly
void Memory_Error_Handler(void);				//Memory error handler function

extern bool TacticalActive;

extern CDTimerClass<FrameTimerClass> TournamentTimer;

extern char Debug_ScenarioName[];

extern Buffer * UnkBuffer;
