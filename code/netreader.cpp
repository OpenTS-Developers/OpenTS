/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "netreader.h"


namespace NetPacket
{
	/// <summary>Starts a bounded read over packet bytes.</summary>
	Reader::Reader(std::span<std::byte const> data) noexcept
		: Data(data), Position(0)
	{
	}


	/// <summary>Returns the current read offset.</summary>
	std::size_t Reader::Offset(void) const noexcept
	{
		return(Position);
	}


	/// <summary>Returns the unread byte count.</summary>
	std::size_t Reader::Remaining(void) const noexcept
	{
		return(Data.size() - Position);
	}


	/// <summary>Checks whether all packet bytes were consumed.</summary>
	bool Reader::Empty(void) const noexcept
	{
		return(Remaining() == 0);
	}


	/// <summary>Advances over a bounded span of packet bytes.</summary>
	std::optional<std::span<std::byte const>> Reader::Take(std::size_t size) noexcept
	{
		if (size > Remaining()) {
			return(std::nullopt);
		}

		std::span<std::byte const> bytes = Data.subspan(Position, size);
		Position += size;
		return(bytes);
	}
}
