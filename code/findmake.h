/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#pragma once

#include "ccini.h"
#include "typelist.h"

class CCINIClass;

template<class T>
T * TFind_Or_Make(char const * name, DynamicVectorClass<T *> const & vector)
{
	if (strcmpi("<none>", name) == 0) {
		return(NULL);
	}

	if (strcmpi("none", name) == 0) {
		return(NULL);
	}

	for (int index = 0; index < vector.Count(); index++) {
		if (stricmp((const char *)vector[index]->IniName, name) == 0) {
			return(vector[index]);
		}
	}
	return(new T(name));
}


template<class T>
TypeList<T const *> TGet_TypeList(CCINIClass const & ini, char const * section, char const * entry, TypeList<T const *> const & defvalue)
{
	std::string value = ini.Get_String(section, entry);

	if (!value.empty()) {
		TypeList<T const *> list;

		const char * token = strtok(value.data(), ",");

		while (token != NULL && token[0] != '\0') {
			T const *ptr = T::Find_Or_Make(token);

			assert(ptr != NULL);

			if (ptr) {
				list.Add(ptr);
			}

			token = strtok(NULL, ",");
		}
		return(list);
	}

	return(defvalue);
}


template<class T>
T * TGet_Class(CCINIClass const & ini, char const * section, char const * entry, T * defvalue)
{
	char buffer[128];

	if (ini.Get_String(section, entry, "", buffer, sizeof(buffer)) != 0) {
		return(T::Find_Or_Make(buffer));
	}

	return(defvalue);
}


template<class T>
bool TPut_Class(CCINIClass & ini, char const * section, char const * entry, T const * value)
{
	return(ini.Put_String(section, entry, value->Name()));
}
