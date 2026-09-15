/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "desyncdlg.h"

#include "_map.h"
#include "_rect.h"
#include "_surface.h"
#include "_xmouse.h"
#include "chat.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "globals.h"
#include "house.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "loaddlg.h"
#include "misc.h"
#include "mpload.h"
#include "ui/screens/desync/uidesync.h"
#include "netdlg.h"
#include "netglobal.h"
#include "savemgr.h"
#include "session.h"
#include "srfcache.h"
#include "syncreport.h"
#include "win.h"
#include "winfix.h"

#include <windowsx.h>

#include <algorithm>
#include <cstdio>
#include <cstring>


namespace {

	constexpr int CHAT_BACKLOG_MAX = 50;

}	// namespace


/// <summary>
/// Shows the screen and runs it until the master has decided, or this player has quit. Game
/// logic is halted for the duration; chat, sign-offs, heartbeats and the master's decision
/// still come through, since the network is serviced the whole time.
/// </summary>
DesyncDialogClass::OutcomeType DesyncDialogClass::Run(void)
{
	DebugString("Out-of-sync dialog opening on frame %d\n", Frame);

	// A raised suspension makes the screen's own pass service the network instead of the game.
	TacticalActive = false;
	Session.Suspended++;

	ContinueReceived = false;
	CountdownActive = false;
	QuitEnabled = false;
	LastCountdownSecond = -1;
	ChatBacklog.clear();
	OpenedAt = Monotonic_Milliseconds();
	State.Begin(OpenedAt);

	OutcomeType outcome = Run_Screen();

	Session.Suspended--;
	TacticalActive = true;
	Map.Flag_To_Redraw(GS_REDRAW_ALL);

	DebugString("Out-of-sync dialog closed with outcome %d\n", (int)outcome);
	return(outcome);
}


/// <summary>
/// Whether the master may load a save to bring the session back into step.
/// </summary>
bool DesyncDialogClass::Load_Is_Allowed(void) const
{
	return(SaveManager.Multiplayer_Load_Is_Allowed() && MultiplayerLoadOptionsClass().Files_Present());
}


/// <summary>
/// The line under the players naming how long is left before the save loads, or nothing
/// while no load is pending.
/// </summary>
std::string DesyncDialogClass::Countdown_Caption(void) const
{
	if (!CountdownActive || !SaveManager.MultiplayerLoad.Is_Pending()) {
		return(std::string());
	}

	int const seconds = SaveManager.MultiplayerLoad.Seconds_Left(Monotonic_Milliseconds());
	char buffer[128];
	std::snprintf(buffer, sizeof(buffer),
		Fetch_String(seconds == 1 ? TXT_LOADING_IN_SECOND : TXT_LOADING_IN_SECONDS), seconds);
	return(std::string(buffer));
}


/// <summary>
/// How much of the countdown bar is still filled, from one down to nothing.
/// </summary>
float DesyncDialogClass::Countdown_Left(void) const
{
	if (!CountdownActive || !SaveManager.MultiplayerLoad.Is_Pending()) {
		return(0.0f);
	}

	int const total = (int)MultiplayerLoadClass::COUNTDOWN_MS;
	int const remaining = std::clamp((int)SaveManager.MultiplayerLoad.Milliseconds_Left(Monotonic_Milliseconds()), 0, total);
	return((total > 0) ? (float)remaining / (float)total : 0.0f);
}


/// <summary>
/// The color the bar is filled in, which turns as the wait runs out.
/// </summary>
char const * DesyncDialogClass::Countdown_Color(void) const
{
	int const total = (int)MultiplayerLoadClass::COUNTDOWN_MS;
	int const remaining = std::clamp((int)SaveManager.MultiplayerLoad.Milliseconds_Left(Monotonic_Milliseconds()), 0, total);
	int const elapsed = total - remaining;

	if (elapsed > total * 4 / 5) {
		return("#c80000");
	}
	if (elapsed > total * 2 / 5) {
		return("#c8c800");
	}
	return("#00c800");
}


/// <summary>
/// Sends a line of chat to everyone still in the session.
/// </summary>
void DesyncDialogClass::Say(char const * text)
{
	if (text == NULL || text[0] == 0) {
		return;
	}

	Session.MessageScope = ChatScopeType::Everyone;
	Session.MessageAddress = IPXAddressClass();
	Chat_Send(text);
}


/// <summary>
/// Runs the pass the dialog's own loop runs, for a screen standing in its place.
/// </summary>
/// <returns>True once the session has settled the outcome without this player choosing
/// it, which is what closes the screen.</returns>
bool DesyncDialogClass::Service_Screen(void)
{
	if (ScreenSettled) {
		return(true);
	}

	std::int64_t const now = Monotonic_Milliseconds();
	if (!IsHostDialog && !QuitEnabled && now - OpenedAt >= DesyncClass::QUIT_DELAY_MS) {
		QuitEnabled = true;
	}

	if (!CountdownActive && SaveManager.MultiplayerLoad.Is_Pending()) {
		Start_Countdown();
	}

	if (CountdownActive) {
		if (SaveManager.MultiplayerLoad.Is_Due(now)) {
			ScreenOutcome = OutcomeType::Load;
			ScreenSettled = true;
		}
	} else if (ContinueReceived) {
		ScreenOutcome = OutcomeType::Continue;
		ScreenSettled = true;
	}

	return(ScreenSettled);
}


/// <summary>
/// Runs the screen, reopening it around anything it raises.
/// </summary>
DesyncDialogClass::OutcomeType DesyncDialogClass::Run_Screen(void)
{
	IsHostDialog = Session.Am_I_Master();
	ScreenActive = true;
	ScreenSettled = false;
	ScreenOutcome = OutcomeType::Continue;

	while (!ScreenSettled) {
		UIDesyncChoiceType const choice = UI_Desync_Dialog();

		if (choice == UI_DESYNC_NONE) {
			// Either the session settled the decision under the screen, or it could not open;
			// both leave the match to carry on, as a dialog that would not create itself did.
			break;

		} else if (choice == UI_DESYNC_QUIT) {
			ScreenOutcome = OutcomeType::Quit;
			ScreenSettled = true;

		} else if (choice == UI_DESYNC_CONTINUE) {
			Send_Continue();
			ScreenOutcome = OutcomeType::Continue;
			ScreenSettled = true;

		} else if (choice == UI_DESYNC_LOAD) {
			SaveManager.Multiplayer_Load_Prompt();
		}
	}

	ScreenActive = false;
	return(ScreenOutcome);
}


void DesyncDialogClass::Service(void)
{
	if (!Is_Active()) {
		return;
	}

	std::int64_t now = Monotonic_Milliseconds();
	if (State.Heartbeat_Is_Due(now)) {
		Send_Heartbeat();
		State.Heartbeat_Sent(now);
	}
	Check_Timeouts();
}


void DesyncDialogClass::Notify_Chat(char const * name, char const * text)
{
	if (!Is_Active()) {
		return;
	}

	char buffer[MAX_MESSAGE_LENGTH + MAX_MESSAGE_PREFIX];
	std::snprintf(buffer, sizeof(buffer), "%s: %s", name, text);
	Append_Chat_Line(buffer);
}


void DesyncDialogClass::Notify_Player_Left(int house, char const * name)
{
	if (!Is_Active()) {
		return;
	}

	State.Mark_Left(house, name);

	if (name != NULL && name[0] != '\0') {
		char buffer[128];
		std::snprintf(buffer, sizeof(buffer), Fetch_String(TXT_LEFT_GAME), name);
		Append_Chat_Line(buffer);
	}

	Become_Host_If_Promoted();
}


void DesyncDialogClass::Notify_Continue(void)
{
	if (!Is_Active()) {
		return;
	}

	DebugString("The master chose to continue without the players out of sync\n");
	ContinueReceived = true;
}


void DesyncDialogClass::Notify_Heartbeat(int house)
{
	if (Is_Active()) {
		State.Heard(house, Monotonic_Milliseconds());
	}
}


void DesyncDialogClass::Notify_Master_Changed(void)
{
	if (!Is_Active()) {
		return;
	}

	Become_Host_If_Promoted();
}


/// <summary>
/// Replaces the wait dialog with the decision dialog once this machine has become master,
/// unless a load is already counting down, when there is nothing left to decide.
/// </summary>
void DesyncDialogClass::Become_Host_If_Promoted(void)
{
	if (!Is_Active() || IsHostDialog || CountdownActive || !Session.Am_I_Master()) {
		return;
	}

	DebugString("This machine is the new master; switching to the decision screen\n");
	IsHostDialog = true;
}


void DesyncDialogClass::Append_Chat_Line(char const * line)
{
	ChatBacklog.emplace_back(line);
	if (ChatBacklog.size() > CHAT_BACKLOG_MAX) {
		ChatBacklog.erase(ChatBacklog.begin());
	}
}


void DesyncDialogClass::Send_Heartbeat(void)
{
	if (PlayerPtr == NULL || Session.Players.Count() == 0) {
		return;
	}

	GlobalPacketType packet;
	NetGlobal::Initialize_Packet(packet, NET_DESYNC_HEARTBEAT);
	std::snprintf(packet.Name, sizeof(packet.Name), "%s", Session.Players[0]->Name);

	for (int index = 1; index < Session.Players.Count(); index++) {
		Ipx.Send_Global_Message(&packet, sizeof(packet), 0, &Session.Players[index]->Address);
	}
	Ipx.Service();
}


void DesyncDialogClass::Send_Continue(void)
{
	DebugString("Telling every seat to continue without the players out of sync\n");

	GlobalPacketType packet;
	NetGlobal::Initialize_Packet(packet, NET_DESYNC_CONTINUE);
	std::snprintf(packet.Name, sizeof(packet.Name), "%s", Session.Players[0]->Name);

	for (int index = 1; index < Session.Players.Count(); index++) {
		Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.Players[index]->Address);
		Ipx.Service();
	}
}


/// <summary>
/// Drops the seats that have fallen silent, so a machine that died without a sign-off neither
/// holds up the decision nor lingers in the seats a later load reconciles.
/// </summary>
void DesyncDialogClass::Check_Timeouts(void)
{
	std::int64_t const now = Monotonic_Milliseconds();

	for (int index = Session.Players.Count() - 1; index >= 1; index--) {
		int const house = Session.Players[index]->Player.ID;
		if (!State.Is_Silent(house, now)) {
			continue;
		}

		DebugString("No heartbeat from %s (house %d) for %d seconds; dropping the seat\n",
			Session.Players[index]->Name, house, (int)(DesyncClass::HEARTBEAT_TIMEOUT_MS / 1000));

		std::string const name = Session.Players[index]->Name;
		Destroy_Connection(house, 1);
		Notify_Player_Left(house, name.c_str());
	}
}


void DesyncDialogClass::Start_Countdown(void)
{
	DebugString("Counting down to the multiplayer load\n");

	CountdownActive = true;
	LastCountdownSecond = -1;

	if (!Is_Active()) {
		return;
	}

	Append_Chat_Line(Fetch_String(TXT_LOADING_SAVED_GAME));
}
