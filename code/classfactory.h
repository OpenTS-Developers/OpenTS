/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "persist.h"

#include <memory>

// The classes a saved game or a unit type can name by class identifier. Startup
// registers each one; nothing is created for an identifier nobody registered.
using ClassCreatorType = std::unique_ptr<IPersistent> (*)(void);

void Register_Class(ClassID const & classid, ClassCreatorType creator);
void Unregister_Classes(void);
std::unique_ptr<IPersistent> Create_Object(ClassID const & classid);

template<class T>
void Register_Class(ClassID const & classid)
{
	Register_Class(classid, []() -> std::unique_ptr<IPersistent> { return(std::make_unique<T>()); });
}
