/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <RmlUi/Core/FileInterface.h>


class UIRmlFileClass : public Rml::FileInterface
{
	public:
		virtual Rml::FileHandle Open(Rml::String const & path) override;
		virtual void Close(Rml::FileHandle file) override;
		virtual size_t Read(void * buffer, size_t size, Rml::FileHandle file) override;
		virtual bool Seek(Rml::FileHandle file, long offset, int origin) override;
		virtual size_t Tell(Rml::FileHandle file) override;
		virtual size_t Length(Rml::FileHandle file) override;
};
