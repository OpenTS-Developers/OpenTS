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

#include "event.h"


// This table is the compressed wire size of each event's data field.
unsigned char EventClass::EventLength[EventClass::LAST_EVENT] = {
	0,                                               // EMPTY
	size_of(EventClass, Data.Target),                // POWERON
	size_of(EventClass, Data.Target),                // POWEROFF
	size_of(EventClass, Data.General),               // ALLY
	size_of(EventClass, Data.MegaMission),           // MEGAMISSION
	size_of(EventClass, Data.MegaMission_F),         // MEGAMISSION_F
	size_of(EventClass, Data.Target),                // IDLE
	size_of(EventClass, Data.Target),                // SCATTER
	0,                                               // DESTRUCT
	size_of(EventClass, Data.Target),                // DEPLOY
	size_of(EventClass, Data.Place),                 // PLACE
	0,                                               // OPTIONS
	size_of(EventClass, Data.General),               // GAMESPEED
	size_of(EventClass, Data.Specific),              // PRODUCE
	size_of(EventClass, Data.Specific),              // SUSPEND
	size_of(EventClass, Data.Specific),              // ABANDON
	size_of(EventClass, Data.Target),                // PRIMARY
	size_of(EventClass, Data.Special),               // SPECIAL_PLACE
	0,                                               // EXIT
	size_of(EventClass, Data.Anim),                  // ANIMATION
	size_of(EventClass, Data.Target),                // REPAIR
	size_of(EventClass, Data.Target),                // SELL
	size_of(EventClass, Data.SellCell),              // SELLCELL
	size_of(EventClass, Data.Options),               // SPECIAL
	0,                                               // FRAMESYNC
	0,                                               // MESSAGE
	size_of(EventClass, Data.FrameInfo.Delay),       // RESPONSE_TIME
	size_of(EventClass, Data.FrameInfo),             // FRAMEINFO
	0,                                               // SAVEGAME
	size_of(EventClass, Data.NavCom),                // ARCHIVE
	size_of(EventClass, Data.Variable.Size),         // ADDPLAYER
	size_of(EventClass, Data.Timing),                // TIMING
	size_of(EventClass, Data.ProcessTime),           // PROCESS_TIME
	0,                                               // PAGEUSER
	size_of(EventClass, Data.General),               // REMOVEPLAYER
	size_of(EventClass, Data.General),               // LATENCYFUDGE
	size_of(EventClass, Data.NetworkReport),         // NETWORK_REPORT
};

char const * EventClass::EventNames[EventClass::LAST_EVENT] = {
	"EMPTY",
	"POWERON",
	"POWEROFF",
	"ALLY",
	"MEGAMISSION",
	"MEGAMISSION_F",
	"IDLE",
	"SCATTER",
	"DESTRUCT",
	"DEPLOY",
	"PLACE",
	"OPTIONS",
	"GAMESPEED",
	"PRODUCE",
	"SUSPEND",
	"ABANDON",
	"PRIMARY",
	"SPECIAL_PLACE",
	"EXIT",
	"ANIMATION",
	"REPAIR",
	"SELL",
	"SELLCELL",
	"SPECIAL",
	"FRAMESYNC",
	"MESSAGE",
	"RESPONSE_TIME",
	"FRAMEINFO",
	"SAVEGAME",
	"ARCHIVE",
	"ADDPLAYER",
	"TIMING",
	"PROCESS_TIME",
	"PAGEUSER",
	"REMOVEPLAYER",
	"LATENCYFUDGE",
	"NETWORK_REPORT",
};
