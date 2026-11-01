/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "contentinstall.h"

#include "contentcatalog.h"
#include "contentnet.h"
#include "contentsha256.h"
#include "gamedirs.h"
#include "version.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>


namespace
{

int const Slice_Milliseconds = 80;
int const Chunk = 256 * 1024;
std::uint64_t const Present_Bytes = 10ull * 1024ull * 1024ull;


std::string Data_Path(char const * name)
{
	return(Data_Directory() + name);
}


std::string Find_Source(std::string const & folder, char const * name)
{
	std::error_code error;
	std::filesystem::path root(folder);
	std::filesystem::path direct = root / name;
	if (std::filesystem::is_regular_file(direct, error)) {
		return(direct.string());
	}
	for (char const * sub : { "INSTALL", "Install", "install" }) {
		std::filesystem::path nested = root / sub / name;
		if (std::filesystem::is_regular_file(nested, error)) {
			return(nested.string());
		}
	}
	return(std::string());
}


bool Seek_File(FILE * file, std::uint64_t offset)
{
#if defined(_WIN32)
	return(_fseeki64(file, (__int64)offset, SEEK_SET) == 0);
#else
	return(fseeko(file, (off_t)offset, SEEK_SET) == 0);
#endif
}


std::uint64_t File_Bytes(std::string const & path, bool & found)
{
	std::error_code error;
	auto size = std::filesystem::file_size(path, error);
	found = !error && std::filesystem::is_regular_file(path, error);
	if (!found) {
		return(0);
	}
	return((std::uint64_t)size);
}

}


struct ContentTransfer::State
{
	enum class Phase
	{
		Idle,
		Next,
		Hash,
		Transfer,
		Stopped,
	};

	Phase phase = Phase::Idle;
	bool copy = false;
	bool ok = false;
	bool hashing_final = false;
	std::string folder;
	std::vector<ContentFile const *> files;
	int index = 0;
	FILE * in = nullptr;
	FILE * out = nullptr;
	ContentSha256 sha;
	ContentHttpGet http;
	bool http_open = false;
	std::uint64_t have = 0;
	std::uint64_t done = 0;
	std::uint64_t total = 0;
	std::uint64_t session_bytes = 0;
	std::uint64_t speed_bytes = 0;
	double speed = -1.0;
	std::chrono::steady_clock::time_point slice;
	std::chrono::steady_clock::time_point speed_mark;
	std::string part_path;
	std::string final_path;
	std::string source_path;
	std::string verb;
	std::string status;
	std::string error;
	unsigned char chunk[Chunk];

	void Close_IO(void);
	void Stop(void);
	bool Budget(void) const;
	void Note_Speed(int added);
	void Describe(void);
	void Begin(bool copying, std::string const & source_folder, bool tsmovies, bool firestorm, bool fsmovies);
	void Open_Part(bool truncate);
	bool Connect(void);
	void Step_Next(void);
	void Step_Hash(void);
	void Step_Transfer(void);
	void Finish_File(void);
};


ContentTransfer::ContentTransfer(void) :
	Data(std::make_unique<State>())
{
}


ContentTransfer::~ContentTransfer(void)
{
	if (Data != nullptr) {
		Data->Close_IO();
	}
}


void ContentTransfer::State::Close_IO(void)
{
	if (in != nullptr) {
		std::fclose(in);
		in = nullptr;
	}
	if (out != nullptr) {
		std::fclose(out);
		out = nullptr;
	}
	if (http_open) {
		http.Close();
		http_open = false;
	}
}


void ContentTransfer::State::Stop(void)
{
	Close_IO();
	phase = Phase::Stopped;
}


bool ContentTransfer::State::Budget(void) const
{
	return(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - slice).count() < Slice_Milliseconds);
}


void ContentTransfer::State::Note_Speed(int added)
{
	session_bytes += (std::uint64_t)added;
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - speed_mark).count();
	if (elapsed >= 500) {
		double seconds = (double)elapsed / 1000.0;
		speed = (double)(session_bytes - speed_bytes) / seconds;
		speed_mark = now;
		speed_bytes = session_bytes;
	}
}


void ContentTransfer::State::Describe(void)
{
	if (index < 0 || index >= (int)files.size()) {
		status.clear();
		return;
	}
	ContentFile const * file = files[(std::size_t)index];
	status = verb;
	status += " ";
	status += file->Name;
	status += ": ";
	status += Content_Size_Text(have);
	status += " of ";
	status += Content_Size_Text(file->Size);
	if (speed >= 0.0 && phase == Phase::Transfer) {
		status += ", ";
		status += Content_Speed_Text(speed);
	}
}


void ContentTransfer::State::Begin(bool copying, std::string const & source_folder, bool tsmovies, bool firestorm, bool fsmovies)
{
	Close_IO();
	copy = copying;
	ok = false;
	folder = source_folder;
	files.clear();
	index = 0;
	have = 0;
	done = 0;
	total = 0;
	session_bytes = 0;
	speed_bytes = 0;
	speed = -1.0;
	speed_mark = std::chrono::steady_clock::now();
	error.clear();
	status = "Preparing";
	phase = Phase::Stopped;

	std::string directory = Data_Directory();
	if (!directory.empty()) {
		std::error_code failure;
		std::filesystem::create_directories(directory, failure);
		if (failure) {
			error = "The data folder could not be created.";
			return;
		}
	}

	for (ContentFile const & file : Content_Files()) {
		if (Content_File_Selected(file, tsmovies, firestorm, fsmovies)) {
			files.push_back(&file);
			total += file.Size;
		}
	}
	if (files.empty()) {
		error = "There are no files to install.";
		return;
	}
	phase = Phase::Next;
}


void ContentTransfer::State::Open_Part(bool truncate)
{
	out = std::fopen(part_path.c_str(), truncate ? "w+b" : "r+b");
	if (out == nullptr) {
		error = std::string(files[(std::size_t)index]->Name) + " could not be written.";
		Stop();
		return;
	}
	if (!truncate && !Seek_File(out, have)) {
		error = std::string(files[(std::size_t)index]->Name) + " could not be written.";
		Stop();
	}
}


bool ContentTransfer::State::Connect(void)
{
	ContentFile const * file = files[(std::size_t)index];
	if (copy) {
		in = std::fopen(source_path.c_str(), "rb");
		if (in == nullptr) {
			error = std::string(file->Name) + " could not be read.";
			Stop();
			return(false);
		}
		if (have > 0 && !Seek_File(in, have)) {
			error = std::string(file->Name) + " could not be read.";
			Stop();
			return(false);
		}
		verb = "Copying";
		return(true);
	}

	std::string url = std::string(Content_Download_Root()) + "/files/" + file->Name;
	std::string agent = std::string("siliconsun/") + Version_Name();
	std::string failure;
	if (!http.Open(url.c_str(), agent.c_str(), Content_Download_Key(), have, failure)) {
		error = failure.empty() ? "The download connection failed." : failure;
		Stop();
		return(false);
	}
	http_open = true;
	if (http.Total() != file->Size) {
		error = std::string(file->Name) + " on the server is not the freeware file.";
		Stop();
		return(false);
	}
	verb = "Downloading";
	return(true);
}


void ContentTransfer::State::Step_Next(void)
{
	Close_IO();
	sha.Reset();
	have = 0;
	if (index >= (int)files.size()) {
		ok = true;
		status = "Done";
		Stop();
		return;
	}

	ContentFile const * file = files[(std::size_t)index];
	final_path = Data_Path(file->Name);
	part_path = final_path + ".part";
	source_path.clear();
	verb = "Checking";

	bool final_found = false;
	std::uint64_t final_size = File_Bytes(final_path, final_found);
	if (final_found && final_size != file->Size) {
		error = std::string(file->Name) + " is already in the data folder and does not match the freeware file.";
		Stop();
		return;
	}
	if (final_found) {
		in = std::fopen(final_path.c_str(), "rb");
		if (in == nullptr) {
			error = std::string(file->Name) + " could not be read.";
			Stop();
			return;
		}
		hashing_final = true;
		phase = Phase::Hash;
		return;
	}

	if (copy) {
		source_path = Find_Source(folder, file->Name);
		if (source_path.empty()) {
			error = std::string("The folder does not hold ") + file->Name + ".";
			Stop();
			return;
		}
		bool source_found = false;
		std::uint64_t source_size = File_Bytes(source_path, source_found);
		if (!source_found || source_size != file->Size) {
			error = std::string(file->Name) + " in that folder is not the freeware file.";
			Stop();
			return;
		}
	}

	bool part_found = false;
	std::uint64_t part_size = File_Bytes(part_path, part_found);
	if (part_found && part_size > file->Size) {
		std::error_code failure;
		std::filesystem::remove(part_path, failure);
		part_found = false;
	}
	if (part_found && part_size > 0) {
		in = std::fopen(part_path.c_str(), "r+b");
		if (in == nullptr) {
			error = std::string(file->Name) + " could not be read.";
			Stop();
			return;
		}
		hashing_final = false;
		phase = Phase::Hash;
		return;
	}

	Open_Part(true);
	if (phase == Phase::Stopped) {
		return;
	}
	if (!Connect()) {
		return;
	}
	phase = Phase::Transfer;
}


void ContentTransfer::State::Step_Hash(void)
{
	ContentFile const * file = files[(std::size_t)index];
	std::size_t got = std::fread(chunk, 1, sizeof(chunk), in);
	if (got > 0) {
		sha.Update(chunk, got);
		have += (std::uint64_t)got;
		return;
	}
	if (std::ferror(in)) {
		error = std::string(file->Name) + " could not be read.";
		Stop();
		return;
	}

	// A short partial keeps the hash so the bytes that follow extend it.
	if (have != file->Size) {
		if (hashing_final) {
			std::fclose(in);
			in = nullptr;
			error = std::string(file->Name) + " is already in the data folder and does not match the freeware file.";
			Stop();
			return;
		}
	} else {
		unsigned char digest[32];
		sha.Final(digest);
		bool matches = Content_File_Matches(*file, digest);
		std::fclose(in);
		in = nullptr;
		if (matches) {
			if (!hashing_final) {
				std::error_code failure;
				std::filesystem::rename(part_path, final_path, failure);
				if (failure) {
					error = std::string(file->Name) + " could not be saved.";
					Stop();
					return;
				}
			}
			done += file->Size;
			have = 0;
			index++;
			phase = Phase::Next;
			return;
		}
		if (hashing_final) {
			error = std::string(file->Name) + " is already in the data folder and does not match the freeware file.";
			Stop();
			return;
		}
		std::error_code failure;
		std::filesystem::remove(part_path, failure);
		sha.Reset();
		have = 0;
		Open_Part(true);
		if (phase == Phase::Stopped) {
			return;
		}
		if (!Connect()) {
			return;
		}
		phase = Phase::Transfer;
		return;
	}

	out = in;
	in = nullptr;
	if (!Seek_File(out, have)) {
		error = std::string(file->Name) + " could not be written.";
		Stop();
		return;
	}
	if (!Connect()) {
		return;
	}
	phase = Phase::Transfer;
}


void ContentTransfer::State::Finish_File(void)
{
	ContentFile const * file = files[(std::size_t)index];
	Close_IO();
	unsigned char digest[32];
	sha.Final(digest);
	if (have != file->Size || !Content_File_Matches(*file, digest)) {
		if (have != file->Size && !copy) {
			error = std::string("The download of ") + file->Name + " ended early. It will resume from where it left off.";
		} else {
			std::error_code failure;
			std::filesystem::remove(part_path, failure);
			error = std::string(file->Name) + " did not match the freeware checksum.";
		}
		Stop();
		return;
	}

	std::error_code failure;
	std::filesystem::rename(part_path, final_path, failure);
	if (failure) {
		error = std::string(file->Name) + " could not be saved.";
		Stop();
		return;
	}
	done += file->Size;
	have = 0;
	index++;
	phase = Phase::Next;
}


void ContentTransfer::State::Step_Transfer(void)
{
	ContentFile const * file = files[(std::size_t)index];
	int got = 0;
	if (copy) {
		got = (int)std::fread(chunk, 1, sizeof(chunk), in);
		if (got == 0 && std::ferror(in)) {
			error = std::string(file->Name) + " could not be read.";
			Stop();
			return;
		}
	} else {
		std::string failure;
		got = http.Read(chunk, (int)sizeof(chunk), failure);
		if (got < 0) {
			error = failure.empty() ? "The download connection failed." : failure;
			Stop();
			return;
		}
	}

	if (got == 0) {
		Finish_File();
		return;
	}

	std::size_t wrote = std::fwrite(chunk, 1, (std::size_t)got, out);
	if (wrote != (std::size_t)got) {
		error = std::string(file->Name) + " could not be written.";
		Stop();
		return;
	}
	sha.Update(chunk, (std::size_t)got);
	have += (std::uint64_t)got;
	Note_Speed(got);
}


void ContentTransfer::Start_Download(bool tsmovies, bool firestorm, bool fsmovies)
{
	Data->Begin(false, std::string(), tsmovies, firestorm, fsmovies);
}


void ContentTransfer::Start_Copy(std::string const & folder, bool tsmovies, bool firestorm, bool fsmovies)
{
	Data->Begin(true, folder, tsmovies, firestorm, fsmovies);
}


void ContentTransfer::Cancel(void)
{
	if (Data == nullptr) {
		return;
	}
	Data->Close_IO();
	Data->ok = false;
	Data->error.clear();
	Data->status.clear();
	Data->phase = State::Phase::Stopped;
}


void ContentTransfer::Tick(void)
{
	if (Data == nullptr || Data->phase == State::Phase::Idle || Data->phase == State::Phase::Stopped) {
		return;
	}
	Data->slice = std::chrono::steady_clock::now();
	while (Data->Budget() && Data->phase != State::Phase::Stopped && Data->phase != State::Phase::Idle) {
		switch (Data->phase) {
			case State::Phase::Next:
				Data->Step_Next();
				break;
			case State::Phase::Hash:
				Data->Step_Hash();
				break;
			case State::Phase::Transfer:
				Data->Step_Transfer();
				break;
			case State::Phase::Idle:
			case State::Phase::Stopped:
				break;
		}
	}
	if (Data->phase != State::Phase::Stopped) {
		Data->Describe();
	}
}


bool ContentTransfer::Active(void) const
{
	return(Data != nullptr && Data->phase != State::Phase::Idle && Data->phase != State::Phase::Stopped);
}


bool ContentTransfer::Succeeded(void) const
{
	return(Data != nullptr && Data->ok);
}


int ContentTransfer::Percent(void) const
{
	if (Data == nullptr || Data->total == 0) {
		return(0);
	}
	std::uint64_t have = Data->done + Data->have;
	if (have > Data->total) {
		have = Data->total;
	}
	return((int)((have * 100ull) / Data->total));
}


std::string const & ContentTransfer::Status(void) const
{
	static std::string empty;
	return(Data != nullptr ? Data->status : empty);
}


std::string const & ContentTransfer::Error(void) const
{
	static std::string empty;
	return(Data != nullptr ? Data->error : empty);
}


bool Content_Data_Present(void)
{
	bool found = false;
	return(File_Bytes(Data_Path("TIBSUN.MIX"), found) >= Present_Bytes && found);
}
