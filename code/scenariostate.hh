/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

// Why a scenario read stopped.
enum ScenarioState {
	SCENARIO_OK,
	SCENARIO_NOT_READ,
	SCENARIO_TERRAIN_DAMAGED,
};
