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



struct IFlyControl
{
	/*
	 * Landing altitude
	 */
	virtual LONG Landing_Altitude(void) = 0;

	/*
	 * Lading direction
	 */
	virtual LONG Landing_Direction(void) = 0;

	/*
	 * Loaded with cargo?
	 */
	virtual BOOL Is_Loaded(void) = 0;

	/*
	 * Does it strafe over the target rather than hover?
	 */
	virtual LONG Is_Strafe(void) = 0;

	/*
	 * Is the aircraft locked into straight flight?
	 */
	virtual LONG Is_Locked(void) = 0;
};
