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

/* $Header: /CounterStrike/INIT.CPP 8     3/14/97 5:15p Joe_b $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : INIT.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : January 20, 1992                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Anim_Init -- Initialize the VQ animation control structure.                               *
 *   Bootstrap -- Perform the initial bootstrap procedure.                                     *
 *   Calculate_CRC -- Calculates a one-way hash from a data block.                             *
 *   Init_Authorization -- Verifies that the player is authorized to play the game.            *
 *   Init_Bootstrap_Mixfiles -- Registers and caches any mixfiles needed for bootstrapping.    *
 *   Init_Bulk_Data -- Initialize the time-consuming mixfile caching.                          *
 *   Init_CDROM_Access -- Initialize the CD-ROM access handler.                                *
 *   Init_Color_Remaps -- Initialize the text remap tables.                                    *
 *   Init_Expansion_Files -- Fetch any override expansion mixfiles.                            *
 *   Init_Fonts -- Initialize all the game font pointers.                                      *
 *   Init_Game -- Main game initialization routine.                                            *
 *   Init_Heaps -- Initialize the game heaps and buffers.                                      *
 *   Init_Keys -- Initialize the cryptographic keys.                                           *
 *   Init_Mouse -- Initialize the mouse system.                                                *
 *   Init_One_Time_Systems -- Initialize internal pointers to the bulk data.                   *
 *   Init_Random -- Initializes the random-number generator                                    *
 *   Init_Secondary_Mixfiles -- Register and cache secondary mixfiles.                         *
 *   Load_Recording_Values -- Loads recording values from recording file                       *
 *   Load_Title_Page -- Load the background art for the title page.                            *
 *   Obfuscate -- Sufficiently transform parameter to thwart casual hackers.                   *
 *   Parse_Command_Line -- Parses the command line parameters.                                 *
 *   Parse_INI_File -- Parses CONQUER.INI for special options                                  *
 *   Play_Intro -- plays the introduction & logo movies                                        *
 *   Save_Recording_Values -- Saves recording values to a recording file                       *
 *   Select_Game -- The game's main menu                                                       *
 *   Load_Prolog_Page -- Loads the special pre-prolog "please wait" page.                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "init.h"

#include "_bench.h"
#include "_command.h"
#include "_convert.h"
#include "_font.h"
#include "_keyboar.h"
#include "_logic.h"
#include "_map.h"
#include "_mixfile.h"
#include "_mono.h"
#include "_palette.h"
#include "_pk.h"
#include "_rand.h"
#include "_rect.h"
#include "_rules.h"
#include "_script.h"
#include "_surface.h"
#include "_tactica.h"
#include "_theater.h"
#include "_timer.h"
#include "_tooltip.h"
#include "_voxel.h"
#include "abstract.h"
#include "addon.h"
#include "aircraft.h"
#include "airctype.h"
#include "alphashp.h"
#include "anim.h"
#include "bench.h"
#include "blight.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "campaign.h"
#include "ccfile.h"
#include "cctooltip.h"
#include "cell.h"
#include "command.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "dialog.h"
#include "dsaudio.h"
#include "dsurface.h"
#include "egos.h"
#include "empulse.h"
#include "enviro.h"
#include "except.h"
#include "expand.h"
#include "factory.h"
#include "fog.h"
#include "gamedirs.h"
#include "gamedlg.h"
#include "getcpu.h"
#include "globals.h"
#include "houstype.h"
#include "incdec.h"
#include "infatype.h"
#include "ionblast.h"
#include "ipxmgr.h"
#include "keyboard.h"
#include "language\language.h"
#include "laser.h"
#include "light.h"
#include "lightcon.h"
#include "loaddlg.h"
#include "logic.h"
#include "mainopt.h"
#include "mixfile.h"
#include "misc.h"
#include "mono.h"
#include "movie.h"
#include "mplayer.h"
#include "msgbox.h"
#include "netdlg.h"
#include "netdlg2.h"
#include "newmenu.h"
#include "obscure.h"
#include "opents_build.h"
#include "overlay.h"
#include "overtype.h"
#include "ovrlight.h"
#include "ownrdraw.h"
#include "partsys.h"
#include "pcx.h"
#include "queue.h"
#include "ramfile.h"
#include "revent.h"
#include "rndstraw.h"
#include "rules.h"
#include "saveload.h"
#include "scenario.h"
#include "scheme.h"
#include "script.h"
#include "session.h"
#include "spawner.h"
#include "side.h"
#include "skirmish.h"
#include "smudtype.h"
#include "stimer.h"
#include "tactical.h"
#include "tag.h"
#include "team.h"
#include "techno.h"
#include "terrain.h"
#include "theme.h"
#include "timer.h"
#include "tracker.h"
#include "trigger.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vein.h"
#include "voc.h"
#include "vox.h"
#include "vqoption.h"
#include "wave.h"
#include "waypoint.h"
#include "winstub.h"
#include "wonline.h"
#include "worlddom.h"
#include "wsproto.h"
#include "wspudp.h"
#include "wwfont.h"

#include "bench.hh"
#include "scrnsel.hh"

#include <algorithm>
#include <conio.h>
#include <ctime>
#include <dos.h>
#include <unordered_set>

extern VoxelDataStruct DropPodVoxel;

struct ChooseCampaignStruct {
	CampaignType ChosenCampaign;
	bool ChoiceMade;
};

/**********************************************************************
**	Optional parameter control for special options.
*/

/*
**	Enable the set of limited cheat key options.
*/
#ifdef _DEBUG
#define	PARM_PLAYTEST		0xF7DDC227		// "PLAYTEST"
#endif

/*
**	Enable the full set of cheat key options.
*/
#ifdef _DEBUG
#ifndef PARM_PLAYTEST
#define	PARM_PLAYTEST		0xF7DDC227		// "PLAYTEST"
#endif
#endif

#define	PARM_INSTALL		0xD95C68A2		//	"FROMINSTALL"


/****************************************
**	Function prototypes for this module **
*****************************************/
static void Play_Intro(bool sequenced=false);
static void Init_Color_Remaps(void);
static void Init_Heaps(void);
static bool Init_Expansion_Files(void);
static bool Init_One_Time_Systems(void);
static bool Init_Fonts(void);
static bool Init_Bootstrap_Mixfiles(void);
static bool Init_Secondary_Mixfiles(void);
static void Init_Mouse(void);
static bool Bootstrap(void);
static bool Init_Bulk_Data(void);
static void Init_Keys(void);
static bool Init_Rules(void);
static void Init_Commands(void);
static CampaignType Choose_Campaign(void);
static void Init_Threads(void);
void Draw_Version_Text(Surface * surface);
void Version_Dialog(void);

BOOL CALLBACK Rules_Choice_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK Main_Menu_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

void Init_Random(void);

#define ATTRACT_MODE_TIMEOUT	TIMER_MINUTE		// timeout for attract mode

bool Load_Recording_Values(CCFileClass & file);
bool Save_Recording_Values(CCFileClass & file);

struct CheatEntryStruct {
	bool * State;
	char const * CheatString;
	char const * VersionSuffix;	/// Suffix appended to version string
	bool IsAllowedInMP;
};

static CheatEntryStruct CheatEntries[] = {
	{ &VisceroidsAsSnoBees,    "PENGO",    " PG", false },
	{ &Just4Fun,               "THETEAM",  NULL,  false },
};

static void Cheat_Disable(void);
static bool Cheat_Key_Process(char chr);
static void Cheat_Version_Suffix(char * string);



/// <summary>
/// Reads a base game file and the expansion's counterpart into one database.
/// The expansion's file is read over the base one, and either file on its own is enough, so a
/// deployment may ship the content in whichever of the two it belongs in.
/// </summary>
/// <param name="ini">The database both files are read into.</param>
/// <param name="basename">The name of the base game's file.</param>
/// <param name="expansion">The name of the expansion's file.</param>
/// <returns>bool; Was either file read?</returns>
static bool Read_INI_And_Expansion(CCINIClass & ini, char const * basename, char const * expansion)
{
	char const * const names[] = {basename, expansion};
	bool read = false;

	for (char const * name : names) {
		CCFileClass file(name);

		if (file.Is_Available() == false) {
			continue;
		}

		if (ini.Load(file, false)) {
			read = true;
		} else {
			DebugString("Failed to load %s!\n", name);
		}
	}

	return(read);
}


/***********************************************************************************************
 * Init_Game -- Main game initialization routine.                                              *
 *                                                                                             *
 *    Perform all one-time game initializations here. This includes all                        *
 *    allocations and table setups. The intro and other one-time startup                       *
 *    tasks are also performed here.                                                           *
 *                                                                                             *
 * INPUT:   argc,argv   -- Command line arguments.                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this ONCE!                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *=============================================================================================*/
int Init_Game(int , char * [])
{
	DebugString("Init Game\n");
	Scen = new ScenarioClass;

	if (Scen == NULL) {
		DebugString("Failed to instantiate Scenario!\n");
		return(-1);
	}

	Rule = new RulesClass;

	if (Rule == NULL) {
		DebugString("Failed to instantiate Rules!\n");
		return(-1);
	}

	/*
	**	Allocate the benchmark tracking objects only if the machine and
	**	compile flags indicate.
	*/
#ifdef _DEBUG
	if (Processor() >= 2) {
		DebugString("Creating benchmarks\n");
		Benches = new Benchmark [BENCH_COUNT];
	}
#endif

	/*
	**	Initialize the encryption keys.
	*/
	DebugString("Init Encryption Keys.\n");

	Init_Keys();

	/*
	**	Bootstrap as much as possible before error-prone initializations are
	**	performed. This bootstrap process will enable the error message
	**	handler to function.
	*/
	DebugString("Bootstrap.....");

	if (!Bootstrap()) {
		DebugStringNoPrefix(" ..Failed to bootstrap!\n");
		return(-1);
	}

	DebugStringNoPrefix(" ...OK\n");

	/*
	**	Check for an initialize a working mouse pointer. Display error and bail if
	**	no mouse driver is installed.
	*/
	DebugString("Init Mouse\n");

	Init_Mouse();

	/*
	**	Register and cache any secondary mixfiles.
	*/
	DebugString("Init Secondary Mixfiles.....");

	if (!Init_Secondary_Mixfiles()) {
		DebugStringNoPrefix(" ...Failed!!!\n");
		return(-1);
	}

	DebugStringNoPrefix(" ...OK\n");

	DebugString("Init Campaigns\n");
	Init_Campaigns();

	/*
	**	Initialize the game object heaps as well as other rules-dependant buffer allocations.
	*/
	DebugString("Init Heaps\n");
	Init_Heaps();

	DebugString("Init Threads\n");
	Init_Threads();

	/*
	**	Read game options, so the GameSpeed is initialized when multiplayer
	**	dialogs are invoked. (GameSpeed must be synchronized between systems.)
	*/
	DebugString("Reading Game Settings\n");
	Options.Load_Settings();

	/*
	**	Initialize the animation system.
	*/
	DebugString("Init Anim System\n");
	Anim_Init();

	/*
	**	Play the startup animation.
	*/
	if (!Spawner_Is_Requested()) {
		if (Special.IsFromInstall == true) {
			DebugString("Playing first time intro sequence.\n");
			Play_Movie("EVA.VQA", THEME_NONE, false);
		}

		DebugString("Playing startup movies.\n");
		Play_Movie("WWLOGO.VQA", THEME_NONE);
		if (!Get_New_Menu()->MixFile) {
			if (CCFileClass("FS_TITLE.VQA").Is_Available() == true) {
				Play_Movie("FS_TITLE.VQA", THEME_NONE, false);
			} else {
				Play_Movie("STARTUP.VQA", THEME_NONE, false);
			}
		}
	}

	Draw_Menu_Background();
	Call_Back();

	/*
	**	Initialize the text remap tables.
	*/
	DebugString("Init Color Remap Tables\n");
	Init_Color_Remaps();

	/*
	**
	*/
	DebugString("Creating TacticalMap\n");

	delete TacticalMap;
	TacticalMap = new Tactical;

	if (TacticalMap == NULL) {
		DebugString("Failed to create TacticalMap!\n");
		return(-1);
	}

	/*
	**	Set the logic page to the seenpage.
	*/
	LogicalSurface = HiddenSurface;

	/*
	**	Initialize the bulk data. This takes the longest time and must be performed once
	**	before the regular game starts.
	*/
	DebugString("Init Bulk Data\n");

	if (!Init_Bulk_Data()) {
		return(-1);
	}

	/*
	**
	*/
	DebugString("Reading SOUND.INI\n");

	CCINIClass voc_ini;
	if (!Read_INI_And_Expansion(voc_ini, "SOUND.INI", "SOUND01.INI")) {
		DebugString("Failed to read SOUND.INI or SOUND01.INI!\n");
		return(-1);
	}

	Free_Vocs();
	Init_Vocs(voc_ini);

	/*
	**
	*/
	DebugString("Reading THEME.INI\n");

	CCINIClass theme_ini;
	if (!Read_INI_And_Expansion(theme_ini, "THEME.INI", "THEME01.INI")) {
		DebugString("Failed to read THEME.INI or THEME01.INI!\n");
		return(-1);
	}

	Theme.Free_Themes();
	Theme.Init_Themes(theme_ini);
	Theme.Scan();

	/*
	**	Find and process any rules for this game.
	*/
	DebugString("Init Rules\n");

	if (!Init_Rules()) {
		DebugString("Failed to initialize Rules!\n");
		return(-1);
	}

	Session.MaxPlayers = Rule->MaxPlayers;

	GadgetClass::Set_Color_Scheme(DEFAULT_GADGET_SCHEME);


	/*
	**	Initialize the multiplayer score values
	*/
	Session.GamesPlayed = 0;
	Session.NumScores = 0;
	Session.CurGame = 0;
	for (int i = 0; i < MAX_MULTI_NAMES; i++) {
		Session.Score[i].Name[0] = '\0';
		Session.Score[i].Wins = 0;
		// -1 = this player didn't play this round
		for (int j = 0; j < MAX_MULTI_GAMES; j++) {
			Session.Score[i].Kills[j] = -1;
			Session.Score[i].Lost[j] = -1;
			Session.Score[i].Built[j] = -1;
			Session.Score[i].Score[j] = -1;
		}
	}

	/*
	**	Copy the title screen's palette into the GamePalette & OriginalPalette,
	**	because the options Load routine uses these palettes to set the brightness, etc.
	*/
//	GamePalette = CCPalette;
//	InGamePalette = CCPalette;
//	OriginalPalette = CCPalette;

	Init_Random();

	DebugString("Init Commands\n");
	Init_Commands();

	DebugString("Game Init Completed.\n");

	return(0);
}


/// <summary>
/// Handles the messages for the rules file choice dialog.
/// This routine lists the name of every rules file that was found, and ends the dialog
/// with the index of the one that the player settled upon.
/// </summary>
/// <remarks>The dialog must be created with the vector of rules files as its parameter.</remarks>
static BOOL CALLBACK Rules_Choice_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	char buffer[128];

	switch (message) {
		case WM_INITDIALOG: {
			Center_Window_Within_Window(window);

			DynamicVectorClass<CCINIClass*> * rules;
			rules = (DynamicVectorClass<CCINIClass*> *)lparam;

			HWND list = GetDlgItem(window, IDC_RULES_LIST);

			for (int index = 0; index < rules->Count(); index++) {
				(*rules)[index]->Get_String("General", "Name", "", buffer, sizeof(buffer));
				ListBox_AddString(list, buffer);
			}
			ListBox_SetCurSel(list, 0);
		}
		break;

		case WM_HELP:
			On_WM_HELP(lparam);
			break;

		case WM_CONTEXTMENU:
			On_WM_CONTEXTMENU(wparam);
			break;

		case WM_MOVING:
			return(On_WM_MOVING(window, wparam, lparam));

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDCANCEL:
				case IDC_RULES_OK:
					if (HIWORD(wparam) == BN_CLICKED) {
						HWND list = GetDlgItem(window, IDC_RULES_LIST);
						EndDialog(window, ListBox_GetCurSel(list));
						DestroyWindow(window);
					}
					break;
			}
			break;
	}

	return(0);
}


/// <summary>
/// Reads the campaign definitions out of the battle control files.
/// This routine gathers every battle file it can find, along with the expansion's own,
/// so that added campaigns show up in the list beside the ones that shipped.
/// </summary>
void Init_Campaigns(void)
{
	bool found = false;

	for (std::string const & name : Search_Files("BATTLE*.INI")) {
		CCFileClass file(name.c_str());
		CCINIClass * ini = new CCINIClass;
		ini->Load(file, false);

		if (stricmp(name.c_str(), "BATTLE.INI") == 0) {
			found = true;
		}

		Read_Battle_INI(*ini);
		delete ini;
	}

	if (!found) {
		CCFileClass file("BATTLE.INI");
		CCINIClass * ini = new CCINIClass;

		if (ini != NULL) {
			ini->Load(file, false);
			Read_Battle_INI(*ini);
			delete ini;
		}
	}

	CCFileClass file("BATTLEFS.INI");
	if (file.Is_Available() == true) {
		CCINIClass * ini = new CCINIClass;

		if (ini != NULL) {
			ini->Load(file, false);
			Read_Battle_INI(*ini);
			delete ini;
		}
	}
}


/// <summary>
/// Reads the countries and the sides they belong to from the rules.
/// This runs before a game is set up, so that a house's side is known before anything asks
/// for it. The side roster is only established by reading it: a country carries the name of
/// its side, and the rules' own side list decides what order the sides are registered in.
/// </summary>
void Prepare_Side_Roster(void)
{
	Rule->Do_HouseTypes(*RuleINI);
	Rule->Do_Sides(*RuleINI);

	for (int index = 0; index < HouseTypes.Count(); index++) {
		HouseTypes[index]->Read_INI(*RuleINI);
	}
}


/// <summary>
/// Can this campaign be played with the addons that are enabled?
/// A base game campaign is offered only when no addon is running, and an addon's own
/// campaign only when that particular addon is running.
/// </summary>
/// <param name="campaign">The campaign to be tested.</param>
/// <returns>bool; Is the campaign available for the player to select?</returns>
static bool Campaign_Available(CampaignClass * campaign)
{
	if (Addon_Enabled(ADDON_ANY) == true) {
		if (campaign->RequiredAddon == ADDON_BASE_GAME) {
			return(false);
		}
		if (Addon_Enabled((AddonType)campaign->RequiredAddon)) {
			return(true);
		}
		return(false);
	}

	if (campaign->RequiredAddon == ADDON_BASE_GAME) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Handles the messages for the campaign choice dialog.
/// This routine lists the campaigns that the player is entitled to play, drives the
/// difficulty slider, and leaves the choice where Choose_Campaign will collect it.
/// </summary>
static BOOL CALLBACK Campaign_Choice_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HWND item;
	struct ChooseCampaignStruct * state;

	int rc;
	rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc) {
		return(rc);
	}

	switch (message) {

		case WM_INITDIALOG:
			item = GetDlgItem(window, IDC_LIST);

			if (item != NULL) {
				DebugString("Initializing Choose_Campaign() Dialog.\n");
				for (int index = 0; index < Campaigns.Count(); index++) {
					CampaignClass * campaign = Campaigns[index];

					if (!Campaign_Available(campaign)) {
						DebugString("\tSkipping Campaign [%d] - %s\n", index, campaign->Description);
						continue;
					}

					DebugString("\tAdding Campaign [%d] - %s\n", index, campaign->Description);
					int pos = ListBox_AddString(item, campaign->Description);
					ListBox_SetItemData(item, pos, index);
				}

				ListBox_SetCurSel(item, 0);
			}

			item = GetDlgItem(window, IDC_DIFFICULTY_SLIDER);

			if (item != NULL) {
				SendMessage(item, OD_TRACKNUMBERS, 0, 0);
				Slider_SetRange(item, 0,2);
				Slider_SetPos(item, Options.Difficulty);
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDOK:
					if (HIWORD(wparam) == BN_CLICKED) {
						state = (ChooseCampaignStruct *)GetWindowLong(window, DWL_USER);

						if (state != NULL) {
							item = GetDlgItem(window, IDC_LIST);

							if (item != NULL) {
								int pos = ListBox_GetCurSel(item);
								state->ChosenCampaign = (CampaignType)ListBox_GetItemData(item, pos);
								state->ChoiceMade = true;
							}
						}

						item = GetDlgItem(window, IDC_DIFFICULTY_SLIDER);

						if (item != NULL) {
							Options.Difficulty = Slider_GetPos(item);
						}
					}
					break;

				case IDCANCEL:
					if (HIWORD(wparam) == BN_CLICKED) {
						state = (ChooseCampaignStruct *)GetWindowLong(window, DWL_USER);

						if (state != NULL) {
							state->ChosenCampaign = CAMPAIGN_NONE;
							state->ChoiceMade = true;
						}
					}

					break;
			}
			break;

		case WM_HSCROLL: {
			int diff = HIWORD(wparam);
			int stringID = 0;

			if ((HWND)lparam == GetDlgItem(window, IDC_DIFFICULTY_SLIDER)) {
				stringID = GameDifficultyNames[diff];
				item = GetDlgItem(window, IDC_DIFFICULTY_LABEL);
				Static_SetText(item, Fetch_String(stringID));
			}
			break;
		}

		default:
			break;
	}

	return(FALSE);
}


/// <summary>
/// Asks the player which campaign to play.
/// This routine reads the campaign list first if that has not already happened, and then
/// runs the campaign dialog until the player either commits or backs out.
/// </summary>
/// <returns>Returns with the campaign chosen, or CAMPAIGN_NONE if the player backed out.</returns>
static CampaignType Choose_Campaign(void)
{
	HWND dialog;
	struct ChooseCampaignStruct state;

	state.ChoiceMade = false;
	state.ChosenCampaign = CAMPAIGN_NONE;

	if (Campaigns.Count() == 0) {
		Init_Campaigns();

		if (Campaigns.Count() == 0) {
			return(CAMPAIGN_NONE);
		}
	}

	dialog = OwnerDraw::Begin_Dialog(IDD_CAMPAIGN, (DLGPROC) Campaign_Choice_Dialog_Proc);

	if (dialog != NULL) {
		SetWindowLong(dialog, DWL_USER, (LONG) &state);

		OwnerDraw::Move_Dialog(dialog, -1, (HiddenSurface->Get_Height() - 400) / 2 + 147);
		OwnerDraw::Display_Dialog(dialog);

		while (state.ChoiceMade == false) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				break;
			}
			Title_Screen_Restore();
		}

		OwnerDraw::End_Dialog(dialog);
	}

	return(state.ChosenCampaign);
}


/// <summary>
/// Loads the rules and the art control files.
/// This routine gathers every rules file it can find and, should there be more than one,
/// asks the player which of them to play with. It then loads the art, expansion, AI and
/// language override files, and seeds the multiplayer defaults from the rules just read.
/// </summary>
/// <returns>bool; Were the rules loaded successfully?</returns>
static bool Init_Rules(void)
{
	DynamicVectorClass<CCINIClass*> Rules;

	bool found = false;

	for (std::string const & name : Search_Files("RULE*.INI")) {
		CCFileClass file(name.c_str());
		CCINIClass * rule = new CCINIClass;

		rule->Load(file, false);

		if (stricmp(name.c_str(), "RULES.INI") == 0) {
			found = true;
			Rules.Add_Head(rule);
		} else {
			Rules.Add(rule);
		}
	}

	if (!found) {
		CCFileClass file("RULES.INI");
		CCINIClass * rule = new CCINIClass;
		rule->Load(file, false);
		Rules.Add_Head(rule);
	}

	assert(Rules.Count() > 0);

	if (Rules.Count() <= 0) {
		return(false);
	}

	CCFileClass art_file("ART.INI");

	if (!ArtINI.Load(art_file, false)) {
		DebugString("Failed to load ART.INI!\n");
		return(false);
	}

	CCINIClass art_ini;
	CCFileClass art_fs_file("ARTFS.INI");

	if (art_fs_file.Is_Available() == true) {
		art_ini.Load(art_fs_file, false);
	}

	if (Addon_Installed(ADDON_FIRESTORM)) {
		CCFileClass rules_fs_file("FIRESTRM.INI");
		if (rules_fs_file.Is_Available() == true) {
			CCINIClass rule_fs;
			if (!FSRuleINI.Load(rules_fs_file, false)) {
				DebugString("Failed to load FIRESTRM.INI!\n");
				return(false);
			}
		}
	}

	if (Rules.Count() == 1) {
		RuleINI = Rules[0];
	} else {
		MouseCursor->Release_Mouse();
		int rules_choice = DialogBoxParam(ProgramInstance, MAKEINTRESOURCE(IDD_RULES_CHOICE), MainWindow, (DLGPROC)Rules_Choice_Dialog_Proc, (LPARAM)&Rules);
		MouseCursor->Capture_Mouse();

		if (rules_choice == -1) {
			rules_choice = 0;
		}

		RuleINI = Rules[rules_choice];
	}

	Rule->Color_Schemes(*RuleINI);
	Rule->Do_Movies(ArtINI);
	Rule->Do_Movies(art_ini);
	Rule->Audio_Visual_Rules(*RuleINI);
	Rule->MPlayer(*RuleINI);

	Session.Options.UnitCount = Rule->MPUnitCount;
	BuildLevel = Rule->MPBuildLevel;
	Session.Options.Credits = Rule->MPMoney;
	Session.Options.FogOfWar = false;
	Session.Options.BridgeDestruction = Rule->IsMPBridgeDestruction;
	Session.Options.Goodies = Rule->IsMPCrates;
	Session.Options.Bases = Rule->IsMPBasesOn;
	Session.Options.CTF = Rule->IsMPCaptureTheFlag;
	Session.Options.AIPlayers = 0;
	Session.Options.AIDifficulty = DIFF_NORMAL;

	CCFileClass lang_file("LANGRULE.INI");

	if (lang_file.Is_Available() == true) {
		CCINIClass lang_ini;

		if (lang_ini.Load(lang_file, true) > 1) {
			return(false);
		}

		Rule->Addition(lang_ini);
	}

	for (int index = 0; index < Rules.Count(); index++) {
		if (Rules[index] != RuleINI) {
			delete Rules[index];
		}
	}

	CCFileClass ai_file("AI.INI");
	AIINI.Load(ai_file, true);

	if (Addon_Installed(ADDON_FIRESTORM)) {
		CCFileClass ai_fs_file("AIFS.INI");
		if (ai_fs_file.Is_Available() == true) {
			CCINIClass ai_fs_ini;
			if (!FSAIINI.Load(ai_fs_file, false)) {
				DebugString("Failed to load AIFS.INI!\n");
				return(false);
			}
		}
	}

	return(true);
}


/***********************************************************************************************
 * Select_Game -- The game's main menu                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    fade     if true, will fade the palette in gradually                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *    none.                                                                                    *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    none.                                                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/05/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
bool Select_Game(bool )
{
	bool gameloaded;				// Has the game been loaded from the menu?
	int selection;					// the default selection
	bool process;					// false = break out of while loop

	static int protocol = -1;

	/// RA2 calls this here, bugfix?
	//Theme.Play_Song(Fetch_Main_Menu_Theme());

	Show_Mouse();

restart:
	gameloaded = false;
	process = true;

	/*
	**	[Re]set any globals that need it, in preparation for a new scenario
	*/
	GameActive = true;
	PlayerPtr = NULL;
	DoList.clear();
	OutList.clear();
	Frame = 0;
	Scen->MissionTimer = 0;
	Scen->MissionTimer.Stop();
	Scen->CDifficulty = DIFF_NORMAL;
	Scen->Difficulty = DIFF_NORMAL;
	PlayerWins = false;
	PlayerLoses = false;
	PlayerRestarts = false;
	PlayerAborts = false;
	Session.ObiWan = false;
#ifdef _DEBUG
	Debug_Unshroud = false;
#endif
	Map.Set_Cursor_Shape(NULL);
	Map.PendingObjectPtr = 0;
	Map.PendingObject = 0;
	Map.PendingHouse = HOUSE_NONE;

	Session.ProcessTicks = 0;
	Session.ProcessFrames = 0;
	Session.DesiredFrameRate = 30;
	NewMaxAheadFrame1 = 0;
	NewMaxAheadFrame2 = 0;

	/*
	**	Init multiplayer game scores.  Let Wins accumulate; just init the current
	**	Kills for this game.  Kills of -1 means this player didn't play this round.
	*/
	for (int i = 0; i < MAX_MULTI_GAMES; i++) {
		Session.Score[i].Kills[Session.CurGame] = -1;
	}

	/*
	**	Set default mouse shape
	*/
	Map.Set_Default_Mouse(MOUSE_NORMAL, false);

	if (ToolTips != NULL) {
		ToolTips->Activate(false);
	}

	/*
	**	If the last game we played was a multiplayer game, jump right to that
	**	menu by pre-setting 'selection'.
	*/
	if (Session.Type == GAME_NORMAL) {
		selection = SEL_NONE;
	} else if (Session.Type == GAME_INTERNET) {
		selection = SEL_INTERNET_RETURN;
	} else {
		selection = SEL_MULTIPLAYER_GAME;
	}

	/*
	**	Main menu processing; only do this if we're not in editor mode.
	*/
	if (!Debug_Map) {

		/*
		**	Menu selection processing loop
		*/
		Theme.Play_Song(Fetch_Main_Menu_Theme());

		/*
		**	If we're playing back a recording, load all pertinent values & skip
		**	the menu loop.  Hide the now-useless mouse pointer.
		*/
		if (Session.Play && Session.RecordFile.Is_Available()) {
			if (Session.RecordFile.Open(FileClass::READ)) {
				Load_Recording_Values(Session.RecordFile);
				process = false;
				Theme.Stop(true);
			} else {
				Session.Play = false;
			}
		}

		/*
		 * A client-requested launch takes the place of the menu, once. A spawned match that
		 * has ended, or a launch that was refused, answers false so the process leaves and
		 * the client sees it go.
		 */
		if (Spawner_Is_Requested()) {
			if (!Spawner_Prepare(gameloaded)) {
				return(false);
			}
			process = false;
			Theme.Stop(true);
		}

		while (process) {

			/*
			**	Display menu and fetch selection from player.
			*/
			if (PacketTransport) {
				Ipx.Shutdown();
			}

			if ((selection == SEL_NONE) && !Debug_ForceScenario) {
				if (Get_New_Menu()->MixFile) {
					selection = New_Main_Menu();
				} else {
					selection = Main_Menu(ATTRACT_MODE_TIMEOUT);
				}
			}

			if (Debug_ForceScenario) {
				selection = SEL_CAMPAIGN_GAME;
			}

			Call_Back();

			switch (selection) {

				case SEL_NEW_SCENARIO:
					new (&Environment) EnvironmentClass;
					Expansion_Dialog();
					Theme.Stop(true);
					Session.Type = GAME_NORMAL;
					process = false;
					break;

				/*
				 * SEL_CAMPAIGN_GAME: Play the game
				 */
				case SEL_CAMPAIGN_GAME: {
					new (&Environment) EnvironmentClass;

					Scen->Campaign = Choose_Campaign();
					if (Scen->Campaign == CAMPAIGN_NONE) {
						process = true;
						selection = SEL_NONE;
						break;
					}

					Theme.Stop(true);

					int timeout = (TickCount + 5 * TIMER_SECOND);

					while (Theme.Still_Playing() && (timeout > TickCount)) {
						Call_Back();
					}

					Theme.Stop();
					Session.Type = GAME_NORMAL;
					process = false;
					break;
				}

				/*
				**	Load a saved game.
				*/
				case SEL_LOAD_GAME:
					if (LoadOptionsClass().Load()) {
						Theme.Stop();
						process = false;
						gameloaded = true;
					} else {
						selection = SEL_NONE;
					}

					break;

				/*
				**	SEL_MULTIPLAYER_GAME: set 'Session.Type' to network play.
				*/
				case SEL_INTERNET_RETURN:
				case SEL_MULTIPLAYER_GAME: {
						Session.Read_MultiPlayer_Settings();

						for (int house = 0; house < HouseTypes.Count(); house++) {
							HouseTypes[house]->Read_INI(*RuleINI);
						}

						Session.Suspended = 0;

						switch (Session.Type) {
							/*
							**	If 'Session.Type' isn't already set up for a multiplayer game,
							**	we must prompt the user for which type of multiplayer game
							**	they want.
							*/
						case GAME_NORMAL:
							if (Get_New_Menu()->MixFile == NULL) {
								Session.Type = Select_MPlayer_Game();
								if (Session.Type == GAME_WDT) {
									Session.Type = GAME_INTERNET;
									Session.IsWDT = true;
									continue;
								} else {
									Session.IsWDT = false;
									if (Session.Type == GAME_NORMAL) {
										selection = SEL_NONE;
									}
									if (Session.Type == GAME_INTERNET || Session.Type == GAME_SKIRMISH) {
										continue;
									}
								}
							} else {
								selection = SEL_NONE;
								continue;
							}
							break;

						case GAME_SKIRMISH:
							if (!Skirmish_Mode_Dialog()) {
								Session.Type = GAME_NORMAL;
								selection = SEL_NONE;
							}
							break;

						/*
						**	Handle being spawned from WChat. Internet play based on IPX code.
						*/
						case GAME_INTERNET: {
							Cheat_Disable();

							OwnerDraw::Capture_Mouse();

							/*
							**	Fetch the house attribute override values.
							*/
							for (int house = 0; house < HouseTypes.Count(); house++) {
								HouseTypes[house]->Read_INI(*RuleINI);
							}

							Ipx.Configure_WOL();
							Session.CommProtocol = COMM_PROTOCOL_MULTI_E_COMP;

							Ipx.Set_Timing(TIMER_SECOND / 2, (unsigned int)-1, 10 * TIMER_SECOND);
							DebugString("About to initialise the network code\n");
							bool success = Init_Network();

							OwnerDraw::Release_Mouse();

							Session.NetOpen = true;

							WonlineResult res;

							if (selection == SEL_INTERNET_RETURN) {
								Title_Screen_Restore(true);
								if (!Session.IsWDT) {
									res = (WonlineResult)Join_WOL_Lobby(0);
								} else {
									while (true) {
										res = (WonlineResult)Join_WOL_Lobby(0);
										if (res == WONLINE_OK) {
											break;
										}
										if (Request_WDT_Cycle() && WDT_Setup_Game(false)) {
											Session.IsWDT = true;
										} else {
											Logout_WOnline();
											res = WONLINE_BACK;
											Session.Type = GAME_NORMAL;
											Session.IsWDT = false;
											selection = SEL_MULTIPLAYER_GAME;
											break;
										}

									}
								}

							} else {
								res = Login_WOL();
								if (res == WONLINE_OK) {
									if (!Session.IsWDT) {
										res = (WonlineResult)Join_WOL_Lobby(0);
									} else {
										bool end = false;
										bool initial = true;
										while (!end) {
											if (WDT_Setup_Game(initial)) {
												initial = false;
												Session.IsWDT = true;
												res = (WonlineResult)Join_WOL_Lobby(0);
												end = (res != WONLINE_BACK);
											} else {
												Logout_WOnline();
												res = WONLINE_BACK;
												Session.Type = GAME_NORMAL;
												Session.IsWDT = false;
												selection = SEL_MULTIPLAYER_GAME;
												end = true;
											}
										}
									}
								}
							}

							Session.NetOpen = false;

							if (res == WONLINE_DOWNLOAD_PATCH) {
								Theme.Stop(true);

								Ipx.Shutdown();
								return(false);
							}

							if (res == WONLINE_BACK) {
								Ipx.Shutdown();
								selection = SEL_NONE;
								Session.Type = GAME_NORMAL;
								Theme.Play_Song(Fetch_Main_Menu_Theme());
							} else {
								DebugString("Return from WOL to play a %d player game\n", Session.Players.Count());
								if (success) {
									process = false;
									protocol = 2;
									Theme.Stop(true);
								} else {
									/*
									** We failed to connect to the other player
									*/
									Session.Type = GAME_NORMAL;
									selection = SEL_NONE;

									Ipx.Shutdown();
									protocol = -1;
								}
							}

							break;
						}
						}
					}
					switch (Session.Type) {
						/*
						**	Internet
						*/
						case GAME_INTERNET:
						case GAME_SKIRMISH:
							Theme.Stop(true);
							process = false;
							break;

						/*
						**	Network: start a new local network game.
						*/
						case GAME_IPX: {
							Cheat_Disable();
							Session.Read_MultiPlayer_Settings();

							/*
							**	Fetch the house attribute override values.
							*/
							for (int house = 0; house < HouseTypes.Count(); house++) {
								HouseTypes[house]->Read_INI(*RuleINI);
							}

							Session.Type = GAME_IPX;
							Session.CommProtocol = COMM_PROTOCOL_MULTI_E_COMP;

							Ipx.Configure_LAN();

							/*
							**	Init network system & remote-connect
							*/
							Draw_Menu_Background();

							if (Net2Init_Network() && Net2Remote_Connect()) {
								process = false;
								Theme.Stop(true);
							} else {
								// user hit cancel, or init failed
								Session.Type = GAME_NORMAL;
								selection = SEL_NONE;

								Ipx.Shutdown();
							}
						}
						break;
					}
					break;

				/*
				**	Play a VQ
				*/
				case SEL_INTRO:
					Theme.Stop();
					if (Debug_Flag) {
						Play_Intro(Debug_Flag);
					} else {
						Play_Movie("INTRO.VQA");
						Clear_Option(OPTION_PLAY_FROM_MIXFILE);
						Play_Movie("SIZZLE1.VQA");
						Set_Option(OPTION_PLAY_FROM_MIXFILE);
					}
					Theme.Queue_Song(Fetch_Main_Menu_Theme());
					selection = SEL_NONE;
					break;

				case SEL_OPTIONS:
					selection = SEL_NONE;
					Main_Options_Dialog();
					break;

				case SEL_VERSION: {
					selection = SEL_NONE;
					Version_Dialog();
					break;
				}

				case SEL_VIEW_CREDITS: {
						selection = SEL_NONE;
						Show_Who_Was_Responsible();
						Theme.Queue_Song(Fetch_Main_Menu_Theme());
					}
					break;

				/*
				**	Exit to DOS.
				*/
				case SEL_EXIT: {
						Theme.Stop(true);

						int timeout = (TickCount + 50 * TIMER_SECOND);

						while (Theme.Still_Playing() && Is_Speaking() && (timeout > TickCount)) {
							Call_Back();
						}

						Theme.Stop();

						return(false);
					}

				/*
				**	Display the hall of fame.
				*/
				case SEL_FAME:
					break;

				case SEL_TIMEOUT: {
						if (Session.Attract && Session.RecordFile.Is_Available()) {
							Session.Play = true;

							if (Session.RecordFile.Open(FileClass::READ)) {
								Load_Recording_Values(Session.RecordFile);
								process = false;
								Theme.Stop(true);
							} else {
								Session.Play = false;
								selection = SEL_NONE;
							}
						} else {
							selection = SEL_NONE;
						}
					}
					break;

				default:
					break;
			}
		}
	} else {

		/*
		** For Debug_Map (editor) mode to load scenario
		*/
		//Scen.Set_Scenario_Name("SCG01EA.INI");
	}

	/*
	**	Don't carry stray keystrokes into game.
	*/
	Keyboard->Clear();
	Reset_Multiplayer_Save_State();

	/*
	**	Initialize the random number generator(s)
	*/
	Init_Random();

	/*
	**	Load the scenario.
	*/
	if (!gameloaded && !Session.LoadGame) {
		DebugString("About to load a %d player game.\n",Session.Players.Count());

		/*
		**	Start_Scenario() changes the palette; so, fade out & clear the screen
		**	before calling it.
		*/
		Hide_Mouse();

		if (selection != SEL_CAMPAIGN_GAME) {
			HiddenSurface->Fill(0);
			Update_Visible_Surface(HiddenSurface);
		}

		Show_Mouse();

		if (Session.Type != GAME_NORMAL) {
			Session.PlayerIsGDI = stricmp(HouseTypes[Session.Players[0]->Player.House]->Name(), "GDI") == 0;
		}

		/*
		 * The campaign handicap pair follows the menu's difficulty setting on every path but
		 * a client launch, which chose the pair itself.
		 */
		if (!Spawner_Is_Active()) {
			Session.CampaignDifficulty = (DiffType)Options.Difficulty;
			Session.CampaignCDifficulty = (DiffType)(DIFF_COUNT - 1 - Options.Difficulty);
		}

		if (Session.Type != GAME_NORMAL || Debug_ForceScenario || Session.Play) {
			if (!Start_Scenario(Scen->ScenarioName, true, CAMPAIGN_NONE)) {
				if (Debug_Map) {
					return(false);
				} else {
					goto restart;
				}
			}
		} else {
			if (!Start_Scenario(Campaigns[Scen->Campaign]->ScenarioName, true, Scen->Campaign)) {
				if (Debug_Map) {
					return(false);
				} else {
					goto restart;
				}
			}
		}

		/*
		**	Save initialization values if we're recording this game.
		*/
		if (Session.Record) {
			if (Session.RecordFile.Open(FileClass::WRITE)) {
				Save_Recording_Values(Session.RecordFile);
			} else {
				Session.Record = false;
			}
		}

		if (Special.IsFromInstall == true) {
			Show_Mouse();
		}

		Special.IsFromInstall = false;
	}

	DebugString("IsTGrowth = %d\n", Special.IsTGrowth);
	DebugString("IsTSpread = %d\n", Special.IsTSpread);

	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH && !Session.Play) {
		Session.Create_Connections();

		if (Session.Type == GAME_IPX) {
			Ipx.Set_Timing(std::max<unsigned>(TIMER_SECOND / 4, Ipx.Global_Response_Time() + 2), (unsigned int) -1, 10 * TIMER_SECOND);

			Ipx.Set_External_Timing(std::max<unsigned>(TIMER_SECOND, Ipx.Global_Response_Time() + 2), (unsigned int) -1, 10 * TIMER_SECOND);
		} else {
			if (Session.Type == GAME_INTERNET) {

				Ipx.Set_Timing(std::max<unsigned>(TIMER_SECOND, Ipx.Global_Response_Time() + 2), (unsigned int) -1, 10 * TIMER_SECOND);
			}
		}
	}

	/*
	**	Hide the SeenPage; force the map to render one frame.  The caller can
	**	then fade the palette in.
	**	(If we loaded a game, this step will fade out the title screen.  If we
	**	started a scenario, Start_Scenario() will have played a couple of VQ
	**	movies, which will have cleared the screen to black already.)
	*/
	Call_Back();
	Hide_Mouse();

	HiddenSurface->Fill(0);
	Update_Visible_Surface(HiddenSurface);
	Show_Mouse();
	LogicalSurface = HiddenSurface;
	Map.Override_Mouse_Shape(MOUSE_NO_MOVE);
	Map.Revert_Mouse_Shape();

	/*
	**	Sidebar is always active in hi-res.
	*/
	if (!Debug_Map) {
		Map.Activate(1);
	}

	Map.Flag_To_Redraw();
	Call_Back();

	Hide_Mouse();

	return(true);
}


/***********************************************************************************************
 * Play_Intro -- plays the introduction & logo movies                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *    none.                                                                                    *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    none.                                                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/06/1995 BRR : Created.                                                                 *
 *   05/08/1996 JLB : Modified for Red Alert and direction control.                            *
 *=============================================================================================*/
static void Play_Intro(bool sequenced)
{
	static VQType _counter = VQ_FIRST;

	//Keyboard->Clear();
	if (sequenced) {
		if (_counter <= VQ_FIRST) _counter = (VQType)Movies.Count();
		if (_counter == Movies.Count()) _counter--;
		//Hide_Mouse();
		//VisiblePage.Clear();
		//Show_Mouse();
		Play_Movie(VQType(_counter--), THEME_NONE);

//		Show_Mouse();
	} else {
		//Hide_Mouse();
		//VisiblePage.Clear();
		//Show_Mouse();
		//Play_Movie(VQ_TITLE, THEME_NONE, false);
	}
}


/***********************************************************************************************
 * Anim_Init -- Initialize the VQ animation control structure.                                 *
 *                                                                                             *
 *    VQ animations are controlled by a structure passed to the VQ player. This routine        *
 *    initializes the structure to values required by C&C.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only need to call this routine once at the beginning of the game.               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/20/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void Anim_Init(void)
{
	Initialize_Options();
	Set_Option(OPTION_PLAY_FROM_MIXFILE);
}


/***********************************************************************************************
 * Parse_Command_Line -- Parses the command line parameters.                                   *
 *                                                                                             *
 *    This routine should be called before the graphic mode is initialized. It examines the    *
 *    command line parameters and sets the appropriate globals. If there is an error, then     *
 *    it outputs a command summary and then returns false.                                     *
 *                                                                                             *
 * INPUT:   argc  -- The number of command line arguments.                                     *
 *                                                                                             *
 *          argv  -- Pointer to character string array that holds the individual arguments.    *
 *                                                                                             *
 * OUTPUT:  bool; Was the command line parsed successfully?                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Parse_Command_Line(int argc, char * argv[])
{
	/*
	**	Parse the command line and set globals to reflect the parameters
	**	passed in.
	*/
	//Whom = HOUSE_GOOD;
	Special.Init();

	Debug_Map = false;
#ifdef _DEBUG
	Debug_Unshroud = false;
#endif

	for (int index = 1; index < argc; index++) {
		char arg_string[512];
		char * string = arg_string;		// Pointer to argument.

		// Quotes are dropped on the way in so a quoted argument matches like an unquoted one.
		char * dest = arg_string;
		for (char * src = argv[index]; *src != '\0' && dest < &arg_string[sizeof(arg_string) - 1]; src++) {
			if (*src != '"') {
				*dest++ = *src;
			}
		}
		*dest = '\0';

		// Matching is done on an upper case copy, so that an option carrying a directory
		// can still take it in the case it was written.
		char original[512];
		strcpy(original, arg_string);
		strupr(string);

		/*
		**	Print usage text only if requested.
		*/
		if (stricmp("/?", string) == 0 || stricmp("-?", string) == 0 || stricmp("-h", string) == 0 || stricmp("/h", string) == 0) {
			/*
			**	Unrecognized command line parameter... Display usage
			**	and then exit.
			*/
			puts(Fetch_String(TXT_OPTION_HELP_01));
			puts(Fetch_String(TXT_OPTION_HELP_02));
			puts(Fetch_String(TXT_OPTION_HELP_03));
			puts(Fetch_String(TXT_OPTION_HELP_04));
			puts(Fetch_String(TXT_OPTION_HELP_05));
			puts(Fetch_String(TXT_OPTION_HELP_06));
			return(false);
		}


		bool processed = true;
		int ob = Obfuscate(string);

#ifdef _DEBUG
		Debug_Playtest = true;
		Debug_Flag = true;
#endif

		switch (ob) {

#ifdef _DEBUG
			case PARM_PLAYTEST:
				Debug_Playtest = true;
				break;
#endif

			/*
			**	Special flag - is C&C being run from the install program?
			*/
			case PARM_INSTALL:
				Special.IsFromInstall = true;
// If uncommented, will disable the <ESC> key during the first movie run.
//				BreakoutAllowed = false;
				break;

			default:
				processed = false;
				break;
		}
		if (processed) continue;


#ifdef _DEBUG
		/*
		**	Scenario Editor Mode
		*/
		if (stricmp(string, "-CHECKMAP") == 0) {
			Debug_Check_Map = true;
			continue;
		}

#endif

		if (strnicmp(string, "-DATADIR=", strlen("-DATADIR=")) == 0) {
			Set_Data_Directory(&original[strlen("-DATADIR=")]);
			continue;
		}

		if (strnicmp(string, "-USERDIR=", strlen("-USERDIR=")) == 0) {
			Set_User_Directory(&original[strlen("-USERDIR=")]);
			continue;
		}

		// A client asking the game to launch what SPAWN.INI describes.
		if (stricmp(string, "-SPAWN") == 0) {
			Spawner_Request();
			continue;
		}

		if (memcmp(string, "-TIME=", 6) == 0) {
			sscanf(&string[6], "%d", &TournamentTime);
		}

		if (strstr(string, "-MPDEBUG")) {
			Session.ShowInternetDebug = true;
		}

		/*
		**	Set the Net Stealth option
		*/
		if (strstr(string, "-STEALTH")) {
			Session.NetStealth = true;
			continue;
		}

		/*
		**	Set the Net Protection option
		*/
		if (strstr(string, "-MESSAGES")) {
			Session.NetProtect = false;
			continue;
		}

		/*
		**	Allow "attract" mode
		*/
		if (strstr(string, "-ATTRACT")) {
			Session.Attract = true;
			continue;
		}

		if (isdigit(string[1])) {
			sscanf(string, "-%dX%d", &Options.ScreenWidth, &Options.ScreenHeight);
			continue;
		}

		/*
		**	Set screen to 640x480 instead of 640x400
		*/
		if (strcmp(string, "-480") == 0) {
			Options.ScreenHeight = 480;
			continue;
		}

		if (stricmp(string, "-WIN") == 0) {
			WindowedMode = true;
			continue;
		}

		/*
		 * Arms a deliberate fault; the mode decides where it is raised later.
		 */
		if (strnicmp(string, "-EXCEPTIONTEST=", strlen("-EXCEPTIONTEST=")) == 0) {
			Exception_Set_Test_Mode(string + strlen("-EXCEPTIONTEST="));
			continue;
		}
#ifdef _DEBUG
		/*
		**	Specify the random number seed (for debugging)
		*/
		if (strstr(string, "-SEED")) {
			CustomSeed = (unsigned short)(atoi(string + strlen("SEED")));
			continue;
		}
#endif

		/*
		**	Special command line control parsing.
		*/
		if (strnicmp(string, "-X", strlen("-O")) == 0) {
			string += strlen("-X");
			while (*string) {
				char code = *string++;
				switch (toupper(code)) {

#ifdef _DEBUG

					/*
					**	Monochrome debug screen enable.
					*/
					case 'M':
						MonoClass::Enable();
						break;

					/*
					**	Inert weapons -- no units take damage.
					*/
					case 'I':
						Special.IsInert = true;
						Debug_Inert = true;
						break;

					/*
					**	Hussled recharge timer.
					*/
					case 'H':
						Special.IsSpeedBuild = true;
						Debug_SpeedBuild = true;
						break;

					/*
					**	"Record" a multi-player game
					*/
					case 'X':
						Session.Record = 1;
						break;

					/*
					**	"Play Back" a multi-player game
					*/
					case 'Y':
						Session.Play = 1;
						break;

					/*
					**	Print lots of debug stuff about events & packets
					*/
					case 'P':
						Debug_Print_Events = true;
						break;
#endif

					/*
					 * Enable debug output to a console.
					 */
					case 'C':
						Debug_Console = true;
						Debug_Init_Console();
						break;

					/*
					**	Quiet mode override control.
					*/
					case 'Q':
						Debug_Quiet = true;
						break;

					default:
						puts(Fetch_String(TXT_INVALID));
						return(false);
				}

			}

			continue;
		}
	}
	return(true);
}


/***************************************************************************
 * Init_Random -- Initializes the random-number generator                  *
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
 *   12/04/1995 BRR : Created.                                             *
 *=========================================================================*/
void Init_Random(void)
{
	DebugString("Init random number\n");
	/*
	**	Do nothing if we've loaded a multiplayer game, or we're playing back
	**	a recording; the random number generator is initialized by loading
	**	the game.
	*/
	if (Session.LoadGame || Session.Play) {
		Scen->RandomNumber = Seed;
		NonCriticalRandomNumber = Seed;
		DebugString("Seed is %08x\n", Seed);
		return;
	}

	/*
	**	Initialize the random number Seed.  For multiplayer, this will have been done
	**	in the connection dialogs.  For single-player games, AND if we're not playing
	**	back a recording, init the Seed to a random value.
	*/
	if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {

	#ifdef WIN32
		/*
		**	Gather some "random" bits from the system timer. Actually, only the
		**	low order millisecond bits are secure. The other bits could be
		**	easily guessed from the system clock (most clocks are fairly accurate
		**	and thus predictable).
		*/
		SYSTEMTIME t;
		GetSystemTime(&t);
		CryptRandom.Seed_Byte(t.wMilliseconds);
		CryptRandom.Seed_Bit(t.wSecond);
		CryptRandom.Seed_Bit(t.wSecond>>1);
		CryptRandom.Seed_Bit(t.wSecond>>2);
		CryptRandom.Seed_Bit(t.wSecond>>3);
		CryptRandom.Seed_Bit(t.wSecond>>4);
		CryptRandom.Seed_Bit(t.wMinute);
		CryptRandom.Seed_Bit(t.wMinute>>1);
		CryptRandom.Seed_Bit(t.wMinute>>2);
		CryptRandom.Seed_Bit(t.wMinute>>3);
		CryptRandom.Seed_Bit(t.wMinute>>4);
		CryptRandom.Seed_Bit(t.wHour);
		CryptRandom.Seed_Bit(t.wDay);
		CryptRandom.Seed_Bit(t.wDayOfWeek);
		CryptRandom.Seed_Bit(t.wMonth);
		CryptRandom.Seed_Bit(t.wYear);
	#else

		/*
		**	Gather some "random" bits from the DOS mode timer.
		*/
		struct timeb t;
		ftime(&t);
		CryptRandom.Seed_Byte(t.millitm);
		CryptRandom.Seed_Byte(t.time);
	#endif

		/*
		**	Set the optional user-specified seed
		*/
		if (CustomSeed != 0) {
			Seed = CustomSeed;
		} else {
			CryptRandom.Get(&Seed, sizeof(Seed));
			Seed = GetTickCount();
			//srand(time(NULL));
			//Seed = rand();
		}
	}

	/*
	**	Initialize the random-number generators
	*/
	DebugString("Seed is %08x\n", Seed);
	Scen->RandomNumber = Seed;
	NonCriticalRandomNumber = Seed;
}


/***********************************************************************************************
 * Load_Title_Page -- Load the background art for the title page.                              *
 *                                                                                             *
 *    This routine will load the background art in a machine independent format. There is      *
 *    different art required for the hi-res and lo-res versions of the game.                   *
 *                                                                                             *
 * INPUT:   visible  -- Should the title page art be copied to the visible page by this        *
 *                      routine?                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Be sure the mouse is hidden if the image is to be copied to the visible page.   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void Load_Title_Page(const char * name, bool visible)
{
	HiddenSurface->Fill(0);
	Load_Title_Screen(name, HiddenSurface, &CCPalette);
	if (visible)
		Update_Visible_Surface(HiddenSurface);
}


/***********************************************************************************************
 * Init_Color_Remaps -- Initialize the text remap tables.                                      *
 *                                                                                             *
 *    There are various color scheme remap tables that are dependant upon the color remap      *
 *    information embedded within the palette control file. This routine will fetch that       *
 *    data and build the text remap tables as indicated.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static void Init_Color_Remaps(void)
{
	GadgetClass::Set_Color_Scheme(DEFAULT_GADGET_SCHEME);
}


/***********************************************************************************************
 * Init_Heaps -- Initialize the game heaps and buffers.                                        *
 *                                                                                             *
 *    This routine will allocate the game heaps and buffers. The rules file has already been   *
 *    processed by the time that this routine is called.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static void Init_Heaps(void)
{
	/*
	**	Speech holding tank buffer. Since speech does not mix, it can be placed
	**	into a custom holding tank only as large as the largest speech file to
	**	be played.
	*/
	for (int index = 0; index < ARRAY_SIZE(SpeechBuffer); index++) {
		SpeechBuffer[index] = new char [SPEECH_BUFFER_SIZE];
		SpeechRecord[index] = VOX_NONE;
		assert(SpeechBuffer[index] != NULL);
	}

	/*
	**	Allocate the theater buffer block.
	*/
//	TheaterBuffer = new Buffer(THEATER_BUFFER_SIZE);
//	assert(TheaterBuffer != NULL);
}


/***********************************************************************************************
 * Init_Expansion_Files -- Fetch any override expansion mixfiles.                              *
 *                                                                                             *
 *    This routine will search for and register/cache any override mixfiles found.             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool Init_Expansion_Files(void)
{
#ifdef _DEMO
	HANDLE handle;
	WIN32_FIND_DATA ff;
	MFCD * ptr;

	/*
	**	Before all else, cache any additional mixfiles.
	*/
	handle = FindFirstFile("ECACHE*.MIX", &ff);

	while (handle != INVALID_HANDLE_VALUE) {
		if ((ff.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY)) == 0) {

			ptr = new MFCD(ff.cFileName, &FastKey);

			ExpandMix.Add(ptr);
			ptr->Cache();
		}

		if (FindNextFile(handle, &ff) == false) {
			break;
		}
	}

	if (handle != INVALID_HANDLE_VALUE) {
		FindClose(handle);
	}

	handle = FindFirstFile("ELOCAL*.MIX", &ff);

	while (handle != INVALID_HANDLE_VALUE) {
		if ((ff.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY)) == 0) {

			ptr = new MFCD(ff.cFileName, &FastKey);

			ExpandMix.Add(ptr);
		}

		if (FindNextFile(handle, &ff) == false) {
			break;
		}
	}

	if (handle != INVALID_HANDLE_VALUE) {
		FindClose(handle);
	}
#endif
	return(true);
}


/***********************************************************************************************
 * Init_One_Time_Systems -- Initialize internal pointers to the bulk data.                     *
 *                                                                                             *
 *    This performs the one-time processing required after the bulk data has been cached but   *
 *    before the game actually starts. Typically, this routine extracts pointers to all the    *
 *    embedded data sub-files within the main game data mixfile. This routine must be called   *
 *    AFTER the bulk data has been cached.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Call this routine AFTER the bulk data has been cached.                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool Init_One_Time_Systems(void)
{
	Call_Back();

	Map.One_Time();
	Logic.One_Time();
	Options.One_Time();
	Session.One_Time();
	ObjectTypeClass::One_Time();
	BuildingTypeClass::One_Time();
	BulletTypeClass::One_Time();
//	HouseTypeClass::One_Time();
	OverlayTypeClass::One_Time();
	SmudgeTypeClass::One_Time();
//	TerrainTypeClass::One_Time();
	UnitTypeClass::One_Time();
	InfantryTypeClass::One_Time();
	AnimTypeClass::One_Time();
	AircraftTypeClass::One_Time();
	HouseClass::One_Time();
	SpotLightClass::One_Time();
	IonBlastClass::One_Time();

	CCFileClass dropvxl("DPOD.VXL");
	DropPodVoxel.VoxLib = new VoxelLibrary(dropvxl);

	if (DropPodVoxel.VoxLib == NULL) {
		DebugString("Failed to create VoxLib!\n");
		return(false);
	}

	CCFileClass dropmot("DPOD.HVA");
	DropPodVoxel.MotLib = new MotionLibrary(dropmot);

	if (DropPodVoxel.MotLib == NULL) {
		DebugString("Failed to create MotLib!\n");
		return(false);
	}

	return(true);
}


/***********************************************************************************************
 * Init_Fonts -- Initialize all the game font pointers.                                        *
 *                                                                                             *
 *    This routine is used to fetch pointers to the game fonts. The mixfile containing these   *
 *    fonts must have been previously cached. This routine is a necessary prerequisite to      *
 *    displaying any dialogs or printing any text.                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool Init_Fonts(void)
{
	const void * ptr;

	ptr = MFCD::Retrieve("12METFNT.FNT");
	if (ptr == NULL) {
		return(false);
	}
	Metal12FontPtr = new WWFontClass(ptr);
	Metal12FontPtr->Set_XSpacing(1);

	ptr = MFCD::Retrieve("KIA6PT.FNT");
	if (ptr == NULL) {
		return(false);
	}
	MapFontPtr = new WWFontClass(ptr);
	MapFontPtr->Set_XSpacing(1);

	ptr = MFCD::Retrieve("6POINT.FNT");
	if (ptr == NULL) {
		return(false);
	}
	Font6Ptr = new WWFontClass(ptr, true);
	Font6Ptr->Set_XSpacing(1);

	ptr = MFCD::Retrieve("EDITFNT.FNT");
	if (ptr == NULL) {
		return(false);
	}
	EditorFont = new WWFontClass(ptr, true);
	EditorFont->Set_XSpacing(1);

	ptr = MFCD::Retrieve("8POINT.FNT");
	if (ptr == NULL) {
		return(false);
	}
	Font8Ptr = new WWFontClass(ptr, true);
	Font8Ptr->Set_XSpacing(1);

	ptr = MFCD::Retrieve("GRAD6FNT.FNT");
	if (ptr == NULL) {
		return(false);
	}
	GradFont6Ptr = new WWFontClass(ptr, true);
	GradFont6Ptr->Set_XSpacing(2);

	return(true);
}


/// <summary>
/// Mounts a named expansion mixfile straight off the disk.
/// The archive must be a loose file. Unlike its cached counterpart, this routine will not
/// find one that is buried inside another mixfile.
/// </summary>
/// <param name="name">The filename of the mixfile to mount.</param>
/// <returns>bool; Was the mixfile found and mounted?</returns>
static bool Add_Raw_Expansion_Mix(const char *name)
{
	if (RawFileClass(name).Is_Available()) {
		MFCD * expand = new MFCD(name, &FastKey);

		ExpandMix.Add(expand);
		DebugStringNoPrefix(" %s", name);
		return(true);
	}
	return(false);
}


/// <summary>
/// Mounts a named expansion mixfile and caches its contents.
/// The archive is looked for through the game's own file system, so it may itself be
/// buried inside another mixfile.
/// </summary>
/// <param name="name">The filename of the mixfile to mount.</param>
/// <returns>bool; Was the mixfile found and mounted?</returns>
static bool Add_CC_Expansion_Mix(const char *name)
{
	if (CCFileClass(name).Is_Available()) {
		MFCD * expand = new MFCD(name, &FastKey);

		ExpandMix.Add(expand);
		DebugStringNoPrefix(" %s", name);
		expand->Cache();
		return(true);
	}
	return(false);
}


/// <summary>
/// Mounts the numbered expansion mixfiles.
/// This routine brings in the loose expansion archives and the cached ones, which is how
/// added content gets to override what the game ships with.
/// </summary>
static void Init_Expand_Mixfiles(void)
{
	int index;
	char name[64];
	MFCD * expand;

	for (index = 99; index >= 0; index--) {
		sprintf(name, "EXPAND%02d.MIX", index);
		// Searched for as a loose file wherever the game's files are kept, but never
		// inside another archive.
		if (CDFileClass(name).Is_Available()) {
			expand = new MFCD(name, &FastKey);

			ExpandMix.Add(expand);
			DebugStringNoPrefix(" %s", name);
		}
	}

	for (index = 99; index >= 0; index--) {
		sprintf(name, "ECACHE%02d.MIX", index);
		if (CCFileClass(name).Is_Available()) {
			expand = new MFCD(name, &FastKey);

			ExpandMix.Add(expand);
			DebugStringNoPrefix(" %s", name);
			expand->Cache();
		}
	}
}


/// <summary>
/// Mounts the patch mixfiles.
/// This routine brings in the patch archives, which is how a released fix replaces files
/// that the game already shipped with.
/// </summary>
static void Init_Patch_Mixfiles(void)
{
	MFCD * expand;

	// As with the expansion archives, found loose in any of the game's folders but never
	// inside another archive.
	if (CDFileClass("PATCH.MIX").Is_Available()) {
		expand = new MFCD("PATCH.MIX", &FastKey);

		ExpandMix.Add(expand);
		DebugStringNoPrefix(" %s", "PATCH.MIX");
	}


	if (CCFileClass("PCACHE.MIX").Is_Available()) {
		expand = new MFCD("PCACHE.MIX", &FastKey);

		ExpandMix.Add(expand);
		DebugStringNoPrefix(" %s", "PCACHE.MIX");
		expand->Cache();
	}

}


/***********************************************************************************************
 * Init_Bootstrap_Mixfiles -- Registers and caches any mixfiles needed for bootstrapping.      *
 *                                                                                             *
 *    This routine will register the initial mixfiles that are required to display error       *
 *    messages and get input from the player.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Be sure to call this routine before any dialogs would be displayed to the       *
 *             player.                                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool Init_Bootstrap_Mixfiles(void)
{
	//int index;
	//char name[64];
	//MFCD * expand;

	Init_Patch_Mixfiles();
	Init_Expand_Mixfiles();


#ifndef _DEMO
	Detect_Addons();

	GameMix = new MFCD("TIBSUN.MIX", &FastKey);
#endif

	/*
	**	Bootstrap enough of the system so that the error dialog box can successfully
	**	be displayed.
	*/
	DebugStringNoPrefix(" CACHE.MIX");

	CacheMix = new MFCD("CACHE.MIX", &FastKey);

	if (MFCD::Cache("CACHE.MIX") == false) {
		return(false);
	}

	DebugStringNoPrefix(" CACHE.MIX");

	LocalMix = new MFCD("LOCAL.MIX", &FastKey);

	DebugStringNoPrefix(" LOCAL.MIX");

	return(true);
}


/***********************************************************************************************
 * Init_Secondary_Mixfiles -- Register and cache secondary mixfiles.                           *
 *                                                                                             *
 *    This routine is used to register the mixfiles that are needed for main menu processing.  *
 *    Call this routine before the main menu is display and processed.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool Init_Secondary_Mixfiles(void)
{
	/*
	**	Inform the file system of the various MIX files.
	*/
	if (CCFileClass("CONQUER.MIX").Is_Available()) {
		ConquerMix = new MFCD("CONQUER.MIX", &FastKey);
	}

	DebugStringNoPrefix(" CONQUER.MIX");

	if (ConquerMix == NULL) {
		return(false);
	}

	MFCD * mix;

	{
		// The map and multiplayer archives are not required. A deployment may keep the maps
		// and the multiplayer content loose or in archives of its own.
		std::vector<std::string> const maps = Search_Files("MAPS*.MIX");

		for (unsigned int index = 0; index < maps.size(); index++) {
			char const * found = maps[index].c_str();
			DebugStringNoPrefix(" %s", found);

			// The first archive found is the game's own; the rest are whatever else is
			// installed alongside it.
			if (index == 0) {
				MapsMix = new MFCD(found, &FastKey);
				continue;
			}

			mix = new MFCD(found, &FastKey);

			MapsMixLocal.Add(mix);
		}
	}

#ifndef _DEMO

	if (CCFileClass("MULTI.MIX").Is_Available()) {
		MultiMix = new MFCD("MULTI.MIX", &FastKey);

		DebugStringNoPrefix(" MULTI.MIX");
	}

#endif

	if (Addon_Installed(ADDON_FIRESTORM) == true) {
		if (CCFileClass("SOUNDS01.MIX").Is_Available()) {
			Sounds01Mix = new MFCD("SOUNDS01.MIX", &FastKey);
		}

		DebugStringNoPrefix(" SOUNDS01.MIX");

		if (Sounds01Mix == NULL) {
			return(false);
		}
	}

	if (CCFileClass("SOUNDS.MIX").Is_Available()) {
		SoundsMix = new MFCD("SOUNDS.MIX", &FastKey);
	}

	DebugStringNoPrefix(" SOUNDS.MIX");

	if (SoundsMix == NULL) {
		return(false);
	}

	/*
	**	Register the score mixfile.
	*/
	if (CCFileClass("SCORES.MIX").Is_Available()) {
		ScoresMix = new MFCD("SCORES.MIX", &FastKey);
	}

	DebugStringNoPrefix(" SCORES.MIX");

	if (ScoresMix == NULL) {
		return(false);
	}

	if (CCFileClass("SCORES01.MIX").Is_Available()) {
		Scores01Mix = new MFCD("SCORES01.MIX", &FastKey);
	}

	DebugStringNoPrefix(" SCORES01.MIX");

	ScoresPresent = true;
	Theme.Scan();

	{
		std::vector<std::string> const movies = Search_Files("MOVIES*.MIX");

		for (unsigned int index = 0; index < movies.size(); index++) {
			char const * found = movies[index].c_str();
			DebugStringNoPrefix(" %s", found);

			if (index == 0) {
				MoviesMix = new MFCD(found, &FastKey);
				continue;
			}

			mix = new MFCD(found, &FastKey);

			MoviesMixLocal.Add(mix);
		}
	}

	if (MoviesMix == NULL) {
		return(false);
	}

	return(true);
}


/***********************************************************************************************
 * Bootstrap -- Perform the initial bootstrap procedure.                                       *
 *                                                                                             *
 *    This routine will load and initialize the game engine such that a dialog box could be    *
 *    displayed. Because this is very critical, call this routine before any other game        *
 *    initialization code.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool Bootstrap(void)
{
	int index;

	/*
	**	Process the message loop until we are in focus. We need to be in focus to read pixels from
	**	the screen.
	*/
	do {
		Keyboard->Check();
	} while (!GameInFocus);

	/*
	**	Perform any special debug-only processing. This includes preparing the
	**	monochrome screen.
	*/
	Mono_Clear_Screen();

	/*
	**	Register and make resident all local mixfiles with particular emphasis
	**	on the mixfiles that are necessary to display and error messages and
	**	process further initialization.
	*/

	if (!Init_Bootstrap_Mixfiles()) {
		DebugString("Failed to initialize bootstrap mixfiles!\n");
		return(false);
	}

	/*
	**	Initialize the resident font pointers.
	*/
	if (!Init_Fonts()) {
		DebugString("Failed to initialize fonts!\n");
		return(false);
	}

	/*
	**	Setup the keyboard processor in preparation for the game.
	*/
	Keyboard->Clear();

	/*
	**	Fetch the language text from the hard drive first. If it cannot be
	**	found on the hard drive, then look for it in the mixfile.
	*/
//	SystemStrings = (char const *)MFCD::Retrieve("CONQUER.ENG");
//	DebugStrings = (char const *)MFCD::Retrieve("DEBUG.ENG");

	/*
	 * House specific scheme palette initialization.
	 */
	memmove((unsigned char *)&SchemePalette[0], (void *)MFCD::Retrieve("UNITSNO.PAL"), sizeof(SchemePalette));

	for (index = 0; index < 256; index++) {
		SchemePalette[index] = RGBClass(
				(unsigned char)(SchemePalette[index].Get_Red()<<2),
				(unsigned char)(SchemePalette[index].Get_Green()<<2),
				(unsigned char)(SchemePalette[index].Get_Blue()<<2));
	}

	/*
	**	Default palette initialization.
	*/
	memmove((unsigned char *)&GamePalette[0], (void *)MFCD::Retrieve("TEMPERAT.PAL"), sizeof(GamePalette));

	for (index = 0; index < 256; index++) {
		GamePalette[index] = RGBClass(
				(unsigned char)(GamePalette[index].Get_Red()<<2),
				(unsigned char)(GamePalette[index].Get_Green()<<2),
				(unsigned char)(GamePalette[index].Get_Blue()<<2));
	}

	OriginalPalette = GamePalette;
	CCPalette = GamePalette;
	WhitePalette[0] = BlackPalette[0];

	memmove((unsigned char *)&WaypointPalette[0], (void *)MFCD::Retrieve("WAYPOINT.PAL"), sizeof(WaypointPalette));

	for (index = 0; index < 256; index++) {
		WaypointPalette[index] = RGBClass(
				(unsigned char)(WaypointPalette[index].Get_Red()<<2),
				(unsigned char)(WaypointPalette[index].Get_Green()<<2),
				(unsigned char)(WaypointPalette[index].Get_Blue()<<2));
	}

	/*
	 * Voxel system initialization.
	 */
	CCFileClass vplfile("voxels.vpl");
	int vplres = VoxelDrawSystem::Load_VPL_File(vplfile);
	assert(vplres == 0);

	for (int i = 0; i < ARRAY_SIZE(VoxelRGBColors); i++) {
		VoxelPalette[i] = RGBClass(VoxelRGBColors[i].Red, VoxelRGBColors[i].Green, VoxelRGBColors[i].Blue);
	}

	UseVoxelCache = true;
	Set_Voxel_Camera_Angle(DefaultCameraAngle);
	Set_Voxel_Light_Angle(DefaultLightAngle);
	VoxelDrawSystem::Enable_Lighting();
	VoxelDrawSystem::Disable_ZBuffer();

	Init_Voxel_Matrices();

	/*
	 * Drawer system initialization.
	 */
	TerrainDrawer = new ConvertClass(GamePalette, GamePalette, *VisibleSurface, NUM_INTENSITY_LEVELS);

	PaletteClass pal;

	memmove((unsigned char *)&pal[0], (void *)MFCD::Retrieve("ANIM.PAL"), sizeof(pal));
	for (index = 0; index < 256; index++) {
		pal[index] = RGBClass(
				(unsigned char)(pal[index].Get_Red()<<2),
				(unsigned char)(pal[index].Get_Green()<<2),
				(unsigned char)(pal[index].Get_Blue()<<2));
	}

	AnimDrawer = new ConvertClass(pal, GamePalette, *VisibleSurface, NUM_INTENSITY_LEVELS);

	memmove((unsigned char *)&pal[0], (void *)MFCD::Retrieve("PALETTE.PAL"), sizeof(pal));
	for (index = 0; index < 256; index++) {
		pal[index] = RGBClass(
				(unsigned char)(pal[index].Get_Red()<<2),
				(unsigned char)(pal[index].Get_Green()<<2),
				(unsigned char)(pal[index].Get_Blue()<<2));
	}

	NormalDrawer = new ConvertClass(pal, GamePalette, *VisibleSurface, NUM_INTENSITY_LEVELS);

	memmove((unsigned char *)&pal[0], (void *)MFCD::Retrieve("UNITSNO.PAL"), sizeof(pal));
	for (index = 0; index < 256; index++) {
		pal[index] = RGBClass(
				(unsigned char)(pal[index].Get_Red()<<2),
				(unsigned char)(pal[index].Get_Green()<<2),
				(unsigned char)(pal[index].Get_Blue()<<2));
	}

	VoxelDrawer = new ConvertClass(pal, GamePalette, *VisibleSurface, NUM_INTENSITY_LEVELS);

	memmove((unsigned char *)&pal[0], (void *)MFCD::Retrieve("CAMEO.PAL"), sizeof(pal));
	for (index = 0; index < 256; index++) {
		pal[index] = RGBClass(
				(unsigned char)(pal[index].Get_Red()<<2),
				(unsigned char)(pal[index].Get_Green()<<2),
				(unsigned char)(pal[index].Get_Blue()<<2));
	}
	CameoDrawer = new ConvertClass(pal, GamePalette, *VisibleSurface, NUM_INTENSITY_LEVELS);

	memmove((unsigned char *)&pal[0], (void *)MFCD::Retrieve("MOUSEPAL.PAL"), sizeof(pal));
	for (index = 0; index < 256; index++) {
		pal[index] = RGBClass(
				(unsigned char)(pal[index].Get_Red()<<2),
				(unsigned char)(pal[index].Get_Green()<<2),
				(unsigned char)(pal[index].Get_Blue()<<2));
	}
	MouseDrawer = new ConvertClass(pal, GamePalette, *VisibleSurface);

	TiberiumDrawer = VoxelDrawer;

	/*
	**	Initialize expansion files (if present). Expansion files must be located
	**	in the current directory.
	*/
	Init_Expansion_Files();

	return(true);
}


/***********************************************************************************************
 * Init_Mouse -- Initialize the mouse system.                                                  *
 *                                                                                             *
 *    This routine will ensure that a valid mouse driver is present and a working mouse        *
 *    pointer can be displayed. The mouse is hidden when this routine exits.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void Init_Mouse(void)
{
	/*
	**	Since there is no mouse shape currently available we need
	**	to set one of our own.
	*/
	//ShowCursor(false);

	/// The menus run on the Windows cursor. The real game cursor is not loaded and assigned
	/// until play begins -- see MouseClass::One_Time, where these shapes are loaded again.
	ShapeSet const * temp_mouse_shapes = (ShapeSet const *)MFCD::Retrieve("MOUSE.SHP");

	if (temp_mouse_shapes) {
		Hide_Mouse();
		Point2D pt = Point2D(0, 0);
		Set_Mouse_Cursor(pt, temp_mouse_shapes, 0);

		while (Get_Mouse_State() < 0) Show_Mouse();

		Hide_Mouse();
		Show_Mouse();
	}

	Map.Set_Default_Mouse(MOUSE_NORMAL, false);
	//Show_Mouse();
	while (Get_Mouse_State() < 0) Show_Mouse();
	Call_Back();
	Hide_Mouse();
}


/***********************************************************************************************
 * Init_Bulk_Data -- Initialize the time-consuming mixfile caching.                            *
 *                                                                                             *
 *    This routine is called to handle the time consuming process of game initialization.      *
 *    The title page will be displayed when this routine is called.                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine will take a very long time.                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool Init_Bulk_Data(void)
{
	/*
	**	Cache the main game data. This operation can take a very long time.
	*/
	if (ConquerMix == NULL || !ConquerMix->Cache()) {
		return(false);
	}

	if (Audio_Available() && !Debug_Quiet) {
		if (SoundsMix != NULL && !SoundsMix->Cache()) {
			return(false);
		}
		if (Sounds01Mix != NULL && !Sounds01Mix->Cache()) {
			return(false);
		}
	}

	Call_Back();

	/*
	**	Fetch the tutorial message data.
	*/
	INIClass ini;
	CCFileClass file("TUTORIAL.INI");
	ini.Load(file);
	int count = ini.Entry_Count("Tutorial");
	for (int index = 0; index < count; index++) {
		char buffer[300];
		const char *entry = ini.Get_Entry("Tutorial", index);
		if (ini.Get_String("Tutorial", entry, "", buffer, sizeof(buffer))) {
			TutorialText.Add_Index(atoi(entry), (char *)strdup(buffer));
		}
	}

	/*
	**	Perform one-time game system initializations.
	*/
	return(Init_One_Time_Systems());
}


/***********************************************************************************************
 * Init_Keys -- Initialize the cryptographic keys.                                             *
 *                                                                                             *
 *    This routine will initialize the fast cryptographic key. It will also initialize the     *
 *    slow one if this is a scenario editor version of the game.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static void Init_Keys(void)
{
	DebugString("Init_Keys - declarations\n");
	RAMFileClass file((void *)Keys, strlen((char *)Keys));
	INIClass ini;
	DebugString("Init_Keys - Load\n");
	ini.Load(file);

	DebugString("Init_Keys - Init fast key\n");
	FastKey = ini.Get_PKey(true);
#ifdef _DEBUG
	DebugString("Init_Keys - Init slow key\n");
	SlowKey = ini.Get_PKey(false);
#endif
}


/***************************************************************************
 * Save_Recording_Values -- Saves multiplayer-specific values              *
 *                                                                         *
 * This routine saves multiplayer values that need to be restored for a    *
 * save game.  In addition to saving the random # seed for this scenario,  *
 * it saves the contents of the actual random number generator; this       *
 * ensures that the random # sequencer will pick up where it left off when *
 * the game was saved.                                                     *
 * This routine also saves the header for a Recording file, so it must     *
 * save some data not needed specifically by a save-game file (ie Seed).   *
 *                                                                         *
 * INPUT:                                                                  *
 *      file      file to save to                                          *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = success, false = failure                                    *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/28/1995 BRR : Created.                                             *
 *=========================================================================*/
bool Save_Recording_Values(CCFileClass & file)
{
	//Session.Save(file);
	DebugString("Saving recording values for scenario : %s\n", Scen->ScenarioName);
	file.Write(&BuildLevel, sizeof(BuildLevel));
#if defined(_DEBUG)
	file.Write(&Debug_Unshroud, sizeof(Debug_Unshroud));
#endif
	file.Write(&Seed, sizeof(Seed));
	file.Write(&Scen->Scenario, sizeof(Scen->Scenario));
	file.Write(Scen->ScenarioName, sizeof(Scen->ScenarioName));
	file.Write(&Whom, sizeof(Whom));
	file.Write(&Special, sizeof(SpecialClass));
	file.Write(&Options, sizeof(OptionsClass));
	return(true);
}


/***************************************************************************
 * Load_Recording_Values -- Loads multiplayer-specific values              *
 *                                                                         *
 * INPUT:                                                                  *
 *      file         file to load from                                     *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = success, false = failure                                    *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/28/1995 BRR : Created.                                             *
 *=========================================================================*/
bool Load_Recording_Values(CCFileClass & file)
{
	//Session.Load(file);
	file.Read(&BuildLevel, sizeof(BuildLevel));
#if defined(_DEBUG)
	file.Read(&Debug_Unshroud, sizeof(Debug_Unshroud));
#endif
	file.Read(&Seed, sizeof(Seed));
	file.Read(&Scen->Scenario, sizeof(Scen->Scenario));
	file.Read(Scen->ScenarioName, sizeof(Scen->ScenarioName));
	file.Read(&Whom, sizeof(Whom));
	file.Read(&Special, sizeof(SpecialClass));
	file.Read(&Options, sizeof(OptionsClass));
	DebugString("Loaded recording values for scenario : %s\n", Scen->ScenarioName);
	return(true);
}


/// <summary>
/// Appends the code letters of the active cheats to the version string.
/// This routine is used by the version text so that the corner of the screen betrays
/// which cheats are switched on.
/// </summary>
/// <param name="string">The version string to append the cheat letters to.</param>
/// <remarks>Be sure that the string has room enough for the extra characters.</remarks>
void Cheat_Version_Suffix(char * string)
{
	for (int c = 0; c < ARRAY_SIZE(CheatEntries); c++) {
		if (*CheatEntries[c].State == true && CheatEntries[c].VersionSuffix != NULL) {
			strcat(string, CheatEntries[c].VersionSuffix);
		}
	}
}


/// <summary>
/// Turns off every cheat that is not allowed in multiplay.
/// This routine is used before a network game begins, so that nobody can carry a single
/// player indulgence into a game where it would be noticed.
/// </summary>
static void Cheat_Disable(void)
{
	for (int c = 0; c < ARRAY_SIZE(CheatEntries); c++) {
		if (!CheatEntries[c].IsAllowedInMP) {
			*CheatEntries[c].State = false;
		}
	}
}


/// <summary>
/// Feeds a typed character to the cheat code recognizer.
/// This routine gathers up alphanumeric keystrokes and toggles a cheat the moment its code
/// appears in what has been typed so far. Anything else the player types wipes the slate.
/// </summary>
/// <param name="chr">The character that the player just typed.</param>
/// <returns>bool; Was a cheat toggled by this character?</returns>
bool Cheat_Key_Process(char chr)
{
	static char _buffer[32] = "";

	if (!isalnum(chr) || chr == '~') {
		memset(_buffer, 0, sizeof(_buffer));
		return(false);
	}

	char tmp[2];
	tmp[0] = chr;
	tmp[1] = 0;
	DebugStringNoPrefix("%s", tmp);

	int len = strlen(_buffer);

	if (len >= ARRAY_SIZE(_buffer)-1) {
		len = 0;
		_buffer[0] = 0;
	}

	_buffer[len] = toupper(chr);

	for (int c = 0; c < ARRAY_SIZE(CheatEntries); c++) {
		if (strstr(_buffer, CheatEntries[c].CheatString) != NULL) {
			*CheatEntries[c].State = !(*CheatEntries[c].State);
			memset(_buffer, 0, sizeof(_buffer));
			return(true);
		}
	}

	return(false);
}


/// <summary>
/// Handles the messages for the version information dialog.
/// This routine fills the list box with the game's title, its version numbers, the build
/// stamp, and a description of the processor it finds itself running upon. It is the
/// first thing to ask for when a player reports a problem.
/// </summary>
BOOL CALLBACK Version_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HWND handle;
	int *res;
	char buffer[256];

	int rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc) {
		return(rc);
	}

	res = (int *)GetWindowLong(window, DWL_USER);

	switch (message) {
		case WM_INITDIALOG:
			handle = GetDlgItem(window, IDC_VERSION_INFO);

			if (Addon_Installed(ADDON_FIRESTORM) == true) {
				strcpy(buffer, Fetch_String(TXT_SHORT_TITLE));
				strcat(buffer, ": ");
				strcat(buffer, Get_Addon_Title(ADDON_FIRESTORM));
				ListBox_AddString(handle, buffer);
			} else {
				ListBox_AddString(handle, Fetch_String(TXT_SHORT_TITLE));
			}

			sprintf(buffer, "Version %s", Version_Name());
			ListBox_AddString(handle, buffer);

			sprintf(buffer, "Internal Version %s", VerNum.Version_Name());
			ListBox_AddString(handle, buffer);

#ifdef _DEBUG
			sprintf(buffer, "Debug Build: %s - %s", OPENTS_BUILD_DESCRIPTION, OPENTS_COMMIT_DATE);
#else
			sprintf(buffer, "Release Build: %s - %s", OPENTS_BUILD_DESCRIPTION, OPENTS_COMMIT_DATE);
#endif
			ListBox_AddString(handle, buffer);

			/// The braces keep the 'case' label from jumping over these initializations.
			{
				int cpu_type = 5;
				bool mmx = false;
				char vendor[32];
				vendor[0] = '\0';
				Get_CPU_Type(cpu_type, mmx, vendor, sizeof(vendor) - 1);

				sprintf(buffer, "CPU vendor: %s", vendor);
			ListBox_AddString(handle, buffer);
			}

			Get_Language_Version(buffer);
			ListBox_AddString(handle, buffer);
			break;

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDCANCEL:
				case IDOK:
					*res = LOWORD(wparam);
					break;
			}
			break;
	}

	return(FALSE);
}


/// <summary>
/// Displays the version information dialog.
/// This routine does not return until the player dismisses the dialog, and keeps the
/// title screen alive behind it while it waits.
/// </summary>
void Version_Dialog(void)
{
	HWND dialog;
	int res = 0;

	dialog = OwnerDraw::Begin_Dialog(IDD_VERSION, (DLGPROC)Version_Dialog_Proc);

	if (dialog != NULL) {
		SetWindowLong(dialog, DWL_USER, (LONG)&res);
		OwnerDraw::Display_Dialog(dialog);

		while (res == 0) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				break;
			}
			Title_Screen_Restore();
		}
		OwnerDraw::End_Dialog(dialog);
	}
}


/***************************************************************************
 * Main_Menu -- Menu processing                                            *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      index of item selected, -1 if time out                             *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/17/1995 BRR : Created.                                             *
 *=========================================================================*/
int Main_Menu(unsigned int timeout)
{
	HWND dialog;
	int retval = SEL_NONE;

	timeout = 0;

	dialog = OwnerDraw::Begin_Dialog(IDD_MAIN_MENU, (DLGPROC) Main_Menu_Dialog_Proc);
	assert(dialog != NULL);

	if (dialog != NULL) {
		SetWindowLong(dialog, DWL_USER, (LONG)&retval);
		char *menu = Get_New_Menu()->Background;
		Load_Title_Screen(menu, HiddenSurface, &CCPalette);
		Draw_Version_Text(HiddenSurface);
		Update_Visible_Surface();
		OwnerDraw::Move_Dialog(dialog, -1, (HiddenSurface->Get_Height() - 400) / 2 + 147);
		OwnerDraw::Display_Dialog(dialog);
		SetFocus(MainWindow);

		do {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				retval = SEL_EXIT;
			}

			Title_Screen_Restore();

			if (Keyboard->Check()) {
				KeyNumType input = Keyboard->Get();

				switch ((unsigned int)input) {
					case (KN_V | KN_CTRL_BIT):
						ShowWindow(dialog, SW_HIDE);
						UpdateWindow(MainWindow);
						Version_Dialog();
						ShowWindow(dialog, SW_SHOW);
						UpdateWindow(dialog);
						SetFocus(MainWindow);
						break;

					case VK_C | KN_CTRL_BIT | KN_ALT_BIT:
						retval = SEL_VIEW_CREDITS;
						break;

					default:
						if ((input & KN_RLSE_BIT) == 0) {
							if (Cheat_Key_Process((char)input) == true) {
								Sound_Effect(Rule->OptionsChanged);
								Title_Screen_Restore(true);
							}
						}
						break;
				}
			}
		}
		while (retval == SEL_NONE);

		OwnerDraw::End_Dialog(dialog);

		/*
		 * Seed cryptographic random number generator.
		 */
		SYSTEMTIME t;
		GetSystemTime(&t);
		CryptRandom.Seed_Byte(t.wMilliseconds);
	} else {
		retval = SEL_EXIT;
	}

	SetFocus(MainWindow);
	return(retval);
}


/// <summary>
/// Handles the messages for the main menu dialog.
/// This routine records the button the player pressed into the result that Main_Menu is
/// waiting upon, and greys out the load button when there is nothing to load.
/// </summary>
BOOL CALLBACK Main_Menu_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int * res;

	int rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (rc) {
		return(rc);
	}

	res = (int *) GetWindowLong(window, DWL_USER);

	switch (message) {
		case WM_INITDIALOG: {
			HWND control = GetDlgItem(window, IDC_LOAD_MISSION);
			if (control) {
				if (LoadOptionsClass().Files_Present() == true) {
					EnableWindow(control, TRUE);
					return(FALSE);
				}
				EnableWindow(control, FALSE);
			}
		}
		break;

		case WM_COMMAND: {
			switch (LOWORD(wparam)) {
				case IDC_OPTIONS:
					*res = SEL_OPTIONS;
					break;

				case IDC_EXIT_GAME:
					*res = SEL_EXIT;
					break;

				case IDC_INTRO:
					*res = SEL_INTRO;
					break;

				case IDC_NEWCAMPAIGN:
					*res = SEL_CAMPAIGN_GAME;
					break;

				case IDC_MULTIPLAYER_GAME:
					*res = SEL_MULTIPLAYER_GAME;
					break;

				case IDC_LOAD_MISSION:
					*res = SEL_LOAD_GAME;
					break;
			}
		}
		break;
	}

	return(false);
}


/// <summary>
/// Redraws the title screen if the display surfaces have been lost.
/// The menu dialogs call this routine from their message loops, so that the background
/// comes back after the display has been taken away and handed back to the game.
/// </summary>
/// <param name="force">Should the title screen be redrawn even if nothing was lost?</param>
void Title_Screen_Restore(bool force)
{
	if (force == true) {
		HiddenSurface->Fill(0);
		char *menu = Get_New_Menu()->Background;
		Load_Title_Screen(menu, HiddenSurface, &CCPalette);
		Draw_Version_Text(HiddenSurface);
		Update_Visible_Surface(HiddenSurface);
	}
}


/// <summary>
/// Draws the version and copyright text onto the surface.
/// This routine is used to stamp the bottom right corner of the title screen. It does
/// nothing at all until the color schemes have been loaded.
/// </summary>
/// <param name="surface">The surface to print the version text upon.</param>
void Draw_Version_Text(Surface * surface)
{
	char version[128];
	Rect rect;

	if (Fetch_Scheme_By_Name("Green") == NULL) {
		return;
	}

	version[0] = '\0';
	strcpy(version, Version_Name());

	Cheat_Version_Suffix(version);

	rect = surface->Get_Rect();

	Fancy_Text_Print(
		"V%s",
		*surface,
		rect,
		Point2D(rect.X + rect.Width - 2, rect.Y + rect.Height - 20),
		Fetch_Scheme_By_Name("Green"),
		TBLACK,
		(TextPrintType)(TPF_EFNT|TPF_NOSHADOW|TPF_RIGHT),
		version
	);

	Fancy_Text_Print(
		Fetch_String(TXT_COPYRIGHT),
		*surface,
		rect,
		Point2D(rect.X + rect.Width - 2, rect.Y + rect.Height - 10),
		Fetch_Scheme_By_Name("Green"),
		TBLACK,
		(TextPrintType)(TPF_EFNT|TPF_NOSHADOW|TPF_RIGHT)
	);
}


static char _cmd_buffer[128];

class CreateTeamCommandClass : public CommandClass
{
	public:
		CreateTeamCommandClass(int team) : Team(team) {}

		virtual char const * Get_Unique_Name(void) const {
			sprintf(_cmd_buffer, "TeamCreate_%d", Team);
			return(_cmd_buffer);
		}
		virtual char const * Get_Display_Name(void) const {
			sprintf(_cmd_buffer, Fetch_String(TXT_CREATE_TEAM), Team);
			return(_cmd_buffer);
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_TEAM)));
		}
		virtual char const * Get_Description(void) const {
			sprintf(_cmd_buffer, Fetch_String(TXT_CREATE_TEAM_DESC), Team);
			return(_cmd_buffer);
		}

		virtual void Execute(void) const {
			for (int i = 0; i < Technos.Count(); i++) {
				TechnoClass * obj = Technos[i];
				if (obj && !obj->IsInLimbo && obj->House->Is_Player_Control()) {
					if (obj->Group == Team - 1) {
						obj->Group = -1;
					}
					if (obj->IsSelected) {
						obj->Group = Team - 1;
					}
				}
			}
		}

	private:
		int Team;
};


class SelectTeamCommandClass : public CommandClass
{
	public:
		SelectTeamCommandClass(int team) : Team(team) {}

		virtual char const * Get_Unique_Name(void) const {
			sprintf(_cmd_buffer, "TeamSelect_%d", Team);
			return(_cmd_buffer);
		}
		virtual char const * Get_Display_Name(void) const {
			sprintf(_cmd_buffer, Fetch_String(TXT_SELECT_TEAM), Team);
			return(_cmd_buffer);
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_TEAM)));
		}
		virtual char const * Get_Description(void) const {
			sprintf(_cmd_buffer, Fetch_String(TXT_SELECT_TEAM_DESC), Team);
			return(_cmd_buffer);
		}

		virtual void Execute(void) const {
			Map.Power_Mode_Control(0);
			Map.Waypoint_Mode_Control(0);
			Map.Repair_Mode_Control(0);
			Map.Sell_Mode_Control(0);

			if (CurrentObject.Count()) {
				if (!CurrentObject[0]->Is_Foot() || ((FootClass *)CurrentObject[0])->Group != (Team - 1)) {
					Unselect_All();
				}
			}
			for (int i = 0; i < Technos.Count(); i++) {
				TechnoClass * obj = Technos[i];
				if (obj && !obj->IsInLimbo && obj->Group == (Team - 1) && obj->House->Is_Player_Control()) {
					if (!obj->IsSelected) {
						obj->Select();
						AllowVoice = false;
					}
				}
			}
			AllowVoice = false;
			TechnoClass::Reset_Action_Line_Timer();
		}

	private:
		int Team;
};


class AddTeamCommandClass : public CommandClass
{
	public:
		AddTeamCommandClass(int team) : Team(team) {}

		virtual char const * Get_Unique_Name(void) const {
			sprintf(_cmd_buffer, "TeamAddSelect_%d", Team);
			return(_cmd_buffer);
		}
		virtual char const * Get_Display_Name(void) const {
			sprintf(_cmd_buffer, Fetch_String(TXT_ADD_SELECT_TEAM), Team);
			return(_cmd_buffer);
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_TEAM)));
		}
		virtual char const * Get_Description(void) const {
			sprintf(_cmd_buffer, Fetch_String(TXT_ADD_SELECT_TEAM_DESC), Team);
			return(_cmd_buffer);
		}

		virtual void Execute(void) const {
			Map.Power_Mode_Control(0);
			Map.Waypoint_Mode_Control(0);
			Map.Repair_Mode_Control(0);
			Map.Sell_Mode_Control(0);

			for (int i = 0; i < Technos.Count(); i++) {
				TechnoClass * obj = Technos[i];

				if (obj && !obj->IsInLimbo && obj->Group == Team-1 && obj->House->Is_Player_Control()) {

					if (!obj->IsSelected) {
						obj->Select();
						AllowVoice = false;
					}
				}
			}
		}

	private:
		int Team;
};


class CenterTeamCommandClass : public CommandClass
{
	public:
		CenterTeamCommandClass(int team) : Team(team) {}

		virtual char const * Get_Unique_Name(void) const {
			sprintf(_cmd_buffer, "TeamCenter_%d", Team);
			return(_cmd_buffer);
		}
		virtual char const * Get_Display_Name(void) const {
			sprintf(_cmd_buffer, Fetch_String(TXT_CENTER_TEAM), Team);
			return(_cmd_buffer);
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_TEAM)));
		}
		virtual char const * Get_Description(void) const {
			sprintf(_cmd_buffer, Fetch_String(TXT_CENTER_TEAM_DESC), Team);
			return(_cmd_buffer);
		}

		virtual void Execute(void) const {
			Map.Power_Mode_Control(0);
			Map.Waypoint_Mode_Control(0);
			Map.Repair_Mode_Control(0);
			Map.Sell_Mode_Control(0);

			if (CurrentObject.Count() && (!CurrentObject[0]->Is_Foot() || ((TechnoClass *)CurrentObject[0])->Group != Team - 1)) {
				Unselect_All();
			}

			for (int i = 0; i < Technos.Count(); i++) {
				TechnoClass * obj = Technos[i];
				if (obj && !obj->IsInLimbo && obj->Group == Team - 1 && obj->House->Is_Player_Control()) {
					if (!obj->IsSelected) {
						obj->Select();
						AllowVoice = false;
					}
				}
			}

			Map.Center_Map();
			Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
			AllowVoice = true;
		}

	private:
		int Team;
};


class PrevObjectCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("PreviousObject");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_PREV_OBJECT));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_SELECTION)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_PREV_OBJECT_DESC));
		}

		virtual void Execute(void) const {
			Map.Power_Mode_Control(0);
			Map.Waypoint_Mode_Control(0);
			Map.Repair_Mode_Control(0);
			Map.Sell_Mode_Control(0);

			ObjectClass * obj = Map.Prev_Object(CurrentObject.Count() ? CurrentObject[0] : NULL);

			if (obj != NULL) {
				Unselect_All();
				obj->Select();
				Map.Center_Map();
				Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
			}
		}
};


class StopCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("StopObject");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_STOP_OBJECT));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_CONTROL)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_STOP_OBJECT_DESC));
		}

		virtual void Execute(void) const {
			if (CurrentObject.Count() > 0) {
				for (int index = 0; index < CurrentObject.Count(); index++) {
					ObjectClass const * tech = CurrentObject[index];

					if (tech != NULL && (tech->Can_Player_Move() || tech->Can_Player_Fire())) {
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::IDLE, TargetClass(tech)));
					}
				}
				Sound_Effect(Rule->StopSound);
			}
		}
};


class DeployCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("DeployObject");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_DEPLOY_OBJECT));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_CONTROL)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_DEPLOY_OBJECT_DESC));
		}

		virtual void Execute(void) const {
			if (CurrentObject.Count() > 0) {
				bool done = false;
				AllowVoice = true;
				for (int index = 0; index < CurrentObject.Count(); index++) {
					ObjectClass * obj = CurrentObject[index];
					if (obj != NULL && (obj->Can_Player_Move() || obj->Can_Player_Fire())) {
						TechnoClass * tech = dynamic_cast<TechnoClass *>(obj);
						if (tech == NULL || (tech->Can_Attack_Now() && tech->Can_Deploy_Now())) {
							OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::DEPLOY, TargetClass(obj)));
							done = true;
							AllowVoice = false;
						}
					}
				}
				AllowVoice = true;
				if (done == true) {
					Sound_Effect(Rule->DeploySound);
				}
			}
		}
};


class GuardCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("GuardObject");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_GUARD));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_CONTROL)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_GUARD_DESC));
		}

		virtual void Execute(void) const {
			if (CurrentObject.Count() > 0) {
				AllowVoice = true;
				for (int index = 0; index < CurrentObject.Count(); index++) {
					TechnoClass * tech = dynamic_cast<TechnoClass *>(CurrentObject[index]);/// should be CurrentObject[index]->As_TechnoClass() but causes regswaps
					if (tech != NULL && tech->Can_Player_Move() && tech->Can_Player_Fire()) {
						tech->Player_Assign_Mission(MISSION_GUARD_AREA, tech->Get_Target_Cell_Ptr());
						AllowVoice = false;
					}
				}
				Sound_Effect(Rule->GuardSound);
				AllowVoice = true;
			}
		}
};


class ScatterCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ScatterObject");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SCATTER));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_CONTROL)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SCATTER_DESC));
		}

		virtual void Execute(void) const {
			if (CurrentObject.Count() > 0) {
				for (int index = 0; index < CurrentObject.Count(); index++) {
					ObjectClass const * tech = CurrentObject[index];

					if (tech != NULL && tech->Can_Player_Move()) {
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::SCATTER, TargetClass(tech)));
					}
				}
				Sound_Effect(Rule->ScatterSound);
			}
		}
};


class CenterViewCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("CenterView");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_CENTER_VIEW));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_SELECTION)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_CENTER_VIEW_DESC));
		}

		virtual void Execute(void) const {
			if (CurrentObject.Count() > 0) {
				Map.Center_Map();
				Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
			}
		}
};


class CenterBaseCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("CenterBase");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_CENTER_BASE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_SELECTION)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_CENTER_BASE_DESC));
		}

		virtual void Execute(void) const {

			Coord conyard_coord = COORD_NONE;
			Coord any_building_coord = COORD_NONE;
			int index = 0;
			bool none = true;

			if (PlayerPtr->CurBuildings) {
				for (index = 0; index < Buildings.Count(); index++) {
					none = false;
					BuildingClass * building = Buildings[index];

					if (building != NULL && !building->IsInLimbo && building->House->Is_Player_Control()) {
						if (building->Class == Rule->BaseUnit->DeploysInto) {
							conyard_coord = building->Center_Coord();
							if (building->IsLeader) {
								break;
							}
						} else if (any_building_coord == COORD_NONE) {
							any_building_coord = building->Center_Coord();
						}
					}
				}
			}

			if (!none) {
				if (any_building_coord == COORD_NONE && conyard_coord == COORD_NONE) {
					none = true;
				}
			}

			if (none) {
				if (PlayerPtr->CurUnits) {
					for (index = 0; index < Units.Count(); index++) {
						UnitClass * unit = Units[index];
						if (unit != NULL && !unit->IsInLimbo && unit->House->Is_Player_Control() && unit->Class == Rule->BaseUnit) {
							conyard_coord = unit->Center_Coord();
							break;
						}
					}
				}
			}

			if (conyard_coord != COORD_NONE) {
				TacticalMap->Set_Tactical_Position(conyard_coord);
			} else if (any_building_coord != COORD_NONE) {
				TacticalMap->Set_Tactical_Position(any_building_coord);
			}

			if (Map.PendingObject) {
				Map.Set_Cursor_Pos();
			}

			Map.Break_Follow_Mode();
			Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
		}
};


class AllianceCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ToggleAlliance");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_ALLIANCE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_CONTROL)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_ALLIANCE_DESC));
		}

		virtual void Execute(void) const {
			if (Session.Type != GAME_NORMAL && !Scen->Special.IsAllianceFixed && Session.Options.AlliesAllowed) {
				if (CurrentObject.Count() && !PlayerPtr->IsDefeated) {
					if (CurrentObject[0]->Owner_HouseClass() != PlayerPtr && CurrentObject[0]->Owner_HouseClass()->IsHuman) {
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::ALLY, CurrentObject[0]->Owner()));
					}
				}
			}
		}
};


class SelectViewCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SelectView");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SELECT_VIEW));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_SELECTION)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SELECT_VIEW_DESC));
		}

		virtual void Execute(void) const {
			TacticalMap->Select_These(TacticalRect);
//			TacticalMap->Select_These(TacticalMap->Pixel_To_Coord(TacticalRect.Top_Left()), TacticalMap->Pixel_To_Coord(TacticalRect.Bottom_Right()));
		}
};


class ToggleRepairCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ToggleRepair");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_REPAIR_MODE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_REPAIR_MODE_DESC));
		}

		virtual void Execute(void) const {
			Map.Repair_Mode_Control(-1);
		}
};


class ToggleSellCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ToggleSell");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SELL_MODE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SELL_MODE_DESC));
		}

		virtual void Execute(void) const {
			Map.Sell_Mode_Control(-1);
		}
};


class TogglePowerCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("TogglePower");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_POWER_MODE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_POWER_MODE_DESC));
		}

		virtual void Execute(void) const {
			Map.Power_Mode_Control(-1);
		}
};


class CenterREventCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("CenterOnRadarEvent");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_RADAR_EVENT));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_RADAR_EVENT_DESC));
		}

		virtual void Execute(void) const {
			Cell cell = LastRadarEventCell;

			if (cell != Cell(0,0)) {
				Coord coord = cell;
				int h = (Map[coord].Height * CELL_LEPTON);
				coord -= Coord(h / 2, h / 2);
				TacticalMap->Set_Tactical_Position(coord);

				if (Map.PendingObject != NULL) {
					Map.Set_Cursor_Pos();
				}
			}
		}
};


class ToggleRadarCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ToggleRadar");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_RADAR_TOGGLE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_RADAR_TOGGLE_DESC));
		}

		virtual void Execute(void) const {
			Map.Zoom_Mode_Control();
		}
};


class SidebarUpCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SidebarUp");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SIDEBAR_UP));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SIDEBAR_UP_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Scroll(true, -1);
		}
};


class LSidebarUpCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("LeftSidebarUp");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_LSIDEBAR_UP));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_LSIDEBAR_UP_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Scroll(true, 0);
		}
};


class RSidebarUpCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("RightSidebarUp");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_RSIDEBAR_UP));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_RSIDEBAR_UP_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Scroll(true, 1);
		}
};


class SidebarPageUpCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SidebarPageUp");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SIDEBAR_PGUP));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SIDEBAR_PGUP_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Page(true, -1);
		}
};


class LSidebarPageUpCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("LeftSidebarPageUp");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_LSIDEBAR_PGUP));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_LSIDEBAR_PGUP_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Page(true, 0);
		}
};


class RSidebarPageUpCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("RightSidebarPageUp");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_RSIDEBAR_PGUP));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_RSIDEBAR_PGUP_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Page(true, 1);
		}
};


class SidebarDownCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SidebarDown");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SIDEBAR_DOWN));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SIDEBAR_DOWN_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Scroll(false, -1);
		}
};


class LSidebarDownCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("LeftSidebarDown");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_LSIDEBAR_DOWN));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_LSIDEBAR_DOWN_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Scroll(false, 0);
		}
};


class RSidebarDownCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("RightSidebarDown");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_RSIDEBAR_DOWN));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_RSIDEBAR_DOWN_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Scroll(false, 1);
		}
};


class SidebarPageDownCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SidebarPageDown");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SIDEBAR_PGDN));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SIDEBAR_PGDN_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Page(false, -1);
		}
};


class LSidebarPageDownCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("LeftSidebarPageDown");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_LSIDEBAR_PGDN));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_LSIDEBAR_PGDN_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Page(false, 0);
		}
};


class RSidebarPageDownCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("RightSidebarPageDown");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_RSIDEBAR_PGDN));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_RSIDEBAR_PGDN_DESC));
		}

		virtual void Execute(void) const {
			Map.SidebarClass::Page(false, 1);
		}
};


class OptionsCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("Options");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_OPTIONS));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_OPTIONS_DESC));
		}

		virtual void Execute(void) const {
			Queue_Options();
		}
};


class ScrollNCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ScrollNorth");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SCROLL_N));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SCROLL_N_DESC));
		}

		virtual void Execute(void) const {
			int distance = 34;
			Map.Scroll_Map(FACING_N, distance, true);
		}
};


class ScrollSCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ScrollSouth");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SCROLL_S));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SCROLL_S_DESC));
		}

		virtual void Execute(void) const {
			int distance = 34;
			Map.Scroll_Map(FACING_S, distance, true);
		}
};


class ScrollECommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ScrollEast");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SCROLL_E));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SCROLL_E_DESC));
		}

		virtual void Execute(void) const {
			int distance = 34;
			Map.Scroll_Map(FACING_E, distance, true);
		}
};


class ScrollWCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ScrollWest");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SCROLL_W));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SCROLL_W_DESC));
		}

		virtual void Execute(void) const {
			int distance = 34;
			Map.Scroll_Map(FACING_W, distance, true);
		}
};


class View1CommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("View1");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_VIEW_BOOKMARK1));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_VIEW_BOOKMARK1_DESC));
		}

		virtual void Execute(void) const {
			Coord coord(Scen->Views[0]);
			coord.Z = Map.Get_Height_GL(coord);
			TacticalMap->Set_Tactical_Position(coord);
			if (Map.PendingObject) {
				Map.Set_Cursor_Pos();
			}
		}
};


class View2CommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("View2");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_VIEW_BOOKMARK2));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_VIEW_BOOKMARK2_DESC));
		}

		virtual void Execute(void) const {
			Coord coord(Scen->Views[1]);
			coord.Z = Map.Get_Height_GL(coord);
			TacticalMap->Set_Tactical_Position(coord);
			if (Map.PendingObject) {
				Map.Set_Cursor_Pos();
			}
		}
};


class View3CommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("View3");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_VIEW_BOOKMARK3));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_VIEW_BOOKMARK3_DESC));
		}

		virtual void Execute(void) const {
			Coord coord(Scen->Views[2]);
			coord.Z = Map.Get_Height_GL(coord);
			TacticalMap->Set_Tactical_Position(coord);
			if (Map.PendingObject) {
				Map.Set_Cursor_Pos();
			}
		}
};


class View4CommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("View4");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_VIEW_BOOKMARK4));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_VIEW_BOOKMARK4_DESC));
		}

		virtual void Execute(void) const {
			Coord coord(Scen->Views[3]);
			coord.Z = Map.Get_Height_GL(coord);
			TacticalMap->Set_Tactical_Position(coord);
			if (Map.PendingObject) {
				Map.Set_Cursor_Pos();
			}
		}
};


class SetView1CommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SetView1");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SET_BOOKMARK1));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SET_BOOKMARK1_DESC));
		}

		virtual void Execute(void) const {
			Point2D pixel = TacticalMap->Get_Relative_Tactical_Position();
			Scen->Views[0] = TacticalMap->Pixel_To_Cell(pixel);
		}
};


class SetView2CommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SetView2");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SET_BOOKMARK2));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SET_BOOKMARK2_DESC));
		}

		virtual void Execute(void) const {
			Point2D pixel = TacticalMap->Get_Relative_Tactical_Position();
			Scen->Views[1] = TacticalMap->Pixel_To_Cell(pixel);
		}
};


class SetView3CommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SetView3");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SET_BOOKMARK3));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SET_BOOKMARK3_DESC));
		}

		virtual void Execute(void) const {
			Point2D pixel = TacticalMap->Get_Relative_Tactical_Position();
			Scen->Views[2] = TacticalMap->Pixel_To_Cell(pixel);
		}
};


class SetView4CommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SetView4");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SET_BOOKMARK4));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SET_BOOKMARK4_DESC));
		}

		virtual void Execute(void) const {
			Point2D pixel = TacticalMap->Get_Relative_Tactical_Position();
			Scen->Views[3] = TacticalMap->Pixel_To_Cell(pixel);
		}
};


class FollowCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("Follow");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_FOLLOW));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_FOLLOW_DESC));
		}

		virtual void Execute(void) const {
			if (CurrentObject.Count() != 0 && Map.Object_To_Follow() == NULL) {
				Map.Set_To_Follow(CurrentObject[0]);
			} else {
				Map.Break_Follow_Mode();
			}
		}
};


class NextObjectCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("NextObject");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_NEXT_OBJECT));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_SELECTION)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_NEXT_OBJECT_DESC));
		}

		virtual void Execute(void) const {
			Map.Power_Mode_Control(0);
			Map.Waypoint_Mode_Control(0);
			Map.Repair_Mode_Control(0);
			Map.Sell_Mode_Control(0);

			ObjectClass * obj = Map.Next_Object(CurrentObject.Count() ? CurrentObject[0] : NULL);

			if (obj != NULL) {
				Unselect_All();
				obj->Select();
				Map.Center_Map();
				Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
			}
		}
};

/*
 * Toggles waypoint mode
 */
class WaypointCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("WaypointMode");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_WAYPOINTMODE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_WAYPOINTMODE_DESC));
		}
		virtual void Execute(void) const {
			Map.Waypoint_Mode_Control(-1);
		}
};

/*
 * Capture the screen to SCRNnnnn.PCX file.
 */
class ScreenCaptureCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ScreenCapture");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SCRNCAP));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SCRNCAP_DESC));
		}
		virtual void Execute(void) const {
			{
				/*
				 * The whole frame is captured whatever size the window happens to be,
				 * limited only by the surface it is copied into.
				 */
				Rect dest_rect = VisibleSurface->Get_Rect();
				dest_rect.Width = std::min(dest_rect.Width, HiddenSurface->Get_Width());
				dest_rect.Height = std::min(dest_rect.Height, HiddenSurface->Get_Height());

				Hide_Mouse();

				HiddenSurface->Blit_From(Rect(0, 0, HiddenSurface->Get_Width(), HiddenSurface->Get_Height()),
					*VisibleSurface, dest_rect);

				Show_Mouse();

				char fname[128];
				int index = -1;

				do {
					index++;
					sprintf(fname, "SCRN%04d.pcx", index);
				} while (CCFileClass(fname).Is_Available());

				CCFileClass file(fname);
				Write_PCX_File(file, *HiddenSurface, &GamePalette);
			}
		}
};


class PageUserCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("PageUser");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_PAGEUSER));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_PAGEUSER_DESC));
		}

		virtual void Execute(void) const {
			if (Session.Type == GAME_INTERNET) {
				Queue_PageUser();
			}
		}
};


class SelectSameTypeCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("SelectType");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_SELECT_TYPE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_SELECTION)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_SELECT_TYPE_DESC));
		}

		virtual void Execute(void) const {
			// A second press within half a second widens the sweep from the view to the whole map.
			int now = TickCount;
			bool widen = (LastTick >= 0) && (now - LastTick < TIMER_SECOND / 2);
			LastTick = now;

			SoughtTypes.clear();
			for (int i = 0; i < CurrentObject.Count(); i++) {
				ObjectClass * obj = CurrentObject[i];
				if (!obj->Is_Techno() || !((TechnoClass *)obj)->House->Is_Player_Control()) {
					continue;
				}
				SoughtTypes.insert(obj->TClass);
			}

			if (SoughtTypes.empty()) {
				return;
			}

			if (widen) {
				for (int i = 0; i < Technos.Count(); i++) {
					TechnoClass * techno = Technos[i];
					if (techno != NULL && techno->IsActive && techno->IsDown && Map.In_Radar(techno->Center_Coord())) {
						Select_Callback(techno);
					}
				}
			} else {
				TacticalMap->Select_These(TacticalRect, Select_Callback);
				}
			}

	private:
		/// <summary>
		/// Selects the object if it is one of the types being hunted for.
		/// This routine is handed to Select_These, and is called directly for the whole-map
		/// sweep, so that every object of a sought type under the player's control joins the
		/// selection.
		/// </summary>
		/// <param name="obj">The object being considered for selection.</param>
		/// <remarks>SoughtTypes must hold the desired types before calling this routine.</remarks>
		static void Select_Callback(ObjectClass * obj)
		{
			if (obj != NULL && obj->Is_Techno() && obj->IsDown && !obj->IsSelected && SoughtTypes.contains(obj->TClass) && ((TechnoClass *)obj)->House->Is_Player_Control()) {
				obj->Select();
			}
		}

		inline static std::unordered_set<TechnoTypeClass const *> SoughtTypes;
		inline static int LastTick = -1;
};


class ManualPlaceCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("ManualPlace");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_MANUAL_PLACE));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_MANUAL_PLACE_DESC));
		}

		virtual void Execute(void) const {
			FactoryClass * factory = PlayerPtr->Fetch_Factory(RTTI_BUILDING);
			if (factory == NULL || !factory->Has_Completed()) {
				return;
			}

			TechnoClass * pending = factory->Get_Object();
			if (pending == NULL || pending->RTTI != RTTI_BUILDING) {
				return;
			}

			if (Map.PendingObjectPtr == pending) {
				return;
			}

			BuildingClass * builder = pending->Who_Can_Build_Me(false, false);
			if (builder == NULL) {
				return;
			}

			// Drop any superweapon cursor, so that placing the building does not return to it.
			Map.IsTargettingMode = SUPER_NONE;

			PlayerPtr->Manual_Place(builder, (BuildingClass *)pending);
		}
};


class DeleteWaypointCommandClass : public CommandClass
{
	public:
		virtual char const * Get_Unique_Name(void) const {
			return("DeleteWaypoint");
		}
		virtual char const * Get_Display_Name(void) const {
			return(Fetch_String(TXT_DEL_WAYPOINT));
		}
		virtual char const * Get_Category(void) const {
			return(Fetch_String((TXT_INTERFACE)));
		}
		virtual char const * Get_Description(void) const {
			return(Fetch_String(TXT_DEL_WAYPOINT_DESC));
		}

		virtual void Execute(void) const {
			if (Map.DraggedWaypoint) {
				char waypoint_id;
				PathType path_type = PATH_NONE;
				PlayerPtr->Fetch_Waypoint_Data(Map.DraggedWaypoint, path_type, waypoint_id);
				PlayerPtr->Ensure_Path(path_type);
				PlayerPtr->Paths[path_type]->Delete_Waypoint((int)waypoint_id);
				Map.DraggedWaypoint = NULL;

				for (int i = Feet.Count() - 1; i >= 0; i--) {
					FootClass *foot = Feet[i];
					if (foot->House == PlayerPtr && foot->CurrentPath == path_type && foot->NextWaypoint > waypoint_id) {
						foot->NextWaypoint--;
					}
				}

				Show_Mouse();
			}
		}
};


/// <summary>
/// Builds the list of every command the player may invoke.
/// This routine is called once during startup to populate the command list, and then binds
/// the hotkeys to it. The delete and escape keys are claimed afterwards, so that no
/// keyboard file can take them away from the player.
/// </summary>
static void Init_Commands(void)
{
	AllCommands.Add(new FollowCommandClass);

	AllCommands.Add(new View1CommandClass);
	AllCommands.Add(new View2CommandClass);
	AllCommands.Add(new View3CommandClass);
	AllCommands.Add(new View4CommandClass);

	AllCommands.Add(new SetView1CommandClass);
	AllCommands.Add(new SetView2CommandClass);
	AllCommands.Add(new SetView3CommandClass);
	AllCommands.Add(new SetView4CommandClass);

	const CommandClass * optcmd = new OptionsCommandClass;
	AllCommands.Add(optcmd);

	AllCommands.Add(new ScrollNCommandClass);
	AllCommands.Add(new ScrollSCommandClass);
	AllCommands.Add(new ScrollECommandClass);
	AllCommands.Add(new ScrollWCommandClass);

	AllCommands.Add(new SidebarUpCommandClass);
	AllCommands.Add(new LSidebarUpCommandClass);
	AllCommands.Add(new RSidebarUpCommandClass);

	AllCommands.Add(new SidebarDownCommandClass);
	AllCommands.Add(new LSidebarDownCommandClass);
	AllCommands.Add(new RSidebarDownCommandClass);

	AllCommands.Add(new SidebarPageUpCommandClass);
	AllCommands.Add(new LSidebarPageUpCommandClass);
	AllCommands.Add(new RSidebarPageUpCommandClass);

	AllCommands.Add(new SidebarPageDownCommandClass);
	AllCommands.Add(new LSidebarPageDownCommandClass);
	AllCommands.Add(new RSidebarPageDownCommandClass);

	AllCommands.Add(new CenterREventCommandClass);

	AllCommands.Add(new ToggleRadarCommandClass);
	AllCommands.Add(new TogglePowerCommandClass);
	AllCommands.Add(new ToggleSellCommandClass);
	AllCommands.Add(new ToggleRepairCommandClass);

	AllCommands.Add(new SelectViewCommandClass);

	AllCommands.Add(new AllianceCommandClass);

	AllCommands.Add(new CenterBaseCommandClass);
	AllCommands.Add(new CenterViewCommandClass);

	AllCommands.Add(new ScatterCommandClass);
	AllCommands.Add(new GuardCommandClass);
	AllCommands.Add(new StopCommandClass);
	AllCommands.Add(new DeployCommandClass);

	AllCommands.Add(new PrevObjectCommandClass);
	AllCommands.Add(new NextObjectCommandClass);

	AllCommands.Add(new CreateTeamCommandClass(1));
	AllCommands.Add(new CreateTeamCommandClass(2));
	AllCommands.Add(new CreateTeamCommandClass(3));
	AllCommands.Add(new CreateTeamCommandClass(4));
	AllCommands.Add(new CreateTeamCommandClass(5));
	AllCommands.Add(new CreateTeamCommandClass(6));
	AllCommands.Add(new CreateTeamCommandClass(7));
	AllCommands.Add(new CreateTeamCommandClass(8));
	AllCommands.Add(new CreateTeamCommandClass(9));
	AllCommands.Add(new CreateTeamCommandClass(10));

	AllCommands.Add(new SelectTeamCommandClass(1));
	AllCommands.Add(new SelectTeamCommandClass(2));
	AllCommands.Add(new SelectTeamCommandClass(3));
	AllCommands.Add(new SelectTeamCommandClass(4));
	AllCommands.Add(new SelectTeamCommandClass(5));
	AllCommands.Add(new SelectTeamCommandClass(6));
	AllCommands.Add(new SelectTeamCommandClass(7));
	AllCommands.Add(new SelectTeamCommandClass(8));
	AllCommands.Add(new SelectTeamCommandClass(9));
	AllCommands.Add(new SelectTeamCommandClass(10));

	AllCommands.Add(new AddTeamCommandClass(1));
	AllCommands.Add(new AddTeamCommandClass(2));
	AllCommands.Add(new AddTeamCommandClass(3));
	AllCommands.Add(new AddTeamCommandClass(4));
	AllCommands.Add(new AddTeamCommandClass(5));
	AllCommands.Add(new AddTeamCommandClass(6));
	AllCommands.Add(new AddTeamCommandClass(7));
	AllCommands.Add(new AddTeamCommandClass(8));
	AllCommands.Add(new AddTeamCommandClass(9));
	AllCommands.Add(new AddTeamCommandClass(10));

	AllCommands.Add(new CenterTeamCommandClass(1));
	AllCommands.Add(new CenterTeamCommandClass(2));
	AllCommands.Add(new CenterTeamCommandClass(3));
	AllCommands.Add(new CenterTeamCommandClass(4));
	AllCommands.Add(new CenterTeamCommandClass(5));
	AllCommands.Add(new CenterTeamCommandClass(6));
	AllCommands.Add(new CenterTeamCommandClass(7));
	AllCommands.Add(new CenterTeamCommandClass(8));
	AllCommands.Add(new CenterTeamCommandClass(9));
	AllCommands.Add(new CenterTeamCommandClass(10));

	AllCommands.Add(new WaypointCommandClass);

	AllCommands.Add(new ScreenCaptureCommandClass);
	AllCommands.Add(new PageUserCommandClass);

	AllCommands.Add(new SelectSameTypeCommandClass);

	AllCommands.Add(new ManualPlaceCommandClass);

	const CommandClass * delwpcmd = new DeleteWaypointCommandClass;
	AllCommands.Add(delwpcmd);

	Init_Hotkeys();

	if (HotkeyCommands.Is_Present(KN_DELETE)) {
		HotkeyCommands.Remove_Index(KN_DELETE);
	}
	HotkeyCommands.Add_Index(KN_DELETE, delwpcmd);

	if (HotkeyCommands.Is_Present(KN_ESC)) {
		HotkeyCommands.Remove_Index(KN_ESC);
	}
	HotkeyCommands.Add_Index(KN_ESC, optcmd);
}


/// <summary>
/// Binds the game commands to the keys named in KEYBOARD.INI.
/// This routine throws the current key assignments away and rebuilds them from the
/// player's keyboard file, matching each entry against a command's unique name.
/// </summary>
/// <returns>bool; Was the keyboard file loaded?</returns>
/// <remarks>The command list must already be built before calling this routine.</remarks>
bool Init_Hotkeys(void)
{
	CCINIClass ini;
	CCFileClass file("KEYBOARD.INI");

	if (ini.Load(file, false)) {

		HotkeyCommands.Clear();

		for (int index = 0; index < ini.Entry_Count("Hotkey"); index++) {
			const char *entry = ini.Get_Entry("Hotkey", index);
			int id = ini.Get_Int("Hotkey", entry, 0);
			const CommandClass *command = NULL;

			for (int cindex = 0; cindex < AllCommands.Count(); cindex++) {
				if (strcmp(AllCommands[cindex]->Get_Unique_Name(), entry) == 0) {
					command = AllCommands[cindex];
					break;
				}
			}

			if ((command != NULL) && (id != 0)) {
				HotkeyCommands.Add_Index(id, command);
			}
		}

		return(true);
	}

	DebugString("Unable to load KEYBOARD.INI\n");
	return(false);
}


/// <summary>
/// Executes the command that goes by the specified unique name.
/// Use this routine where a command must be triggered by name rather than by the key it
/// happens to be bound to. A name that matches no command is quietly ignored.
/// </summary>
/// <param name="name">The unique name of the command to execute.</param>
void Execute_Command(char const * name)
{
	for (int command = 0; command < AllCommands.Count(); command++) {
		if (strcmp(AllCommands[command]->Get_Unique_Name(), name) == 0) {
			AllCommands[command]->Execute();
			break;
		}
	}
}


/// <summary>
/// Allocates the game's drawing surfaces.
/// This routine releases whatever surfaces are already in hand and builds a fresh set to
/// the dimensions given. A rectangle that is not valid means that surface is not wanted.
/// The composite and tile surfaces must share the same kind of memory as each other, so
/// both are pushed into system memory if the video card cannot hold the pair.
/// </summary>
/// <param name="hidden_rect">The dimensions for the hidden and alternate surfaces.</param>
/// <param name="composite_rect">The dimensions for the composite surface.</param>
/// <param name="tile_rect">The dimensions for the tile surface.</param>
/// <param name="sidebar_rect">The dimensions for the sidebar surface.</param>
/// <param name="hidden_first">Should the hidden surface get first claim on video memory?</param>
/// <returns>bool; Were the surfaces allocated?</returns>
bool Allocate_Surfaces(const Rect & hidden_rect, const Rect & composite_rect, const Rect & tile_rect, const Rect & sidebar_rect, bool hidden_first)
{
	bool success = true;

	DebugString("Allocating new surfaces\n");

	if (AlternateSurface != NULL) {
		DebugString("Deleting AlternateSurface\n");
		delete AlternateSurface;
		AlternateSurface = NULL;
	}

	if (HiddenSurface != NULL) {
		DebugString("Deleting HiddenSurface\n");
		delete HiddenSurface;
		HiddenSurface = NULL;
	}

	if (CompositeSurface != NULL) {
		DebugString("Deleting CompositeSurface\n");
		delete CompositeSurface;
		CompositeSurface = NULL;
	}

	if (TileSurface != NULL) {
		DebugString("Deleting TileSurface\n");
		delete TileSurface;
		TileSurface = NULL;
	}

	if (SidebarSurface != NULL) {
		DebugString("Deleting SidebarSurface\n");
		delete SidebarSurface;
		SidebarSurface = NULL;
	}

	if (hidden_first && hidden_rect.Is_Valid()) {
		HiddenSurface = new DSurface(hidden_rect.Width, hidden_rect.Height);
		assert(HiddenSurface != NULL);
		HiddenSurface->Fill(0);

		DebugString("HiddenSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
	}

	if (composite_rect.Is_Valid()) {
		CompositeSurface = new DSurface(composite_rect.Width, composite_rect.Height);
		CompositeSurface->Fill(0);

		DebugString("CompositeSurface (%dx%d)\n", composite_rect.Width, composite_rect.Height);
	}

	if (tile_rect.Is_Valid()) {
		TileSurface = new DSurface(tile_rect.Width, tile_rect.Height);
		TileSurface->Fill(0);

		DebugString("TileSurface (%dx%d)\n", tile_rect.Width, tile_rect.Height);
	}

	if (sidebar_rect.Is_Valid()) {
		SidebarSurface = new DSurface(sidebar_rect.Width, sidebar_rect.Height);
		SidebarSurface->Fill(0);

		DebugString("SidebarSurface (%dx%d)\n", sidebar_rect.Width, sidebar_rect.Height);
	}

	if (!hidden_first && hidden_rect.Is_Valid()) {
		HiddenSurface = new DSurface(hidden_rect.Width, hidden_rect.Height);
		HiddenSurface->Fill(0);

		DebugString("HiddenSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
	}

	if (hidden_rect.Is_Valid()) {
		AlternateSurface = new DSurface(hidden_rect.Width, hidden_rect.Height);
		assert(AlternateSurface != NULL);
		AlternateSurface->Fill(0);

		DebugString("AlternateSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
	}


	return(success);
}


/// <summary>
/// Initializes the game's threading support.
/// There are no worker threads to start -- the game runs entirely off the main thread.
/// </summary>
static void Init_Threads(void)
{
	//nothing
}


/// <summary>
/// Deletes every game object and object type in existence.
/// This routine is used when tearing a scenario down so that the next one may start from a
/// clean slate. Everything from bullets through houses to the tactical map itself is
/// released, and the cell array goes with them.
/// </summary>
/// <remarks>The game logic must not be running -- there is nothing left for it to run
/// upon.</remarks>
void Delete_All_Objects(void)
{
	ScenarioInit++;

	ObjectsToDelete.Clear();

	while (TargetTracker.Count()) {
		AbstractClass * ptr = TargetTracker.Fetch_By_Position(0);
		delete ptr;
	}
	Process_Deferred_Deletion();
	while (Bullets.Count()) {
		Bullets[0]->Release();
	}
	Process_Deferred_Deletion();
	while (Objects.Count()) {
		delete Objects[0];
	}
	Process_Deferred_Deletion();
	while (Tags.Count()) {
		delete Tags[0];
	}
	Process_Deferred_Deletion();
	while (Triggers.Count()) {
		delete Triggers[0];
	}
	Process_Deferred_Deletion();
	while (Tubes.Count()) {
		delete Tubes[0];
	}
	Process_Deferred_Deletion();
	while (BuildingLights.Count()) {
		delete BuildingLights[0];
	}
	Process_Deferred_Deletion();
	while (Overlays.Count()) {
		delete Overlays[0];
	}
	Process_Deferred_Deletion();
	while (ParticleSystems.Count()) {
		delete ParticleSystems[0];
	}
	Process_Deferred_Deletion();
	while (Waves.Count()) {
		delete Waves[0];
	}
	Process_Deferred_Deletion();
	while (Factories.Count()) {
		delete Factories[0];
	}
	Process_Deferred_Deletion();
	while (Sides.Count()) {
		delete Sides[0];
	}
	Process_Deferred_Deletion();
	while (Teams.Count()) {
		delete Teams[0];
	}
	Process_Deferred_Deletion();
	while (Houses.Count()) {
		delete Houses[0];
	}
	Process_Deferred_Deletion();
	while (Anims.Count()) {
		delete Anims[0];
	}
	Process_Deferred_Deletion();
	while (Scripts.Count()) {
		delete Scripts[0];
	}
	Process_Deferred_Deletion();
	while (LightSources.Count()) {
		delete LightSources[0];
	}
	Process_Deferred_Deletion();
	while (EMPulseClass::EMPulses.Count()) {
		delete EMPulseClass::EMPulses[0];
	}
	Process_Deferred_Deletion();
	while (SpotLights.Count()) {
		delete SpotLights[0];
	}
	Process_Deferred_Deletion();
	while (FoggedObjectClass::FoggyObjects.Count()) {
		delete FoggedObjectClass::FoggyObjects[0];
	}
	Process_Deferred_Deletion();
	while (AlphaShapes.Count()) {
		delete AlphaShapes[0];
	}
	Process_Deferred_Deletion();

	while (Terrains.Count()) {
		delete Terrains[0];
	}
	Process_Deferred_Deletion();

	LaserDrawClass::All_Clear();

	while (AbstractTypes.Count()) {
		delete AbstractTypes[0];
	}
	Process_Deferred_Deletion();

	delete TacticalMap;
	TacticalMap = NULL;
	Map.Free_Cells();
	Process_Deferred_Deletion();

	VeinholeMonsterClass::Reset();
	VeinholeMonsterClass::Clear_Global_Data();

	ScenarioInit--;

	/*
	 * Check if all deletions succeeded before leaving.
	 */
	assert(AbstractTypes.Count() == 0);
	assert(SpotLights.Count() == 0);
	assert(EMPulseClass::EMPulses.Count() == 0);
	assert(FoggedObjectClass::FoggyObjects.Count() == 0);
	assert(LightSources.Count() == 0);
	assert(Scripts.Count() == 0);
	assert(TargetTracker.Count() == 0);
	assert(Bullets.Count() == 0);
	assert(Objects.Count() == 0);
	assert(Tags.Count() == 0);
	assert(Triggers.Count() == 0);
	assert(Tubes.Count() == 0);
	assert(BuildingLights.Count() == 0);
	assert(Overlays.Count() == 0);
	assert(Particles.Count() == 0);
	assert(ParticleSystems.Count() == 0);
	assert(Waves.Count() == 0);
	assert(Factories.Count() == 0);
	assert(Sides.Count() == 0);
	assert(Teams.Count() == 0);
	assert(Houses.Count() == 0);
	assert(Anims.Count() == 0);
}


/***********************************************************************************************
 * DisplayClass::Init_Theater -- Performs theater-specific initialization (mixfiles, etc)      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      theater         new theater                                                            *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/17/1995 BRR : Created.                                                                 *
 *   05/07/1996 JLB : Added translucent tables.                                                *
 *=============================================================================================*/
void Init_Theater(TheaterType theater)
{
	char			fullname[16];
	char			shortname[16];
	char			isofullname[16];

	/*
	**	Unload old mixfiles, and cache the new ones
	*/
	wsprintf(fullname, "%s.MIX", Theaters[theater].Root);
	wsprintf(isofullname, "%s.MIX", Theaters[theater].IsoRoot);
	wsprintf(shortname, "%s.MIX", Theaters[theater].Suffix);

	DebugString("Init theater %s\n", Theaters[theater].Name);

	/*
	**	Save the new theater value
	*/
	Scen->Theater = theater;
	Session.Update_Progress(8);

	if (Scen->Theater != LastTheater) {
		if (TheaterData != NULL) {
			delete TheaterData;
		}
		TheaterData = new MFCD(fullname, &FastKey);

		if (TheaterDat != NULL) {
			delete TheaterDat;
		}
		TheaterDat = new MFCD(shortname, &FastKey);
		TheaterDat->Cache();

		TheaterData->Cache();
		Session.Update_Progress(6);

		if (IsometricTheaterData != NULL) {
			delete IsometricTheaterData;
		}
		IsometricTheaterData = new MFCD(isofullname, &FastKey);

		Session.Update_Progress(12);

		/*
		**	Load the custom palette associated with this theater.
		**	The fading palettes will have to be generated as well.
		*/
		wsprintf(fullname, "%s.PAL", Theaters[theater].Root);

		unsigned char * ptr = (unsigned char *)MFCD::Retrieve(fullname);

		assert(ptr != NULL);
		if (ptr != NULL) {
			for (int color = 0; color < PaletteClass::COLOR_COUNT; color++) {
				unsigned char r = (unsigned char)((*ptr++)<<2);
				unsigned char g = (unsigned char)((*ptr++)<<2);
				unsigned char b = (unsigned char)((*ptr++)<<2);
				GamePalette[color] = RGBClass(r, g, b);
			}
		} else {
			for (int color = 0; color < PaletteClass::COLOR_COUNT; color++) {
				GamePalette[color] = RGBClass(color, 255 - color, ((color << 2) & 0xff));
			}
		}

		OriginalPalette = GamePalette;

		PaletteClass * unitpal = NULL;
		char const * palname = NULL;

		bool valid = false;
		switch (theater) {
			case THEATER_TEMPERATE:
				valid = true;
				palname ="UNITTEM.PAL";
				break;
			case THEATER_SNOW:
				valid = true;
				palname = "UNITSNO.PAL";
				break;
		};

		if (valid) {
			unitpal = (PaletteClass *)MFCD::Retrieve(palname);
		}

		Call_Back();

		if (unitpal != NULL) {
			SchemePalette = *unitpal;
		}

		for (int index = 0; index < 256; index++) {
			SchemePalette[index] = RGBClass(
					(unsigned char)(SchemePalette[index].Get_Red()<<2),
					(unsigned char)(SchemePalette[index].Get_Green()<<2),
					(unsigned char)(SchemePalette[index].Get_Blue()<<2));
		}

		int last_percent = 12;
		int prog_step = ColorSchemes.Count() / (25 - 12);
		for (int s = 0; s < ColorSchemes.Count(); s++) {
			ColorSchemes[s]->Build_Light_Converters(SchemePalette, GamePalette);
			int percent = (std::min(12 + (s / prog_step),25));
			if (percent != last_percent) {
				Session.Update_Progress(percent);
				last_percent = percent;
			}
			Call_Back();
		}

		Session.Update_Progress(25);

		Map.Reload_Sidebar();

		Session.Update_Progress(28);
	}
}


/// <summary>
/// Prepares the art and data mixfiles for the specified side.
/// This routine is called whenever the side being played changes. It releases the previous
/// side's archives, mounts the cached, uncached and CD archives belonging to the new one,
/// and then lets the map rebuild whatever it keeps on a per house basis.
/// </summary>
/// <param name="side">The side whose archives should be made available.</param>
/// <returns>bool; Were all of the required archives for the side found?</returns>
bool Prep_For_Side(SideType side)
{
	int id;
	char name[64];
	int index;

	DebugString("Preparing Mixfiles for Side %02d.\n", side);

	if (SideCMix != NULL) {
		DebugString("     Releasing %s\n", SideCMix->Filename);
		delete SideCMix;
		SideCMix = NULL;
	}

	if (SideNCMix != NULL) {
		DebugString("     Releasing %s\n", SideNCMix->Filename);
		delete SideNCMix;
		SideNCMix = NULL;
	}

	if (SideCDMix != NULL) {
		DebugString("     Releasing %s\n", SideCDMix->Filename);
		delete SideCDMix;
		SideCDMix = NULL;
	}

	id = (int)side + 1;

	while (ExpandSideMix.Count() > 0) {
		delete ExpandSideMix[0];
		ExpandSideMix.Delete_Index(0);
	}

	if (Addon_Enabled(ADDON_ANY) == true) {
		for (index = 99; index >= 0; index--) {
			sprintf(name, "E%02dSC%02d.MIX", index, id);

			if (CCFileClass(name).Is_Available()) {

				DebugString("     Initializing %s\n", name);
				MFCD * mix = new MFCD(name, &FastKey);
				ExpandSideMix.Add(mix);
				mix->Cache();
			}
		}
	}

	sprintf(name, "SIDEC%02d.MIX", id);
	DebugString("     Initializing %s\n", name);

	if (CCFileClass(name).Is_Available()) {
		SideCMix = new MFCD(name, &FastKey);
	}

	if (SideCMix == NULL) {
		DebugString("     FAILED!\n");
		return(false);
	}

	SideCMix->Cache();

	if (Addon_Enabled(ADDON_ANY) == true) {
		for (index = 99; index >= 0; index--) {
			sprintf(name, "E%02dSNC%02d.MIX", index, id);

			if (CCFileClass(name).Is_Available()) {

				DebugString("     Initializing %s\n", name);
				MFCD *mix = new MFCD(name, &FastKey);
				ExpandSideMix.Add(mix);
			}
		}
	}

	sprintf(name, "SIDENC%02d.MIX", id);
	DebugString("     Initilizing %s\n", name);

	if (CCFileClass(name).Is_Available()) {
		SideNCMix = new MFCD(name, &FastKey);
	}

	if (Session.Type == GAME_NORMAL) {

		if (Addon_Enabled(ADDON_ANY) == false) {
			sprintf(name, "SIDECD%02d.MIX", id);
		} else {
			sprintf(name, "E%02dSCD%02d.MIX", Get_Required_Addon(), id);
		}

		DebugString("     Initilizing %s\n", name);
		if (CCFileClass(name).Is_Available()) {
			SideCDMix = new MFCD(name, &FastKey);
		}
		if (SideCDMix == NULL) {
			DebugString("     FAILED!\n");
			return(false);
		}
	}

	Map.Init_For_House();

	return(true);
}


/// <summary>
/// Prepares the speech mixfiles for the specified side.
/// This routine releases whatever voices are currently mounted and brings in the archive
/// belonging to the new side, along with any expansion voices that apply to it.
/// </summary>
/// <param name="side">The side whose speech should be made available.</param>
/// <returns>bool; Was the speech archive for the side found and mounted?</returns>
bool Prep_Speech_For_Side(SideType side)
{
	int id;
	char name[64];

	if (side == SIDE_NONE) {
		return(false);
	}

	if (SpeechMix != NULL) {
		DebugString("     Releasing %s\n", SpeechMix->Filename);
		delete SpeechMix;
		SpeechMix = NULL;
	}

	while (ExpandSpeechMix.Count() > 0) {
		delete ExpandSpeechMix[0];
		ExpandSpeechMix.Delete_Index(0);
	}

	id = (int)side + 1;

	for (AddonType addon = ADDON_COUNT; addon > 0; --addon) {
		if (Addon_Enabled(addon) == true) {
			sprintf(name, "E%02dVOX%02d.MIX", addon, id);

			if (CCFileClass(name).Is_Available()) {
				MFCD *mix = new MFCD(name, &FastKey);
				ExpandSpeechMix.Add(mix);
				DebugStringNoPrefix(" %s", name);
			}
		}
	}

	sprintf(name, "SPEECH%02d.MIX", id);
	DebugString("     Initilizing %s\n", name);
	if (CCFileClass(name).Is_Available()) {
		SpeechMix = new MFCD(name, &FastKey);
	}

	if (SpeechMix == NULL) {
		DebugString("     FAILED!\n");
		return(false);
	}

	return(true);
}


/// <summary>
/// Fetches the theme to play behind the main menu.
/// </summary>
/// <returns>Returns with the theme to play, favoring the expansion's own music whenever the
/// expansion is installed.</returns>
ThemeType Fetch_Main_Menu_Theme(void)
{
	if (Addon_Installed(ADDON_FIRESTORM)) {
		ThemeType theme = Theme.From_Name("FSMENU");
		if (theme != THEME_NONE) {
			return(theme);
		}
	}
	return(Theme.From_Name("INTRO"));
}


/// <summary>
/// Fetches the theme to play over the map selection screen.
/// </summary>
/// <returns>Returns with the theme to play, favoring the expansion's own music whenever the
/// expansion is installed.</returns>
ThemeType Fetch_Map_Select_Theme(void)
{
	if (Addon_Installed(ADDON_FIRESTORM)) {
		ThemeType theme = Theme.From_Name("MAPS");
		if (theme != THEME_NONE) {
			return(theme);
		}
	}
	return(Theme.From_Name("INTRO"));
}


/// <summary>
/// Fetches the most interesting object out of the current selection.
/// This routine is used wherever a single object must stand in for the whole selection.
/// An armed and mobile combatant outranks a defensive building, and anything that cannot
/// move under its own power ranks lowest of all.
/// </summary>
/// <returns>Returns with a pointer to the best object selected. Otherwise, NULL is
/// returned.</returns>
ObjectClass * Best_Selected_Object(void)
{
	ObjectClass *best_obj = NULL;
	int best_value = -1;

	if (CurrentObject.Count() > 0) {
		for (int index = 0; index < CurrentObject.Count(); index++) {
			ObjectClass * obj = CurrentObject[index];

			int value = 0;

			if (obj->Is_Techno()) {
				value = 2;
				TechnoClass * tech = (TechnoClass *)obj;
				if (tech->Is_Immobilized()) {
					value = 1;
				} else if (tech->Is_Weapon_Equipped()) {
					value = 3;
					if (tech->RTTI != RTTI_BUILDING) {
						value = 4;
						if (tech->Combat_Damage() > 0) {
							value = 5;
						}
					}
				}
			}

			if (value > best_value) {
				best_value = value;
				best_obj = obj;
			}
		}
	}
	return(best_obj);
}


/// <summary>
/// Handles one pass through the new front end menu.
/// This routine lets NewMenuClass gather the player's choice, sets up the session for
/// whichever flavor of multiplay was picked, and hands control back to the old menu if
/// the player asked for it.
/// </summary>
/// <returns>Returns with the SEL_ selection that the game loop should act upon.</returns>
int New_Main_Menu(void)
{
	NewMenuClass * newmenu = Get_New_Menu();

	Session.Type = GAME_NORMAL;
	Session.IsWDT = false;

	int selection = newmenu->Process_Game_Select();

	if (selection == NSEL_OLD_MENU) {
		return(Main_Menu(TIMER_MINUTE));
	}

	switch (selection) {
		case NSEL_START_NEW_GAME:
			return(SEL_CAMPAIGN_GAME);

		case NSEL_LOAD_MISSION:
			return(SEL_LOAD_GAME);

		case NSEL_LAN:
			Session.Type = GAME_IPX;
			break;

		case NSEL_INTERNET:
			Session.Type = GAME_INTERNET;
			break;

		case NSEL_SKIRMISH:
			Session.Type = GAME_SKIRMISH;
			break;

		case NSEL_WDT:
			Session.Type = GAME_INTERNET;
			Session.IsWDT = true;
			break;

		case NSEL_OPTIONS:
			return(SEL_OPTIONS);

		case NSEL_INTRO:
			return(SEL_INTRO);

		case NSEL_VERSION:
			return(SEL_VERSION);

		case NSEL_VIEW_CREDITS:
			return(SEL_VIEW_CREDITS);

		case NSEL_EXIT:
			return(SEL_EXIT);

		default:
			break;
	}

	if (Session.Type != GAME_NORMAL) {
		Session.Read_MultiPlayer_Settings();
		for (int i = 0; i < HouseTypes.Count(); i++) {
			HouseTypes[i]->Read_INI(*RuleINI);
		}
		Session.Suspended = false;
		Session.Read_Scenario_Descriptions();
		return(SEL_MULTIPLAYER_GAME);
	}

	return(SEL_NONE);
}
