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

		void Add_Heading(std::vector<std::string> & lines, char const * name)
		{
			lines.push_back("");
			lines.push_back(std::string("--- ") + name + " ---");
		}

		void Add_Checkpoint_Rows(std::vector<std::string> & lines, char const * name, std::vector<Checkpoint> const & checkpoints)
		{
			Add_Heading(lines, name);
			for (std::size_t index = 0; index < checkpoints.size(); index++) {
				lines.push_back(Format_Row((int)index + 1, checkpoints[index].Label.c_str(), checkpoints[index].CRC));
			}
		}

		template<class T>
		void Add_Type_Rows(std::vector<std::string> & lines, char const * name, DynamicVectorClass<T *> const & heap)
		{
			Add_Heading(lines, name);
			CRCEngine crc;
			for (int index = 0; index < heap.Count(); index++) {
				heap[index]->Compute_CRC(crc);
				lines.push_back(Format_Row(index, heap[index]->Name(), (unsigned int)crc()));
			}
		}

		template<class T>
		void Add_Instance_Rows(std::vector<std::string> & lines, char const * name, DynamicVectorClass<T *> const & heap)
		{
			Add_Heading(lines, name);
			CRCEngine crc;
			for (int index = 0; index < heap.Count(); index++) {
				heap[index]->Compute_CRC(crc);
				char label[24];
				std::snprintf(label, sizeof(label), "ID:%d", heap[index]->Fetch_ID());
				lines.push_back(Format_Row(index, label, (unsigned int)crc()));
			}
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
	/// Replaces the list with the breakdown of the current game state as lines of text: the
	/// scenario and the rules in groups of values, then one row per named type and one row per
	/// identified object. Each row gives the running checksum of its part through that row, so
	/// the first row that differs between two machines narrows a difference to one group of
	/// values, one type or one object. The cells have no rows.
	/// </summary>
	void Compute_Rows(std::vector<std::string> & lines)
	{
		lines.clear();

		std::vector<Checkpoint> checkpoints;
		CRCEngine scenario_crc;
		Scen->Compute_CRC(scenario_crc, &checkpoints);
		Add_Checkpoint_Rows(lines, "Scenario", checkpoints);

		checkpoints.clear();
		CRCEngine rules_crc;
		Rule->Compute_CRC(rules_crc, &checkpoints);
		Add_Checkpoint_Rows(lines, "Rules", checkpoints);

#define TYPE_ROWS(VECTOR) Add_Type_Rows(lines, #VECTOR, VECTOR);

		OBJECT_HEAP_LIST_TYPES(TYPE_ROWS)

#undef TYPE_ROWS

		Add_Type_Rows(lines, "IsometricTileTypes", IsometricTileTypes);

#define INSTANCE_ROWS(VECTOR) Add_Instance_Rows(lines, #VECTOR, VECTOR);

		OBJECT_HEAP_LIST_INSTANCES(INSTANCE_ROWS)

#undef INSTANCE_ROWS
	}


	/// <summary>
	/// Writes the rows of Compute_Rows for the current game state to the debug log.
	/// </summary>
	void Log_Rows(void)
	{
		std::vector<std::string> lines;
		Compute_Rows(lines);
		for (std::string const & line : lines) {
			DebugString("%s\n", line.c_str());
		}
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
	/// Records the game state hash once after Arm_Start and makes it the value this machine's
	/// frame syncs send, so that a network game whose machines loaded different state ends at
	/// its start. The hash and its table go to the debug log. Later calls do nothing until the
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

		ScenarioCRC = StartHash;
		Log_Start();
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
