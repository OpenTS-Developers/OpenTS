/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "classfactory.h"
#include "dbgprint.h"

#include <vector>

namespace {

struct ClassEntryType {
	CLSID Class;
	ClassCreatorType Creator;
};

std::vector<ClassEntryType> Classes;

}	// namespace


// A later registration of the same identifier wins, as the last class object
// published did before.
void Register_Class(CLSID const & classid, ClassCreatorType creator)
{
	for (ClassEntryType & entry : Classes) {
		if (entry.Class == classid) {
			entry.Creator = creator;
			return;
		}
	}
	Classes.push_back({ classid, creator });
}


void Unregister_Classes(void)
{
	Classes.clear();
}


/// <summary>
/// Creates a new object of the registered class named by the identifier.
/// </summary>
/// <returns>The object, owned by the caller, or nothing with a debug line naming the
/// identifier when no class was registered for it.</returns>
std::unique_ptr<IPersistent> Create_Object(CLSID const & classid)
{
	for (ClassEntryType const & entry : Classes) {
		if (entry.Class == classid) {
			return(entry.Creator());
		}
	}

	DebugString("No class is registered for {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
		(unsigned long)classid.Data1, (unsigned int)classid.Data2, (unsigned int)classid.Data3,
		classid.Data4[0], classid.Data4[1], classid.Data4[2], classid.Data4[3],
		classid.Data4[4], classid.Data4[5], classid.Data4[6], classid.Data4[7]);
	return(nullptr);
}
