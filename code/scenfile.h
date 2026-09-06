/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

/*
 * The scenario file as it was read, kept while the scenario is played and carried in its
 * save, so that a re-read of the same name is served from here rather than from a file
 * that may have been changed or replaced since.
 */
class ScenarioFileClass
{
	public:
		bool Is_Present(void) const {return(!Bytes.empty());}

		// Names are compared without regard to case; nothing matches while no file is held.
		bool Matches(char const * name) const;

		char const * Name(void) const {return(FileName.c_str());}
		char const * Data(void) const {return(Bytes.data());}
		int Size(void) const {return((int)Bytes.size());}

		// A file with no bytes clears the holder.
		void Assign(char const * name, std::vector<char> && bytes);
		void Clear(void);

		template<typename StreamType>
		void Serialize(StreamType & stream)
		{
			stream.Serialize(FileName);
			stream.Serialize(Bytes);
		}

	private:
		std::string FileName;
		std::vector<char> Bytes;
};
