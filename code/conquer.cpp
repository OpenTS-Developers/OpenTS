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

/* $Header: /CounterStrike/CONQUER.CPP 6     3/13/97 2:05p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CONQUER.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 3, 1991                                                *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CC_Draw_Shape -- Custom draw shape handler.                                               *
 *   Call_Back -- Main game maintenance callback routine.                                      *
 *   Color_Cycle -- Handle the general palette color cycling.                                  *
 *   Crate_From_Name -- Given a crate name convert it to a crate type.                         *
 *   Disk_Space_Available -- returns bytes of free disk space                                  *
 *   Do_Record_Playback -- handles saving/loading map pos & current object                     *
 *   Fading_Table_Name -- Builds a theater specific fading table name.                         *
 *   Fetch_Techno_Type -- Convert type and ID into TechnoTypeClass pointer.                    *
 *   Force_CD_Available -- Ensures that specified CD is available.                             *
 *   Get_Radar_Icon -- Builds and alloc a radar icon from a shape file                         *
 *   Handle_Team -- Processes team selection command.                                          *
 *   Handle_View -- Either records or restores the tactical view.                              *
 *   KN_To_Facing -- Converts a keyboard input number into a facing value.                     *
 *   Keyboard_Process -- Processes the tactical map input codes.                               *
 *   Language_Name -- Build filename for current language.                                     *
 *   List_Copy -- Makes a copy of a cell offset list.                                          *
 *   Main_Game -- Main game startup routine.                                                   *
 *   Main_Loop -- This is the main game loop (as a single loop).                               *
 *   Map_Edit_Loop -- a mini-main loop for map edit mode only                                  *
 *   Message_Input -- allows inter-player message input processing                             *
 *   MixFileHandler -- Handles VQ file access.                                                 *
 *   Name_From_Source -- retrieves the name for the given SourceType                           *
 *   Owner_From_Name -- Convert an owner name into a bitfield.                                 *
 *   Play_Movie -- Plays a VQ movie.                                                           *
 *   Shake_The_Screen -- Dispatcher that shakes the screen.                                    *
 *   Shape_Dimensions -- Determine the minimum rectangle for the shape.                        *
 *   Source_From_Name -- Converts ASCII name into SourceType.                                  *
 *   Sync_Delay -- Forces the game into a 15 FPS rate.                                         *
 *   Theater_From_Name -- Converts ASCII name into a theater number.                           *
 *   Unselect_All -- Causes all selected objects to become unselected.                         *
 *   VQ_Call_Back -- Maintenance callback used for VQ movies.                                  *
 *   Game_Registry_Key -- Returns pointer to string containing the registry subkey for the game.
 *   Is_Counterstrike_Installed -- Function to determine the availability of the CS expansion.
 *   Is_Aftermath_Installed -- Function to determine the availability of the AM expansion.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "conquer.h"

#include "_keyboar.h"
#include "_map.h"
#include "_palette.h"
#include "_rules.h"
#include "_source.h"
#include "_surface.h"
#include "_tactica.h"
#include "_theater.h"
#include "_tooltip.h"
#include "_wsproto.h"
#include "autosave.h"
#include "cctooltip.h"
#include "chat.h"
#include "data.h"
#include "dbgprint.h"
#include "desyncdlg.h"
#include "dsaudio.h"
#include "gamedirs.h"
#include "gamedlg.h"
#include "globals.h"
#include "houstype.h"
#include "incdec.h"
#include "init.h"
#include "ipxmgr.h"
#include "keyboard.h"
#include "language/language.h"
#include "loaddlg.h"
#include "logic.h"
#include "mainloop.h"
#include "msgbox.h"
#include "movie.h"
#include "movieskip.h"
#include "mplayer.h"
#include "msgloop.h"
#include "netdlg.h"
#include "netdlg2.h"
#include "netglobal.h"
#include "netshare.h"
#include "progress.h"
#include "queue.h"
#include "rules.h"
#include "saveload.h"
#include "scenario.h"
#include "session.h"
#include "sidebar.h"
#include "sounddlg.h"
#include "stats.h"
#include "surface.h"
#include "tactical.h"
#include "theme.h"
#include "voc.h"
#include "vox.h"
#include "wsproto.h"

#include "special.hh"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <direct.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <span>


/****************************************
**	Function prototypes for this module **
*****************************************/
bool Main_Loop(void);
void Message_Input(KeyNumType &input);
bool Map_Edit_Loop(void);

void Print_MP_Stats(void);

//
// Special module globals for recording and playback
//
extern int TeamEvent;
extern int TeamNumber;


void conquer_noop(void)
{

}


#if NEVER
/***********************************************************************************************
 * Special_Dialog -- Handles the special options dialog.                                       *
 *                                                                                             *
 *    This dialog is used when setting the special game options. It does not appear in the     *
 *    final version of the game.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Special_Dialog(void)
{
}
#endif


/// <summary>
/// Handles the in-game menu dialogs.
/// This routine is called by the main loop whenever a dialog has been requested. The
/// scenario is paused for as long as the player stays in the menus, and one menu chains
/// into the next until the player leaves them behind. In a multiplayer game the menus are
/// refused unless the player has already been defeated.
/// </summary>
void Ingame_Menu_Dialog(void)
{
	if (SpecialDialog != SDLG_NONE) {
		if (Session.Type != GAME_NORMAL) {
			if (PlayerPtr->IsToLose || PlayerPtr->IsToWin || PlayerPtr->IsToDie) {
				SpecialDialog = SDLG_NONE;
				return;
			}
			if (!_special_dialog_flag) {
				if (PlayerPtr->IsDefeated) {
					_special_dialog_flag = true;
				} else {
					SpecialDialog = SDLG_NONE;
					return;
				}
			}
		}

		Pause_Scenario();


		while (SpecialDialog != SDLG_NONE) {
			switch (SpecialDialog) {
				#if 0
				case SDLG_SPECIAL:
					Map.Help_Text(TXT_NONE);
					Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
					Special_Dialog();
					Map.Revert_Mouse_Shape();
					SpecialDialog = SDLG_NONE;
					break;
				#endif

				case SDLG_OPTIONS:
					Game_Options_Dialog();
					if (SpecialDialog != SDLG_OPTIONS) {
						break;
					}
					SpecialDialog = SDLG_NONE;
					break;

				case SDLG_QUICKLOAD:
					{
						AutosaveClass::KindType kind = Session.Type == GAME_NORMAL ? AutosaveClass::KindType::Campaign : AutosaveClass::KindType::Skirmish;
						if (LoadOptionsClass().Load_File(Quick_Save_File_Name(kind).c_str())) {
							Keyboard->Clear();
							IgnoreInput = Scen->IsInputLocked;
							if (!MouseCursor->Is_Hidden() && Scen->IsInputLocked) {
								Hide_Mouse();
							} else if (MouseCursor->Is_Hidden() && !Scen->IsInputLocked) {
								Show_Mouse();
							}
							Map.Flag_To_Redraw(GS_REDRAW_ALL);
							SpecialDialog = SDLG_NONE;
						} else {
							// The scenario may already be gone, so the player is left in the options menu as a failed dialog load leaves them.
							WWMessageBox().Process(TXT_ERROR_LOADING_GAME, TXT_OK, TXT_NONE, TXT_NONE);
							SpecialDialog = SDLG_OPTIONS;
						}
					}
					break;

				case SDLG_SETTINGS:
					GameControlsClass().Dialog();
					if (SpecialDialog != SDLG_SETTINGS) {
						break;
					}
					SpecialDialog = SDLG_OPTIONS;
					break;

				case SDLG_SOUND:
					SoundControlsClass().Dialog();
					SpecialDialog = SDLG_SETTINGS;
					break;

				case SDLG_LOAD:
					SpecialDialog = Multiplayer_Load_Prompt() ? SDLG_NONE : SDLG_OPTIONS;
					break;

				case SDLG_KEYBOARD:
					Options.Hotkey_Dialog();
					SpecialDialog = SDLG_SETTINGS;
					break;

				case SDLG_ABORT:
					switch (Abort_Dialog()) {

						/// cancel
						case 1:
							Queue_Exit();
							SpecialDialog = SDLG_NONE;
							break;

						case 2:
							break;

						// abort
						case 3:
							if (Session.Type == GAME_NORMAL) {
								PlayerRestarts = true;
							} else {
								OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::DESTRUCT));
								_special_dialog_flag = false;
							}
							break;

					}
					SpecialDialog = SDLG_NONE;
					break;

				case SDLG_SURRENDER:
					if (!PlayerPtr->IsDefeated && !PlayerPtr->IsToWin && !PlayerPtr->IsToLose && !PlayerPtr->IsToDie && Surrender_Dialog(TXT_SURRENDER)) {
						if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
							PlayerPtr->Flag_To_Lose();
						} else {
							OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::DESTRUCT));
							_special_dialog_flag = false;
						}
					}
					SpecialDialog = SDLG_NONE;
					break;

				default:
					break;
			}
		}

		Resume_Scenario();
	}
}


/***********************************************************************************************
 * Main_Game -- Main game startup routine.                                                     *
 *                                                                                             *
 *    This is the first official routine of the game. It handles game initialization and       *
 *    the main game loop control.                                                              *
 *                                                                                             *
 *    Initialization:                                                                          *
 *    - Init_Game handles one-time-only inits                                                  *
 *    - Select_Game is responsible for initializations required for each new game played       *
 *      (these may be different depending on whether a multiplayer game is selected, and       *
 *      other parameters)                                                                      *
 *    - This routine performs any un-inits required, both for each game played, and one-time   *
 *                                                                                             *
 * INPUT:   argc  -- Number of command line arguments (including program name itself).         *
 *                                                                                             *
 *          argv  -- Array of command line argument pointers.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void Main_Game(int argc, char * argv[])
{
	static bool fade = true;

	/*
	**	Perform one-time-only initializations
	*/
	int ret = Init_Game(argc, argv);
	if (ret) {
		if (ret < 0) {
			MSGBOXPARAMS params;
			params.cbSize = sizeof(MSGBOXPARAMS);
			params.hwndOwner = MainWindow;
			params.hInstance = ProgramInstance;
			params.lpszText = Fetch_String(TXT_INITGAME_FAILED);
			params.lpszCaption = Fetch_String(TXT_SHORT_TITLE);
			params.dwStyle = (MB_OK | MB_ICONSTOP | MB_SETFOREGROUND | MB_TOPMOST);
			params.lpszIcon = NULL;
			params.dwContextHelpId = NULL;
			params.lpfnMsgBoxCallback = NULL;
			params.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
			MessageBoxIndirect(&params);
		}
		return;
	}

	/*
	**	Game processing loop:
	**	1) Select which game to play, or whether to exit (don't fade the palette
	**		on the first game selection, but fade it in on subsequent calls)
	**	2) Invoke either the main-loop routine, or the editor-loop routine,
	**		until they indicate that the user wants to exit the scenario.
	*/
	while (Select_Game(fade)) {
		fade = false;
		ScenarioInit = 0;		// Kludge.
		fade = true;

		/*
		**	Make the game screen visible, clear the keyboard buffer of spurious
		**	values, and then show the mouse.  This PRESUMES that Select_Game() has
		**	told the map to draw itself.
		*/
		Keyboard->Clear();
		/*
		**	Only show the mouse if we're not playing back a recording.
		*/
		if (Session.Play) {
			Hide_Mouse();
			TeamEvent = 0;
			TeamNumber = 0;
		}

		if (Session.Type == GAME_INTERNET) {
			Register_Game_Start_Time();
			GameStatisticsPacketSent = false;
			PacketLater = NULL;
			ConnectionLost = false;
		}

		if (ToolTips != NULL) {
			ToolTips->Activate(Options.ToolTips);
		}

		IgnoreInput = Scen->IsInputLocked;
		if (!Scen->IsInputLocked) {
			Show_Mouse();
		}

		SpecialDialog = SDLG_NONE;
		_special_dialog_flag = true;

#ifdef _DEBUG
		/*
		**	Scenario-editor version of main-loop processing
		*/
		for (;;) {
			/*
			**	Non-scenario-editor-mode: call the game's main loop
			*/
			if (!Debug_Map) {
				if (Main_Loop()) {
					break;
				}

				Ingame_Menu_Dialog();
			} else {

				/*
				**	Scenario-editor-mode: call the editor's main loop
				*/
				if (Map_Edit_Loop()) {
					break;
				}
			}
		}
#else
		/*
		**	Non-editor version of main-loop processing
		*/
		for (;;) {
			/*
			**	Call the game's main loop
			*/
			if (Main_Loop()) {
				break;
			}

			/*
			**	If the SpecialDialog flag is set, invoke the given special dialog.
			**	This must be done outside the main loop, since the dialog will call
			**	Main_Loop(), allowing the game to run in the background.
			*/
			Ingame_Menu_Dialog();
		}
#endif
		Stop_Ingame_Movie();

		if (ToolTips != NULL) {
			ToolTips->Activate(false);
		}
		Hide_Mouse();
		ScenarioActive = false;
		TacticalActive = false;

		DebugString("Game loop finished. Average FPS = %d\n", SecondsPassed ? TotalFrames / SecondsPassed : 0);

		Print_MP_Stats();

		//VisiblePage.Clear();
		Title_Screen_Restore(true);

		// Un-initialize whatever needs it, for each game played. The network is shut
		// down rather than left running, so that the next game starts from a fresh
		// one. Playback never opened it, so that case is skipped.
		if (Session.Record || Session.Play) {
			Session.RecordFile.Close();
		}

		if (Session.Type == GAME_IPX) {
			if (!Session.Play) {
				Shutdown_Network();
			}
		}

		/*
		**	If we're playing back, the mouse will be hidden; show it.
		**	Also, set all variables back to normal, to return to the main menu.
		*/
		if (Session.Play) {
			Session.Type = GAME_NORMAL;
			Session.Play = 0;
			Show_Mouse();
		}
	}

	if (MouseCursor != NULL) {
		MouseCursor->Release_Mouse();
	}

	/*
	**	Free the scenario description buffers
	*/
	//Session.Free_Scenario_Descriptions();
}


/***********************************************************************************************
 * Call_Back -- Main game maintenance callback routine.                                        *
 *                                                                                             *
 *    This routine handles all the "real time" processing that needs to                        *
 *    occur. This includes palette fading and sound updating. It needs                         *
 *    to be called as often as possible.                                                       *
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
void Call_Back(void)
{
	/*
	**	Music and speech maintenance
	*/
	if (Audio_Available() && GameInFocus == true) {
		Audio.Sound_Callback();
		Theme.AI();
		Speak_AI();
	}

	/*
	 * Network game maintenance.
	 */
	if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
		IPX_Call_Back();
	}
}


/// <summary>
/// Handles the game maintenance while the random map generator is working.
/// The generator calls this routine through its callback pointer so that sound, music, and
/// network traffic keep flowing while a map is being built.
/// </summary>
/// <returns>bool; Should the map generation be abandoned?</returns>
bool MapGen_Call_Back(void)
{
	Call_Back();
	return(false);
}


static NetGlobal::RejectionCounters GlobalPacketRejections;


/// <summary>Records a rejected global packet.</summary>
static void Record_Global_Packet_Rejection(NetGlobal::DecodeError error)
{
	NetGlobal::RejectionRecord const record = GlobalPacketRejections.Record(error);
	if (record.ShouldLog) {
		DebugString("In-game global packet drop [%s]: %u\n", NetGlobal::Error_Name(error), record.Count);
	}
}


/// <summary>Resolves a registered packet source.</summary>
static NodeNameType * Session_Member_From_Address(IPXAddressClass const & address, int & player_index, NetGlobal::DecodeError & error)
{
	std::array<NetGlobal::Endpoint, MAX_PLAYERS> endpoints = {};
	std::array<NodeNameType *, MAX_PLAYERS> players = {};
	std::array<int, MAX_PLAYERS> player_indices = {};
	std::size_t count = 0;

	player_index = -1;
	for (int index = 0; index < Session.Players.Count(); index++) {
		NodeNameType * player = Session.Players[index];
		if (player != NULL && count < endpoints.size()) {
			endpoints[count] = {player->Address.Get_IP(), player->Address.Get_Port()};
			players[count] = player;
			player_indices[count] = index;
			count++;
		}
	}

	NetGlobal::Endpoint const source{address.Get_IP(), address.Get_Port()};
	NetGlobal::EndpointResolution const resolution = NetGlobal::Resolve_Sender(source, std::span<NetGlobal::Endpoint const>(endpoints.data(), count));
	error = resolution.Error;
	if (resolution.Error != NetGlobal::DecodeError::NONE || resolution.RosterIndex < 0) {
		return(NULL);
	}

	player_index = player_indices[resolution.RosterIndex];
	return(players[resolution.RosterIndex]);
}


/// <summary>Builds the membership facts used to validate a global packet.</summary>
static NetGlobal::ValidationContext Global_Validation_Context(NodeNameType const * sender)
{
	NetGlobal::ValidationContext context;
	for (int index = 0; index < Session.Players.Count(); index++) {
		NodeNameType const * player = Session.Players[index];
		if (player != NULL && player->Player.ID >= 0 && player->Player.ID < static_cast<int>(context.ActivePlayers.size())) {
			context.ActivePlayers[player->Player.ID] = true;
		}
	}

	if (sender != NULL) {
		context.SenderIsMember = true;
		context.SenderPlayerID = sender->Player.ID;
		context.SenderPlayerColor = sender->Player.Color;
	}
	context.MasterPlayerID = Session.Master_Player_ID();
	return(context);
}


/// <summary>
/// Handles the network maintenance for a network game.
/// This routine services the network connection and deals with the global packets that
/// have arrived -- players signing off, chat messages, kick proposals, movie skip votes, and
/// the loading progress the other machines report. It needs to be called as often as possible.
/// </summary>
void IPX_Call_Back(void)
{
	Windows_Message_Handler();

	Ipx.Service();

	// The dialog's heartbeats must keep going while a nested dialog owns the message loop.
	DesyncDialog.Service();

	/*
	**	Read packets only if the game is "closed", so we don't steal global
	**	messages from the connection dialogs.
	*/
	if (!Session.NetOpen) {
		while (Ipx.Get_Global_Message (&Session.GPacket, sizeof(Session.GPacket), &Session.GPacketlen, &Session.GAddress, &Session.GProductID)) {

			if (Session.GProductID == IPXGlobalConnClass::COMMAND_AND_CONQUER2) {
				int sender_index = -1;
				NetGlobal::DecodeError resolution_error = NetGlobal::DecodeError::SENDER_NOT_MEMBER;
				NodeNameType * sender = Session_Member_From_Address(Session.GAddress, sender_index, resolution_error);
				NetGlobal::ValidationContext const context = Global_Validation_Context(sender);
				NetGlobal::DecodeError error = NetGlobal::Validate_In_Game_Packet(Session.GPacket, Session.GPacketlen, context);
				if (error == NetGlobal::DecodeError::SENDER_NOT_MEMBER && resolution_error == NetGlobal::DecodeError::AMBIGUOUS_SENDER) {
					error = resolution_error;
				}

				if (error != NetGlobal::DecodeError::NONE) {
					Record_Global_Packet_Rejection(error);
				} else {
					switch (Session.GPacket.Command) {
						case NET_QUERY_GAME:
						case NET_QUERY_PLAYER:
							Process_Global_Packet(&Session.GPacket, &Session.GAddress);
							break;

						case NET_PROPOSE_KICK:
							error = Kick_Packet_Received(sender->Player.ID, static_cast<int>(Session.GPacket.Kick.KickeeID));
							if (error != NetGlobal::DecodeError::NONE) {
								Record_Global_Packet_Rejection(error);
							}
							break;

						case NET_SIGN_OFF: {
							int const house = sender->Player.ID;
							std::string const name = sender->Name;
							if (Ipx.Connection_Index(house) >= 0) {
								Forget_Kick_Player(house);
								Destroy_Connection(house, 0);
								DesyncDialog.Notify_Player_Left(house, name.c_str());
							} else if (Multiplayer_Load_Is_In_Progress()) {
								// The connections are rebuilt after the load, so the seat itself goes.
								Multiplayer_Load_Unseat(sender_index);
							}
							break;
						}

						case NET_MESSAGE:
							Chat_Receive(Session.GPacket, Session.GAddress);
							break;

						case NET_HOST_ANNOUNCE:
							if (Session.MasterPlayerID == -1 || Session.MasterPlayerID == sender->Player.ID) {
								DebugString("Adopting %s (house %d) as master\n", sender->Name, sender->Player.ID);
								Session.Adopt_Master(sender->Player.ID, sender->Name);
								DesyncDialog.Notify_Master_Changed();
							} else {
								DebugString("Ignoring a host announcement from %s while %s is master\n",
									sender->Name, Session.MasterPlayerName);
							}
							break;

						case NET_DESYNC_HEARTBEAT:
							DesyncDialog.Notify_Heartbeat(sender->Player.ID);
							break;

						case NET_DESYNC_CONTINUE:
							DesyncDialog.Notify_Continue();
							break;

						case NET_LOAD_GAME:
							Multiplayer_Load_Receive(Session.GPacket.LoadGame.Slot);
							break;

						case NET_PROGRESS_REPORT:
							DebugString("Received progress message - %d%% from %s\n", Session.GPacket.Progress.Percent, sender->Name);
							Progress.Set_Progress_Percent(sender_index, Session.GPacket.Progress.Percent);
							break;

						case NET_READY_TO_GO:
							break;

						case NET_MOVIE_SKIP:
							MovieSkip::Receive(sender->Player.ID, Session.GPacket);
							break;

						default:
							Record_Global_Packet_Rejection(NetGlobal::DecodeError::INVALID_COMMAND);
							break;
					}
				}
			}

			Windows_Message_Handler();
			Ipx.Service();
		}
	}
}


/***********************************************************************************************
 * Source_From_Name -- Converts ASCII name into SourceType.                                    *
 *                                                                                             *
 *    This routine is used to convert an ASCII name representing a                             *
 *    SourceType into the actual SourceType value. Typically, this is                          *
 *    used when processing the scenario INI file.                                              *
 *                                                                                             *
 * INPUT:   name  -- The ASCII source name to process.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the SourceType represented by the name                                *
 *          specified.                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SourceType Source_From_Name(char const * name)
{
	if (name) {
		for (SourceType source = SOURCE_FIRST; source < SOURCE_COUNT; source++) {
			if (stricmp(SourceName[source], name) == 0) {
				return(source);
			}
		}
	}
	return(SOURCE_NONE);
}


/***********************************************************************************************
 * Name_From_Source -- retrieves the name for the given SourceType                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      source      SourceType to get the name for                                             *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      name of SourceType                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/15/1994 BR : Created.                                                                  *
 *=============================================================================================*/
char const * Name_From_Source(SourceType source)
{
	if ((unsigned)source < SOURCE_COUNT) {
		return(SourceName[source]);
	}
	return("None");
}


/***********************************************************************************************
 * Theater_From_Name -- Converts ASCII name into a theater number.                             *
 *                                                                                             *
 *    This routine converts an ASCII representation of a theater and converts it into a        *
 *    matching theater number. If no match was found, then THEATER_NONE is returned.           *
 *                                                                                             *
 * INPUT:   name  -- Pointer to ASCII name to convert.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the name converted into a theater number.                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TheaterType Theater_From_Name(char const * name)
{
	TheaterType	index;

	//if (name) {
		for (index = THEATER_FIRST; index < THEATER_COUNT; index++) {
			if (stricmp(name, Theaters[index].Name) == 0) {
				return(index);
			}
		}
	//}
	return(THEATER_NONE);
}


/***********************************************************************************************
 * KN_To_Facing -- Converts a keyboard input number into a facing value.                       *
 *                                                                                             *
 *    This routine determine which compass direction is represented by the keyboard value      *
 *    provided. It is used for map scrolling and other directional control operations from     *
 *    the keyboard.                                                                            *
 *                                                                                             *
 * INPUT:   input -- The KN number to convert.                                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the facing type that the keyboard number represents. If it could      *
 *          not be translated, then FACING_NONE is returned.                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FacingType KN_To_Facing(int input)
{
	input &= ~(KN_ALT_BIT|KN_SHIFT_BIT|KN_CTRL_BIT);
	switch (input) {
		case KN_LEFT:
			return(FACING_W);

		case KN_RIGHT:
			return(FACING_E);

		case KN_UP:
			return(FACING_N);

		case KN_DOWN:
			return(FACING_S);

		case KN_UPLEFT:
			return(FACING_NW);

		case KN_UPRIGHT:
			return(FACING_NE);

		case KN_DOWNLEFT:
			return(FACING_SW);

		case KN_DOWNRIGHT:
			return(FACING_SE);

		default:
			break;
	}
	return(FACING_NONE);
}

#ifdef _DEBUG
/***************************************************************************
 * Map_Edit_Loop -- a mini-main loop for map edit mode only                *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/19/1994 BR : Created.                                              *
 *=========================================================================*/
bool Map_Edit_Loop(void)
{
	if (!Debug_Map) return false;

	/*
	**	Redraw the map.
	*/
	Map.Render();

	if (GameActive) {

		/*
		**	Update the display, unless we're inside a dialog.
		*/
		if (SpecialDialog == SDLG_NONE && GameInFocus) {

			Map.Flag_To_Redraw();

			/*
			**	Get user input (keys, mouse clicks).
			*/
			KeyNumType input;
			int x;
			int y;

			Map.Input(input, x, y);

			/*
			**	Process keypress.
			*/
			if (input != KN_NONE) {

				/*
				**
				*/
				int scroll_distance = 21;
				bool scroll = false;

				if (Keyboard->Down(Options.KeySelect1) || Keyboard->Down(Options.KeySelect2)) {
					scroll_distance = scroll_distance * 2.5f;
					scroll = true;
				}

				if (Keyboard->Down(Options.KeyForceAttack1) || Keyboard->Down(Options.KeyForceAttack2)) {
					if (Map.MapRect.Width <= Map.MapRect.Height) {
						scroll_distance = Map.MapRect.Height * CELL_LEPTON;
					} else {
						scroll_distance = Map.MapRect.Width * CELL_LEPTON;
					}
					scroll = true;
				}

				if (scroll) {
					if (input == KN_E_LEFT) {
						Map.Scroll_Map(FACING_W, scroll_distance, true);
					}
					if (input == KN_E_RIGHT) {
						Map.Scroll_Map(FACING_E, scroll_distance, true);
					}
					if (input == KN_E_UP) {
						Map.Scroll_Map(FACING_N, scroll_distance, true);
					}
					if (input == KN_E_DOWN) {
						Map.Scroll_Map(FACING_S, scroll_distance, true);
					}
					input = KN_NONE;
				}

				switch (input) {
					case KN_ESC:
					case KN_SPACE:
						SpecialDialog = SDLG_OPTIONS;
						break;

					/*
					 * Block use of the arrow keys, the interfere with the gadgets.
					 */
					case KN_LEFT:
					case KN_RIGHT:
					case KN_UP:
					case KN_DOWN:
						break;

					default:
						Keyboard_Process(input);
						break;
				};
			}

			/// ::Frame++;

			Map.Render();
			TacticalMap->AI();
		}

	}

	Call_Back();								// maintains Theme.AI() for music
//	Color_Cycle();

	Sleep(1);

	return(!GameActive);
}


/// <summary>
/// Resizes the tactical view to suit the map editor.
/// This routine reallocates the drawing surfaces and hands the map its new view dimensions,
/// so that the tactical display either takes over the whole screen for editing, or shrinks
/// back to leave room for the sidebar and the tab strip.
/// </summary>
/// <param name="flag">Should the tactical view expand to fill the entire screen?</param>
static void Resize_Tactical_View(bool flag)
{
	static int _tab_height = 16;
	static int _sidebar_width = SidebarClass::SIDE_WIDTH;

	if (flag) {

		Rect hidden(0, 0, Options.ScreenWidth-_sidebar_width, Options.ScreenHeight);
		Rect comp(0, 0, Options.ScreenWidth, Options.ScreenHeight);
		Rect tile(0, 0, Options.ScreenWidth, Options.ScreenHeight);
		Rect sidebar(0, 0, _sidebar_width, Options.ScreenHeight);
		Allocate_Surfaces(hidden, comp, tile, sidebar);

		Rect view(0, 0, Options.ScreenWidth, Options.ScreenHeight);
		Map.Set_View_Dimensions(view);

		Sleep(2);

	} else {

		Rect hidden(0, 0, Options.ScreenWidth-_sidebar_width, Options.ScreenHeight);
		Rect comp(0, 0, Options.ScreenWidth-_sidebar_width, Options.ScreenHeight);
		Rect tile(0, 0, Options.ScreenWidth-_sidebar_width, Options.ScreenHeight);
		Rect sidebar(0, 0, _sidebar_width, Options.ScreenHeight);
		Allocate_Surfaces(hidden, comp, tile, sidebar);

		Rect view(0, _tab_height, Options.ScreenWidth-_sidebar_width, Options.ScreenHeight-_tab_height);
		Map.Set_View_Dimensions(view);

		Sleep(2);
	}
}


/***************************************************************************
 * Go_Editor -- Enables/disables the map editor                            *
 *                                                                         *
 * INPUT:                                                                  *
 *      flag      true = go into editor mode; false = go into game mode    *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/19/1994 BR : Created.                                              *
 *=========================================================================*/
void Go_Editor(bool flag)
{
	static bool _input_locked = false;

	Keyboard->Clear();

	/*
	 * Toggle the tactical view area.
	 */
	Resize_Tactical_View(flag);

	/*
	**	Go into Scenario Editor mode
	*/
	if (flag) {
		Debug_Map = true;
		Debug_Unshroud = true;

		/*
		 * Added otherwise scrolling does not work
		 */
		_input_locked = IgnoreInput;
		IgnoreInput = false;
		Scen->IsInputLocked = false;

		AllowVoice = false;

		/*
		**	Un-select any selected objects
		*/
		Unselect_All();
		Map.Abort_Drag_Select();

		/*
		**	Turn off the sidebar if it's on
		*/
		Map.Activate(0);

		/*
		**	Reset the map's Button list for the new mode
		*/
		Map.Init_IO();

		/*
		**	Force a complete redraw of the screen
		*/
		HiddenSurface->Fill(0);
		Map.Flag_To_Redraw(GS_REDRAW_ALL);
		Map.Render();

	} else {

		/*
		**	Go into normal game mode
		*/
		Debug_Map = false;
		Debug_Unshroud = false;

		/*
		 * Added otherwise scrolling does not restore after editor switch.
		 */
		IgnoreInput = _input_locked;
		Scen->IsInputLocked = _input_locked;

		AllowVoice = true;

		/*
		**	Un-select any selected objects
		*/
		Unselect_All();
		Map.Abort_Drag_Select();

		/*
		**	Reset the map's Button list for the new mode
		*/
		Map.Init_IO();

		/*
		**	Force a complete redraw of the screen
		*/
		HiddenSurface->Fill(0);
		Map.Flag_To_Redraw(GS_REDRAW_ALL);
		Map.Render();
	}

	Update_Visible_Surface(HiddenSurface);
}

#endif


/***********************************************************************************************
 * Unselect_All -- Causes all selected objects to become unselected.                           *
 *                                                                                             *
 *    This routine will unselect all objects that are currently selected.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Unselect_All(void)
{
	while (CurrentObject.Count()) {
		CurrentObject[0]->Unselect();
	}
}


/***********************************************************************************************
 * Fetch_Techno_Type -- Convert type and ID into TechnoTypeClass pointer.                      *
 *                                                                                             *
 *    This routine will convert the supplied RTTI type number and the ID value into a valid    *
 *    TechnoTypeClass pointer. If there is an error in conversion, then NULL is returned.      *
 *                                                                                             *
 * INPUT:   type  -- RTTI type of the techno class object.                                     *
 *                                                                                             *
 *          id    -- Integer representation of the techno sub type number.                     *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the techno type class object specified or NULL if the    *
 *          conversion could not occur.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TechnoTypeClass const * Fetch_Techno_Type(RTTIType type, int id)
{
	switch (type) {
		case RTTI_UNITTYPE:
		case RTTI_UNIT:
			if (id >= 0 && id < UnitTypes.Count()) {
				return(TechnoTypeClass *)(UnitTypes[id]);
			}
			break;

		case RTTI_BUILDINGTYPE:
		case RTTI_BUILDING:
			if (id >= 0 && id < BuildingTypes.Count()) {
				return(TechnoTypeClass *)(BuildingTypes[id]);
			}
			break;

		case RTTI_INFANTRYTYPE:
		case RTTI_INFANTRY:
			if (id >= 0 && id < InfantryTypes.Count()) {
				return(TechnoTypeClass *)(InfantryTypes[id]);
			}
			break;

		case RTTI_AIRCRAFTTYPE:
		case RTTI_AIRCRAFT:
			if (id >= 0 && id < AircraftTypes.Count()) {
				return(TechnoTypeClass *)(AircraftTypes[id]);
			}
			break;

		default:
			break;
	}
	return(NULL);
}


/***************************************************************************
 * DISK_SPACE_AVAILABLE -- returns bytes of free disk space                *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     returns amount of free disk space                           *
 *                                                                         *
 * HISTORY:                                                                *
 *   08/11/1995 PWG : Created.                                             *
 *=========================================================================*/
unsigned int Disk_Space_Available(void)
{
	ULARGE_INTEGER freebytecount;		// Free bytes on disk available to caller (caller may not have access to entire disk).
	ULARGE_INTEGER totalbytecount;		// Total bytes on disk.
	ULARGE_INTEGER totalfreebytecount;
	unsigned int diskspace;

	/// This pointer is declared as returning bool, where the API it is bound to returns BOOL.
	bool (__stdcall *getfreediskspaceex) (LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

	DebugString("Checking available disk space\n");

	/*
	 * Measured where the game's saved games will actually go, which is not the current
	 * directory once a player has one of their own.
	 */
	std::string const user_directory = User_File_Write_Name("");
	LPCTSTR const disk = user_directory.empty() ? NULL : user_directory.c_str();

	// Get the free disk space on the drive.
	// NOTE IML: For Win'95, must query for support for GetDiskFreeSpaceEx before using it - otherwise use GetDiskFreeSpace().
	HINSTANCE kernel = GetModuleHandle("KERNEL32.DLL");
	if (kernel != NULL) {
		getfreediskspaceex = (bool (_stdcall*) (LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER)) GetProcAddress (kernel, "GetDiskFreeSpaceExA");
		if (getfreediskspaceex != NULL) {

			DebugString("Using GetDiskFreeSpaceEx\n");

			// NOTE: This function uses GetDiskFreeSpaceEx() and therefore assumes Win '95 OSR2 or greater.
			if (!getfreediskspaceex(disk, &freebytecount, &totalbytecount, &totalfreebytecount)) {
				DWORD const error = GetLastError();
				DebugString("GetDiskFreeSpaceEx failed with error code %d - %s\n", error, Last_Error_Text(error));
			} else {
				/// Convert to a 32-bit integer.
				diskspace = int(((int)freebytecount.LowPart + ((int)freebytecount.HighPart * (double)((__int64)UINT_MAX + 1))) / (double)1024);
				DebugString("Free disk space is %d Mb\n", diskspace / 1024);
				return(diskspace);
			}
		} else {
			DWORD const error = GetLastError();
			DebugString("GetProcAddress failed with error code %d - %s\n", error, Last_Error_Text(error));
		}
	} else {
		DebugString("Failed to get module handle for KERNEL32.DLL\n");
	}

	DWORD sectorspercluster, bytespersector, freeclustercount, totalclustercount;

	// The Ex version is not available. Use the Win'95 version.
	// QUESTION: SDK docs say that values returned by this function are erroneous if partition > 2Gb.
	//				 Does that mean that the partition is guaranteed to be <= 2Gb if Ex is not available?

	if (GetDiskFreeSpace(disk, &sectorspercluster, &bytespersector, &freeclustercount, &totalclustercount)) {
		diskspace = ((sectorspercluster * bytespersector) / 1024) * freeclustercount;
		DebugString("Free disk space is %d Mb\n", diskspace / 1024);
		return(diskspace);
	}

	return(0);
}


/***********************************************************************************************
 * Crate_From_Name -- Given a crate name convert it to a crate type.                           *
 *                                                                                             *
 *    Use this routine to convert an ASCII crate name into a crate type. If no match could     *
 *    be found, then CRATE_MONEY is assumed.                                                   *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the crate name text to convert into a crate type.              *
 *                                                                                             *
 * OUTPUT:  Returns with the crate name converted into a crate type.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
CrateType Crate_From_Name(char const * name)
{
	if (name != NULL) {
		for (CrateType crate = CRATE_FIRST; crate < CRATE_COUNT; crate++) {
			if (stricmp(name, CrateNames[crate]) == 0) return(crate);
		}
	}
	return(CRATE_MONEY);
}


/***********************************************************************************************
 * Owner_From_Name -- Convert an owner name into a bitfield.                                   *
 *                                                                                             *
 *    This will take an owner specification and convert it into a bitfield that represents     *
 *    it. Sometimes this will be just a single house bit, but other times it could be          *
 *    all the allies or soviet house bits combined.                                            *
 *                                                                                             *
 * INPUT:   text  -- Pointer to the text to convert into a house bitfield.                     *
 *                                                                                             *
 * OUTPUT:  Returns with the houses specified. The value is in the form of a bit field with    *
 *          one bit per house type.                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int Owner_From_Name(char const * text)
{
	int ownable = 0;
	HousesType h = HouseTypeClass::From_Name(text);
	if (h != HOUSE_NONE) {
		ownable |= (1 << h);
	}
	return(ownable);
}


/***********************************************************************************************
 * Shake_The_Screen -- Dispatcher that shakes the screen.                                      *
 *                                                                                             *
 *    This routine will shake the game screen the number of shakes requested.                  *
 *                                                                                             *
 * INPUT:   shakes   -- The number of shakes to shake the screen.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/04/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void Shake_The_Screen(int shakes)
{
	//
}


/***********************************************************************************************
 * List_Copy -- Makes a copy of a cell offset list.                                            *
 *                                                                                             *
 *    This routine will make a copy of a cell offset list. It will only copy the significant   *
 *    elements of the list limited by the maximum length specified.                            *
 *                                                                                             *
 * INPUT:   source   -- Pointer to a cell offset list.                                         *
 *                                                                                             *
 *          len      -- The maximum number of cell offset elements to store in to the          *
 *                      destination list pointer.                                              *
 *                                                                                             *
 *          dest     -- Pointer to the destination list to store the copy into.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Ensure that the destination list is large enough to hold the list copy.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void List_Copy(Cell const * source, int len, Cell * dest)
{
	if (source == NULL || dest == NULL) {
		return;
	}

	while (len > 0) {
		*dest = *source;
		if (*dest == REFRESH_EOL) return;
		dest++;
		source++;
		len--;
	}

	/// Terminate the list.
	*(dest-1) = REFRESH_EOL;
}


/// <summary>
/// Converts an ASCII name into a VQType.
/// This routine is used when processing the movie names found in the scenario INI file.
/// </summary>
/// <param name="name">Pointer to the ASCII movie name to convert.</param>
/// <returns>Returns with the movie that matches the name. VQ_NONE is returned if there is
/// no match.</returns>
VQType VQ_From_Name(char const * name)
{
	if (name != NULL && strcmpi("<none>", name)) {
		for (int movie = 0; movie < Movies.Count(); movie++) {
			if (stricmp(name, Movies[movie]) == 0) return(VQType(movie));
		}
	}
	return(VQ_NONE);
}


/// <summary>
/// Converts an ASCII name into a LandType.
/// This routine is used when processing the terrain land type specified in an INI file.
/// </summary>
/// <param name="name">Pointer to the ASCII name to convert.</param>
/// <returns>Returns with the land type that matches the name. LAND_NONE is returned if
/// there is no match.</returns>
LandType Land_From_Name(char const * name)
{
	if (name != NULL && strcmpi("<none>", name)) {
		for (LandType land = LAND_FIRST; land < LAND_COUNT; land++) {
			if (stricmp(LandName[land], name) == 0) return(land);
		}
	}
	return(LAND_NONE);
}


/// <summary>
/// Converts a land type into its ASCII name.
/// This routine is used when writing a terrain land type back out to an INI file.
/// </summary>
/// <param name="land">The land type to fetch the name of.</param>
/// <returns>Returns with a pointer to the name of the land type. An illegal land type
/// fetches the "none" name instead.</returns>
char const * Name_From_Land(LandType land)
{
	if ((unsigned)land < LAND_COUNT) {
		return(LandName[land]);
	}
	return("<none>");
}


/// <summary>
/// Converts an ASCII name into a SpeedType.
/// This routine is used when processing the movement speed specified in an INI file.
/// </summary>
/// <param name="name">Pointer to the ASCII name to convert.</param>
/// <returns>Returns with the speed type that matches the name. SPEED_NONE is returned if
/// there is no match.</returns>
SpeedType Speed_From_Name(char const * name)
{
	if (name != NULL && strcmpi("<none>", name)) {
		for (SpeedType speed = SPEED_FIRST; speed < SPEED_COUNT; speed++) {
			if (stricmp(SpeedName[speed], name) == 0) return(speed);
		}
	}
	return(SPEED_NONE);
}


/// <summary>
/// Converts a speed type into its ASCII name.
/// This routine is used when writing a movement speed back out to an INI file.
/// </summary>
/// <param name="speed">The speed type to fetch the name of.</param>
/// <returns>Returns with a pointer to the name of the speed type. An illegal speed type
/// fetches the "none" name instead.</returns>
char const * Name_From_Speed(SpeedType speed)
{
	if ((unsigned)speed < SPEED_COUNT) {
		return(SpeedName[speed]);
	}
	return("<none>");
}


/// <summary>
/// Writes the multiplayer connection statistics out to a text file.
/// Use this routine to dump the round trip times, resends, stalls, and packet loss that
/// were recorded for each player. Only an internet game is reported upon.
/// </summary>
void Print_MP_Stats(void)
{
	if (Session.Type == GAME_INTERNET) {
		FILE *file = fopen("mpstats.txt", "wt");
		if (file != NULL) {
			fprintf(file, "Frames: %d\n", Frame);

			fprintf(file, "Average FPS: %d\n", SecondsPassed != 0 ? TotalFrames / SecondsPassed : 0);
			fprintf(file, "Max MaxAhead: %d\n", Session.MaxMaxAhead);
			fprintf(file, "Latency setting: %d\n", Session.LatencyFudge);
			fprintf(file, "Game speed setting: %d\n", Options.GameSpeed);

			if (PacketTransport != NULL) {
				for (int i = 0; i < PacketTransport->Get_Num_Local_Addresses(); i++) {
					unsigned char *addr = PacketTransport->Get_Local_Address(i);
					if (addr != NULL) {
						unsigned int a = *(unsigned int *)addr;
						fprintf(file, "Local address: %d.%d.%d.%d\n", (a) & 0xFF, (a >> 8) & 0xFF, (a >> 16) & 0xFF, (a >> 24) & 0xFF);
					}
				}
			}

			for (int i = 0; i < MAX_PLAYERS; i++) {

				MPStatsType &stats = Session.ConnectionStats[i];;
				if (stats.Name[0] == '\0') {
					continue;
				}

				fprintf(file, "\nName: %s\n", stats.Name);
				fprintf(file, "Address: %s\n", stats.Address.As_String());
				fprintf(file, "Max avg round trip: %d\n", stats.MaxAvgRoundTrip);
				fprintf(file, "Max round trip: %d\n", stats.MaxRoundTrip);
				fprintf(file, "Resends: %d\n", stats.Resends);
				fprintf(file, "Frame sync stalls: %d\n", stats.FrameSyncStalls);
				fprintf(file, "Command cound stalls: %d\n", stats.CommandCountStalls);
				fprintf(file, "Lost: %d\n", stats.Lost);
				fprintf(file, "Percent lost: %d\n", stats.PercentLost);
			}

			fclose(file);
		}
	}
}
