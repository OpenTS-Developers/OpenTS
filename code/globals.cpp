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

/* $Header: /counterstrike/GLOBALS.CPP 2     3/10/97 6:22p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : GLOBALS.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : September 10, 1993   [JLB]                                   *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

/// create all com interfaces here
#include "iblowfish.h"
#include "iblowfish_i.c"
#include "sun.h"
#include "isun_i.c"
#include "ilocos.h"
#include "ilocos_i.c"
#include "ipiggy.h"
#include "ipiggy_i.c"
#include "iblockci.h"
#include "iblockci_i.c"
#include "iflyctrl.h"
#include "iflyctrl_i.c"
#undef INCLUDE_COM

#include "_voxel.h"
#include "globals.h"
#include "goptions.h"
#include "ipxmgr.h"
#include "logic.h"
#include "mission.h"
#include "partsys.h"
#include "queue.h"
#include "rndstraw.h"
#include "rules.h"
#include "session.h"
#include "theme.h"
#include "vector.h"
#include "version.h"

#include "house.hh"
#include "special.hh"

#include <algorithm>


/*
 * Various world globals. These are defined here rather than in a header, so that every
 * module that includes globals.h does not get a copy of its own.
 */

/// Radian angles
const double ONE_RAD = M_PI / 180.0;
const double RAD_45 = 45 * ONE_RAD;
const double RAD_60 = 60 * ONE_RAD;
const double RAD_90 = 90 * ONE_RAD;

/// Cell diagonal length
const double CELL_LEPTON_DIAG = std::sqrt(pow((double)CELL_LEPTON_H, 2) * 2);					// 362.038665771484

/// Isometric tile size
const double ISO_TILE_SIZE = std::sqrt(pow(34, 2) * 2);                                        // 48.0832595825195
const int ISO_TILE_PIXEL_W = (int)(ISO_TILE_SIZE);                                                  // 48
const int ISO_TILE_PIXEL_H = (int)(std::cos(RAD_60) * ISO_TILE_SIZE);                          // 24

/// Height-related
const int LEVEL_LEPTON_H = (int)(std::tan(RAD_90 - RAD_60) * CELL_LEPTON_DIAG / 2);            // 104
const int LEVEL_PIXEL_H = ISO_TILE_PIXEL_H / 2;                                                     // 12
const double CELL_SLOPE_ANGLE = std::atan(LEVEL_LEPTON_H * (1 / (double)CELL_LEPTON_H));       // 0.372388541698456
const double CELL_DIAG_SLOPE_ANGLE = std::atan((LEVEL_LEPTON_H * 2) / CELL_LEPTON_DIAG);       // 0.511634767055511
const int LEVEL_PIXEL_H_1 = LEVEL_PIXEL_H;                                                          // 12
const int BRIDGE_LEPTON_HEIGHT = (int)((LEVEL_LEPTON_H * BRIDGE_CELL_HEIGHT) + 0.5);                /// 416

/// "None" coordinate and cell values
Cell const CELL_NONE(0,0);
Coord const COORD_NONE(0,0,0);


/*
 * Actual game globals.
 */

bool ScenarioActive = false;
bool Debug_ForceScenario = false;
bool Debug_Create_Maps = false;
bool Debug_SpeedBuild = false;
bool Debug_Inert = false;
bool Debug_MotionCapture = false;
bool Debug_Quiet = false;
bool Debug_Cheat = false;
bool Debug_Remap = false;
bool Debug_Icon = false;
bool Debug_Flag = false;
bool Debug_Lose = false;
bool Debug_Win = false;
bool Debug_Map = false;					// true = map editor mode
bool Debug_Passable = false;			// true = show passable/impassable terrain
bool Debug_Unshroud = false;			// true = hide the shroud
bool Debug_Threat = false;
bool Debug_Find_Path = false;
bool Debug_Check_Map = false;			// true = validate the map each frame
bool Debug_Playtest = false;
bool Debug_Trap_Check_Heap = false; // true = check the Heap
bool Debug_Print_Events = false;    // true = print event & packet processing
bool Debug_Console = false;


ParticleSystemClass * GasSystem;

CDTimerClass<FrameTimerClass> TournamentTimer;


/*
**	Game object allocation and tracking classes.
*/
DynamicVectorClass<AITriggerTypeClass *>		AITriggerTypes;
DynamicVectorClass<TriggerTypeClass *>			TriggerTypes;
DynamicVectorClass<TriggerClass *>				Triggers;
DynamicVectorClass<BuildingLightClass *>		BuildingLights;
DynamicVectorClass<ParticleSystemClass *>		ParticleSystems;
DynamicVectorClass<TubeClass *>					Tubes;
DynamicVectorClass<WaveClass *>					Waves;
DynamicVectorClass<BulletClass *>				Bullets;
DynamicVectorClass<OverlayClass *>				Overlays;
DynamicVectorClass<ParticleClass *>				Particles;
DynamicVectorClass<SmudgeClass *>				Smudges;
DynamicVectorClass<TerrainClass *>				Terrains;
DynamicVectorClass<InfantryClass *>				Infantry;
DynamicVectorClass<BuildingClass *>				Buildings;
DynamicVectorClass<AircraftClass *>				Aircraft;
DynamicVectorClass<UnitClass *>					Units;
DynamicVectorClass<HouseClass *>				Houses;
DynamicVectorClass<AnimClass *>					Anims;
DynamicVectorClass<TeamClass *>					Teams;
DynamicVectorClass<FactoryClass *>				Factories;
DynamicVectorClass<CampaignClass *>				Campaigns;
DynamicVectorClass<SideClass *>					Sides;
DynamicVectorClass<HouseTypeClass *>			HouseTypes;
DynamicVectorClass<TaskForceClass *>			TaskForces;
DynamicVectorClass<TeamTypeClass *>				TeamTypes;
DynamicVectorClass<ScriptTypeClass *>			ScriptTypes;
DynamicVectorClass<OverlayTypeClass *>			OverlayTypes;
DynamicVectorClass<SmudgeTypeClass *>			SmudgeTypes;
DynamicVectorClass<TerrainTypeClass *>			TerrainTypes;
DynamicVectorClass<BuildingTypeClass *>			BuildingTypes;
DynamicVectorClass<AircraftTypeClass *>			AircraftTypes;
DynamicVectorClass<UnitTypeClass *>				UnitTypes;
DynamicVectorClass<AnimTypeClass *>				AnimTypes;
DynamicVectorClass<InfantryTypeClass *>			InfantryTypes;
DynamicVectorClass<BulletTypeClass *>			BulletTypes;
DynamicVectorClass<VoxelAnimTypeClass *>		VoxelAnimTypes;
DynamicVectorClass<IsometricTileTypeClass *>	IsometricTileTypes;
DynamicVectorClass<ParticleTypeClass *>			ParticleTypes;
DynamicVectorClass<ParticleSystemTypeClass *>	ParticleSystemTypes;
DynamicVectorClass<SuperWeaponTypeClass *>		SuperWeaponTypes;
DynamicVectorClass<SuperClass *>				SuperWeapons;
DynamicVectorClass<TechnoClass *>				Technos;
DynamicVectorClass<TechnoTypeClass *>			TechnoTypes;
DynamicVectorClass<ObjectClass *>				Objects;
DynamicVectorClass<AbstractTypeClass *>			AbstractTypes;

DynamicVectorClass<AnimClass *>					MoveFlashes;


int NewINIFormat = 0;


/***************************************************************************
**	This is true if the game is the currently in focus windows app
**
*/
bool GameInFocus = false;


/***************************************************************************
**	These are the mission control structures. They hold the information about
**	how the missions should behave in the system.
*/
MissionControlClass MissionControl[MISSION_COUNT];


/***************************************************************************
**	There are various tutorial messages that can appear in the game. These
**	are called upon by number and pointed to by this array.
*/
IndexClass<int, char *> TutorialText;


/***************************************************************************
**	This is the source of the random numbers used in the game. This controls
**	the game logic and thus must be in sync with any networked machines.
*/
RandomStraw CryptRandom;


/***************************************************************************
**	This is a list of all selected objects (for this map). The support functions
**	are used to control access to this list. Do not modify it directly.
*/
DynamicVectorClass<ObjectClass *> CurrentObject;


/***************************************************************************
**	This is the game version.
*/
VersionClass VerNum;


VoxelDataStruct DropPodVoxel;


/***************************************************************************
**	These are the movie names to use for mission briefing, winning, and losing
**	sequences. They are read from the INI file.
*/
ScenarioClass * Scen;


/***************************************************************************
**	This records if the score (music) file is present. If not, then much of
**	the streaming score system can be disabled.
*/
bool ScoresPresent;

bool UnknownGlobalBool = true;

bool DrawShapeShadows = true;


int TournamentTime = -1;


/***************************************************************************
**	This flag will control whether there is a response from game units.
**	By carefully controlling this global, multiple responses are suppressed
**	when a large group of infantry is given the movement order.
*/
bool AllowVoice = true;


/***************************************************************************
**	This is the current frame number. This number is guaranteed to count
**	upward at the rate of one per game logic process. The target rate is 15
**	per second. This value is saved and restored with the saved game.
*/
int Frame = 0;


int _dialog_count = 0;


/***************************************************************************
**	These globals are constantly monitored to determine if the player
**	has won or lost. They get set according to the trigger events associated
**	with the scenario.
*/
bool PlayerWins;
bool PlayerLoses;
bool PlayerRestarts;

/*
**	This flag is set if the player neither wins nor loses; it's mostly for
**	multiplayer mode.
*/
bool PlayerAborts;


/***************************************************************************
**	This is a running accumulation of the number of ticks that were unused.
**	This accumulates into a useful value that contributes to a
**	histogram of game performance.
*/
int SpareTicks;
int PathCount;         // Number of findpaths called.
int CellCount;         // Number of cells redrawn.
int TargetScan;        // Number of target scans.
int SidebarRedraws;    // Number of sidebar redraws.


/***************************************************************************
**	This is the options control class. The options control such things as
**	game speed, visual controls, and other user settings.
*/
GameOptionsClass Options;


/***************************************************************************
**	This handles the background music.
*/
ThemeClass Theme;


/***************************************************************************
**	The running credit display is controlled by this class (and member
**	functions.
*/
CreditClass CreditDisplay;


/**************************************************************************
**	This class records the special command override options that C&C
**	supports.
*/
SpecialClass Special;


/***************************************************************************
**	This is the scenario data for the currently loaded scenario.
**	These variables should all be set together.
*/
HousesType Whom;							// Initial command line house choice.
int ScenarioInit;
bool SpecialFlag = false;


/***************************************************************************
**	This value tells the sidebar what items it's allowed to add.  The
**	lower the value, the simpler the sidebar will be. This value is the
**	displayed value for tech level in the multiplay dialogs. It remaps to
**	the in-game rules.ini tech levels.
*/
int BuildLevel = MPLAYER_BUILD_LEVEL_MAX;		// Buildable level (1 = simplest)


/***************************************************************************
**	The game plays as long as this var is true.
*/
bool GameActive;


/***************************************************************************
**	This is a scratch variable that is used to when a reference is needed to
**	a long, but the value wasn't supplied to a function. This is used
**	specifically for the default reference value. As such, it is not stable.
*/
int LParam;


#ifdef _DEBUG
/***************************************************************************
**	The currently-selected cell for the Scenario Editor
*/
Cell CurrentCell(0, 0);
#endif


/***************************************************************************
**	This is the house that the human player is currently playing.
*/
HouseClass * PlayerPtr;


/***************************************************************************
**	These are the event queues. One is for holding events until they are ready to be
**	sent to the remote computer for processing. The other list is for incoming events
**	that need to be executed when the correct frame has been reached.
*/
std::deque<EventClass> OutList;
std::deque<EventClass> DoList;


/***************************************************************************
**	These are arrays/lists of trigger pointers for each cell & the houses.
*/
DynamicVectorClass<TagClass *> MapTags;
int MapTriggerID;
DynamicVectorClass<TagClass *> LogicTags;
int LogicTriggerID;


/***************************************************************************
**	This is the list of carry over objects. These objects are part of the
**	pseudo saved game that might be carried along with the current saved
**	game.
*/
//CarryoverClass * Carryover;


/***************************************************************************
**	This value is computed every time a new scenario is loaded; it's a
**	CRC of the INI and binary map files.
*/
unsigned int ScenarioCRC;

char Debug_ScenarioName[128];


/***************************************************************************
**	This class manages data specific to multiplayer games.
*/
SessionClass Session;
#if (TIMING_FIX)
//
// These values store the min & max frame #'s for when MaxAhead >>increases<<.
// If MaxAhead increases, and the other systems free-run to the new MaxAhead
// value, they may miss an event generated after the MaxAhead event was sent,
// but before it executed, since it will have been scheduled with the older,
// shorter MaxAhead value.  This will cause a Packet_Received_Too_Late error.
// The frames from the point where the new MaxAhead takes effect, up to that
// frame Plus the new MaxAhead, represent a "period of vulnerability"; any
// events received that are scheduled to execute during this period should
// be re-scheduled for after that period.
//
int NewMaxAheadFrame1;
int NewMaxAheadFrame2;
#endif


/***************************************************************************
**	This is the network IPX manager class.  It handles multiple remote
**	connections.  Declaring this class doesn't perform any allocations;
**	the class itself is 140 bytes.
*/
IPXManagerClass Ipx(
	std::max(sizeof (GlobalPacketType), sizeof(RemoteFileTransferType) - 32),		// size of Global Channel packets
	MAX_IPX_PACKET_SIZE,
	160,                                        // # entries in Global Queue
	32,                                         // # entries in Private Queues
	IPXGlobalConnClass::COMMAND_AND_CONQUER2);  // Product ID #


bool VisceroidsAsSnoBees = false;
bool Just4Fun = false;


/***************************************************************************
**	This is the random-number seed; it's synchronized between systems for
**	multiplayer games.
*/
int Seed = 0;


/***************************************************************************
**	If this value is non-zero, use it as the random # seed instead; this should
**	help reproduce some bugs.
*/
int CustomSeed = 0;


/***************************************************************************
**
*/
bool IgnoreInput = false;
bool drag_select_aborted = false;

TheaterType LastTheater = THEATER_NONE;	//Lets us know when theater type changes.


/***************************************************************************
**	This flag is for popping up dialogs that call the main loop.
*/
SpecialDialogType SpecialDialog = SDLG_NONE;

bool _special_dialog_flag = true;

//
// Variables for helping track how much time goes bye in routines
//
int LogLevel = 0;
unsigned int LogLevelTime[ MAX_LOG_LEVEL ] = { 0 };
unsigned int LogLastTime = 0;
bool LogDump_Print = false;		// true = print the Log time Stuff

/***************************************************************************
**	Win32 specific globals
*/
int ReadyToQuit = 0;

bool TacticalActive;

Buffer * UnkBuffer; /// Possibly TheaterBuffer.
