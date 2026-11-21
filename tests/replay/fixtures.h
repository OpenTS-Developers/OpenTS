/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ramfile.h"
#include "replay/replayrecords.h"
#include "replay/replaywriter.h"

#include "replaytest.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <vector>


// A recording's first sections, with values in every kind of field a launcher fills.
inline Replay::RecordingInfo Sample_Info(void)
{
	Replay::RecordingInfo info;
	info.Engine = "opents";
	info.EngineVersion = "0.1.0 (test)";
	info.DataVersion = 1;
	info.Package = "Tiberian Sun";
	info.PackageVersion = "2.03";
	info.Files.push_back(Replay::FileHash{ "RULES.INI", "sha256:9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08", {} });
	info.RecordedAt = 1794800000;
	info.Recorder = 0;
	info.MapName = "Synthetic Map";
	info.GameMode = "Battle";
	info.MapPath = "maps/synthetic.map";
	info.Colors.push_back(Replay::PlayerColor{ 0, 0x00FFD700, {} });
	info.Colors.push_back(Replay::PlayerColor{ 3, 0x00C00000, {} });
	return(info);
}


inline Replay::Launch Sample_Launch(void)
{
	Replay::Launch launch;
	launch.Kind = Replay::GameKind::GAME_KIND_SKIRMISH;
	launch.Seed = 12345;

	Replay::Player human;
	human.House = 0;
	human.Name = "Player";
	human.Human = true;
	human.Country = 0;
	human.CountryName = "GDI";
	human.Color = 0;
	human.Start = -1;
	human.Allies = {};
	human.ResolvedCountry = 0;
	human.ResolvedColor = 0;
	human.ResolvedStart = 2;
	launch.Players.push_back(human);

	Replay::Player computer;
	computer.House = 1;
	computer.Country = -1;
	computer.CountryName = "Nod";
	computer.Color = 3;
	computer.Start = 0;
	computer.AiLevel = 1;
	computer.ResolvedCountry = 1;
	computer.ResolvedColor = 3;
	computer.ResolvedStart = 0;
	launch.Players.push_back(computer);

	Replay::GameOption credits;
	credits.Key = "Credits";
	credits.Value = std::int64_t(10000);
	Replay::GameOption crates;
	crates.Key = "Crates";
	crates.Value = true;
	Replay::GameOption speed;
	speed.Key = "GameSpeed";
	speed.Value = std::int64_t(0);
	launch.Options = { crates, credits, speed };
	return(launch);
}


inline ByteVector Sample_Init(void)
{
	return(From_Hex("0a0412345678"));
}


struct SectionEntry {
	std::uint32_t Tag = 0;
	std::uint32_t Codec = 0;
	std::uint64_t Stored = 0;
	std::uint64_t Raw = 0;
	std::size_t Offset = 0;   // of the body
};


inline std::uint64_t Read_LE(ByteVector const & bytes, std::size_t offset, int size)
{
	std::uint64_t value = 0;
	for (int i = 0; i < size; i++) {
		value |= std::uint64_t(bytes[offset + i]) << (8 * i);
	}
	return(value);
}


inline std::string Tag_Name(std::uint32_t tag)
{
	std::string name;
	for (int i = 0; i < 4; i++) {
		name += char(tag >> (8 * i));
	}
	return(name);
}


// Lists the sections of a complete file, independently of the reader. An open section ends the
// list, since its body runs to the end of the file.
inline std::vector<SectionEntry> Walk_Sections(ByteVector const & file)
{
	std::vector<SectionEntry> sections;
	std::size_t offset = 8;
	while (offset + 24 <= file.size()) {
		SectionEntry entry;
		entry.Tag = std::uint32_t(Read_LE(file, offset, 4));
		entry.Codec = std::uint32_t(Read_LE(file, offset + 4, 4));
		entry.Stored = Read_LE(file, offset + 8, 8);
		entry.Raw = Read_LE(file, offset + 16, 8);
		entry.Offset = offset + 24;
		sections.push_back(entry);
		if (entry.Stored == ~0ull) {
			break;
		}
		offset = entry.Offset + std::size_t(entry.Stored);
	}
	return(sections);
}


inline std::string Section_Tags(std::vector<SectionEntry> const & sections)
{
	std::string tags;
	for (SectionEntry const & entry : sections) {
		if (!tags.empty()) {
			tags += ",";
		}
		tags += Tag_Name(entry.Tag);
	}
	return(tags);
}


// A growable RAMFileClass open for writing that also keeps its contents as they stood at the
// last flush, which is what survives a crash.
class MemoryFile : public RAMFileClass
{
	public:
		MemoryFile(void) { Open(WRITE); }

		bool Flush(void) override
		{
			FlushedData = Written();
			return(true);
		}

		ByteVector Written(void) const
		{
			std::span<std::uint8_t const> const contents = Contents();
			return(ByteVector(contents.begin(), contents.end()));
		}

		ByteVector const & Flushed(void) const { return(FlushedData); }

	private:
		ByteVector FlushedData;
};


// A growable RAMFileClass open for reading that reads from a copy of data.
class InputFile : public RAMFileClass
{
	public:
		explicit InputFile(Replay::ByteSpan data)
		{
			Open(WRITE);
			Write(data.data(), int(data.size()));
			Close();
			Open(READ);
		}
};


// Fails the Nth call to Write, Seek, or Flush, counting from 1, and every call after it. A
// write that fails can first store part of its bytes, as a full disk does.
class FailingFile : public MemoryFile
{
	public:
		FailingFile(int fail_at, std::size_t partial) : Calls(0), FailAt(fail_at), Partial(partial) {}

		int Write(void const * buffer, int size) override
		{
			if (!Pass()) {
				MemoryFile::Write(buffer, int(std::min(Partial, std::size_t(size))));
				return(0);
			}
			return(MemoryFile::Write(buffer, size));
		}

		int Seek(int pos, int dir = SEEK_CUR) override
		{
			return(Pass() ? MemoryFile::Seek(pos, dir) : -1);
		}

		bool Flush(void) override
		{
			return(Pass() && MemoryFile::Flush());
		}

		int Calls;

	private:
		bool Pass(void)
		{
			return(++Calls < FailAt);
		}

		int FailAt;
		std::size_t Partial;
};
