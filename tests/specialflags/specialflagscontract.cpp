/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds the session options a skirmish or a network game applies at scenario start to their
// contract: the truce and bridge options are copied either way, a skirmish drops the fast
// growth an earlier network match turned on, and no other flag changes.
//
// Needs no game data.

#include "special.h"

#include <cstdio>


namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-64s %s\n", what, condition ? "ok" : "FAILED");
	if (!condition) {
		Failures++;
	}
}


// The flags SpecialClass::Init leaves when the game starts.
SpecialClass Startup_Flags(void)
{
	SpecialClass special{};
	special.IsTSpread = true;
	special.IsDestroyBridges = true;
	return(special);
}


// The flags a network match with the truce on and bridge destruction off leaves behind.
SpecialClass After_Network_Match(void)
{
	SpecialClass special = Startup_Flags();
	special.IsHarvesterImmune = true;
	special.IsDestroyBridges = false;
	special.IsTGrowth = true;
	return(special);
}

}	// namespace


int main(void)
{
	/*
	 * The truce follows the option in both directions.
	 */
	{
		SpecialClass special = Startup_Flags();
		special.Apply_Session_Options(true, true, true);
		Check(special.IsHarvesterImmune, "a skirmish with the truce on applies it");

		special = After_Network_Match();
		special.Apply_Session_Options(false, true, true);
		Check(!special.IsHarvesterImmune, "a skirmish with the truce off clears an earlier truce");

		special = After_Network_Match();
		special.Apply_Session_Options(true, false, false);
		Check(special.IsHarvesterImmune, "a network game with the truce on keeps it");
	}

	/*
	 * Bridge destruction follows the option in both directions.
	 */
	{
		SpecialClass special = Startup_Flags();
		special.Apply_Session_Options(false, false, true);
		Check(!special.IsDestroyBridges, "a skirmish with bridges off turns them off");

		special.Apply_Session_Options(false, true, true);
		Check(special.IsDestroyBridges, "a later skirmish with bridges on turns them back on");

		special = After_Network_Match();
		special.Apply_Session_Options(false, true, true);
		Check(special.IsDestroyBridges, "a skirmish after a match with bridges off turns them on");
	}

	/*
	 * Only a skirmish clears fast growth, and nothing here turns it on.
	 */
	{
		SpecialClass special = After_Network_Match();
		special.Apply_Session_Options(false, true, true);
		Check(!special.IsTGrowth, "a skirmish drops fast growth from an earlier network match");

		special = After_Network_Match();
		special.Apply_Session_Options(true, false, false);
		Check(special.IsTGrowth, "a network game keeps the fast growth its setup turned on");

		special = Startup_Flags();
		special.Apply_Session_Options(false, true, false);
		Check(!special.IsTGrowth, "a network game does not turn fast growth on by itself");
	}

	/*
	 * No other flag changes.
	 */
	{
		SpecialClass special{};
		special.IsShadowGrow = true;
		special.IsSpeedBuild = true;
		special.IsFromInstall = true;
		special.IsCaptureTheFlag = true;
		special.IsInert = true;
		special.IsTSpread = true;
		special.IsMCVDeploy = true;
		special.IsInitialVeteran = true;
		special.IsAllianceFixed = true;
		special.IsFogOfWar = true;
		special.Bit2_16 = true;
		special.IsTExplode = true;
		special.IsTiberiumMeteorites = true;
		special.IsIonStorms = true;
		special.IsVisceroid = true;
		special.IsScrapMetal = true;

		special.Apply_Session_Options(false, false, true);

		Check(special.IsShadowGrow && special.IsSpeedBuild && special.IsFromInstall
			&& special.IsCaptureTheFlag && special.IsInert, "the command-line and mode flags are kept");
		Check(special.IsTSpread && special.IsMCVDeploy && special.IsInitialVeteran
			&& special.IsAllianceFixed && special.IsFogOfWar && special.Bit2_16, "the map and lobby flags are kept");
		Check(special.IsTExplode && special.IsTiberiumMeteorites && special.IsIonStorms
			&& special.IsVisceroid && special.IsScrapMetal, "the remaining flags are kept");
	}

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
