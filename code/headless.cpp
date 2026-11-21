/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "headless.h"

#include "dbgprint.h"
#include "except.h"
#include "sdl/sdlwindow.h"
#include "win.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>


namespace Headless
{
	namespace {

		Task Requested = {};
		bool IsRequested = false;
		char const * ConflictingTask = nullptr;
		bool IsConsoleAttached = false;
		int ExitCode = EXIT_FAILURE;
		char FailureText[_MAX_PATH + 256] = "";


		/// <summary>
		/// Stops every dialog the run could show, and connects stderr to the console the game
		/// was started from, when there is one.
		/// </summary>
		void Enter_Unattended(void)
		{
			Exception_Set_Unattended(true);
			Main_Window_Set_Unattended(true);

			if (AttachConsole(ATTACH_PARENT_PROCESS)) {
				FILE * stream = NULL;
				IsConsoleAttached = freopen_s(&stream, "CONOUT$", "w", stderr) == 0;
			}
		}
	}


	/// <summary>
	/// Makes this run headless and names the task it performs. Call it while the command line
	/// is parsed, so that a failure later in startup shows no dialog either. Requesting a
	/// second, different task is refused by Check_Request.
	/// </summary>
	void Request(Task const & task)
	{
		if (!IsRequested) {
			Requested = task;
			IsRequested = true;
			Enter_Unattended();
		} else if (task.Run != Requested.Run) {
			ConflictingTask = task.Name;
		}
	}


	bool Is_Active(void)
	{
		return(IsRequested);
	}


	/// <summary>
	/// Refuses a headless request that names two tasks, or one whose options are incomplete,
	/// reporting why.
	/// </summary>
	/// <returns>bool; Can startup go on?</returns>
	bool Check_Request(void)
	{
		if (!IsRequested) {
			return(true);
		}

		if (ConflictingTask != nullptr) {
			Fail("%s and %s cannot run together.", Requested.Name, ConflictingTask);
			return(false);
		}

		return(Requested.Check());
	}


	/// <summary>
	/// Performs the requested task in place of the menu and sets the exit code from its result.
	/// </summary>
	void Run(void)
	{
		ExitCode = Requested.Run() ? EXIT_SUCCESS : EXIT_FAILURE;
	}


	/// <summary>
	/// Records why the run failed, in the debug log and on the console the game was started
	/// from. The run exits with a failure code unless the task goes on to succeed.
	/// </summary>
	void Fail(char const * format, ...)
	{
		char text[_MAX_PATH + 256];

		va_list args;
		va_start(args, format);
		std::vsnprintf(text, sizeof(text), format, args);
		va_end(args);

		ExitCode = EXIT_FAILURE;
		std::snprintf(FailureText, sizeof(FailureText), "%s", text);
		DebugString("%s: %s\n", Requested.Name != nullptr ? Requested.Name : "Headless", text);

		if (IsConsoleAttached) {
			std::fprintf(stderr, "%s\n", text);
			std::fflush(stderr);
		}
	}


	/// <summary>
	/// The process exit code: success only once the task has succeeded.
	/// </summary>
	int Exit_Code(void)
	{
		return(ExitCode);
	}


	/// <summary>
	/// The reason the last Fail gave, or an empty string when nothing failed.
	/// </summary>
	char const * Failure(void)
	{
		return(FailureText);
	}
}
