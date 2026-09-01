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

#include "random.h"
#include "vector.h"
#include "win.h"

#include <new>
#include <objidl.h>

template<typename T>
class TypeList : public DynamicVectorClass<T>
{
		typedef DynamicVectorClass<T> BASECLASS;

	public:
		using BASECLASS::Count;
		using BASECLASS::Add;

		TypeList(void) {}
		TypeList(std::initializer_list<T> list) : BASECLASS(list) {}

		bool Is_In_List(T const & object) const;

		T const Pick(int index) const;
		T const Random_Pick(Random2Class & rand) const;
};


template<typename T>
inline bool TypeList<T>::Is_In_List(T const & object) const
{
	return(BASECLASS::ID(object) != -1);
}


// An empty list picks nothing rather than dividing by zero.
template<typename T>
inline T const TypeList<T>::Pick(int index) const
{
	if (Count() <= 0) {
		return(T());
	}
	return(*this)[(unsigned)index % (unsigned)Count()];
}


// An empty list picks nothing rather than drawing from an empty range.
template<typename T>
inline T const TypeList<T>::Random_Pick(Random2Class & rand) const
{
	if (Count() <= 0) {
		return(T());
	}
	return(*this)[rand((unsigned)0, (unsigned)Count()-1)];
}


