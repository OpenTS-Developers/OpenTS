/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the out-of-sync screen: what it reads out of the session and the entry
// DesyncDialogClass calls.

#include "ui/screens/desync/uidesync.h"

#include "_ui.h"
#include "data.h"
#include "desyncdlg.h"
#include "house.h"
#include "language/language.h"
#include "savemgr.h"
#include "session.h"
#include "syncreport.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"

#include <cstdio>


// The colors the dialog draws a player's standing in, as its own cells carry them.
static char const * const UI_DESYNC_OK = "#00c800";
static char const * const UI_DESYNC_OUT = "#c8c800";
static char const * const UI_DESYNC_LEFT_COLOR = "#c80000";


namespace
{

class UIDesyncEngineServiceClass : public UIDesyncServiceClass
{
	public:
		virtual void Read(UIDesyncState & state) override
		{
			state.Host = DesyncDialog.Is_Host();
			state.Players.clear();

			int const master = Session.Master_Player_ID();
			for (int house = 0; house < MAX_PLAYERS && house < Houses.Count(); house++) {
				HouseClass const * housep = Houses[house];
				bool const left = DesyncDialog.Has_Left(house);

				// A player who left stays listed, though their seat is no longer human.
				if (housep == NULL || (!housep->IsHuman && !left)) {
					continue;
				}

				UIDesyncPlayerRow row;

				// The roster entry is gone by now, so the kept name is the only copy while the
				// list rebuilds.
				char const * kept = DesyncDialog.Left_Name(house);
				row.Name = (left && kept[0] != '\0') ? kept : housep->IniName.c_str();
				if (left) {
					row.Status = Fetch_String(TXT_SYNC_STATUS_LEFT);
					row.Color = UI_DESYNC_LEFT_COLOR;
				} else if (Sync_Is_Out_Of_Sync(house)) {
					row.Status = Fetch_String(TXT_SYNC_STATUS_OUT);
					row.Color = UI_DESYNC_OUT;
				} else {
					row.Status = Fetch_String(TXT_OK);
					row.Color = UI_DESYNC_OK;
				}
				if (house == master) {
					row.Mark = "wolhost.pcx";
				}

				state.Players.push_back(std::move(row));
			}

			state.Chat = DesyncDialog.Chat_Backlog();

			state.Counting = DesyncDialog.Is_Counting_Down();
			state.CountdownText = DesyncDialog.Countdown_Caption();
			state.Countdown = DesyncDialog.Countdown_Left();
			state.CountdownColor = DesyncDialog.Countdown_Color();

			state.LoadEnabled = state.Host && !state.Counting && DesyncDialog.Load_Is_Allowed();
			state.ContinueEnabled = state.Host && !state.Counting;
			state.QuitEnabled = DesyncDialog.Quit_Is_Allowed();
		}

		virtual void Say(char const * text) override
		{
			DesyncDialog.Say(text);
		}

		virtual bool Settled(void) override
		{
			return(DesyncDialog.Decision_Is_Settled());
		}
};

}


UIDesyncServiceClass & UI_Desync_Service(void)
{
	static UIDesyncEngineServiceClass service;
	return(service);
}


UIDesyncChoiceType UI_Desync_Dialog(void)
{
	UIDesyncPresenterClass presenter(UI_Desync_Service());
	std::unique_ptr<UIViewClass> view = UI_Desync_View(presenter);

	UIResult result = UIShell.Run_Modal(*view, []() {
		UI_Service_Game();
		DesyncDialog.Service_Screen();
		return(DesyncDialog.Decision_Is_Settled());
	});

	if (result == UI_RESULT_FAILED_TO_OPEN) {
		return(UI_DESYNC_NONE);
	}

	return(presenter.Choice);
}
