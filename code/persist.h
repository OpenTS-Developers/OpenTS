/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <comdef.h>

class SaveStreamClass;

// Not a COM interface: it has no identifier, and the loader reaches it by dynamic_cast
// from the IUnknown a class factory hands out.
struct IPersistent : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * classid) = 0;
	virtual HRESULT Load(SaveStreamClass & stream) = 0;
	// Restores what the record could not carry, once the record has been checked; an object
	// takes its place in the map or a side table here, never while its record is still in doubt.
	virtual void Post_Load(void) {}
	virtual HRESULT Save(SaveStreamClass & stream, BOOL cleardirty) = 0;
};
