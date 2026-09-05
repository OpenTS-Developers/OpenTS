/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "_timer.h"
#include "mono.h"


// Connection timing uses its injected clock; the engine timer and debug display stay idle.
TTimerClass<SystemTimerClass> TickCount;
MonoClass Mono;


int SystemTimerClass::operator () (void) const {return(0);}


SystemTimerClass::operator int(void) const {return(0);}


void __cdecl DebugString(char const *, ...) {}


MonoClass::MonoClass(void) {}


MonoClass::~MonoClass(void) {}


void MonoClass::Clear(void) {}


void MonoClass::Set_Cursor(int, int) {}


void __cdecl MonoClass::Printf(char const *, ...) {}
