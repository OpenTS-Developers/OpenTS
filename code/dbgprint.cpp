/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#include "always.h"

#include "dbgprint.h"

#include "opents_build.h"
#include "win.h"

#include <shellapi.h>

#include <algorithm>
#include <cerrno>
#include <conio.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define CONSOLE_WINDOW_NAME		"Debug Console"

#ifdef _DEBUG
static char const BuildType[] = "debug";
#else
static char const BuildType[] = "release";
#endif

static char const DebugTruncationNotice[] = "\n*** Log size limit reached. Nothing further will be written to this file. ***\n";

static constexpr size_t DEBUG_MESSAGE_MAX = 4096;
static constexpr unsigned DEBUG_LOG_MAX_AGE_DAYS = 14;
static constexpr unsigned __int64 DEBUG_LOG_MAX_BYTES = 64ui64 * 1024ui64 * 1024ui64;
static constexpr unsigned __int64 DEBUG_LOG_NOTICE_RESERVE = sizeof(DebugTruncationNotice) - 1;
static constexpr unsigned __int64 DEBUG_LOG_BUDGET = DEBUG_LOG_MAX_BYTES - DEBUG_LOG_NOTICE_RESERVE;

static SRWLOCK DebugLock = SRWLOCK_INIT;
static DWORD DebugLockOwner = 0;
static bool DebugInitDone = false;
static bool AtLineStart = true;
static bool ConsoleActive = false;
static HANDLE DebugFile = INVALID_HANDLE_VALUE;
static HANDLE DebugConsole = INVALID_HANDLE_VALUE;
static char DebugDirectory[MAX_PATH];
static char DebugFileName[MAX_PATH];
static unsigned __int64 DebugBytesWritten = 0;

/// <summary>
/// Reports whether the command line asks for the debug console. The game's own parser runs
/// too late to catch the messages written during early startup, so the raw command line is
/// read here instead.
/// </summary>
static bool Command_Line_Requests_Console(void)
{
	int argc = 0;
	LPWSTR * argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argv == NULL) {
		return(false);
	}

	bool requested = false;

	// Index zero is the executable path, which may itself look like an option.
	for (int index = 1; index < argc && !requested; index++) {
		wchar_t const * token = argv[index];

		if (token[0] != L'-' || (token[1] != L'X' && token[1] != L'x')) {
			continue;
		}

		for (wchar_t const * code = token + 2; *code != L'\0'; code++) {
			if (*code == L'C' || *code == L'c') {
				requested = true;
				break;
			}
		}
	}

	LocalFree(argv);
	return(requested);
}


/// <summary>
/// Deletes files matching a pattern that were last written more than the given number of days
/// ago. Directories are never removed.
/// </summary>
/// <param name="directory">Directory to search, without a trailing separator.</param>
/// <param name="pattern">File name pattern, such as "DEBUG_*.LOG".</param>
/// <param name="days">Age threshold in days. Values above 90 are rejected.</param>
/// <returns>True if the directory was searched.</returns>
bool Delete_Files_Older_Than(char const * directory, char const * pattern, unsigned days)
{
	if (directory == NULL || pattern == NULL || days > 90) {
		return(false);
	}

	SYSTEMTIME now;
	FILETIME now_stamp;
	GetSystemTime(&now);
	if (!SystemTimeToFileTime(&now, &now_stamp)) {
		return(false);
	}

	ULARGE_INTEGER cutoff;
	cutoff.LowPart = now_stamp.dwLowDateTime;
	cutoff.HighPart = now_stamp.dwHighDateTime;

	unsigned __int64 const age = (unsigned __int64)days * 24ui64 * 60ui64 * 60ui64 * 10000000ui64;
	if (cutoff.QuadPart < age) {
		return(false);
	}
	cutoff.QuadPart -= age;

	char search[MAX_PATH];
	snprintf(search, sizeof(search), "%s\\%s", directory, pattern);

	WIN32_FIND_DATA found;
	HANDLE search_handle = FindFirstFile(search, &found);
	if (search_handle == INVALID_HANDLE_VALUE) {
		return(false);
	}

	do {
		if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			continue;
		}

		ULARGE_INTEGER written;
		written.LowPart = found.ftLastWriteTime.dwLowDateTime;
		written.HighPart = found.ftLastWriteTime.dwHighDateTime;

		if (written.QuadPart == 0 || written.QuadPart >= cutoff.QuadPart) {
			continue;
		}

		char victim[MAX_PATH];
		snprintf(victim, sizeof(victim), "%s\\%s", directory, found.cFileName);
		DeleteFile(victim);

	} while (FindNextFile(search_handle, &found));

	FindClose(search_handle);
	return(true);
}


/// <summary>
/// Allocates the console and points the standard streams at it. The caller holds the logging
/// lock.
/// </summary>
static void Init_Console_Locked(void)
{
	if (ConsoleActive) {
		return;
	}

	if (!AllocConsole()) {
		return;
	}

	SetConsoleTitle(CONSOLE_WINDOW_NAME);

	// A new console decodes output in the OEM page, and the log lines it shows are UTF-8.
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	// Redirecting the standard streams is what lets ordinary stdio output, such as the
	// command line help, reach the console of a windowed application.
	FILE * stream = NULL;
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
	freopen_s(&stream, "CONIN$", "r", stdin);

	HANDLE output = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	HANDLE input = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (output != INVALID_HANDLE_VALUE) {
		SetStdHandle(STD_OUTPUT_HANDLE, output);
		SetStdHandle(STD_ERROR_HANDLE, output);
	}

	if (input != INVALID_HANDLE_VALUE) {
		SetStdHandle(STD_INPUT_HANDLE, input);
	}

	DebugConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (DebugConsole == NULL) {
		DebugConsole = INVALID_HANDLE_VALUE;
	}

	if (DebugConsole != INVALID_HANDLE_VALUE) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		if (GetConsoleScreenBufferInfo(DebugConsole, &info)) {
			COORD size;
			size.X = info.dwSize.X;
			size.Y = 4096;
			SetConsoleScreenBufferSize(DebugConsole, size);
		}
	}

	// Without this, closing the console window would take the game down with it.
	HWND console_window = GetConsoleWindow();
	if (console_window != NULL) {
		HMENU menu = GetSystemMenu(console_window, FALSE);
		if (menu != NULL) {
			DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
		}
	}

	ConsoleActive = true;
}


static void Write_Banner_Locked(SYSTEMTIME const & started);


/// <summary>
/// Prepares the log directory and this run's log file, then opens the console if this build
/// or the command line asks for it. The caller holds the logging lock. A log that cannot be
/// opened leaves the debugger and console sinks working.
/// </summary>
static void Init_Locked(void)
{
	if (DebugInitDone) {
		return;
	}
	DebugInitDone = true;

	char path_to_exe[MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];

	// The log belongs beside the executable, which is not yet the current directory.
	if (GetModuleFileName(GetModuleHandle(NULL), path_to_exe, sizeof(path_to_exe)) != 0) {
		_splitpath(path_to_exe, drive, dir, NULL, NULL);
		snprintf(DebugDirectory, sizeof(DebugDirectory), "%s%sDebug", drive, dir);
	}

	SYSTEMTIME now;
	GetLocalTime(&now);

	char timestamp[32];
	snprintf(timestamp, sizeof(timestamp), "%02u-%02u-%04u_%02u-%02u-%02u",
				now.wDay, now.wMonth, now.wYear, now.wHour, now.wMinute, now.wSecond);

	if (DebugDirectory[0] != '\0'
		&& (CreateDirectory(DebugDirectory, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)) {

		Delete_Files_Older_Than(DebugDirectory, "DEBUG_*.LOG", DEBUG_LOG_MAX_AGE_DAYS);

		snprintf(DebugFileName, sizeof(DebugFileName), "%s\\DEBUG_%s.LOG", DebugDirectory, timestamp);
		DebugFile = CreateFile(DebugFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
										CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		// A second process started in the same second must not disturb the first one's log.
		if (DebugFile == INVALID_HANDLE_VALUE) {
			snprintf(DebugFileName, sizeof(DebugFileName), "%s\\DEBUG_%s_%lu.LOG",
						DebugDirectory, timestamp, GetCurrentProcessId());
			DebugFile = CreateFile(DebugFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
											CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		}

		if (DebugFile == INVALID_HANDLE_VALUE) {
			DebugFileName[0] = '\0';
		}
	}

#ifdef _DEBUG
	Init_Console_Locked();
#else
	if (Command_Line_Requests_Console()) {
		Init_Console_Locked();
	}
#endif

	// Last, so that the banner heads the log and also reaches a console that has just opened.
	Write_Banner_Locked(now);
}


/// <summary>
/// Writes raw text to every enabled sink. The caller holds the logging lock.
/// </summary>
static void Write_Text_Locked(char const * text, size_t length)
{
	DWORD actual;

	if (DebugFile != INVALID_HANDLE_VALUE) {

		// The notice is paid for out of the reserve, so the file never passes its limit.
		if (DebugBytesWritten + length > DEBUG_LOG_BUDGET) {
			WriteFile(DebugFile, DebugTruncationNotice, (DWORD)DEBUG_LOG_NOTICE_RESERVE, &actual, NULL);
			CloseHandle(DebugFile);
			DebugFile = INVALID_HANDLE_VALUE;
		} else {
			WriteFile(DebugFile, text, (DWORD)length, &actual, NULL);
			DebugBytesWritten += length;
		}
	}

	// Reporting to a debugger that is not there costs an exception round trip per message,
	// which is far more than the rest of this function put together.
	if (IsDebuggerPresent()) {
		OutputDebugString(text);
	}

	if (ConsoleActive && DebugConsole != INVALID_HANDLE_VALUE) {
		WriteConsole(DebugConsole, text, (DWORD)length, &actual, NULL);
	}
}


/// <summary>
/// Writes one finished message, stamping the record prefix when one is due. The caller holds
/// the logging lock.
/// </summary>
static void Write_Message_Locked(char const * buffer, bool with_prefix)
{
	size_t const length = strlen(buffer);
	if (length == 0) {
		return;
	}

	// The prefix identifies a record rather than a call, so it is written once per line: a
	// line assembled from several calls is stamped where it starts. Prefix and message go out
	// together to keep this to one write per call.
	if (with_prefix && AtLineStart) {
		SYSTEMTIME now;
		GetLocalTime(&now);

		char stamped[DEBUG_MESSAGE_MAX + 32];
		int const written = snprintf(stamped, sizeof(stamped), "[%02u:%02u:%02u.%03u] %s",
												now.wHour, now.wMinute, now.wSecond, now.wMilliseconds, buffer);
		if (written > 0) {
			// snprintf reports the length it wanted, which is not what was stored.
			size_t const kept = std::min(size_t(written), sizeof(stamped) - 1);
			Write_Text_Locked(stamped, kept);
			AtLineStart = buffer[length - 1] == '\n';
			return;
		}
	}

	Write_Text_Locked(buffer, length);
	AtLineStart = buffer[length - 1] == '\n';
}


/// <summary>
/// Opens the log with the wordmark and the facts that identify the build and the run. The
/// caller holds the logging lock, so the text goes through the unlocked sink directly;
/// DebugStringNoPrefix would meet the re-entrancy guard and reach the debugger only.
/// </summary>
/// <param name="started">The time this run's log was opened.</param>
static void Write_Banner_Locked(SYSTEMTIME const & started)
{
	// A raw literal keeps the lettering readable, and keeps its backslashes out of the reach of
	// escape processing. It opens on its own line so the rows line up here, which costs a
	// leading newline that the write below steps over.
	static char const Wordmark[] =
R"ART(
  ___                  _____ ____
 / _ \ _ __   ___ _ __|_   _/ ___|
| | | | '_ \ / _ \ '_ \ | | \___ \
| |_| | |_) |  __/ | | || |  ___) |
 \___/| .__/ \___|_| |_||_| |____/
      |_|

)ART";

	Write_Message_Locked(Wordmark + 1, false);

	char line[512];

	snprintf(line, sizeof(line), "Version  : OpenTS %s (%s build)\n", OPENTS_VERSION, BuildType);
	Write_Message_Locked(line, false);

	snprintf(line, sizeof(line), "Commit   : %s on %s%s\n", OPENTS_COMMIT, OPENTS_BRANCH,
				OPENTS_COMMIT_DIRTY ? " (modified)" : "");
	Write_Message_Locked(line, false);

	snprintf(line, sizeof(line), "Committed: %s\n", OPENTS_COMMIT_DATE);
	Write_Message_Locked(line, false);

	snprintf(line, sizeof(line), "Started  : %04u-%02u-%02u %02u:%02u:%02u\n",
				started.wYear, started.wMonth, started.wDay,
				started.wHour, started.wMinute, started.wSecond);
	Write_Message_Locked(line, false);

	// Windows answers GetVersionEx with 6.2 for want of a compatibility manifest, so the real
	// build number has to come from RtlGetVersion.
	char system[64] = "unknown";
	HMODULE ntdll = GetModuleHandle("ntdll.dll");
	if (ntdll != NULL) {
		typedef LONG (WINAPI * RtlGetVersionType)(PRTL_OSVERSIONINFOW);
		RtlGetVersionType const rtl_get_version =
			(RtlGetVersionType)GetProcAddress(ntdll, "RtlGetVersion");

		if (rtl_get_version != NULL) {
			RTL_OSVERSIONINFOW version = { 0 };
			version.dwOSVersionInfoSize = sizeof(version);
			if (rtl_get_version(&version) == 0) {
				snprintf(system, sizeof(system), "Windows %lu.%lu.%lu",
							version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber);
			}
		}
	}

	snprintf(line, sizeof(line), "System   : %s\n", system);
	Write_Message_Locked(line, false);

	// Windows before 10 version 1903 ignores the manifest's request for UTF-8.
	snprintf(line, sizeof(line), "Codepage : ANSI %u, OEM %u\n", GetACP(), GetOEMCP());
	Write_Message_Locked(line, false);

	// The arguments only. The executable path usually carries the account name, and re-joining
	// the arguments loses the shell's original quoting, which a diagnostic can live without.
	char options[256] = "(none)";
	int argc = 0;
	LPWSTR * argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argv != NULL) {
		size_t used = 0;
		for (int index = 1; index < argc; index++) {
			int const written = snprintf(options + used, sizeof(options) - used, "%s%ls",
													used == 0 ? "" : " ", argv[index]);
			if (written <= 0 || size_t(written) >= sizeof(options) - used) {
				break;
			}
			used += size_t(written);
		}
		LocalFree(argv);
	}

	snprintf(line, sizeof(line), "Options  : %s\n", options);
	Write_Message_Locked(line, false);

	Write_Message_Locked("--------------------------------------------------------------------------------\n", false);
}


/// <summary>
/// Takes the logging lock and reports one finished message.
/// </summary>
static void Emit(char const * buffer, bool with_prefix)
{
	DWORD const self = GetCurrentThreadId();

	// A fault raised inside a logging call brings the handler back here on the same thread,
	// where taking the lock again would deadlock. Such a message reaches the debugger only.
	if (DebugLockOwner == self) {
		if (IsDebuggerPresent()) {
			OutputDebugString(buffer);
		}
		return;
	}

	AcquireSRWLockExclusive(&DebugLock);
	DebugLockOwner = self;

	Init_Locked();
	Write_Message_Locked(buffer, with_prefix);

	DebugLockOwner = 0;
	ReleaseSRWLockExclusive(&DebugLock);
}


/// <summary>
/// Runs first time initialisation under the logging lock.
/// </summary>
static void Init_Once(bool with_console)
{
	AcquireSRWLockExclusive(&DebugLock);
	DebugLockOwner = GetCurrentThreadId();

	Init_Locked();
	if (with_console) {
		Init_Console_Locked();
	}

	DebugLockOwner = 0;
	ReleaseSRWLockExclusive(&DebugLock);
}


/// <summary>
/// Prepares the debug log and, when the build or the command line asks for it, the debug
/// console. Logging works without this call, but calling it early fixes the log's timestamp
/// at process start and puts the console up before the first message.
/// </summary>
void Debug_Init(void)
{
	Init_Once(false);
}


/// <summary>
/// Opens the debug console if it is not open already.
/// </summary>
void Debug_Init_Console(void)
{
	Init_Once(true);
}


/// <summary>
/// Waits for a keypress when the debug console is open, so that text written just before the
/// process exits stays readable. Does nothing when there is no console.
/// </summary>
void Debug_Console_Hold(void)
{
	if (!ConsoleActive) {
		return;
	}

	DebugString("Press any key to close this window.\n");
	_getch();
}


/// <summary>
/// Returns the full path of this run's debug log, or an empty string when no log could be
/// opened. Intended for startup code and user interface text; never call it from a crash
/// handler, because it takes the logging lock.
/// </summary>
char const * Debug_Log_File_Name(void)
{
	Init_Once(false);
	return(DebugFileName);
}


/// <summary>
/// Returns the folder where per-run diagnostic files belong, or an empty string when it could
/// not be created. Shared by callers that write their own files beside the debug log.
/// </summary>
char const * Debug_Directory(void)
{
	Init_Once(false);
	return(DebugDirectory);
}


/// <summary>
/// Reports a formatted message to the debug log, the debugger, and the debug console. A
/// message that starts a line is stamped with the time it was reported.
/// </summary>
/// <param name="string">The printf style format string to report.</param>
void __cdecl DebugString(char const * string, ...)
{
	// Callers report an error and then branch on it, so logging must not disturb it.
	DWORD const last_error = GetLastError();
	int const last_errno = errno;

	char buffer[DEBUG_MESSAGE_MAX];

	va_list va;
	va_start(va, string);
	vsnprintf(buffer, sizeof(buffer), string, va);
	va_end(va);

	Emit(buffer, true);

	errno = last_errno;
	SetLastError(last_error);
}


/// <summary>
/// Reports a formatted message with no identifying prefix, so the text appears exactly as
/// given. Callers use it to continue a line another call began, and for text such as the
/// startup banner that reads better unstamped. It reaches the same places DebugString does.
/// </summary>
/// <param name="string">The printf style format string to report.</param>
void __cdecl DebugStringNoPrefix(char const * string, ...)
{
	DWORD const last_error = GetLastError();
	int const last_errno = errno;

	char buffer[DEBUG_MESSAGE_MAX];

	va_list va;
	va_start(va, string);
	vsnprintf(buffer, sizeof(buffer), string, va);
	va_end(va);

	Emit(buffer, false);

	errno = last_errno;
	SetLastError(last_error);
}


/// <summary>
/// Returns the system message text for a Win32 error code, in a buffer owned by the calling
/// thread.
/// </summary>
/// <param name="error">A code as returned by GetLastError.</param>
char const * Last_Error_Text(unsigned long error)
{
	static thread_local char message_buffer[256];

	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							message_buffer, sizeof(message_buffer), NULL) == 0) {
		message_buffer[0] = '\0';
	}

	return(message_buffer);
}
