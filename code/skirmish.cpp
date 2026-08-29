/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "skirmish.h"

#include "_rules.h"
#include "data.h"
#include "globals.h"
#include "goptions.h"
#include "houstype.h"
#include "init.h"
#include "language\language.h"
#include "mapgen.h"
#include "mplayer.h"
#include "msgbox.h"
#include "netshare.h"
#include "newmenu.h"
#include "ownrdraw.h"
#include "rules.h"
#include "win.h"


BOOL CALLBACK Skirmish_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL Skirmish_On_WM_INITDIALOG(HWND window, WPARAM wparam, LPARAM lparam);


/// <summary>
/// Handles a control notification from the skirmish dialog.
/// This routine services the buttons and check boxes of the setup dialog. When the player
/// accepts the dialog, the slider and combo box settings are harvested into the session
/// options and the local player is added to the player list; when the player cancels, only
/// the handle, side, and color are remembered.
/// </summary>
/// <param name="message">The identifier of the control that sent the notification.</param>
/// <param name="lparam">The notification code that came with the command.</param>
void Skirmish_On_WM_COMMAND(HWND window, int message, WPARAM wparam, LPARAM lparam)
{
	int * rc = (int *)GetWindowLong(window, DWL_USER);
	char buffer[256];
	HWND handle;

	switch (message) {
		case IDOK: {
			if (lparam == 0) {
				EnableWindow(GetDlgItem(window, 1), FALSE);

				int waypoint_count = RandomMapWaypointCount(Session.Options.ScenarioIndex);
				int waypoint = 1;

				handle = GetDlgItem(window, IDC_SKIRMISH_AIPLAYERS);
				if (handle) waypoint = Slider_GetPos(handle) + 1;

				if (waypoint_count < waypoint) {
					sprintf(buffer, Fetch_String(TXT_SCENARIO_TOO_SMALL), waypoint_count);
					WWMessageBox().Process(buffer, TXT_OK);
					EnableWindow(GetDlgItem(window, 1), TRUE);
					return;
				}

				GetWindowText(GetDlgItem(window, IDC_SKIRMISH_NAME), Session.Handle, sizeof(Session.Handle));

				handle = GetDlgItem(window, IDC_SKIRMISH_UNITCOUNT);
				if (handle) Session.Options.UnitCount = Slider_GetPos(handle);

				handle = GetDlgItem(window, IDC_SKIRMISH_TECHLEVEL);
				if (handle) BuildLevel = Slider_GetPos(handle);

				handle = GetDlgItem(window, IDC_SKIRMISH_CREDITS);
				if (handle) Session.Options.Credits = Slider_GetPos(handle);

				handle = GetDlgItem(window, IDC_DIFFICULTY_SLIDER);
				if (handle) Session.Options.AIDifficulty = (DiffType)Slider_GetPos(handle);

				handle = GetDlgItem(window, IDC_SKIRMISH_AIPLAYERS);
				if (handle) Session.Options.AIPlayers = Slider_GetPos(handle);

				handle = GetDlgItem(window, IDC_GAME_SPEED_SLIDER);
				if (handle) {
					Session.Options.GameSpeed = 6 - Slider_GetPos(handle);
					Options.GameSpeed = Session.Options.GameSpeed;
				}

				handle = GetDlgItem(window, IDC_SKIRMISH_SIDE);
				if (handle) Session.House = ComboBox_GetCurSel(handle);

				handle = GetDlgItem(window, IDC_SKIRMISH_COLOR);
				if (handle) {
					Session.ColorIdx = ComboBox_GetCurSel(handle);
					Session.PrefColor = Session.ColorIdx;
				}

				NodeNameType * who = new NodeNameType;
				if (who) {
					strcpy(who->Name, Session.Handle);
					who->Player.House = Session.House;
					who->Player.Color = Session.ColorIdx;
					who->Player.ProcessTime = -1;
					Session.Players.Add(who);
				}

				handle = GetDlgItem(window, IDC_SKIRMISH_BASES);
				if (handle) Session.Options.Bases = Button_GetCheck(handle) == BST_CHECKED;
				handle = GetDlgItem(window, IDC_SKIRMISH_CRATES);
				if (handle) Session.Options.Goodies = Button_GetCheck(handle) == BST_CHECKED;
				handle = GetDlgItem(window, IDC_SKIRMISH_FOG);
				if (handle) Session.Options.FogOfWar = Button_GetCheck(handle) == BST_CHECKED;
				handle = GetDlgItem(window, IDC_SKIRMISH_BRIDGES);
				if (handle) Session.Options.BridgeDestruction = Button_GetCheck(handle) == BST_CHECKED;
				handle = GetDlgItem(window, IDC_REDEPLOY_MCV);
				if (handle) Session.Options.MCVRedeploy = Button_GetCheck(handle) == BST_CHECKED;
				handle = GetDlgItem(window, IDC_SHORT_GAME);
				if (handle) Session.Options.ShortGame = Button_GetCheck(handle) == BST_CHECKED;
				Session.Options.HarvTruce = false;
				handle = GetDlgItem(window, IDC_MULTI_ENGINEER);
				if (handle) Session.Options.CrapEngineers = Button_GetCheck(handle) == BST_CHECKED;

				if (MultiplayerMapPreview) {
					delete MultiplayerMapPreview;
					MultiplayerMapPreview = NULL;
				}
				*rc = IDOK;
			}
		}
			break;

		case IDCANCEL:
			if (!lparam) {
				GetWindowText(GetDlgItem(window, IDC_SKIRMISH_NAME), Session.Handle, sizeof(Session.Handle));
				handle = GetDlgItem(window, IDC_SKIRMISH_SIDE);
				if (handle) Session.House = ComboBox_GetCurSel(handle);
				handle = GetDlgItem(window, IDC_SKIRMISH_COLOR);
				if (handle) {
					Session.ColorIdx = ComboBox_GetCurSel(handle);
					Session.PrefColor = Session.ColorIdx;
				}
				*rc = IDCANCEL;
			}
			break;

		case IDC_SHORT_GAME:
			handle = GetDlgItem(window, IDC_SHORT_GAME);
			if (handle && Button_GetCheck(handle) == BST_CHECKED) {
				SendDlgItemMessage(window, IDC_SKIRMISH_BASES, BM_SETCHECK, TRUE, 0);
			}
			break;

		case IDC_MULTIMAP: {
			int old_scen = Session.Options.ScenarioIndex;
			strcpy(buffer, Session.ScenarioFileName);
			strcpy(buffer, Session.Options.ScenarioDescription);
			ShowWindow(window, SW_HIDE);
			if (Scenario_Dialog(MainWindow) == IDCANCEL) {
				Session.Options.ScenarioIndex = old_scen;
				Set_Scenario_Info_From_Index(old_scen);
				Update_Network_Dialog_Preview(window);
				ShowWindow(window, SW_SHOW);
				if (stricmp(Session.Scenarios[Session.Options.ScenarioIndex]->Get_Filename(), RANDOM_MAP_FILE_NAME) == 0) {
					delete MultiplayerMapPreview;
					MultiplayerMapPreview = new MapPreviewClass;
					MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
					if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
						Update_Network_Dialog_Preview(window);
					}
					InvalidateRect(window, NULL, FALSE);
				} else {
					Update_Network_Dialog_Preview(window);
				}
				InvalidateRect(window, NULL, FALSE);
			} else {
				ShowWindow(window, SW_SHOW);
				if (Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex) == true) {
					SendDlgItemMessage(window, IDC_SCENARIONAME, WM_SETTEXT, 0, (LPARAM)Session.Options.ScenarioDescription);
					if (stricmp(Session.Scenarios[Session.Options.ScenarioIndex]->Get_Filename(), "RandMap.Sed") == 0) {
						if (MultiplayerMapPreview != NULL) {
							delete MultiplayerMapPreview;
							MultiplayerMapPreview = new MapPreviewClass;
							MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
						}
						if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
							Update_Network_Dialog_Preview(window);
						}
						InvalidateRect(window, NULL, FALSE);
					} else {
						Update_Network_Dialog_Preview(window);
					}
				} else {
					Session.Options.ScenarioIndex = old_scen;
				}
			}
		}
			break;

		case IDC_SKIRMISH_BASES:
			handle = GetDlgItem(window, IDC_SKIRMISH_BASES);
			if (handle && Button_GetCheck(handle) != 1) {
				SendDlgItemMessage(window, IDC_SHORT_GAME, BM_SETCHECK, 0, 0);
			}
			break;
	}
}


/// <summary>
/// Handles the skirmish game setup dialog.
/// This routine is used by the main menu when the player picks a skirmish game. The house
/// and side rules are re-read first so that the dialog offers the current playable sides,
/// and the chosen settings are recorded as the player's multiplayer preferences on the way
/// out.
/// </summary>
/// <returns>bool; Did the player accept the settings and ask for the game to start?</returns>
bool Skirmish_Mode_Dialog(void)
{
	int rc = -1;

	Prepare_Side_Roster();

	Hide_Mouse();
	Draw_Menu_Background();
	Show_Mouse();

	HWND dialog = OwnerDraw::Begin_Dialog(IDD_SKIRMISH, Skirmish_Dialog_Proc);
	if (dialog) {
		SetWindowLong(dialog, DWL_USER, (LONG)&rc);
		OwnerDraw::Display_Dialog(dialog);
		while (rc != IDOK && rc != IDCANCEL) {
			if (OwnerDraw::Dialog_Message_Handler() == IDOK) {
				break;
			}
			Title_Screen_Restore();
		}
		OwnerDraw::End_Dialog(dialog);
	}

	if (MultiplayerMapPreview != NULL) {
		delete MultiplayerMapPreview;
		MultiplayerMapPreview = NULL;
	}

	Session.Write_MultiPlayer_Settings();

	if (rc == IDCANCEL) {
		Hide_Mouse();
		Draw_Menu_Background();
		Show_Mouse();
	}

	if (rc == IDOK) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Handles the messages sent to the skirmish setup dialog.
/// The owner draw dialog handler is given first refusal on every message. Anything it
/// leaves alone is dealt with here -- dialog setup, button and slider notifications, and
/// repainting the map preview.
/// </summary>
/// <returns>Returns with TRUE if the message was handled, otherwise FALSE so that Windows
/// performs its default processing.</returns>
BOOL CALLBACK Skirmish_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc == 0) {

		switch (message) {
			case WM_COMMAND:
				Skirmish_On_WM_COMMAND(window, LOWORD(wparam), lparam, HIWORD(wparam));
				return(TRUE);

			case WM_INITDIALOG:
				return(Skirmish_On_WM_INITDIALOG(window, wparam, lparam));

			case WM_PAINT:
				if (MultiplayerMapPreview) {
					MultiplayerMapPreview->Blit_Preview(window);
				}
				ValidateRect(window, NULL);
				break;

			case WM_HSCROLL: {
				int code = LOWORD(wparam);
				if (code != SB_THUMBPOSITION && code != SB_THUMBTRACK) {
					Slider_GetPos((HWND)lparam);
				}
				switch (GetDlgCtrlID((HWND)lparam)) {
					case IDC_SKIRMISH_UNITCOUNT:
						GetDlgItem(window, IDC_SKIRMISH_UNITCOUNT_LABEL);
						break;
					case IDC_SKIRMISH_TECHLEVEL:
						GetDlgItem(window, IDC_SKIRMISH_TECHLEVEL_LABEL);
						break;
					case IDC_DIFFICULTY_SLIDER:
						GetDlgItem(window, IDC_SKIRMISH_AILEVEL_LABEL);
						break;
					case IDC_SKIRMISH_AIPLAYERS:
						GetDlgItem(window, IDC_SKIRMISH_AIPLAYERS_LABEL);
						break;
				}
			}
				break;
		}
		return(FALSE);
	}
	return(rc);
}


/// <summary>
/// Prepares the skirmish dialog for display.
/// This routine fills the sliders, side and color combo boxes, and option check boxes
/// with the player's current multiplayer settings, selects the starting scenario, and
/// puts up its map preview.
/// </summary>
/// <returns>Always FALSE, so that Windows leaves the keyboard focus where the dialog
/// template put it.</returns>
BOOL Skirmish_On_WM_INITDIALOG(HWND window, WPARAM wparam, LPARAM lparam)
{
	#define MP_MIN_MONEY 2500

	HWND handle = GetDlgItem(window, IDC_SKIRMISH_UNITCOUNT);
	if (handle) {
		Slider_SetRange(handle, SessionClass::CountMin[1], SessionClass::CountMax[1]);
		Slider_SetPos(handle, Session.Options.UnitCount);
	}

	handle = GetDlgItem(window, IDC_SKIRMISH_UNITCOUNT_LABEL);
	if (handle) {
		//
	}

	handle = GetDlgItem(window, IDC_SKIRMISH_TECHLEVEL);
	if (handle) {
		Slider_SetRange(handle, 1, MPLAYER_BUILD_LEVEL_MAX);
		Slider_SetPos(handle, BuildLevel);
	}

	handle = GetDlgItem(window, IDC_SKIRMISH_TECHLEVEL_LABEL);
	if (handle) {
		//
	}

	handle = GetDlgItem(window, IDC_DIFFICULTY_SLIDER);
	if (handle) {
		Slider_SetRange(handle, 0, 2);
		Slider_SetPos(handle, Session.Options.AIDifficulty);
	}

	handle = GetDlgItem(window, IDC_SKIRMISH_AILEVEL_LABEL);
	if (handle) {
		//
	}

	handle = GetDlgItem(window, IDC_SKIRMISH_CREDITS);
	if (handle) {
		Slider_SetRange(handle, MP_MIN_MONEY, Rule->MPMaxMoney);
		Slider_SetPos(handle, Session.Options.Credits);
		SendMessage(handle, OD_SETTRACKSTEP, 0, 250);
	}

	handle = GetDlgItem(window, IDC_SKIRMISH_AIPLAYERS);
	if (handle) {
		Slider_SetRange(handle, 1, 7);
		Slider_SetPos(handle, Session.Options.AIPlayers > 1 ? Session.Options.AIPlayers : 1);
	}

	handle = GetDlgItem(window, IDC_GAME_SPEED_SLIDER);
	if (handle) {
		Slider_SetRange(handle, 0, 6);
		Slider_SetPos(handle, 6 - Session.Options.GameSpeed);
	}

	handle = GetDlgItem(window, IDC_SKIRMISH_NAME);
	if (handle) SetWindowText(handle, Session.Handle);

	handle = GetDlgItem(window, IDC_SKIRMISH_SIDE);
	if (handle) {
		for (int i = 0; i < HouseTypes.Count(); i++) {
			if (HouseTypes[i]->IsMultiplay) {
				ComboBox_AddString(handle, HouseTypes[i]->GivenName);
			}
		}
		ComboBox_SetCurSel(handle, Session.House <= HOUSE_BAD ? Session.House : HOUSE_BAD);
	}

	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_GOLD));
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_RED));
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_BLUE));
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_GREEN));
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_ORANGE));
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_SKY_BLUE));
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_PURPLE));
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_PINK));
	SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, CB_SETCURSEL, Session.PrefColor, 0);

	for (int player = 0; player < MAX_PLAYERS; player++) {
		SendDlgItemMessage(window, IDC_SKIRMISH_COLOR, OD_SETCOLOR, player, (LPARAM)PlayerColorTable[player]);
	}

	Set_Scenario_Info_From_Index(0);
	Session.Options.ScenarioIndex = 0;
	SendDlgItemMessage(window, IDC_SCENARIONAME, WM_SETTEXT, 0, (LPARAM)Session.Options.ScenarioDescription);
	Clear_Vector(&Session.Players);

	handle = GetDlgItem(window, IDC_SKIRMISH_BASES);
	if (handle) Button_SetCheck(handle, Session.Options.Bases ? BST_CHECKED : BST_UNCHECKED);

	handle = GetDlgItem(window, IDC_SKIRMISH_CRATES);
	if (handle) Button_SetCheck(handle, Session.Options.Goodies ? BST_CHECKED : BST_UNCHECKED);

	handle = GetDlgItem(window, IDC_SKIRMISH_FOG);
	if (handle) Button_SetCheck(handle, Session.Options.FogOfWar ? BST_CHECKED : BST_UNCHECKED);

	handle = GetDlgItem(window, IDC_SKIRMISH_BRIDGES);
	if (handle) Button_SetCheck(handle, Session.Options.BridgeDestruction ? BST_CHECKED : BST_UNCHECKED);

	handle = GetDlgItem(window, IDC_REDEPLOY_MCV);
	if (handle) Button_SetCheck(handle, Session.Options.MCVRedeploy ? BST_CHECKED : BST_UNCHECKED);

	handle = GetDlgItem(window, IDC_MULTI_ENGINEER);
	if (handle) Button_SetCheck(handle, Session.Options.CrapEngineers ? BST_CHECKED : BST_UNCHECKED);

	handle = GetDlgItem(window, IDC_SHORT_GAME);
	if (handle) Button_SetCheck(handle, Session.Options.ShortGame ? BST_CHECKED : BST_UNCHECKED);

	Update_Network_Dialog_Preview(window);
	return(FALSE);
}
