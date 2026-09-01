/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <format>
#include <string_view>


/*
 * A string held in a fixed buffer embedded in the owning object, used for the
 * INI names and art filenames of the type classes. The interface follows
 * std::string wherever the operation makes sense for a bounded buffer.
 */
template<int SIZE>
struct TStringID
{
	static_assert(SIZE > 0, "A TStringID must have room for at least one character.");

	public:
		using value_type = char;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = char &;
		using const_reference = char const &;
		using iterator = char *;
		using const_iterator = char const *;

		TStringID(void) noexcept {StringBuffer[0] = '\0';}
		TStringID(char const * string) noexcept {assign(string);}
		TStringID(std::string_view string) noexcept {assign(string);}
		TStringID(TStringID const & that) = default;

		template<int OTHER>
		explicit TStringID(TStringID<OTHER> const & that) noexcept {assign(that.c_str());}

		TStringID & operator=(TStringID const & that) = default;
		TStringID & operator=(char const * string) noexcept {assign(string); return(*this);}
		TStringID & operator=(std::string_view string) noexcept {assign(string); return(*this);}

		operator char const * (void) const noexcept {return(StringBuffer);}
		operator std::string_view (void) const noexcept {return(View());}
		explicit operator bool (void) const noexcept {return(!empty());}

		char & operator[](int index) noexcept {assert((unsigned)index <= (unsigned)SIZE); return(StringBuffer[index]);}
		char const & operator[](int index) const noexcept {assert((unsigned)index <= (unsigned)SIZE); return(StringBuffer[index]);}

		[[nodiscard]] char const * c_str(void) const noexcept {return(StringBuffer);}
		[[nodiscard]] char const * data(void) const noexcept {return(StringBuffer);}
		[[nodiscard]] char * data(void) noexcept {return(StringBuffer);}

		[[nodiscard]] size_type size(void) const noexcept {return(std::strlen(StringBuffer));}
		[[nodiscard]] bool empty(void) const noexcept {return(StringBuffer[0] == '\0');}
		[[nodiscard]] size_type capacity(void) const noexcept {return((size_type)SIZE);}

		[[nodiscard]] char & front(void) noexcept {assert(!empty()); return(StringBuffer[0]);}
		[[nodiscard]] char const & front(void) const noexcept {assert(!empty()); return(StringBuffer[0]);}
		[[nodiscard]] char & back(void) noexcept {assert(!empty()); return(StringBuffer[size() - 1]);}
		[[nodiscard]] char const & back(void) const noexcept {assert(!empty()); return(StringBuffer[size() - 1]);}

		[[nodiscard]] char * begin(void) noexcept {return(StringBuffer);}
		[[nodiscard]] char * end(void) noexcept {return(StringBuffer + size());}
		[[nodiscard]] char const * begin(void) const noexcept {return(StringBuffer);}
		[[nodiscard]] char const * end(void) const noexcept {return(StringBuffer + size());}

		void clear(void) noexcept {StringBuffer[0] = '\0';}

		/*
		 * A string too long for the buffer is silently truncated to fit, as is
		 * anything appended past the end of it.
		 */
		void assign(std::string_view string) noexcept
		{
			size_type const length = std::min(string.size(), capacity());

			if (length > 0) {
				std::memmove(StringBuffer, string.data(), length);
			}
			StringBuffer[length] = '\0';
		}

		void assign(char const * string) noexcept
		{
			if (string == nullptr) {
				clear();
			} else {
				assign(std::string_view(string));
			}
		}

		void push_back(char character) noexcept
		{
			size_type const length = size();

			if (length < capacity()) {
				StringBuffer[length] = character;
				StringBuffer[length + 1] = '\0';
			}
		}

		void pop_back(void) noexcept
		{
			size_type const length = size();

			if (length > 0) {
				StringBuffer[length - 1] = '\0';
			}
		}

		TStringID & operator+=(std::string_view string) noexcept
		{
			size_type const length = size();
			size_type const count = std::min(string.size(), capacity() - length);

			if (count > 0) {
				std::memmove(StringBuffer + length, string.data(), count);
			}
			StringBuffer[length + count] = '\0';
			return(*this);
		}

		TStringID & operator+=(char character) noexcept {push_back(character); return(*this);}

		/*
		 * These match the way the game looks names up in its INI files, which
		 * ignores case.
		 */
		bool operator==(char const * string) const {return(stricmp(StringBuffer, string) == 0);}
		bool operator!=(char const * string) const {return(stricmp(StringBuffer, string) != 0);}
		bool operator==(TStringID const & that) const {return(stricmp(StringBuffer, that.StringBuffer) == 0);}
		bool operator!=(TStringID const & that) const {return(stricmp(StringBuffer, that.StringBuffer) != 0);}

		[[nodiscard]] bool starts_with(std::string_view string) const noexcept {return(View().starts_with(string));}
		[[nodiscard]] bool ends_with(std::string_view string) const noexcept {return(View().ends_with(string));}
		[[nodiscard]] bool contains(std::string_view string) const noexcept {return(View().find(string) != std::string_view::npos);}
		[[nodiscard]] size_type find(std::string_view string, size_type pos = 0) const noexcept {return(View().find(string, pos));}
		[[nodiscard]] size_type rfind(std::string_view string, size_type pos = std::string_view::npos) const noexcept {return(View().rfind(string, pos));}
		[[nodiscard]] std::string_view substr(size_type pos, size_type count = std::string_view::npos) const {return(View().substr(pos, count));}

		// Carries the string to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(StringBuffer);
		}

	private:
		/*
		 * Built from the buffer directly, since constructing a view from this
		 * object could go through either conversion and would be ambiguous.
		 */
		[[nodiscard]] std::string_view View(void) const noexcept {return(std::string_view(StringBuffer, size()));}

		/*
		 * This is the string storage, embedded in the object rather than allocated -- SIZE
		 * characters plus the slot for the null terminator.
		 */
		char StringBuffer[SIZE + 1];
};


template<int SIZE>
struct std::formatter<TStringID<SIZE>> : std::formatter<std::string_view>
{
	auto format(TStringID<SIZE> const & string, std::format_context & context) const
	{
		return(std::formatter<std::string_view>::format(std::string_view(string.data(), string.size()), context));
	}
};
