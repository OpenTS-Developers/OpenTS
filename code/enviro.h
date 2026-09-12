/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "win.h"

#include "diff.hh"


class SaveStreamClass;

class EnvironmentClass
{
	public:
		EnvironmentClass(void);
		~EnvironmentClass(void);

		void Store(void);
		void Restore(void);

		bool Load(SaveStreamClass & stream);
		bool Save(SaveStreamClass & stream);

		void Serialize(SaveStreamClass & stream);

	//private:
	public:
		/*
		 * These are the scenario global flags as they stood when the previous mission ended.
		 * They are handed back to the new scenario as it starts, which is how a campaign
		 * remembers what the player did in an earlier mission.
		 */
		bool Globals[50];

		/*
		 * This is the money the player still had when the previous mission was won. The new
		 * mission grants whatever share of it the scenario allows, up to its carry over cap.
		 */
		int CarryOverMoney;

		/*
		 * This is the mission timer as it stood when the previous mission ended. It is resumed
		 * in the new mission only if that scenario asks to inherit the timer.
		 */
		int MissionTimer;

		/*
		 * This is the difficulty handicap the player was under when the previous mission ended.
		 * It remains part of the serialized carry over record, while each scenario assigns the
		 * house handicap from its own difficulty as its houses are read.
		 */
		DiffType Difficulty;

		/*
		 * This is the campaign stage the player had reached, so that a branching campaign
		 * picks up at the same node of the map selection screen it left off at.
		 */
		unsigned short Stage;
};

extern EnvironmentClass Environment;
