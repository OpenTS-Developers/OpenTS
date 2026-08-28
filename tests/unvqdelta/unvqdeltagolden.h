/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Recorded from the delta decoders on the supported Win32 target. Regenerate by building the
// harness for Win32 and running it with --emit.
//
// Generated file. Do not hand-edit.

#pragma once

int const DELTA_C1_4X4 = 0;
int const KEY_C4_4X4 = 1;
int const DELTA_C4_4X4 = 2;
int const KEY_C4_4X2 = 3;
int const DELTA_C4_4X2 = 4;
int const TABLE_DELTA_4X4 = 5;
int const TABLE_DELTA_4X2 = 6;
int const TABLE_KEY_4X4 = 7;
int const TABLE_KEY_4X2 = 8;

struct UnVQDeltaCase {
	int Which;
	unsigned long BlocksPerRow;
	unsigned long NumRows;
	unsigned long BufWidth;
	int RowsPerBlock;
	unsigned long long Hash;
};

static UnVQDeltaCase const UnVQDeltaCases[] = {
	{DELTA_C1_4X4, 4ul, 2ul, 16ul, 4, 16745876676405328585ULL},
	{DELTA_C1_4X4, 8ul, 3ul, 32ul, 4, 25963804811604909ULL},
	{DELTA_C1_4X4, 16ul, 4ul, 64ul, 4, 16532413467130163177ULL},
	{DELTA_C1_4X4, 7ul, 1ul, 28ul, 4, 8494350179263150235ULL},

	{KEY_C4_4X4, 4ul, 2ul, 16ul, 4, 3950201892696438866ULL},
	{KEY_C4_4X4, 8ul, 3ul, 32ul, 4, 6092168037277926673ULL},
	{KEY_C4_4X4, 16ul, 4ul, 64ul, 4, 17675574741337503829ULL},
	{KEY_C4_4X4, 7ul, 1ul, 28ul, 4, 3253359328177142839ULL},

	{DELTA_C4_4X4, 4ul, 2ul, 16ul, 4, 15079054060155983208ULL},
	{DELTA_C4_4X4, 8ul, 3ul, 32ul, 4, 3938045626304728319ULL},
	{DELTA_C4_4X4, 16ul, 4ul, 64ul, 4, 6361998877672883446ULL},
	{DELTA_C4_4X4, 7ul, 1ul, 28ul, 4, 15010382440369693751ULL},

	{KEY_C4_4X2, 4ul, 2ul, 16ul, 2, 2475083423483687160ULL},
	{KEY_C4_4X2, 8ul, 3ul, 32ul, 2, 14753882924513352491ULL},
	{KEY_C4_4X2, 16ul, 4ul, 64ul, 2, 8659572642207154005ULL},
	{KEY_C4_4X2, 7ul, 1ul, 28ul, 2, 12885367043690754569ULL},

	{DELTA_C4_4X2, 4ul, 2ul, 16ul, 2, 13175677009034320097ULL},
	{DELTA_C4_4X2, 8ul, 3ul, 32ul, 2, 5767342595967712325ULL},
	{DELTA_C4_4X2, 16ul, 4ul, 64ul, 2, 15957000554265536892ULL},
	{DELTA_C4_4X2, 7ul, 1ul, 28ul, 2, 15269911261028103590ULL},

	{TABLE_DELTA_4X4, 4ul, 2ul, 16ul, 4, 7505733390386066281ULL},
	{TABLE_DELTA_4X4, 8ul, 3ul, 32ul, 4, 4707293248795405949ULL},
	{TABLE_DELTA_4X4, 16ul, 4ul, 64ul, 4, 18337368886862803353ULL},
	{TABLE_DELTA_4X4, 7ul, 1ul, 28ul, 4, 79493863202387643ULL},

	{TABLE_DELTA_4X2, 4ul, 2ul, 16ul, 4, 10166568108056195072ULL},
	{TABLE_DELTA_4X2, 8ul, 3ul, 32ul, 4, 5556587268757378596ULL},
	{TABLE_DELTA_4X2, 16ul, 4ul, 64ul, 4, 18383256628949962997ULL},
	{TABLE_DELTA_4X2, 7ul, 1ul, 28ul, 4, 17422735419458052585ULL},

	{TABLE_KEY_4X4, 4ul, 2ul, 16ul, 4, 13579545109996753257ULL},
	{TABLE_KEY_4X4, 8ul, 3ul, 32ul, 4, 15750183458356141231ULL},
	{TABLE_KEY_4X4, 16ul, 4ul, 64ul, 4, 3396036357653678528ULL},
	{TABLE_KEY_4X4, 7ul, 1ul, 28ul, 4, 11430180194805833923ULL},

	{TABLE_KEY_4X2, 4ul, 2ul, 16ul, 4, 3208014430190845657ULL},
	{TABLE_KEY_4X2, 8ul, 3ul, 32ul, 4, 5568011146175100430ULL},
	{TABLE_KEY_4X2, 16ul, 4ul, 64ul, 4, 13625735497349505144ULL},
	{TABLE_KEY_4X2, 7ul, 1ul, 28ul, 4, 8709584357462039549ULL},
};

int const UnVQDeltaCaseCount = (int)(sizeof(UnVQDeltaCases) / sizeof(UnVQDeltaCases[0]));
