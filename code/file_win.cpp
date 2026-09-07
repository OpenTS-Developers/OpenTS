/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2020-2024 Vanilla Conquer contributors
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Vanilla Conquer (https://github.com/TheAssemblyArmada/Vanilla-Conquer).
 * Modified by OpenTS contributors, 2026.
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#ifdef _WIN32
#include "file.h"

#include <windows.h>
#include <io.h>

class Find_File_Data_Win : public Find_File_Data
{
public:
    Find_File_Data_Win();
    virtual ~Find_File_Data_Win();

    virtual const char* GetName() const;
    virtual unsigned int GetTime() const;

    virtual bool FindFirst(const char* fname);
    virtual bool FindNext();
    virtual void Close();

private:
    HANDLE FindHandle;
    WIN32_FIND_DATAA FindData;
};

Find_File_Data_Win::Find_File_Data_Win()
    : FindHandle(INVALID_HANDLE_VALUE)
    , FindData({0})
{
}

Find_File_Data_Win::~Find_File_Data_Win()
{
    Close();
}

const char* Find_File_Data_Win::GetName() const
{
    return FindData.cFileName;
}

unsigned int Find_File_Data_Win::GetTime() const
{
    ULARGE_INTEGER ull;
    ull.LowPart = FindData.ftLastWriteTime.dwLowDateTime;
    ull.HighPart = FindData.ftLastWriteTime.dwHighDateTime;
    return (unsigned int)(ull.QuadPart / 10000000ULL - 11644473600ULL);
}

bool Find_File_Data_Win::FindFirst(const char* fname)
{
    FindHandle = FindFirstFileA(fname, &FindData);
    return (FindHandle != INVALID_HANDLE_VALUE);
}

bool Find_File_Data_Win::FindNext()
{
    if (FindHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    return (FindNextFileA(FindHandle, &FindData) != FALSE);
}

void Find_File_Data_Win::Close()
{
    if (FindHandle != INVALID_HANDLE_VALUE) {
        FindClose(FindHandle);
        FindHandle = INVALID_HANDLE_VALUE;
    }
}

Find_File_Data* Find_File_Data::CreateFindData()
{
    return new Find_File_Data_Win();
}
#endif
