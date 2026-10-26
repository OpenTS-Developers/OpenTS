/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#if !defined(_WIN32)

#include "platform.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unordered_map>
#include <vector>

#include <crt_externs.h>
#include <mach-o/dyld.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>

namespace
{

// FILETIME counts 100-nanosecond ticks from 1601-01-01 UTC.
constexpr unsigned long long FILETIME_UNIX_OFFSET = 11644473600ULL * 10000000ULL;

thread_local DWORD LastError = 0;

enum class Kind
{
	File,
	Bitmap,
	Context,
	Font,
};

struct PlatformObject
{
	explicit PlatformObject(Kind kind) : Type(kind) {}
	virtual ~PlatformObject() = default;
	Kind Type;
};

struct FileObject : PlatformObject
{
	FileObject(void) : PlatformObject(Kind::File) {}
	FILE * Stream = nullptr;
	bool Owned = true;
};

struct BitmapObject : PlatformObject
{
	BitmapObject(void) : PlatformObject(Kind::Bitmap) {}
	int Width = 0;
	int Height = 0;
	int Pitch = 0;
	std::vector<unsigned char> Pixels;
};

struct FontObject : PlatformObject
{
	FontObject(void) : PlatformObject(Kind::Font) {}
	int Height = 16;
};

struct ContextObject : PlatformObject
{
	ContextObject(void) : PlatformObject(Kind::Context) {}
	BitmapObject * Selected = nullptr;
	FontObject * Font = nullptr;
	int StretchMode = COLORONCOLOR;
	bool Permanent = false;
};

struct Finder
{
	DIR * Directory;
	std::string DirectoryPath;
	std::string Pattern;
};

struct ArgumentList
{
	std::vector<std::wstring> Storage;
	std::vector<wchar_t *> Pointers;
};

struct Allocation
{
	std::size_t Size = 0;
	std::size_t Alignment = 0;
};

// Other static constructors log before a namespace-scope map would exist.
std::mutex & Argument_Lock(void)
{
	static std::mutex lock;
	return(lock);
}


std::unordered_map<void *, ArgumentList *> & Argument_Map(void)
{
	static std::unordered_map<void *, ArgumentList *> arguments;
	return(arguments);
}


std::mutex & Allocation_Lock(void)
{
	// Left allocated so exit can still lock it. A function-local static is destroyed before atexit runs.
	static std::mutex * lock = new std::mutex;
	return(*lock);
}


std::unordered_map<void *, Allocation> & Allocation_Map(void)
{
	static auto * allocations = new std::unordered_map<void *, Allocation>;
	return(*allocations);
}

HANDLE StandardHandles[3] = { nullptr, nullptr, nullptr };


void Set_Errno_Error(void)
{
	if (errno == ENOENT) LastError = ERROR_FILE_NOT_FOUND;
	else if (errno == EEXIST) LastError = ERROR_ALREADY_EXISTS;
	else if (errno == EACCES || errno == EPERM) LastError = ERROR_ACCESS_DENIED;
	else LastError = ERROR_INVALID_HANDLE;
}


std::string Normalize_Path(char const * path)
{
	if (path == nullptr) return(std::string());
	std::string text(path);
	for (char & character : text) {
		if (character == '\\') character = '/';
	}
	return(text);
}


template <typename T>
T * As(HANDLE handle, Kind kind)
{
	if (handle == nullptr || handle == INVALID_HANDLE_VALUE) return(nullptr);
	auto * object = static_cast<PlatformObject *>(handle);
	if (object->Type != kind) return(nullptr);
	return(static_cast<T *>(object));
}


FileObject * Console_File(FILE * stream)
{
	static FileObject output;
	static FileObject input;
	static FileObject error;
	FileObject * file = &output;
	if (stream == stdin) file = &input;
	else if (stream == stderr) file = &error;
	file->Stream = stream;
	file->Owned = false;
	return(file);
}


bool Console_Path(std::string const & path, FILE ** stream)
{
	if (path == "CONOUT$") {
		*stream = stdout;
		return(true);
	}
	if (path == "CONIN$") {
		*stream = stdin;
		return(true);
	}
	return(false);
}


char const * Open_Mode(DWORD access, DWORD disposition)
{
	bool const read = (access & GENERIC_READ) != 0;
	bool const write = (access & GENERIC_WRITE) != 0;
	if (disposition == CREATE_NEW) return(read ? "w+bx" : "wbx");
	if (disposition == CREATE_ALWAYS || disposition == TRUNCATE_EXISTING) return(read ? "w+b" : "wb");
	// OPEN_ALWAYS is resolved by CreateFileA before this function is reached.
	return(write ? "r+b" : "rb");
}


bool Write_Stamp(std::time_t seconds, int milliseconds, FILETIME * stamp)
{
	if (stamp == nullptr || seconds < 0) return(false);
	unsigned long long const ticks = (unsigned long long)seconds * 10000000ULL
		+ (unsigned long long)milliseconds * 10000ULL + FILETIME_UNIX_OFFSET;
	stamp->dwLowDateTime = (DWORD)ticks;
	stamp->dwHighDateTime = (DWORD)(ticks >> 32);
	return(true);
}


bool Read_Stamp(FILETIME const * stamp, std::time_t * seconds, int * milliseconds)
{
	if (stamp == nullptr || seconds == nullptr) return(false);
	unsigned long long const ticks = ((unsigned long long)stamp->dwHighDateTime << 32) | stamp->dwLowDateTime;
	if (ticks < FILETIME_UNIX_OFFSET) return(false);
	unsigned long long const unix_ticks = ticks - FILETIME_UNIX_OFFSET;
	*seconds = (std::time_t)(unix_ticks / 10000000ULL);
	if (milliseconds != nullptr) *milliseconds = (int)((unix_ticks / 10000ULL) % 1000ULL);
	return(true);
}


bool Time_From_System(SYSTEMTIME const & time, std::time_t * seconds)
{
	std::tm fields = {};
	fields.tm_year = (int)time.wYear - 1900;
	fields.tm_mon = (int)time.wMonth - 1;
	fields.tm_mday = (int)time.wDay;
	fields.tm_hour = (int)time.wHour;
	fields.tm_min = (int)time.wMinute;
	fields.tm_sec = (int)time.wSecond;
	std::time_t const converted = timegm(&fields);
	if (converted == (std::time_t)-1) return(false);
	*seconds = converted;
	return(true);
}


void System_From_Tm(std::tm const & fields, int milliseconds, SYSTEMTIME * time)
{
	time->wYear = (WORD)(fields.tm_year + 1900);
	time->wMonth = (WORD)(fields.tm_mon + 1);
	time->wDayOfWeek = (WORD)fields.tm_wday;
	time->wDay = (WORD)fields.tm_mday;
	time->wHour = (WORD)fields.tm_hour;
	time->wMinute = (WORD)fields.tm_min;
	time->wSecond = (WORD)fields.tm_sec;
	time->wMilliseconds = (WORD)milliseconds;
}


int Font_Height(ContextObject const * context)
{
	int height = (context != nullptr && context->Font != nullptr) ? context->Font->Height : 16;
	if (height < 0) height = -height;
	if (height == 0) height = 16;
	return(height);
}


void Fill_Found(WIN32_FIND_DATAA * found, char const * name, struct stat const & info)
{
	std::memset(found, 0, sizeof(*found));
	found->dwFileAttributes = S_ISDIR(info.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	found->nFileSizeLow = (DWORD)info.st_size;
	found->nFileSizeHigh = (DWORD)((unsigned long long)info.st_size >> 32);
	Write_Stamp(info.st_mtime, 0, &found->ftLastWriteTime);
	std::snprintf(found->cFileName, sizeof(found->cFileName), "%s", name);
}


std::string Join_Path(std::string const & directory, std::string const & name)
{
	if (directory == ".") return(name);
	if (!directory.empty() && directory.back() == '/') return(directory + name);
	return(directory + "/" + name);
}


int Standard_Index(DWORD which)
{
	if (which == STD_INPUT_HANDLE) return(0);
	if (which == STD_OUTPUT_HANDLE) return(1);
	if (which == STD_ERROR_HANDLE) return(2);
	return(-1);
}

}


DWORD GetLastError(void)
{
	return(LastError);
}


void SetLastError(DWORD error)
{
	LastError = error;
}


HANDLE CreateFileA(char const * path, DWORD desired, DWORD share, void *, DWORD disposition, DWORD, HANDLE)
{
	(void)share;
	std::string const text = Normalize_Path(path);
	FILE * console = nullptr;
	if (Console_Path(text, &console)) {
		LastError = ERROR_SUCCESS;
		return(Console_File(console));
	}

	if (disposition == TRUNCATE_EXISTING && access(text.c_str(), F_OK) != 0) {
		Set_Errno_Error();
		return(INVALID_HANDLE_VALUE);
	}

	char const * mode = Open_Mode(desired, disposition);
	if (disposition == OPEN_ALWAYS) {
		FILE * existing = std::fopen(text.c_str(), "r+b");
		if (existing != nullptr) {
			auto * file = new FileObject;
			file->Stream = existing;
			LastError = ERROR_SUCCESS;
			return(file);
		}
		mode = ((desired & GENERIC_READ) != 0) ? "w+b" : "wb";
	}

	FILE * opened = std::fopen(text.c_str(), mode);
	if (opened == nullptr) {
		Set_Errno_Error();
		if (disposition == CREATE_NEW && errno == EEXIST) LastError = ERROR_FILE_EXISTS;
		return(INVALID_HANDLE_VALUE);
	}

	auto * file = new FileObject;
	file->Stream = opened;
	LastError = ERROR_SUCCESS;
	return(file);
}


BOOL ReadFile(HANDLE handle, void * buffer, DWORD bytes, DWORD * read, void *)
{
	FileObject * file = As<FileObject>(handle, Kind::File);
	if (file == nullptr || file->Stream == nullptr || buffer == nullptr) {
		LastError = ERROR_INVALID_HANDLE;
		return(FALSE);
	}
	std::size_t const got = std::fread(buffer, 1, bytes, file->Stream);
	if (read != nullptr) *read = (DWORD)got;
	if (got < bytes && std::ferror(file->Stream)) {
		Set_Errno_Error();
		return(FALSE);
	}
	LastError = ERROR_SUCCESS;
	return(TRUE);
}


BOOL WriteFile(HANDLE handle, void const * buffer, DWORD bytes, DWORD * written, void *)
{
	FileObject * file = As<FileObject>(handle, Kind::File);
	if (file == nullptr || file->Stream == nullptr || (buffer == nullptr && bytes != 0)) {
		LastError = ERROR_INVALID_HANDLE;
		return(FALSE);
	}
	std::size_t const put = std::fwrite(buffer, 1, bytes, file->Stream);
	std::fflush(file->Stream);
	if (written != nullptr) *written = (DWORD)put;
	if (put != bytes) {
		Set_Errno_Error();
		return(FALSE);
	}
	LastError = ERROR_SUCCESS;
	return(TRUE);
}


BOOL CloseHandle(HANDLE handle)
{
	FileObject * file = As<FileObject>(handle, Kind::File);
	if (file == nullptr) return(FALSE);
	if (!file->Owned) return(TRUE);
	if (file->Stream != nullptr) std::fclose(file->Stream);
	delete file;
	return(TRUE);
}


BOOL DeleteFileA(char const * path)
{
	if (unlink(Normalize_Path(path).c_str()) != 0) {
		Set_Errno_Error();
		return(FALSE);
	}
	LastError = ERROR_SUCCESS;
	return(TRUE);
}


DWORD GetFileAttributesA(char const * path)
{
	struct stat info;
	if (stat(Normalize_Path(path).c_str(), &info) != 0) {
		Set_Errno_Error();
		return(INVALID_FILE_ATTRIBUTES);
	}
	DWORD attributes = S_ISDIR(info.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	if ((info.st_mode & S_IWUSR) == 0) attributes |= FILE_ATTRIBUTE_READONLY;
	LastError = ERROR_SUCCESS;
	return(attributes);
}


BOOL GetFileAttributesExA(char const * path, int, void * information)
{
	if (information == nullptr) return(FALSE);
	struct stat info;
	if (stat(Normalize_Path(path).c_str(), &info) != 0) {
		Set_Errno_Error();
		return(FALSE);
	}
	auto * data = static_cast<WIN32_FILE_ATTRIBUTE_DATA *>(information);
	std::memset(data, 0, sizeof(*data));
	data->dwFileAttributes = S_ISDIR(info.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	data->nFileSizeLow = (DWORD)info.st_size;
	data->nFileSizeHigh = (DWORD)((unsigned long long)info.st_size >> 32);
	Write_Stamp(info.st_mtime, 0, &data->ftLastWriteTime);
	LastError = ERROR_SUCCESS;
	return(TRUE);
}


DWORD GetFileSize(HANDLE handle, DWORD * high)
{
	FileObject * file = As<FileObject>(handle, Kind::File);
	if (file == nullptr || file->Stream == nullptr) {
		LastError = ERROR_INVALID_HANDLE;
		return(INVALID_FILE_SIZE);
	}
	struct stat info;
	if (fstat(fileno(file->Stream), &info) != 0) {
		Set_Errno_Error();
		return(INVALID_FILE_SIZE);
	}
	if (high != nullptr) *high = (DWORD)((unsigned long long)info.st_size >> 32);
	LastError = ERROR_SUCCESS;
	return((DWORD)info.st_size);
}


BOOL FlushFileBuffers(HANDLE handle)
{
	FileObject * file = As<FileObject>(handle, Kind::File);
	if (file == nullptr || file->Stream == nullptr) return(FALSE);
	if (std::fflush(file->Stream) != 0) {
		Set_Errno_Error();
		return(FALSE);
	}
	return(TRUE);
}


BOOL MoveFileExA(char const * from, char const * to, DWORD)
{
	if (rename(Normalize_Path(from).c_str(), Normalize_Path(to).c_str()) != 0) {
		Set_Errno_Error();
		return(FALSE);
	}
	LastError = ERROR_SUCCESS;
	return(TRUE);
}


DWORD SetFilePointer(HANDLE handle, LONG distance, LONG * high, DWORD method)
{
	FileObject * file = As<FileObject>(handle, Kind::File);
	if (file == nullptr || file->Stream == nullptr) {
		LastError = ERROR_INVALID_HANDLE;
		return(INVALID_SET_FILE_POINTER);
	}
	off_t target = distance;
	if (high != nullptr) {
		target = ((off_t)(unsigned long)*high << 32) | (unsigned long)distance;
	}
	int whence = SEEK_SET;
	if (method == FILE_CURRENT) whence = SEEK_CUR;
	else if (method == FILE_END) whence = SEEK_END;
	if (fseeko(file->Stream, target, whence) != 0) {
		Set_Errno_Error();
		return(INVALID_SET_FILE_POINTER);
	}
	off_t const position = ftello(file->Stream);
	if (high != nullptr) *high = (LONG)((unsigned long long)position >> 32);
	LastError = ERROR_SUCCESS;
	return((DWORD)position);
}


BOOL CreateDirectoryA(char const * path, void *)
{
	if (mkdir(Normalize_Path(path).c_str(), 0777) != 0) {
		Set_Errno_Error();
		return(FALSE);
	}
	LastError = ERROR_SUCCESS;
	return(TRUE);
}


BOOL CopyFileA(char const * from, char const * to, BOOL fail_if_exists)
{
	std::string const source = Normalize_Path(from);
	std::string const dest = Normalize_Path(to);
	if (fail_if_exists && access(dest.c_str(), F_OK) == 0) {
		LastError = ERROR_FILE_EXISTS;
		return(FALSE);
	}
	FILE * input = std::fopen(source.c_str(), "rb");
	if (input == nullptr) {
		Set_Errno_Error();
		return(FALSE);
	}
	FILE * output = std::fopen(dest.c_str(), "wb");
	if (output == nullptr) {
		Set_Errno_Error();
		std::fclose(input);
		return(FALSE);
	}
	char buffer[8192];
	bool ok = true;
	while (ok) {
		std::size_t const got = std::fread(buffer, 1, sizeof(buffer), input);
		if (got > 0 && std::fwrite(buffer, 1, got, output) != got) ok = false;
		if (got < sizeof(buffer)) break;
	}
	if (std::ferror(input) || std::ferror(output)) ok = false;
	std::fclose(input);
	std::fclose(output);
	if (!ok) {
		Set_Errno_Error();
		return(FALSE);
	}
	LastError = ERROR_SUCCESS;
	return(TRUE);
}


void GetSystemTime(SYSTEMTIME * time)
{
	if (time == nullptr) return;
	std::time_t const now = std::time(nullptr);
	std::tm fields = {};
	gmtime_r(&now, &fields);
	System_From_Tm(fields, 0, time);
}


void GetLocalTime(SYSTEMTIME * time)
{
	if (time == nullptr) return;
	std::time_t const now = std::time(nullptr);
	std::tm fields = {};
	localtime_r(&now, &fields);
	System_From_Tm(fields, 0, time);
}


BOOL SystemTimeToFileTime(SYSTEMTIME const * time, FILETIME * stamp)
{
	if (time == nullptr) return(FALSE);
	std::time_t seconds = 0;
	if (!Time_From_System(*time, &seconds)) return(FALSE);
	return(Write_Stamp(seconds, time->wMilliseconds, stamp) ? TRUE : FALSE);
}


BOOL FileTimeToSystemTime(FILETIME const * stamp, SYSTEMTIME * time)
{
	if (time == nullptr) return(FALSE);
	std::time_t seconds = 0;
	int milliseconds = 0;
	if (!Read_Stamp(stamp, &seconds, &milliseconds)) return(FALSE);
	std::tm fields = {};
	if (gmtime_r(&seconds, &fields) == nullptr) return(FALSE);
	System_From_Tm(fields, milliseconds, time);
	return(TRUE);
}


BOOL FileTimeToLocalFileTime(FILETIME const * utc, FILETIME * local)
{
	std::time_t seconds = 0;
	int milliseconds = 0;
	if (!Read_Stamp(utc, &seconds, &milliseconds) || local == nullptr) return(FALSE);
	std::tm fields = {};
	if (localtime_r(&seconds, &fields) == nullptr) return(FALSE);
	fields.tm_isdst = 0;
	std::time_t const shifted = timegm(&fields);
	if (shifted == (std::time_t)-1) return(FALSE);
	return(Write_Stamp(shifted, milliseconds, local) ? TRUE : FALSE);
}


void GetSystemTimeAsFileTime(FILETIME * stamp)
{
	SYSTEMTIME time;
	GetSystemTime(&time);
	SystemTimeToFileTime(&time, stamp);
}


DWORD GetTickCount(void)
{
	return(timeGetTime());
}


ULONGLONG GetTickCount64(void)
{
	auto const now = std::chrono::steady_clock::now().time_since_epoch();
	return((ULONGLONG)std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}


BOOL QueryPerformanceCounter(LARGE_INTEGER * counter)
{
	if (counter == nullptr) return(FALSE);
	auto const now = std::chrono::steady_clock::now().time_since_epoch();
	counter->QuadPart = (std::int64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
	return(TRUE);
}


BOOL QueryPerformanceFrequency(LARGE_INTEGER * frequency)
{
	if (frequency == nullptr) return(FALSE);
	frequency->QuadPart = 1000000000;
	return(TRUE);
}


UINT timeBeginPeriod(UINT)
{
	return(0);
}


UINT timeEndPeriod(UINT)
{
	return(0);
}


HANDLE GetCurrentProcess(void)
{
	return((HANDLE)(intptr_t)-1);
}


DWORD GetCurrentProcessId(void)
{
	return((DWORD)getpid());
}


DWORD GetCurrentThreadId(void)
{
	uint64_t tid = 0;
	pthread_threadid_np(nullptr, &tid);
	return((DWORD)tid);
}


BOOL SetProcessInformation(HANDLE, int, void const *, DWORD)
{
	return(TRUE);
}


BOOL GetDiskFreeSpaceExA(char const * path, ULARGE_INTEGER * available, ULARGE_INTEGER * total, ULARGE_INTEGER * free_total)
{
	std::string const text = (path == nullptr || path[0] == '\0') ? std::string(".") : Normalize_Path(path);
	struct statvfs info;
	if (statvfs(text.c_str(), &info) != 0) {
		Set_Errno_Error();
		return(FALSE);
	}
	unsigned long long const bytes = (unsigned long long)info.f_blocks * (unsigned long long)info.f_frsize;
	unsigned long long const free_bytes = (unsigned long long)info.f_bfree * (unsigned long long)info.f_frsize;
	unsigned long long const available_bytes = (unsigned long long)info.f_bavail * (unsigned long long)info.f_frsize;
	if (available != nullptr) available->QuadPart = available_bytes;
	if (total != nullptr) total->QuadPart = bytes;
	if (free_total != nullptr) free_total->QuadPart = free_bytes;
	LastError = ERROR_SUCCESS;
	return(TRUE);
}


void GlobalMemoryStatus(MEMORYSTATUS * status)
{
	if (status == nullptr) return;
	uint64_t bytes = 0;
	std::size_t length = sizeof(bytes);
	sysctlbyname("hw.memsize", &bytes, &length, nullptr, 0);
	status->dwMemoryLoad = 0;
	status->dwTotalPhys = (SIZE_T)bytes;
	status->dwAvailPhys = (SIZE_T)bytes;
	status->dwTotalPageFile = (SIZE_T)bytes;
	status->dwAvailPageFile = (SIZE_T)bytes;
	status->dwTotalVirtual = (SIZE_T)bytes;
	status->dwAvailVirtual = (SIZE_T)bytes;
}


HMODULE GetModuleHandleA(char const * name)
{
	static char executable;
	if (name == nullptr) return(&executable);
	return(nullptr);
}


DWORD GetModuleFileNameA(HMODULE module, char * buffer, DWORD size)
{
	if (buffer == nullptr || size == 0) return(0);
	if (module != nullptr && module != GetModuleHandleA(nullptr)) return(0);

	char raw[PATH_MAX];
	uint32_t raw_size = sizeof(raw);
	if (_NSGetExecutablePath(raw, &raw_size) != 0) return(0);
	char resolved[PATH_MAX];
	if (realpath(raw, resolved) == nullptr) {
		std::snprintf(resolved, sizeof(resolved), "%s", raw);
	}
	std::size_t const length = std::strlen(resolved);
	if (length + 1 > size) {
		std::memcpy(buffer, resolved, size - 1);
		buffer[size - 1] = '\0';
		LastError = ERROR_INSUFFICIENT_BUFFER;
		return(size);
	}
	std::memcpy(buffer, resolved, length + 1);
	LastError = ERROR_SUCCESS;
	return((DWORD)length);
}


UINT GetWindowsDirectoryA(char * buffer, UINT size)
{
	char const path[] = "/System/Library";
	UINT const length = (UINT)(sizeof(path) - 1);
	if (buffer == nullptr || size <= length) return(length + 1);
	std::memcpy(buffer, path, length + 1);
	return(length);
}


#if defined(__APPLE__)
extern int Language_Load_String(unsigned id, char * buffer, int size);
#endif


HMODULE LoadLibraryA(char const * name)
{
	std::string const text = Normalize_Path(name);
	bool const language = text.size() >= 12 && strcasecmp(text.c_str() + text.size() - 12, "Language.dll") == 0;
	if (language && (text.size() == 12 || text[text.size() - 13] == '/')) {
		static char module;
		LastError = ERROR_SUCCESS;
		return(&module);
	}
	LastError = ERROR_FILE_NOT_FOUND;
	return(nullptr);
}


int LoadStringA(HINSTANCE, UINT id, char * buffer, int size)
{
#if defined(__APPLE__)
	return(Language_Load_String(id, buffer, size));
#else
	(void)id;
	(void)buffer;
	(void)size;
	return(0);
#endif
}


UINT GetACP(void)
{
	return(CP_UTF8);
}


UINT GetOEMCP(void)
{
	return(CP_UTF8);
}


DWORD GetFileVersionInfoSizeA(char const *, DWORD *)
{
	return(0);
}


BOOL GetFileVersionInfoA(char const *, DWORD, DWORD, void *)
{
	return(FALSE);
}


BOOL VerQueryValueA(void const *, char const *, void **, UINT *)
{
	return(FALSE);
}


FARPROC GetProcAddress(HMODULE, char const *)
{
	return(nullptr);
}


LONG RegOpenKeyExA(HKEY, char const *, DWORD, DWORD, HKEY *)
{
	return(ERROR_FILE_NOT_FOUND);
}


LONG RegQueryValueExA(HKEY, char const *, DWORD *, DWORD *, BYTE *, DWORD *)
{
	return(ERROR_FILE_NOT_FOUND);
}


LONG RegCloseKey(HKEY)
{
	return(ERROR_SUCCESS);
}


BOOL AllocConsole(void)
{
	return(TRUE);
}


BOOL AttachConsole(DWORD)
{
	return(isatty(STDERR_FILENO) ? TRUE : FALSE);
}


BOOL SetConsoleTitleA(char const *)
{
	return(TRUE);
}


BOOL SetConsoleOutputCP(UINT)
{
	return(TRUE);
}


BOOL SetConsoleCP(UINT)
{
	return(TRUE);
}


int freopen_s(FILE ** opened, char const * path, char const * mode, FILE * stream)
{
	if (opened == nullptr) return(EINVAL);
	std::string const text = Normalize_Path(path);
	if (text == "CONOUT$" || text == "CONIN$") {
		*opened = stream;
		return(0);
	}
	FILE * result = std::freopen(text.c_str(), mode, stream);
	*opened = result;
	return(result == nullptr ? errno : 0);
}


HANDLE GetStdHandle(DWORD which)
{
	int const index = Standard_Index(which);
	if (index < 0) return(INVALID_HANDLE_VALUE);
	if (StandardHandles[index] != nullptr) return(StandardHandles[index]);
	if (which == STD_INPUT_HANDLE) return(Console_File(stdin));
	if (which == STD_ERROR_HANDLE) return(Console_File(stderr));
	return(Console_File(stdout));
}


BOOL SetStdHandle(DWORD which, HANDLE handle)
{
	int const index = Standard_Index(which);
	if (index < 0) return(FALSE);
	StandardHandles[index] = handle;
	return(TRUE);
}


BOOL GetConsoleScreenBufferInfo(HANDLE, CONSOLE_SCREEN_BUFFER_INFO *)
{
	return(FALSE);
}


BOOL SetConsoleScreenBufferSize(HANDLE, COORD)
{
	return(TRUE);
}


HWND GetConsoleWindow(void)
{
	return(nullptr);
}


HMENU GetSystemMenu(HWND, BOOL)
{
	return(nullptr);
}


BOOL DeleteMenu(HMENU, UINT, UINT)
{
	return(FALSE);
}


BOOL WriteConsoleA(HANDLE handle, void const * text, DWORD chars, DWORD * written, void *)
{
	return(WriteFile(handle, text, chars, written, nullptr));
}


void OutputDebugStringA(char const * text)
{
	if (text == nullptr) return;
	std::fputs(text, stderr);
}


BOOL IsDebuggerPresent(void)
{
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
	struct kinfo_proc info;
	std::memset(&info, 0, sizeof(info));
	std::size_t size = sizeof(info);
	if (sysctl(mib, 4, &info, &size, nullptr, 0) != 0) return(FALSE);
	return((info.kp_proc.p_flag & P_TRACED) != 0 ? TRUE : FALSE);
}


wchar_t const * GetCommandLineW(void)
{
	static std::wstring line;
	static bool ready = false;
	if (!ready) {
		int const argc = *_NSGetArgc();
		char ** argv = *_NSGetArgv();
		for (int index = 0; index < argc; index++) {
			if (index != 0) line.push_back(L' ');
			char const * text = argv[index] != nullptr ? argv[index] : "";
			while (*text != '\0') {
				line.push_back((unsigned char)*text);
				text++;
			}
		}
		ready = true;
	}
	return(line.c_str());
}


wchar_t ** CommandLineToArgvW(wchar_t const *, int * count)
{
	int const argc = *_NSGetArgc();
	char ** argv = *_NSGetArgv();
	auto * list = new ArgumentList;
	for (int index = 0; index < argc; index++) {
		std::wstring wide;
		char const * text = argv[index] != nullptr ? argv[index] : "";
		while (*text != '\0') {
			wide.push_back((unsigned char)*text);
			text++;
		}
		list->Storage.push_back(std::move(wide));
	}
	for (std::wstring & item : list->Storage) {
		list->Pointers.push_back(item.data());
	}
	list->Pointers.push_back(nullptr);
	wchar_t ** result = list->Pointers.data();
	std::lock_guard<std::mutex> lock(Argument_Lock());
	Argument_Map()[result] = list;
	if (count != nullptr) *count = argc;
	return(result);
}


void LocalFree(void * memory)
{
	std::lock_guard<std::mutex> lock(Argument_Lock());
	auto found = Argument_Map().find(memory);
	if (found == Argument_Map().end()) return;
	delete found->second;
	Argument_Map().erase(found);
}


HDC GetDC(HWND)
{
	static ContextObject screen;
	screen.Permanent = true;
	return(&screen);
}


int ReleaseDC(HWND, HDC)
{
	return(1);
}


HDC CreateCompatibleDC(HDC)
{
	return(new ContextObject);
}


BOOL DeleteDC(HDC dc)
{
	ContextObject * context = As<ContextObject>(dc, Kind::Context);
	if (context == nullptr || context->Permanent) return(context != nullptr ? TRUE : FALSE);
	delete context;
	return(TRUE);
}


HBITMAP CreateDIBSection(HDC, BITMAPINFO const * info, UINT, void ** bits, HANDLE, DWORD)
{
	if (info == nullptr) return(nullptr);
	int const width = info->bmiHeader.biWidth;
	int height = info->bmiHeader.biHeight;
	if (height < 0) height = -height;
	int const depth = info->bmiHeader.biBitCount == 0 ? 16 : info->bmiHeader.biBitCount;
	if (width <= 0 || height <= 0) return(nullptr);

	auto * bitmap = new BitmapObject;
	bitmap->Width = width;
	bitmap->Height = height;
	bitmap->Pitch = ((width * depth + 31) / 32) * 4;
	bitmap->Pixels.assign((std::size_t)bitmap->Pitch * (std::size_t)height, 0);
	if (bits != nullptr) *bits = bitmap->Pixels.data();
	return(bitmap);
}


HGDIOBJ SelectObject(HDC dc, HGDIOBJ object)
{
	ContextObject * context = As<ContextObject>(dc, Kind::Context);
	if (context == nullptr || object == nullptr) return(nullptr);
	auto * item = static_cast<PlatformObject *>(object);
	if (item->Type == Kind::Bitmap) {
		BitmapObject * previous = context->Selected;
		context->Selected = static_cast<BitmapObject *>(item);
		return(previous);
	}
	if (item->Type == Kind::Font) {
		FontObject * previous = context->Font;
		context->Font = static_cast<FontObject *>(item);
		return(previous);
	}
	return(nullptr);
}


BOOL DeleteObject(HGDIOBJ object)
{
	if (object == nullptr) return(FALSE);
	auto * item = static_cast<PlatformObject *>(object);
	if (item->Type != Kind::Bitmap && item->Type != Kind::Font) return(FALSE);
	delete item;
	return(TRUE);
}


int GetObject(HGDIOBJ object, int size, void * into)
{
	BitmapObject * bitmap = As<BitmapObject>(object, Kind::Bitmap);
	if (bitmap == nullptr || into == nullptr || size < (int)sizeof(DIBSECTION)) return(0);
	DIBSECTION section;
	std::memset(&section, 0, sizeof(section));
	section.dsBm.bmWidth = bitmap->Width;
	section.dsBm.bmHeight = bitmap->Height;
	section.dsBm.bmWidthBytes = bitmap->Pitch;
	section.dsBm.bmPlanes = 1;
	section.dsBm.bmBitsPixel = 16;
	section.dsBm.bmBits = bitmap->Pixels.data();
	section.dsBmih.biSize = sizeof(BITMAPINFOHEADER);
	section.dsBmih.biWidth = bitmap->Width;
	section.dsBmih.biHeight = -bitmap->Height;
	section.dsBmih.biPlanes = 1;
	section.dsBmih.biBitCount = 16;
	std::memcpy(into, &section, sizeof(section));
	return((int)sizeof(section));
}


BOOL GdiFlush(void)
{
	return(TRUE);
}


int SetStretchBltMode(HDC dc, int mode)
{
	ContextObject * context = As<ContextObject>(dc, Kind::Context);
	int const previous = context != nullptr ? context->StretchMode : COLORONCOLOR;
	if (context != nullptr) context->StretchMode = mode;
	return(previous);
}


BOOL StretchBlt(HDC dest, int x, int y, int width, int height, HDC source, int sx, int sy, int swidth, int sheight, DWORD)
{
	ContextObject * dest_context = As<ContextObject>(dest, Kind::Context);
	ContextObject * source_context = As<ContextObject>(source, Kind::Context);
	if (dest_context == nullptr || source_context == nullptr) return(FALSE);
	BitmapObject * dest_bitmap = dest_context->Selected;
	BitmapObject * source_bitmap = source_context->Selected;
	if (dest_bitmap == nullptr || source_bitmap == nullptr) return(FALSE);
	if (width <= 0 || height <= 0 || swidth <= 0 || sheight <= 0) return(FALSE);

	std::vector<unsigned char> copy;
	unsigned char const * source_pixels = source_bitmap->Pixels.data();
	if (source_bitmap == dest_bitmap) {
		copy = source_bitmap->Pixels;
		source_pixels = copy.data();
	}

	for (int row = 0; row < height; row++) {
		int const dy = y + row;
		int const sy_row = sy + (int)((long long)row * sheight / height);
		if (dy < 0 || dy >= dest_bitmap->Height || sy_row < 0 || sy_row >= source_bitmap->Height) continue;
		auto * drow = (std::uint16_t *)(dest_bitmap->Pixels.data() + (std::size_t)dy * (std::size_t)dest_bitmap->Pitch);
		auto const * srow = (std::uint16_t const *)(source_pixels + (std::size_t)sy_row * (std::size_t)source_bitmap->Pitch);
		for (int column = 0; column < width; column++) {
			int const dx = x + column;
			int const sx_column = sx + (int)((long long)column * swidth / width);
			if (dx < 0 || dx >= dest_bitmap->Width || sx_column < 0 || sx_column >= source_bitmap->Width) continue;
			drow[dx] = srow[sx_column];
		}
	}
	return(TRUE);
}


HFONT CreateFontA(int height, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, char const *)
{
	auto * font = new FontObject;
	font->Height = height;
	return(font);
}


HFONT CreateFontIndirectA(LOGFONT const * description)
{
	auto * font = new FontObject;
	if (description != nullptr) font->Height = description->lfHeight;
	return(font);
}


COLORREF SetTextColor(HDC, COLORREF color)
{
	return(color);
}


int SetBkMode(HDC, int mode)
{
	return(mode);
}


UINT SetTextAlign(HDC, UINT align)
{
	return(align);
}


// No glyphs are written. The call succeeds so the surrounding layout still runs.
BOOL TextOutA(HDC, int, int, char const *, int)
{
	return(TRUE);
}


BOOL GetTextExtentPoint32A(HDC dc, char const *, int length, SIZE * size)
{
	if (size == nullptr) return(FALSE);
	int const height = Font_Height(As<ContextObject>(dc, Kind::Context));
	int const advance = height / 2 > 0 ? height / 2 : 1;
	size->cx = length > 0 ? length * advance : 0;
	size->cy = height;
	return(TRUE);
}


BOOL GetTextMetricsA(HDC dc, TEXTMETRIC * metrics)
{
	if (metrics == nullptr) return(FALSE);
	std::memset(metrics, 0, sizeof(*metrics));
	int const height = Font_Height(As<ContextObject>(dc, Kind::Context));
	metrics->tmHeight = height;
	metrics->tmAveCharWidth = height / 2 > 0 ? height / 2 : 1;
	return(TRUE);
}


int SaveDC(HDC)
{
	return(1);
}


BOOL RestoreDC(HDC, int)
{
	return(TRUE);
}


int SetGraphicsMode(HDC, int mode)
{
	return(mode);
}


BOOL ModifyWorldTransform(HDC, void const *, DWORD)
{
	return(TRUE);
}


BOOL SetViewportOrgEx(HDC, int, int, POINT *)
{
	return(TRUE);
}


BOOL SetWindowOrgEx(HDC, int, int, POINT *)
{
	return(TRUE);
}


BOOL DPtoLP(HDC, POINT *, int)
{
	return(TRUE);
}


SHORT GetKeyState(int virtualkey)
{
	int count = 0;
	bool const * keys = SDL_GetKeyboardState(&count);
	auto down = [&](SDL_Scancode scancode) {
		return(keys != nullptr && (int)scancode < count && keys[scancode]);
	};
	bool held = false;
	switch (virtualkey) {
		case VK_LSHIFT: held = down(SDL_SCANCODE_LSHIFT); break;
		case VK_RSHIFT: held = down(SDL_SCANCODE_RSHIFT); break;
		case VK_SHIFT: held = down(SDL_SCANCODE_LSHIFT) || down(SDL_SCANCODE_RSHIFT); break;
		case VK_LCONTROL: held = down(SDL_SCANCODE_LCTRL); break;
		case VK_RCONTROL: held = down(SDL_SCANCODE_RCTRL); break;
		case VK_CONTROL: held = down(SDL_SCANCODE_LCTRL) || down(SDL_SCANCODE_RCTRL); break;
		case VK_LMENU: held = down(SDL_SCANCODE_LALT); break;
		case VK_RMENU: held = down(SDL_SCANCODE_RALT); break;
		case VK_MENU: held = down(SDL_SCANCODE_LALT) || down(SDL_SCANCODE_RALT); break;
		case VK_CAPITAL: return((SDL_GetModState() & SDL_KMOD_CAPS) != 0 ? 1 : 0);
		case VK_NUMLOCK: return((SDL_GetModState() & SDL_KMOD_NUM) != 0 ? 1 : 0);
		case VK_SCROLL: return((SDL_GetModState() & SDL_KMOD_SCROLL) != 0 ? 1 : 0);
		default: break;
	}
	SDL_MouseButtonFlags const buttons = SDL_GetGlobalMouseState(nullptr, nullptr);
	switch (virtualkey) {
		case VK_LBUTTON: held = (buttons & SDL_BUTTON_LMASK) != 0; break;
		case VK_RBUTTON: held = (buttons & SDL_BUTTON_RMASK) != 0; break;
		case VK_MBUTTON: held = (buttons & SDL_BUTTON_MMASK) != 0; break;
		case VK_XBUTTON1: held = (buttons & SDL_BUTTON_X1MASK) != 0; break;
		case VK_XBUTTON2: held = (buttons & SDL_BUTTON_X2MASK) != 0; break;
		default: break;
	}
	return(held ? (SHORT)0x8000 : 0);
}


int GetSystemMetrics(int index)
{
	if (index == SM_CXDRAG || index == SM_CYDRAG) return(4);
	return(0);
}


LRESULT DefWindowProcW(HWND, UINT, WPARAM, LPARAM)
{
	return(0);
}


void * _aligned_malloc(std::size_t size, std::size_t alignment)
{
	if (size == 0) return(nullptr);
	if (alignment < sizeof(void *)) alignment = sizeof(void *);
	std::size_t rounded = 1;
	while (rounded < alignment) rounded <<= 1;
	void * memory = nullptr;
	if (posix_memalign(&memory, rounded, size) != 0) return(nullptr);
	std::lock_guard<std::mutex> lock(Allocation_Lock());
	Allocation_Map()[memory] = Allocation{ size, rounded };
	return(memory);
}


void _aligned_free(void * memory)
{
	if (memory == nullptr) return;
	{
		std::lock_guard<std::mutex> lock(Allocation_Lock());
		Allocation_Map().erase(memory);
	}
	std::free(memory);
}


void * _aligned_realloc(void * memory, std::size_t size, std::size_t alignment)
{
	if (memory == nullptr) return(_aligned_malloc(size, alignment));
	if (size == 0) {
		_aligned_free(memory);
		return(nullptr);
	}
	std::size_t previous = 0;
	{
		std::lock_guard<std::mutex> lock(Allocation_Lock());
		auto found = Allocation_Map().find(memory);
		if (found != Allocation_Map().end()) previous = found->second.Size;
	}
	void * next = _aligned_malloc(size, alignment);
	if (next == nullptr) return(nullptr);
	std::memcpy(next, memory, previous < size ? previous : size);
	_aligned_free(memory);
	return(next);
}


void AcquireSRWLockExclusive(SRWLOCK * lock)
{
	if (lock != nullptr) pthread_rwlock_wrlock(&lock->Lock);
}


void ReleaseSRWLockExclusive(SRWLOCK * lock)
{
	if (lock != nullptr) pthread_rwlock_unlock(&lock->Lock);
}


int GetDateFormatA(DWORD, DWORD, SYSTEMTIME const * time, char const *, char * buffer, int size)
{
	if (time == nullptr || buffer == nullptr || size <= 0) return(0);
	int const written = std::snprintf(buffer, (std::size_t)size, "%04u-%02u-%02u", time->wYear, time->wMonth, time->wDay);
	if (written < 0 || written >= size) return(0);
	return(written + 1);
}


int GetTimeFormatA(DWORD, DWORD flags, SYSTEMTIME const * time, char const *, char * buffer, int size)
{
	if (time == nullptr || buffer == nullptr || size <= 0) return(0);
	int written = 0;
	if ((flags & TIME_NOSECONDS) != 0) {
		written = std::snprintf(buffer, (std::size_t)size, "%02u:%02u", time->wHour, time->wMinute);
	} else {
		written = std::snprintf(buffer, (std::size_t)size, "%02u:%02u:%02u", time->wHour, time->wMinute, time->wSecond);
	}
	if (written < 0 || written >= size) return(0);
	return(written + 1);
}


unsigned int _controlfp(unsigned int, unsigned int)
{
	return(0x0009001Fu);
}


int _getch(void)
{
	return(std::getchar());
}


DWORD FormatMessageA(DWORD, void const *, DWORD message, DWORD, char * buffer, DWORD size, void *)
{
	if (buffer == nullptr || size == 0) return(0);
	char const * text = nullptr;
	if (message == ERROR_FILE_NOT_FOUND) text = "The system cannot find the file specified.\r\n";
	else if (message == ERROR_ACCESS_DENIED) text = "Access is denied.\r\n";
	else if (message == ERROR_ALREADY_EXISTS) text = "Cannot create a file when that file already exists.\r\n";
	else if (message == ERROR_FILE_EXISTS) text = "The file exists.\r\n";
	int written = 0;
	if (text != nullptr) written = std::snprintf(buffer, size, "%s", text);
	else written = std::snprintf(buffer, size, "Error %lu.\r\n", (unsigned long)message);
	if (written < 0 || (DWORD)written >= size) {
		buffer[size - 1] = '\0';
		return(0);
	}
	return((DWORD)written);
}


HKL GetKeyboardLayout(DWORD)
{
	static char layout;
	return(&layout);
}


UINT MapVirtualKeyExW(UINT, UINT, HKL)
{
	return(0);
}


wchar_t * CharUpperW(wchar_t * text)
{
	if (text == nullptr) return(nullptr);
	for (wchar_t * cursor = text; *cursor != L'\0'; cursor++) {
		if (*cursor >= L'a' && *cursor <= L'z') {
			*cursor = (wchar_t)(*cursor - L'a' + L'A');
		}
	}
	return(text);
}


int WideCharToMultiByte(UINT, DWORD, wchar_t const * wide, int wide_count, char * buffer, int buffer_size, char const *, int *)
{
	if (wide == nullptr) return(0);

	bool const terminated = wide_count < 0;
	if (terminated) {
		wide_count = 0;
		while (wide[wide_count] != L'\0') wide_count++;
	}

	std::string encoded;
	encoded.reserve((std::size_t)wide_count * 4);
	for (int index = 0; index < wide_count; index++) {
		unsigned int const code = (unsigned int)wide[index];
		char bytes[4];
		int count = 0;
		if (code < 0x80) {
			bytes[0] = (char)code;
			count = 1;
		} else if (code < 0x800) {
			bytes[0] = (char)(0xC0 | (code >> 6));
			bytes[1] = (char)(0x80 | (code & 0x3F));
			count = 2;
		} else if (code < 0x10000) {
			bytes[0] = (char)(0xE0 | (code >> 12));
			bytes[1] = (char)(0x80 | ((code >> 6) & 0x3F));
			bytes[2] = (char)(0x80 | (code & 0x3F));
			count = 3;
		} else {
			bytes[0] = (char)(0xF0 | (code >> 18));
			bytes[1] = (char)(0x80 | ((code >> 12) & 0x3F));
			bytes[2] = (char)(0x80 | ((code >> 6) & 0x3F));
			bytes[3] = (char)(0x80 | (code & 0x3F));
			count = 4;
		}
		encoded.append(bytes, (std::size_t)count);
	}

	// A NULL buffer asks for the required size, including the terminator a negative
	// count requests, without writing.
	if (buffer == nullptr) {
		return((int)encoded.size() + (terminated ? 1 : 0));
	}
	if (buffer_size <= 0) return(0);

	std::size_t const limit = (std::size_t)buffer_size - 1;
	std::size_t const copy = encoded.size() < limit ? encoded.size() : limit;
	std::memcpy(buffer, encoded.data(), copy);
	buffer[copy] = '\0';
	return((int)copy);
}


HANDLE FindFirstFileA(char const * pattern, WIN32_FIND_DATAA * found)
{
	if (pattern == nullptr || found == nullptr) return(INVALID_HANDLE_VALUE);

	std::string text = Normalize_Path(pattern);
	std::string directory = ".";
	std::string name = text;
	std::string::size_type const slash = text.find_last_of('/');
	if (slash != std::string::npos) {
		directory = slash == 0 ? "/" : text.substr(0, slash);
		name = text.substr(slash + 1);
	}

	bool const wild = name.find_first_of("*?") != std::string::npos;
	if (!wild) {
		std::string const path = (directory == ".") ? name : directory + "/" + name;
		struct stat info;
		if (stat(path.c_str(), &info) != 0) return(INVALID_HANDLE_VALUE);
		Fill_Found(found, name.c_str(), info);
		return(new Finder{ nullptr, directory, {} });
	}

	DIR * opened = opendir(directory.c_str());
	if (opened == nullptr) return(INVALID_HANDLE_VALUE);
	Finder * finder = new Finder{ opened, directory, name };
	if (!FindNextFileA(finder, found)) {
		FindClose(finder);
		return(INVALID_HANDLE_VALUE);
	}
	return(finder);
}


HANDLE FindFirstFile(char const * pattern, WIN32_FIND_DATAA * found)
{
	return(FindFirstFileA(pattern, found));
}


int FindNextFileA(HANDLE handle, WIN32_FIND_DATAA * found)
{
	Finder * finder = static_cast<Finder *>(handle);
	if (finder == nullptr || finder->Directory == nullptr || found == nullptr) return(0);

	while (dirent * entry = readdir(finder->Directory)) {
		if (fnmatch(finder->Pattern.c_str(), entry->d_name, 0) != 0) continue;
		std::string const path = Join_Path(finder->DirectoryPath, entry->d_name);
		struct stat info;
		if (stat(path.c_str(), &info) != 0) continue;
		Fill_Found(found, entry->d_name, info);
		return(1);
	}
	return(0);
}


int FindNextFile(HANDLE handle, WIN32_FIND_DATAA * found)
{
	return(FindNextFileA(handle, found));
}


int FindClose(HANDLE handle)
{
	Finder * finder = static_cast<Finder *>(handle);
	if (finder == nullptr) return(0);
	if (finder->Directory != nullptr) closedir(finder->Directory);
	delete finder;
	return(1);
}

#endif
