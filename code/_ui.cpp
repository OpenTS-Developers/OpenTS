/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "_ui.h"

#include "ui/rml/rmlfile.h"
#include "ui/rml/rmlrender.h"
#include "ui/rml/rmlsystem.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"

#include <memory>


// The one UI shell, over the engine's window, renderer and file search chain. Its
// constructor stores what it is given and touches none of it, so the order in which the
// program's globals are built does not matter to it.
UIShellClass UIShell(UI_Engine_Host(),
					 std::make_unique<UIRmlSystemClass>(UI_Engine_Host()),
					 std::make_unique<UIRmlFileClass>(),
					 std::make_unique<UIRmlBgfxRenderClass>());
