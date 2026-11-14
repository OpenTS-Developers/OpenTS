/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the debug logger without the engine or any game data: many threads writing at
// once, the size limit, and the console sink.

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "dbgprint.h"

#include "doctest/doctest.h"

namespace {

constexpr int THREAD_COUNT = 4;
constexpr int MESSAGES_PER_THREAD = 20000;


std::string Log_Directory(void)
{
	char path[MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];

	GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path));
	_splitpath(path, drive, dir, NULL, NULL);

	return(std::string(drive) + dir + "Debug");
}


// Every line this harness writes has the same shape, so a torn or interleaved write shows up
// as a line that does not match.
bool Line_Is_Intact(std::string const & raw)
{
	int thread_id = 0;
	int sequence = 0;
	char tail[64] = { 0 };

	// Every line opens with the "[hh:mm:ss.mmm] " stamp that DebugString writes.
	std::string::size_type const stamp = raw.find("] ");
	if (raw.empty() || raw[0] != '[' || stamp == std::string::npos) {
		return(false);
	}
	std::string const line = raw.substr(stamp + 2);

	if (std::sscanf(line.c_str(), "thread %d message %d %63s", &thread_id, &sequence, tail) == 3) {
		return(thread_id >= 0 && thread_id < THREAD_COUNT
				&& sequence >= 0 && sequence < MESSAGES_PER_THREAD
				&& std::strcmp(tail, "0123456789abcdef0123456789abcdef") == 0);
	}

	if (std::sscanf(line.c_str(), "solo message %d %63s", &sequence, tail) == 2) {
		return(sequence >= 0 && sequence < MESSAGES_PER_THREAD
				&& std::strcmp(tail, "0123456789abcdef") == 0);
	}

	return(false);
}

}	// namespace


// One case, since each check reads the one log this process writes: the count of intact lines
// is every message the solo and threaded runs wrote.
TEST_CASE("the logger keeps up with many threads and writes every record whole, after a banner")
{
	CreateDirectoryA(Log_Directory().c_str(), NULL);

	// The timings below describe the sinks a released game actually runs with, so the console
	// stays shut until they are done.
	Debug_Init();

	std::filesystem::path const log = Debug_Log_File_Path();
	REQUIRE_MESSAGE(!log.empty(), "the log file opened");

	// A record assembled from several calls must come out as one stamped line.
	DebugString("fragment-test:");
	DebugStringNoPrefix(" part1");
	DebugStringNoPrefix(" part2\n");

	// The game loop logs from one thread, so the per-call cost it actually pays is measured
	// on its own. The threaded run that follows is about correctness and throughput.
	{
		std::vector<long long> solo(MESSAGES_PER_THREAD);
		for (int message = 0; message < MESSAGES_PER_THREAD; message++) {
			auto const before = std::chrono::steady_clock::now();
			DebugString("solo message %d 0123456789abcdef\n", message);
			auto const after = std::chrono::steady_clock::now();
			solo[message] = std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();
		}

		long long solo_total = 0;
		for (long long value : solo) {
			solo_total += value;
		}
		std::sort(solo.begin(), solo.end());

		double const solo_mean_us = double(solo_total) / double(solo.size()) / 1000.0;
		double const solo_p999_us = double(solo[size_t(double(solo.size()) * 0.999)]) / 1000.0;

		char line[256];
		std::snprintf(line, sizeof(line),
						"uncontended latency: mean %.2f us, p99.9 %.2f us, worst %.2f us",
						solo_mean_us, solo_p999_us, double(solo.back()) / 1000.0);
		MESSAGE(line);

		CHECK_MESSAGE(solo_mean_us <= 10.0, "uncontended mean call latency within 10 us");
		CHECK_MESSAGE(solo_p999_us <= 1000.0, "uncontended p99.9 call latency within 1 ms");
	}

	std::vector<long long> latencies(THREAD_COUNT * MESSAGES_PER_THREAD);

	auto const started = std::chrono::steady_clock::now();

	std::vector<std::thread> threads;
	for (int index = 0; index < THREAD_COUNT; index++) {
		threads.emplace_back([index, &latencies]() {
			for (int message = 0; message < MESSAGES_PER_THREAD; message++) {
				auto const before = std::chrono::steady_clock::now();
				DebugString("thread %d message %d 0123456789abcdef0123456789abcdef\n", index, message);
				auto const after = std::chrono::steady_clock::now();
				latencies[index * MESSAGES_PER_THREAD + message] =
					std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();
			}
		});
	}

	for (std::thread & thread : threads) {
		thread.join();
	}

	auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
								std::chrono::steady_clock::now() - started).count();

	long long total = 0;
	for (long long value : latencies) {
		total += value;
	}

	std::sort(latencies.begin(), latencies.end());
	double const mean_us = double(total) / double(latencies.size()) / 1000.0;
	double const p999_us = double(latencies[size_t(double(latencies.size()) * 0.999)]) / 1000.0;
	double const worst_us = double(latencies.back()) / 1000.0;

	char summary[512];
	std::snprintf(summary, sizeof(summary),
					"%d threads x %d messages in %lld ms (%.0f messages/second); "
					"latency: mean %.2f us, p99.9 %.2f us, worst %.2f us (worst is informational)",
					THREAD_COUNT, MESSAGES_PER_THREAD, elapsed,
					elapsed > 0 ? double(latencies.size()) * 1000.0 / double(elapsed) : 0.0,
					mean_us, p999_us, worst_us);
	MESSAGE(summary);

	// Under contention the lock serialises the writes, so the useful guarantee is that the
	// logger keeps up in aggregate rather than that any one call stays quick. The tail budget
	// only has to rule out a pathological stall: a shared CI runner preempts a lock holder
	// often enough to put p99.9 into the several-millisecond range with nothing wrong here.
	CHECK_MESSAGE(latencies.size() * 1000 / size_t(elapsed > 0 ? elapsed : 1) >= 20000u,
			"sustains at least 20000 messages per second");
	CHECK_MESSAGE(p999_us <= 50000.0, "contended p99.9 call latency within 50 ms");

	// Every line must have survived intact, and every message must be present.
#ifdef _WIN32
	FILE * handle = _wfopen(log.c_str(), L"rb");
#else
	FILE * handle = std::fopen(log.c_str(), "rb");
#endif
	REQUIRE_MESSAGE(handle != NULL, "the log is readable while the game holds it open");

	int intact = 0;
	int broken = 0;
	int fragments_joined = 0;
	int banner_lines = 0;
	int banner_stamped = 0;
	bool banner_named_version = false;
	bool banner_named_commit = false;
	bool in_banner = true;
	char line[512];
	while (std::fgets(line, sizeof(line), handle) != NULL) {
		std::string text(line);
		while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
			text.pop_back();
		}
		if (text.empty()) {
			continue;
		}

		// The banner heads the log and ends at its rule.
		if (in_banner) {
			if (text.compare(0, 3, "---") == 0) {
				in_banner = false;
				continue;
			}
			banner_lines++;
			if (text[0] == '[') {
				banner_stamped++;
			}
			if (text.compare(0, 8, "Version ") == 0) {
				banner_named_version = true;
			}
			if (text.compare(0, 7, "Commit ") == 0) {
				banner_named_commit = true;
			}
			continue;
		}

		if (text.find("fragment-test:") != std::string::npos) {
			std::string::size_type const stamp = text.find("] ");
			if (stamp != std::string::npos
				&& text.substr(stamp + 2) == "fragment-test: part1 part2") {
				fragments_joined++;
			}
			continue;
		}

		if (Line_Is_Intact(text)) {
			intact++;
		} else {
			broken++;
		}
	}
	std::fclose(handle);

	CHECK_MESSAGE(fragments_joined == 1, "a continued line is one record with one stamp");
	CHECK_MESSAGE(banner_lines > 0, "the log opens with a banner closed by its rule");
	CHECK_MESSAGE(!in_banner, "the log opens with a banner closed by its rule");
	CHECK_MESSAGE(banner_stamped == 0, "the banner lines carry no timestamp");
	CHECK_MESSAGE(banner_named_version, "the banner names the version");
	CHECK_MESSAGE(banner_named_commit, "the banner names the commit");
	CHECK_MESSAGE(broken == 0, "no torn or interleaved lines");
	CHECK_MESSAGE(intact == (THREAD_COUNT + 1) * MESSAGES_PER_THREAD, "every message reached the log");
}


// Pruning must leave a file that is younger than the threshold alone.
TEST_CASE("pruning keeps a recent log and refuses a null directory")
{
	std::filesystem::path const directory = Log_Directory();
	CreateDirectoryW(directory.c_str(), NULL);

	std::filesystem::path const keep = directory / u8"DEBUG_keepme.LOG";
	FILE * young = _wfopen(keep.c_str(), L"wb");
	if (young != NULL) {
		std::fputs("young\n", young);
		std::fclose(young);
	}
	Delete_Files_Older_Than(directory, "DEBUG_.*\\.LOG", 14);
	CHECK(GetFileAttributesW(keep.c_str()) != INVALID_FILE_ATTRIBUTES);
	DeleteFileW(keep.c_str());

	CHECK_MESSAGE(Delete_Files_Older_Than(directory, "DEBUG_.*\\.LOG", 9001), "an absurd age should not be refused anymore");
	CHECK_MESSAGE(!Delete_Files_Older_Than("", "DEBUG_.*\\.LOG", 14), "a null directory is refused");
}


// Last in the file, since it detaches the program from the console it started with.
TEST_CASE("the console sink allocates a console with its close button disabled")
{
	// A console subsystem program already owns a console, which would turn AllocConsole away.
	FreeConsole();
	Debug_Init();
	Debug_Enable_Console();
	DebugString("the console sink is reachable\n");

	HWND const console = GetConsoleWindow();
	REQUIRE_MESSAGE(console != NULL, "a console is allocated after detaching");

	// A missing SC_CLOSE proves the window was set up rather than merely allocated.
	CHECK(GetMenuState(GetSystemMenu(console, FALSE), SC_CLOSE, MF_BYCOMMAND) == UINT(-1));
}
