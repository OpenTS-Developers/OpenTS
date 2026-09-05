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

#include "always.h"

#include "mainloop.h"

#include "_bench.h"
#include "_command.h"
#include "_logic.h"
#include "_map.h"
#include "_palette.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_timer.h"
#include "_xmouse.h"
#include "bench.h"
#include "chat.h"
#include "command.h"
#include "conquer.h"
#include "data.h"
#include "debug.h"
#include "dialog.h"
#include "dsaudio.h"
#include "dsurface.h"
#include "fog.h"
#include "globals.h"
#include "goptions.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "logic.h"
#include "misc.h"
#include "mpscore.h"
#include "msgbox.h"
#include "msgloop.h"
#include "mstimer.h"
#include "netdlg.h"
#include "pcx.h"
#include "queue.h"
#include "rules.h"
#include "savemgr.h"
#include "scenario.h"
#include "scheme.h"
#include "session.h"
#include "stats.h"
#include "stimer.h"
#include "surface.h"
#include "tactical.h"
#include "theme.h"
#include "timer.h"
#include "tracker.h"

#include "bench.hh"
#include "special.hh"

#include <algorithm>

//
// Special module globals for recording and playback
//
int TeamEvent = 0;			// 0 = no event, 1,2,3 = team event type
int TeamNumber = 0;			// which team was selected? (1-9)

void Message_Input(KeyNumType &input);
void Sync_Delay(void);
void Multiplayer_Debug_Print(bool noframecheck);
static void Do_Record_Playback(void);


/// <summary>
/// Captures the visible screen to a movie sequence.
/// This routine grabs the client area of the game window into an off screen surface
/// each time it is called, and when the sequence is full it writes the whole thing out
/// as numbered PCX files. It is a debugging aid and does nothing unless motion capture
/// has been switched on.
/// </summary>
/// <remarks>Call this routine once per frame -- one call captures one frame of the
/// sequence.</remarks>
void Motion_Capture(void)
{
	if (Debug_MotionCapture) {
		static Surface ** _array = NULL;
		static int _sequence = 0;
		static int _seqsize = Rule->MovieTime * TICKS_PER_MINUTE;

		if (_array == NULL) {
			_array = new Surface * [_seqsize];
			memset(_array, '\0', _seqsize * sizeof(Surface*));
		}

		if (_array == NULL) {
			Debug_MotionCapture = false;
		}

		Rect rect = VisibleSurface->Get_Rect();

		/// Compensate for screen shake
		if (Map.ScreenX != 0 || Map.ScreenY != 0) {
			if (Map.ScreenX > 0) {
				rect.X += Map.ScreenX;
				rect.Width -= Map.ScreenX;
			} else if (Map.ScreenX < 0) {
				rect.Width += Map.ScreenX;
			}
			if (Map.ScreenY > 0) {
				rect.Y += Map.ScreenY;
				rect.Height -= Map.ScreenY;
			} else if (Map.ScreenY < 0) {
				rect.Height += Map.ScreenY;
			}
		}

		if (_sequence < _seqsize) {
			if (_array[_sequence] == NULL) {
				_array[_sequence] = new BSurface(rect.Width, rect.Height, 2);
			}

			if (_array[_sequence] != NULL) {
				_array[_sequence]->Blit_From(Rect(0, 0, rect.Width, rect.Height), *VisibleSurface, rect);
			}
			_sequence++;

		} else {
			Debug_MotionCapture = false;

			DSurface temp_page(rect.Width, rect.Height);

			for (int index = 0; index < _sequence; index++) {
				char filename[30];
				sprintf(filename, "cap%04d.pcx", index);
				CCFileClass file(filename);
				temp_page.Blit_From(*_array[index], false, true);
				Write_PCX_File(file, temp_page, & GamePalette);
			}

			_sequence = 0;
		}
	}
}


/// <summary>
/// Handles the game losing the input focus.
/// This routine parks the main loop while another application holds the focus, pumping
/// the Windows message queue so that the game can be restored. A network game cannot
/// afford to stall, so it pumps the queue once and lets play carry on regardless.
/// </summary>
static void Check_For_Focus_Loss(void)
{
	while (!GameInFocus) {
		if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
			Sleep(500);
			Windows_Message_Handler();
		} else {
			Sleep(10);
			Windows_Message_Handler();
			break;
		}
	}
	return;
}

bool InMainLoop = false;

/***********************************************************************************************
 * Main_Loop -- This is the main game loop (as a single loop).                                 *
 *                                                                                             *
 *    This function will perform one game loop.                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Should the game end?                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Main_Loop(void)
{
	KeyNumType	input;					// Player input.
	int x;
	int y;
	int framedelay;

	//Mono_Set_Cursor(0,0);

	if (!GameActive) {return(!GameActive);}

	InMainLoop = true;

	/*
	**	Call the focus loss handler
	*/
	#if 0
	Check_For_Focus_Loss();
	#else
	while (!GameInFocus) {
		if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
			Sleep(500);
			Windows_Message_Handler();
		} else {
			Sleep(10);
			Windows_Message_Handler();
			break;
		}
	}
	#endif

	/*
	**	Sync-bug trapping code
	*/
	if (Frame >= Session.TrapFrame) {
		Session.Trap_Object();
	}

	//
	// Initialize our AI processing timer
	//
	Session.ProcessTimer = timeGetTime();/// TickCount;

	if (Session.TrapCheckHeap) {
		Debug_Trap_Check_Heap = true;
	}

#ifdef _DEBUG

	/*
	**	Update the running status debug display.
	*/
	Self_Regulate();
#endif

	BStart(BENCH_GAME_FRAME);

	/*
	**	If there is no theme playing, but it looks like one is required, then start one
	**	playing. This is usually the symptom of there being no transition score.
	*/
	if (Audio_Available() && Theme.What_Is_Playing() == THEME_NONE) {
		Theme.Queue_Song(THEME_PICK_ANOTHER);
	}

	/*
	**	Setup the timer so that the Main_Loop function processes at the correct rate.
	*/
	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH &&
		Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {

		//
		// In playback mode, run as fast as possible.
		//
		if (Session.Play) {
			FrameTimer = 0;
		} else {
			framedelay = TIMER_SECOND / Session.DesiredFrameRate;
			FrameTimer = framedelay;
			framedelay = 1000 / Session.DesiredFrameRate;
			NetFrameTimer = framedelay;

			int maxahead = Session.MaxAhead;
			int worst_latency = 0;
			if (Session.Type == GAME_INTERNET) {
				for (int i = 0; i < Ipx.Num_Connections(); i++) {
					if (worst_latency <= Session.PlayerLatency[i]) {
						worst_latency = Session.PlayerLatency[i];
					}
				}

				if (worst_latency) {
					if (worst_latency >= maxahead / 4) {
						NetFrameTimer = NetFrameTimer + 10;
					}
					if (worst_latency >= maxahead / 2) {
						NetFrameTimer = NetFrameTimer + 10;
					}
					if (worst_latency >= (3 * maxahead) / 4) {
						NetFrameTimer = NetFrameTimer + 10;
					}
				}
			}
		}
	} else {
		FrameTimer = Options.GameSpeed;
	}

	/*
	**	Update the display, unless we're inside a dialog.
	*/
	if (!Session.Play) {
		if (SpecialDialog == SDLG_NONE && GameInFocus) {
			Map.Input(input, x, y);
			if (input) {
				Keyboard_Process(input);
			}
			if (Session.ShowInternetDebug) {
				Multiplayer_Debug_Print(false);
			}
			if ((Frame & 7) == 7 && Session.Type == GAME_INTERNET) {
				Ipx.Store_Stats();
			}
			Update_Fogged_Objects();
			Map.Render();
		}
	}

	drag_select_aborted = false;

	/*
	**	Save map's position & selected objects, if we're recording the game.
	*/
	if (Session.Record || Session.Play) {
		Do_Record_Playback();
	}

	/*
	**	Sort the map's ground layer by y-coordinate value.  This is done
	**	outside the IsToRedraw check, for the purposes of game sync'ing
	**	between machines; this way, all machines will sort the Map's
	**	layer in the same way, and any processing done that's based on
	**	the order of this layer will remain in sync.
	*/
	DisplayClass::Layer[LAYER_GROUND].Sort();

	/*
	**	AI logic operations are performed here.
	*/
	Logic.AI();

	/*
	**	Manage the inter-player message list.  If Manage() returns true, it means
	**	a message has expired & been removed, and the entire map must be updated.
	*/
	Session.Messages.Manage();

	//
	// Measure how long it took to process the AI
	//
	Session.ProcessTicks += std::min<int>(1000, (timeGetTime() - Session.ProcessTimer)); // (TickCount - Session.ProcessTimer)
	Session.ProcessFrames++;

	/*
	**	Process all commands that are ready to be processed.
	*/
	Queue_AI();

	Call_Back();

	bool done = false;
	if (PlayerWins || PlayerLoses || PlayerRestarts || PlayerAborts) {
		Unlock_Scenario_Input();

		/*
		**	Check for player wins or loses according to global event flag.
		*/
		if (PlayerWins) {
			if (Session.Type == GAME_INTERNET && !GameStatisticsPacketSent) {
				if (WestwoodOnline_Tournament) {
					Session.SawGameCompletion = true;
				}
				Register_Game_End_Time();
				Send_Statistics_Packet();		// Player just won.
			}
			PlayerLoses = false;
			PlayerWins = false;
			PlayerRestarts = false;
			PlayerAborts = false;
			Do_Win();
			done = true;
		} else if (PlayerLoses) {
			if (Session.Type == GAME_INTERNET && !GameStatisticsPacketSent) {
				if (WestwoodOnline_Tournament) {
					Session.SawGameCompletion = true;
				}
				Register_Game_End_Time();
				Send_Statistics_Packet();		// Player just lost.
			}
			PlayerWins = false;
			PlayerLoses = false;
			PlayerRestarts = false;
			PlayerAborts = false;
			Do_Lose();
			done = true;
		}
	}

	if (!done) {
		if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH && TournamentTime > 0 && TournamentTimer == 0) {
			if (Single_Score_Presentation(PlayerPtr)) {
				PlayerPtr->Flag_To_Win();
			} else {
				PlayerPtr->Flag_To_Lose();
			}
		}

		if (PlayerRestarts) {
			PlayerWins = false;
			PlayerLoses = false;
			PlayerRestarts = false;
			PlayerAborts = false;
			Do_Restart();
			done = true;
		} else if (PlayerAborts) {
			PlayerWins = false;
			PlayerLoses = false;
			PlayerRestarts = false;
			PlayerAborts = false;
			Do_Abort();
			done = true;
		}
	}

	if (!done) {
		/*
		**	The frame logic has been completed. Increment the frame
		**	counter.
		*/
		Frame++;

#ifdef _DEBUG
		/*
		**	Is there a memory trasher altering the map??
		*/
		if (Debug_Check_Map) {
			if (!Map.Validate()) {
				if (WWMessageBox().Process (TXT_MAP_ERROR, TXT_STOP, TXT_CONTINUE)==0) {
					GameActive = 0;
				}
				Map.Validate();		// give debugger a chance to catch it
			}
		}
#endif

		Sync_Delay();
		Process_Deferred_Deletion();
		SaveManager.Service();
	}

	BEnd(BENCH_GAME_FRAME);

	InMainLoop = false;

	return(!GameActive);
}


void Ingame_Menu_Dialog(void);

/***********************************************************************************************
 * Keyboard_Process -- Processes the tactical map input codes.                                 *
 *                                                                                             *
 *    This routine is used to process the input codes while the player                         *
 *    has the tactical map displayed. It handles all the keys that                             *
 *    are appropriate to that mode.                                                            *
 *                                                                                             *
 * INPUT:   input -- Input code as returned from Input_Num().                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/21/1992 JLB : Created.                                                                 *
 *   07/04/1995 JLB : Handles team and map control hotkeys.                                    *
 *=============================================================================================*/
void Keyboard_Process(KeyNumType & input)
{
	/*
	**	Don't do anything if there is not keyboard event.
	*/
	if (input == KN_NONE) {
		return;
	}
	/*
	**	For network, process user input for inter-player messages.
	*/
	Message_Input(input);

	/*
	**	The VK_BIT must be stripped from the "plain" value of the key so that a comparison to
	**	KN_1, for example, will yield TRUE if in fact the "1" key was pressed.
	*/
	KeyNumType plain = KeyNumType(input & ~(WWKEY_SHIFT_BIT|WWKEY_ALT_BIT|WWKEY_CTRL_BIT|WWKEY_VK_BIT));
	KeyNumType key = KeyNumType(input /*& ~WWKEY_VK_BIT*/);

	const CommandClass * cmd = HotkeyCommands[key];

	if (cmd != NULL) {

		cmd->Execute();

	} else {

		/*
		**	Brings up the options dialog box.
		*/
		if (plain == KN_SPACE || plain == KN_ESC) {
			Queue_Options();
		}

		/*
		**	Toggles the map zoom mode similarly to pressing the map button.
		*/
		if (plain == KN_TAB) {
			Map.Zoom_Mode_Control();
		}

#ifdef _DEBUG
		if (Debug_Flag) {
			switch (int(input)) {
				case int(int(KN_M) | int(KN_SHIFT_BIT)):
				case int(int(KN_M) | int(KN_ALT_BIT)):
				case int(int(KN_M) | int(KN_CTRL_BIT)):
					for (int h = 0; h < Houses.Count(); h++) {
						Houses[h]->Refund_Money(10000);
					}
					break;

				default:
					break;
			}
		}

		if (Debug_Playtest && input == (KN_W|KN_ALT_BIT)) {
			PlayerPtr->Blockage = false;
			PlayerPtr->Flag_To_Win();
		}

		if ((Debug_Flag || Debug_Playtest) && plain == KN_F4) {
			if (Session.Type == GAME_NORMAL) {
				Debug_Unshroud = (Debug_Unshroud == false);
				Map.Flag_To_Redraw(GS_REDRAW_ALL);
			}
		}

		if (Debug_Flag && input == KN_SLASH) {
			if (Session.Type != GAME_NORMAL) {
				SpecialDialog = SDLG_SPECIAL;
				input = KN_NONE;
			} else {
				Ingame_Menu_Dialog();
			}
		}

		if (input != 0 && Debug_Flag && input && (input & KN_RLSE_BIT) == 0) {
			Debug_Key(input);
		}
#endif

	}
}


/***********************************************************************************************
 * Sync_Delay -- Forces the game into a 15 FPS rate.                                           *
 *                                                                                             *
 *    This routine will wait until the timer for the current frame has expired before          *
 *    returning. It is called at the end of every game loop in order to force the game loop    *
 *    to run at a fixed rate.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine will delay an amount of time according to the game speed setting.  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1995 JLB : Created.                                                                 *
 *   03/06/1995 JLB : Fixed.                                                                   *
 *=============================================================================================*/
void Sync_Delay(void)
{
	/*
	**	Accumulate the number of 'spare' ticks that are frittered away here.
	*/
	SpareTicks += FrameTimer;

	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
		while (NetFrameTimer) {
			Call_Back();
			if (SpecialDialog == SDLG_NONE && GameInFocus == true) {
				KeyNumType input = KN_NONE;
				int x, y;
				if (NetFrameTimer > 10) {
					Map.Input(input, x, y);
					Keyboard_Process(input);
					TacticalMap->AI();
					Map.Render();
				} else {
					Sleep(0);
				}
				if (!NetFrameTimer()) {
					break;
				}
			}
			Sleep(0);
		}
	} else {
		while (FrameTimer) {
			Call_Back();
			if (SpecialDialog == SDLG_NONE && GameInFocus == true) {
				KeyNumType input = KN_NONE;
				int x, y;
				Map.Input(input, x, y);
				Keyboard_Process(input);
				TacticalMap->AI();
				Map.Render();
				if (!FrameTimer) {
					break;
				}
			}
			if (GameInFocus || (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH)) {
				Sleep(0);
			} else {
				Sleep(16 * FrameTimer);
			}
		}
	}

	static CDTimerClass<SystemTimerClass> fps_timer;
	if (!fps_timer) {
		LastFramesPerSecond = FramesThisSecond;
		FramesThisSecond = 0;
		TotalFrames += LastFramesPerSecond;
		SecondsPassed++;
		if (TotalFrames > 0x7FFFFFFF) {
			TotalFrames = 0;
			SecondsPassed = 0;
		}
		fps_timer = TIMER_SECOND;
	}
}


/***********************************************************************************************
 * Message_Input -- allows inter-player message input processing                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      input      key value                                                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
//#pragma off (unreferenced)
void Message_Input(KeyNumType &input)
{
	int rc;
	KeyNumType copy_input;

	/*
	**	Check keyboard input for a request to send a message.
	**	The 'to' argument for Add_Edit is prefixed to the message buffer; the
	**	message buffer is big enough for the 'to' field plus MAX_MESSAGE_LENGTH.
	**	To send the message, calling Get_Edit_Buf retrieves the buffer minus the
	**	'to' portion.  At the other end, the buffer allocated to display the
	**	message must be MAX_MESSAGE_LENGTH plus the size of "From: xxx (house)".
	*/
	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH && input >= KN_F1 && input < (KN_F1 + Session.MaxPlayers) && !Session.Messages.Is_Edit()) {
		if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
			/*
			**	For a network game:
			**	F1-F7 = "To <name> (house):" (only allowed if we're not in ObiWan mode)
			**	F8 = "To All:"
			*/
			if (input==(KN_F1 + Session.MaxPlayers - 1)) {
				Chat_Begin(ChatScopeType::Everyone);
			} else if ((input - KN_F1) < Ipx.Num_Connections()) {
				Chat_Begin(ChatScopeType::Player, Ipx.Connection_ID(input - KN_F1));
			}
		}
	}

	/*
	**	Process message-system input; send the message out if RETURN is hit.
	*/
	copy_input = input;
	rc = Session.Messages.Input(input);

	/*
	**	If a single character has been added to an edit buffer, update the display.
	*/
	if (rc == 1 && Session.Type != GAME_NORMAL) {
		Map.Flag_To_Redraw();
	}

	/*
	**	If backspace was hit, redraw the map.  If the edit message was removed,
	**	the map must be force-drawn, since it won't be able to compute the
	**	cells to redraw; otherwise, let the map compute the cells to redraw,
	**	by not force-drawing it, but just setting the IsToRedraw bit.
	*/
	if (rc==2 && Session.Type != GAME_NORMAL) {
		if (copy_input==KN_ESC) {
			Map.Flag_To_Redraw(GS_REDRAW_ALL);
		} else {
			Map.Flag_To_Redraw();
		}
	}

	/*
	**	Send a message
	*/
	if ((rc==3 || rc==4) && Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
		if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
			if (rc==3) {
				Chat_Send(Session.Messages.Get_Edit_Buf());
			} else {
				Chat_Send(Session.Messages.Get_Overflow_Buf());
				Session.Messages.Clear_Overflow_Buf();
			}
		}

		/*
		**	Tell the map to completely update itself, since a message is now missing.
		*/
		Map.Flag_To_Redraw(GS_REDRAW_ALL);
	}
}
//#pragma on (unreferenced)


/// <summary>
/// Draws the network statistics overlay.
/// This routine prints the frame counter, frame rate, latency and processing figures
/// across the bottom of the screen, and then lets the connection manager add its own
/// per-connection display. It is used while debugging a multiplayer game and does
/// nothing at all in a single player game.
/// </summary>
/// <param name="noframecheck">Should the display be drawn regardless of the frame
/// counter?</param>
void Multiplayer_Debug_Print(bool noframecheck)
{
	if (!noframecheck && (Frame & 7) != 7) {
		return;
	}

	if (Session.Type == GAME_NORMAL) {
		return;
	}

	Hide_Mouse();

	VisibleSurface->Fill_Rect(Rect(0, 400, 639, 80), 0);

	char buffer[256];

	sprintf(buffer, "Frame : %d", Frame);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 402), Fetch_Scheme_By_Name("Grey"), 0, (TextPrintType)(TPF_EFNT | TPF_NOSHADOW));

	sprintf(buffer, "FPS : %d", LastFramesPerSecond);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 410), Fetch_Scheme_By_Name("Grey"), 0, (TextPrintType)(TPF_EFNT | TPF_NOSHADOW));

	sprintf(buffer, "MaxAhead : %d", Session.MaxAhead);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 418), Fetch_Scheme_By_Name("Grey"), 0, (TextPrintType)(TPF_EFNT | TPF_NOSHADOW));

	sprintf(buffer, "Resp Time : %d ms", (int)(Ipx.Response_Time() * 1000) / TIMER_SECOND);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 426), Fetch_Scheme_By_Name("Grey"), 0, (TextPrintType)(TPF_EFNT | TPF_NOSHADOW));

	sprintf(buffer, "Req fps : %d", Session.DesiredFrameRate);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 434), Fetch_Scheme_By_Name("Grey"), 0, (TextPrintType)(TPF_EFNT | TPF_NOSHADOW));

	sprintf(buffer, "Process : %d", Session.Players[0]->Player.ProcessTime);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 442), Fetch_Scheme_By_Name("Grey"), 0, (TextPrintType)(TPF_EFNT | TPF_NOSHADOW));

	Ipx.Multiplayer_Debug_Print();

	Show_Mouse();
}


/***********************************************************************************************
 * Do_Record_Playback -- handles saving/loading map pos & current object                       *
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
 *   08/15/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static void Do_Record_Playback(void)
{
	int count;
	int tgt;
	int i;
	Point2D coord;
	ObjectClass * obj;
	unsigned int sum;
	unsigned int sum2;
	unsigned int ltgt;

	/*
	**	Record a game
	*/
	if (Session.Record) {

		/*
		**	Save the map's location
		*/
		coord = TacticalMap->Get_Tactical_Position();
		Session.RecordFile.Write(&coord, sizeof(coord));

		/*
		**	Save the current object list count
		*/
		count = CurrentObject.Count();
		Session.RecordFile.Write(&count, sizeof(count));

		/*
		**	Save a CRC of the selected-object list.
		*/
		sum = 0;
		for (i = 0; i < count; i++) {
			ltgt = TargetClass(CurrentObject[i]).Encode();
			sum += ltgt;
		}
		Session.RecordFile.Write (&sum, sizeof(sum));

		/*
		**	Save all selected objects.
		*/
		for (i = 0; i < count; i++) {
			tgt = TargetClass(CurrentObject[i]).Encode();
			Session.RecordFile.Write (&tgt, sizeof(tgt));
		}

		//
		// Save team-selection and formation events
		//
		Session.RecordFile.Write (&TeamEvent, sizeof(TeamEvent));
		Session.RecordFile.Write (&TeamNumber, sizeof(TeamNumber));
		TeamEvent = 0;
		TeamNumber = 0;
	}

	/*
	**	Play back a game ("attract" mode)
	*/
	if (Session.Play) {

		/*
		**	Read & set the map's location.
		*/
		if (Session.RecordFile.Read(&coord, sizeof(coord))==sizeof(coord)) {
			TacticalMap->Set_Tactical_Position(coord);
		}

		if (Session.RecordFile.Read(&count, sizeof(count))==sizeof(count)) {
			/*
			**	Compute a CRC of the current object-selection list.
			*/
			sum = 0;
			for (i = 0; i < CurrentObject.Count(); i++) {
				ltgt = TargetClass(CurrentObject[i]).Encode();
				sum += ltgt;
			}

			/*
			**	Load the CRC of the objects on disk; if it doesn't match, select
			**	all objects as they're loaded.
			*/
			Session.RecordFile.Read (&sum2, sizeof(sum2));
			if (sum2 != sum) {
				Unselect_All();
			}

			AllowVoice = true;

			for (i = 0; i < count; i++) {
				if (Session.RecordFile.Read (&tgt, sizeof(tgt))==sizeof(tgt)) {
					TargetClass tmp;
					tmp.Decode(tgt);
					obj = tmp.As_Object();
					if (obj && (sum2 != sum)) {
						obj->Select();
						AllowVoice = false;
					}
				}
			}

			AllowVoice = true;

		}

		//
		// Save team-selection and formation events
		//
		Session.RecordFile.Read (&TeamEvent, sizeof(TeamEvent));
		Session.RecordFile.Read (&TeamNumber, sizeof(TeamNumber));

		/*
		**	The map isn't drawn in playback mode, so draw it here.
		*/
		Map.Flag_To_Redraw();
		Map.Render();
	}
}
