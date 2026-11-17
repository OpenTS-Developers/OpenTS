/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "crc.h"

#include <cstdio>
#include <string>
#include <vector>

namespace StateHash
{
	// One row of a game state checksum table: a named part of the state, how many records it
	// has, and the checksum of those records.
	struct Part
	{
		std::string Name;
		int Count;
		unsigned int CRC;
	};


	// The two functions below are defined here so that tests/statehash needs no engine code.

	/// <summary>
	/// Returns one checksum for a table of parts. The result depends on the order of the parts
	/// and on each part's record count and checksum, but not on the part names.
	/// </summary>
	inline unsigned int Combine(std::vector<Part> const & parts)
	{
		CRCEngine crc;
		for (Part const & part : parts) {
			crc(part.Count);
			crc((int)part.CRC);
		}
		return((unsigned int)crc());
	}


	/// <summary>
	/// Returns the part as one line of text without a line ending, in the form the out-of-sync
	/// report and the debug log print.
	/// </summary>
	inline std::string Format_Part(Part const & part)
	{
		char line[96];
		std::snprintf(line, sizeof(line), "%-28s count=%-6d crc=%08x", part.Name.c_str(), part.Count, part.CRC);
		return(line);
	}


	// The game as it is now.
	void Compute_Parts(std::vector<Part> & parts);

	// The table taken before the scenario's first frame.
	void Arm_Start(void);
	void Clear_Start(void);
	void Take_Start(void);
	void Print_Start(FILE * fp);
	void Log_Start(void);
}
