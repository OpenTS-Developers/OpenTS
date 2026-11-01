/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "contentnet.h"

#if !defined(_WIN32) && !defined(__APPLE__)

struct ContentHttpGet::State
{
	std::uint64_t TotalBytes = 0;
};


ContentHttpGet::ContentHttpGet(void) :
	Data(std::make_unique<State>())
{
}


ContentHttpGet::~ContentHttpGet(void) = default;


bool ContentHttpGet::Open(char const *, char const *, std::uint64_t, std::string & error)
{
	error = "Downloads are not available on this system.";
	return(false);
}


int ContentHttpGet::Read(void *, int, std::string & error)
{
	error = "Downloads are not available on this system.";
	return(-1);
}


void ContentHttpGet::Close(void)
{
}


std::uint64_t ContentHttpGet::Total(void) const
{
	return(0);
}


bool Content_Pick_Folder(std::string &)
{
	return(false);
}

#endif
