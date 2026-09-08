/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uifile.h"

#include "ccfile.h"

#include <climits>


static Rml::String Base_Name(Rml::String const & path)
{
	size_t start = path.find_last_of("/\\");
	return((start == Rml::String::npos) ? path : path.substr(start + 1));
}


Rml::FileHandle UIFileInterfaceClass::Open(Rml::String const & path)
{
	Rml::String name = Base_Name(path);
	if (name.empty()) {
		return(0);
	}

	CCFileClass * file = new CCFileClass(name.c_str());
	if (!file->Is_Available() || !file->Open(FileClass::READ)) {
		delete file;
		return(0);
	}

	return((Rml::FileHandle)file);
}


void UIFileInterfaceClass::Close(Rml::FileHandle file)
{
	CCFileClass * ccfile = (CCFileClass *)file;
	if (ccfile != NULL) {
		ccfile->Close();
		delete ccfile;
	}
}


size_t UIFileInterfaceClass::Read(void * buffer, size_t size, Rml::FileHandle file)
{
	CCFileClass * ccfile = (CCFileClass *)file;
	if (ccfile == NULL || size == 0) {
		return(0);
	}

	int count = ccfile->Read(buffer, size > INT_MAX ? INT_MAX : (int)size);
	return(count > 0 ? (size_t)count : 0);
}


// The engine reports the position it reached rather than success, so a clamped seek is
// recognized by comparing the two.
bool UIFileInterfaceClass::Seek(Rml::FileHandle file, long offset, int origin)
{
	CCFileClass * ccfile = (CCFileClass *)file;
	if (ccfile == NULL) {
		return(false);
	}

	int target = (int)offset;
	switch (origin) {
		case SEEK_CUR:
			target += ccfile->Seek(0, SEEK_CUR);
			break;

		case SEEK_END:
			target += ccfile->Size();
			break;

		default:
			break;
	}

	return(ccfile->Seek((int)offset, origin) == target);
}


size_t UIFileInterfaceClass::Tell(Rml::FileHandle file)
{
	CCFileClass * ccfile = (CCFileClass *)file;
	if (ccfile == NULL) {
		return(0);
	}

	int position = ccfile->Seek(0, SEEK_CUR);
	return(position > 0 ? (size_t)position : 0);
}


size_t UIFileInterfaceClass::Length(Rml::FileHandle file)
{
	CCFileClass * ccfile = (CCFileClass *)file;
	if (ccfile == NULL) {
		return(0);
	}

	int size = ccfile->Size();
	return(size > 0 ? (size_t)size : 0);
}
