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

/* $Header: /CounterStrike/CONST.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CONST.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 20, 1993                                           *
 *                                                                                             *
 *                  Last Update : September 20, 1993   [JLB]                                   *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "coord.h"
#include "globals.h"


char const * LandName[LAND_COUNT] = {
	"Clear",
	"Road",
	"Water",
	"Rock",
	"Wall",
	"Tiberium",
	"Beach",
	"Rough",
	"Ice",
	"Railroad",
	"Tunnel",
	"Weeds"
};


char const * SpeedName[SPEED_COUNT] = {
	"Foot",
	"Track",
	"Wheel",
	"Hover",
	"Winged",
	"Float",
	"Amphibious",
	"Creep"
};


/***************************************************************************
**	These are the text names for the various armor types a unit may possess.
*/
char const * const ArmorName[ARMOR_COUNT] = {
	"none",
	"wood",
	"light",
	"heavy",
	"concrete"
};


/***************************************************************************
**	Relative coordinate offsets from the center of a cell for each
**	of the legal positions that an object in a cell may stop at. Only infantry
**	are allowed to stop at other than the center of the cell.
*/
Coord const StoppingCoordAbs[5] = {
	Coord(CELL_LEPTON_W / 2,     CELL_LEPTON_H / 2, 0),     // center
	Coord(CELL_LEPTON_W / 4,     CELL_LEPTON_H / 4, 0),     // upper left
	Coord(3 * CELL_LEPTON_W / 4, CELL_LEPTON_H / 4, 0),     // upper right
	Coord(CELL_LEPTON_W / 4,     3 * CELL_LEPTON_H / 4, 0), // lower left
	Coord(3 * CELL_LEPTON_W / 4, 3 * CELL_LEPTON_H / 4, 0)  // lower right
};


char const * const CrateNames[CRATE_COUNT] = {
	"Money",
	"Unit",
	"HealBase",
	"Cloak",
	"Explosion",
	"Napalm",
	"Squad",
	"Darkness",
	"Reveal",
	"Armor",
	"Speed",
	"Firepower",
	"ICBM",
	"Invulnerability",
	"Veteran",
	"IonStorm",
	"Gas",
	"Tiberium",
	"Pod"
};
