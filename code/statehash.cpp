/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "statehash.h"

#include "_map.h"
#include "_rules.h"
#include "_script.h"
#include "_vanim.h"
#include "_warhead.h"
#include "_weapon.h"
#include "aircraft.h"
#include "airctype.h"
#include "aitrig.h"
#include "anim.h"
#include "animtype.h"
#include "blight.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "cell.h"
#include "crc.h"
#include "dbgprint.h"
#include "empulse.h"
#include "factory.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "infantry.h"
#include "infatype.h"
#include "isotype.h"
#include "light.h"
#include "overtype.h"
#include "particle.h"
#include "partsys.h"
#include "psystype.h"
#include "ptype.h"
#include "rules.h"
#include "scenario.h"
#include "script.h"
#include "side.h"
#include "smudtype.h"
#include "super.h"
#include "suprtype.h"
#include "taction.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforc.h"
#include "team.h"
#include "teamtype.h"
#include "terrain.h"
#include "terrtype.h"
#include "tevent.h"
#include "tiberium.h"
#include "trigger.h"
#include "trigtype.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vanim.h"
#include "vanimtype.h"
#include "warhead.h"
#include "weapon.h"

#include "objheaps.hh"

#include <cstdio>
#include <string>
#include <vector>


namespace StateHash
{
	namespace {
		bool IsDue = false;
		bool IsTaken = false;
		unsigned int StartHash = 0;
		std::vector<Part> StartParts;

		template<class T>
		void Add_Heap_Part(std::vector<Part> & parts, char const * name, DynamicVectorClass<T *> const & heap)
		{
			CRCEngine crc;
			for (int index = 0; index < heap.Count(); index++) {
				heap[index]->Compute_CRC(crc);
			}
			parts.push_back({name, heap.Count(), (unsigned int)crc()});
		}
	}


	/// <summary>
	/// Replaces the list with one checksum per part of the game state: the scenario, the rules,
	/// each object and type heap, the isometric tile types, and the map cells, in that order.
	/// The start state hash and the out-of-sync report are both built from this list.
	/// </summary>
	void Compute_Parts(std::vector<Part> & parts)
	{
		parts.clear();

		CRCEngine scenario_crc;
		Scen->Compute_CRC(scenario_crc);
		parts.push_back({"Scenario", 1, (unsigned int)scenario_crc()});

		CRCEngine rules_crc;
		Rule->Compute_CRC(rules_crc);
		parts.push_back({"Rules", 1, (unsigned int)rules_crc()});

#define HEAP_PART(VECTOR) Add_Heap_Part(parts, #VECTOR, VECTOR);

		OBJECT_HEAP_LIST(HEAP_PART)

#undef HEAP_PART

		Add_Heap_Part(parts, "IsometricTileTypes", IsometricTileTypes);

		CRCEngine cell_crc;
		int cells = 0;
		for (int index = 0; index < Map.Array.Length(); index++) {
			CellClass const * const cell = Map.Array[index];
			if (cell != NULL) {
				cell->Compute_CRC(cell_crc);
				cells++;
			}
		}
		parts.push_back({"Cells", cells, (unsigned int)cell_crc()});
	}


	/// <summary>
	/// Marks a scenario as started, so that the next call to Take_Start records its state.
	/// </summary>
	void Arm_Start(void)
	{
		Clear_Start();
		IsDue = true;
	}


	/// <summary>
	/// Forgets the recorded start state and cancels a pending capture.
	/// </summary>
	void Clear_Start(void)
	{
		IsDue = false;
		IsTaken = false;
		StartHash = 0;
		StartParts.clear();
	}


	/// <summary>
	/// Records the game state hash once after Arm_Start. Later calls do nothing until the
	/// next arm.
	/// </summary>
	/// <remarks>Call this before the first frame's logic runs, so that every machine hashes the
	/// game at the same point.</remarks>
	void Take_Start(void)
	{
		if (!IsDue) {
			return;
		}
		IsDue = false;

		Compute_Parts(StartParts);
		StartHash = Combine(StartParts);
		IsTaken = true;

		DebugString("Start state hash: %08x\n", StartHash);
	}


	/// <summary>
	/// Writes the recorded start state table and its hash, or a line saying none was recorded.
	/// </summary>
	void Print_Start(FILE * fp)
	{
		if (!IsTaken) {
			fprintf(fp, "No start state was recorded for this game.\n");
			return;
		}

		fprintf(fp, "Start state hash: %08x\n", StartHash);
		for (Part const & part : StartParts) {
			fprintf(fp, "%s\n", Format_Part(part).c_str());
		}
	}


	/// <summary>
	/// Writes the same table as Print_Start to the debug log.
	/// </summary>
	void Log_Start(void)
	{
		if (!IsTaken) {
			DebugString("No start state was recorded for this game.\n");
			return;
		}

		DebugString("Start state hash: %08x\n", StartHash);
		for (Part const & part : StartParts) {
			DebugString("%s\n", Format_Part(part).c_str());
		}
	}
}
