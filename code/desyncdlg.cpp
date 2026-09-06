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
#include "netdlg.h"
#include "netglobal.h"
#include "ownrdraw.h"
#include "savemgr.h"
#include "session.h"
#include "srfcache.h"
#include "syncreport.h"
#include "win.h"
#include "windlg.h"
#include "winfix.h"

#include <windowsx.h>

#include <algorithm>
#include <cstdio>
#include <cstring>


namespace {

	// Column positions within the player list, in the units the lobby's lists use.
	constexpr int HOST_COLUMN_X = 2;
	constexpr int NAME_COLUMN_X = 20;
	constexpr int STATUS_COLUMN_WIDTH = 56;
	constexpr int CHAT_BACKLOG_MAX = 50;


	// The dialog's pixel size follows the presentation layout, so the column is measured.
	int Status_Column_X(HWND list)
	{
		RECT rect = {};
		GetClientRect(list, &rect);
		return(rect.right - STATUS_COLUMN_WIDTH);
	}

}	// namespace


/// <summary>
/// Shows the dialog and pumps it until the master has decided, or this player has quit. Game
/// logic is halted for the duration; chat, sign-offs, heartbeats and the master's decision
/// still come through, since the network is serviced the whole time.
/// </summary>
DesyncDialogClass::OutcomeType DesyncDialogClass::Run(void)
{
	DebugString("Out-of-sync dialog opening on frame %d\n", Frame);

	// A raised suspension makes a nested dialog's pump service the network instead of the game.
	TacticalActive = false;
	Session.Suspended++;

	Decision = 0;
	ContinueReceived = false;
	CountdownActive = false;
	QuitEnabled = false;
	LastCountdownSecond = -1;
	ChatBacklog.clear();
	OpenedAt = Monotonic_Milliseconds();
	State.Begin(OpenedAt);

	Create_Dialog();

	OutcomeType outcome = OutcomeType::Continue;

	if (Window == NULL) {
		DebugString("The out-of-sync dialog could not be created; continuing\n");
	} else {
		while (true) {
			Call_Back();

			if (Decision == IDC_DESYNC_QUIT) {
				outcome = OutcomeType::Quit;
				break;
			}

			std::int64_t now = Monotonic_Milliseconds();
			if (!IsHostDialog && !QuitEnabled && now - OpenedAt >= DesyncClass::QUIT_DELAY_MS) {
				QuitEnabled = true;
				EnableWindow(GetDlgItem(Window, IDC_DESYNC_QUIT), TRUE);
			}

			if (!CountdownActive && SaveManager.MultiplayerLoad.Is_Pending()) {
				Start_Countdown();
			}

			if (CountdownActive) {
				Update_Countdown_Text();
				InvalidateRect(Window, NULL, FALSE);
				if (SaveManager.MultiplayerLoad.Is_Due(now)) {
					outcome = OutcomeType::Load;
					break;
				}
			} else if (ContinueReceived || Decision == IDC_DESYNC_CONTINUE) {
				if (Decision == IDC_DESYNC_CONTINUE) {
					Send_Continue();
				}
				outcome = OutcomeType::Continue;
				break;
			} else if (Decision == IDC_DESYNC_LOAD) {
				EnableWindow(Window, FALSE);
				SaveManager.Multiplayer_Load_Prompt();
				EnableWindow(Window, TRUE);
				SetFocus(GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST));
			}

			Decision = 0;
			Sleep(10);
		}
	}

	Destroy_Dialog();

	Session.Suspended--;
	TacticalActive = true;
	Map.Flag_To_Redraw(GS_REDRAW_ALL);

	DebugString("Out-of-sync dialog closed with outcome %d\n", (int)outcome);
	return(outcome);
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

	char buffer[256];
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

	Update_Player_List();
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

	Update_Player_List();
	Become_Host_If_Promoted();
}


/// <summary>
/// Creates the variant the local player gets: the decision dialog for the master, the wait
/// dialog for everyone else.
/// </summary>
void DesyncDialogClass::Create_Dialog(void)
{
	IsHostDialog = Session.Am_I_Master();
	int const id = IsHostDialog ? IDD_DESYNC_HOST : IDD_DESYNC_WAIT;

	Window = WS_Create_Dialog(ProgramInstance, id, MainWindow, Dialog_Proc, FALSE);
	if (Window == NULL) {
		return;
	}

	Fit_To_Screen();
	Center_Window_Within_Window(Window);

	RECT placed;
	GetWindowRect(Window, &placed);
	MapWindowPoints(HWND_DESKTOP, MainWindow, (POINT *)&placed, 2);
	DebugString("Out-of-sync dialog placed at %d,%d size %dx%d in a %dx%d view\n",
		placed.left, placed.top, placed.right - placed.left, placed.bottom - placed.top, VideoModeWidth, VideoModeHeight);

	// The name column goes first: the list draws each row's own string in the first column added.
	HWND list = GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST);
	if (list != NULL) {
		int const status_x = Status_Column_X(list);
		SendMessage(list, OD_ADDCOLUMN, status_x - NAME_COLUMN_X - 6, NAME_COLUMN_X);
		SendMessage(list, OD_ADDCOLUMN, 0, HOST_COLUMN_X);
		SendMessage(list, OD_ADDCOLUMN, 0, status_x);
	}
	Update_Player_List();

	if (IsHostDialog) {
		bool const can_load = SaveManager.Multiplayer_Load_Is_Allowed() && MultiplayerLoadOptionsClass().Files_Present();
		EnableWindow(GetDlgItem(Window, IDC_DESYNC_LOAD), can_load && !CountdownActive);
		EnableWindow(GetDlgItem(Window, IDC_DESYNC_CONTINUE), !CountdownActive);
	} else {
		EnableWindow(GetDlgItem(Window, IDC_DESYNC_QUIT), QuitEnabled);
	}

	Refill_Chat_List();

	HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
	if (edit != NULL) {
		SetWindowText(edit, Fetch_String(TXT_CHAT_HINT));
		ChatPlaceholderActive = true;
	}

	if (CountdownActive) {
		ShowWindow(GetDlgItem(Window, IDC_DESYNC_COUNTDOWN_TEXT), SW_SHOW);
		ShowWindow(GetDlgItem(Window, IDC_DESYNC_COUNTDOWN_BAR), SW_SHOW);
		Update_Countdown_Text();
	}

	MouseCursor->Hide_Mouse();
	ShowWindow(Window, SW_SHOWNORMAL);
	UpdateWindow(Window);
	MouseCursor->Show_Mouse();

	// The player list takes the focus, or the dialog would hand it to the chat box and clear the hint.
	SetForegroundWindow(Window);
	SetFocus(GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST));
}


void DesyncDialogClass::Destroy_Dialog(void)
{
	if (Window != NULL) {
		WS_Destroy_Dialog(Window, 0);
		Window = NULL;
	}
}


/// <summary>
/// Takes the excess height out of the chat list when the presented dialog is taller than the
/// screen, and moves everything below the list up by the same amount.
/// </summary>
void DesyncDialogClass::Fit_To_Screen(void)
{
	RECT dialog_rect;
	GetWindowRect(Window, &dialog_rect);
	int const dialog_height = dialog_rect.bottom - dialog_rect.top;
	if (dialog_height <= VideoModeHeight) {
		return;
	}

	HWND chat = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
	if (chat == NULL) {
		return;
	}
	RECT chat_rect;
	GetWindowRect(chat, &chat_rect);
	int const chat_height = chat_rect.bottom - chat_rect.top;

	int const delta = std::min<int>(dialog_height - VideoModeHeight, chat_height * 2 / 3);
	SetWindowPos(chat, NULL, 0, 0, chat_rect.right - chat_rect.left, chat_height - delta,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	for (int id : {IDC_DESYNC_CHAT_EDIT, IDC_DESYNC_COUNTDOWN_TEXT, IDC_DESYNC_COUNTDOWN_BAR,
			IDC_DESYNC_LOAD, IDC_DESYNC_CONTINUE, IDC_DESYNC_QUIT}) {
		HWND control = GetDlgItem(Window, id);
		if (control != NULL) {
			RECT rect;
			GetWindowRect(control, &rect);
			MapWindowPoints(HWND_DESKTOP, Window, (POINT *)&rect, 1);
			SetWindowPos(control, NULL, rect.left, rect.top - delta, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	SetWindowPos(Window, NULL, 0, 0, dialog_rect.right - dialog_rect.left, dialog_height - delta,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
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

	DebugString("This machine is the new master; switching to the decision dialog\n");
	Destroy_Dialog();
	Create_Dialog();
}


void DesyncDialogClass::Update_Player_List(void)
{
	if (!Is_Active()) {
		return;
	}

	HWND list = GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST);
	if (list == NULL) {
		return;
	}

	ListBox_ResetContent(list);

	int const status_x = Status_Column_X(list);
	int const master = Session.Master_Player_ID();

	for (int house = 0; house < MAX_PLAYERS && house < Houses.Count(); house++) {
		HouseClass const * housep = Houses[house];
		bool const left = State.Has_Left(house);

		// A player who left stays listed, though their seat is no longer human.
		if (housep == NULL || (!housep->IsHuman && !left)) {
			continue;
		}

		// The roster entry is gone by now, so the kept name is the only copy while the list rebuilds.
		char const * name = left && State.Left_Name(house)[0] != '\0' ? State.Left_Name(house) : housep->IniName.c_str();
		int const row = ListBox_AddString(list, name);
		if (row < 0) {
			continue;
		}

		if (house == master) {
			OwnerDraw::CellData host;
			host.type = OwnerDraw::CellData::SURFACE;
			host.surf = SurfaceCache.GetSurface("wolhost.pcx");
			host.hint.set("");
			SendMessage(list, OD_SETCELL, MAKEWPARAM(HOST_COLUMN_X, row), (LPARAM)&host);
		}

		int text = TXT_OK;
		COLORREF color = RGB(0, 200, 0);
		if (left) {
			text = TXT_SYNC_STATUS_LEFT;
			color = RGB(200, 0, 0);
		} else if (Sync_Is_Out_Of_Sync(house)) {
			text = TXT_SYNC_STATUS_OUT;
			color = RGB(200, 200, 0);
		}

		OwnerDraw::CellData status;
		status.type = OwnerDraw::CellData::TEXT;
		status.string.set(Fetch_String(text));
		status.hint.set("");
		status.color = color;
		SendMessage(list, OD_SETCELL, MAKEWPARAM(status_x, row), (LPARAM)&status);
	}

	InvalidateRect(list, NULL, FALSE);
}


void DesyncDialogClass::Refill_Chat_List(void)
{
	if (!Is_Active()) {
		return;
	}

	HWND list = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
	if (list == NULL) {
		return;
	}

	ListBox_ResetContent(list);
	for (std::string const & line : ChatBacklog) {
		ListBox_AddString(list, line.c_str());
	}
	ListBox_SetTopIndex(list, ListBox_GetCount(list) - 1);
}


void DesyncDialogClass::Append_Chat_Line(char const * line)
{
	ChatBacklog.emplace_back(line);
	if (ChatBacklog.size() > CHAT_BACKLOG_MAX) {
		ChatBacklog.erase(ChatBacklog.begin());
	}

	if (!Is_Active()) {
		return;
	}

	HWND list = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
	if (list == NULL) {
		return;
	}

	ListBox_AddString(list, line);
	while (ListBox_GetCount(list) > CHAT_BACKLOG_MAX) {
		ListBox_DeleteString(list, 0);
	}
	ListBox_SetTopIndex(list, ListBox_GetCount(list) - 1);
}


void DesyncDialogClass::Send_Chat(void)
{
	if (!Is_Active() || ChatPlaceholderActive) {
		return;
	}

	HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
	if (edit == NULL) {
		return;
	}

	char buffer[MAX_MESSAGE_LENGTH];
	GetWindowText(edit, buffer, sizeof(buffer));
	if (buffer[0] == '\0') {
		return;
	}

	SetWindowText(edit, "");
	SetFocus(edit);

	Session.MessageScope = ChatScopeType::Everyone;
	Session.MessageAddress = IPXAddressClass();
	Chat_Send(buffer);
}


void DesyncDialogClass::On_Chat_Edit_Focus(bool gained)
{
	if (!Is_Active()) {
		return;
	}

	HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
	if (edit == NULL) {
		return;
	}

	if (gained && ChatPlaceholderActive) {
		SetWindowText(edit, "");
		ChatPlaceholderActive = false;
	} else if (!gained && GetWindowTextLength(edit) == 0) {
		SetWindowText(edit, Fetch_String(TXT_CHAT_HINT));
		ChatPlaceholderActive = true;
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

	ShowWindow(GetDlgItem(Window, IDC_DESYNC_COUNTDOWN_TEXT), SW_SHOW);
	ShowWindow(GetDlgItem(Window, IDC_DESYNC_COUNTDOWN_BAR), SW_SHOW);
	Update_Countdown_Text();

	if (IsHostDialog) {
		EnableWindow(GetDlgItem(Window, IDC_DESYNC_LOAD), FALSE);
		EnableWindow(GetDlgItem(Window, IDC_DESYNC_CONTINUE), FALSE);
	}

	InvalidateRect(Window, NULL, FALSE);
}


void DesyncDialogClass::Update_Countdown_Text(void)
{
	if (!Is_Active() || !CountdownActive || !SaveManager.MultiplayerLoad.Is_Pending()) {
		return;
	}

	int const seconds = SaveManager.MultiplayerLoad.Seconds_Left(Monotonic_Milliseconds());
	if (seconds == LastCountdownSecond) {
		return;
	}
	LastCountdownSecond = seconds;

	char buffer[128];
	std::snprintf(buffer, sizeof(buffer),
		Fetch_String(seconds == 1 ? TXT_LOADING_IN_SECOND : TXT_LOADING_IN_SECONDS), seconds);
	SetDlgItemText(Window, IDC_DESYNC_COUNTDOWN_TEXT, buffer);
}


/// <summary>
/// Draws the countdown bar over its placeholder the way the reconnect dialog draws its sync
/// bars: shrinking, and green to yellow to red as the load nears.
/// </summary>
void DesyncDialogClass::Draw_Countdown_Bar(HWND window)
{
	if (!CountdownActive || !SaveManager.MultiplayerLoad.Is_Pending()) {
		return;
	}

	HWND bar = GetDlgItem(window, IDC_DESYNC_COUNTDOWN_BAR);
	if (bar == NULL) {
		return;
	}

	RECT winrect;
	Get_Display_Rect(bar, &winrect);

	Rect bar_rect;
	bar_rect.X = winrect.left;
	bar_rect.Y = winrect.top;
	bar_rect.Width = winrect.right - winrect.left;
	bar_rect.Height = winrect.bottom - winrect.top;

	int const total = (int)MultiplayerLoadClass::COUNTDOWN_MS;
	int const remaining = std::clamp((int)SaveManager.MultiplayerLoad.Milliseconds_Left(Monotonic_Milliseconds()), 0, total);
	int const elapsed = total - remaining;

	unsigned short color = DSurface::Build_Hicolor_Pixel(0, 200, 0);
	if (elapsed > total * 2 / 5) {
		color = DSurface::Build_Hicolor_Pixel(200, 200, 0);
		if (elapsed > total * 4 / 5) {
			color = DSurface::Build_Hicolor_Pixel(200, 0, 0);
		}
	}

	bar_rect.Width = std::max(6, bar_rect.Width * remaining / total);

	AlternateSurface->Fill_Rect(AlternateSurface->Get_Rect(), bar_rect, color);
}


BOOL CALLBACK DesyncDialogClass::Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
		case WM_INITDIALOG:
			OwnerDraw::Subclass_Dialog(window, 0);
			break;

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((DRAWITEMSTRUCT *)lparam);
			return(TRUE);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(window);
			DesyncDialog.Draw_Countdown_Bar(window);
			ValidateRect(window, NULL);
			break;

		case WM_MOVING:
			return(On_WM_MOVING(window, wparam, lparam));

		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
			return((BOOL)GetStockObject(BLACK_BRUSH));

		case WM_ERASEBKGND:
			return(TRUE);

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDC_DESYNC_LOAD:
				case IDC_DESYNC_CONTINUE:
				case IDC_DESYNC_QUIT:
					DesyncDialog.Decision = LOWORD(wparam);
					break;

				// Enter in the chat box arrives as IDOK, since the dialog has no default button.
				case IDOK:
					DesyncDialog.Send_Chat();
					break;

				case IDC_DESYNC_CHAT_EDIT:
					if (HIWORD(wparam) == EN_SETFOCUS) {
						DesyncDialog.On_Chat_Edit_Focus(true);
					} else if (HIWORD(wparam) == EN_KILLFOCUS) {
						DesyncDialog.On_Chat_Edit_Focus(false);
					}
					break;
			}
			break;
	}

	return(FALSE);
}
