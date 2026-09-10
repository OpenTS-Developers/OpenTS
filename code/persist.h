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

#include "classid.h"

class SaveStreamClass;

// What a saved game asks of an object it carries: the class identifier the record is
// tagged with, and the record itself.
struct IPersistent
{
	virtual ~IPersistent(void) {}

	virtual ClassID Class_ID(void) const = 0;
	virtual bool Load(SaveStreamClass & stream) = 0;
	// Restores what the record could not carry, once the record has been checked; an object
	// takes its place in the map or a side table here, never while its record is still in doubt.
	virtual void Post_Load(void) {}
	virtual bool Save(SaveStreamClass & stream, bool cleardirty) = 0;
};
