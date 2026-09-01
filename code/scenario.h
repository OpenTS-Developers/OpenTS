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

/* $Header: /CounterStrike/SCENARIO.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SCENARIO.H                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 02/26/96                                                     *
 *                                                                                             *
 *                  Last Update : February 26, 1996 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "addon.h"
#include "coord.h"
#include "ftimer.h"
#include "random.h"
#include "special.h"
#include "stimer.h"
#include "timer.h"
#include "typelist.h"
#include "types.h"

#include "campaign.hh"
#include "diff.hh"
#include "house.hh"
#include "side.hh"
#include "theater.hh"
#include "theme.hh"
#include "vq.hh"
#include "waypoint.hh"

#include <cstdlib>

#define SCEN_LOCAL_COUNT 50
#define SCEN_GLOBAL_COUNT 50

class CRCEngine;
class INIClass;
class SaveStreamClass;
class TechnoTypeClass;
class AbstractClass;
class CellClass;
class ObjectClass;


/*
**	This class holds the information about the current game being played. This information is
**	global to the scenario and is generally of a similar nature to the information that was held
**	in the controlling scenario INI file.
*/
class ScenarioClass {
	public:

		// Constructor.
		ScenarioClass(void);
		void Set_Scenario_Name(char const * name);
		void Reset(void);

		int Get_Unique_ID(void);

		bool Fetch_Global_Value(int global, bool & value);
		bool Fetch_Global_Value(char const * variable_name, bool & value);
		bool Set_Global_To(int global, bool value);
		bool Set_Global_To(char const * variable_name, bool value);
		int Global_From_Name(char const * variable_name);
		bool Read_Global_INI(CCINIClass const & ini);

		int Used_Local_Count(void);
		bool Fetch_Local_Value(int index, bool & value);
		bool Fetch_Local_Value(char const * variable_name, bool & value);
		bool Set_Local_To(int index, bool value);
		bool Set_Local_To(char const * variable_name, bool value);
		int First_Unused_Local(void) const;
		int Local_From_Name(char const * variable_name);
		bool Read_Local_INI(CCINIClass const & ini);
		bool Write_Local_INI(CCINIClass & ini) const;

		bool Read_INI(CCINIClass const & ini);
		bool Write_INI(CCINIClass & ini, bool mplayer=false) const;

		void Save(IStream * stream) const;
		void Load(IStream * stream);

		void Serialize(SaveStreamClass & stream);

		void Compute_CRC(CRCEngine & crc) const;

		Cell Get_Waypoint_Cell(WAYPOINT waypoint) const;
		Coord Get_Waypoint_Coord(WAYPOINT waypoint) const;
		CellClass * Get_Waypoint_CellClass(WAYPOINT waypoint) const;
		AbstractClass * Get_Waypoint_Target(WAYPOINT waypoint) const;

		char const * First_Unused_Waypoint_Name(void) const;

		void Read_Waypoints(CCINIClass const & ini);
		void Write_Waypoints(CCINIClass & ini) const;

		bool Is_Valid_Waypoint(WAYPOINT waypoint) const;
		void Set_Waypoint(WAYPOINT waypoint, Cell cell);
		void Clear_Waypoint(WAYPOINT waypoint);
		void Clear_All_Waypoints(void);

	public:

		/*
		**	This class records the special command override options that C&C
		**	supports.
		*/
		SpecialClass Special;

		/*
		 * These are the scenarios that follow this one when the map selection screen is
		 * skipped. The alternate is taken in place of the first when global flag 1 is set,
		 * which is how a mission branches on what the player accomplished.
		 */
		char NextScenarioName[_MAX_PATH];
		char AltNextScenarioName[_MAX_PATH];

		/*
		 * These are the waypoints the tactical view is centered on as the scenario opens.
		 * The alternate is taken in place of the first when global flag 0 is set, so a
		 * branching mission can start the player looking somewhere else.
		 */
		WAYPOINT Home;
		WAYPOINT AltHome;

		/*
		 * This is the running counter that hands out the unique identifier every object is
		 * given as it is created. Fetch the next one through the Get_Unique_ID function.
		 */
		int UniqueID;

		/*
		**	This is the source of the random numbers used in the game. This controls
		**	the game logic and thus must be in sync with any networked machines.
		*/
		Random2Class RandomNumber;

		/*
		**	This is the difficulty setting of the game.
		*/
		DiffType Difficulty;    // For human player.
		DiffType CDifficulty;   // For computer players.

		/*
		**	This is the main mission timer. This is the timer that is reset at the
		**	start of the mission. It, effectively, holds the elapsed time of the
		**	mission.
		*/
		mutable TTimerClass<SystemTimerClass> ElapsedTimer;

	private:
		/*
		**	This is an array of waypoints; each waypoint corresponds to a letter of
		**	the alphabet, and points to a cell number.  -1 means unassigned.
		**	The CellClass has a bit that tells if that cell has a waypoint attached to
		**	it; the only way to find which waypoint it is, is to scan this array.  This
		**	shouldn't be needed often; usually, you know the waypoint & you want the CELL.
		*/
		Cell Waypoint[WAYPT_COUNT];
	public:

		/*
		**	This holds the system wide mission countdown timer. Time based missions
		**	are governed by this timer. Various trigger events can modify and examine
		**	this timer. The current value of this timer will display on the game
		**	screen.
		*/
		CDTimerClass<FrameTimerClass> MissionTimer;

		/*
		**	The shroud regrowth (if enabled) is regulated by this timer. When the
		**	timer expires, the shroud will regrow one step.
		*/
		CDTimerClass<FrameTimerClass> ShroudTimer;

		/*
		 * The fog regrowth (if enabled) is regulated by this timer. When the
		 * timer expires, the fog will regrow one step.
		 */
		CDTimerClass<FrameTimerClass> FogTimer;

		/*
		 * The ice regrowth (if enabled) is regulated by this timer. When the
		 * timer expires, the ice will regrow one step.
		 */
		CDTimerClass<FrameTimerClass> IceGrowthTimer;

		/// Unused. Never started or checked -- it only feeds the scenario CRC.
		CDTimerClass<FrameTimerClass> VeinGrowthTimer;

		/*
		 * The gradual change of the ambient light level is regulated by this timer. When
		 * the timer expires, the current level takes one more step toward the desired one.
		 */
		CDTimerClass<FrameTimerClass> AmbientChangeTimer;

		/*
		**	The scenario number.
		*/
		int Scenario;

		/*
		**	The theater of the current scenario.
		*/
		TheaterType Theater;

		/*
		**	The full name of the scenario (as it exists on disk).
		*/
		char ScenarioName[_MAX_PATH];

		/*
		**	Description of the scenario.
		*/
		char Description[DESCRIP_MAX];

		/*
		**	The filename of the introduction movie.
		*/
		VQType IntroMovie;

		/*
		**	The filename of the briefing movie.
		*/
		VQType BriefMovie;

		/*
		**	The filename of the movie to play if the scenario is won.
		*/
		VQType WinMovie;

		/*
		**	The filename of the movie to play if the scenario is lost.
		*/
		VQType LoseMovie;

		/*
		**	The filename of the movie to play right after the briefing and
		**	just before the game.
		*/
		VQType ActionMovie;

		/*
		 * The filename of the movie to play after the score screen has been shown.
		 */
		VQType PostScoreMovie;

		/*
		 * The filename of the movie to play just before the map selection screen.
		 */
		VQType PreMapSelectMovie;

		/*
		**	This is the full text of the briefing. This text will be
		**	displayed when the player commands the "restate mission
		**	objectives" operation.
		*/
		char BriefingText[1024];

		/*
		**	This is the theme to start playing at the beginning of the action
		**	movie. A score started in this fashion will continue to play as
		**	the game progresses.
		*/
		ThemeType TransitTheme;

		/*
		**	The house that the player is to be (obsolete).
		*/
		HousesType PlayerHouse;

		/*
		**	The percentage of money that is allowed to be carried over into the
		**	following scenario.
		*/
		double CarryOverPercent;

		/*
		**	This specifies the maximum amount of money that is allowed to be
		**	carried over from the previous scenario. This limits the amount
		**	regardless of what the carry over percentage is set to.
		*/
		int CarryOverCap;

		/*
		**	This is the percent that the computer controlled base is to be
		**	built up to at the scenario start.
		*/
		int Percent;

		struct ScenarioFlagType {
			/*
			 * A scenario flag is a named boolean that the trigger system can test and set.
			 * The name is the one the designer gave it in the scenario file, and is left
			 * empty for a slot that was never declared.
			 */
			char VariableName[40];
			bool Value;

			// Carries the scenario flag to or from a save game.
			template<typename S>
			void Serialize(S & stream)
			{
				stream.Serialize(VariableName);
				stream.Serialize(Value);
			}
		};

		/*
		**	Global flags that are used in the trigger system and are persistent
		**	over the course of the game.
		*/
		ScenarioFlagType GlobalFlags[SCEN_GLOBAL_COUNT];

		/*
		 * Local flags that are used in the trigger system but last only for the duration
		 * of the scenario that declared them.
		 */
		ScenarioFlagType LocalFlags[SCEN_LOCAL_COUNT];

		/*
		**	This records the bookmark view locations the player has recorded.
		*/
		Cell Views[4];

		/*
		**	This is the number of active passable bridges in the current game.
		*/
		int BridgeCount;

		/*
		 * If the player is to have a radar map without building or holding a radar
		 * structure, then this flag will be true.
		 */
		bool IsFreeRadar;

		/*
		 * If trains are supposed to drop wood crates when they explode, then this flag
		 * will be set to true. This is the train counterpart of the IsTruckCrate flag.
		 */
		bool IsTrainCargo;

		/*
		 * These flags control whether tiberium, veins and ice are allowed to spread over
		 * the course of the scenario. The scenario establishes them and a trigger action
		 * can switch any of them while the mission is in progress.
		 */
		bool IsTibGrowth;
		bool IsVeinGrowth;
		bool IsIceGrowth;

		/*
		**	If a bridge has been destroyed, then this flag will be set to true.
		**	If there is a trigger that depends on this, it might be triggered.
		*/
		bool IsBridgeChanged;

		/*
		**	If a global has changed and global change trigger events must be
		**	processed, then this flag will be set to true.
		*/
		bool IsGlobalChanged;

		/*
		 * If the ambient light level has changed and the lighting trigger events must be
		 * processed, then this flag will be set to true.
		 */
		bool IsAmbientLightChanged;

		/*
		**	If this scenario is to be the last mission of the game (for this side), then
		**	this flag will be true.
		*/
		bool IsEndOfGame;

		/*
		**	If the mission countdown timer is to be inherited from the previous
		**	scenario, then this flag will be set to true.
		*/
		bool IsInheritTimer;

		/*
		**	If the score screen (and "mission accomplished" voice) is to be skipped when
		**	this scenario is finished, then this flag will be true.
		*/
		bool IsSkipScore;

		/*
		**	If this is to be a one time only mission such that when it is completed, the game
		**	will return to the main menu, then this flag will be set to true.
		*/
		bool IsOneTimeOnly;

		/*
		**	If the map selection is to be skipped then this flag will be true. If this
		**	ins't a one time only scenario, then the next scenario will have the same
		**	name as the current one but will be for variation "B".
		*/
		bool IsNoMapSel;

		/*
		**	If trucks are supposed to drop wood crates when they explode, then this flag
		**	will be set to true.
		*/
		bool IsTruckCrate;

		/*
		**	If the initial money is to be assigned as ore in available silos, then
		**	this flag will be set to true.
		*/
		bool IsMoneyTiberium;

		/*
		 * If infantry killed by tiberium poisoning are to leave a visceroid behind, then
		 * this flag will be set to true.
		 */
		bool IsTiberiumDeathToVisceroid;

		/*
		 * If the scenario brings its own AI triggers and the standard set would interfere,
		 * then this flag will be true and every global scope AI trigger is passed over.
		 */
		bool IsIgnoreGlobalAITriggers;

		/*
		 * If the player is fighting for GDI in this scenario, then this flag will be true.
		 * It picks the side's art, speech and interface before the mission is read in.
		 */
		bool IsGDI;

		/// Unused. Round-trips through the scenario INI and feeds the CRC, but nothing acts on it.
		bool IsMultiplayerOnly;

		/*
		 * If the map being played was built by the random map generator rather than read
		 * from a scenario file, then this flag will be true.
		 */
		bool IsRandom;

		/*
		 * If a crate was picked up during this game frame, then this flag will be true. It
		 * gives the crate pickup trigger events their chance to spring, and is cleared
		 * again once the logic pass is over.
		 */
		bool IsCrateBeenPickedUp;

		/*
		**	This is the fading countdown timer.  As this timer counts down, the
		**	fading to b&w or color will progress.  This timer represents a
		**	percentage of the Options.Get_Saturation() to fade towards.
		*/
		CDTimerClass<FrameTimerClass> FadeTimer;

		/*
		 * This is the campaign that the current scenario belongs to, or CAMPAIGN_NONE if
		 * this is not a campaign mission. It decides which CD the mission's movies live on
		 * and which credits movie plays at the end of the game.
		 */
		CampaignType Campaign;

		/*
		 * This is the number of dropships the player is given to fill before the mission
		 * begins. If zero, then the dropship loadout screen is skipped entirely.
		 */
		int StartingDropships;

		/*
		 * These three lists run in parallel and restrict what the player may bring in on
		 * the dropship -- the unit type, how many of it the scenario allows (-1 for no
		 * limit), and how many have been ordered so far.
		 */
		TypeList<TechnoTypeClass *> AllowableUnits;
		TypeList<int> AllowableUnitMaximums;
		TypeList<int> AllowableUnitCounts;

		/*
		 * This is the scenario's base ambient light level, expressed in hundredths where
		 * 100 is normal daylight. Both the current and the desired level start out at it.
		 */
		int AmbientLight;

		/*
		 * This is the ambient level actually in force. It drifts toward the desired level
		 * one step at a time, as governed by the AmbientChangeTimer.
		 */
		int CurrentAmbientLight;

		/*
		 * This is the ambient level the lighting is heading toward. Triggers and ion storms
		 * set it, and the gradual approach is what makes the change look natural rather
		 * than instant.
		 */
		int DesiredAmbientLight;

		/*
		 * These are the color tints applied to the scenario's lighting, expressed in
		 * hundredths where 100 leaves the color untouched.
		 */
		int RedTint;
		int GreenTint;
		int BlueTint;

		/*
		 * These bias a cell's brightness by its height -- LevelLight is added for every
		 * level the cell stands above the ground and GroundLight is subtracted outright.
		 * Both are expressed in thousandths, where NORMAL_LIGHT is full strength.
		 */
		int GroundLight;
		int LevelLight;

		/*
		 * These are the lighting values that take the place of the normal ones while an ion
		 * storm is raging. They are expressed just as their counterparts are.
		 */
		int IonAmbientLight;
		int IonRedTint;
		int IonGreenTint;
		int IonBlueTint;
		int IonGroundLight;
		int IonLevelLight;

		/*
		 * If a scenario is in the process of being read in, then this flag will be true.
		 * The game is not running yet, so only the housekeeping the load needs is done.
		 */
		bool IsReadingScenario;

		/// Unused. Round-trips through the scenario INI, but nothing acts on it.
		int InitTime;

		/*
		 * This is the expansion pack that must be installed before this scenario can be
		 * played. It is enabled as the scenario starts so that its rules and art apply.
		 */
		AddonType RequiredAddOn;

		/*
		 * This is the side whose voice set the mission is to be narrated with. If SIDE_NONE,
		 * then the player's own side supplies the speech instead.
		 */
		SideType SpeechSide;

		/*
		 * This is how far the player has progressed through the campaign's branching map
		 * selection, expressed as an index into the stages the player's house declares.
		 * The map selection screen offers the choices that lead on from this stage and
		 * records the one taken here. It carries over from one mission to the next.
		 */
		unsigned short Stage;

		/*
		 * If the player's control of the game has been taken away -- during a scripted
		 * sequence or a modal screen -- then this flag will be true.
		 */
		bool IsInputLocked;
};


void Write_Scenario_INI(char const * root, bool mplayer=false);
bool Read_Scenario_INI(char const * root, bool fresh=true);
bool Read_Scenario_INI(CCINIClass const & ini, bool is_mapgen=false);
int Scan_Place_Object(ObjectClass * obj, Cell const & cell);
void Assign_Houses(void);

void Post_Load_Game(void);
bool End_Game(void);
bool Read_Scenario(char const * root);
bool Start_Scenario(char const * name, bool briefing, CampaignType campaign);
HousesType Select_House(void);
void Clear_Scenario(void);
void Do_Lose(void);
void Do_Win(void);
void Do_Restart(void);
void Do_Abort(void);
void Fill_In_Data(void);
bool Restate_Mission(char const * name, int button1, int button2);

void Lock_Scenario_Input(void);
void Unlock_Scenario_Input(void);

void Pause_Scenario(void);
void Resume_Scenario(void);

int Adjust_To_CPU_Timing(int time);
void Toggle_Display_Mode(bool ingame);

extern ScenarioClass * Scen;
extern unsigned int ScenarioCRC;
