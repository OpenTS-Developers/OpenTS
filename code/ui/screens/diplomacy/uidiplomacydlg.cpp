/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "_ui.h"
#include "chat.h"
#include "data.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "houstype.h"
#include "init.h"
#include "ipxmgr.h"
#include "nettiming.h"
#include "scenario.h"
#include "session.h"
#include "ui/screens/diplomacy/uidiplomacy.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"

#include <array>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>


namespace
{

// How often the open screen re-reads the game, in milliseconds.
constexpr NetTiming::Milliseconds UI_DIPLOMACY_REFRESH = 250;

// A click waits at most this long for its event before the box shows the game again.
constexpr NetTiming::Milliseconds UI_DIPLOMACY_PENDING_LIMIT = 10000;

// The waiting mark appears once this machine has waited this long on a player in the window.
constexpr NetTiming::Milliseconds UI_DIPLOMACY_WAITING_MARK = 1000;

constexpr NetTiming::Milliseconds UI_DIPLOMACY_LATENCY_FAIR = 150;
constexpr NetTiming::Milliseconds UI_DIPLOMACY_LATENCY_POOR = 300;


struct UIDiplomacyPending
{
	bool Active = false;
	bool Wanted = false;
	int QueuedFrame = 0;
	NetTiming::Milliseconds QueuedAt = 0;
};


std::array<UIDiplomacyPending, HOUSE_MAX> PendingAlly;
std::array<UIDiplomacyPending, HOUSE_MAX> PendingGive;


NetTiming::Milliseconds Now(void)
{
	return(NetTiming::Default_Clock().Now());
}


HouseClass * House_At(int house)
{
	return((house >= 0 && house < Houses.Count()) ? Houses[house] : NULL);
}


void Mark_Pending(UIDiplomacyPending & pending, bool wanted)
{
	pending.Active = true;
	pending.Wanted = wanted;
	pending.QueuedFrame = Frame;
	pending.QueuedAt = Now();
}


/// <summary>
/// Returns the state a box shows: the requested one while its event is on the way, the game's
/// once the game agrees, the event has had time to run, or the wait limit passes.
/// </summary>
bool Shown_State(UIDiplomacyPending & pending, bool actual)
{
	if (!pending.Active) {
		return(actual);
	}

	int const due = pending.QueuedFrame + (int)Session.MaxAhead + (int)Session.FrameSendRate;
	if (actual == pending.Wanted || Frame > due
		|| NetTiming::Milliseconds_Have_Elapsed(pending.QueuedAt, Now(), UI_DIPLOMACY_PENDING_LIMIT)) {
		pending.Active = false;
		return(actual);
	}
	return(pending.Wanted);
}


class UIDiplomacyEngineServiceClass : public UIDiplomacyServiceClass
{
	public:
		virtual void Toggle_Alliance(int house) override
		{
			HouseClass const * target = House_At(house);
			bool const allied = target != NULL && PlayerPtr->Is_Ally(target);
			if (Request_Alliance_Toggle(target)) {
				Mark_Pending(PendingAlly[house], !allied);
			}
		}

		virtual void Toggle_Control(int house) override
		{
			HouseClass const * target = House_At(house);
			bool const granted = target != NULL && PlayerPtr->Grants_Control_To(target);
			if (Request_Control_Grant_Toggle(target)) {
				Mark_Pending(PendingGive[house], !granted);
			}
		}

		virtual void Set_Sends_To(int house, bool sends) override
		{
			Chat_Set_Sends_To(House_At(house), sends);
		}

		virtual void Set_Muted(int house, bool muted) override
		{
			Chat_Set_Muted(House_At(house), muted);
		}

		virtual bool Chat_Edit_Open(void) override
		{
			return(Session.Messages.Is_Edit() != 0);
		}
};


UIDiplomacyEngineServiceClass Service;
std::unique_ptr<UIDiplomacyPresenterClass> Presenter;
std::unique_ptr<UIViewClass> View;
NetTiming::Milliseconds LastBuild = 0;


std::string Name_Color(HouseClass const * house)
{
	if (house->IsDefeated || house->IsObserver) {
		return(UI_Color_Text(RGB(0x84, 0x8A, 0x84)));
	}
	RGBClass const & color = house->RemapColorRGB;
	return(UI_Color_Text(RGB(color.Get_Red(), color.Get_Green(), color.Get_Blue())));
}


std::string Seconds_Text(NetTiming::Milliseconds milliseconds)
{
	char text[32];
	std::snprintf(text, sizeof(text), "%u.%u s", milliseconds / 1000, (milliseconds % 1000) / 100);
	return(text);
}


char const * Alliance_Tip(HouseClass const * house, bool allied, bool enabled)
{
	if (enabled) {
		return(allied ? "Ending the alliance also ends theirs" : "Ally with this player");
	}
	if (PlayerPtr->IsObserver) {
		return("Observers cannot make alliances");
	}
	if (PlayerPtr->IsDefeated) {
		return("You can no longer change alliances");
	}
	if (Scen->Special.IsAllianceFixed) {
		return("Alliances are fixed in this game");
	}
	if (!Session.Options.AlliesAllowed) {
		return("Alliances are off in this game");
	}
	if (!house->IsHuman) {
		return("Only human players can be allies");
	}
	return("Allying with them would leave no enemies");
}


char const * Control_Tip(HouseClass const * house, bool granted, bool enabled)
{
	if (enabled) {
		return(granted ? "Take back command of your units" : "Let this player command your units");
	}
	if (PlayerPtr->IsObserver) {
		return("Observers have no units to share");
	}
	if (PlayerPtr->IsDefeated) {
		return("You can no longer share control");
	}
	if (!house->IsHuman) {
		return("Only human players can be given control");
	}
	return("Needs an alliance on both sides");
}


void Fill_Connection(UIDiplomacyRow & row, HouseClass const * house, bool chat)
{
	bool const linked = house != PlayerPtr && Session.Is_Network_Game()
		&& Ipx.Connection_Index(house->HeapID) != ConnManClass::CONNECTION_NONE;

	row.HasChat = chat && linked && house->IsHuman;
	row.Send = Chat_Sends_To(house);
	row.Mute = Chat_Is_Muted(house);

	std::optional<NetTiming::Milliseconds> const round_trip = linked ? Ipx.Round_Trip_MS(house->HeapID) : std::nullopt;
	if (round_trip) {
		row.Latency = std::to_string(*round_trip) + " ms";
		row.LatencyFair = *round_trip > UI_DIPLOMACY_LATENCY_FAIR && *round_trip <= UI_DIPLOMACY_LATENCY_POOR;
		row.LatencyPoor = *round_trip > UI_DIPLOMACY_LATENCY_POOR;
	} else {
		row.Latency = "-";
	}

	NetTiming::Milliseconds const waited = linked ? Session.WaitedOn.Recent((unsigned int)house->HeapID, Now()) : 0;
	if (waited >= UI_DIPLOMACY_WAITING_MARK) {
		row.Waiting = true;
		row.WaitingTip = "Waited on them " + Seconds_Text(waited) + " in the last " + std::to_string(NetTiming::WAIT_WINDOW_SECONDS) + " s";
	}
}


UIDiplomacyRow Player_Row(HouseClass const * house, UIDiplomacyState const & state)
{
	UIDiplomacyRow row;
	row.House = house->HeapID;
	row.Name = Session.Shown_Name(house);
	row.Color = Name_Color(house);
	row.Kills = std::to_string(house->Total_Kills());

	if (house == PlayerPtr) {
		row.Latency = "-";
		return(row);
	}

	if (house->IsDefeated) {
		row.NameTip = "Defeated";
	} else {
		bool const allied = PlayerPtr->Is_Ally(house);
		row.HasAlly = true;
		row.AllyEnabled = PlayerPtr->Can_Change_Alliance(house) && (allied || PlayerPtr->Is_Allowed_To_Ally(house));
		row.AllyTip = Alliance_Tip(house, allied, row.AllyEnabled);
		row.Ally = Shown_State(PendingAlly[house->HeapID], allied);
		if (PendingAlly[house->HeapID].Active) {
			row.AllyEnabled = false;
		}
		row.TheirsYes = house->Is_Ally(PlayerPtr);
		row.Theirs = row.TheirsYes ? "Yes" : "No";

		if (state.ShowControl) {
			bool const granted = PlayerPtr->Grants_Control_To(house);
			row.HasGive = true;
			row.GiveEnabled = PlayerPtr->Can_Grant_Control(house);
			row.GiveTip = Control_Tip(house, granted, row.GiveEnabled);
			row.Give = Shown_State(PendingGive[house->HeapID], granted);
			if (PendingGive[house->HeapID].Active) {
				row.GiveEnabled = false;
			}
			row.GivenYes = house->Grants_Control_To(PlayerPtr);
			row.Given = row.GivenYes ? "Yes" : "No";
		}
	}

	Fill_Connection(row, house, state.ShowChat);
	return(row);
}


UIDiplomacyRow Observer_Row(HouseClass const * house, UIDiplomacyState const & state)
{
	UIDiplomacyRow row;
	row.House = house->HeapID;
	row.Name = Session.Shown_Name(house);
	row.Color = Name_Color(house);
	if (house == PlayerPtr) {
		row.Latency = "-";
	} else {
		Fill_Connection(row, house, state.ShowChat);
	}
	return(row);
}


std::string Info_Text(void)
{
	unsigned int const seconds = (unsigned int)(Scen->ElapsedTimer / TIMER_SECOND);
	char time[16];
	if (seconds >= 3600) {
		std::snprintf(time, sizeof(time), "%u:%02u:%02u", seconds / 3600, (seconds / 60) % 60, seconds % 60);
	} else {
		std::snprintf(time, sizeof(time), "%u:%02u", seconds / 60, seconds % 60);
	}

	std::string info = Scen->Description;
	if (!info.empty()) {
		info += " - ";
	}
	info += time;

	if (Session.Is_Network_Game()) {
		NetTiming::TimingSettings const settings{Session.FrameSendRate, Session.MaxAhead};
		info += " - Connection: ";
		info += Fetch_String(Network_Quality_Text_ID(NetTiming::Connection_Quality_For_Settings(settings)));
	}
	return(info);
}


std::string Footer_Text(void)
{
	if (!Session.Is_Network_Game()) {
		return(std::string());
	}

	HouseClass const * worst = NULL;
	NetTiming::Milliseconds most = 0;
	for (int index = 0; index < Houses.Count(); index++) {
		HouseClass const * house = Houses[index];
		if (house == NULL || house == PlayerPtr) {
			continue;
		}
		NetTiming::Milliseconds const waited = Session.WaitedOn.Recent((unsigned int)house->HeapID, Now());
		if (waited > most) {
			most = waited;
			worst = house;
		}
	}

	if (worst == NULL || most < UI_DIPLOMACY_WAITING_MARK) {
		return(std::string());
	}
	return("Waiting most on " + Session.Shown_Name(worst) + ": " + Seconds_Text(most) + " in the last "
		+ std::to_string(NetTiming::WAIT_WINDOW_SECONDS) + " s");
}


UIDiplomacyState Build_State(void)
{
	UIDiplomacyState state;
	state.ShowControl = Session.Is_Network_Game() && Session.Options.SharedControl;
	state.ShowChat = Session.Is_Network_Game();

	if (!PlayerPtr->IsObserver && PlayerPtr->Class->IsMultiplay) {
		state.Players.push_back(Player_Row(PlayerPtr, state));
	}

	for (int index = 0; index < Houses.Count(); index++) {
		HouseClass const * house = Houses[index];
		if (house == NULL || !house->Class->IsMultiplay || (house == PlayerPtr && !house->IsObserver)) {
			continue;
		}
		if (house->IsObserver) {
			state.Observers.push_back(Observer_Row(house, state));
		} else {
			state.Players.push_back(Player_Row(house, state));
		}
	}

	state.Info = Info_Text();
	state.Footer = Footer_Text();
	return(state);
}


bool Can_Show(void)
{
	return(GameActive && Session.Type != GAME_NORMAL && PlayerPtr != NULL
		&& SpecialDialog == SDLG_NONE && !UIShell.Screen_Shown());
}

}


/// <summary>
/// Opens the diplomacy screen beside the game, or closes it when it is open. Nothing opens in a
/// campaign, during a recording's playback, or while another screen or dialog is up.
/// </summary>
void UI_Diplomacy_Toggle(void)
{
	if (View != nullptr) {
		UI_Diplomacy_Close();
		return;
	}

	if (!Can_Show()) {
		return;
	}

	Presenter = std::make_unique<UIDiplomacyPresenterClass>(Service);
	Presenter->State = Build_State();
	View = UI_Diplomacy_View(*Presenter);
	if (!UIShell.Show_Modeless(*View, true)) {
		View.reset();
		Presenter.reset();
		return;
	}
	LastBuild = Now();
}


/// <summary>
/// Runs the open screen: its clicks take effect, its rows follow the game within a quarter
/// second, and it closes when it may no longer show. Each loop that renders the map calls it
/// once per pass.
/// </summary>
void UI_Diplomacy_Update(void)
{
	if (View == nullptr) {
		return;
	}

	if (!Can_Show()) {
		UI_Diplomacy_Close();
		return;
	}

	Presenter->Drain();
	if (Presenter->Closed) {
		UI_Diplomacy_Close();
		return;
	}

	bool const acted = Presenter->Take_Acted();
	if (acted || NetTiming::Milliseconds_Have_Elapsed(LastBuild, Now(), UI_DIPLOMACY_REFRESH)) {
		LastBuild = Now();
		UIDiplomacyState state = Build_State();
		if (acted || state != Presenter->State) {
			Presenter->State = std::move(state);
			View->Sync();
		}
	}
}


void UI_Diplomacy_Close(void)
{
	if (View != nullptr) {
		UIShell.Hide_Modeless(*View);
		View.reset();
	}
	Presenter.reset();
	PendingAlly = {};
	PendingGive = {};
}


bool UI_Diplomacy_Is_Shown(void)
{
	return(View != nullptr);
}
