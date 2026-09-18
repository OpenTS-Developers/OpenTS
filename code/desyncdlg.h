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
 * The screen shown when a network game goes out of sync. The master chooses to load a saved
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

		bool Is_Active(void) const {return(ScreenActive);}

		bool Is_Host(void) const {return(IsHostDialog);}
		bool Has_Left(int house) const {return(State.Has_Left(house));}
		char const * Left_Name(int house) const {return(State.Left_Name(house));}
		std::vector<std::string> const & Chat_Backlog(void) const {return(ChatBacklog);}
		bool Is_Counting_Down(void) const {return(CountdownActive);}
		bool Quit_Is_Allowed(void) const {return(IsHostDialog || QuitEnabled);}
		bool Load_Is_Allowed(void) const;
		std::string Countdown_Caption(void) const;
		float Countdown_Left(void) const;
		char const * Countdown_Color(void) const;
		void Say(char const * text);

		bool Service_Screen(void);
		bool Decision_Is_Settled(void) const {return(ScreenSettled);}

		void Service(void);

		void Notify_Chat(char const * name, char const * text);
		void Notify_Player_Left(int house, char const * name);
		void Notify_Continue(void);
		void Notify_Heartbeat(int house);
		void Notify_Master_Changed(void);

	private:
		void Become_Host_If_Promoted(void);
		void Append_Chat_Line(char const * line);
		void Send_Heartbeat(void);
		void Send_Continue(void);
		void Check_Timeouts(void);
		void Start_Countdown(void);

		OutcomeType Run_Screen(void);

		bool IsHostDialog = false;
		bool ScreenActive = false;
		bool ScreenSettled = false;
		OutcomeType ScreenOutcome = OutcomeType::Continue;
		bool ContinueReceived = false;
		bool CountdownActive = false;
		bool QuitEnabled = false;
		std::int64_t OpenedAt = 0;
		int LastCountdownSecond = -1;
		DesyncClass State;
		std::vector<std::string> ChatBacklog;
};

extern DesyncDialogClass DesyncDialog;
