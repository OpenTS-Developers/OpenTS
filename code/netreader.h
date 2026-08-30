/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstddef>
#include <cstring>
#include <optional>
#include <span>
#include <type_traits>


class NetReaderClass
{
	public:
		explicit NetReaderClass(std::span<std::byte const> data) noexcept;

		std::size_t Offset(void) const noexcept;
		std::size_t Remaining(void) const noexcept;
		bool Empty(void) const noexcept;

		std::optional<std::span<std::byte const>> Take(std::size_t size) noexcept;

		template<typename T>
			requires std::is_trivially_copyable_v<T>
		std::optional<T> Read_Value(void) noexcept
		{
			auto bytes = Take(sizeof(T));
			if (!bytes) {
				return(std::nullopt);
			}

			T value{};
			std::memcpy(&value, bytes->data(), sizeof(value));
			return(value);
		}

	private:
		std::span<std::byte const> Data;
		std::size_t Position;
};
