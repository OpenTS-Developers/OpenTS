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
#include <cstdint>
#include <cstdio>

#if defined(__APPLE__)
#include <objc/objc.h>
#else
using BOOL = int;
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Widths match the Windows x64 build, where long is 32 bits.
using BYTE = std::uint8_t;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
using COLORREF = DWORD;
using LONG = std::int32_t;
using ULONG = std::uint32_t;
using LONGLONG = std::int64_t;
using ULONGLONG = std::uint64_t;
using INT = int;
using UINT = unsigned int;
using SHORT = std::int16_t;
using USHORT = std::uint16_t;
using HRESULT = std::int32_t;
using LRESULT = std::intptr_t;
using LPARAM = std::intptr_t;
using WPARAM = std::uintptr_t;
using INT_PTR = std::intptr_t;
using UINT_PTR = std::uintptr_t;
using LONG_PTR = std::intptr_t;
using ULONG_PTR = std::uintptr_t;
using DWORD_PTR = std::uintptr_t;
using SIZE_T = std::size_t;

using HANDLE = void *;
using HWND = void *;
using HINSTANCE = void *;
using HMODULE = void *;
using HICON = void *;
using HCURSOR = void *;
using HBRUSH = void *;
using HMENU = void *;
using HDC = void *;
using HFONT = void *;
using HBITMAP = void *;
using HGDIOBJ = void *;
using HKL = void *;
using HRGN = void *;
using HPEN = void *;
using HKEY = void *;

using LPVOID = void *;
using LPCVOID = void const *;
using LPSTR = char *;
using LPCSTR = char const *;
using LPWSTR = wchar_t *;
using LPCWSTR = wchar_t const *;
using LPBYTE = BYTE *;
using LPDWORD = DWORD *;
using LPCTSTR = char const *;
using LPTSTR = char *;

#define TEXT(text) text

struct POINT
{
	LONG x;
	LONG y;
};

struct RECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
};

struct SIZE
{
	LONG cx;
	LONG cy;
};

struct FILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
};

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define IDOK 1
#define IDCANCEL 2
#define IDABORT 3
#define IDRETRY 4
#define IDIGNORE 5
#define IDYES 6
#define IDNO 7

#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_HIDDEN 0x00000002
#define FILE_ATTRIBUTE_SYSTEM 0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

#define LOWORD(value) ((WORD)((DWORD_PTR)(value) & 0xFFFF))
#define HIWORD(value) ((WORD)(((DWORD_PTR)(value) >> 16) & 0xFFFF))
#define RGB(red, green, blue) ((DWORD)(((BYTE)(red)) | ((WORD)((BYTE)(green)) << 8) | ((DWORD)((BYTE)(blue)) << 16)))

#ifndef VK_LBUTTON
#define VK_LBUTTON 0x01
#endif
#ifndef VK_RBUTTON
#define VK_RBUTTON 0x02
#endif
#ifndef VK_MBUTTON
#define VK_MBUTTON 0x04
#endif
#ifndef VK_XBUTTON1
#define VK_XBUTTON1 0x05
#endif
#ifndef VK_XBUTTON2
#define VK_XBUTTON2 0x06
#endif
#ifndef VK_BACK
#define VK_BACK 0x08
#endif
#ifndef VK_TAB
#define VK_TAB 0x09
#endif
#ifndef VK_CLEAR
#define VK_CLEAR 0x0C
#endif
#ifndef VK_RETURN
#define VK_RETURN 0x0D
#endif
#ifndef VK_SHIFT
#define VK_SHIFT 0x10
#endif
#ifndef VK_CONTROL
#define VK_CONTROL 0x11
#endif
#ifndef VK_MENU
#define VK_MENU 0x12
#endif
#ifndef VK_PAUSE
#define VK_PAUSE 0x13
#endif
#ifndef VK_CAPITAL
#define VK_CAPITAL 0x14
#endif
#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif
#ifndef VK_SPACE
#define VK_SPACE 0x20
#endif
#ifndef VK_PRIOR
#define VK_PRIOR 0x21
#endif
#ifndef VK_NEXT
#define VK_NEXT 0x22
#endif
#ifndef VK_END
#define VK_END 0x23
#endif
#ifndef VK_HOME
#define VK_HOME 0x24
#endif
#ifndef VK_LEFT
#define VK_LEFT 0x25
#endif
#ifndef VK_UP
#define VK_UP 0x26
#endif
#ifndef VK_RIGHT
#define VK_RIGHT 0x27
#endif
#ifndef VK_DOWN
#define VK_DOWN 0x28
#endif
#ifndef VK_SNAPSHOT
#define VK_SNAPSHOT 0x2C
#endif
#ifndef VK_INSERT
#define VK_INSERT 0x2D
#endif
#ifndef VK_DELETE
#define VK_DELETE 0x2E
#endif
#ifndef VK_LWIN
#define VK_LWIN 0x5B
#endif
#ifndef VK_RWIN
#define VK_RWIN 0x5C
#endif
#ifndef VK_APPS
#define VK_APPS 0x5D
#endif
#ifndef VK_NUMPAD0
#define VK_NUMPAD0 0x60
#endif
#ifndef VK_NUMPAD1
#define VK_NUMPAD1 0x61
#endif
#ifndef VK_NUMPAD2
#define VK_NUMPAD2 0x62
#endif
#ifndef VK_NUMPAD3
#define VK_NUMPAD3 0x63
#endif
#ifndef VK_NUMPAD4
#define VK_NUMPAD4 0x64
#endif
#ifndef VK_NUMPAD5
#define VK_NUMPAD5 0x65
#endif
#ifndef VK_NUMPAD6
#define VK_NUMPAD6 0x66
#endif
#ifndef VK_NUMPAD7
#define VK_NUMPAD7 0x67
#endif
#ifndef VK_NUMPAD8
#define VK_NUMPAD8 0x68
#endif
#ifndef VK_NUMPAD9
#define VK_NUMPAD9 0x69
#endif
#ifndef VK_MULTIPLY
#define VK_MULTIPLY 0x6A
#endif
#ifndef VK_ADD
#define VK_ADD 0x6B
#endif
#ifndef VK_SEPARATOR
#define VK_SEPARATOR 0x6C
#endif
#ifndef VK_SUBTRACT
#define VK_SUBTRACT 0x6D
#endif
#ifndef VK_DECIMAL
#define VK_DECIMAL 0x6E
#endif
#ifndef VK_DIVIDE
#define VK_DIVIDE 0x6F
#endif
#ifndef VK_F1
#define VK_F1 0x70
#endif
#ifndef VK_F2
#define VK_F2 0x71
#endif
#ifndef VK_F3
#define VK_F3 0x72
#endif
#ifndef VK_F4
#define VK_F4 0x73
#endif
#ifndef VK_F5
#define VK_F5 0x74
#endif
#ifndef VK_F6
#define VK_F6 0x75
#endif
#ifndef VK_F7
#define VK_F7 0x76
#endif
#ifndef VK_F8
#define VK_F8 0x77
#endif
#ifndef VK_F9
#define VK_F9 0x78
#endif
#ifndef VK_F10
#define VK_F10 0x79
#endif
#ifndef VK_F11
#define VK_F11 0x7A
#endif
#ifndef VK_F12
#define VK_F12 0x7B
#endif
#ifndef VK_F13
#define VK_F13 0x7C
#endif
#ifndef VK_F24
#define VK_F24 0x87
#endif
#ifndef VK_HELP
#define VK_HELP 0x2F
#endif
#ifndef VK_SELECT
#define VK_SELECT 0x29
#endif
#ifndef VK_EXECUTE
#define VK_EXECUTE 0x2B
#endif
#ifndef VK_SLEEP
#define VK_SLEEP 0x5F
#endif
#ifndef VK_VOLUME_MUTE
#define VK_VOLUME_MUTE 0xAD
#endif
#ifndef VK_VOLUME_DOWN
#define VK_VOLUME_DOWN 0xAE
#endif
#ifndef VK_VOLUME_UP
#define VK_VOLUME_UP 0xAF
#endif
#ifndef VK_MEDIA_NEXT_TRACK
#define VK_MEDIA_NEXT_TRACK 0xB0
#endif
#ifndef VK_MEDIA_PREV_TRACK
#define VK_MEDIA_PREV_TRACK 0xB1
#endif
#ifndef VK_MEDIA_STOP
#define VK_MEDIA_STOP 0xB2
#endif
#ifndef VK_MEDIA_PLAY_PAUSE
#define VK_MEDIA_PLAY_PAUSE 0xB3
#endif
#ifndef VK_NUMLOCK
#define VK_NUMLOCK 0x90
#endif
#ifndef VK_SCROLL
#define VK_SCROLL 0x91
#endif
#ifndef VK_LSHIFT
#define VK_LSHIFT 0xA0
#endif
#ifndef VK_RSHIFT
#define VK_RSHIFT 0xA1
#endif
#ifndef VK_LCONTROL
#define VK_LCONTROL 0xA2
#endif
#ifndef VK_RCONTROL
#define VK_RCONTROL 0xA3
#endif
#ifndef VK_LMENU
#define VK_LMENU 0xA4
#endif
#ifndef VK_RMENU
#define VK_RMENU 0xA5
#endif
#ifndef VK_OEM_1
#define VK_OEM_1 0xBA
#endif
#ifndef VK_OEM_PLUS
#define VK_OEM_PLUS 0xBB
#endif
#ifndef VK_OEM_COMMA
#define VK_OEM_COMMA 0xBC
#endif
#ifndef VK_OEM_MINUS
#define VK_OEM_MINUS 0xBD
#endif
#ifndef VK_OEM_PERIOD
#define VK_OEM_PERIOD 0xBE
#endif
#ifndef VK_OEM_2
#define VK_OEM_2 0xBF
#endif
#ifndef VK_OEM_3
#define VK_OEM_3 0xC0
#endif
#ifndef VK_OEM_4
#define VK_OEM_4 0xDB
#endif
#ifndef VK_OEM_5
#define VK_OEM_5 0xDC
#endif
#ifndef VK_OEM_6
#define VK_OEM_6 0xDD
#endif
#ifndef VK_OEM_7
#define VK_OEM_7 0xDE
#endif
#ifndef VK_OEM_8
#define VK_OEM_8 0xDF
#endif
#ifndef VK_OEM_102
#define VK_OEM_102 0xE2
#endif

#define CALLBACK
#define WINAPI
#define APIENTRY

#define GENERIC_READ 0x80000000u
#define GENERIC_WRITE 0x40000000u
#define FILE_SHARE_READ 0x00000001u
#define FILE_SHARE_WRITE 0x00000002u
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#define FILE_FLAG_RANDOM_ACCESS 0x10000000u
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFFu)
#define INVALID_SET_FILE_POINTER ((DWORD)0xFFFFFFFFu)
#define MOVEFILE_REPLACE_EXISTING 0x00000001u

#define ERROR_SUCCESS 0
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_ACCESS_DENIED 5
#define ERROR_INVALID_HANDLE 6
#define ERROR_FILE_EXISTS 80
#define ERROR_ALREADY_EXISTS 183
#define ERROR_INSUFFICIENT_BUFFER 122

#define CP_UTF8 65001u
#define CP_ACP 0u

#define HKEY_LOCAL_MACHINE ((HKEY)(ULONG_PTR)0x80000002)
#define KEY_READ 0x20019u

#define STD_INPUT_HANDLE ((DWORD)-10)
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define STD_ERROR_HANDLE ((DWORD)-12)
#define ATTACH_PARENT_PROCESS ((DWORD)-1)
#define MF_BYCOMMAND 0x0000u
#define SC_CLOSE 0xF060u

#define SM_CXDRAG 68
#define SM_CYDRAG 69

#define BI_RGB 0
#define BI_BITFIELDS 3
#define DIB_RGB_COLORS 0
#define SRCCOPY 0x00CC0020u
#define COLORONCOLOR 3
#define TRANSPARENT 1
#define OPAQUE 2
#define FW_NORMAL 400
#define FW_BOLD 700
#define ANSI_CHARSET 0
#define DEFAULT_CHARSET 1
#define OUT_DEFAULT_PRECIS 0
#define OUT_RASTER_PRECIS 6
#define CLIP_DEFAULT_PRECIS 0
#define DEFAULT_QUALITY 0
#define PROOF_QUALITY 2
#define CLEARTYPE_QUALITY 5
#define DEFAULT_PITCH 0
#define FIXED_PITCH 1
#define FF_DONTCARE 0
#define FF_ROMAN 16
#define FF_SWISS 32
#define FF_MODERN 48
#define TA_LEFT 0
#define TA_RIGHT 2
#define TA_CENTER 6
#define GM_ADVANCED 2
#define MWT_IDENTITY 1
#define LF_FACESIZE 32

#define LANG_USER_DEFAULT 0x0400u
#define LANG_NEUTRAL 0x00u
#define SUBLANG_DEFAULT 0x01u
#define MAKELANGID(primary, sub) ((WORD)((((WORD)(sub)) << 10) | (WORD)(primary)))
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000u
#define TIME_NOMINUTESORSECONDS 0x00000001u
#define TIME_NOSECONDS 0x00000002u
#define MAPVK_VK_TO_CHAR 2u
#define MAPVK_VSC_TO_VK_EX 3u

#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_NT_SIGNATURE 0x00004550u
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10Bu

#define PROCESS_POWER_THROTTLING_CURRENT_VERSION 1
#define ProcessPowerThrottling 4

#define GetFileExInfoStandard 0

struct WIN32_FIND_DATAA
{
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	char cFileName[260];
	char cAlternateFileName[14];
};

using WIN32_FIND_DATA = WIN32_FIND_DATAA;

struct SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
};

union LARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		LONG HighPart;
	};
	std::int64_t QuadPart;
};

union ULARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		DWORD HighPart;
	};
	unsigned long long QuadPart;
};

struct BITMAPINFOHEADER
{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
};

struct RGBQUAD
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
};

struct BITMAPINFO
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[1];
};

struct BITMAP
{
	LONG bmType;
	LONG bmWidth;
	LONG bmHeight;
	LONG bmWidthBytes;
	WORD bmPlanes;
	WORD bmBitsPixel;
	LPVOID bmBits;
};

struct DIBSECTION
{
	BITMAP dsBm;
	BITMAPINFOHEADER dsBmih;
	DWORD dsBitfields[3];
	HANDLE dshSection;
	DWORD dsOffset;
};

struct LOGFONT
{
	LONG lfHeight;
	LONG lfWidth;
	LONG lfEscapement;
	LONG lfOrientation;
	LONG lfWeight;
	BYTE lfItalic;
	BYTE lfUnderline;
	BYTE lfStrikeOut;
	BYTE lfCharSet;
	BYTE lfOutPrecision;
	BYTE lfClipPrecision;
	BYTE lfQuality;
	BYTE lfPitchAndFamily;
	char lfFaceName[LF_FACESIZE];
};

struct TEXTMETRIC
{
	LONG tmHeight;
	LONG tmAscent;
	LONG tmDescent;
	LONG tmInternalLeading;
	LONG tmExternalLeading;
	LONG tmAveCharWidth;
	LONG tmMaxCharWidth;
	LONG tmWeight;
};

struct MEMORYSTATUS
{
	DWORD dwLength;
	DWORD dwMemoryLoad;
	SIZE_T dwTotalPhys;
	SIZE_T dwAvailPhys;
	SIZE_T dwTotalPageFile;
	SIZE_T dwAvailPageFile;
	SIZE_T dwTotalVirtual;
	SIZE_T dwAvailVirtual;
};

struct WIN32_FILE_ATTRIBUTE_DATA
{
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
};

struct IMAGE_DOS_HEADER
{
	WORD e_magic;
	WORD e_cblp;
	WORD e_cp;
	WORD e_crlc;
	WORD e_cparhdr;
	WORD e_minalloc;
	WORD e_maxalloc;
	WORD e_ss;
	WORD e_sp;
	WORD e_csum;
	WORD e_ip;
	WORD e_cs;
	WORD e_lfarlc;
	WORD e_ovno;
	WORD e_res[4];
	WORD e_oemid;
	WORD e_oeminfo;
	WORD e_res2[10];
	LONG e_lfanew;
};

struct IMAGE_FILE_HEADER
{
	WORD Machine;
	WORD NumberOfSections;
	DWORD TimeDateStamp;
	DWORD PointerToSymbolTable;
	DWORD NumberOfSymbols;
	WORD SizeOfOptionalHeader;
	WORD Characteristics;
};

struct IMAGE_OPTIONAL_HEADER32
{
	WORD Magic;
	BYTE MajorLinkerVersion;
	BYTE MinorLinkerVersion;
	DWORD SizeOfCode;
	DWORD SizeOfInitializedData;
	DWORD SizeOfUninitializedData;
	DWORD AddressOfEntryPoint;
	DWORD BaseOfCode;
	DWORD BaseOfData;
	DWORD ImageBase;
	DWORD SizeOfImage;
};

struct IMAGE_NT_HEADERS32
{
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};

using IMAGE_NT_HEADERS = IMAGE_NT_HEADERS32;

struct PROCESS_POWER_THROTTLING_STATE
{
	ULONG Version;
	ULONG ControlMask;
	ULONG StateMask;
};

#include <pthread.h>

struct SRWLOCK
{
	pthread_rwlock_t Lock;
};

#define SRWLOCK_INIT { PTHREAD_RWLOCK_INITIALIZER }

struct COORD
{
	SHORT X;
	SHORT Y;
};

struct SMALL_RECT
{
	SHORT Left;
	SHORT Top;
	SHORT Right;
	SHORT Bottom;
};

struct CONSOLE_SCREEN_BUFFER_INFO
{
	COORD dwSize;
	COORD dwCursorPosition;
	WORD wAttributes;
	SMALL_RECT srWindow;
	COORD dwMaximumWindowSize;
};

using FARPROC = void (*)();
using WCHAR = wchar_t;

struct RTL_OSVERSIONINFOW
{
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	WCHAR szCSDVersion[128];
};

using PRTL_OSVERSIONINFOW = RTL_OSVERSIONINFOW *;

inline int CompareFileTime(FILETIME const * left, FILETIME const * right)
{
	unsigned long long const a = ((unsigned long long)left->dwHighDateTime << 32) | left->dwLowDateTime;
	unsigned long long const b = ((unsigned long long)right->dwHighDateTime << 32) | right->dwLowDateTime;
	if (a < b) return(-1);
	if (a > b) return(1);
	return(0);
}

HANDLE FindFirstFileA(char const * pattern, WIN32_FIND_DATAA * found);
HANDLE FindFirstFile(char const * pattern, WIN32_FIND_DATAA * found);
int FindNextFileA(HANDLE handle, WIN32_FIND_DATAA * found);
int FindNextFile(HANDLE handle, WIN32_FIND_DATAA * found);
int FindClose(HANDLE handle);

HANDLE CreateFileA(char const * path, DWORD access, DWORD share, void * security, DWORD disposition, DWORD flags, HANDLE template_file);
inline HANDLE CreateFile(char const * path, DWORD access, DWORD share, void * security, DWORD disposition, DWORD flags, HANDLE template_file)
{
	return(CreateFileA(path, access, share, security, disposition, flags, template_file));
}
BOOL ReadFile(HANDLE file, void * buffer, DWORD bytes, DWORD * read, void * overlapped);
BOOL WriteFile(HANDLE file, void const * buffer, DWORD bytes, DWORD * written, void * overlapped);
BOOL CloseHandle(HANDLE handle);
BOOL DeleteFileA(char const * path);
inline BOOL DeleteFile(char const * path) { return(DeleteFileA(path)); }
DWORD GetFileAttributesA(char const * path);
inline DWORD GetFileAttributes(char const * path) { return(GetFileAttributesA(path)); }
BOOL GetFileAttributesExA(char const * path, int level, void * information);
inline BOOL GetFileAttributesEx(char const * path, int level, void * information)
{
	return(GetFileAttributesExA(path, level, information));
}
DWORD GetFileSize(HANDLE file, DWORD * high);
BOOL FlushFileBuffers(HANDLE file);
BOOL MoveFileExA(char const * from, char const * to, DWORD flags);
DWORD SetFilePointer(HANDLE file, LONG distance, LONG * high, DWORD method);
BOOL CreateDirectoryA(char const * path, void * security);
inline BOOL CreateDirectory(char const * path, void * security) { return(CreateDirectoryA(path, security)); }
BOOL CopyFileA(char const * from, char const * to, BOOL fail_if_exists);
inline BOOL CopyFile(char const * from, char const * to, BOOL fail_if_exists)
{
	return(CopyFileA(from, to, fail_if_exists));
}

void GetSystemTime(SYSTEMTIME * time);
void GetLocalTime(SYSTEMTIME * time);
BOOL SystemTimeToFileTime(SYSTEMTIME const * time, FILETIME * stamp);
BOOL FileTimeToSystemTime(FILETIME const * stamp, SYSTEMTIME * time);
BOOL FileTimeToLocalFileTime(FILETIME const * utc, FILETIME * local);
void GetSystemTimeAsFileTime(FILETIME * stamp);
DWORD GetTickCount(void);
ULONGLONG GetTickCount64(void);
BOOL QueryPerformanceCounter(LARGE_INTEGER * counter);
BOOL QueryPerformanceFrequency(LARGE_INTEGER * frequency);
UINT timeBeginPeriod(UINT period);
UINT timeEndPeriod(UINT period);

DWORD GetLastError(void);
void SetLastError(DWORD error);
HANDLE GetCurrentProcess(void);
DWORD GetCurrentProcessId(void);
DWORD GetCurrentThreadId(void);
BOOL SetProcessInformation(HANDLE process, int kind, void const * information, DWORD size);
BOOL GetDiskFreeSpaceExA(char const * path, ULARGE_INTEGER * available, ULARGE_INTEGER * total, ULARGE_INTEGER * free_total);
inline BOOL GetDiskFreeSpaceEx(char const * path, ULARGE_INTEGER * available, ULARGE_INTEGER * total, ULARGE_INTEGER * free_total)
{
	return(GetDiskFreeSpaceExA(path, available, total, free_total));
}
void GlobalMemoryStatus(MEMORYSTATUS * status);
HMODULE GetModuleHandleA(char const * name);
inline HMODULE GetModuleHandle(char const * name) { return(GetModuleHandleA(name)); }
DWORD GetModuleFileNameA(HMODULE module, char * buffer, DWORD size);
inline DWORD GetModuleFileName(HMODULE module, char * buffer, DWORD size)
{
	return(GetModuleFileNameA(module, buffer, size));
}
UINT GetWindowsDirectoryA(char * buffer, UINT size);
HMODULE LoadLibraryA(char const * name);
inline HMODULE LoadLibrary(char const * name) { return(LoadLibraryA(name)); }
int LoadStringA(HINSTANCE module, UINT id, char * buffer, int size);
inline int LoadString(HINSTANCE module, UINT id, char * buffer, int size)
{
	return(LoadStringA(module, id, buffer, size));
}
UINT GetACP(void);
UINT GetOEMCP(void);
DWORD GetFileVersionInfoSizeA(char const * path, DWORD * handle);
inline DWORD GetFileVersionInfoSize(char const * path, DWORD * handle)
{
	return(GetFileVersionInfoSizeA(path, handle));
}
BOOL GetFileVersionInfoA(char const * path, DWORD handle, DWORD size, void * data);
inline BOOL GetFileVersionInfo(char const * path, DWORD handle, DWORD size, void * data)
{
	return(GetFileVersionInfoA(path, handle, size, data));
}
BOOL VerQueryValueA(void const * block, char const * query, void ** value, UINT * length);
inline BOOL VerQueryValue(void const * block, char const * query, void ** value, UINT * length)
{
	return(VerQueryValueA(block, query, value, length));
}
FARPROC GetProcAddress(HMODULE module, char const * name);

LONG RegOpenKeyExA(HKEY key, char const * name, DWORD options, DWORD access, HKEY * opened);
inline LONG RegOpenKeyEx(HKEY key, char const * name, DWORD options, DWORD access, HKEY * opened)
{
	return(RegOpenKeyExA(key, name, options, access, opened));
}
LONG RegQueryValueExA(HKEY key, char const * name, DWORD * reserved, DWORD * type, BYTE * data, DWORD * size);
inline LONG RegQueryValueEx(HKEY key, char const * name, DWORD * reserved, DWORD * type, BYTE * data, DWORD * size)
{
	return(RegQueryValueExA(key, name, reserved, type, data, size));
}
LONG RegCloseKey(HKEY key);

BOOL AllocConsole(void);
BOOL AttachConsole(DWORD process);
BOOL SetConsoleTitleA(char const * title);
inline BOOL SetConsoleTitle(char const * title) { return(SetConsoleTitleA(title)); }
BOOL SetConsoleOutputCP(UINT codepage);
BOOL SetConsoleCP(UINT codepage);
int freopen_s(FILE ** opened, char const * path, char const * mode, FILE * stream);
HANDLE GetStdHandle(DWORD which);
BOOL SetStdHandle(DWORD which, HANDLE handle);
BOOL GetConsoleScreenBufferInfo(HANDLE console, CONSOLE_SCREEN_BUFFER_INFO * info);
BOOL SetConsoleScreenBufferSize(HANDLE console, COORD size);
HWND GetConsoleWindow(void);
HMENU GetSystemMenu(HWND window, BOOL revert);
BOOL DeleteMenu(HMENU menu, UINT position, UINT flags);
BOOL WriteConsoleA(HANDLE console, void const * text, DWORD chars, DWORD * written, void * reserved);
inline BOOL WriteConsole(HANDLE console, void const * text, DWORD chars, DWORD * written, void * reserved)
{
	return(WriteConsoleA(console, text, chars, written, reserved));
}
void OutputDebugStringA(char const * text);
inline void OutputDebugString(char const * text) { OutputDebugStringA(text); }
BOOL IsDebuggerPresent(void);
wchar_t const * GetCommandLineW(void);
wchar_t ** CommandLineToArgvW(wchar_t const * command, int * count);
void LocalFree(void * memory);

HDC GetDC(HWND window);
int ReleaseDC(HWND window, HDC dc);
HDC CreateCompatibleDC(HDC dc);
BOOL DeleteDC(HDC dc);
HBITMAP CreateDIBSection(HDC dc, BITMAPINFO const * info, UINT usage, void ** bits, HANDLE section, DWORD offset);
HGDIOBJ SelectObject(HDC dc, HGDIOBJ object);
BOOL DeleteObject(HGDIOBJ object);
int GetObject(HGDIOBJ object, int size, void * into);
BOOL GdiFlush(void);
int SetStretchBltMode(HDC dc, int mode);
BOOL StretchBlt(HDC dest, int x, int y, int width, int height, HDC source, int sx, int sy, int swidth, int sheight, DWORD rop);
HFONT CreateFontA(int height, int width, int escapement, int orientation, int weight, DWORD italic, DWORD underline, DWORD strike, DWORD charset, DWORD out_precision, DWORD clip_precision, DWORD quality, DWORD pitch, char const * face);
inline HFONT CreateFont(int height, int width, int escapement, int orientation, int weight, DWORD italic, DWORD underline, DWORD strike, DWORD charset, DWORD out_precision, DWORD clip_precision, DWORD quality, DWORD pitch, char const * face)
{
	return(CreateFontA(height, width, escapement, orientation, weight, italic, underline, strike, charset, out_precision, clip_precision, quality, pitch, face));
}
HFONT CreateFontIndirectA(LOGFONT const * font);
inline HFONT CreateFontIndirect(LOGFONT const * font) { return(CreateFontIndirectA(font)); }
COLORREF SetTextColor(HDC dc, COLORREF color);
int SetBkMode(HDC dc, int mode);
UINT SetTextAlign(HDC dc, UINT align);
BOOL TextOutA(HDC dc, int x, int y, char const * text, int length);
inline BOOL TextOut(HDC dc, int x, int y, char const * text, int length)
{
	return(TextOutA(dc, x, y, text, length));
}
BOOL GetTextExtentPoint32A(HDC dc, char const * text, int length, SIZE * size);
inline BOOL GetTextExtentPoint32(HDC dc, char const * text, int length, SIZE * size)
{
	return(GetTextExtentPoint32A(dc, text, length, size));
}
BOOL GetTextMetricsA(HDC dc, TEXTMETRIC * metrics);
inline BOOL GetTextMetrics(HDC dc, TEXTMETRIC * metrics) { return(GetTextMetricsA(dc, metrics)); }
int SaveDC(HDC dc);
BOOL RestoreDC(HDC dc, int saved);
int SetGraphicsMode(HDC dc, int mode);
BOOL ModifyWorldTransform(HDC dc, void const * transform, DWORD mode);
BOOL SetViewportOrgEx(HDC dc, int x, int y, POINT * previous);
BOOL SetWindowOrgEx(HDC dc, int x, int y, POINT * previous);
BOOL DPtoLP(HDC dc, POINT * points, int count);

SHORT GetKeyState(int virtualkey);
int GetSystemMetrics(int index);
LRESULT DefWindowProcW(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

void * _aligned_malloc(std::size_t size, std::size_t alignment);
void _aligned_free(void * memory);
void * _aligned_realloc(void * memory, std::size_t size, std::size_t alignment);
void AcquireSRWLockExclusive(SRWLOCK * lock);
void ReleaseSRWLockExclusive(SRWLOCK * lock);

int GetDateFormatA(DWORD locale, DWORD flags, SYSTEMTIME const * time, char const * format, char * buffer, int size);
inline int GetDateFormat(DWORD locale, DWORD flags, SYSTEMTIME const * time, char const * format, char * buffer, int size)
{
	return(GetDateFormatA(locale, flags, time, format, buffer, size));
}
int GetTimeFormatA(DWORD locale, DWORD flags, SYSTEMTIME const * time, char const * format, char * buffer, int size);
inline int GetTimeFormat(DWORD locale, DWORD flags, SYSTEMTIME const * time, char const * format, char * buffer, int size)
{
	return(GetTimeFormatA(locale, flags, time, format, buffer, size));
}
unsigned int _controlfp(unsigned int value, unsigned int mask);
int _getch(void);
DWORD FormatMessageA(DWORD flags, void const * source, DWORD message, DWORD language, char * buffer, DWORD size, void * arguments);
inline DWORD FormatMessage(DWORD flags, void const * source, DWORD message, DWORD language, char * buffer, DWORD size, void * arguments)
{
	return(FormatMessageA(flags, source, message, language, buffer, size, arguments));
}
HKL GetKeyboardLayout(DWORD thread);
UINT MapVirtualKeyExW(UINT code, UINT map_type, HKL layout);
wchar_t * CharUpperW(wchar_t * text);
int WideCharToMultiByte(UINT codepage, DWORD flags, wchar_t const * wide, int wide_count, char * buffer, int buffer_size, char const * fallback, int * used_fallback);

#define GetRValue(rgb) ((BYTE)(rgb))
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) ((BYTE)((rgb) >> 16))

#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <strings.h>
#include <unistd.h>

#ifdef stricmp
#undef stricmp
#endif
#ifdef _stricmp
#undef _stricmp
#endif
#ifdef strnicmp
#undef strnicmp
#endif
#ifdef _strnicmp
#undef _strnicmp
#endif

inline void Sleep(DWORD milliseconds)
{
	usleep((useconds_t)milliseconds * 1000u);
}

inline DWORD timeGetTime(void)
{
	auto const now = std::chrono::steady_clock::now().time_since_epoch();
	return((DWORD)std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

inline int stricmp(char const * left, char const * right)
{
	return(strcasecmp(left == nullptr ? "" : left, right == nullptr ? "" : right));
}

inline int _stricmp(char const * left, char const * right)
{
	return(stricmp(left, right));
}

inline int _strnicmp(char const * left, char const * right, std::size_t count)
{
	return(strncasecmp(left == nullptr ? "" : left, right == nullptr ? "" : right, count));
}

inline int strnicmp(char const * left, char const * right, std::size_t count)
{
	return(_strnicmp(left, right, count));
}
