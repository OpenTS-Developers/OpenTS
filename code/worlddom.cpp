/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "worlddom.h"

#include "_mixfile.h"
#include "_pk.h"
#include "addon.h"
#include "grphmenu.h"
#include "language/language.h"
#include "mapgen.h"
#include "mixfile.h"
#include "ownrdraw.h"
#include "wdtnet.h"


using namespace WorldDominationTour;


// The name a tour campaign is recorded under. The online service that supplied it is
// gone, so it stays empty until a tour server can be reached again.
static char g_NickName[40];


extern WDTPointer<WDTState> g_WDTNewState;
extern WDTPointer<WDTState> g_WDTResumedState;
extern WDTPointer<void> g_WDTUnusedPointer1;
extern WDTPointer<void> g_WDTUnusedPointer2;
extern WDTPointer<Campaign> g_WDTNewCampaign;
extern WDTPointer<Campaign> g_WDTResumedCampaign;



/// <summary>
/// Handles the dialog messages for the tour side choice menu.
/// This routine gives the owner draw default handler first refusal and, for a button it
/// does not consume, records the player's choice in the dialog result.
/// </summary>
/// <returns>
/// Returns with the result of the owner draw handler, or FALSE if it left the message alone.
/// </returns>
BOOL CALLBACK WDT_Faction_Choice_Menu_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int* retval;

	int rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc == 0) {

		switch (message) {
			case WM_COMMAND: {
				retval = (int *)GetWindowLong(window, DWL_USER);
				switch (LOWORD(wparam)) {
					case IDC_PICKCLAN_JOIN:
						*retval = 1;
						break;

					case IDC_PICKCLAN_GDI:
						*retval = 2;
						break;

					case IDC_PICKCLAN_NOD:
						*retval = 3;
						break;

					case IDC_CANCEL:
						*retval = 4;
						break;
				}
				break;
			}
		}
		return(FALSE);
	}
	return(rc);
}


/// <summary>
/// Asks the player which side to fight for in the tour.
/// This routine runs the graphic menu that offers the two sides and tidies it away again.
/// </summary>
/// <returns>Returns with the side the player chose, or zero if the choice was declined.</returns>
int Do_WDT_Faction_Choice_Menu(void)
{
	GraphicMenu *gmenu = Do_Graphic_Menu("WDTChoice.ini", "WDTFactionChoiceMenu");
	if (gmenu == NULL) {
		return(0);
	}
	int rc = gmenu->Presentation();
	delete gmenu;
	switch (rc) {
		case 1: return(2);
		case 2: return(3);
	}
	return(0);
}

int g_WDTChosenSide;


/// <summary>
/// Asks the tour server for the campaign cycle the player is to fight in next.
/// </summary>
/// <returns>bool; Was a cycle obtained? Never, at present: the tour was reached over an
/// online service that has been retired, and nothing has taken over its side of this
/// exchange, so a tour game can get no further than the side choice.</returns>
bool Request_WDT_Cycle(void)
{
	return(false);
}


/// <summary>
/// Sets up a World Domination Tour game for play.
/// This routine mounts the tour data files, asks the player which side to fight for,
/// negotiates a cycle with the tour server and then hands the campaign to the mission
/// selection screen. A player who abandons the selection is offered the side choice again
/// rather than being dumped back to the main menu.
/// </summary>
/// <param name="first_time">Is this the first tour game since the player logged in?</param>
/// <returns>bool; Is there a tour mission ready to play?</returns>
bool WDT_Setup_Game(bool first_time)
{
	MFCD *wdtmix = NULL;
	MFCD *wdtvoxmix = NULL;
	MFCD *localmix = NULL;

	if (CCFileClass("WDT.MIX").Is_Available()) {
		wdtmix = new MFCD("WDT.MIX", &FastKey);
	}

	if (CCFileClass("WDTVOX.MIX").Is_Available()) {
		wdtvoxmix = new MFCD("WDTVOX.MIX", &FastKey);
	}

	if (CCFileClass("Local.MIX").Is_Available()) {
		localmix = new MFCD("Local.MIX", &FastKey);
	}

	if (wdtmix != NULL && localmix != NULL) {
		bool valid = false;
		bool done = false;
		bool intro_pending = first_time;
		bool choose = first_time;
		while (!valid && !done) {

			if (choose) {
				g_WDTChosenSide = Do_WDT_Faction_Choice_Menu();
				choose = false;
			}
			if (g_WDTChosenSide == 0) {
				done = true;
			} else if (!Request_WDT_Cycle()) {
				done = true;
			} else {
				Campaign * campaign = *g_WDTNewCampaign;
				campaign->Set_Faction(g_WDTChosenSide);
				if (campaign->Is_Different_Cycle()) {
					Campaign * rcampaign = *g_WDTResumedCampaign;
					if (rcampaign != NULL && !rcampaign->Is_Different_Cycle()) {
						WDT_Review_Campaign(rcampaign);
					}
					first_time = true;
				} else {
					first_time = intro_pending;
				}
				Session.House = g_WDTChosenSide == 3 ? HOUSE_BAD : HOUSE_GOOD;
				valid = WDT_Select_Campaign(campaign, first_time);
				intro_pending = false;
				if (valid) {
					RandomMapGen.SeedData.Fixup_WDT_Settings();
				} else {
					choose = true;
				}
			}
		}

		delete wdtmix;
		delete localmix;
		delete wdtvoxmix;

		return(valid);
	}

	delete wdtmix;
	delete localmix;
	delete wdtvoxmix;

	return(false);
}


WDTPointer<WDTState> g_WDTNewState;
WDTPointer<WDTState> g_WDTResumedState;
WDTPointer<void> g_WDTUnusedPointer1;
WDTPointer<void> g_WDTUnusedPointer2;
WDTPointer<Campaign> g_WDTNewCampaign;
WDTPointer<Campaign> g_WDTResumedCampaign;


/// <summary>
/// Fetches the tour state the new campaign was built from.
/// </summary>
/// <returns>
/// Returns with a pointer to the new state, or NULL if no campaign has been started.
/// </returns>
WDTState * WDT_Get_New_State(void)
{
	return(*g_WDTNewState);
}


/// <summary>
/// Starts a fresh World Domination Tour campaign.
/// This routine adopts the state block the tour server handed back and builds the campaign
/// object that the tour screens work through for the logged in player.
/// </summary>
/// <param name="state">The tour state to start the campaign from.</param>
void WDT_Start_New_Campaign(WDTState *state)
{
	g_WDTNewState = state;
	g_WDTNewCampaign = new Campaign(g_NickName, *g_WDTNewState);
}


/// <summary>
/// Fetches the tour state the resumed campaign was built from.
/// </summary>
/// <returns>
/// Returns with a pointer to the resumed state, or NULL if nothing was resumed.
/// </returns>
WDTState * WDT_Get_Resumed_State(void)
{
	return(*g_WDTResumedState);
}


/// <summary>
/// Resumes a World Domination Tour campaign already in progress.
/// This routine adopts the state block the tour server handed back and builds the campaign
/// object that the tour screens work through for the logged in player.
/// </summary>
/// <param name="state">The tour state to resume the campaign from.</param>
void WDT_Resume_Campaign(WDTState *state)
{
	g_WDTResumedState = state;
	g_WDTResumedCampaign = new Campaign(g_NickName, *g_WDTResumedState);
}


/// <summary>
/// Fetches the first of the tour's spare state pointers.
/// </summary>
/// <returns>Returns with the pointer held, or NULL if none was ever set.</returns>
void * WDT_Get_Unused_Pointer1(void)
{
	return(*g_WDTUnusedPointer1);
}


/// <summary>
/// Sets the first of the tour's spare state pointers.
/// </summary>
/// <param name="p">The pointer to take ownership of.</param>
void WDT_Set_Unused_Pointer1(void *p)
{
	g_WDTUnusedPointer1 = p;
}


/// <summary>
/// Fetches the second of the tour's spare state pointers.
/// </summary>
/// <returns>Returns with the pointer held, or NULL if none was ever set.</returns>
void * WDT_Get_Unused_Pointer2(void)
{
	return(*g_WDTUnusedPointer2);
}


/// <summary>
/// Sets the second of the tour's spare state pointers.
/// </summary>
/// <param name="p">The pointer to take ownership of.</param>
void WDT_Set_Unused_Pointer2(void *p)
{
	g_WDTUnusedPointer2 = p;
}


/// <summary>
/// Fetches the campaign the player has just begun.
/// </summary>
/// <returns>
/// Returns with a pointer to the new campaign, or NULL if none has been started.
/// </returns>
void * WDT_Get_New_Campaign(void)
{
	return(*g_WDTNewCampaign);
}


/// <summary>
/// Fetches the campaign the player is picking up again.
/// </summary>
/// <returns>
/// Returns with a pointer to the resumed campaign, or NULL if nothing was resumed.
/// </returns>
void * WDT_Get_Resumed_Campaign(void)
{
	return(*g_WDTResumedCampaign);
}


/// <summary>
/// Fetches the territory that one of the campaign's conflicts is fought over.
/// This routine is used to resolve a conflict slot of the new campaign into the piece of
/// the world map it disputes.
/// </summary>
/// <param name="index">Index of the conflict to look up.</param>
/// <returns>
/// Returns with a pointer to the territory in dispute. Otherwise, NULL is returned.
/// </returns>
WDTTerritory *WDT_Get_Territory(int index)
{
	Conflict * conflict = (*g_WDTNewCampaign)->FindConflict(index);
	if (conflict != NULL) {
		return(conflict->Territory);
	}
	return(NULL);
}


/// <summary>
/// Determines if the World Domination Tour is available.
/// This routine guards every entry point into the tour. The mode ships with the Firestorm
/// addon, so a plain Tiberian Sun installation never sees it.
/// </summary>
/// <returns>bool; Is the tour available to the player?</returns>
bool WDT_Is_Allowed(void)
{
	return(Addon_Enabled(ADDON_FIRESTORM));
}
