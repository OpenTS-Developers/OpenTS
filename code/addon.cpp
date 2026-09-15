/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "addon.h"

#include "ui/screens/menu/uimenu.h"

#include "_deploymentconfig.h"
#include "ccfile.h"
#include "data.h"
#include "deploymentconfig.h"
#include "language/language.h"
#include "win.h"

int AvailableAddOns = 1 << ADDON_BASE_GAME;
int ActiveAddOns = 1 << ADDON_BASE_GAME;
AddonType RequiredAddon = ADDON_BASE_GAME;

/// <summary>
/// Steps an addon type on to the value after it.
/// </summary>
/// <returns>Returns with the incremented value.</returns>
AddonType operator++(AddonType & val)
{
	val = AddonType(int(val) + 1);
	return(val);
}


/// <summary>
/// Steps an addon type back to the value before it.
/// </summary>
/// <returns>Returns with the decremented value.</returns>
AddonType operator--(AddonType & val)
{
	val = AddonType(int(val) - 1);
	return(val);
}


/// <summary>
/// Asks the player which game they wish to play.
/// This routine puts up the game type dialog whenever an expansion has been detected, and
/// enables whichever addon the player settles on. With nothing but the base game present
/// there is no choice to make, so the dialog never appears.
/// </summary>
/// <param name="type">The addon that the player chose to play.</param>
/// <returns>bool; Should the game carry on? Returns false if the player backed out.</returns>
bool Select_Game_Type_Dialog(AddonType &type)
{
	type = ADDON_BASE_GAME;

	if (Addon_Installed(ADDON_ANY)) {
		UIMenuState menu;
		menu.Kind = UI_MENU_GAME_TYPE;
		menu.Title = "Select Game Type";
		menu.Wide = false;
		menu.Items.push_back(UIMenuItemType{"Tiberian Sun (Original)", IDC_GAMETYPE_ORIGINAL, true});
		menu.Items.push_back(UIMenuItemType{"Firestorm", IDC_GAMETYPE_FIRESTORM, true});
		menu.Items.push_back(UIMenuItemType{"Main Menu", IDCANCEL, true});

		int chosen = UI_Menu_Dialog(menu);

		ActiveAddOns = 1 << ADDON_BASE_GAME;
		if (chosen == IDC_GAMETYPE_FIRESTORM) {
			Enable_Addon(ADDON_FIRESTORM);
			type = ADDON_FIRESTORM;
		} else if (chosen != IDC_GAMETYPE_ORIGINAL) {
			return(false);
		}

		Set_Required_Addon(type);
		return(true);
	}

	return(true);
}


/// <summary>
/// Rebuilds the installed and active addon sets from the expansion rules files present.
/// </summary>
void Detect_Addons(void)
{
	AvailableAddOns = (1 << ADDON_BASE_GAME);
	ActiveAddOns = (1 << ADDON_BASE_GAME);

	if (CCFileClass(DeploymentConfig.RulesExpansionFile.c_str()).Is_Available() == true) {
		AvailableAddOns |= (1 << ADDON_FIRESTORM);
	}
}


/// <summary>
/// Is the specified addon installed on this machine?
/// Pass ADDON_ANY to ask whether any expansion at all was found.
/// </summary>
/// <returns>bool; Is the addon present?</returns>
bool Addon_Installed(AddonType addon)
{
	if (addon == ADDON_ANY) {
		return((AvailableAddOns & ~(1 << ADDON_BASE_GAME)) != false);
	}

	return((AvailableAddOns & (1 << addon)) != false);
}


/// <summary>
/// Is the specified addon switched on?
/// An addon must be both installed and enabled to count. Pass ADDON_ANY to ask whether any
/// expansion at all is currently in force.
/// </summary>
/// <returns>bool; Is the addon active?</returns>
bool Addon_Enabled(AddonType addon)
{
	if (addon == ADDON_ANY) {
		return((ActiveAddOns & AvailableAddOns & ~(1 << ADDON_BASE_GAME)) != false);
	}
	return((ActiveAddOns & AvailableAddOns & (1 << addon)) != false);
}


/// <summary>
/// Switches on an installed addon.
/// Only an addon that was actually detected can be enabled, so asking for one that is not
/// present is harmless. Pass ADDON_ANY to switch on everything that is installed.
/// </summary>
void Enable_Addon(AddonType addon)
{
	if (addon == ADDON_ANY) {
		ActiveAddOns = AvailableAddOns;
	} else if (addon > ADDON_BASE_GAME) {
		ActiveAddOns |= AvailableAddOns & (1 << addon);
	}
}


/// <summary>
/// Switches an addon back off.
/// Pass ADDON_ANY to drop all the way back to the base game.
/// </summary>
void Disable_Addon(AddonType addon)
{
	if (addon == ADDON_ANY) {
		ActiveAddOns = (1 << ADDON_BASE_GAME);
	} else if (addon > ADDON_BASE_GAME) {
		ActiveAddOns &= ~(1 << addon);
	}
}


/// <summary>
/// Is the specified addon the one the current game demands?
/// </summary>
/// <returns>bool; Is this the required addon?</returns>
bool Is_Required_Addon(AddonType addon)
{
	return(addon == RequiredAddon);
}


/// <summary>
/// Fetches the addon that the current game demands.
/// </summary>
/// <returns>Returns with the required addon.</returns>
AddonType Get_Required_Addon(void)
{
	return(RequiredAddon);
}


/// <summary>
/// Sets the addon that the current game demands.
/// This routine is called when the player picks a game type from the menus, and again when
/// a scenario or a saved game states which addon its rules were built for.
/// </summary>
void Set_Required_Addon(AddonType addon)
{
	RequiredAddon = addon;
}


/// <summary>
/// Fetches the display name of an addon.
/// </summary>
/// <returns>Returns with the localized title text for the addon specified.</returns>
const char * Get_Addon_Title(AddonType addon)
{
	static int _id[ADDON_COUNT] = {
		TXT_SHORT_TITLE,
		TXT_EXPANSION_TITLE,
	};
	return(Fetch_String(_id[addon]));
}
