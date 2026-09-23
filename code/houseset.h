/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>

class HouseClass;


constexpr int HOUSE_MAX = 64;

// Only a HouseClass converts, so a country's heap index cannot pass for a house's.
template<std::same_as<HouseClass> H>
int House_Slot(H const * house)
{
	assert(house != nullptr && house->HeapID >= 0 && house->HeapID < HOUSE_MAX);
	return(house->HeapID);
}


/// <summary>
/// One bit per house.
/// </summary>
class HouseSet
{
	public:
		constexpr HouseSet(void) : Bits(0) {}

		template<std::same_as<HouseClass> H>
		bool operator[](H const * house) const {return((Bits & Bit(house)) != 0);}

		template<std::same_as<HouseClass> H>
		void Set(H const * house) {Bits |= Bit(house);}

		template<std::same_as<HouseClass> H>
		void Clear(H const * house) {Bits &= ~Bit(house);}

		void Clear(void) {Bits = 0;}
		bool Any(void) const {return(Bits != 0);}

		HouseSet operator&(HouseSet other) const {return(HouseSet(Bits & other.Bits));}
		HouseSet operator|(HouseSet other) const {return(HouseSet(Bits | other.Bits));}
		HouseSet operator~(void) const {return(HouseSet(~Bits));}
		HouseSet & operator&=(HouseSet other) {Bits &= other.Bits; return(*this);}
		HouseSet & operator|=(HouseSet other) {Bits |= other.Bits; return(*this);}
		bool operator==(HouseSet other) const {return(Bits == other.Bits);}
		bool operator!=(HouseSet other) const {return(Bits != other.Bits);}

		// For debug output only.
		std::uint64_t Raw(void) const {return(Bits);}

		template<typename C>
		void Compute_CRC(C & crc) const
		{
			crc((int)(Bits & 0xFFFFFFFF));
			crc((int)(Bits >> 32));
		}

		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Bits);
		}

	private:
		explicit constexpr HouseSet(std::uint64_t bits) : Bits(bits) {}

		template<std::same_as<HouseClass> H>
		static std::uint64_t Bit(H const * house) {return(std::uint64_t(1) << House_Slot(house));}

		std::uint64_t Bits;
};


/// <summary>
/// One value per house.
/// </summary>
template<typename T>
class HouseArray
{
	public:
		constexpr HouseArray(void) : Values{} {}

		template<std::same_as<HouseClass> H>
		T & operator[](H const * house) {return(Values[House_Slot(house)]);}

		template<std::same_as<HouseClass> H>
		T const & operator[](H const * house) const {return(Values[House_Slot(house)]);}

		void Fill(T const & value) {Values.fill(value);}

		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Values);
		}

	private:
		std::array<T, HOUSE_MAX> Values;
};
