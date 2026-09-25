/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


struct WindowEvent;

bool Game_Window_Open(int width, int height);
void Game_Window_Begin_Shutdown(void);
void Game_Window_Close(void);

void Game_Window_Handle_Event(WindowEvent const & event);
