/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "replaytest.h"

#include <string>
#include <vector>


struct SyntheticFile {
	std::string Name;
	ByteVector Bytes;
};

// The synthetic recordings the harness writes to REPLAY_OUT_DIR.
std::vector<SyntheticFile> Synthetic_Files(void);
