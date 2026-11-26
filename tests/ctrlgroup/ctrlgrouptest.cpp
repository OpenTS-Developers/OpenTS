/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Tests the control group table: an object is in at most one group, leaves it when removed,
// and passes it to the object that replaces it, and the table writes every member to the
// save stream.
//
// Needs no game data.

#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>


// Replaces the engine's techno, which cannot be compiled without the rest of the
// engine. The table only compares pointers to it.
class TechnoClass
{
	public:
		int Unused;
};

#include "ctrlgroup.h"

#include "doctest/doctest.h"

namespace {

TechnoClass Object[4];


// Writes or reads the raw bytes of numbers and pointers. The engine's save stream writes a
// pointer as an identity it resolves after the load, which this stream does not model.
class ByteStream
{
	public:
		explicit ByteStream(bool saving) : Saving(saving), Cursor(0) {}

		bool Is_Saving(void) const {return(Saving);}
		bool Is_Loading(void) const {return(!Saving);}

		template<typename T> requires std::is_trivially_copyable_v<T>
		void Serialize(T & value)
		{
			unsigned char * bytes = reinterpret_cast<unsigned char *>(&value);
			if (Saving) {
				Bytes.insert(Bytes.end(), bytes, bytes + sizeof(T));
			} else {
				std::memcpy(bytes, Bytes.data() + Cursor, sizeof(T));
				Cursor += sizeof(T);
			}
		}

		template<typename A, typename B>
		void Serialize(std::pair<A, B> & value)
		{
			Serialize(value.first);
			Serialize(value.second);
		}

		template<typename T>
		void Serialize(std::vector<T> & value)
		{
			int count = (int)value.size();
			Serialize(count);
			if (!Saving) {
				value.resize((std::size_t)count);
			}
			for (T & element : value) {
				Serialize(element);
			}
		}

		bool Saving;
		std::size_t Cursor;
		std::vector<unsigned char> Bytes;
};

}


TEST_CASE("an object is in no control group until one is assigned")
{
	ControlGroupsClass groups;
	CHECK_MESSAGE(groups.Group_Of(&Object[0]) == -1, "a fresh table lists no object");

	groups.Assign(&Object[0], 0);
	groups.Assign(&Object[1], ControlGroupsClass::COUNT - 1);
	CHECK_MESSAGE(groups.Group_Of(&Object[0]) == 0, "the first group is group 0");
	CHECK_MESSAGE(groups.Group_Of(&Object[1]) == ControlGroupsClass::COUNT - 1, "the tenth group is group 9");
	CHECK_MESSAGE(groups.Group_Of(&Object[2]) == -1, "an object never assigned stays in no group");
	CHECK_MESSAGE(groups.Group_Of(nullptr) == -1, "no object is in no group");
}


TEST_CASE("assigning another group moves the object out of its old one")
{
	ControlGroupsClass groups;
	groups.Assign(&Object[0], 3);
	groups.Assign(&Object[1], 3);
	groups.Assign(&Object[0], 5);
	CHECK_MESSAGE(groups.Group_Of(&Object[0]) == 5, "the object is in the group assigned last");
	CHECK_MESSAGE(groups.Group_Of(&Object[1]) == 3, "the other member of the old group stays in it");

	groups.Remove(&Object[0]);
	CHECK_MESSAGE(groups.Group_Of(&Object[0]) == -1, "a removed object is in no group");
	CHECK_MESSAGE(groups.Group_Of(&Object[1]) == 3, "removing one object leaves the others");
}


TEST_CASE("removing an object in no group changes nothing")
{
	ControlGroupsClass groups;
	groups.Assign(&Object[0], 2);
	groups.Remove(&Object[1]);
	groups.Remove(nullptr);
	CHECK_MESSAGE(groups.Group_Of(&Object[0]) == 2, "the listed object keeps its group");
}


TEST_CASE("a replacement takes the group of the object it replaces")
{
	ControlGroupsClass groups;
	groups.Assign(&Object[0], 4);
	groups.Transfer(&Object[0], &Object[1]);
	CHECK_MESSAGE(groups.Group_Of(&Object[1]) == 4, "the replacement is in the original's group");
	CHECK_MESSAGE(groups.Group_Of(&Object[0]) == -1, "the original leaves the group");

	groups.Assign(&Object[3], 6);
	groups.Transfer(&Object[2], &Object[3]);
	CHECK_MESSAGE(groups.Group_Of(&Object[3]) == -1, "a replacement for an object in no group is in no group");
	CHECK_MESSAGE(groups.Group_Of(&Object[1]) == 4, "a transfer leaves the other members alone");
}


TEST_CASE("clearing the table empties every group")
{
	ControlGroupsClass groups;
	groups.Assign(&Object[0], 1);
	groups.Assign(&Object[1], 7);
	groups.Clear();
	CHECK_MESSAGE(groups.Group_Of(&Object[0]) == -1, "a cleared table lists no object");
	CHECK_MESSAGE(groups.Group_Of(&Object[1]) == -1, "a cleared table lists no object");
}


TEST_CASE("the control groups write every member to a stream and read the same bytes back")
{
	ControlGroupsClass groups;
	groups.Assign(&Object[0], 0);
	groups.Assign(&Object[2], 9);

	ByteStream out(true);
	groups.Serialize(out);

	ControlGroupsClass loaded;
	ByteStream in(false);
	in.Bytes = out.Bytes;
	loaded.Serialize(in);

	CHECK_MESSAGE(loaded.Group_Of(&Object[0]) == 0, "a member comes back in its group");
	CHECK_MESSAGE(loaded.Group_Of(&Object[2]) == 9, "a member comes back in its group");
	CHECK_MESSAGE(in.Cursor == out.Bytes.size(), "a load reads exactly what the save wrote");
}
