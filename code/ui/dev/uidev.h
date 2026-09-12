/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "win.h"

class UIRmlBgfxRenderClass;


// The Dear ImGui developer overlays. The context is created on the first toggle, so a
// build whose developer keys never arm allocates nothing here.
bool UIDev_Active(void);
void UIDev_Toggle(UIRmlBgfxRenderClass const & render);
void UIDev_Tick(void);
void UIDev_Render(UIRmlBgfxRenderClass & render);
void UIDev_Shutdown(UIRmlBgfxRenderClass & render);

// Input reaches the overlays before the documents and the game. A true return means the
// overlays want that message kept from both.
void UIDev_Mouse_Position(int x, int y);
bool UIDev_Mouse_Button(int button, bool down);
bool UIDev_Mouse_Wheel(float delta);
bool UIDev_Key(WPARAM virtualkey, bool down);
bool UIDev_Character(wchar_t unit);
void UIDev_Focus(bool focused);
bool UIDev_Wants_Mouse(void);
