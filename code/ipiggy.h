/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "iloco.h"


struct IPiggyback
{
	/*
	 * Piggybacks a locomotor onto this one.
	 */
	virtual bool Begin_Piggyback(std::unique_ptr<ILocomotion> carried) = 0;

	/*
	 * Hands the carried locomotor back, or nothing when none is carried.
	 */
	virtual std::unique_ptr<ILocomotion> End_Piggyback(void) = 0;

	/*
	 * Is it ok to end the piggyback process?
	 */
	virtual bool Is_Ok_To_End(void) = 0;

	/*
	 * Fetches piggybacked locomotor class ID.
	 */
	virtual HRESULT Piggyback_CLSID(GUID * classid) = 0;

	/*
	 * Is it currently piggy backing another locomotor?
	 */
	virtual bool Is_Piggybacking(void) = 0;
};


// The piggyback side of a locomotor, or null when it cannot carry one.
inline IPiggyback * Piggyback_Of(ILocomotion * locomotion)
{
	return(dynamic_cast<IPiggyback *>(locomotion));
}
