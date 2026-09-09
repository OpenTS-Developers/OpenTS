/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "abstract.h"
#include "loco.h"
#include "swizzle.h"
#include "win.h"

#include <array>
#include <deque>
#include <optional>
#include <source_location>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

class SaveStreamClass;

/*
 * A pointer is worth swizzling only when the object it names announces a swizzle
 * identity as it loads. The two persistent roots do that; nothing else does.
 */
template<typename T>
concept SwizzleTarget = std::is_base_of_v<AbstractClass, std::remove_cv_t<T>>
	|| std::is_base_of_v<LocomotionClass, std::remove_cv_t<T>>;

template<typename T>
concept HasSerializeMember = requires(T & object, SaveStreamClass & stream) {
	object.Serialize(stream);
};

/*
 * A container takes the caller's source location, so that an unresolved pointer names
 * the member it belongs to rather than the loop inside the container.
 */
template<typename T>
concept HasSerializeMemberWhere = requires(T & object, SaveStreamClass & stream, std::source_location const & where) {
	object.Serialize(stream, where);
};


/*
 * This carries one object's members to and from a save game. The same Serialize call
 * reads or writes depending on the mode the stream was opened in, so a class describes
 * its members once and cannot drift between saving and loading.
 *
 * Two rules govern what may be handed to Serialize:
 *
 * A pointer must be serialized where it finally lives. The swizzle manager remembers
 * the address of the slot it has to patch, so a pointer routed through a temporary, or
 * held in storage that is reallocated afterwards, is fixed up over memory that no
 * longer belongs to the object.
 *
 * A union may travel as its raw image only while every alternative is trivially
 * copyable and free of pointers. One that gains a pointer has to be serialized arm by
 * arm, switched on whatever discriminates it.
 */
class SaveStreamClass
{
	public:
		enum ModeType {
			MODE_SAVE,
			MODE_LOAD
		};

		SaveStreamClass(IStream * stream, ModeType mode);

		bool Is_Saving(void) const {return(Mode == MODE_SAVE);}
		bool Is_Loading(void) const {return(Mode == MODE_LOAD);}

		/*
		 * The first stream failure freezes this object and every Serialize that follows
		 * does nothing, so a class lists its members without checking each one and the
		 * caller asks once whether the whole pass worked.
		 */
		HRESULT Result(void) const {return(ErrorCode);}
		bool Was_Error(void) const {return(FAILED(ErrorCode));}

		/*
		 * Stops the pass here. A container that reads back a length no honest save could
		 * have written calls this rather than sizing itself from it.
		 */
		void Fail(void);

		/*
		 * The version stamp of the save being read, or the one being written. A member
		 * added to a later format is serialized only when this reaches the version that
		 * introduced it.
		 */
		unsigned int Version(void) const {return(FormatVersion);}

		/*
		 * The stream underneath, for the sub-objects that are still framed by OLE.
		 */
		IStream * Get_Stream(void) const {return(Stream);}

		/*
		 * Names the record this stream is carrying, so that a pointer which nothing
		 * answers for can be reported against the object that asked for it.
		 */
		void Set_Context(char const * ownertype, SwizzleIDType ownerid = 0)
		{
			OwnerType = ownertype;
			OwnerID = ownerid;
		}

		void Serialize_Bytes(void * data, int length);

		/*
		 * Numbers and enumerations travel as their declared width.
		 */
		template<typename T> requires (std::is_arithmetic_v<T> || std::is_enum_v<T>)
		void Serialize(T & value)
		{
			Serialize_Bytes(&value, sizeof(value));
		}

		/*
		 * A pointer travels as the identity the object it names announces on the way back
		 * in. Loading leaves the slot with the swizzle manager until the object says where
		 * it landed.
		 */
		template<SwizzleTarget T>
		void Serialize(T * & pointer, std::source_location const & where = std::source_location::current())
		{
			SwizzleIDType id = Is_Loading() ? 0 : Swizzler.ID_Of(pointer);
			Serialize_Bytes(&id, sizeof(id));

			if (Is_Loading() && !Was_Error()) {
				Swizzler.Swizzle((void **)&pointer, id, OwnerType, OwnerID, typeid(T).name(), where.file_name(), where.line());
			}
		}

		/*
		 * Anything that describes its own members.
		 */
		template<typename T> requires (HasSerializeMember<T> && !HasSerializeMemberWhere<T>)
		void Serialize(T & object)
		{
			object.Serialize(*this);
		}

		/*
		 * The same, for one that wants the call site to report its elements against.
		 */
		template<typename T> requires HasSerializeMemberWhere<T>
		void Serialize(T & object, std::source_location const & where = std::source_location::current())
		{
			object.Serialize(*this, where);
		}

		/*
		 * Arrays of a fixed size. Numbers go out in one block; anything else is
		 * serialized in place, one element at a time.
		 */
		template<typename T, int N>
		void Serialize(T (&array)[N], std::source_location const & where = std::source_location::current())
		{
			if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
				Serialize_Bytes(array, sizeof(array));
			} else {
				for (int index = 0; index < N; index++) {
					Serialize_Element(array[index], where);
				}
			}
		}

		/*
		 * The standard library's fixed size array travels as the built in one does.
		 */
		template<typename T, std::size_t N>
		void Serialize(std::array<T, N> & value, std::source_location const & where = std::source_location::current())
		{
			if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
				if constexpr (N > 0) {
					Serialize_Bytes(value.data(), (int)(sizeof(T) * N));
				}
			} else {
				for (std::size_t index = 0; index < N; index++) {
					Serialize_Element(value[index], where);
				}
			}
		}

		/*
		 * A vector travels as its length followed by its elements. Loading sizes it in
		 * full before any element registers the slot address it occupies.
		 */
		template<typename T> requires (!std::is_same_v<T, bool>)
		void Serialize(std::vector<T> & value, std::source_location const & where = std::source_location::current())
		{
			int count = (int)value.size();
			Serialize(count);

			if (Is_Loading()) {
				if (count < 0) {
					Fail();
					return;
				}
				value.clear();
				value.resize(count);
			}

			if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
				if (count > 0) {
					Serialize_Bytes(value.data(), (int)(sizeof(T) * count));
				}
			} else {
				for (int index = 0; index < count; index++) {
					Serialize_Element(value[index], where);
				}
			}
		}

		/*
		 * An optional value is a flag followed by the value itself when there is one.
		 */
		template<typename T>
		void Serialize(std::optional<T> & value, std::source_location const & where = std::source_location::current())
		{
			bool present = value.has_value();
			Serialize(present);

			if (Is_Loading()) {
				value.reset();
				if (present) {
					value.emplace();
				}
			}

			if (present) {
				Serialize_Element(*value, where);
			}
		}

		/*
		 * A deque travels as a vector does, and is the safest of these to keep pointers
		 * in, since adding to either end leaves the elements already in it where they are.
		 */
		template<typename T>
		void Serialize(std::deque<T> & value, std::source_location const & where = std::source_location::current())
		{
			int count = (int)value.size();
			Serialize(count);

			if (Is_Loading()) {
				if (count < 0) {
					Fail();
					return;
				}
				value.clear();
				value.resize(count);
			}

			for (int index = 0; index < count; index++) {
				Serialize_Element(value[index], where);
			}
		}

		/*
		 * A vector of bool holds its elements packed and hands out a proxy rather than an
		 * element, so each one is unpacked into an ordinary variable for the trip and put
		 * back afterwards. Assigning it back is harmless while saving.
		 */
		void Serialize(std::vector<bool> & value)
		{
			int count = (int)value.size();
			Serialize(count);

			if (Is_Loading()) {
				if (count < 0) {
					Fail();
					return;
				}
				value.assign((std::size_t)count, false);
			}

			for (int index = 0; index < count; index++) {
				bool element = value[(std::size_t)index];
				Serialize(element);
				value[(std::size_t)index] = element;
			}
		}

		/*
		 * A string travels as its length followed by its characters.
		 */
		void Serialize(std::string & value)
		{
			int count = (int)value.size();
			Serialize(count);

			if (Is_Loading()) {
				if (count < 0) {
					Fail();
					return;
				}
				value.resize(count);
			}

			if (count > 0) {
				Serialize_Bytes(value.data(), count);
			}
		}

		/*
		 * A pair travels as its two halves, in order.
		 */
		template<typename A, typename B>
		void Serialize(std::pair<A, B> & value, std::source_location const & where = std::source_location::current())
		{
			Serialize_Element(value.first, where);
			Serialize_Element(value.second, where);
		}

	private:
		/*
		 * One element of a container, handed the owning call site if it can take one --
		 * a container nested inside another included.
		 */
		template<typename T>
		void Serialize_Element(T & element, std::source_location const & where)
		{
			if constexpr (requires { Serialize(element, where); }) {
				Serialize(element, where);
			} else {
				Serialize(element);
			}
		}

		IStream * Stream;
		ModeType Mode;
		HRESULT ErrorCode;
		unsigned int FormatVersion;

		/*
		 * The record this stream is carrying, named for the swizzle manager's report.
		 * Nothing on the save side needs it.
		 */
		char const * OwnerType;
		SwizzleIDType OwnerID;
};


/*
 * A bit field has no address to hand out, so its value makes the trip in an ordinary
 * variable. Assigning it back is harmless while saving.
 */
#define SERIALIZE_BIT(stream, field) \
	do { \
		bool serialize_bit = ((field) != 0); \
		(stream).Serialize(serialize_bit); \
		(field) = serialize_bit; \
	} while (false)


/*
 * The version stamp of the save game currently being read. Each object builds its own
 * stream inside IPersistStream::Load, which has no way to be told, so the value is left
 * here by the load as a whole.
 */
extern unsigned int LoadedSaveVersion;
