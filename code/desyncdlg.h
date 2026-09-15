/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "desync.h"
#include "win.h"

#include <cstdint>
#include <string>
#include <vector>

/*
 * The dialog shown when a network game goes out of sync. The master chooses to load a saved
 * game, to continue without the players out of sync, or to quit; everyone else waits. Both
 * variants list the players with their state and carry a chat box. Game logic is halted while
 * it is up, and the network is kept alive with heartbeats.
 */
class DesyncDialogClass
{
	public:
		enum class OutcomeType {
			Continue,
			Load,
			Quit,
		};

		// Blocks until a decision has been made; the network is serviced throughout.
		OutcomeType Run(void);

		bool Is_Active(void) const {return(Window != NULL || ScreenActive);}

		// What the out-of-sync screen reads and does while it stands in for the dialog. The
		// dialog's own loop reads the same state directly.
		bool Is_Host(void) const {return(IsHostDialog);}
		bool Has_Left(int house) const {return(State.Has_Left(house));}
		char const * Left_Name(int house) const {return(State.Left_Name(house));}
		std::vector<std::string> const & Chat_Backlog(void) const {return(ChatBacklog);}
		bool Is_Counting_Down(void) const {return(CountdownActive);}
		bool Quit_Is_Allowed(void) const {return(IsHostDialog || QuitEnabled);}
		bool Load_Is_Allowed(void) const;
		std::string Countdown_Caption(void) const;
		float Countdown_Left(void) const;
		char const * Countdown_Colour(void) const;
		void Say(char const * text);

		// One pass of the loop the dialog runs in, for a screen's runner to call instead.
		// True once the outcome has settled without this player choosing it.
		bool Service_Screen(void);
		bool Decision_Is_Settled(void) const {return(ScreenSettled);}

		// Sends the heartbeat and drops silent players; called from the network maintenance
		// so that both outlive a nested dialog's message loop.
		void Service(void);

		// Every notification is a no-op while the dialog is not open.
		void Notify_Chat(char const * name, char const * text);
		void Notify_Player_Left(int house, char const * name);
		void Notify_Continue(void);
		void Notify_Heartbeat(int house);
		void Notify_Master_Changed(void);

	private:
		void Create_Dialog(void);
		void Destroy_Dialog(void);
		void Fit_To_Screen(void);
		void Become_Host_If_Promoted(void);
		void Update_Player_List(void);
		void Refill_Chat_List(void);
		void Append_Chat_Line(char const * line);
		void Send_Chat(void);
		void On_Chat_Edit_Focus(bool gained);
		void Send_Heartbeat(void);
		void Send_Continue(void);
		void Check_Timeouts(void);
		void Start_Countdown(void);
		void Update_Countdown_Text(void);
		void Draw_Countdown_Bar(HWND window);
		static INT_PTR CALLBACK Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

		bool Run_Screen(OutcomeType & outcome);

		HWND Window = NULL;
		bool IsHostDialog = false;
		bool ScreenActive = false;
		bool ScreenSettled = false;
		OutcomeType ScreenOutcome = OutcomeType::Continue;
		int Decision = 0;
		bool ContinueReceived = false;
		bool ChatPlaceholderActive = false;
		bool CountdownActive = false;
		bool QuitEnabled = false;
		std::int64_t OpenedAt = 0;
		int LastCountdownSecond = -1;
		DesyncClass State;
		std::vector<std::string> ChatBacklog;
};

extern DesyncDialogClass DesyncDialog;
