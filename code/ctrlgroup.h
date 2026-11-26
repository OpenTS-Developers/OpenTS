/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class TechnoClass;


// The local player's ten control groups. Only this machine stores them, so the simulation must
// never read them. Remove may be called from the object's destructor: the table compares the
// pointers it lists and never dereferences one.
class ControlGroupsClass
{
	public:
		static constexpr int COUNT = 10;

		// The group the object is in, from 0 to COUNT - 1, or -1.
		int Group_Of(TechnoClass const * object) const
		{
			std::size_t index = Index_Of(object);
			return(index < Members.size() ? Members[index].second : -1);
		}

		// Puts the object in the group, taking it out of any other.
		void Assign(TechnoClass * object, int group)
		{
			assert(object != nullptr && group >= 0 && group < COUNT);
			std::size_t index = Index_Of(object);
			if (index < Members.size()) {
				Members[index].second = group;
			} else {
				Members.emplace_back(object, group);
			}
		}

		void Remove(TechnoClass const * object)
		{
			std::size_t index = Index_Of(object);
			if (index < Members.size()) {
				Members.erase(Members.begin() + index);
			}
		}

		// The replacement takes the original's group and the original leaves it. When the
		// original is in no group, the replacement ends up in none.
		void Transfer(TechnoClass const * from, TechnoClass * to)
		{
			int group = Group_Of(from);
			Remove(from);
			if (group >= 0) {
				Assign(to, group);
			} else {
				Remove(to);
			}
		}

		void Clear(void) {Members.clear();}

		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Members);
		}

	private:
		std::size_t Index_Of(TechnoClass const * object) const
		{
			std::size_t index = 0;
			while (index < Members.size() && Members[index].first != object) {
				index++;
			}
			return(index);
		}

		std::vector<std::pair<TechnoClass *, int>> Members;
};

extern ControlGroupsClass ControlGroups;
