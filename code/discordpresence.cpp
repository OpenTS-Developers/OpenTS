/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "discordpresence.h"

#include "dbgprint.h"
#include "discordformat.h"
#include "goptions.h"
#include "headless.h"
#include "addon.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "scenario.h"
#include "session.h"
#include "side.hh"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif


// Public Discord application ID. CMake overrides this with SILICONSUN_DISCORD_APPLICATION_ID.
#ifndef SILICONSUN_DISCORD_APPLICATION_ID
#define SILICONSUN_DISCORD_APPLICATION_ID 1565832094893211698ULL
#endif


namespace {

std::uint64_t const Application_Id = (std::uint64_t)SILICONSUN_DISCORD_APPLICATION_ID;


// Discord answers on a pipe of its own and closes that pipe if a ping goes unread.
// The game thread only hands this thread a new activity, so a missing client cannot stall a frame.
struct Desired {
	int Mode = DISCORD_PRESENCE_MENU;
	std::string Title;
	std::string Kind;
	std::string Map;
	std::string Faction;
	std::int64_t Started = 0;
	unsigned Generation = 0;
};


std::mutex Mutex;
std::condition_variable Wake;
Desired Pending;
std::atomic<bool> Stop{false};
std::thread Worker;
bool Noted_Missing_Id = false;


struct Pipe {
#if defined(_WIN32)
	HANDLE Handle = INVALID_HANDLE_VALUE;
#else
	int Fd = -1;
#endif

	Pipe() = default;
	Pipe(Pipe const &) = delete;
	Pipe & operator=(Pipe const &) = delete;
	~Pipe();
};


void Close_Pipe(Pipe & pipe)
{
#if defined(_WIN32)
	if (pipe.Handle != INVALID_HANDLE_VALUE) {
		CloseHandle(pipe.Handle);
		pipe.Handle = INVALID_HANDLE_VALUE;
	}
#else
	if (pipe.Fd >= 0) {
		::close(pipe.Fd);
		pipe.Fd = -1;
	}
#endif
}


Pipe::~Pipe()
{
	Close_Pipe(*this);
}


bool Pipe_Is_Open(Pipe const & pipe)
{
#if defined(_WIN32)
	return(pipe.Handle != INVALID_HANDLE_VALUE);
#else
	return(pipe.Fd >= 0);
#endif
}


bool Open_Pipe(Pipe & pipe)
{
	Close_Pipe(pipe);
#if defined(_WIN32)
	for (int index = 0; index < 10 && !Stop.load(); index++) {
		wchar_t name[] = L"\\\\?\\pipe\\discord-ipc-0";
		name[(sizeof(name) / sizeof(name[0])) - 2] = (wchar_t)(L'0' + index);
		HANDLE handle = CreateFileW(name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		if (handle != INVALID_HANDLE_VALUE) {
			pipe.Handle = handle;
			return(true);
		}
	}
	return(false);
#else
	char const * roots[5];
	int root_count = 0;
	char const * keys[] = { "XDG_RUNTIME_DIR", "TMPDIR", "TMP", "TEMP" };
	for (char const * key : keys) {
		char const * value = std::getenv(key);
		if (value != nullptr && value[0] != '\0' && root_count < 4) {
			roots[root_count++] = value;
		}
	}
	roots[root_count++] = "/tmp";

	for (int root = 0; root < root_count; root++) {
		for (int index = 0; index < 10; index++) {
			if (Stop.load()) {
				return(false);
			}
			char path[sizeof(sockaddr_un::sun_path)];
			int wrote = std::snprintf(path, sizeof(path), "%s/discord-ipc-%d", roots[root], index);
			if (wrote <= 0 || wrote >= (int)sizeof(path)) {
				continue;
			}
			int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
			if (fd < 0) {
				return(false);
			}
			sockaddr_un address = {};
			address.sun_family = AF_UNIX;
			std::memcpy(address.sun_path, path, (size_t)wrote + 1);
#if defined(__APPLE__)
			address.sun_len = (unsigned char)sizeof(address);
#endif
			if (::connect(fd, (sockaddr const *)&address, sizeof(address)) == 0) {
				pipe.Fd = fd;
				return(true);
			}
			::close(fd);
		}
	}
	return(false);
#endif
}


void Wait_Ms(int milliseconds)
{
	if (milliseconds <= 0) {
		return;
	}
#if defined(_WIN32)
	Sleep((DWORD)milliseconds);
#else
	::poll(nullptr, 0, milliseconds);
#endif
}


// 1 when a byte is waiting, 0 when the pipe is idle, -1 when it has failed.
int Pipe_Available(Pipe & pipe)
{
#if defined(_WIN32)
	DWORD available = 0;
	if (!PeekNamedPipe(pipe.Handle, nullptr, 0, nullptr, &available, nullptr)) {
		return(-1);
	}
	return(available > 0 ? 1 : 0);
#else
	pollfd query = {};
	query.fd = pipe.Fd;
	query.events = POLLIN;
	int result = ::poll(&query, 1, 0);
	if (result < 0) {
		return(errno == EINTR ? 0 : -1);
	}
	if (result == 0) {
		return(0);
	}
	if ((query.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
		return(-1);
	}
	return(1);
#endif
}


bool Wait_Readable(Pipe & pipe, int timeout_ms)
{
	int waited = 0;
	while (waited <= timeout_ms) {
		if (Stop.load()) {
			return(false);
		}
		int available = Pipe_Available(pipe);
		if (available < 0) {
			return(false);
		}
		if (available > 0) {
			return(true);
		}
		if (waited >= timeout_ms) {
			break;
		}
		int step = timeout_ms - waited;
		if (step > 10) {
			step = 10;
		}
		Wait_Ms(step);
		waited += step;
	}
	return(false);
}


int Read_Some(Pipe & pipe, unsigned char * dest, int size)
{
#if defined(_WIN32)
	DWORD got = 0;
	// A message-mode pipe returns the rest of the frame through ERROR_MORE_DATA.
	BOOL ok = ReadFile(pipe.Handle, dest, (DWORD)size, &got, nullptr);
	if (!ok && GetLastError() != ERROR_MORE_DATA) {
		return(-1);
	}
	if (got == 0) {
		return(-1);
	}
	return((int)got);
#else
	for (;;) {
		ssize_t got = ::read(pipe.Fd, dest, (size_t)size);
		if (got < 0 && errno == EINTR) {
			continue;
		}
		return((int)got);
	}
#endif
}


bool Read_Exact(Pipe & pipe, unsigned char * dest, int size, int timeout_ms)
{
	int got = 0;
	while (got < size) {
		if (!Wait_Readable(pipe, timeout_ms)) {
			return(false);
		}
		int piece = Read_Some(pipe, dest + got, size - got);
		if (piece <= 0) {
			return(false);
		}
		got += piece;
		timeout_ms = 500;
	}
	return(true);
}


bool Write_All(Pipe & pipe, unsigned char const * data, int size)
{
	int sent = 0;
	while (sent < size) {
		if (Stop.load()) {
			return(false);
		}
#if defined(_WIN32)
		DWORD wrote = 0;
		if (!WriteFile(pipe.Handle, data + sent, (DWORD)(size - sent), &wrote, nullptr) || wrote == 0) {
			return(false);
		}
		sent += (int)wrote;
#else
		ssize_t wrote = ::write(pipe.Fd, data + sent, (size_t)(size - sent));
		if (wrote < 0) {
			if (errno == EINTR) {
				continue;
			}
			return(false);
		}
		if (wrote == 0) {
			return(false);
		}
		sent += (int)wrote;
#endif
	}
	return(true);
}


bool Write_Pong(Pipe & pipe, unsigned char const * payload, int payload_size)
{
	std::vector<unsigned char> frame((size_t)payload_size + 8);
	int size = Discord_Presence_Pong_Frame(payload, payload_size, frame.data(), (int)frame.size());
	return(size > 0 && Write_All(pipe, frame.data(), size));
}


// Reads one IPC frame. A ping is answered. A close or a broken pipe returns false.
// An ordinary frame returns true.
bool Read_One_Frame(Pipe & pipe, int timeout_ms, bool & got_frame)
{
	got_frame = false;
	unsigned char header[8];
	if (!Read_Exact(pipe, header, 8, timeout_ms)) {
		return(false);
	}
	int opcode = Discord_Presence_Frame_Opcode(header, 8);
	int length = Discord_Presence_Frame_Length(header, 8);
	if (length < 0 || length > 65536) {
		return(false);
	}
	std::vector<unsigned char> payload((size_t)length);
	if (length > 0 && !Read_Exact(pipe, payload.data(), length, 500)) {
		return(false);
	}
	if (opcode == DISCORD_IPC_PING) {
		return(Write_Pong(pipe, payload.data(), length));
	}
	if (opcode == DISCORD_IPC_CLOSE) {
		return(false);
	}
	got_frame = opcode == DISCORD_IPC_FRAME;
	return(true);
}


bool Handshake(Pipe & pipe)
{
	unsigned char frame[80];
	int size = Discord_Presence_Handshake_Frame(Application_Id, frame, (int)sizeof(frame));
	if (size <= 0 || !Write_All(pipe, frame, size)) {
		return(false);
	}
	for (int attempt = 0; attempt < 2; attempt++) {
		bool got_frame = false;
		if (!Read_One_Frame(pipe, 500, got_frame)) {
			return(false);
		}
		if (got_frame) {
			return(true);
		}
	}
	return(false);
}


bool Send_Activity(Pipe & pipe, Desired const & desired, unsigned nonce)
{
	unsigned char frame[1024];
	DiscordPresenceFields fields;
	fields.Mode = desired.Mode;
	fields.Title = desired.Title.c_str();
	fields.Kind = desired.Kind.c_str();
	fields.Map = desired.Map.c_str();
	fields.Faction = desired.Faction.c_str();
	fields.StartedAt = desired.Started;
#if defined(_WIN32)
	long pid = (long)GetCurrentProcessId();
#else
	long pid = (long)::getpid();
#endif
	int size = Discord_Presence_Activity_Frame(fields, nonce, pid, frame, (int)sizeof(frame));
	return(size > 0 && Write_All(pipe, frame, size));
}


bool Service(Pipe & pipe)
{
	int available = Pipe_Available(pipe);
	if (available < 0) {
		return(false);
	}
	if (available == 0) {
		return(true);
	}
	bool got_frame = false;
	return(Read_One_Frame(pipe, 200, got_frame));
}


void Wait_For_Change(unsigned generation, int milliseconds)
{
	std::unique_lock<std::mutex> lock(Mutex);
	Wake.wait_for(lock, std::chrono::milliseconds(milliseconds), [&]() {
		return(Stop.load() || Pending.Generation != generation);
	});
}


void Worker_Main(void)
{
	try {
		Pipe pipe;
		unsigned sent = 0;
		unsigned nonce = 1;
		while (!Stop.load()) {
			Desired desired;
			bool service_only = false;
			{
				std::unique_lock<std::mutex> lock(Mutex);
				if (Stop.load()) {
					break;
				}
				if (Pending.Generation == sent) {
					int wait_ms = Pipe_Is_Open(pipe) ? 50 : 2000;
					Wake.wait_for(lock, std::chrono::milliseconds(wait_ms), [&]() {
						return(Stop.load() || Pending.Generation != sent);
					});
					if (Stop.load()) {
						break;
					}
					service_only = Pending.Generation == sent && Pipe_Is_Open(pipe);
				}
				desired = Pending;
			}

			// With the activity current and the pipe closed, fall through to reopen and resend.
			if (service_only) {
				if (!Service(pipe)) {
					Close_Pipe(pipe);
				}
				continue;
			}

			if (!Pipe_Is_Open(pipe)) {
				if (!Open_Pipe(pipe) || !Handshake(pipe)) {
					Close_Pipe(pipe);
					Wait_For_Change(desired.Generation, 2000);
					continue;
				}
			}
			if (!Send_Activity(pipe, desired, nonce++)) {
				Close_Pipe(pipe);
				Wait_For_Change(desired.Generation, 2000);
				continue;
			}
			sent = desired.Generation;
		}
		Close_Pipe(pipe);
	} catch (...) {
		Stop.store(true);
	}
}


bool May_Run(void)
{
	if (Stop.load() || Headless::Is_Active() || !Options.DiscordPresence) {
		return(false);
	}
	if (Application_Id == 0) {
		if (!Noted_Missing_Id) {
			Noted_Missing_Id = true;
			DebugString("Discord presence is on and the application ID is still 0, so Discord is not contacted.\n");
		}
		return(false);
	}
	return(true);
}


void Publish(int mode, char const * title, char const * kind, char const * map, char const * faction, bool restart_clock)
{
	if (!May_Run()) {
		return;
	}
	if (title == nullptr) {
		title = "";
	}
	if (kind == nullptr) {
		kind = "";
	}
	if (map == nullptr) {
		map = "";
	}
	if (faction == nullptr) {
		faction = "";
	}
	std::lock_guard<std::mutex> lock(Mutex);
	if (Stop.load()) {
		return;
	}
	bool same = Worker.joinable()
		&& Pending.Mode == mode
		&& Pending.Title == title
		&& Pending.Kind == kind
		&& Pending.Map == map
		&& Pending.Faction == faction;
	if (same && !restart_clock) {
		return;
	}
	Pending.Mode = mode;
	Pending.Title = title;
	Pending.Kind = kind;
	Pending.Map = map;
	Pending.Faction = faction;
	Pending.Started = (std::int64_t)std::time(nullptr);
	Pending.Generation += 1;
	if (!Worker.joinable()) {
		Worker = std::thread(Worker_Main);
	}
	Wake.notify_one();
}


void Copy_Trimmed(char * dest, int capacity, char const * text)
{
	if (capacity <= 0) {
		return;
	}
	if (text == nullptr) {
		text = "";
	}
	while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
		text++;
	}
	int count = 0;
	while (text[count] != '\0' && text[count] != '\r' && text[count] != '\n' && count < capacity - 1) {
		dest[count] = text[count];
		count++;
	}
	while (count > 0 && (dest[count - 1] == ' ' || dest[count - 1] == '\t')) {
		count--;
	}
	dest[count] = '\0';
}


void Map_Label(char * dest, int capacity)
{
	if (Scen == nullptr) {
		Copy_Trimmed(dest, capacity, "");
		return;
	}
	Copy_Trimmed(dest, capacity, Scen->Description);
	if (dest[0] != '\0' || Scen->ScenarioName[0] == '\0') {
		return;
	}
	char const * base = std::strrchr(Scen->ScenarioName, '\\');
	if (base == nullptr) {
		base = std::strrchr(Scen->ScenarioName, '/');
	}
	base = base != nullptr ? base + 1 : Scen->ScenarioName;
	Copy_Trimmed(dest, capacity, base);
	char * dot = std::strrchr(dest, '.');
	if (dot != nullptr) {
		*dot = '\0';
	}
}

}


static void Match_Kind(char * dest, int capacity)
{
	if (Session.Type == GAME_NORMAL) {
		Copy_Trimmed(dest, capacity, "Campaign");
		return;
	}

	HouseClass * playing[64];
	int count = 0;
	for (int index = 0; index < Houses.Count() && count < 64; index++) {
		HouseClass * house = Houses[index];
		if (house == nullptr || house->IsObserver || house->IsDefeated || house->Class->IsMultiplayPassive) {
			continue;
		}
		playing[count++] = house;
	}

	std::uint64_t allies[64] = {};
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
			if (playing[i]->Is_Ally(playing[j])) {
				allies[i] |= 1ULL << j;
			}
		}
	}
	Discord_Presence_Match_Label(dest, capacity, allies, count);
	if (dest[0] == '\0') {
		Copy_Trimmed(dest, capacity, Session.Type == GAME_SKIRMISH ? "Skirmish" : "Multiplayer");
	}
}


void Discord_Presence_Show_Menu(void)
{
	Publish(DISCORD_PRESENCE_MENU, "", "", "", "", false);
}


void Discord_Presence_Show_Match(void)
{
	int mode = DISCORD_PRESENCE_CAMPAIGN;
	if (Session.Type == GAME_SKIRMISH) {
		mode = DISCORD_PRESENCE_SKIRMISH;
	} else if (Session.Type != GAME_NORMAL) {
		mode = DISCORD_PRESENCE_MULTIPLAYER;
	}

	char map[128];
	Map_Label(map, (int)sizeof(map));

	char const * faction = "";
	if (Scen != nullptr) {
		if (Scen->PlayerSide == SIDE_GDI) {
			faction = "GDI";
		} else if (Scen->PlayerSide == SIDE_NOD) {
			faction = "Nod";
		}
	}

	char kind[64];
	Match_Kind(kind, (int)sizeof(kind));
	char const * title = Addon_Enabled(ADDON_FIRESTORM) ? "Tiberian Sun - Firestorm" : "Tiberian Sun";
	Publish(mode, title, kind, map, faction, true);
}


void Discord_Presence_Shutdown(void)
{
	std::thread done;
	{
		std::lock_guard<std::mutex> lock(Mutex);
		Stop.store(true);
		done.swap(Worker);
	}
	Wake.notify_all();
	if (done.joinable()) {
		done.join();
	}
}
