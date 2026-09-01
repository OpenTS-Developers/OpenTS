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

/* $Header: /CounterStrike/SPECIAL.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SPECIAL.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 02/27/95                                                     *
 *                                                                                             *
 *                  Last Update : February 27, 1995 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

class CCINIClass;

class SpecialClass
{
	public:

		/*
		**	This initializes all members just like a constructor. A constructor
		**	cannot be used for this class because it is part of a union.
		*/
		void Init(void);

		bool operator==(const SpecialClass &that) const;

		void Apply_To_Game(void);
		void Initialize(void);

		void Read_INI(CCINIClass const & ini);
		void Write_INI(CCINIClass & ini) const;

		/*
		**	If the shroud should regenerated, then this flag will be true.
		*/
		unsigned IsShadowGrow:1;

		/*
		**	Controls the speedy build option -- used for testing.
		*/
		unsigned IsSpeedBuild:1;

		/*
		**	If from install, then play the special installation movie and
		**	skip asking them what type of game they want to play.
		*/
		unsigned IsFromInstall:1;

		/*
		**	If capture the flag mode is on, this flag will be true. With this
		**	flag enabled, then the flag is initially placed at the start of
		**	the scenario.
		*/
		unsigned IsCaptureTheFlag:1;

		/*
		**	This flags controls whether weapons are inert. An inert weapon doesn't do any
		**	damage. Effectively, if this is true, then the units never die.
		*/
		unsigned IsInert:1;

		/*
		**	If Tiberium is allowed to spread and grow, then these flags will be true.
		**	These are duplicated from the rules.ini file and also controlled by the
		**	multiplayer dialog box.
		*/
		unsigned IsTGrowth:1;
		unsigned IsTSpread:1;

		/*
		**	If this flag is true, then the construction yard can undeploy back into an MCV.
		*/
		unsigned IsMCVDeploy:1;

		/*
		 * If the units a player starts the game with are to be built as veterans, then this
		 * flag will be true.
		 */
		unsigned IsInitialVeteran:1;

		/*
		 * If the players are locked into the alliances they began with, then this flag will
		 * be true. It is set when the alliances were dictated from outside the game -- by
		 * clan squads on Westwood Online -- rather than agreed to in the game options.
		 */
		unsigned IsAllianceFixed:1;

		/*
		 * If the harvester "truce" is in effect, then this flag will be true. Harvesters
		 * cannot then be targeted or caught in a blast, so that a player's economy cannot
		 * be attacked directly.
		 */
		unsigned IsHarvesterImmune:1;

		/*
		 * If the shroud grows back into "fog" over ground nobody is watching, then this
		 * flag will be true. Objects left behind in the fog are remembered and drawn as
		 * they were last seen.
		 */
		unsigned IsFogOfWar:1;

		/*
		 * This is one of the game option flags recorded with the scenario, but nothing in
		 * the game sets it or reads it, so what option it stands for is not known.
		 */
		unsigned Bit2_16:1;

		/*
		 * If Tiberium is to detonate when it is hit, then this flag will be true. Nothing
		 * in the game consults it.
		 */
		unsigned IsTExplode:1;

		/*
		 * If bridges can be brought down by weapon fire, then this flag will be true. Only
		 * a warhead that is able to destroy walls will bring one down.
		 */
		unsigned IsDestroyBridges:1;

		/*
		 * If meteorites are to fall during the game and seed fresh Tiberium, then this flag
		 * will be true. Nothing in the game consults it.
		 */
		unsigned IsTiberiumMeteorites:1;

		/*
		 * If ion storms are to sweep across the map during the game, then this flag will be
		 * true. Nothing in the game consults it.
		 */
		unsigned IsIonStorms:1;

		/*
		 * If visceroids are to spawn from infantry that die in Tiberium, then this flag
		 * will be true. Nothing in the game consults it.
		 */
		unsigned IsVisceroid:1;

		/*
		 * A bit field has no address to hand to the stream, so each option makes the trip
		 * in an ordinary variable and is assigned back afterwards. The assignment is
		 * harmless while saving.
		 */
		template<typename S>
		static bool Serialize_Flag(S & stream, unsigned field)
		{
			bool flag = (field != 0);
			stream.Serialize(flag);
			return(flag);
		}

		// Carries the option flags to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			IsShadowGrow = Serialize_Flag(stream, IsShadowGrow);
			IsSpeedBuild = Serialize_Flag(stream, IsSpeedBuild);
			IsFromInstall = Serialize_Flag(stream, IsFromInstall);
			IsCaptureTheFlag = Serialize_Flag(stream, IsCaptureTheFlag);
			IsInert = Serialize_Flag(stream, IsInert);
			IsTGrowth = Serialize_Flag(stream, IsTGrowth);
			IsTSpread = Serialize_Flag(stream, IsTSpread);
			IsMCVDeploy = Serialize_Flag(stream, IsMCVDeploy);
			IsInitialVeteran = Serialize_Flag(stream, IsInitialVeteran);
			IsAllianceFixed = Serialize_Flag(stream, IsAllianceFixed);
			IsHarvesterImmune = Serialize_Flag(stream, IsHarvesterImmune);
			IsFogOfWar = Serialize_Flag(stream, IsFogOfWar);
			Bit2_16 = Serialize_Flag(stream, Bit2_16);
			IsTExplode = Serialize_Flag(stream, IsTExplode);
			IsDestroyBridges = Serialize_Flag(stream, IsDestroyBridges);
			IsTiberiumMeteorites = Serialize_Flag(stream, IsTiberiumMeteorites);
			IsIonStorms = Serialize_Flag(stream, IsIonStorms);
			IsVisceroid = Serialize_Flag(stream, IsVisceroid);
		}
};
