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

/* $Header: /CounterStrike/SPECIAL.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SPECIAL.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/27/95                                                     *
 *                                                                                             *
 *                  Last Update : August 20, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Fetch_Difficulty -- Fetches the difficulty setting desired.                               *
 *   Fetch_Password -- Prompts for a password entry from client.                               *
 *   PWEditClass::Draw_Text -- Draws password style obscured text.                             *
 *   Special_Dialog -- Handles the special options dialog.                                     *
 *   SpecialClass::Init -- Initialize the special class of options.                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "special.h"

#include "ccini.h"
#include "globals.h"
#include "session.h"

#define	OPTION_WIDTH	236*2
#define	OPTION_HEIGHT	162*2
#define	OPTION_X			((640 - OPTION_WIDTH) / 2)
#define	OPTION_Y			(400 - OPTION_HEIGHT) / 2


/***********************************************************************************************
 * SpecialClass::Init -- Initialize the special class of options.                              *
 *                                                                                             *
 *    This initialization function is required (as opposed to using a constructor) because     *
 *    the SpecialClass is declared as part of a union. A union cannot have a member with a     *
 *    constructor. Other than this anomoly, the function serves the same purpose as a          *
 *    normal constructor.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void SpecialClass::Init(void)
{
	IsShadowGrow = false;
	IsSpeedBuild = false;
	IsFromInstall = false;
	IsCaptureTheFlag = false;
	IsInert = false;
	IsTGrowth = false;
	IsTSpread = true;

	IsMCVDeploy = false;
	IsInitialVeteran = false;
	IsAllianceFixed = false;
	IsHarvesterImmune = false;
	IsFogOfWar = false;
	Bit2_16 = false;
	IsTExplode = false;
	IsDestroyBridges = true;
	IsTiberiumMeteorites = false;
	IsIonStorms = false;
	IsVisceroid = false;
	IsScrapMetal = false;
}


/// <summary>
/// Compares two sets of special option flags.
/// This routine is used by the multiplayer guest logic to notice that the host has changed
/// the game rules since the options were last examined.
/// </summary>
/// <returns>bool; Do both sets specify exactly the same game rules?</returns>
bool SpecialClass::operator==(const SpecialClass &that) const
{
	return(
		(
		IsShadowGrow == that.IsShadowGrow &&
		IsSpeedBuild == that.IsSpeedBuild &&
		IsFromInstall == that.IsFromInstall &&
		IsCaptureTheFlag == that.IsCaptureTheFlag &&
		IsInert == that.IsInert &&
		IsTGrowth == that.IsTGrowth &&
		IsTSpread == that.IsTSpread &&
		IsMCVDeploy == that.IsMCVDeploy &&
		IsInitialVeteran == that.IsInitialVeteran &&
		IsAllianceFixed == that.IsAllianceFixed &&
		IsHarvesterImmune == that.IsHarvesterImmune &&
		IsFogOfWar == that.IsFogOfWar &&
		Bit2_16 == that.Bit2_16 &&
		IsTExplode == that.IsTExplode &&
		IsDestroyBridges == that.IsDestroyBridges &&
		IsTiberiumMeteorites == that.IsTiberiumMeteorites &&
		IsIonStorms == that.IsIonStorms &&
		IsVisceroid == that.IsVisceroid &&
		IsScrapMetal == that.IsScrapMetal
		) ? true : false);
}


/// <summary>
/// Applies the special option flags to the game.
/// This routine is called once the flags have been settled -- at scenario start, and by the
/// multiplayer setup screens after the players have agreed on the game options.
/// </summary>
void SpecialClass::Apply_To_Game(void)
{

}


/// <summary>
/// Prepares the special option flags for a fresh game.
/// This routine is called by the modem host setup before it starts gathering the game
/// options from the player.
/// </summary>
void SpecialClass::Initialize(void)
{

}


/// <summary>
/// Records the special option flags into the scenario file.
/// This routine is used by the map editor when it saves a scenario back out, so that the
/// rules in force can be recovered later by Read_INI.
/// </summary>
/// <param name="ini">The scenario database to record the flags into.</param>
void SpecialClass::Write_INI(CCINIClass &ini) const
{
	static char const * SPECIAL = "SpecialFlags";

	ini.Put_Bool(SPECIAL, "TiberiumGrows", IsTGrowth);
	ini.Put_Bool(SPECIAL, "TiberiumSpreads", IsTSpread);
	ini.Put_Bool(SPECIAL, "TiberiumExplosive", IsTExplode);
	ini.Put_Bool(SPECIAL, "DestroyableBridges", IsDestroyBridges);
	ini.Put_Bool(SPECIAL, "MCVDeploy", IsMCVDeploy);
	ini.Put_Bool(SPECIAL, "InitialVeteran", IsInitialVeteran);
	ini.Put_Bool(SPECIAL, "FixedAlliance", IsAllianceFixed);
	ini.Put_Bool(SPECIAL, "HarvesterImmune", IsHarvesterImmune);
	ini.Put_Bool(SPECIAL, "FogOfWar", IsFogOfWar);
	ini.Put_Bool(SPECIAL, "Inert", IsInert);
	ini.Put_Bool(SPECIAL, "IonStorms", IsIonStorms);
	ini.Put_Bool(SPECIAL, "Meteorites", IsTiberiumMeteorites);
	ini.Put_Bool(SPECIAL, "Visceroids", IsVisceroid);
	ini.Put_Bool(SPECIAL, "ScrapMetal", IsScrapMetal);
}


/// <summary>
/// Fetches the special option flags from the scenario file.
/// This routine is called while the scenario is being read in. The flags that the players
/// negotiate between themselves are only honored here for a solo mission or when the map
/// debugger is active -- a network scenario is not allowed to overrule the game options.
/// </summary>
/// <param name="ini">The scenario database to fetch the flags from.</param>
void SpecialClass::Read_INI(CCINIClass const & ini)
{
	static char const * SPECIAL = "SpecialFlags";

	IsTExplode = ini.Get_Bool(SPECIAL, "TiberiumExplosive", IsTExplode);
	IsMCVDeploy = ini.Get_Bool(SPECIAL, "MCVDeploy", IsMCVDeploy);
	IsInitialVeteran = ini.Get_Bool(SPECIAL, "InitialVeteran", IsInitialVeteran);
	IsIonStorms = ini.Get_Bool(SPECIAL, "IonStorms", IsIonStorms);
	IsTiberiumMeteorites = ini.Get_Bool(SPECIAL, "Meteorites", IsTiberiumMeteorites);
	IsVisceroid = ini.Get_Bool(SPECIAL, "Visceroids", IsVisceroid);

	if (Session.Type == GAME_NORMAL || Debug_Map) {
		IsTGrowth = ini.Get_Bool(SPECIAL, "TiberiumGrows", IsTGrowth);
		IsTSpread = ini.Get_Bool(SPECIAL, "TiberiumSpreads", IsTSpread);
		IsDestroyBridges = ini.Get_Bool(SPECIAL, "DestroyableBridges", IsDestroyBridges);
		IsAllianceFixed = ini.Get_Bool(SPECIAL, "FixedAlliance", IsAllianceFixed);
		IsFogOfWar = ini.Get_Bool(SPECIAL, "FogOfWar", IsFogOfWar);
		IsInert = ini.Get_Bool(SPECIAL, "Inert", IsInert);
		IsHarvesterImmune = ini.Get_Bool(SPECIAL, "HarvesterImmune", IsHarvesterImmune);
		IsScrapMetal = ini.Get_Bool(SPECIAL, "ScrapMetal", IsScrapMetal);
	}
}
