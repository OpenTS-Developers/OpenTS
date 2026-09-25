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

bool Game_Window_Handle_Event(WindowEvent const & event);

void Game_Window_On_Paint(bool update_surface);
void Game_Window_On_Right_Mouse_Up(void);
void Game_Window_On_Mouse_Wheel(int delta);
