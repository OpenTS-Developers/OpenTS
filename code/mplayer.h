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

#pragma once

#include "ui/screens/abort/uiabort.h"
#include "session.h"

template<class T> class DynamicVectorClass;

GameType Select_MPlayer_Game (void);
//void Clear_Listbox(ListClass *list);
void Clear_Vector(DynamicVectorClass <NodeNameType *> *vector);
//void Computer_Message(void);
int Surrender_Dialog(int text);
UIAbortChoice Abort_Dialog(void);
