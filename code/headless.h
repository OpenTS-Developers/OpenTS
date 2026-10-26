/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#if defined(_WIN32)
#include <sal.h>
#else
#define _Printf_format_string_
#endif


// Runs one task in place of the menu and exits with its result, showing no dialog or console
// and playing no sound. The game window is still created, but never shown.
namespace Headless
{
	struct Task
	{
		char const * Name;		// prefixes the task's log lines
		bool (*Check)(void);	// are the task's options complete? Reports why not.
		bool (*Run)(void);		// performs the task; true when it succeeded
	};

	void Request(Task const & task);
	bool Is_Active(void);
	bool Check_Request(void);
	void Run(void);
	void Fail(_Printf_format_string_ char const * format, ...);
	int Exit_Code(void);
}
