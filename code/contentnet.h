/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <memory>
#include <string>


class ContentHttpGet
{
	public:
		ContentHttpGet(void);
		~ContentHttpGet(void);
		ContentHttpGet(ContentHttpGet const &) = delete;
		ContentHttpGet & operator = (ContentHttpGet const &) = delete;

		bool Open(char const * url, char const * user_agent, char const * key, std::uint64_t offset, std::string & error);
		int Read(void * buffer, int capacity, std::string & error);
		void Close(void);
		std::uint64_t Total(void) const;

	private:
		struct State;
		std::unique_ptr<State> Data;
};


bool Content_Pick_Folder(std::string & path);
