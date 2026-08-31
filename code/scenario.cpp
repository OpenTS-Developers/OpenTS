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

//Mono_Printf("%d %s\n",__LINE__,__FILE__);
/* $Header: /CounterStrike/SCENARIO.CPP 15    3/13/97 2:06p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SCENARIO.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : October 21, 1996 [JLB]                                       *
 *                                                                                             *
 * This module handles the scenario reading and writing. Scenario related                      *
 * code that is executed between scenario play can also be here.                               *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Assign_Houses -- Assigns multiplayer houses to various players                            *
 *   Clear_Flag_Spots -- Clears flag overlays off the map                                      *
 *   Clear_Scenario -- Clears all data in preparation for scenario load.                       *
 *   Clip_Move -- moves in given direction from given cell; clips to map                       *
 *   Clip_Scatter -- randomly scatters from given cell; won't fall off map                     *
 *   Create_Units -- Creates infantry & units, for non-base multiplayer                        *
 *   Do_Lose -- Display losing comments.                                                       *
 *   Do_Restart -- Handle the restart mission process.                                         *
 *   Do_Win -- Display winning congratulations.                                                *
 *   Fill_In_Data -- Recreate all data that is not loaded with scenario.                       *
 *   Post_Load_Game -- Fill in an inferred data from the game state.                           *
 *   Read_Scenario -- Reads a scenario from disk.                                              *
 *   Read_Scenario_INI -- Read specified scenario INI file.                                    *
 *   Remove_AI_Players -- Removes the computer AI houses & their units                         *
 *   Restate_Mission -- Handles restating the mission objective.                               *
 *   Scan_Place_Object -- places an object >near< the given cell                               *
 *   ScenarioClass::ScenarioClass -- Constructor for the scenario control object.              *
 *   ScenarioClass::Set_Global_To -- Set scenario global to value specified.                   *
 *   Set_Scenario_Name -- Creates the INI scenario name string.                                *
 *   Start_Scenario -- Starts the scenario.                                                    *
 *   Write_Scenario_INI -- Write the scenario INI file.                                        *
 *   ScenarioClass::Do_BW_Fade -- Cause the palette to temporarily shift to B/W.               *
 *   ScenarioClass::Do_Fade_AI -- Process the palette fading effect.                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "scenario.h"

#include "_bench.h"
#include "_keyboar.h"
#include "_logic.h"
#include "_map.h"
#include "_palette.h"
#include "_rect.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_timer.h"
#include "_tooltip.h"
#include "_wsproto.h"
#include "addon.h"
#include "aircraft.h"
#include "aitrig.h"
#include "anim.h"
#include "astar.h"
#include "bench.h"
#include "building.h"
#include "builtype.h"
#include "campaign.h"
#include "ccrand.h"
#include "cctooltip.h"
#include "cell.h"
#include "conquer.h"
#include "convert.h"
#include "crc.h"
#include "data.h"
#include "dbgprint.h"
#include "egos.h"
#include "empulse.h"
#include "enviro.h"
#include "getcpu.h"
#include "globals.h"
#include "goptions.h"
#include "houstype.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "init.h"
#include "inline.h"
#include "intro.h"
#include "ion.h"
#include "ipxmgr.h"
#include "isotype.h"
#include "language\language.h"
#include "light.h"
#include "logic.h"
#include "mainopt.h"
#include "mapgen.h"
#include "mapsel.h"
#include "misc.h"
#include "mouse.h"
#include "movie.h"
#include "mpu.h"
#include "msgbox.h"
#include "netdlg.h"
#include "newmenu.h"
#include "overlay.h"
#include "overtype.h"
#include "partsys.h"
#include "preview.h"
#include "progress.h"
#include "psystype.h"
#include "queue.h"
#include "revent.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "score.h"
#include "script.h"
#include "session.h"
#include "smudge.h"
#include "stats.h"
#include "surface.h"
#include "swizzle.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforc.h"
#include "teamtype.h"
#include "terrain.h"
#include "theme.h"
#include "tiberium.h"
#include "tracker.h"
#include "trigger.h"
#include "trigtype.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vein.h"
#include "vox.h"
#include "wave.h"
#include "waypoint.h"
#include "wsproto.h"

#include "bench.hh"

#include <algorithm>

CDTimerClass<SystemTimerClass> ScenUnusedTimer;


static void Remove_AI_Players(void);
static void Create_Units(bool official);
static Cell const Clip_Scatter(Cell const & cell, int maxdist);
static Cell const Clip_Move(Cell const & cell, FacingType facing, int dist);
static void Multiplayer_Last_Minute_Fixups(bool official = true);
static char const * Pick_Load_Background_Name(Point2D & text_pos);


/***********************************************************************************************
 * ScenarioClass::ScenarioClass -- Constructor for the scenario control object.                *
 *                                                                                             *
 *    This constructs the default scenario control object. Normally, all the default values    *
 *    are meaningless since the act of starting a scenario will fill in all of the values with *
 *    settings retrieved from the scenario control file.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
ScenarioClass::ScenarioClass(void)
{
	Reset();
	IsReadingScenario = false;
	IsRandom = false;
	Special.Init();

	Scenario = 1;
	Campaign = CAMPAIGN_NONE;
	NextScenarioName[0] = '\0';
	AltNextScenarioName[0] = '\0';

	int i;
	for (i = 0; i < ARRAY_SIZE(Waypoint); i++) {
		Waypoint[i] = CELL_NONE;
	}
	strcpy(Description, "");
	strcpy(ScenarioName, "");
	strcpy(BriefingText, "");
	for (i = 0; i < ARRAY_SIZE(GlobalFlags); i++) {
		GlobalFlags[i].VariableName[0] = '\0';
		Set_Global_To(i, false);
	}
	for (i = 0; i < ARRAY_SIZE(LocalFlags); i++) {
		LocalFlags[i].VariableName[0] = '\0';
		Set_Local_To(i, false);
	}
	memset(Views, '\0', sizeof(Views));
}


/// <summary>
/// Resets the scenario data back to its defaults.
/// This routine runs before a scenario is read, so that any setting the mission neglects
/// to mention falls back on a sensible value rather than a leftover from the last game.
/// </summary>
void ScenarioClass::Reset(void)
{
	Home = WAYPT_HOME;
	AltHome = WAYPT_HOME;
	UniqueID = 1000 * 1000;
	Difficulty = DIFF_NORMAL;
	CDifficulty = DIFF_NORMAL;
	ElapsedTimer = 0;
	ElapsedTimer.Stop();
	MissionTimer = 0;
	MissionTimer.Stop();
	ShroudTimer = 0;
	FogTimer = 0;
	IceGrowthTimer = 0;
	VeinGrowthTimer = 0;
	AmbientChangeTimer = 0;
	Theater = THEATER_NONE;
	IntroMovie = VQ_NONE;
	BriefMovie = VQ_NONE;
	WinMovie = VQ_NONE;
	LoseMovie = VQ_NONE;
	ActionMovie = VQ_NONE;
	PostScoreMovie = VQ_NONE;
	PreMapSelectMovie = VQ_NONE;
	TransitTheme = THEME_NONE;
	PlayerHouse = HOUSE_FIRST;
	CarryOverPercent = 0;
	CarryOverCap = 0;
	Percent = 0;
	BridgeCount = 0;
	IsFreeRadar = false;
	IsTrainCargo = false;
	IsTibGrowth = true;
	IsVeinGrowth = true;
	IsIceGrowth = true;
	IsBridgeChanged = false;
	IsGlobalChanged = false;
	IsAmbientLightChanged = false;
	IsEndOfGame = false;
	IsInheritTimer = false;
	IsSkipScore = false;
	IsOneTimeOnly = false;
	IsNoMapSel = false;
	IsTruckCrate = false;
	IsMoneyTiberium = false;
	IsIgnoreGlobalAITriggers = false;
	IsGDI = true;
	IsMultiplayerOnly = false;
	IsCrateBeenPickedUp = false;
	FadeTimer = 0;
	StartingDropships = 0;
	AmbientLight = 100;
	CurrentAmbientLight = 100;
	DesiredAmbientLight = 100;
	RedTint = 100;
	GreenTint = 100;
	BlueTint = 100;
	GroundLight = NORMAL_LIGHT / 10;
	LevelLight = NORMAL_LIGHT / 60;
	IonAmbientLight = 100;
	IonRedTint = 0;
	IonGreenTint = 0;
	IonBlueTint = 0;
	IonGroundLight = NORMAL_LIGHT / 10;
	IonLevelLight = NORMAL_LIGHT / 60;
	InitTime = 10000;
	Stage = 0;
	IsInputLocked = false;
	IsTiberiumDeathToVisceroid = true;

	AllowableUnits.Clear();
	AllowableUnitMaximums.Clear();
	AllowableUnitCounts.Clear();

	RequiredAddOn = ADDON_BASE_GAME;
	SpeechSide = SIDE_NONE;
}


/***********************************************************************************************
 * Start_Scenario -- Starts the scenario.                                                      *
 *                                                                                             *
 *    This routine will start the scenario. In addition to loading the scenario data, it will  *
 *    play the briefing and action movies.                                                     *
 *                                                                                             *
 * INPUT:   root     -- Pointer to the filename root for this scenario (e.g., "SCG01EA").      *
 *                                                                                             *
 *          briefing -- Should the briefing be played? Normally this is true except when the   *
 *                      scenario is restarting.                                                *
 *                                                                                             *
 * OUTPUT:  Was the scenario started without error?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Start_Scenario(char const * name, bool briefing, CampaignType campaign)
{
	if ((name == NULL || strlen(name) == 0) && campaign != CAMPAIGN_NONE) {
		if (Debug_ForceScenario == true) {
			name = Debug_ScenarioName;
		} else {
			name = Campaigns[campaign]->ScenarioName;
		}
	}

	Scen->Campaign = campaign;

	DebugString("\n----- Starting scnenario: %s -----\n", name);
	DebugString("Player Count: %d\n", Session.Players.Count());

	strcpy(Scen->ScenarioName, name);
	strupr(Scen->ScenarioName);

	Theme.Stop();

	if ((briefing == true) && (campaign != CAMPAIGN_NONE) && (Scen->Scenario == 1) && (Campaigns[Scen->Campaign]->CDNumber < 2)) {
		Choose_Side(Campaigns[Scen->Campaign]->CDNumber);
	}

	DebugString("Reading scenario: %s\n", name);

	if (!Read_Scenario(name)) {
		return(false);
	}

	Theme.Stop();

	if (briefing) {
		Play_Movie(Scen->IntroMovie);
		Play_Movie(Scen->BriefMovie);
	}

	/*
	**	If there's no briefing movie, restate the mission at the beginning.
	*/
	char buffer[25];

	if (Scen->BriefMovie != VQ_NONE) {
		wsprintf(buffer, "%s.VQA", Movies[Scen->BriefMovie]);
	}

	if (Scen->StartingDropships > 0) {
		Dropship_Screen();
	}

	if (briefing) {
		Play_Movie(Scen->ActionMovie, Scen->TransitTheme);
	}

	if (Scen->ActionMovie == VQ_NONE && Scen->TransitTheme != THEME_NONE) {
		Theme.Queue_Song(Scen->TransitTheme);
	} else {
		Theme.Queue_Song(THEME_PICK_ANOTHER);
	}

	/*
	**	Set the options values, since the palette has been initialized by Read_Scenario
	*/
	Options.Set();

	HiddenSurface->Fill(0);
	Update_Visible_Surface();

	Scen->ElapsedTimer.Start();

	ScenarioActive = true;
	TacticalActive = true;

	return(true);
}


/// <summary>
/// Pauses the scenario in progress.
/// The mission clock is stopped for solo play, any in-game movie is suspended, and the
/// tactical map is redrawn with the plain cursor so that the game is left looking tidy
/// while the player is away from it.
/// </summary>
void Pause_Scenario(void)
{
	if ((Session.Type == GAME_NORMAL) || (Session.Type == GAME_SKIRMISH)) {
		DebugString("Paused ElapsedTimer = %d\n", (int)Scen->ElapsedTimer);
		Scen->ElapsedTimer.Stop();
	}

	Pause_Ingame_Movie(true);

	if (ToolTips != NULL) {
		ToolTips->Activate(false);
	}

	Map.Abort_Drag_Select();

	Map.Override_Mouse_Shape(MOUSE_NORMAL, false);

	Hide_Mouse();
	Map.Flag_To_Redraw(GS_REDRAW_ALL);
	Map.Render();
	Show_Mouse();
}


/// <summary>
/// Resumes a scenario that was paused.
/// The mission clock starts again, the mouse shape is put back the way the player left it,
/// and any in-game movie picks up where it stopped.
/// </summary>
void Resume_Scenario(void)
{
	Map.Revert_Mouse_Shape();

	Scen->ElapsedTimer.Start();
	DebugString("Resume ElapsedTimer = %d\n", (int)Scen->ElapsedTimer);

	if (ToolTips != NULL) {
		ToolTips->Activate(Options.ToolTips);
	}

	Pause_Ingame_Movie(false);
}


/// <summary>
/// Takes control of the game away from the player.
/// This routine is used while a scripted sequence plays out, so that the player cannot
/// interfere with it. The mouse is hidden and all input is discarded until
/// Unlock_Scenario_Input is called.
/// </summary>
void Lock_Scenario_Input(void)
{
	Map.Abort_Drag_Select();

	if (!Scen->IsInputLocked) {
		Hide_Mouse();
	}

	Scen->IsInputLocked = true;
	IgnoreInput = true;
}


/// <summary>
/// Returns control of the game to the player.
/// This routine undoes Lock_Scenario_Input. The mouse comes back and anything the player
/// typed while input was being ignored is thrown away.
/// </summary>
void Unlock_Scenario_Input(void)
{
	if (Scen->IsInputLocked == true) {
		Show_Mouse();
	}

	Scen->IsInputLocked = false;
	IgnoreInput = false;
	Keyboard->Clear();
}


/// <summary>
/// Waits for the other players to finish loading the scenario.
/// The network is kept alive and the progress display updated until every machine reports
/// that it is ready. A player who stops making progress is dropped rather than being
/// allowed to hold the game up forever. Solo and skirmish games have nobody to wait for.
/// </summary>
/// <returns>bool; Did everyone make it into the game?</returns>
bool Wait_For_Players_To_Load(void)
{
	if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
		return(true);
	}

	CDTimerClass<SystemTimerClass> wait_timeout;
	CDTimerClass<SystemTimerClass> timer = TIMER_SECOND * 5;

	wait_timeout = 60 * TIMER_SECOND;
	double last_progress = Progress.Get_Current_Progress();

	for (;;) {
		Keyboard->Check();
		Call_Back();

		if (wait_timeout <= 0) {
			for (int i = 1; i < Session.Players.Count(); i++) {
				if (Progress.Get_Current_Progress(i) != 1.0) {
					DebugString("Player %s failed to make it into the game. Dropping %s\n", Session.Players[i]->Name, Session.Players[i]->Name);
					Destroy_Connection(Session.Players[i]->Player.ID, 1);
					i--;
				}
			}
			return(false);
		}

		double current_progress = Progress.Get_Current_Progress();
		if (current_progress >= 0.9995) {
			Session.Update_Progress(100 + (Scen->IsRandom ? 100 : 0));
			break;
		}

		if (current_progress != last_progress) {
			wait_timeout = 60 * TIMER_SECOND;
			last_progress = current_progress;
		}

		if (timer <= 0) {
			timer = TIMER_SECOND;
			Session.Update_Progress(100 + (Scen->IsRandom ? 100 : 0));
		}
	}
	return(true);
}


/***********************************************************************************************
 * Read_Scenario -- Reads a scenario from disk.                                                *
 *                                                                                             *
 *    This will read a scenario from disk. Use this to begin a scenario.                       *
 *    It doesn't perform any rendering, it merely sets up the system                           *
 *    with the proper data. Setting of the right game state will start                         *
 *    the scenario running.                                                                    *
 *                                                                                             *
 * INPUT:   root     -- Scenario root filename                                                 *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:   You must clear out the system variables before calling                          *
 *             this function. Use the Clear_Scenario() function.                               *
 *               It is assumed that Scenario is set to the current scenario number.            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/22/1991     : Created.                                                                 *
 *   02/03/1992 JLB : Uses house identification.                                               *
 *=============================================================================================*/
bool Read_Scenario(char const * fname)
{
	bool read_ok = true;
	char name[_MAX_PATH];

	strcpy(name, fname);

	Frame = 0;

	if (TournamentTime > 0) {
		TournamentTimer = TournamentTime * TICKS_PER_MINUTE;
	}

	BStart(BENCH_SCENARIO);

	Scen->IsReadingScenario = true;
	ScenarioInit++;

	char * ext = name + strlen(name) - 4;

	if (stricmp((ext), ".SED") == 0) {
		Scen->IsRandom = true;
		DebugString("Scen->IsRandom = true\n");
	} else {
		Scen->IsRandom = false;
		DebugString("Scen->IsRandom = false\n");
	}

	if (!Debug_Map) {

		int players = 1;

		if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
			players = Session.Players.Count();
		}

		Point2D prog_bar_pos;
		char const * background = Pick_Load_Background_Name(prog_bar_pos);
		Progress.Initialize(100, players);

		char * prog_msg = NULL;
		char prog_msg_buffer[129];

		if (Session.Type == GAME_INTERNET && WestwoodOnline_Tournament) {
			sprintf(prog_msg_buffer, Fetch_String(TXT_GAME_ID), WestwoodOnline_GameID);
			prog_msg = prog_msg_buffer;
		}

		Progress.Set_Graphic_Data((players > 1) ? "PROGBARM.SHP" : "PROGBAR.SHP", background, prog_msg, prog_bar_pos);
		Progress.Display_Progress();

		if (PacketTransport != NULL && Ipx.Transport_Mode() == IPXManagerClass::TRANSPORT_DIRECT && Session.Players.Count() > 1) {
			DebugString("Setting addresses for UDP broadcast\n");
			PacketTransport->Clear_Broadcast_Addresses();

			for (int p = 1; p < Session.Players.Count(); p++) {
				DebugString("Adding broadcast address %s\n", Session.Players[p]->Address.As_String());
				PacketTransport->Set_Broadcast_Address (Session.Players[p]->Address);
			}
		}
	}

	if (Scen->IsRandom) {
		read_ok = RandomMapGen.SeedData.Load(name);

		if (read_ok) {
			RandomMapGen.Generate_Random_Map(false, NULL);
			Multiplayer_Last_Minute_Fixups();
		}
		strcpy(Scen->ScenarioName, name);
	} else {
		read_ok = Read_Scenario_INI(name);
	}

	if (!read_ok) {
		DebugString("Error - Unable to read scenario: %s\n", name);
		WWMessageBox().Process(TXT_UNABLE_READ_SCENARIO, TXT_OK);

		BEnd(BENCH_SCENARIO);
		ScenarioInit--;
		Scen->IsReadingScenario = false;
		return(false);
	}


	/*
	**	For multiplayer games, initialize the inter-player message system.
	**	Do this after loading the scenario, so the map's upper-left corner is
	**	properly set.
	*/
	Session.Messages.Init(
		TacticalRect.X, TacticalRect.Y,	// x,y for messages
		6, 										// max # msgs
		MAX_MESSAGE_LENGTH - 14,			// max msg length
		7 * 2,									// font height in pixels
		-1, -1, 									// x,y for edit line (appears above msgs)
		0,//BG		1,							// enable edit overflow
		20,										// min,
		MAX_MESSAGE_LENGTH - 14,			// max for trimming overflow
		TacticalRect.Width);					// Width in pixels of buffer

	Fill_In_Data();

	Session.Update_Progress(100 + (Scen->IsRandom ? 100 : 0));

	Session.SawGameCompletion = false;
	Session.OutOfSync = false;

	bool all_loaded = Wait_For_Players_To_Load();

	if (Session.Type == GAME_INTERNET) {
		Session.Init_Fixed_Alliances();
	}

	ScenarioInit--;

	if (!Debug_Map) {
		if (all_loaded) {

			Call_Back();
			double progress = Progress.Get_Current_Progress();

			while (progress < 1.0) {
				for (int i = 0; i < Session.Players.Count(); i++) {
					Progress.Set_Progress_Percent(i, 100);
				}
				progress = Progress.Get_Current_Progress();
			}
		}
	}

	Scen->IsReadingScenario = false;

	Progress.End();

	BEnd(BENCH_SCENARIO);

	return(true);
}


/***********************************************************************************************
 * Fill_In_Data -- Recreate all data that is not loaded with scenario.                         *
 *                                                                                             *
 *    This routine is called after the INI file for the scenario has been processed. It will   *
 *    infer the game state from the scenario INI data.                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *=============================================================================================*/
void Fill_In_Data(void)
{
	/*
	**	The basic scenario data load does not contain the full set of
	**	game data. We now must fill in the missing pieces.
	*/
	ScenarioInit++;
	int index;
	for (index = 0; index < Buildings.Count(); index++) {
		Buildings[index]->Update_Buildables();
	}

	Map.Flag_To_Redraw(GS_REDRAW_ALL);

	/*
	**	Since the sidebar starts up activated, adjust the home start position so that
	**	the right edge of the map will still be visible.
	*/
	if (!Debug_Map) {
		Map.SidebarClass::Activate(1);

		if (Session.Type == GAME_NORMAL) {
			bool flag = Environment.Globals[0];
			Cell view = Scen->Get_Waypoint_Cell(flag ? Scen->AltHome : Scen->Home);

			Scen->Views[0] = Scen->Views[1] = Scen->Views[2] = Scen->Views[3] = view;
			Coord coord = Coord(view);
			coord.Z = Map.Get_Height_GL(coord);
			Map.Set_Tactical_Position(coord);
		}
	}

	/*
	**	Handle any data resetting that can be safely inferred from the actual
	**	data that has been loaded.
	*/
	/*
	**	Distribute the trigger pointers to the appropriate working lists.
	*/
	for (index = 0; index < TagTypes.Count(); index++) {
		TagTypeClass * tp = TagTypes[index];

		if (tp->Attaches_To() & ATTACH_MAP) {
			MapTags.Add(Find_Or_Make(tp));
		}

		if (tp->Attaches_To() & ATTACH_GENERAL) {
			LogicTags.Add(Find_Or_Make(tp));
		}

		if (tp->Attaches_To() & ATTACH_HOUSE) {
			TagClass * tt = Find_Or_Make(tp);
			House_From_HousesType(tt->Class->FirstTrigger->House->House)->HouseTags.Add(tt);
		}
	}

	ScenarioInit--;

#ifdef DO_CARRYOVERS
	/*
	**	If inheriting from a previous scenario was indicated, then create the carry over
	**	objects at this time.
	*/
	if (Scen->IsToInherit) {
		CarryoverClass * cptr = Carryover;

		while (cptr != NULL) {
			cptr->Create();
			cptr = (CarryoverClass *)cptr->Get_Next();
		}
	}
#endif

	/*
	**	The "allow win" action is a special case that is handled here. The total number
	**	of triggers that have this action must be recorded.
	*/
	for (index = 0; index < TagTypes.Count(); index++) {
		TagTypeClass * tp = TagTypes[index];
		if (tp->Is_Allow_Win()) {
			Houses[tp->FirstTrigger->House->HeapID]->Blockage++;
		}
	}

	/*
	**	Move available money to silos, if the scenario flag so indicates.
	*/
	if (Scen->IsMoneyTiberium) {
		for (int house = 0; house < Houses.Count(); house++) {
			HouseClass * hptr = Houses[house];

			if (hptr != NULL) {
				int money = hptr->Available_Money();
				TiberiumClass const * tib = Tiberiums[0];

				while (money > tib->CreditValue && hptr->Available_Storage() > 0) {
					hptr->Harvested(1, TiberiumType(0));
					money -= tib->CreditValue;
				}
			}
		}
	}

	Map.All_To_Look(true);
	WaveClass::Init_Statics();

	/*
	**	Reset the movement zones according to the terrain passability.
	*/
	Map.Reset_Iterator();
	CellClass * cellptr = Map.Iterate();

	while (cellptr) {
		cellptr->Recalc_Attributes();
		cellptr = Map.Iterate();
	}

	Map.Compute_Zone_Connections();
	Map.Zone_Reset();
	Map.Reset_All_Subzones();
	Map.Set_WasUnderBridge_Flags();

	if (GasSystem == NULL) {
		GasSystem = new ParticleSystemClass(ParticleSystemTypes[ParticleSystemTypeClass::From_Name("GasCloudSys")], Cell(10, 10));
	}
}


/***********************************************************************************************
 * Post_Load_Game -- Fill in an inferred data from the game state.                             *
 *                                                                                             *
 *    This routine is typically called after a game has been loaded. Some working data lists   *
 *    can be rebuild from the game state. This working data is rebuilt rather than being       *
 *    stored with the game data file.                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Although it is safe to call this routine whenever, it is only needed after a    *
 *             game load.                                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/30/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Post_Load_Game(void)
{
	BuildingTypeClass::Post_Load_Game();
	TiberiumClass::Post_Load_Game();
	VeinholeMonsterClass::Init(Scen->Theater);

	Map.Post_Load_Radar_Fixup();

	TechnoClass::ActionLineTimer.Stop();
	TechnoClass::ActionLineTimer.Start();

	Search.Update_Map_Dimensions(Map.PlayRect);

	IonStormClass::Apply_Secondary_Effect(false);

	AnimClass::Post_Load_Game();

	Map.Flag_To_Redraw(GS_REDRAW_ALL);
	InvalidateRect(MainWindow, NULL, FALSE);
}


/***********************************************************************************************
 * Clear_Scenario -- Clears all data in preparation for scenario load.                         *
 *                                                                                             *
 *    This routine will clear out all data specific to a scenario in                           *
 *    preparation for a subsequent scenario data load. This will free                          *
 *    all units, animations, and icon maps.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/22/1991     : Created.                                                                 *
 *   03/21/1992 JLB : Changed buffer allocations, so changes memset code.                      *
 *   07/13/1995 JLB : End count down moved here.                                               *
 *=============================================================================================*/
void Clear_Scenario(void)
{
	int index;

	Scen->UniqueID = 1000 * 1000;

	PlayerPtr = NULL;

	Scen->Reset();

	IonStormClass::Ion_Storm_End();

	for (index = AbstractTypes.Count() - 1; index >= 0; index--) {
		AbstractTypeClass * abstype = AbstractTypes[index];
		if (abstype->What_Am_I() == RTTI_ISOTILETYPE) {
			AbstractTypes.Delete_Index(index);
		}
	}
	Delete_All_Objects();
	for (index = 0; index < IsometricTileTypes.Count(); index++) {
		AbstractTypes.Add(IsometricTileTypes[index]);
	}

	Map.Free_Cells();

	Map.ZoneConnections.Clear();

	TagClass::Delete_All();

	for (index = 0; index < ARRAY_SIZE(Scen->GlobalFlags); index++) {
		Scen->Set_Global_To(index, false);
	}

	delete TacticalMap;
	TacticalMap = new Tactical;
	TacticalMap->Reset_Dirty_Rectangles();

	LightSourceClass::Recalc = false;
	while (Objects.Count()) {
		if (Objects[0]->RTTI == RTTI_BULLET) {
			Objects[0]->Release();
		} else {
			delete Objects[0];
		}
	}

	LightSourceClass::Recalc = true;

	while (TagTypes.Count()) {
		delete TagTypes[0];
	}

	MapTags.Clear();
	LogicTags.Clear();


	LightSourceClass::Reset();
	IonStormClass::Init();
	EMPulseClass::Reset();
	VeinholeMonsterClass::Reset();

	TiberiumClass::Deinit_Tiberium_Spread_System();
	TiberiumClass::Deinit_Tiberium_Growth_System();

	/*
	**	Call everyone's Init routine, except the Map's; for the Map, only call
	**	MapClass::Init, which clears the Cell array.  The Display::Init requires
	**	a Theater argument, and the theater is not known at this point; also, it
	**	would reload MixFiles, which isn't desired.  Display::Read_INI calls its
	**	own Init, which will Init the entire Map hierarchy.
	*/
	Map.PlayRect = Rect(0, 0, 0, 0);
	DebugString("Map.Init_Clear()\n");
	Map.Init_Clear();
	DebugString("Logic.Init()\n");
	Logic.Init();

	DebugString("CurrentObjects.Clear()\n");
	CurrentObject.Clear();

	DebugString("Scen->Clear_All_Waypoints()\n");
	Scen->Clear_All_Waypoints();

	DebugString("Init_Campaigns()\n");
	Init_Campaigns();

	if (GasSystem) {
		GasSystem->Clear_System();
		delete GasSystem;
		GasSystem = NULL;
	}

	Scen->UniqueID = 1000 * 1000;

	Set_Speech_State(true);
}


/***********************************************************************************************
 * Do_Win -- Display winning congratulations.                                                  *
 *                                                                                             *
 *    Perform the win the mission process. This will display any winning movies and the score  *
 *    screen. Followed by the map selection screen and then the load of the new scenario.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1992 JLB : Created.                                                                 *
 *   01/01/1995 JLB : Carries money forward into next scenario.                                *
 *=============================================================================================*/
void Do_Win(void)
{
	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
		if (!Session.Play) {
			Wait_For_End_Of_Queue();
		}
	}

	ScenarioActive = false;
	TacticalActive = false;

	Scen->ElapsedTimer.Stop();
	Stop_Ingame_Movie();

	Map.Set_Default_Mouse(MOUSE_NORMAL);
	Hide_Mouse();
	Theme.Queue_Song(THEME_QUIET);

	/*
	**	If this is a multiplayer game, clear the game's name so we won't respond
	**	to game queries any more (in Call_Back)
	*/
	if (Session.Type != GAME_NORMAL) {
		Session.GameName[0] = 0;
	}

	/*
	**	Stop here if this is a multiplayer game.
	*/
	if (Session.Type != GAME_NORMAL) {
		if (!Session.Play) {
			Session.GamesPlayed++;
			Multi_Score_Presentation();
			Session.CurGame++;
			if (Session.CurGame >= MAX_MULTI_GAMES) {
				Session.CurGame = MAX_MULTI_GAMES - 1;
			}
		}

		GameActive = false;
		Show_Mouse();
		return;
	}

	Play_Movie(Scen->WinMovie);

	/*
	**	Do the ending screens only if not playing back a recorded game.
	*/
	if (!Session.Play) {
		/*
		**	If the score presentation should be performed, then do
		**	so now.
		*/
		Keyboard->Clear();

		if (!Scen->IsSkipScore) {
			ScoreClass().Presentation();
		}

		if (Scen->PostScoreMovie != VQ_NONE) {
			Play_Movie(Scen->PostScoreMovie);
		}
		if (Scen->PreMapSelectMovie != VQ_NONE) {
			Play_Movie(Scen->PreMapSelectMovie);
		}

		if (Scen->IsOneTimeOnly) {
			GameActive = false;
			Show_Mouse();
			return;
		}

		/*
		**	If this scenario is flagged as ending the game then print the credits and exit.
		*/
		if (Scen->IsEndOfGame) {
			if (Scen->Campaign != CAMPAIGN_NONE) {
				Play_Movie(Campaigns[Scen->Campaign]->FinalMovie);
			}
			Show_Who_Was_Responsible();
#ifdef _DEMO
			Load_Title_Page("tsdemoss.pcx", true);
			Keyboard->Clear();
			Keyboard->Get();
#endif
			GameActive = false;
			Show_Mouse();
			return;
		}

		/*
		** Hack section.  If it's allied scenario 10, variation A, then skip the
		** score and map selection, don't increment scenario, and set it to
		** variation B.
		*/
		if (Scen->IsNoMapSel) {
#ifdef _DEMO
			if (Scen->GlobalFlags[1].Value) {
				strcpy(Scen->ScenarioName, Scen->AltNextScenarioName);
			} else {
				strcpy(Scen->ScenarioName, Scen->NextScenarioName);
			}
#else
			if (Scen->GlobalFlags[1].Value) {
				Map_Select_Advance(Scen, Scen->AltNextScenarioName);
			} else {
				Map_Select_Advance(Scen, Scen->NextScenarioName);
			}
#endif
		} else {
			Show_Mouse();
			Map_Selection(Scen);
			Hide_Mouse();
		}

		Keyboard->Clear();
	}

	Show_Mouse();

	Environment.Store();

	Scen->Scenario++;

	/*
	**	Generate a new scenario filename
	*/
	Start_Scenario(Scen->ScenarioName, true, Scen->Campaign);
	Environment.Restore();

	Map.Render();
}


/***********************************************************************************************
 * Do_Lose -- Display losing comments.                                                         *
 *                                                                                             *
 *    Performs the lose mission processing. This will generally display a "would you like      *
 *    to replay" dialog and then either reload the scenario or set flags such that the main    *
 *    menu will appear.                                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1992 JLB : Created.                                                                 *
 *=============================================================================================*/
void Do_Lose(void)
{
	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
		if (!Session.Play) {
			Wait_For_End_Of_Queue();
		}
	}

	ScenarioActive = false;
	TacticalActive = false;

	Scen->ElapsedTimer.Stop();
	Stop_Ingame_Movie();

	Map.Set_Default_Mouse(MOUSE_NORMAL);
	Hide_Mouse();

	Theme.Queue_Song(THEME_QUIET);

	/*
	**	If this is a multiplayer game, clear the game's name so we won't respond
	**	to game queries any more (in Call_Back)
	*/
	if (Session.Type != GAME_NORMAL) {
		Session.GameName[0] = 0;
	}

	/*
	**	Announce win to player.
	*/
	int timeout = TickCount + (TIMER_SECOND * 5);

	while (Is_Speaking() && (timeout > TickCount)) {
		Call_Back();
	}

	Stop_Speaking();

	/*
	**	Stop here if this is a multiplayer game.
	*/
	if (Session.Type != GAME_NORMAL) {
		if (!Session.Play) {
			Session.GamesPlayed++;
			Multi_Score_Presentation();
			Session.CurGame++;

			if (Session.CurGame >= MAX_MULTI_GAMES) {
				Session.CurGame = MAX_MULTI_GAMES - 1;
			}
		}
		GameActive = false;
		Show_Mouse();
		return;
	}

	Play_Movie(Scen->LoseMovie);

	/*
	**	Start same scenario again
	*/
	Draw_Menu_Background();
	Show_Mouse();

	if (!Session.Play && !WWMessageBox().Process(TXT_TO_REPLAY, TXT_YES, TXT_NO)) {
		Keyboard->Clear();
		Start_Scenario(Scen->ScenarioName, false, Scen->Campaign);
		Environment.Restore();

		Map.Render();
	} else {
		GameActive = false;
	}
}


/***********************************************************************************************
 * Do_Restart -- Handle the restart mission process.                                           *
 *                                                                                             *
 *    This routine is called in the main game loop when the mission must be restarted. This    *
 *    routine will throw away the current game and reload the appropriate mission. The         *
 *    game will "resume" at the start of the mission.                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Do_Restart(void)
{
	ScenarioActive = false;
	TacticalActive = false;

	Scen->ElapsedTimer.Stop();
	Stop_Ingame_Movie();

	/*
	**	Start a timer going, before we restart the scenario
	*/
	CDTimerClass<SystemTimerClass> timer;
	timer = TICKS_PER_SECOND * 4;
	Theme.Queue_Song(THEME_QUIET);

	Map.Set_Default_Mouse(MOUSE_NORMAL);
	Keyboard->Clear();

	Start_Scenario(Scen->ScenarioName, false, Scen->Campaign);
	Environment.Restore();

	/*
	**	Make sure the message stays displayed for at least 1 second
	*/
	while (timer > 0) {
		Call_Back();
	}
	Keyboard->Clear();

	Map.Render();
}


/// <summary>
/// Handles the player abandoning the mission in progress.
/// The scenario is shut down and the exit acknowledgement is spoken. This routine waits
/// for that speech to finish before letting the game fall back out to the menus.
/// </summary>
void Do_Abort(void)
{
	ScenarioActive = false;
	TacticalActive = false;

	Scen->ElapsedTimer.Stop();
	Scen->Scenario = 1;

	Stop_Ingame_Movie();
	Keyboard->Clear();

	Map.Set_Default_Mouse(MOUSE_NORMAL);
	Theme.Queue_Song(THEME_QUIET);

	Stop_Speaking();
	Speak(VOX_CONTROL_EXIT);

	int timeout = TickCount + (TICKS_PER_SECOND * 20);

	while (Is_Speaking() && timeout > TickCount) {
		Call_Back();
	}

	Stop_Speaking();
	GameActive = false;
}


/// <summary>
/// Sets the name of the scenario being played.
/// The name is what the save game system records and what the mission description and
/// briefing text are looked up by.
/// </summary>
/// <remarks>A NULL name is quietly ignored, leaving the previous name in place.</remarks>
void ScenarioClass::Set_Scenario_Name(char const * name)
{
	if (name != NULL) {
		strncpy(ScenarioName, name, sizeof(ScenarioName));
		ScenarioName[ARRAY_SIZE(ScenarioName) - 1] = '\0';
	}
}


/***********************************************************************************************
 * Read_Scenario_INI -- Read specified scenario INI file.                                      *
 *                                                                                             *
 *    Read in the scenario INI file. This routine only sets the game                           *
 *    globals with that data that is explicitly defined in the INI file.                       *
 *    The remaining necessary interpolated data is generated elsewhere.                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *          root      root filename for scenario file to read                                  *
 *                                                                                             *
 *          fresh      true = should the current scenario be cleared?                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the scenario read successful?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Read_Scenario_INI(char const * fname, bool)
{
	Frame = 0;

	if (TournamentTime > 0) {
		TournamentTimer = TournamentTime * TICKS_PER_MINUTE;
	}

	/*
	**	Create scenario filename and read the file.
	*/
	CCINIClass ini;
	CCFileClass file(fname);

	DebugString("Read_Scenario_INI - Filename is %s\n", fname);

	int result = ini.Load(file, true);

	if (result == 0) {
		DebugString("Scenario ini load failed!\n");
		return(false);
	}

	strcpy(Scen->ScenarioName, fname);

	bool ok = Read_Scenario_INI(ini);
	return(ok);
}


/// <summary>
/// Fetches the artwork to display while a scenario loads.
/// The picture is chosen to suit the player's side and the current screen resolution, and
/// one of the pair available is taken at random so that the loading screen is not always
/// the same one.
/// </summary>
/// <param name="pos">Filled in with the position the loading text belongs at.</param>
/// <returns>Returns with the filename of the backdrop picture to display.</returns>
char const * Pick_Load_Background_Name(Point2D & pos)
{
	static char _backgrounds[12][13] = {
		"LOAD400C.PCX",
		"LOAD400D.PCX",
		"LOAD400A.PCX",
		"LOAD400B.PCX",
		"LOAD480C.PCX",
		"LOAD480D.PCX",
		"LOAD480A.PCX",
		"LOAD480B.PCX",
		"LOAD600C.PCX",
		"LOAD600D.PCX",
		"LOAD600A.PCX",
		"LOAD600B.PCX"
	};

	int player = 0;

	if (Session.Type == GAME_NORMAL) {
		if (Scen->Campaign != CAMPAIGN_NONE) {
			CampaignClass * campaign = Campaigns[Scen->Campaign];
			player = campaign->CDNumber;

			if (player > 1) {
				player = 0;
				if (strstr(campaign->ScenarioName, "GDI") == 0) {
					player = 1;
				}
			}
		}
	} else {
		player = Session.Players[player]->Player.House;
	}

	// Only two sides have loading art, so any other house is shown the first side's.
	if (player < 0 || player > 1) {
		player = 0;
	}

	int choice = (player << 1) + Random_Pick(0, 1);

	if (VisibleRect.Width == 640) {
		if (VisibleRect.Height == 480) {
			if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
				pos = Point2D(440, 189);
			} else {
				pos = Point2D(570, 180);
			}
			if (player == 1) {
				pos += Point2D(-4, 10);
			} else {
				pos += Point2D(-4, 3);
			}

			return(_backgrounds[choice + 4]);
		}
	}

	if (VisibleRect.Width >= 800 && VisibleRect.Height >= 600) {
		if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
			pos = Point2D(550, 236);
		} else {
			pos = Point2D(715, 230);
		}

		int x_adjust = (VisibleRect.Width - 800) / 2;
		int y_adjust = (VisibleRect.Height- 600) / 2;

		pos += Point2D(x_adjust, y_adjust);

		if (player == 1) {
			pos += Point2D(-4, 10);
		} else {
			pos += Point2D(-4, 3);
		}

		return(_backgrounds[choice + 8]);
	}

	if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
		pos = Point2D(440, 158);
	} else {
		pos = Point2D(570, 155);
	}

	if (player == 1) {
		pos += Point2D(-4, 10);
	} else {
		pos += Point2D(-4, 3);
	}

	return(_backgrounds[choice]);
}


/// <summary>
/// Performs the multiplayer adjustments that must wait for the map.
/// Surplus computer players are removed, each side's starting units and any random crates
/// are placed, and everyone is allied with the special house. None of this can be done
/// until every object in the scenario has been read, so it is left until the very end.
/// </summary>
/// <param name="official">Is this one of the maps that shipped with the game?</param>
void Multiplayer_Last_Minute_Fixups(bool official)
{
	/*
	**	If Ghosts are disabled and we're not editing, remove computer players
	**	(Must be done after all objects are read in from the INI)
	*/
	if ((Session.Options.AIPlayers + Session.Players.Count() < Rule->MaxPlayers) && !Debug_Map) {
		Remove_AI_Players();
	}
	Call_Back();

	/*
	**	Units must be created for each house.  If bases are ON, this routine
	**	will create an MCV along with the units; otherwise, it will just create
	**	a whole bunch of units.  Session.Options.UnitCount is the total # of units
	**	to create.
	*/
	if (!Debug_Map) {
		int save_init = ScenarioInit;			// turn ScenarioInit off
		ScenarioInit = 0;
		Create_Units(official);
		ScenarioInit = save_init;				// turn ScenarioInit back on
	}
	Session.Update_Progress(93);
	Call_Back();

	/*
	**	Place crates if random crates are enabled for
	**	this scenario.
	*/
	if (Session.Options.Goodies) {
		int count = std::max(Rule->CrateMinimum, Session.NumPlayers);
		count = std::min(count, Rule->CrateMaximum);
		for (int index = 0; index < count; index++) {
			Map.Place_Random_Crate();
		}
	}
	Call_Back();

	/*
	**	Compute my starting location as the average Coord of all my stuff.
	*/
	Map.Compute_Start_Pos();

	HouseClass * special_hptr = House_From_HousesType(HouseTypeClass::From_Name("Special"));

	for (int i = 0; i < Houses.Count(); i++) {
		Houses[i]->Base.House = Houses[i];

		if (!Houses[i]->IsHuman && !Houses[i]->Class->IsMultiplayPassive) {

			Houses[i]->PickEnemyTimer = Rule->AIHateDelays[Houses[i]->Difficulty];

			int available_money = Houses[i]->Available_Money();
			Houses[i]->Refund_Money((int)((Rule->MultiplayerAICreditMultipliers[Houses[i]->Difficulty] / 100.0) * available_money));
		}

		if (Houses[i]->Class->IsMultiplayPassive) {
			Houses[i]->Class->IsWallOwner = false;
		} else {
			Houses[i]->Class->IsWallOwner = true;
		}

		if (Houses[i] != special_hptr) {
			Houses[i]->Make_Ally(special_hptr);
			special_hptr->Make_Ally(Houses[i]);
		}
	}
}


/// <summary>
/// Reads a scenario from the database supplied.
/// This is the workhorse of scenario startup. Whatever scenario was in place is cleared,
/// the theater, side, and rules are prepared, and then every object type in the game is
/// given the chance to read itself in. The loading screen is kept moving throughout, since
/// this can take a while.
/// </summary>
/// <param name="is_mapgen">Is the scenario built by the random map generator?</param>
/// <returns>bool; Was the scenario read successfully?</returns>
bool Read_Scenario_INI(CCINIClass const & ini, bool is_mapgen)
{
	char buffer[32];

	ScenarioInit++;

	DebugString("Clearing old scenario\n");
	Clear_Scenario();

	if (Session.Type == GAME_NORMAL) {
		Scen->Difficulty = Session.CampaignDifficulty;
		Scen->CDifficulty = Session.CampaignCDifficulty;
		Scen->Special.IsFogOfWar = false;
		Special.IsFogOfWar = false;
	} else {
		Scen->Difficulty = (DiffType)Session.Options.AIDifficulty;
		Scen->CDifficulty = (DiffType)(DIFF_COUNT - 1 - Scen->Difficulty);
		Scen->Special.IsFogOfWar = Session.Options.FogOfWar;
		Special.IsFogOfWar = Session.Options.FogOfWar;
	}

	char const * const BASIC = "Basic";
	Scen->InitTime = ini.Get_Int(BASIC, "InitTime", 10000);
	bool official = ini.Get_Bool(BASIC, "Official", false); /// read here, but never consulted
	official = official; /// suppresses the unused variable warning

	if (Session.Type == GAME_NORMAL) {
		Disable_Addon(ADDON_ANY);
		Scen->RequiredAddOn = (AddonType)ini.Get_Int(BASIC, "RequiredAddOn", ADDON_BASE_GAME);
		Set_Required_Addon(Scen->RequiredAddOn);
		if (!Addon_Installed(Scen->RequiredAddOn)) {
			return(false);
		}
		Enable_Addon(Scen->RequiredAddOn);
	} else {
		Scen->RequiredAddOn = Get_Required_Addon();
	}

	/*
	**
	*/
	Session.Update_Progress(3);
	Swizzler.Discard();

	/*
	**
	*/
	DebugString("Creating new tactical map\n");
	if (TacticalMap != NULL) {
		delete TacticalMap;
	}

	TacticalMap = new Tactical;
	TacticalMap->Set_View_Dimensions(TacticalRect);

	/*
	**
	*/
	DebugString("Initializing Theater\n");
	Scen->Theater = ini.Get_TheaterType("Map", "Theater", THEATER_TEMPERATE);
	Init_Theater(Scen->Theater);

	Session.Update_Progress(30);

	/*
	**
	*/
	if (Session.Type == GAME_NORMAL) {
		ini.Get_String(BASIC, "Player", "GDI", buffer, sizeof(buffer));
		Scen->IsGDI = strcmpi(buffer, "GDI") == 0;
		Scen->SpeechSide = Scen->IsGDI == true ? SIDE_GDI : SIDE_NOD;
	} else {
		Scen->IsGDI = Session.PlayerIsGDI;
		Scen->SpeechSide = Session.PlayerIsGDI == false ? SIDE_NOD : SIDE_GDI;
	}

	DebugString("Calling Prep_For_Side()\n");
	if (!Prep_For_Side(Scen->IsGDI == true ? SIDE_GDI : SIDE_NOD)) {
		return(false);
	}

	/*
	**
	*/
	DebugString("Initializeing Rules\n");
	Rule->Initialize(*RuleINI);
	Session.Update_Progress(35);
	Call_Back();

	/*
	**
	*/
	if (Session.Type == GAME_NORMAL) {
		Scen->SpeechSide = ini.Get_Side("Basic", "SpeechSide", Scen->SpeechSide);
	}

	/*
	**
	*/
	DebugString("Calling Prep_Speech_For_Side()\n");
	if (!Prep_Speech_For_Side(Scen->SpeechSide)) {
		return(false);
	}

	/*
	**
	*/
	DebugString("Calling Scen->Read_Global_INI(*RuleINI);\n");
	Scen->Read_Global_INI(*RuleINI);
	Call_Back();

	/*
	**
	*/
	DebugString("Calling Rule->Addition() with scenario overrides\n");
	Rule->Addition(ini);
	DebugString("Finished Rule->Addition() with scenario overrides\n");
	Session.Update_Progress(45);

	/*
	**	Init the Scenario CRC value
	*/
	ScenarioCRC = 0;
#ifdef TOFIX
	len = strlen(buffer);
	for (int i = 0; i < len; i++) {
		val = (unsigned char)buffer[i];
		Add_CRC(&ScenarioCRC, (unsigned int)val);
	}
#endif

	/*
	**	Read in the specific information for each of the house types.  This creates
	**	the houses of different types.
	*/
	if (Session.Type == GAME_NORMAL) {
		DebugString("Reading in scenario house types\n");
		HouseClass::Read_All(ini);
	}
	Session.Update_Progress(50);

	/*
	**
	*/
	if (Scen->Read_INI(ini) == false) {
		return(false);
	}

	Session.Update_Progress(58);

	/*
	**
	*/
	if (Session.Type != GAME_NORMAL && !Session.Options.BridgeDestruction) {
		Special.IsDestroyBridges = Session.Options.BridgeDestruction;
	}
	Special.Apply_To_Game();

	Call_Back();

	/*
	**	Read in the team-type data. The team types must be created before any
	**	triggers can be created.
	*/
	TeamTypeClass::Read_All(AIINI, SCOPE_GLOBAL);
	if (Addon_Enabled(ADDON_FIRESTORM) == true) {
		TeamTypeClass::Read_All(FSAIINI, SCOPE_GLOBAL);
	}
	TeamTypeClass::Read_All(ini, SCOPE_LOCAL);

	/*
	**
	*/
	ScriptTypeClass::Read_All(AIINI, SCOPE_GLOBAL);
	if (Addon_Enabled(ADDON_FIRESTORM) == true) {
		ScriptTypeClass::Read_All(FSAIINI, SCOPE_GLOBAL);
	}
	ScriptTypeClass::Read_All(ini, SCOPE_LOCAL);

	/*
	**
	*/
	TaskForceClass::Read_All(AIINI, SCOPE_GLOBAL);
	if (Addon_Enabled(ADDON_FIRESTORM) == true) {
		TaskForceClass::Read_All(FSAIINI, SCOPE_GLOBAL);
	}
	TaskForceClass::Read_All(ini, SCOPE_LOCAL);

	/*
	**	Read in the trigger data. The triggers must be created before any other
	**	objects can be initialized.
	*/
	TriggerTypeClass::Read_All(ini);
	TagTypeClass::Read_All(ini);

	AITriggerTypeClass::Read_All(AIINI, SCOPE_GLOBAL);
	if (Addon_Enabled(ADDON_FIRESTORM) == true) {
		AITriggerTypeClass::Read_All(FSAIINI, SCOPE_GLOBAL);
	}
	AITriggerTypeClass::Read_All(ini, SCOPE_LOCAL);

	Session.Update_Progress(60);

	/*
	**	Read in the map control values. This includes dimensions
	**	as well as theater information.
	*/
	Map.Read_INI(ini);
	Call_Back();

	/*
	**
	*/
	TubeClass::Read_INI(ini);

	BuildingTypeClass::Post_Read_Tile_Fixup();

	Map.Flag_To_Redraw(GS_REDRAW_ALL);

	Session.Update_Progress(70);

	Call_Back();

	/*
	**	Read in any normal overlay objects.
	*/
	OverlayClass::Read_INI(ini);
	Call_Back();

	/*
	**
	*/
	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	while (cptr) {
		cptr->Recalc_Attributes();
		cptr = Map.Iterate();
	}

	/*
	**
	*/
	OverlayClass::Post_Read_Vein_Fixups();

	/*
	**	Read in and place the 3D terrain objects.
	*/
	TerrainClass::Read_INI(ini);
	Call_Back();

	/*
	**
	*/
	VeinholeMonsterClass::Init_Vein_Growth_System(true);
	TiberiumClass::Init_Tiberium_Growth_System();
	TiberiumClass::Init_Tiberium_Spread_System();
	Session.Update_Progress(72);

	/*
	**
	*/
	Map.Compute_Radar_Image();

	/*
	**	Read in and place the units (all sides).
	*/
	UnitClass::Read_INI(ini);
	Call_Back();
	Session.Update_Progress(74);

	AircraftClass::Read_INI(ini);
	Call_Back();

	/*
	**	Read in and place the infantry units (all sides).
	*/
	InfantryClass::Read_INI(ini);
	Call_Back();
	Session.Update_Progress(76);

	LightSourceClass::Recalc = false;

	/*
	**	Read in and place all the buildings on the map.
	*/
	BuildingClass::Read_INI(ini);
	Call_Back();
	Session.Update_Progress(78);

	LightSourceClass::Recalc = true;

	Call_Back();

	/*
	**	Read in any smudge overlays.
	*/
	SmudgeClass::Read_INI(ini);
	Call_Back();

	/*
	**
	*/
	CCINIClass mini;
	CCFileClass cfile;

	if (Session.Type == GAME_NORMAL) {
		_splitpath(Scen->ScenarioName, NULL, NULL, buffer, NULL);
		strcat(buffer, ".INI");
		cfile.Set_Name(buffer);

		if (cfile.Is_Available() == true) {
			mini.Load(cfile, false);
			Rule->Addition(mini);
		}

		cfile.Close();
		if (Scen->RequiredAddOn > ADDON_FIRST) {
			char fname[32];
			sprintf(fname, "MISSION%1d.INI", Scen->RequiredAddOn);
			cfile.Set_Name(fname);
		} else {
			cfile.Set_Name("MISSION.INI");
		}

		if (cfile.Is_Available() == true) {
			mini.Load(cfile, false);

			if (mini.Is_Present(Scen->ScenarioName, "Name")) {
				mini.Get_String(Scen->ScenarioName, "Name", "", Scen->Description, sizeof(Scen->Description));
			}

			if (mini.Is_Present(Scen->ScenarioName, "Briefing")) {
				mini.Get_String(Scen->ScenarioName, "Briefing", "", buffer, sizeof(buffer));

				if (strlen(buffer) > 0) {
					mini.Get_TextBlock(buffer, Scen->BriefingText, sizeof(Scen->BriefingText));
				}
			}
		}
	}

	if (Session.Type == GAME_SKIRMISH) {
		if (Just4Fun) {
			cfile.Close();
			cfile.Set_Name("TMCJ4F.INI");

			if (cfile.Is_Available() == true) {
				mini.Load(cfile, false);
				Rule->Addition(mini);
			}
		}
	}

	Session.Update_Progress(82);
	Call_Back();

	/*
	**	Perform a final overpass of the map. This handles smoothing of certain
	**	types of terrain (tiberium).
	*/
	Map.Overpass();

	Session.Update_Progress(86);
	Call_Back();

	Session.Update_Progress(90);
	Call_Back();

	/*
	**	Multi-player last-minute fixups:
	**	- If computer players are disabled, remove all computer-owned houses
	**	- If bases are disabled, create the scenario dynamically
	**	- Remove any flag spot overlays lying around
	**	- If capture-the-flag is enabled, assign flags to cells.
	*/
	if (Session.Type != GAME_NORMAL && !is_mapgen) {
		Multiplayer_Last_Minute_Fixups(ini.Get_Bool(BASIC, "Official", false));
	}

	Call_Back();

	Swizzler.Resolve();

	Session.Update_Progress(96);
	Call_Back();

	Process_Deferred_Deletion();

	if (Session.Type != GAME_NORMAL) {
		Scen->Special = Special;
	}

	/*
	**	Return with flag saying that the scenario file was read.
	*/
	ScenarioInit--;

	int save_init = ScenarioInit;
	ScenarioInit = 0;
	BuildingClass::Init_All_Laser_Fences();
	ScenarioInit = save_init;

	Session.Update_Progress(98);
	Call_Back();

	Map.Clear_Background_Stack();

	if (Scen->Special.IsFogOfWar) {
		Map.Init_Fog_System();
	}

	RadarEventClass::Clear();

	Map.Complete_Radar_Refresh();

	return(true);
}


/***********************************************************************************************
 * Write_Scenario_INI -- Write the scenario INI file.                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      root      root filename for the scenario                                               *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   05/11/1995 JLB : Updates movie data.                                                      *
 *=============================================================================================*/
void Write_Scenario_INI(char const * fname, bool mplayer)
{
	CCINIClass ini;

	/*
	**	Preload the old scenario if it is present because there may
	**	be some fields in the INI that are processed but not written
	**	out. Preloading the scenario will preserve these manually
	**	maintained entries.
	*/
	if (Debug_Map) {
		if (CCFileClass(fname).Is_Available()) {
			CCFileClass cfile(fname);
			ini.Load(cfile, true);
		}
	}

	Scen->Write_INI(ini, mplayer);

	if (mplayer) {
		if (RandomMapGen.MapPreview != NULL && RandomMapGen.MapPreview->Get_Preview_Surface() != NULL) {
			RandomMapGen.MapPreview->Write_INI(ini);
		} else {
			MapPreviewClass map_preview;
			map_preview.Create_Preview();
			map_preview.Write_INI(ini);
		}
	} else {
		HouseClass::Write_All(ini);
	}
	Map.Write_INI(ini);
	TerrainClass::Write_INI(ini);
	UnitClass::Write_INI(ini);
	InfantryClass::Write_INI(ini);
	AircraftClass::Write_INI(ini);
	BuildingClass::Write_All(ini);
	OverlayClass::Write_INI(ini);
	SmudgeClass::Write_INI(ini);
	TubeClass::Write_INI(ini);

	if (!mplayer || Debug_Map) {
		ScriptTypeClass::Write_All(ini, SCOPE_LOCAL);
		TaskForceClass::Write_All(ini, SCOPE_LOCAL);
		TeamTypeClass::Write_All(ini, SCOPE_LOCAL);
		AITriggerTypeClass::Write_All(ini, SCOPE_LOCAL);
	}

	TriggerTypeClass::Write_All(ini);
	TagTypeClass::Write_All(ini);

	RawFileClass rawfile(fname);
	ini.Save(rawfile, true);
}


/// <summary>
/// Fetches the node a seat of the match was described by. The player list is held in each
/// machine's own order, so a seat is found by the house it was assigned, not by position.
/// </summary>
/// <returns>The node describing that seat, or NULL if the match does not hold it.</returns>
static NodeNameType * Seated_Node(int seat)
{
	for (int i = 0; i < Session.Players.Count(); i++) {
		if (Session.Players[i]->Player.ID == seat) {
			return(Session.Players[i]);
		}
	}

	for (int i = 0; i < Session.Computers.Count(); i++) {
		if (Session.Computers[i]->Player.ID == seat) {
			return(Session.Computers[i]);
		}
	}

	return(NULL);
}


/***********************************************************************************************
 * Assign_Houses -- Assigns multiplayer houses to various players                              *
 *                                                                                             *
 * This routine assigns all players to a multiplayer house slot; it forms network connections  *
 * to each player.  The Connection ID used is the value for that player's HousesType.          *
 *                                                                                             *
 * PlayerPtr is also set here.                                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      This routine assumes the 'Players' vector has been properly filled in with players'    *
 *      names, addresses, color, etc.                                                          *
 *      Also, it's assumed that the HouseClass's have all been created & initialized.          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1995 BRR : Created.                                                                 *
 *   07/14/1995 JLB : Records name of player in house structure.                               *
 *=============================================================================================*/
void Assign_Houses(void)
{
	int i;
	HousesType pref_house;

	//------------------------------------------------------------------------
	// Initialize
	//------------------------------------------------------------------------
	bool color_used[64];
	for (i = 0; i < ARRAY_SIZE(color_used); i++) {
		color_used[i] = false;
	}

	bool assigned[MAX_PLAYERS];
	for (i = 0; i < MAX_PLAYERS; i++) {
		assigned[i] = false;
	}

//	DebugString( "Assign_Houses()\n" );
	//------------------------------------------------------------------------
	// Assign each player in 'Players' to a multiplayer house.  Players will
	// be sorted by their chosen color value (this value must be unique among
	// all the players).
	//------------------------------------------------------------------------
	for (i = 0; i < Session.Players.Count(); i++) {

		//.....................................................................
		// Find the player with the lowest color index
		//.....................................................................
		int index = -1;
		int lowest_color = -1;
		for (int j = 0; j < Session.Players.Count(); j++) {
			//..................................................................
			// If we've already assigned this house, skip it.
			//..................................................................
			if (!assigned[j] && (lowest_color == -1 || Session.Players[j]->Player.Color < lowest_color)) {
				lowest_color = Session.Players[j]->Player.Color;
				index = j;
			}
		}
		assigned[index] = true;

		NodeNameType * player = Session.Players[index];

		//.....................................................................
		// Mark this player as having been assigned.
		//.....................................................................
		color_used[player->Player.Color] = true;

		/*
		**	Assign the lowest-color'd player to the next available slot in the
		**	HouseClass array.
		*/
		HouseClass * housep = new HouseClass(HouseTypes[player->Player.House]);

		housep->IniName = player->Name;
		housep->IsHuman = true;
		housep->Init_Data(player->Player.Color, (HousesType)player->Player.House, Session.Options.Credits);

		housep->Scheme = Session.Color_Index_To_Scheme(player->Player.Color);
		housep->Initialize_Radar_Color();

		if (index == 0) {
			PlayerPtr = housep;
			PlayerPtr->IsPlayerControl = true;
		}

		/*
		**	Convert the build level into an actual tech level to assign to the house.
		**	There isn't a one-to-one correspondence.
		*/
		housep->Control.TechLevel = BuildLevel;

		housep->Assign_Handicap(DIFF_NORMAL);

		housep->SpawnWaypoint = player->Player.SpawnChoice;

		//.....................................................................
		// Record where we placed this player
		//.....................................................................
		player->Player.ID = housep->HeapID;

//		DebugString( "Assigned ID of %i to %s\n", house, player->Name );
	}

	//------------------------------------------------------------------------
	// Now assign computer players to the remaining houses.
	//------------------------------------------------------------------------
	for (i = Session.Players.Count(); i < Session.Players.Count() + Session.Options.AIPlayers; i++) {

		/*
		 * A session source may have seated this computer player itself. What it left
		 * unnamed the game draws, exactly as it does for a game set up from the menu.
		 */
		int seatnum = i - Session.Players.Count();
		NodeNameType * seat = seatnum < Session.Computers.Count() ? Session.Computers[seatnum] : NULL;

		pref_house = (HousesType)Random_Pick(0, 1);
		if (seat != NULL && seat->Player.House != -1) {
			pref_house = (HousesType)seat->Player.House;
		}

		// Pick a color for this house; keep looping until we find one.
		int color = -1;
		for (;;) {
			color = Random_Pick(0, 7);
			if (color_used[color] == false) {
				break;
			}
		}

		/*
		 * A seated color is taken as written, repeats included, since a cooperative team
		 * shares one.
		 */
		if (seat != NULL && seat->Player.Color != -1) {
			color = seat->Player.Color;
		}
		color_used[color] = true;

		/*
		**	Set up the house
		*/
		HouseClass * housep = new HouseClass(HouseTypes[pref_house]);

		housep->Control.TechLevel = BuildLevel;
		housep->IsHuman = false;

		housep->Init_Data(color, pref_house, Session.Options.Credits);
		housep->Scheme = Session.Color_Index_To_Scheme(color);
		housep->Initialize_Radar_Color();
		housep->IniName = (seat != NULL && seat->Name[0] != '\0') ? seat->Name : Fetch_String(TXT_COMPUTER);

		if (Session.Type != GAME_NORMAL) {
			housep->IQ = Rule->MaxIQ;
		}

		DiffType difficulty = Scen->CDifficulty;
		if (Session.Players.Count() > 1 && Rule->IsCompEasyBonus && difficulty > DIFF_EASY) {
			difficulty = (DiffType)(difficulty - 1);
		}
		if (seat != NULL && seat->Player.Handicap >= 0) {
			difficulty = (DiffType)seat->Player.Handicap;
		}
		housep->Assign_Handicap(difficulty);

		if (seat != NULL) {
			housep->SpawnWaypoint = seat->Player.SpawnChoice;
			seat->Player.ID = housep->HeapID;
		}
	}

	// A seat's mask names other seats, not the houses they became.
	int seated = Session.Players.Count() + Session.Computers.Count();
	for (int seatnum = 0; seatnum < seated; seatnum++) {
		NodeNameType * node = Seated_Node(seatnum);
		if (node == NULL || node->Player.AlliesMask == 0) {
			continue;
		}

		for (int target = 0; target < seated; target++) {
			if (target == seatnum || (node->Player.AlliesMask & (1u << target)) == 0) {
				continue;
			}

			NodeNameType * other = Seated_Node(target);
			if (other != NULL) {
				Houses[node->Player.ID]->Make_Ally(Houses[other->Player.ID]);
			}
		}
	}

	HouseClass * neutral_house = new HouseClass(HouseTypes[HouseTypeClass::From_Name("Neutral")]);
	neutral_house->Scheme = Fetch_Scheme_Index_By_Name("LightGrey", NUM_INTENSITY_LEVELS);
	neutral_house->Initialize_Radar_Color();

	HouseClass * special_house = new HouseClass(HouseTypes[HouseTypeClass::From_Name("Special")]);
	special_house->Scheme = Fetch_Scheme_Index_By_Name("LightGrey", NUM_INTENSITY_LEVELS);
	special_house->Initialize_Radar_Color();
}


/***********************************************************************************************
 * Remove_AI_Players -- Removes the computer AI houses & their units                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static void Remove_AI_Players(void)
{
	for (int i = 0; i < Houses.Count(); i++) {
		int aicount = 0;
		HouseClass * housep = Houses[i];
		if (housep->IsHuman == false && !housep->Class->IsMultiplayPassive) {
			aicount++;
			if (aicount > Session.Options.AIPlayers) {
				housep->Clobber_All();
			}
		}
	}
}


/// <summary>
/// Makes up a shortfall of starting locations with open ground, since a map need not declare
/// a start position for everybody playing.
/// </summary>
/// <param name="waypts">The list of starting locations to append to.</param>
/// <param name="usable">How many of the locations may actually be started from; raised as
/// spots are appended.</param>
/// <param name="wanted">How many are needed.</param>
static void Append_Open_Start_Positions(DynamicVectorClass<Cell> & waypts, int & usable, int wanted)
{
	if (usable >= wanted) {
		return;
	}

	DebugString("Multiplayer start waypoint deficiency - looking for more start positions\n");

	while (usable < wanted) {
		Cell trycell = Cell(Map.MapRect.X + Random_Pick(10, Map.MapRect.Width - 10), Map.MapRect.Y + 10 + Random_Pick(0, Map.MapRect.Height - 10));

		trycell = Map.Nearby_Location(trycell, SPEED_TRACK, -1, MZONE_NORMAL, false, Point2D(8, 8));
		if (trycell != CELL_NONE) {
			waypts.Add(trycell);
			usable++;
			DebugString("Random multiplayer start waypoint added at cell %d,%d\n", trycell.X, trycell.Y);
		}
	}
}


/// <summary>
/// Fetches the starting locations a multiplayer game may use, making up any shortfall with
/// open ground. Keeping identity numbers each entry by waypoint, undeclared ones left as holes.
/// </summary>
/// <param name="official">Is this one of the maps that shipped with the game?</param>
/// <param name="keep_identity">Must an entry's place in the list be its waypoint number?</param>
/// <returns>Returns with the list of cells that players may be started from.</returns>
static DynamicVectorClass<Cell> Build_Start_Waypoint_List(bool official, bool keep_identity)
{
	DynamicVectorClass<Cell> waypts;

	if (keep_identity) {
		int usable = 0;
		for (int waycount = 0; waycount < MAX_PLAYERS; waycount++) {
			bool declared = Scen->Is_Valid_Waypoint(waycount);
			waypts.Add(declared ? Scen->Get_Waypoint_Cell(waycount) : CELL_NONE);
			if (declared) {
				usable++;
			}
		}

		/*
		 * The spots making up a shortfall are appended past the numbered ones, so that no
		 * number comes to mean a place the map never named.
		 */
		Append_Open_Start_Positions(waypts, usable, Session.Players.Count() + Session.Options.AIPlayers);

		return(waypts);
	}

	int num_waypts = 0;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (Scen->Is_Valid_Waypoint(i)) {
			num_waypts++;
		} else {
			break;
		}
	}

	/*
	**	Calculate the number of waypoints (as a minimum) that will be lifted from the
	**	mission file. Bias this number so that only the first 4 waypoints are used
	**	if there are 4 or fewer players. Unofficial maps will pick from all the
	**	available waypoints.
	*/
	int look_for = std::max(num_waypts, Session.Players.Count()+Session.Options.AIPlayers);
	if (!official) {
		look_for = MAX_PLAYERS;
	}

	for (int waycount = 0; waycount < look_for; waycount++) {
		if (Scen->Is_Valid_Waypoint(waycount)) {
			waypts.Add(Scen->Get_Waypoint_Cell(waycount));
			DebugString("Multiplayer start waypoint found at cell %d,%d\n", Scen->Get_Waypoint_Cell(waycount).X, Scen->Get_Waypoint_Cell(waycount).Y);
		}
	}

	int usable = waypts.Count();
	Append_Open_Start_Positions(waypts, usable, look_for);

	return(waypts);
}


/***********************************************************************************************
 * Create_Units -- Creates infantry & units, for non-base multiplayer                          *
 *                                                                                             *
 * This routine uses data tables to determine which units to create for either                 *
 * a GDI or NOD house, and how many of each.                                                   *
 *                                                                                             *
 * It also sets each house's FlagHome & FlagLocation to the Waypoint selected                  *
 * as that house's "home" cell.                                                                *
 *                                                                                             *
 * INPUT:   official -- Directs the placement logic to use the full set of waypoints rather    *
 *                      than biasing toward the first four.                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static void Create_Units(bool official)
{
	Cell centroid;			// centroid of this house's stuff
	DynamicVectorClass <TechnoClass*> deployed_list;
	int unit_count = Session.Options.UnitCount;

	if (Session.Options.Bases) {
		unit_count--;
	}

	DebugString ("Creating %d units\n", unit_count);
	DebugString ("UniqueID is %08x\n", Scen->UniqueID);

	int total_cost = 0;
	int total_objs = 0;
	for (int u = 0; u < UnitTypes.Count(); u++) {
		UnitTypeClass * utype = UnitTypes[u];
		if (utype->IsAllowedToStartInMultiplayer) {
			if (utype->Fetch_ID() != Rule->BaseUnit->Fetch_ID()) {
				total_cost += utype->Raw_Cost();
				total_objs++;
			}
		}
	}

	for (int inf = 0; inf < InfantryTypes.Count(); inf++) {
		InfantryTypeClass * itype = InfantryTypes[inf];
		if (itype->IsAllowedToStartInMultiplayer) {
			total_cost += itype->Raw_Cost();
			total_objs++;
		}
	}

	int average_cost = total_cost / total_objs;
	int max_value = unit_count * average_cost;

	/*
	 * A house only asks for a position by number when a session source chose one for it,
	 * and that is what decides whether the numbers have to keep their identity.
	 */
	bool choices = false;
	for (int index = 0; index < Houses.Count(); index++) {
		if (Houses[index] != NULL && Houses[index]->SpawnWaypoint >= 0) {
			choices = true;
			break;
		}
	}

	/*
	**	Build a list of the valid waypoints. This normally shouldn't be
	**	necessary because the scenario level designer should have assigned
	**	valid locations to the first N waypoints, but just in case, this
	**	loop verifies that.
	*/
	DynamicVectorClass<Cell> waypts = Build_Start_Waypoint_List(official, choices);
	bool taken[MAX_PLAYERS * 2];
	for (int index = 0; index < ARRAY_SIZE(taken); index++) {
		taken[index] = choices && index < waypts.Count() && waypts[index] == CELL_NONE;
	}

	/*
	 * A house that named a position holds it before anybody draws, so a house that named
	 * none cannot take one somebody asked for. Two naming the same position: first keeps it.
	 */
	int reserved[MAX_PLAYERS * 2];
	for (int index = 0; index < ARRAY_SIZE(reserved); index++) {
		reserved[index] = -1;
	}

	if (choices) {
		for (int index = 0; index < Houses.Count(); index++) {
			HouseClass * housep = Houses[index];
			if (housep == NULL || housep->Class->IsMultiplayPassive) {
				continue;
			}

			int spot = housep->SpawnWaypoint;
			if (spot >= 0 && spot < waypts.Count() && !taken[spot]) {
				reserved[spot] = index;
				taken[spot] = true;
			}
		}
	}

	/*
	**	Loop through all houses.  Computer-controlled houses, with Session.Options.Bases
	**	ON, are treated as though bases are OFF (since we have no base-building
	**	AI logic.)
	*/
	int numtaken = 0;
	for (HousesType house = HOUSE_FIRST; house < Houses.Count(); house++) {

		/*
		**	Get a pointer to this house; if there is none, go to the next house
		*/
		HouseClass * hptr = Houses[house];

		if (hptr->Class->IsMultiplayPassive) continue;

		DebugString("Generating units for house %d (%s)\n", (int)house, (char const *)hptr->Class->IniName);

		DynamicVectorClass<UnitTypeClass*> units;
		DynamicVectorClass<InfantryTypeClass*> infantry;
		unsigned int mask = 1 << hptr->Class->HeapID;

		for (int unit = 0; unit < UnitTypes.Count(); unit++) {
			UnitTypeClass * utype = UnitTypes[unit];
			if (utype->IsAllowedToStartInMultiplayer) {
				if (utype->Level <= hptr->Control.TechLevel && (utype->Ownable & mask)) {
					if (utype->Fetch_ID() != Rule->BaseUnit->Fetch_ID()) {
						units.Add(utype);
					}
				}
			}
		}

		for (int inf = 0; inf < InfantryTypes.Count(); inf++) {
			InfantryTypeClass * itype = InfantryTypes[inf];
			if (itype->IsAllowedToStartInMultiplayer) {
				if (itype->Level <= hptr->Control.TechLevel && (itype->Ownable & mask)) {
					infantry.Add(itype);
				}
			}
		}


		/*
		**	Pick the starting location for this house. The first house just picks
		**	one of the valid locations at random. The other houses pick the furthest
		**	wapoint from the existing houses.
		*/
		if (choices && hptr->SpawnWaypoint >= 0 && hptr->SpawnWaypoint < waypts.Count() &&
			reserved[hptr->SpawnWaypoint] == (int)house) {
			centroid = waypts[hptr->SpawnWaypoint];
			numtaken++;
		} else if (numtaken == 0) {
			int pick;
			do {
				pick = Random_Pick(0, waypts.Count() - 1);
			} while (taken[pick]);
			centroid = waypts[pick];
			taken[pick] = true;
			hptr->SpawnWaypoint = pick;
			numtaken++;
		} else {

			/*
			**	Set all waypoints to have a score of zero in preparation for giving
			**	a distance score to all waypoints.
			*/
			int score[26];
			memset(score, '\0', sizeof(score));

			/*
			**	Scan through all waypoints and give a score as a value of the sum
			**	of the distances from this waypoint to all taken waypoints.
			*/
			for (int index = 0; index < waypts.Count(); index++) {

				/*
				**	If this waypoint has not already been taken, then accumulate the
				**	sum of the distance between this waypoint and all other taken
				**	waypoints.
				*/
				if (!taken[index]) {
					for (int trypoint = 0; trypoint < waypts.Count(); trypoint++) {

						if (taken[trypoint] && waypts[trypoint] != CELL_NONE) {
							score[index] += Distance(waypts[index], waypts[trypoint]);
						}
					}
				}
			}

			/*
			**	Now find the waypoint with the largest score. This waypoint is the one
			**	that is furthest from all other taken waypoints.
			*/
			int best = 0;
			int bestvalue = 0;
			for (int searchindex = 0; searchindex < waypts.Count(); searchindex++) {
				if (waypts[searchindex] == CELL_NONE) {
					continue;
				}
				if (score[searchindex] > bestvalue || bestvalue == 0) {
					bestvalue = score[searchindex];
					best = searchindex;
				}
			}

			/*
			**	Assign this best position to the house.
			*/
			centroid = waypts[best];
			taken[best] = true;
			hptr->SpawnWaypoint = best;
			numtaken++;
		}

		/*
		**	Assign the center of this house to the waypoint location.
		*/
		hptr->Center = Coord(Cell(centroid));

		/*
		**	If Bases are ON, human & computer houses are treated differently
		*/
		if (Session.Options.Bases) {

			/*
			**	- For a human-controlled house:
			**	- Set 'scaleval' to 1
			**	- Create an MCV
			**	- Attach a flag to it for capture-the-flag mode
			*/
//			scaleval = 1;
			TechnoClass * obj = new UnitClass(Rule->BaseUnit, hptr);
			if (!obj->Unlimbo(Coord(centroid))) {
				if (!Scan_Place_Object(obj, centroid)) {
					delete obj;
					obj = NULL;
				}
			}
			if (obj != NULL) {
				hptr->FlagHome = CELL_NONE;
				hptr->FlagLocation = NULL;
				if (Scen->Special.IsCaptureTheFlag) {
					hptr->Flag_Attach((UnitClass *)obj, true);
				}
			}
		} else {

			/*
			**	If bases are OFF, set 'scaleval' to 1 & create a Mobile HQ for
			**	capture-the-flag mode.
			*/
//			scaleval = 1;
#ifdef TOFIX
			if (Special.IsCaptureTheFlag) {
				obj = new UnitClass(UNIT_TRUCK, house);
				obj->Unlimbo(Coord(centroid).As_Coord());
				hptr->FlagHome = 0;					// turn house's flag off
				hptr->FlagLocation = 0;
			}
#endif
		}

		int deployed_value = 0;
		deployed_list.Clear();

		/*
		**	Place objects; loop through all unit in this category
		*/
		while (deployed_value < max_value) {

			TechnoTypeClass const * tech = NULL;

			if (deployed_value < (max_value * 2) / 3 && units.Count() > 0) {
				tech = units[Random_Pick(0, units.Count() - 1)];
			} else if (infantry.Count() > 0) {
				tech = infantry[Random_Pick(0, infantry.Count() - 1)];
			}

			/*
			 * Create units (Note: Unlimbo calls Enter_Idle_Mode(), which
			 * assigns the unit to HUNT; we must use Set_Mission() to override
			 * this state.)
			 */
			ObjectClass * obj = tech->Create_One_Of(hptr);
			TechnoClass * tobj = Dynamic_Cast<TechnoClass *>(obj);

			if (!Scan_Place_Object(obj, centroid)) {
				delete obj;
			} else {
				DebugString("House %s deployed object %s\n", (char const *)hptr->Class->IniName, (char const *)tech->IniName);

				deployed_value += tech->Raw_Cost();
				deployed_list.Add(tobj);

				if (Scen->Special.IsInitialVeteran) {
					tobj->Veterancy.Set_Elite(true);
				}

				if (!hptr->Is_Human_Player()) {
					tobj->Set_Mission(MISSION_GUARD_AREA);
				} else {
					tobj->Set_Mission(MISSION_GUARD);
				}
			}
		}

		if (hptr->Is_Human_Player()) {
			for (int depl = 0; depl < deployed_list.Count(); depl++) {
				deployed_list[depl]->Scatter(COORD_NONE);
			}
		}
		deployed_list.Clear();
	}
	DebugString("Finished unit generation. Random number is %d\n", Random_Pick(0, 65535));

}


/***********************************************************************************************
 * Scan_Place_Object -- places an object >near< the given cell                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      obj      ptr to object to Unlimbo                                                      *
 *      cell      center of search area                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = object was placed; false = it wasn't                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
int Scan_Place_Object(ObjectClass * obj, Cell const & cell)
{
	int dist;               // for object placement
	FacingType rot;         // for object placement
	FacingType fcounter;    // for object placement
	int tryval;
	Cell newcell;
	TechnoClass * techno;
	int skipit;

	/*
	**	First try to unlimbo the object in the given cell.
	*/
	if (Map.In_Radar(cell)) {
		techno = Map[cell].Cell_Techno();
		if (!techno || (techno->RTTI == RTTI_INFANTRY &&
			obj->RTTI == RTTI_INFANTRY)) {
			if (obj->Unlimbo(Coord(cell, Map.Get_Height_GL(Coord(cell))))) {
				return(true);
			}
		}
	}

	/*
	**	Loop through distances from the given center cell; skip the center cell.
	**	For each distance, try placing the object along each rotational direction;
	**	if none are available, try each direction with a random scatter value.
	**	If that fails, go to the next distance.
	**	This ensures that the closest coordinates are filled first.
	*/
	for (dist = 1; dist < 32; dist++) {

		/*
		**	Pick a random starting direction
		*/
		rot = Random_Pick(FACING_N, FACING_NW);

		/*
		**	Try all directions twice
		*/
		for (tryval = 0; tryval < 2; tryval++) {

			/*
			**	Loop through all directions, at this distance.
			*/
			for (fcounter = FACING_N; fcounter <= FACING_NW; fcounter++) {

				skipit = false;

				/*
				**	Pick a coordinate along this directional axis
				*/
				newcell = Clip_Move(cell, rot, dist);

				/*
				**	If this is our second try at this distance, add a random scatter
				**	to the desired cell, so our units aren't all aligned along spokes.
				*/
				if (tryval > 0) {
					newcell = Clip_Scatter(newcell, 1);
				}

				/*
				**	If, by randomly scattering, we've chosen the exact center, skip
				**	it & try another direction.
				*/
				if (newcell == cell) {
					skipit = true;
				}

				if (!skipit) {
					/*
					**	Only attempt to Unlimbo the object if:
					**	- there is no techno in the cell
					**	- the techno in the cell & the object are both infantry
					*/
					techno = Map[newcell].Cell_Techno();
					if (techno == NULL || (techno->RTTI == RTTI_INFANTRY &&
						obj->RTTI == RTTI_INFANTRY)) {
						if (obj->Unlimbo(Coord(newcell, Map.Get_Height_GL(Coord(newcell))))) {
							return(true);
						}
					}
				}

				rot++;
				if (rot > FACING_NW) {
					rot = FACING_N;
				}
			}
		}
	}

	return(false);
}


/***********************************************************************************************
 * Clip_Scatter -- randomly scatters from given cell; won't fall off map                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      cell      cell to scatter from                                                         *
 *      maxdist   max distance to scatter                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      new cell number                                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static Cell const Clip_Scatter(Cell const & cell, int maxdist)
{
	int x, y;
	int xdist;
	int ydist;
	int xmin, xmax;
	int ymin, ymax;

	/*
	**	Get X & Y coords of given starting cell
	*/
	x = cell.X;
	y = cell.Y;

	/*
	**	Compute our x & y limits
	*/
	xmin = Map.MapRect.X;
	xmax = xmin + Map.MapRect.Width - 1;
	ymin = Map.MapRect.Y;
	ymax = ymin + Map.MapRect.Height - 1;

	/*
	**	Adjust the x-coordinate
	*/
	xdist = Random_Pick(0, maxdist);
	if (Percent_Chance(50)) {
		x += xdist;
		if (x > xmax) {
			x = xmax;
		}
	} else {
		x -= xdist;
		if (x < xmin) {
			x = xmin;
		}
	}

	/*
	**	Adjust the y-coordinate
	*/
	ydist = Random_Pick(0, maxdist);
	if (Percent_Chance(50)) {
		y += ydist;
		if (y > ymax) {
			y = ymax;
		}
	} else {
		y -= ydist;
		if (y < ymin) {
			y = ymin;
		}
	}

	return(Cell(x, y));
}


/***********************************************************************************************
 * Clip_Move -- moves in given direction from given cell; clips to map                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      cell      cell to start from                                                           *
 *      facing   direction to move                                                             *
 *      dist      distance to move                                                             *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      new cell number                                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static Cell const Clip_Move(Cell const & cell, FacingType facing, int dist)
{
	int x, y;
	int xmin, xmax;
	int ymin, ymax;

	/*
	**	Get X & Y coords of given starting cell
	*/
	x = cell.X;
	y = cell.Y;

	/*
	**	Compute our x & y limits
	*/
	xmin = Map.MapRect.X;
	xmax = xmin + Map.MapRect.Width - 1;
	ymin = Map.MapRect.Y;
	ymax = ymin + Map.MapRect.Height - 1;

	/*
	**	Adjust the x-coordinate
	*/
	switch (facing) {
		case FACING_N:
			y -= dist;
			break;

		case FACING_NE:
			x += dist;
			y -= dist;
			break;

		case FACING_E:
			x += dist;
			break;

		case FACING_SE:
			x += dist;
			y += dist;
			break;

		case FACING_S:
			y += dist;
			break;

		case FACING_SW:
			x -= dist;
			y += dist;
			break;

		case FACING_W:
			x -= dist;
			break;

		case FACING_NW:
			x -= dist;
			y -= dist;
			break;
	}

	/*
	**	Clip to the map
	*/
	if (x > xmax) {
		x = xmax;
	}
	if (x < xmin) {
		x = xmin;
	}

	if (y > ymax) {
		y = ymax;
	}
	if (y < ymin) {
		y = ymin;
	}

	return(Cell(x, y));
}


/// <summary>
/// Writes the scenario object to the save game stream.
/// The elapsed mission clock is halted across the write so that the time recorded is the
/// one the player will be given back when the game is resumed.
/// </summary>
void ScenarioClass::Save(IStream * stream) const
{
	DebugString("Scenario Save: ElapsedTimer = %d\n", (int)ElapsedTimer);
	ElapsedTimer.Stop();

	SaveStreamClass savestream(stream, SaveStreamClass::MODE_SAVE);

	/*
	 * One member list serves both directions, so it cannot be declared const even though
	 * writing changes nothing.
	 */
	const_cast<ScenarioClass *>(this)->Serialize(savestream);

	ElapsedTimer.Start();
}


/// <summary>
/// Reads the scenario object back from the save game stream.
/// The elapsed mission clock is halted across the read for the same reason it is halted
/// across the write, so that it does not advance over the value coming back in.
/// </summary>
void ScenarioClass::Load(IStream * stream)
{
	ElapsedTimer.Stop();

	SaveStreamClass savestream(stream, SaveStreamClass::MODE_LOAD);
	savestream.Set_Context("ScenarioClass");
	Serialize(savestream);

	ElapsedTimer.Start();
	DebugString("Scenario Load: ElapsedTimer = %d\n", (int)ElapsedTimer);
}


/// <summary>
/// Lists the members the scenario holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ScenarioClass::Serialize(SaveStreamClass & stream)
{
	stream.Serialize(Special);
	stream.Serialize(NextScenarioName);
	stream.Serialize(AltNextScenarioName);
	stream.Serialize(Home);
	stream.Serialize(AltHome);
	stream.Serialize(UniqueID);
	stream.Serialize(RandomNumber);
	stream.Serialize(Difficulty);
	stream.Serialize(CDifficulty);
	stream.Serialize(ElapsedTimer);
	stream.Serialize(Waypoint);
	stream.Serialize(MissionTimer);
	stream.Serialize(ShroudTimer);
	stream.Serialize(FogTimer);
	stream.Serialize(IceGrowthTimer);
	stream.Serialize(VeinGrowthTimer);
	stream.Serialize(AmbientChangeTimer);
	stream.Serialize(Scenario);
	stream.Serialize(Theater);
	stream.Serialize(ScenarioName);
	stream.Serialize(Description);
	stream.Serialize(IntroMovie);
	stream.Serialize(BriefMovie);
	stream.Serialize(WinMovie);
	stream.Serialize(LoseMovie);
	stream.Serialize(ActionMovie);
	stream.Serialize(PostScoreMovie);
	stream.Serialize(PreMapSelectMovie);
	stream.Serialize(BriefingText);
	stream.Serialize(TransitTheme);
	stream.Serialize(PlayerHouse);
	stream.Serialize(CarryOverPercent);
	stream.Serialize(CarryOverCap);
	stream.Serialize(Percent);
	stream.Serialize(GlobalFlags);
	stream.Serialize(LocalFlags);
	stream.Serialize(Views);
	stream.Serialize(BridgeCount);
	stream.Serialize(IsFreeRadar);
	stream.Serialize(IsTrainCargo);
	stream.Serialize(IsTibGrowth);
	stream.Serialize(IsVeinGrowth);
	stream.Serialize(IsIceGrowth);
	stream.Serialize(IsBridgeChanged);
	stream.Serialize(IsGlobalChanged);
	stream.Serialize(IsAmbientLightChanged);
	stream.Serialize(IsEndOfGame);
	stream.Serialize(IsInheritTimer);
	stream.Serialize(IsSkipScore);
	stream.Serialize(IsOneTimeOnly);
	stream.Serialize(IsNoMapSel);
	stream.Serialize(IsTruckCrate);
	stream.Serialize(IsMoneyTiberium);
	stream.Serialize(IsTiberiumDeathToVisceroid);
	stream.Serialize(IsIgnoreGlobalAITriggers);
	stream.Serialize(IsGDI);
	stream.Serialize(IsMultiplayerOnly);
	stream.Serialize(IsRandom);
	stream.Serialize(IsCrateBeenPickedUp);
	stream.Serialize(FadeTimer);
	stream.Serialize(Campaign);
	stream.Serialize(StartingDropships);
	stream.Serialize(AllowableUnits);
	stream.Serialize(AllowableUnitMaximums);
	stream.Serialize(AllowableUnitCounts);
	stream.Serialize(AmbientLight);
	stream.Serialize(CurrentAmbientLight);
	stream.Serialize(DesiredAmbientLight);
	stream.Serialize(RedTint);
	stream.Serialize(GreenTint);
	stream.Serialize(BlueTint);
	stream.Serialize(GroundLight);
	stream.Serialize(LevelLight);
	stream.Serialize(IonAmbientLight);
	stream.Serialize(IonRedTint);
	stream.Serialize(IonGreenTint);
	stream.Serialize(IonBlueTint);
	stream.Serialize(IonGroundLight);
	stream.Serialize(IonLevelLight);
	stream.Serialize(IsReadingScenario);
	stream.Serialize(InitTime);
	stream.Serialize(RequiredAddOn);
	stream.Serialize(SpeechSide);
	stream.Serialize(Stage);
	stream.Serialize(IsInputLocked);
}


/***********************************************************************************************
 * ScenarioClass::Set_Global_To -- Set scenario global to value specified.                     *
 *                                                                                             *
 *    This routine will set the global flag to the falue (true/false) specified. It will       *
 *    also scan for and spring any triggers that are dependant upon that global.               *
 *                                                                                             *
 * INPUT:   global   -- The global flag to change.                                             *
 *                                                                                             *
 *          value    -- The value to change the global flag to.                                *
 *                                                                                             *
 * OUTPUT:  Returns with the previous value of the flag.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ScenarioClass::Set_Global_To(int global, bool value)
{
	if ((unsigned)global < ARRAY_SIZE(Scen->GlobalFlags)) {

		bool previous = GlobalFlags[global].Value;
		if (previous != value) {
			GlobalFlags[global].Value = value;
			IsGlobalChanged = true;

			/*
			**	Special case to scan through all triggers and if any are found that depend on this
			**	global being set/cleared, then if there is an elapsed time event associated, it
			**	will be reset at this time.
			*/
			TagClass::All_Timer_Global_Reset(global);
		}
		return(previous);
	}
	return(false);
}


/// <summary>
/// Sets a global scenario variable by name.
/// This routine is used by trigger and script code that refers to a global by the
/// designer's name rather than by its number.
/// </summary>
/// <returns>bool; What was the previous state of the global? Returns true if there is
/// no global by that name.</returns>
bool ScenarioClass::Set_Global_To(char const * variable_name, bool value)
{
	int index = Global_From_Name(variable_name);
	if (index != -1) {
		return(Set_Global_To(index, value));
	}
	return(true);
}


/// <summary>
/// Fetches the current state of a global scenario variable.
/// </summary>
/// <param name="value">Reference to be filled in with the state of the global.</param>
/// <returns>bool; Was the global index a legal one?</returns>
bool ScenarioClass::Fetch_Global_Value(int global, bool & value)
{
	if (global >= 0 && global < ARRAY_SIZE(GlobalFlags)) {
		value = GlobalFlags[global].Value;
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the current state of a global scenario variable by name.
/// </summary>
/// <param name="value">Reference to be filled in with the state of the global.</param>
/// <returns>bool; Was a global of that name found?</returns>
bool ScenarioClass::Fetch_Global_Value(char const * variable_name, bool & value)
{
	int index = Global_From_Name(variable_name);
	if (index != -1) {
		return(Fetch_Global_Value(index, value));
	}
	return(false);
}


/// <summary>
/// Finds the index of the global variable that carries the specified name.
/// The names are supplied by the rules database, so this is how designer-authored logic
/// reaches a global without knowing its number.
/// </summary>
/// <returns>Returns with the index of the global, or -1 if no global carries that
/// name.</returns>
int ScenarioClass::Global_From_Name(char const * variable_name)
{
	for (int i = 0; i < ARRAY_SIZE(GlobalFlags); i++) {
		if (!strcmp(variable_name, GlobalFlags[i].VariableName)) {
			return(i);
		}
	}
	return(-1);
}


/// <summary>
/// Reads the global variable names from an INI database.
/// This routine picks up the name assigned to each global flag so that triggers and
/// scripts can refer to the globals by name.
/// </summary>
bool ScenarioClass::Read_Global_INI(CCINIClass const & ini)
{
	char const * const SECTION = "VariableNames";
	int length = std::min(ini.Entry_Count(SECTION), ARRAY_SIZE(GlobalFlags));
	for (int i = 0; i < length; i++) {
		char const * entry = ini.Get_Entry(SECTION, i);
		int index = atoi(entry);

		ini.Get_String(SECTION, entry, NULL, GlobalFlags[index].VariableName, sizeof(GlobalFlags[index].VariableName));
	}
	return(true);
}


/// <summary>
/// Sets one of the scenario's local variables.
/// This routine notifies the trigger system whenever the flag actually changes, so that
/// any elapsed time event keyed to this local is restarted.
/// </summary>
/// <returns>bool; What was the previous state of the local? Returns false if the index
/// is out of range.</returns>
bool ScenarioClass::Set_Local_To(int index, bool value)
{
	if ((unsigned)index < ARRAY_SIZE(Scen->LocalFlags)) {

		bool previous = LocalFlags[index].Value;
		if (previous != value) {
			LocalFlags[index].Value = value;
			IsGlobalChanged = true;

			/*
			**	Special case to scan through all triggers and if any are found that depend on this
			**	global being set/cleared, then if there is an elapsed time event associated, it
			**	will be reset at this time.
			*/
			TagClass::All_Timer_Local_Reset(index);
		}
		return(previous);
	}
	return(false);
}


/// <summary>
/// Sets one of the scenario's local variables by name.
/// </summary>
/// <returns>bool; What was the previous state of the local? Returns true if there is no
/// local by that name.</returns>
bool ScenarioClass::Set_Local_To(char const * variable_name, bool value)
{
	int index = Local_From_Name(variable_name);
	if (index != -1) {
		return(Set_Local_To(index, value));
	}
	return(true);
}


/// <summary>
/// Fetches the current state of a scenario local variable.
/// </summary>
/// <param name="value">Reference to be filled in with the state of the local.</param>
/// <returns>bool; Was the local index a legal one?</returns>
bool ScenarioClass::Fetch_Local_Value(int index, bool & value)
{
	if (index >= 0 && index < ARRAY_SIZE(LocalFlags)) {
		value = LocalFlags[index].Value;
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the current state of a scenario local variable by name.
/// </summary>
/// <param name="value">Reference to be filled in with the state of the local.</param>
/// <returns>bool; Was a local of that name found?</returns>
bool ScenarioClass::Fetch_Local_Value(char const * variable_name, bool & value)
{
	int index = Local_From_Name(variable_name);
	if (index != -1) {
		return(Fetch_Local_Value(index, value));
	}
	return(false);
}


/// <summary>
/// Finds the index of the local variable that carries the specified name.
/// </summary>
/// <returns>Returns with the index of the local, or -1 if no local carries that
/// name.</returns>
int ScenarioClass::Local_From_Name(char const * variable_name)
{
	for (int i = 0; i < ARRAY_SIZE(LocalFlags); i++) {
		if (strcmp(variable_name, LocalFlags[i].VariableName) == 0) {
			return(i);
		}
	}
	return(-1);
}


/// <summary>
/// Reads the scenario's local variables from an INI database.
/// Every local is cleared first, then those the designer named are given their name and
/// their starting state.
/// </summary>
bool ScenarioClass::Read_Local_INI(CCINIClass const & ini)
{
	char buffer[128];
	int index;

	for (index = 0; index < ARRAY_SIZE(LocalFlags); index++) {
		LocalFlags[index].VariableName[0] = '\0';
	}

	char const * const SECTION = "VariableNames";
	int length = std::min(ini.Entry_Count(SECTION), ARRAY_SIZE(LocalFlags));
	for (index = 0; index < length; index++) {
		char const * entry = ini.Get_Entry(SECTION, index);
		int index = atoi(entry);

		ini.Get_String(SECTION, entry, NULL, buffer, sizeof(buffer));
		char * token = strtok(buffer, ",");
		strcpy(LocalFlags[index].VariableName, token);

		token = strtok(NULL, ",");
		if (token != NULL) {
			LocalFlags[index].Value = (atoi(token) == 0) ? false : true;
		}
	}

	return(true);
}


/// <summary>
/// Writes the scenario's named local variables to an INI database.
/// Locals that the designer never named are not recorded.
/// </summary>
bool ScenarioClass::Write_Local_INI(CCINIClass & ini) const
{
	static char const * const SECTION = "VariableNames";
	char index_buffer[10];
	char buffer[128];

	ini.Clear(SECTION);
	int length = ARRAY_SIZE(LocalFlags);
	for (int index = 0; index < length; index++) {
		if (LocalFlags[index].VariableName[0] != '\0') {
			wsprintf(index_buffer, "%d", index);
			wsprintf(buffer, "%s,%d", LocalFlags[index].VariableName, LocalFlags[index].Value ? 1 : 0);
			ini.Put_String(SECTION, index_buffer, buffer);
		}
	}
	return(true);
}


/// <summary>
/// Counts the local variables that the designer has named.
/// </summary>
/// <returns>Returns with the number of locals this scenario puts to use.</returns>
int ScenarioClass::Used_Local_Count(void)
{
	int count = 0;
	for (int i = 0; i < ARRAY_SIZE(LocalFlags); i++) {
		if (LocalFlags[i].VariableName[0] != '\0') {
			count++;
		}
	}
	return(count);
}


/// <summary>
/// Finds a local variable slot that has not been claimed.
/// This routine is used by the editor when the designer asks for another local.
/// </summary>
/// <returns>Returns with the index of the first free local, or -1 if all of them are
/// in use.</returns>
int ScenarioClass::First_Unused_Local(void) const
{
	for (int index = 0; index < ARRAY_SIZE(LocalFlags); index++) {
		if (LocalFlags[index].VariableName[0] == '\0') {
			return(index);
		}
	}
	return(-1);
}


/// <summary>
/// Reads the scenario's parameters from an INI database.
/// This routine picks up everything that describes the scenario itself -- the movies, the
/// theme, the carry over money rules, the growth switches and the ambient lighting. The
/// player's house is established here as well, since the objects created afterward attach
/// themselves to it.
/// </summary>
bool ScenarioClass::Read_INI(CCINIClass const & ini)
{
	char const * const BASIC = "Basic";

	int i;

	Special.Read_INI(ini);
	ini.Get_String(BASIC, "Name", "<none>", Description, sizeof(Description));
	ini.Get_TextBlock("Briefing", BriefingText, sizeof(BriefingText));
	ini.Get_String(BASIC, "NextScenario", NextScenarioName, NextScenarioName, sizeof(NextScenarioName));
	ini.Get_String(BASIC, "AltNextScenario", AltNextScenarioName, AltNextScenarioName, sizeof(AltNextScenarioName));
	IntroMovie = ini.Get_VQType(BASIC, "Intro", IntroMovie);
	BriefMovie = ini.Get_VQType(BASIC, "Brief", BriefMovie);
	WinMovie = ini.Get_VQType(BASIC, "Win", WinMovie);
	LoseMovie = ini.Get_VQType(BASIC, "Lose", LoseMovie);
	ActionMovie = ini.Get_VQType(BASIC, "Action", ActionMovie);
	PostScoreMovie = ini.Get_VQType(BASIC, "PostScore", PostScoreMovie);
	PreMapSelectMovie = ini.Get_VQType(BASIC, "PreMapSelect", PreMapSelectMovie);

	IsMultiplayerOnly = ini.Get_Bool(BASIC, "MultiplayerOnly", IsMultiplayerOnly);
	IsInheritTimer = ini.Get_Bool(BASIC, "TimerInherit", IsInheritTimer);
	IsEndOfGame = ini.Get_Bool(BASIC, "EndOfGame", IsEndOfGame);
	TransitTheme = ini.Get_ThemeType(BASIC, "Theme", TransitTheme);
	NewINIFormat = ini.Get_Int(BASIC, "NewINIFormat", 0);
	CarryOverPercent = ini.Get_Float(BASIC, "CarryOverMoney", CarryOverPercent);
	CarryOverPercent = std::min(CarryOverPercent, 1.0);
	CarryOverCap = ini.Get_Int(BASIC, "CarryOverCap", CarryOverCap);
	IsSkipScore = ini.Get_Bool(BASIC, "SkipScore", IsSkipScore);
	IsOneTimeOnly = ini.Get_Bool(BASIC, "OneTimeOnly", IsOneTimeOnly);
	IsNoMapSel = ini.Get_Bool(BASIC, "SkipMapSelect", IsNoMapSel);
	IsTruckCrate = ini.Get_Bool(BASIC, "TruckCrate", IsTruckCrate);
	IsTrainCargo = ini.Get_Bool(BASIC, "TrainCrate", IsTrainCargo);
	IsMoneyTiberium = ini.Get_Bool(BASIC, "FillSilos", IsMoneyTiberium);
	IsIgnoreGlobalAITriggers = ini.Get_Bool(BASIC, "IgnoreGlobalAITriggers", IsIgnoreGlobalAITriggers);
	Percent = ini.Get_Int(BASIC, "Percent", Percent);
	StartingDropships = ini.Get_Int(BASIC, "StartingDropships", StartingDropships);
	AllowableUnits = ini.Get_TechnoType_List(BASIC, "AllowableUnits", AllowableUnits);
	AllowableUnitMaximums = ini.Get_IntList(BASIC, "AllowableUnitMaximums", AllowableUnitMaximums);
	IsTibGrowth = ini.Get_Bool(BASIC, "TiberiumGrowthEnabled", IsTibGrowth);
	IsVeinGrowth = ini.Get_Bool(BASIC, "VeinGrowthEnabled", IsVeinGrowth);
	IsIceGrowth = ini.Get_Bool(BASIC, "IceGrowthEnabled", IsIceGrowth);
	IsTiberiumDeathToVisceroid = ini.Get_Bool(BASIC, "TiberiumDeathToVisceroid", IsTiberiumDeathToVisceroid);
	IsFreeRadar = ini.Get_Bool(BASIC, "FreeRadar", IsFreeRadar);
	Home = ini.Get_Int(BASIC, "HomeCell", Home);
	AltHome = ini.Get_Int(BASIC, "AltHomeCell", AltHome);
	RequiredAddOn = (AddonType)ini.Get_Int(BASIC, "RequiredAddOn", RequiredAddOn);

	ini.Get_Bool(BASIC, "CivEvac", false);

	for (i = 0; i < AllowableUnits.Count() - AllowableUnitMaximums.Count(); i++) {
		AllowableUnitMaximums.Add(-1);
	}

	for (i = 0; i < AllowableUnitMaximums.Count(); i++) {
		AllowableUnitCounts.Add(0);
	}

	Read_Local_INI(ini);
	Call_Back();

	char const * const LIGHTING = "Lighting";

	AmbientLight        = (int)(100.0 * ini.Get_Float(LIGHTING, "Ambient", AmbientLight / 100.0));
	CurrentAmbientLight = AmbientLight;
	DesiredAmbientLight = AmbientLight;
	RedTint         = (int)(100.0 * ini.Get_Float(LIGHTING, "Red", RedTint / 100.0));
	GreenTint       = (int)(100.0 * ini.Get_Float(LIGHTING, "Green", GreenTint / 100.0));
	BlueTint        = (int)(100.0 * ini.Get_Float(LIGHTING, "Blue", BlueTint / 100.0));
	GroundLight     = (int)(NORMAL_LIGHT * ini.Get_Float(LIGHTING, "Ground", GroundLight / NORMAL_LIGHT));
	LevelLight      = (int)(NORMAL_LIGHT * ini.Get_Float(LIGHTING, "Level", LevelLight / NORMAL_LIGHT));
	IonAmbientLight = (int)(100.0 * ini.Get_Float(LIGHTING, "IonAmbient", AmbientLight / 100.0));
	IonRedTint      = (int)(100.0 * ini.Get_Float(LIGHTING, "IonRed", RedTint / 100.0));
	IonGreenTint    = (int)(100.0 * ini.Get_Float(LIGHTING, "IonGreen", GreenTint / 100.0));
	IonBlueTint     = (int)(100.0 * ini.Get_Float(LIGHTING, "IonBlue", BlueTint / 100.0));
	IonGroundLight  = (int)(NORMAL_LIGHT * ini.Get_Float(LIGHTING, "IonGround", GroundLight / NORMAL_LIGHT));
	IonLevelLight   = (int)(NORMAL_LIGHT * ini.Get_Float(LIGHTING, "IonLevel", LevelLight / NORMAL_LIGHT));

	Session.Update_Progress(55);
	Call_Back();

	/*
	**	Assign PlayerPtr by reading the player's house from the INI;
	**	Must be done before any TechnoClass objects are created.
	*/
	if (Session.Type == GAME_NORMAL) {
		HousesType house = ini.Get_HousesType(BASIC, "Player", HOUSE_NONE);
		if (house == HOUSE_NONE) {
			house = HOUSE_FIRST;
		}
		PlayerHouse = house;
		PlayerPtr = House_From_HousesType(house);

	} else {
		Assign_Houses();
	}

	PlayerPtr->IsHuman = true;
	PlayerPtr->IsPlayerControl = true;
	PlayerPtr->CurrentDropship = 0;

	Session.Update_Progress(58);
	Call_Back();

	if (Debug_Map) {
		IsometricTileTypeClass::Init_Drawers();
	}

	Session.Update_Progress(60);
	Call_Back();

	return(true);
}


/// <summary>
/// Writes the scenario's parameters to an INI database.
/// This routine is used by the map editor when it saves a scenario back out.
/// </summary>
/// <param name="mplayer">Is the scenario being saved as a multiplayer map?</param>
bool ScenarioClass::Write_INI(CCINIClass & ini, bool mplayer) const
{
	static char const * const BASIC = "Basic";
	ini.Clear(BASIC);

	ini.Clear(BASIC, "CivEvac");

	if (strlen(Scen->BriefingText)) {
		ini.Put_TextBlock("Briefing", Scen->BriefingText);
	}

	Special.Write_INI(ini);

	ini.Put_Int(BASIC, "RequiredAddOn", RequiredAddOn, ADDON_BASE_GAME);
	ini.Put_Side(BASIC, "SpeechSide", Scen->SpeechSide);
	ini.Put_String(BASIC, "NextScenario", NextScenarioName);
	ini.Put_String(BASIC, "AltNextScenario", AltNextScenarioName);
	ini.Put_String(BASIC, "Name", Description);
	ini.Put_Int(BASIC, "NewINIFormat", 4);
	ini.Put_Int(BASIC, "CarryOverCap", CarryOverCap / 100);
	ini.Put_Bool(BASIC, "EndOfGame", IsEndOfGame);
	ini.Put_Bool(BASIC, "SkipScore", IsSkipScore);
	ini.Put_Bool(BASIC, "OneTimeOnly", IsOneTimeOnly);
	ini.Put_Bool(BASIC, "SkipMapSelect", IsNoMapSel);
	ini.Put_Bool(BASIC, "Official", true);
	ini.Put_Bool(BASIC, "IgnoreGlobalAITriggers", IsIgnoreGlobalAITriggers);
	ini.Put_Bool(BASIC, "TruckCrate", IsTruckCrate);
	ini.Put_Bool(BASIC, "TrainCrate", IsTrainCargo);
	ini.Put_Int(BASIC, "Percent", Percent);
	ini.Put_TechnoType_List(BASIC, "AllowableUnits", AllowableUnits);
	ini.Put_IntList(BASIC, "AllowableUnitMaximums", AllowableUnitMaximums);

	if (!mplayer) {
		ini.Put_HousesType(BASIC, "Player", PlayerPtr->Class->House);
		ini.Put_VQType(BASIC, "Intro", IntroMovie);
		ini.Put_VQType(BASIC, "Brief", BriefMovie);
		ini.Put_VQType(BASIC, "Win", WinMovie);
		ini.Put_VQType(BASIC, "Lose", LoseMovie);
		ini.Put_VQType(BASIC, "Action", ActionMovie);
		ini.Put_VQType(BASIC, "PostScore", PostScoreMovie);
		ini.Put_VQType(BASIC, "PreMapSelect", PreMapSelectMovie);
		ini.Put_ThemeType(BASIC, "Theme", TransitTheme);
		ini.Put_Float(BASIC, "CarryOverMoney", CarryOverPercent);
		ini.Put_Bool(BASIC, "TimerInherit", IsInheritTimer);
		ini.Put_Bool(BASIC, "FillSilos", IsMoneyTiberium);
		ini.Put_Int(BASIC, "StartingDropships", StartingDropships);
		ini.Put_Int(BASIC, "HomeCell", Home);
		ini.Put_Int(BASIC, "AltHomeCell", AltHome);
	}

	ini.Put_Int(BASIC, "MultiplayerOnly", IsMultiplayerOnly);
	ini.Put_Bool(BASIC, "TiberiumGrowthEnabled", IsTibGrowth);
	ini.Put_Bool(BASIC, "VeinGrowthEnabled", IsVeinGrowth);
	ini.Put_Bool(BASIC, "IceGrowthEnabled", IsIceGrowth);
	ini.Put_Bool(BASIC, "TiberiumDeathToVisceroid", IsTiberiumDeathToVisceroid);
	ini.Put_Bool(BASIC, "FreeRadar", IsFreeRadar);

	ini.Put_Int(BASIC, "InitTime", InitTime);

	Write_Local_INI(ini);

	char const * const LIGHTING = "Lighting";
	ini.Clear(LIGHTING);
	ini.Put_Float(LIGHTING, "Ambient", ((double)AmbientLight) / 100.0);
	ini.Put_Float(LIGHTING, "Red",     ((double)RedTint)      / 100.0);
	ini.Put_Float(LIGHTING, "Green",   ((double)GreenTint)    / 100.0);
	ini.Put_Float(LIGHTING, "Blue",    ((double)BlueTint)     / 100.0);
	ini.Put_Float(LIGHTING, "Ground",  ((double)GroundLight)  / NORMAL_LIGHT);
	ini.Put_Float(LIGHTING, "Level",   ((double)LevelLight)   / NORMAL_LIGHT);
	ini.Put_Float(LIGHTING, "IonAmbient", ((double)IonAmbientLight) / 100.0);
	ini.Put_Float(LIGHTING, "IonRed",     ((double)IonRedTint)      / 100.0);
	ini.Put_Float(LIGHTING, "IonGreen",   ((double)IonGreenTint)    / 100.0);
	ini.Put_Float(LIGHTING, "IonBlue",    ((double)IonBlueTint)     / 100.0);
	ini.Put_Float(LIGHTING, "IonGround",  ((double)IonGroundLight)  / NORMAL_LIGHT);
	ini.Put_Float(LIGHTING, "IonLevel",   ((double)IonLevelLight)   / NORMAL_LIGHT);
	return(true);
}


/// <summary>
/// Adjusts a time value to suit the speed of this machine.
/// Use this routine where a delay should feel the same regardless of the processor it
/// runs on. The value is scaled against a normalized machine speed.
/// </summary>
/// <returns>Returns with the time value scaled for this machine.</returns>
int Adjust_To_CPU_Timing(int time)
{
	static const double P_SIX_TWEAK = .8;
	static const double TIME_SCALE = 200;

	int cpu_type;
	bool mmx;

	int speed = 0;

	Get_CPU_Type(cpu_type, mmx, NULL, 0);

	if (cpu_type > PROC_PENTIUM_PRO) {
		time = (int)(time * P_SIX_TWEAK);
	}

	double hiscale = ((__int64)2 << 31);

	unsigned int lo;
	unsigned int hi;
	lo = Get_CPU_Rate(hi);

	if (lo != 0 || hi != 0) {
		speed = (int)(((double)lo + ((double)hi * hiscale)) / (double)1000000);
	}

	time = (int)(time * TIME_SCALE / speed);
	return(time);
}


/// <summary>
/// Submits the scenario state to a CRC calculation.
/// This routine is used by the multiplayer sync check, which compares the CRC taken on
/// each machine in order to spot a game that has drifted out of step.
/// </summary>
void ScenarioClass::Compute_CRC(CRCEngine & crc) const
{
	crc(Description);
	crc(IntroMovie);
	crc(BriefMovie);
	crc(BriefingText);
	crc(WinMovie);
	crc(LoseMovie);
	crc(ActionMovie);
	crc(PostScoreMovie);
	crc(PreMapSelectMovie);
	crc(PlayerHouse);
	crc(Percent);
	crc(TransitTheme);
	crc(CarryOverPercent);
	crc(IsMultiplayerOnly);
	crc(IsInheritTimer);
	crc(CarryOverCap / 100);
	crc(BridgeCount);
	crc(IsEndOfGame);
	crc(Campaign);
	crc(IsSkipScore);
	crc(IsOneTimeOnly);
	crc(IsNoMapSel);
	crc(IsMoneyTiberium);
	crc(IsIgnoreGlobalAITriggers);
	crc(IsTruckCrate);
	crc(Percent);
	crc(StartingDropships);
	crc((int)MissionTimer);
	crc((int)ShroudTimer);
	crc((int)FogTimer);
	crc((int)IceGrowthTimer);
	crc((int)VeinGrowthTimer);
	crc((int)ElapsedTimer);
	crc(Difficulty);
	crc(CDifficulty);
	crc(Theater);
	crc(GroundLight);
	crc(LevelLight);
	crc(IsFreeRadar);
	crc(IonAmbientLight);
	crc(IonRedTint);
	crc(IonGreenTint);
	crc(IonBlueTint);
	crc(IonGroundLight);
	crc(IonLevelLight);
	crc(AmbientLight);
	crc(CurrentAmbientLight);
	crc(DesiredAmbientLight);
}


/// <summary>
/// Fetches a unique identification number.
/// This routine hands out a number that nothing else in the scenario shares.
/// </summary>
/// <returns>Returns with the newly allocated identifier.</returns>
int ScenarioClass::Get_Unique_ID(void)
{
	UniqueID++;

	return(UniqueID);
}


/// <summary>
/// Fetches the cell that a waypoint is anchored to.
/// </summary>
/// <returns>Returns with the cell the waypoint occupies.</returns>
/// <remarks>Only call this routine for a waypoint that has been placed on the map.</remarks>
Cell ScenarioClass::Get_Waypoint_Cell(WAYPOINT waypoint) const
{
	assert(waypoint < ARRAY_SIZE(Waypoint));
	assert(waypoint >= 0);
	assert(Waypoint[waypoint] != CELL_NONE);

	return(Waypoint[waypoint]);
}


/// <summary>
/// Fetches the waypoint's cell as a target.
/// This routine is used where mission and trigger logic needs something it can be sent to
/// rather than a bare cell number.
/// </summary>
/// <returns>Returns with a pointer to the cell object the waypoint occupies.</returns>
/// <remarks>Only call this routine for a waypoint that has been placed on the map.</remarks>
AbstractClass * ScenarioClass::Get_Waypoint_Target(WAYPOINT waypoint) const
{
	assert(waypoint < ARRAY_SIZE(Waypoint));
	assert(waypoint >= 0);
	assert(Waypoint[waypoint] != CELL_NONE);

	return(&Map[Waypoint[waypoint]]);
}


/// <summary>
/// Fetches the coordinate at the center of a waypoint's cell.
/// A waypoint that sits under a bridge is lifted to the bridge deck, so that whatever is
/// sent there travels over the span rather than beneath it.
/// </summary>
/// <returns>Returns with the coordinate of the waypoint.</returns>
/// <remarks>Only call this routine for a waypoint that has been placed on the map.</remarks>
Coord ScenarioClass::Get_Waypoint_Coord(WAYPOINT waypoint) const
{
	assert(waypoint < ARRAY_SIZE(Waypoint));
	assert(waypoint >= 0);
	assert(Waypoint[waypoint] != CELL_NONE);

	CellClass const * cell = &Map[Waypoint[waypoint]];
	Coord coord = cell->Center_Coord();

	if (cell->IsUnderBridge || cell->WasUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}
	return(coord);
}


/// <summary>
/// Removes every waypoint from the scenario.
/// This routine is used when the scenario is being torn down to make room for another.
/// </summary>
void ScenarioClass::Clear_All_Waypoints(void)
{
	for (int index = 0; index < ARRAY_SIZE(Waypoint); index++) {
		Waypoint[index] = CELL_NONE;
	}
}


/// <summary>
/// Has this waypoint been placed on the map?
/// </summary>
/// <returns>bool; Does the waypoint refer to a real cell?</returns>
bool ScenarioClass::Is_Valid_Waypoint(WAYPOINT waypoint) const
{
	if (waypoint < ARRAY_SIZE(Waypoint) &&
		waypoint >= 0 &&
		Waypoint[waypoint] != CELL_NONE) {

		return(true);
	}
	return(false);
}


/// <summary>
/// Reads the waypoint list from an INI database.
/// The cells named are also flagged as waypoints so that the editor can display them.
/// </summary>
void ScenarioClass::Read_Waypoints(CCINIClass const & ini)
{
	char buf[20];

	for (int i = 0; i < WAYPT_COUNT; i++) {
		wsprintf(buf, "%d", i);
		int val = ini.Get_Int("Waypoints", buf, 0);
		if (val == 0) {
			Waypoint[i] = CELL_NONE;
		} else {
			Waypoint[i] = Cell(val % 1000, val / 1000);
		}

		if (Is_Valid_Waypoint(i)) {
			Get_Waypoint_CellClass(i)->IsWaypoint = 1;
		}
	}
}


/// <summary>
/// Writes the placed waypoints to an INI database.
/// Waypoints the designer never placed are not recorded.
/// </summary>
void ScenarioClass::Write_Waypoints(CCINIClass & ini) const
{
	static char const * const WAYNAME = "Waypoints";
	char entry[32];

	ini.Clear(WAYNAME);
	for (int i = 0; i < WAYPT_COUNT; i++) {
		if (Waypoint[i] != CELL_NONE) {
			wsprintf(entry, "%d", i);
			ini.Put_Int(WAYNAME, entry, Waypoint[i].Y * 1000 + Waypoint[i].X);
		}
	}
}


/// <summary>
/// Removes a single waypoint from the scenario.
/// </summary>
void ScenarioClass::Clear_Waypoint(WAYPOINT waypoint)
{
	assert(waypoint < ARRAY_SIZE(Waypoint));
	assert(waypoint >= 0);
	assert(Waypoint[waypoint] != CELL_NONE);

	Waypoint[waypoint] = CELL_NONE;
}


/// <summary>
/// Anchors a waypoint to the specified cell.
/// </summary>
void ScenarioClass::Set_Waypoint(WAYPOINT waypoint, Cell cell)
{
	assert(waypoint < ARRAY_SIZE(Waypoint));
	assert(waypoint >= 0);

	Waypoint[waypoint] = cell;
}


/// <summary>
/// Fetches the cell object that a waypoint is anchored to.
/// </summary>
/// <returns>Returns with a pointer to the cell the waypoint occupies.</returns>
/// <remarks>Only call this routine for a waypoint that has been placed on the map.</remarks>
CellClass * ScenarioClass::Get_Waypoint_CellClass(WAYPOINT waypoint) const
{
	assert(waypoint < ARRAY_SIZE(Waypoint));
	assert(waypoint >= 0);
	assert(Waypoint[waypoint] != CELL_NONE);

	return(&Map[Waypoint[waypoint]]);
}


/// <summary>
/// Finds the name of the first waypoint that has not been placed.
/// This routine is used by the editor when it must offer the designer a fresh waypoint.
/// </summary>
/// <returns>Returns with the name of the first unused waypoint, or an empty string if
/// every waypoint has been placed.</returns>
char const * ScenarioClass::First_Unused_Waypoint_Name(void) const
{
	for (int index = 0; index < ARRAY_SIZE(Waypoint); index++) {
		if (!Is_Valid_Waypoint(index)) {
			return(Waypoint_To_Name(index));
		}
	}
	return("");
}
