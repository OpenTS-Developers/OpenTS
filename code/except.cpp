/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/Except.cpp                             $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/07/02 12:28p                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *                                                                                             *
 * Exception_Proc -- Windows dialog callback for the exception dialog                          *
 * Exception_Dialog -- Brings up the exception options dialog.                                 *
 * Add_Txt -- Add the given text to the machine state dump buffer.                             *
 * Dump_Exception_Info -- Dump machine state information into a buffer                         *
 * Exception_Handler -- Exception handler filter function                                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "except.h"

#include "misc.h"
#include "resource.h"
#include "version.h"
#include "win.h"

#include <dbghelp.h>
#include <tlhelp32.h>

#include <atomic>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <eh.h>
#include <exception>

#define MS_VC_THREAD_NAME_EXCEPTION			0x406D1388
#define MS_CPP_EXCEPTION					0xE06D7363

#define EXCEPTION_REPORT_SIZE				0x20000
#define EXCEPTION_SCRATCH_SIZE				2048
#define NUM_CODE_BYTES 32
#define MAX_STACK_DUMP						512
#define MAX_FRAME_DEPTH						128
#define MAX_MODULE_COUNT					256
#define EXCEPTION_STACK_GUARANTEE			(64 * 1024)
#define DUMPER_TIMEOUT_MS					60000
#define LOG_TAIL_BYTES						(256 * 1024)
#define EXCEPTION_FOLDER_DAYS				30

// DbgHelp is not reentrant, and it is also the part of the report most likely to fault on a
// corrupt stack. Everything that enters it, MiniDumpWriteDump included, holds this.
//
// Lock order for the whole subsystem is the lock free entry gates, then this. Nothing on the
// exception path may take a lock owned by a game subsystem or by the logger, because the
// thread holding it may be the one that just crashed.
static CRITICAL_SECTION DbgHelpLock;
static bool DbgHelpLockReady = false;

// The symbol handler is prepared during installation rather than on demand, so that a crash
// never has to take the loader lock. Until that finishes the filter has to report without it,
// including when the fault happened inside the preparation itself.
enum class SymbolStateType
{
	Uninitialized,
	Initializing,
	Ready
};

static std::atomic<SymbolStateType> SymbolState { SymbolStateType::Uninitialized };
static bool SymbolsVerified = false;

// Entry gates. The first thread to fault owns the report and the exit; a second one parks
// rather than racing it, and an owner that faults again degrades instead of looping.
static std::atomic<DWORD> FirstCrashThreadId { 0 };
static std::atomic<DWORD> DumpingThreadId { 0 };
static std::atomic<DWORD> DumperThreadId { 0 };
static std::atomic<int> RecursionCount { 0 };

static HANDLE DumperThread = NULL;
static HANDLE DumperWakeEvent = NULL;
static HANDLE ArtifactsWrittenEvent = NULL;

// The crashing thread's machine state is copied here before it hands off, because a pointer
// into the faulting thread's own stack would not outlive the handoff.
static EXCEPTION_RECORD SavedRecord;
static CONTEXT SavedContext;
static EXCEPTION_POINTERS SavedPointers;
static DWORD CrashedThreadId = 0;
static SYSTEMTIME CrashTime;

static DWORD MainThreadId = 0;
static char ExecutableDirectory[MAX_PATH];
static char ExecutablePath[MAX_PATH];
static char ArtifactFolder[MAX_PATH];
static char RegisteredLogFile[MAX_PATH];
static char FullDumpPath[MAX_PATH];

// Report storage. Allocating any of this at crash time would depend on the heap that the
// crash may have been caused by.
static char ExceptionReport[EXCEPTION_REPORT_SIZE];
static unsigned ExceptionReportLength = 0;
static bool ExceptionReportFinished = false;
static char PendingMessage[1024];

struct ModuleEntryType
{
	DWORD_PTR Base;
	DWORD_PTR End;
	char Name[64];
	char Path[MAX_PATH];
};

static ModuleEntryType ModuleTable[MAX_MODULE_COUNT];
static unsigned ModuleCount = 0;

struct ExceptionCodeType
{
	DWORD Code;
	char const * Name;
	char const * Description;
};

static ExceptionCodeType const ExceptionCodes[] = {
	{ EXCEPTION_ACCESS_VIOLATION, "EXCEPTION_ACCESS_VIOLATION",
		"The thread tried to read from or write to a virtual address it does not have access to." },
	{ EXCEPTION_DATATYPE_MISALIGNMENT, "EXCEPTION_DATATYPE_MISALIGNMENT",
		"The thread tried to read or write misaligned data on hardware that does not fix it up." },
	{ EXCEPTION_BREAKPOINT, "EXCEPTION_BREAKPOINT",
		"A breakpoint was encountered." },
	{ EXCEPTION_SINGLE_STEP, "EXCEPTION_SINGLE_STEP",
		"A trace trap signaled that one instruction has executed." },
	{ EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "EXCEPTION_ARRAY_BOUNDS_EXCEEDED",
		"The thread tried to access an array element that is out of bounds." },
	{ EXCEPTION_FLT_DENORMAL_OPERAND, "EXCEPTION_FLT_DENORMAL_OPERAND",
		"An operand in a floating point operation is denormal." },
	{ EXCEPTION_FLT_DIVIDE_BY_ZERO, "EXCEPTION_FLT_DIVIDE_BY_ZERO",
		"The thread tried to divide a floating point value by a floating point zero." },
	{ EXCEPTION_FLT_INEXACT_RESULT, "EXCEPTION_FLT_INEXACT_RESULT",
		"The result of a floating point operation cannot be represented exactly." },
	{ EXCEPTION_FLT_INVALID_OPERATION, "EXCEPTION_FLT_INVALID_OPERATION",
		"A floating point exception that is not otherwise represented." },
	{ EXCEPTION_FLT_OVERFLOW, "EXCEPTION_FLT_OVERFLOW",
		"The exponent of a floating point operation is greater than its type allows." },
	{ EXCEPTION_FLT_STACK_CHECK, "EXCEPTION_FLT_STACK_CHECK",
		"The floating point stack overflowed or underflowed." },
	{ EXCEPTION_FLT_UNDERFLOW, "EXCEPTION_FLT_UNDERFLOW",
		"The exponent of a floating point operation is less than its type allows." },
	{ EXCEPTION_INT_DIVIDE_BY_ZERO, "EXCEPTION_INT_DIVIDE_BY_ZERO",
		"The thread tried to divide an integer by zero." },
	{ EXCEPTION_INT_OVERFLOW, "EXCEPTION_INT_OVERFLOW",
		"An integer operation carried out of the most significant bit." },
	{ EXCEPTION_PRIV_INSTRUCTION, "EXCEPTION_PRIV_INSTRUCTION",
		"The thread tried to execute an instruction not allowed in the current machine mode." },
	{ EXCEPTION_IN_PAGE_ERROR, "EXCEPTION_IN_PAGE_ERROR",
		"A page that was not present could not be loaded, so the access failed." },
	{ EXCEPTION_ILLEGAL_INSTRUCTION, "EXCEPTION_ILLEGAL_INSTRUCTION",
		"The thread tried to execute an invalid instruction." },
	{ EXCEPTION_NONCONTINUABLE_EXCEPTION, "EXCEPTION_NONCONTINUABLE_EXCEPTION",
		"The thread tried to continue after an exception that cannot be continued from." },
	{ EXCEPTION_STACK_OVERFLOW, "EXCEPTION_STACK_OVERFLOW",
		"The thread used up its stack." },
	{ EXCEPTION_INVALID_DISPOSITION, "EXCEPTION_INVALID_DISPOSITION",
		"An exception handler returned an invalid disposition to the dispatcher." },
	{ EXCEPTION_GUARD_PAGE, "EXCEPTION_GUARD_PAGE",
		"The thread accessed memory allocated with the guard page modifier." },
	{ EXCEPTION_INVALID_HANDLE, "EXCEPTION_INVALID_HANDLE",
		"The thread used a handle that was invalid or of the wrong type." },
	{ CONTROL_C_EXIT, "CONTROL_C_EXIT",
		"The application was ended by a console control break." },
	{ MS_CPP_EXCEPTION, "EXCEPTION_CPP_UNHANDLED",
		"A C++ exception was thrown and nothing caught it." },
	{ 0xC0000409, "STATUS_STACK_BUFFER_OVERRUN",
		"A security check detected a buffer overrun or a comparable fatal condition." },
	{ 0xC0000374, "STATUS_HEAP_CORRUPTION",
		"The heap was found corrupt. The damage was usually done well before this point." },
	{ EXCEPTION_OPENTS_FATAL, "OPENTS_FATAL_ERROR",
		"The engine reported an unrecoverable error and ended the process itself." },
	{ EXCEPTION_OPENTS_TERMINATE, "OPENTS_TERMINATE",
		"The C++ runtime called terminate, usually for an uncaught exception or a failed allocation." },
	{ EXCEPTION_OPENTS_PURECALL, "OPENTS_PURE_VIRTUAL_CALL",
		"A pure virtual function was called, usually on a partly built or already destroyed object." },
	{ EXCEPTION_OPENTS_INVALID_PARAMETER, "OPENTS_INVALID_PARAMETER",
		"A C runtime function was called with an argument it rejects." }
};

// Test faults are requested by name so that every path through this file can be reached
// without the game data the engine would otherwise need to reach them. The two paths that
// only exist while this subsystem is starting up are requested through the environment
// instead, because the launch options have not been read yet when they run.
static char TestMode[32];
static bool TestSectionFault = false;
static bool TestDumperFault = false;

static bool Resolve_Symbol(DWORD_PTR address, char * name, unsigned name_size, DWORD_PTR * displacement);
static bool Resolve_Line(DWORD_PTR address, char * file, unsigned file_size, unsigned * line);
static void Write_Crash_Artifacts(EXCEPTION_POINTERS * e_info, DWORD crashed_thread);
static void Show_Recursion_Notice(void);
[[noreturn]] static void Show_Exception_Dialog(void);


/// <summary>
/// Ends the process immediately, without unwinding.
/// </summary>
/// <remarks>
/// Returning through the normal exit path would run atexit handlers and static destructors
/// against the subsystems whose state the crash calls into question.
/// </remarks>
[[noreturn]] static void Terminate_Now(void)
{
	TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
	__assume(0);
}


/// <summary>
/// Appends printf style text to the machine state report.
/// </summary>
/// <param name="format">The printf style format string to append.</param>
static void Exception_Printf(_Printf_format_string_ char const * format, ...)
{
	char scratch[EXCEPTION_SCRATCH_SIZE];
	va_list args;

	va_start(args, format);
	int const written = vsnprintf(scratch, sizeof(scratch), format, args);
	va_end(args);

	if (written <= 0) {
		return;
	}

	unsigned length = (unsigned)written;
	if (length > sizeof(scratch) - 1) {
		length = sizeof(scratch) - 1;
	}
	if (ExceptionReportLength + length > sizeof(ExceptionReport) - 1) {
		length = (unsigned)(sizeof(ExceptionReport) - 1 - ExceptionReportLength);
	}
	if (length == 0) {
		return;
	}

	memcpy(ExceptionReport + ExceptionReportLength, scratch, length);
	ExceptionReportLength += length;
	ExceptionReport[ExceptionReportLength] = '\0';
}


/// <summary>
/// Reports whether the symbol handler finished preparing and may be called.
/// </summary>
static bool Symbols_Usable(void)
{
	return(SymbolState.load() == SymbolStateType::Ready && DbgHelpLockReady);
}


/// <summary>
/// Reports whether the dump writer may be called.
/// </summary>
/// <remarks>
/// Writing a dump needs the library loaded and the lock built, but not working symbols: a
/// machine with no symbol file still deserves the one artifact that carries the whole crash.
/// The exception is a fault raised while the symbol handler was still starting, where the
/// library's own state is what cannot be trusted.
/// </remarks>
static bool Dump_Allowed(void)
{
	return(DbgHelpLockReady && SymbolState.load() != SymbolStateType::Initializing);
}


/// <summary>
/// Fills the module table from a snapshot of the modules currently loaded.
/// </summary>
/// <remarks>
/// Module bases make every raw address in the report meaningful even with no symbols at all,
/// and the full paths expose any injected library the loaded set would not otherwise show.
/// </remarks>
static void Capture_Module_Table(void)
{
	ModuleCount = 0;

	HANDLE const snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (snapshot == INVALID_HANDLE_VALUE) {
		return;
	}

	MODULEENTRY32 entry;
	memset(&entry, 0, sizeof(entry));
	entry.dwSize = sizeof(entry);

	if (Module32First(snapshot, &entry)) {
		do {
			ModuleEntryType & slot = ModuleTable[ModuleCount];
			slot.Base = (DWORD_PTR)entry.modBaseAddr;
			slot.End = slot.Base + entry.modBaseSize;

			strncpy(slot.Name, entry.szModule, sizeof(slot.Name) - 1);
			slot.Name[sizeof(slot.Name) - 1] = '\0';
			strncpy(slot.Path, entry.szExePath, sizeof(slot.Path) - 1);
			slot.Path[sizeof(slot.Path) - 1] = '\0';

			ModuleCount++;
		} while (ModuleCount < MAX_MODULE_COUNT && Module32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
}


/// <summary>
/// Finds the loaded module that contains an address.
/// </summary>
/// <returns>The module table entry, or null when no loaded module covers the address.</returns>
static ModuleEntryType const * Module_For_Address(DWORD_PTR address)
{
	for (unsigned index = 0; index < ModuleCount; index++) {
		if (address >= ModuleTable[index].Base && address < ModuleTable[index].End) {
			return(&ModuleTable[index]);
		}
	}

	return(NULL);
}


/// <summary>
/// Appends everything but the address itself: the module, symbol, and source line it belongs to.
/// </summary>
/// <param name="address">The address to identify.</param>
static void Append_Address_Details(DWORD_PTR address)
{
	char symbol[256];
	char file[MAX_PATH];
	DWORD_PTR displacement = 0;
	unsigned line = 0;

	ModuleEntryType const * const module = Module_For_Address(address);
	if (module != NULL) {
		Exception_Printf("  %s+0x%IX", module->Name, address - module->Base);
	}

	if (Resolve_Symbol(address, symbol, sizeof(symbol), &displacement)) {
		Exception_Printf("  %s()+0x%IX", symbol, displacement);
	}

	if (Resolve_Line(address, file, sizeof(file), &line)) {
		Exception_Printf("  [%s:%u]", file, line);
	}
}


/// <summary>
/// Appends one code address with every identification the process can still supply for it.
/// </summary>
/// <param name="address">The address to describe.</param>
/// <param name="prefix">Text placed before the address, normally indentation.</param>
static void Append_Address(DWORD_PTR address, char const * prefix)
{
	Exception_Printf("%s0x%016IX", prefix, address);
	Append_Address_Details(address);
	Exception_Printf("\r\n");
}


// Every symbol handler entry point below is wrapped so that a fault inside it costs the report
// one line rather than the report. The guard has to sit inside the callee: the caller releases
// DbgHelpLock on the way out, and an exception crossing that boundary would strand it held.

static BOOL Guarded_Sym_From_Addr(DWORD_PTR address, DWORD64 * displacement, SYMBOL_INFO * symbol)
{
	__try {
		return(SymFromAddr(GetCurrentProcess(), (DWORD64)address, displacement, symbol));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return(FALSE);
	}
}


static BOOL Guarded_Sym_Get_Line(DWORD_PTR address, DWORD * displacement, IMAGEHLP_LINE64 * line)
{
	__try {
		return(SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64)address, displacement, line));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return(FALSE);
	}
}


static BOOL Guarded_Stack_Walk(STACKFRAME64 * frame, CONTEXT * context)
{
	__try {
		return(StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(),
					frame, context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return(FALSE);
	}
}


static BOOL Guarded_Mini_Dump_Write(HANDLE file, MINIDUMP_TYPE flags, MINIDUMP_EXCEPTION_INFORMATION * info)
{
	__try {
		return(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, flags, info, NULL, NULL));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return(FALSE);
	}
}


/// <summary>
/// Resolves an address to a symbol name and the offset into it.
/// </summary>
/// <returns>True when a symbol was found.</returns>
static bool Resolve_Symbol(DWORD_PTR address, char * name, unsigned name_size, DWORD_PTR * displacement)
{
	if (!Symbols_Usable()) {
		return(false);
	}

	char storage[sizeof(SYMBOL_INFO) + 256];
	memset(storage, 0, sizeof(storage));

	SYMBOL_INFO * const symbol = (SYMBOL_INFO *)storage;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = 255;

	DWORD64 offset = 0;

	EnterCriticalSection(&DbgHelpLock);
	BOOL const found = Guarded_Sym_From_Addr(address, &offset, symbol);
	LeaveCriticalSection(&DbgHelpLock);

	if (!found) {
		return(false);
	}

	strncpy(name, symbol->Name, name_size - 1);
	name[name_size - 1] = '\0';
	*displacement = (DWORD_PTR)offset;

	return(true);
}


/// <summary>
/// Resolves an address to the source file and line that generated it.
/// </summary>
/// <returns>True when line information was found.</returns>
static bool Resolve_Line(DWORD_PTR address, char * file, unsigned file_size, unsigned * line)
{
	if (!Symbols_Usable()) {
		return(false);
	}

	IMAGEHLP_LINE64 info;
	memset(&info, 0, sizeof(info));
	info.SizeOfStruct = sizeof(info);

	DWORD offset = 0;

	EnterCriticalSection(&DbgHelpLock);
	BOOL const found = Guarded_Sym_Get_Line(address, &offset, &info);
	LeaveCriticalSection(&DbgHelpLock);

	if (!found || info.FileName == NULL) {
		return(false);
	}

	strncpy(file, info.FileName, file_size - 1);
	file[file_size - 1] = '\0';
	*line = info.LineNumber;

	return(true);
}


/// <summary>
/// Describes a code address as "Name()+0x1a [file.cpp:34]", or as the name alone when the build
/// carries no line information. The file is named without its directory, so that the result fits
/// a report carrying one line per entry rather than a stack trace.
/// </summary>
/// <param name="address">The address to describe. For a return address, pass the byte before it,
/// or the description names the line after the call.</param>
/// <returns>True when the address was named; false empties the buffer.</returns>
bool Describe_Code_Address(void const * address, char * buffer, unsigned size)
{
	if (buffer == NULL || size == 0) {
		return(false);
	}

	buffer[0] = '\0';

	char symbol[256];
	DWORD_PTR displacement = 0;

	if (!Resolve_Symbol((DWORD_PTR)address, symbol, sizeof(symbol), &displacement)) {
		return(false);
	}

	char file[MAX_PATH];
	unsigned line = 0;

	if (Resolve_Line((DWORD_PTR)address, file, sizeof(file), &line)) {
		char const * name = file;
		for (char const * scan = file; *scan != '\0'; scan++) {
			if (*scan == '\\' || *scan == '/') {
				name = scan + 1;
			}
		}
		snprintf(buffer, size, "%s()+0x%IX [%s:%u]", symbol, displacement, name, line);
	} else {
		snprintf(buffer, size, "%s()+0x%IX", symbol, displacement);
	}

	buffer[size - 1] = '\0';

	return(true);
}


/// <summary>
/// Decodes one of the eight 80 bit x87 registers into an ordinary double.
/// </summary>
/// <remarks>
/// The 80 bit format stores the leading significand bit explicitly, unlike float and double,
/// so the whole mantissa is scaled by the unbiased exponent less the mantissa width.
/// </remarks>
static double Read_X87_Register(BYTE const * bytes)
{
	unsigned __int64 mantissa = 0;
	unsigned short sign_exponent = 0;

	memcpy(&mantissa, bytes, sizeof(mantissa));
	memcpy(&sign_exponent, bytes + 8, sizeof(sign_exponent));

	int const exponent = (int)(sign_exponent & 0x7FFF);
	double const sign = ((sign_exponent & 0x8000) != 0) ? -1.0 : 1.0;

	return(sign * ldexp((double)mantissa, exponent - 16383 - 63));
}


/// <summary>
/// Appends the name and meaning of the exception, and what it was doing to what address.
/// </summary>
static void Append_Exception_Description(EXCEPTION_RECORD const * record)
{
	char const * name = "UNKNOWN_EXCEPTION";
	char const * description = "The exception code is not one this handler recognizes.";

	for (unsigned index = 0; index < sizeof(ExceptionCodes) / sizeof(ExceptionCodes[0]); index++) {
		if (ExceptionCodes[index].Code == record->ExceptionCode) {
			name = ExceptionCodes[index].Name;
			description = ExceptionCodes[index].Description;
			break;
		}
	}

	Exception_Printf("Code        : 0x%016X %s\r\n", record->ExceptionCode, name);
	Exception_Printf("Description : %s\r\n", description);

	if ((record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || record->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
				&& record->NumberParameters >= 2) {

		char const * operation = "accessed";
		switch (record->ExceptionInformation[0]) {
			case 0:
				operation = "read from";
				break;

			case 1:
				operation = "written to";
				break;

			case 8:
				operation = "executed, which data execution prevention forbids";
				break;

			default:
				break;
		}

		Exception_Printf("Access      : 0x%016IX was %s.\r\n", (DWORD_PTR)record->ExceptionInformation[1], operation);
	}

	// The engine's own codes carry their message as the first parameter, pointing into static
	// storage owned by whatever raised it.
	if (record->NumberParameters >= 1
				&& (record->ExceptionCode == EXCEPTION_OPENTS_FATAL
					|| record->ExceptionCode == EXCEPTION_OPENTS_PURECALL
					|| record->ExceptionCode == EXCEPTION_OPENTS_INVALID_PARAMETER
					|| record->ExceptionCode == EXCEPTION_OPENTS_TERMINATE)) {

		char const * const message = (char const *)record->ExceptionInformation[0];
		if (message != NULL && !IsBadStringPtrA(message, 1024)) {
			Exception_Printf("Message     : %s\r\n", message);
		}
	}

	// Bounded because a damaged record can point at itself, and the chain is only ever a few
	// records deep when it is intact.
	EXCEPTION_RECORD const * nested = record->ExceptionRecord;
	for (unsigned depth = 0; depth < 4 && nested != NULL; depth++) {
		if (IsBadReadPtr(nested, sizeof(EXCEPTION_RECORD))) {
			break;
		}

		Exception_Printf("Nested      : 0x%16X at 0x%16IX\r\n", nested->ExceptionCode, (DWORD_PTR)nested->ExceptionAddress);
		nested = nested->ExceptionRecord;
	}
}


/// <summary>
/// Appends the integer, segment and debug registers.
/// </summary>
static void Append_Registers(CONTEXT const * context)
{
	Exception_Printf("\r\nRegisters\r\n---------\r\n");
#ifdef _WIN64
	Exception_Printf("Rip:%016X  Rsp:%016X  Rbp:%016X\r\n", context->Rip, context->Rsp, context->Rbp);
	Exception_Printf("Rax:%016X  Rbx:%016X  Rcx:%016X\r\n", context->Rax, context->Rbx, context->Rcx);
	Exception_Printf("Rdx:%016X  Rsi:%016X  Rdi:%016X\r\n", context->Rdx, context->Rsi, context->Rdi);
	Exception_Printf("R8:%016X  R9:%016X  R10:%016X\r\n", context->R8, context->R9, context->R10);
	Exception_Printf("R11:%016X  R12:%016X  R13:%016X\r\n", context->R11, context->R12, context->R13);
	Exception_Printf("R14:%016X  R15:%016X\r\n", context->R14, context->R15);
#else
	Exception_Printf("Eip:%016X  Esp:%016X  Ebp:%016X\r\n", context->Eip, context->Esp, context->Ebp);
	Exception_Printf("Eax:%016X  Ebx:%016X  Ecx:%016X\r\n", context->Eax, context->Ebx, context->Ecx);
	Exception_Printf("Edx:%016X  Esi:%016X  Edi:%016X\r\n", context->Edx, context->Esi, context->Edi);
#endif
	Exception_Printf("EFlags:%016X\r\n", context->EFlags);
	Exception_Printf("CS:%04X  SS:%04X  DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n",
				context->SegCs, context->SegSs, context->SegDs, context->SegEs, context->SegFs, context->SegGs);

	if ((context->ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) {
		Exception_Printf("Dr0:%016X  Dr1:%016X  Dr2:%016X  Dr3:%016X\r\n",
					context->Dr0, context->Dr1, context->Dr2, context->Dr3);
		Exception_Printf("Dr6:%016X  Dr7:%016X\r\n", context->Dr6, context->Dr7);
	}
}


/// <summary>
/// Appends the x87 control state, the eight x87 registers, and the SSE state.
/// </summary>
static void Append_Floating_Point(CONTEXT const * context)
{
	if ((context->ContextFlags & CONTEXT_FLOATING_POINT) != CONTEXT_FLOATING_POINT) {
		return;
	}

#ifdef _WIN64
	XMM_SAVE_AREA32 const & save = context->FltSave;
	Exception_Printf("\r\nFloating point\r\n--------------\r\n");
	Exception_Printf("Control:%016X  Status:%016X  Tag:%016X\r\n", save.ControlWord, save.StatusWord, save.TagWord);
	Exception_Printf("ErrorOffset:%016X  ErrorSelector:%016X\r\n", save.ErrorOffset, save.ErrorSelector);
	Exception_Printf("DataOffset:%016X  DataSelector:%016X\r\n", save.DataOffset, save.DataSelector);
	Exception_Printf("MXCSR:%016X\r\n", save.MxCsr);
	for (size_t index = 0; index < 16; index++) {
		Exception_Printf("XMM%u: %016X %016X \r\n", index, save.XmmRegisters[index].High, save.XmmRegisters[index].Low);
	}
#else
	FLOATING_SAVE_AREA const & save = context->FloatSave;

	Exception_Printf("\r\nFloating point\r\n--------------\r\n");
	Exception_Printf("Control:%08X  Status:%08X  Tag:%08X\r\n", save.ControlWord, save.StatusWord, save.TagWord);
	Exception_Printf("ErrorOffset:%08X  ErrorSelector:%08X\r\n", save.ErrorOffset, save.ErrorSelector);
	Exception_Printf("DataOffset:%08X  DataSelector:%08X\r\n", save.DataOffset, save.DataSelector);

	for (unsigned index = 0; index < 8; index++) {
		BYTE const * const bytes = &save.RegisterArea[index * 10];

		Exception_Printf("ST%u : ", index);
		for (int position = 9; position >= 0; position--) {
			Exception_Printf("%02X", bytes[position]);
		}
		Exception_Printf("   %+.17e\r\n", Read_X87_Register(bytes));
	}

	// The engine is built for SSE2, so the XMM registers are as much a part of the machine
	// state as the integer ones. On this architecture they are only reachable through the
	// saved FXSAVE image rather than through named context fields.
	if ((context->ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS) {
		BYTE const * const fxsave = context->ExtendedRegisters;
		unsigned mxcsr = 0;
		memcpy(&mxcsr, fxsave + 24, sizeof(mxcsr));

		Exception_Printf("MXCSR:%08X\r\n", mxcsr);

		for (unsigned index = 0; index < 8; index++) {
			unsigned word[4];
			memcpy(word, fxsave + 160 + (index * 16), sizeof(word));
			Exception_Printf("XMM%u: %08X %08X %08X %08X\r\n", index, word[3], word[2], word[1], word[0]);
		}
	}
#endif
}


/// <summary>
/// Appends the instruction bytes at the faulting address.
/// </summary>
static void Append_Code_Bytes(CONTEXT const * context)
{
#ifdef _WIN64
	BYTE const * const code = (BYTE const *)context->Rip;
#else
	BYTE const * const code = (BYTE const *)context->Eip;
#endif

	Exception_Printf("Bytes       : ");
	for (unsigned index = 0; index < NUM_CODE_BYTES; index++) {
		if (IsBadReadPtr(code + index, 1)) {
			Exception_Printf("?? ");
		} else {
			Exception_Printf("%02X ", code[index]);
		}
	}
	Exception_Printf("\r\n");
}


/// <summary>
/// Appends the return addresses reachable by following the saved frame pointer chain.
/// </summary>
/// <remarks>
/// This needs no symbol handler, so it still produces a usable list of return addresses when
/// the symbol driven walk below fails outright. Optimized builds omit the frame pointer, so a
/// short result here is expected rather than a sign of damage.
/// </remarks>
static void Append_Frame_Chain(CONTEXT const * context)
{
	Exception_Printf("\r\nCall stack (frame chain)\r\n------------------------\r\n");

	// The chain records return addresses, so the faulting instruction is not in it and is
	// listed here for the two walks to start from the same place.
#ifdef _WIN64
	Append_Address((DWORD_PTR)context->Rip, "  ");
	DWORD_PTR const * frame = (DWORD_PTR const *)context->Rbp;
#else
	Append_Address((DWORD_PTR)context->Eip, "  ");
	DWORD_PTR const * frame = (DWORD_PTR const *)context->Ebp;
#endif
	DWORD_PTR previous = 0;

	for (unsigned depth = 0; depth < MAX_FRAME_DEPTH; depth++) {
		// The stack grows down, so a frame that does not move to a higher address is a
		// corrupt or looping chain rather than a caller.
		if (frame == NULL || (DWORD_PTR)frame <= previous || IsBadReadPtr(frame, 2 * sizeof(DWORD_PTR))) {
			break;
		}

		// The outermost frame returns nowhere, which ends the chain rather than naming a caller.
		if (frame[1] == 0) {
			break;
		}

		Append_Address(frame[1], "  ");

		previous = (DWORD_PTR)frame;
		frame = (DWORD_PTR const *)frame[0];
	}
}


/// <summary>
/// Appends the call stack as reconstructed by the symbol handler.
/// </summary>
static void Append_Call_Stack(CONTEXT const * context)
{
	Exception_Printf("\r\nCall stack (symbol handler)\r\n---------------------------\r\n");

	if (!Symbols_Usable()) {
		Exception_Printf("  <no symbol handler; skipped>\r\n");
		return;
	}

	// The walk updates the context it is given, so it gets a copy. The original is the
	// evidence, and the rest of the report still needs it intact.
	CONTEXT working = *context;

	STACKFRAME64 frame;
	memset(&frame, 0, sizeof(frame));
#ifdef _WIN64
	frame.AddrPC.Offset = working.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = working.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = working.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;
#else
	frame.AddrPC.Offset = working.Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = working.Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = working.Esp;
	frame.AddrStack.Mode = AddrModeFlat;
#endif
	for (unsigned depth = 0; depth < MAX_FRAME_DEPTH; depth++) {
		EnterCriticalSection(&DbgHelpLock);
		BOOL const walked = Guarded_Stack_Walk(&frame, &working);
		LeaveCriticalSection(&DbgHelpLock);

		if (!walked || frame.AddrPC.Offset == 0) {
			break;
		}

		Append_Address((DWORD_PTR)frame.AddrPC.Offset, "  ");
	}
}


/// <summary>
/// Appends the loaded modules with the address range each one occupies.
/// </summary>
static void Append_Module_List(void)
{
	Exception_Printf("\r\nLoaded modules\r\n--------------\r\n");

	for (unsigned index = 0; index < ModuleCount; index++) {
		ModuleEntryType const & module = ModuleTable[index];
		Exception_Printf("0x%016IX - 0x%016IX  %s\r\n", module.Base, module.End, module.Path);
	}
}


/// <summary>
/// Appends the system memory position at the time of the crash.
/// </summary>
static void Append_Memory_Status(void)
{
	MEMORYSTATUSEX status;
	memset(&status, 0, sizeof(status));
	status.dwLength = sizeof(status);

	if (!GlobalMemoryStatusEx(&status)) {
		return;
	}

	Exception_Printf("\r\nMemory\r\n------\r\n");
	Exception_Printf("Load          : %u%%\r\n", status.dwMemoryLoad);
	Exception_Printf("Physical      : %I64u MB free of %I64u MB\r\n",
				status.ullAvailPhys / (1024 * 1024), status.ullTotalPhys / (1024 * 1024));
	Exception_Printf("Page file     : %I64u MB free of %I64u MB\r\n",
				status.ullAvailPageFile / (1024 * 1024), status.ullTotalPageFile / (1024 * 1024));
	Exception_Printf("Address space : %I64u MB free of %I64u MB\r\n",
				status.ullAvailVirtual / (1024 * 1024), status.ullTotalVirtual / (1024 * 1024));
}


/// <summary>
/// Appends a raw scan of the stack, identifying the values that could be code addresses.
/// </summary>
static void Append_Stack_Dump(CONTEXT const * context)
{
	Exception_Printf("\r\nStack dump (* marks a possible code address)\r\n");
	Exception_Printf("-------------------------------------------\r\n");

#ifdef _WIN64
	DWORD_PTR const * const stack = (DWORD_PTR const *)context->Rsp;
#else
	DWORD_PTR const * const stack = (DWORD_PTR const *)context->Esp;
#endif

	for (unsigned index = 0; index < MAX_STACK_DUMP; index++) {
		DWORD_PTR const * const slot = stack + index;

		if (IsBadReadPtr(slot, sizeof(DWORD_PTR))) {
			Exception_Printf("0x%016IX: ????????\r\n", (DWORD_PTR)slot);
			continue;
		}

		DWORD_PTR const value = *slot;
		Exception_Printf("0x%016IX: 0x%016IX", (DWORD_PTR)slot, value);

		if (Module_For_Address(value) != NULL) {
			Exception_Printf(" *");
			Append_Address_Details(value);
		}

		Exception_Printf("\r\n");
	}
}


// Each risky section of the report runs behind its own guard, so that one section faulting
// costs a marker rather than everything the report had still to say. The guards only bite
// when the report runs on the dumper thread; inside a filter a nested fault is not catchable,
// which is what the latch in the filter exists to end.

static void Guarded_Exception_Description(EXCEPTION_RECORD const * record)
{
	__try {
		Append_Exception_Description(record);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Exception_Printf("  <exception description faulted>\r\n");
	}
}


static void Guarded_Crash_Site(CONTEXT const * context)
{
	__try {
		Exception_Printf("\r\nCrash site\r\n----------\r\n");
#ifdef _WIN64
		Append_Address((DWORD_PTR)context->Rip, "Address     : ");
#else
		Append_Address((DWORD_PTR)context->Eip, "Address     : ");
#endif
		Append_Code_Bytes(context);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Exception_Printf("  <crash site faulted>\r\n");
	}
}


static void Guarded_Registers(CONTEXT const * context)
{
	__try {
		Append_Registers(context);
		Append_Floating_Point(context);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Exception_Printf("  <register dump faulted>\r\n");
	}
}


static void Guarded_Frame_Chain(CONTEXT const * context)
{
	__try {
		if (TestSectionFault) {
			*(volatile int *)16 = 1;
		}
		Append_Frame_Chain(context);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Exception_Printf("  <frame chain walk faulted>\r\n");
	}
}


static void Guarded_Call_Stack(CONTEXT const * context)
{
	__try {
		Append_Call_Stack(context);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Exception_Printf("  <call stack walk faulted (corrupt stack or symbols); skipped>\r\n");
	}
}


static void Guarded_Module_List(void)
{
	__try {
		Append_Module_List();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Exception_Printf("  <module list faulted>\r\n");
	}
}


static void Guarded_Memory_Status(void)
{
	__try {
		Append_Memory_Status();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Exception_Printf("  <memory status faulted>\r\n");
	}
}


static void Guarded_Stack_Dump(CONTEXT const * context)
{
	__try {
		Append_Stack_Dump(context);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Exception_Printf("  <stack dump faulted; truncated>\r\n");
	}
}


static void Guarded_Capture_Module_Table(void)
{
	__try {
		Capture_Module_Table();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		ModuleCount = 0;
	}
}


/// <summary>
/// Reads the linker timestamp out of the running image's headers.
/// </summary>
/// <returns>True when the headers were readable.</returns>
static bool Get_Link_Time(SYSTEMTIME * when)
{
	BYTE const * const base = (BYTE const *)GetModuleHandle(NULL);
	if (base == NULL || IsBadReadPtr(base, sizeof(IMAGE_DOS_HEADER))) {
		return(false);
	}

	IMAGE_DOS_HEADER const * const dos = (IMAGE_DOS_HEADER const *)base;
	if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
		return(false);
	}

	IMAGE_NT_HEADERS const * const headers = (IMAGE_NT_HEADERS const *)(base + dos->e_lfanew);
	if (IsBadReadPtr(headers, sizeof(IMAGE_NT_HEADERS)) || headers->Signature != IMAGE_NT_SIGNATURE) {
		return(false);
	}

	// The header counts seconds from the start of 1970, which is 11644473600 seconds after
	// the epoch file times are measured from.
	unsigned __int64 const stamp = (11644473600ULL + headers->FileHeader.TimeDateStamp) * 10000000ULL;

	FILETIME file_time;
	file_time.dwLowDateTime = (DWORD)(stamp & 0xFFFFFFFF);
	file_time.dwHighDateTime = (DWORD)(stamp >> 32);

	return(FileTimeToSystemTime(&file_time, when) != FALSE);
}


/// <summary>
/// Builds the whole machine state report into the static report buffer.
/// </summary>
/// <param name="e_info">The saved machine state of the crashing thread.</param>
/// <param name="crashed_thread">The thread that faulted.</param>
static void Build_Exception_Report(EXCEPTION_POINTERS * e_info, DWORD crashed_thread)
{
	ExceptionReportLength = 0;
	ExceptionReport[0] = '\0';

	Exception_Printf("OpenTS exception report\r\n");
	Exception_Printf("=======================\r\n\r\n");

	Exception_Printf("Time        : %04u-%02u-%02u %02u:%02u:%02u\r\n",
				CrashTime.wYear, CrashTime.wMonth, CrashTime.wDay,
				CrashTime.wHour, CrashTime.wMinute, CrashTime.wSecond);
	Exception_Printf("Version     : %s\r\n", Version_Name());

	SYSTEMTIME link_time;
	if (Get_Link_Time(&link_time)) {
		Exception_Printf("Linked      : %04u-%02u-%02u %02u:%02u:%02u UTC\r\n",
					link_time.wYear, link_time.wMonth, link_time.wDay,
					link_time.wHour, link_time.wMinute, link_time.wSecond);
	}

#ifdef _DEBUG
	Exception_Printf("Build       : Debug\r\n");
#else
	Exception_Printf("Build       : Release\r\n");
#endif

	Exception_Printf("Executable  : %s\r\n", ExecutablePath);
	Exception_Printf("Command line: %s\r\n", GetCommandLineA());
	Exception_Printf("Thread      : 0x%04X%s\r\n", crashed_thread,
				(crashed_thread == MainThreadId) ? " (main thread)" : "");

	if (!Symbols_Usable()) {
		Exception_Printf("Symbols     : unavailable, so addresses are reported without names\r\n");
	} else if (!SymbolsVerified) {
		Exception_Printf("Symbols     : no matching symbol file was found for the executable\r\n");
	}

	Exception_Printf("\r\nException\r\n---------\r\n");

	if (e_info == NULL || e_info->ExceptionRecord == NULL || e_info->ContextRecord == NULL) {
		Exception_Printf("No machine context was available, so the report ends here.\r\n");
		ExceptionReportFinished = true;
		return;
	}

	Guarded_Exception_Description(e_info->ExceptionRecord);
	Guarded_Crash_Site(e_info->ContextRecord);
	Guarded_Registers(e_info->ContextRecord);
	Guarded_Frame_Chain(e_info->ContextRecord);
	Guarded_Call_Stack(e_info->ContextRecord);
	Guarded_Module_List();
	Guarded_Memory_Status();
	Guarded_Stack_Dump(e_info->ContextRecord);

	ExceptionReportFinished = true;
}


/// <summary>
/// Writes a buffer to a new file, replacing any file already at that path.
/// </summary>
/// <returns>True when the whole buffer reached the file.</returns>
static bool Write_Whole_File(char const * path, void const * data, DWORD length, DWORD flags)
{
	HANDLE const file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, flags, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return(false);
	}

	DWORD written = 0;
	BOOL const ok = WriteFile(file, data, length, &written, NULL);
	CloseHandle(file);

	return(ok != FALSE && written == length);
}


/// <summary>
/// Writes a minidump of the crashed process.
/// </summary>
/// <param name="path">The file to write.</param>
/// <param name="full_memory">True to include the whole address space rather than a summary.</param>
/// <returns>True when the dump was written.</returns>
static bool Write_Mini_Dump(char const * path, bool full_memory)
{
	if (!Dump_Allowed()) {
		return(false);
	}

	// Deliberately cached rather than written through: the dump is written as a long series
	// of small writes, and forcing each one to the disk turns a full memory dump from seconds
	// into minutes of apparent hang.
	HANDLE const file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return(false);
	}

	MINIDUMP_EXCEPTION_INFORMATION info;
	memset(&info, 0, sizeof(info));

	// The dump normally runs on the dumper thread, so the thread to open the dump on has to
	// be named explicitly. Left to itself the dump would present the reporting thread.
	info.ThreadId = CrashedThreadId;
	info.ExceptionPointers = &SavedPointers;
	info.ClientPointers = FALSE;

	MINIDUMP_TYPE flags = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithDataSegs
				| MiniDumpWithIndirectlyReferencedMemory);
	if (full_memory) {
		// A full memory dump already contains everything the indirect scan would collect, so
		// asking for both only buys a pointer chase across every thread stack.
		flags = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithFullMemory);
	}

	EnterCriticalSection(&DbgHelpLock);
	BOOL const written = Guarded_Mini_Dump_Write(file, flags, &info);
	LeaveCriticalSection(&DbgHelpLock);

	CloseHandle(file);

	return(written != FALSE);
}


/// <summary>
/// Copies the tail of the current debug log into the artifact folder.
/// </summary>
/// <returns>True when a copy was made.</returns>
/// <remarks>
/// The path was registered during startup and is read here as plain text. Asking the logger
/// for it now would enter a lock that the crashing thread may already hold.
/// </remarks>
static bool Copy_Log_Tail(void)
{
	if (RegisteredLogFile[0] == '\0') {
		return(false);
	}

	HANDLE const source = CreateFileA(RegisteredLogFile, GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	if (source == INVALID_HANDLE_VALUE) {
		return(false);
	}

	LARGE_INTEGER size;
	if (!GetFileSizeEx(source, &size)) {
		CloseHandle(source);
		return(false);
	}

	LARGE_INTEGER position;
	position.QuadPart = (size.QuadPart > LOG_TAIL_BYTES) ? (size.QuadPart - LOG_TAIL_BYTES) : 0;

	if (!SetFilePointerEx(source, position, NULL, FILE_BEGIN)) {
		CloseHandle(source);
		return(false);
	}

	char path[MAX_PATH];
	snprintf(path, sizeof(path), "%s\\debug-tail.log", ArtifactFolder);

	HANDLE const target = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
	if (target == INVALID_HANDLE_VALUE) {
		CloseHandle(source);
		return(false);
	}

	static char transfer[32768];
	bool ok = true;

	for (;;) {
		DWORD read = 0;
		if (!ReadFile(source, transfer, sizeof(transfer), &read, NULL)) {
			ok = false;
			break;
		}
		if (read == 0) {
			break;
		}

		DWORD written = 0;
		if (!WriteFile(target, transfer, read, &written, NULL) || written != read) {
			ok = false;
			break;
		}
	}

	CloseHandle(target);
	CloseHandle(source);

	return(ok);
}


/// <summary>
/// Creates the folder that holds every artifact for this crash.
/// </summary>
/// <returns>True when a folder of its own was created.</returns>
static bool Create_Artifact_Folder(void)
{
	char root[MAX_PATH];
	snprintf(root, sizeof(root), "%s\\Exceptions", ExecutableDirectory);
	CreateDirectoryA(root, NULL);

	snprintf(ArtifactFolder, sizeof(ArtifactFolder), "%s\\exception-%04u%02u%02u-%02u%02u%02u-%u",
				root, CrashTime.wYear, CrashTime.wMonth, CrashTime.wDay,
				CrashTime.wHour, CrashTime.wMinute, CrashTime.wSecond, GetCurrentProcessId());

	if (!CreateDirectoryA(ArtifactFolder, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		// Somewhere to write beats nowhere. Beside the executable is where the player will
		// look anyway.
		strncpy(ArtifactFolder, ExecutableDirectory, sizeof(ArtifactFolder) - 1);
		ArtifactFolder[sizeof(ArtifactFolder) - 1] = '\0';
		return(false);
	}

	return(true);
}


/// <summary>
/// Writes every artifact for this crash into a folder of its own.
/// </summary>
/// <param name="e_info">The saved machine state of the crashing thread.</param>
/// <param name="crashed_thread">The thread that faulted, which is not this one on the dumper.</param>
static void Write_Crash_Artifacts(EXCEPTION_POINTERS * e_info, DWORD crashed_thread)
{
	Create_Artifact_Folder();
	Guarded_Capture_Module_Table();

	// The minidump goes first. Everything after it can fault on damaged state, and this is
	// the artifact that keeps its value when the rest of the report does not survive.
	char dump_path[MAX_PATH];
	snprintf(dump_path, sizeof(dump_path), "%s\\minidump.dmp", ArtifactFolder);
	bool const dumped = Write_Mini_Dump(dump_path, false);

	Build_Exception_Report(e_info, crashed_thread);

	bool const copied = Copy_Log_Tail();

	Exception_Printf("\r\nArtifacts\r\n---------\r\n");
	Exception_Printf("Folder        : %s\r\n", ArtifactFolder);
	Exception_Printf("minidump.dmp  : %s\r\n", dumped ? "written" : "not written");
	Exception_Printf("debug-tail.log: %s\r\n", copied ? "written" : "not written, no debug log was available");
	Exception_Printf("\r\nPlease include this whole folder when reporting the crash.\r\n");

	char report_path[MAX_PATH];
	snprintf(report_path, sizeof(report_path), "%s\\except.txt", ArtifactFolder);

	// Written through, unlike the dump: the report is small, and it is what survives if the
	// machine is lost before the file system catches up.
	Write_Whole_File(report_path, ExceptionReport, ExceptionReportLength,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH);
}


/// <summary>
/// Handles the messages for the exception report dialog.
/// </summary>
static INT_PTR CALLBACK Exception_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM)
{
	switch (message) {
		case WM_INITDIALOG:
		{
			// A fixed pitch face is what lets the register columns and the stack dump line up.
			// The font is deliberately never freed; the process ends with this dialog.
			HDC const dc = GetDC(window);
			int const height = -MulDiv(9, GetDeviceCaps(dc, LOGPIXELSY), 72);
			ReleaseDC(window, dc);

			HFONT const font = CreateFontA(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
						FIXED_PITCH | FF_MODERN, "Consolas");
			if (font != NULL) {
				SendDlgItemMessage(window, IDC_EXCEPTION_DETAILS, WM_SETFONT, (WPARAM)font, TRUE);
			}

			SetDlgItemTextA(window, IDC_EXCEPTION_FOLDER, ArtifactFolder);
			SetDlgItemTextA(window, IDC_EXCEPTION_DETAILS,
						ExceptionReportFinished ? ExceptionReport : "The report could not be generated.");

			RECT work;
			RECT self;
			if (SystemParametersInfo(SPI_GETWORKAREA, 0, &work, 0) && GetWindowRect(window, &self)) {
				SetWindowPos(window, HWND_TOP,
							work.left + ((work.right - work.left) - (self.right - self.left)) / 2,
							work.top + ((work.bottom - work.top) - (self.bottom - self.top)) / 2,
							0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}

			SetForegroundWindow(window);
			SetFocus(GetDlgItem(window, IDC_EXCEPTION_QUIT));

			// False because the focus above is deliberate and the dialog manager would
			// otherwise put it back on the first control.
			return(FALSE);
		}

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDC_EXCEPTION_FULLDUMP:
				{
					EnableWindow(GetDlgItem(window, IDC_EXCEPTION_FULLDUMP), FALSE);
					HCURSOR const previous = SetCursor(LoadCursor(NULL, IDC_WAIT));

					snprintf(FullDumpPath, sizeof(FullDumpPath), "%s\\fulldump.dmp", ArtifactFolder);
					bool const written = Write_Mini_Dump(FullDumpPath, true);

					SetCursor(previous);
					SetDlgItemTextA(window, IDC_EXCEPTION_FULLDUMP, written ? "Dump saved" : "Dump failed");
					return(TRUE);
				}

				case IDC_EXCEPTION_DEBUG:
					EndDialog(window, IDC_EXCEPTION_DEBUG);
					return(TRUE);

				case IDC_EXCEPTION_QUIT:
				case IDOK:
				case IDCANCEL:
					EndDialog(window, IDC_EXCEPTION_QUIT);
					return(TRUE);

				default:
					break;
			}
			break;

		default:
			break;
	}

	return(FALSE);
}


/// <summary>
/// Breaks into an attached debugger, and does nothing when there is none.
/// </summary>
static void Guarded_Debug_Break(void)
{
	__try {
		__debugbreak();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}


/// <summary>
/// Puts the game window out of the way so that the report can be seen and read.
/// </summary>
/// <remarks>
/// The presenter never changes the display mode, so there is no mode to restore and no
/// renderer call worth making from a crashed process. What does have to be undone is the
/// captured cursor and a full screen window covering the report.
/// </remarks>
static void Release_Display(void)
{
	ClipCursor(NULL);

	while (ShowCursor(TRUE) < 0) {
	}

	if (!WindowedMode && MainWindow != NULL) {
		// Asynchronous because the thread that owns the window is parked in the filter and
		// will never answer a message. The dialog is topmost, so this only tidies up behind.
		ShowWindowAsync(MainWindow, SW_MINIMIZE);
	}
}


/// <summary>
/// Tells the player that the handler crashed while reporting a crash.
/// </summary>
static void Show_Recursion_Notice(void)
{
	char message[MAX_PATH + 256];

	if (ArtifactFolder[0] != '\0') {
		snprintf(message, sizeof(message),
					"The game crashed again while reporting a crash.\n\nWhatever was saved is in:\n%s",
					ArtifactFolder);
	} else {
		snprintf(message, sizeof(message),
					"The game crashed again while reporting a crash, before anything could be saved.");
	}

	MessageBoxA(NULL, message, "OpenTS", MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}


/// <summary>
/// Shows the report to the player and ends the process the way they choose.
/// </summary>
[[noreturn]] static void Show_Exception_Dialog(void)
{
	Release_Display();

	INT_PTR const result = DialogBoxParam(ProgramInstance, MAKEINTRESOURCE(IDD_EXCEPTION), NULL,
				Exception_Dialog_Proc, 0);

	if (result == -1) {
		char message[MAX_PATH + 256];
		snprintf(message, sizeof(message),
					"The game has crashed.\n\nA report was saved to:\n%s\n\n"
					"Please include that whole folder when reporting the crash.", ArtifactFolder);
		MessageBoxA(NULL, message, "OpenTS", MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
	} else if (result == IDC_EXCEPTION_DEBUG) {
		Guarded_Debug_Break();
	}

	Terminate_Now();
}


/// <summary>
/// Copies the crashing thread's machine state somewhere it will outlive that thread.
/// </summary>
static void Save_Machine_State(EXCEPTION_POINTERS * e_info, DWORD self)
{
	CrashedThreadId = self;

	SavedRecord = *e_info->ExceptionRecord;
	SavedContext = *e_info->ContextRecord;
	SavedPointers.ExceptionRecord = &SavedRecord;
	SavedPointers.ContextRecord = &SavedContext;
}


/// <summary>
/// Writes the artifacts on the crashing thread, for the crashes the dumper cannot take.
/// </summary>
/// <remarks>
/// This runs inside an exception filter, where a nested fault cannot be caught by anything the
/// filter goes on to call, so the guards in the report are inert here. The latch the filter
/// checks on entry is what ends a fault taken in this state.
/// </remarks>
[[noreturn]] static void Handle_Crash_Inline(EXCEPTION_POINTERS * e_info, DWORD self)
{
	DumpingThreadId.store(self);
	Save_Machine_State(e_info, self);

	Write_Crash_Artifacts(&SavedPointers, self);

	DumpingThreadId.store(0);

	Show_Exception_Dialog();
}


/// <summary>
/// Hands the crash to the dumper thread and waits for it to finish.
/// </summary>
[[noreturn]] static void Handle_Crash_Handoff(EXCEPTION_POINTERS * e_info, DWORD self)
{
	Save_Machine_State(e_info, self);

	SetEvent(DumperWakeEvent);

	// The wait is bounded so that a dumper wedged inside the symbol handler ends the process
	// instead of hanging it. Retrying the work here would only block on the lock it holds.
	if (WaitForSingleObject(ArtifactsWrittenEvent, DUMPER_TIMEOUT_MS) != WAIT_OBJECT_0) {
		Terminate_Now();
	}

	// The artifacts are safe, and the dumper owns the dialog and the exit from here.
	for (;;) {
		SuspendThread(GetCurrentThread());
		Sleep(1000);
	}
}


/// <summary>
/// Reports every unhandled exception in the process.
/// </summary>
/// <param name="e_info">The machine state at the point of the fault.</param>
/// <returns>Only returns to let a debugger or the operating system take the exception.</returns>
static LONG CALLBACK Exception_Filter(EXCEPTION_POINTERS * e_info)
{
	DWORD const code = e_info->ExceptionRecord->ExceptionCode;
	DWORD const self = GetCurrentThreadId();

	// None of these is a crash. Letting one take the gate below would leave the handler spent
	// for the fault that matters.
	if (code == EXCEPTION_BREAKPOINT || code == EXCEPTION_SINGLE_STEP || code == MS_VC_THREAD_NAME_EXCEPTION) {
		return(EXCEPTION_CONTINUE_SEARCH);
	}

	// Reached when the report itself faults while running inline, where its guards cannot
	// catch anything. The minidump is already written by this point.
	if (DumpingThreadId.load() == self) {
		Terminate_Now();
	}

	DWORD expected = 0;
	if (FirstCrashThreadId.compare_exchange_strong(expected, self)) {
		GetLocalTime(&CrashTime);

		// The dumper cannot wait for itself, so a first crash on it is reported in place, as
		// is any crash from before it was running.
		if (self == DumperThreadId.load() || DumperThread == NULL
					|| WaitForSingleObject(DumperThread, 0) == WAIT_OBJECT_0) {
			Handle_Crash_Inline(e_info, self);
		}

		Handle_Crash_Handoff(e_info, self);
	}

	if (expected == self || self == DumperThreadId.load()) {
		// The owner of the report faulted again, in code no guard covers. Degrade rather than
		// start the whole report over.
		if (RecursionCount.fetch_add(1) == 0) {
			Show_Recursion_Notice();
		}
		Terminate_Now();
	}

	// Another thread faulted while the first crash is being reported. Park it: the winner owns
	// the exit, and letting this one run would race the report it is part of.
	SuspendThread(GetCurrentThread());
	ExitProcess(EXIT_FAILURE);

	return(EXCEPTION_CONTINUE_SEARCH);
}


/// <summary>
/// Raises an OpenTS exception so that a runtime callback is reported like any other crash.
/// </summary>
/// <param name="code">The OpenTS exception code to raise.</param>
/// <param name="message">Text describing what went wrong, copied into static storage.</param>
[[noreturn]] static void Raise_Engine_Exception(DWORD code, char const * message)
{
	if (message != NULL) {
		strncpy(PendingMessage, message, sizeof(PendingMessage) - 1);
		PendingMessage[sizeof(PendingMessage) - 1] = '\0';
	} else {
		PendingMessage[0] = '\0';
	}

	ULONG_PTR const argument = (ULONG_PTR)PendingMessage;
	RaiseException(code, EXCEPTION_NONCONTINUABLE, 1, &argument);

	Terminate_Now();
}


static void __cdecl Purecall_Handler(void)
{
	Raise_Engine_Exception(EXCEPTION_OPENTS_PURECALL, "A pure virtual function was called.");
}


static void Terminate_Handler(void)
{
	Raise_Engine_Exception(EXCEPTION_OPENTS_TERMINATE, "The C++ runtime called terminate.");
}


static void __cdecl Abort_Handler(int)
{
	Raise_Engine_Exception(EXCEPTION_OPENTS_TERMINATE, "The C runtime aborted the process.");
}


static void __cdecl Invalid_Parameter_Handler(wchar_t const * expression, wchar_t const * function,
			wchar_t const * file, unsigned int line, uintptr_t)
{
	char message[512];

	snprintf(message, sizeof(message), "Invalid argument to %ls at %ls:%u (%ls)",
				(function != NULL) ? function : L"a C runtime function",
				(file != NULL) ? file : L"unknown",
				line,
				(expression != NULL) ? expression : L"no expression");

	Raise_Engine_Exception(EXCEPTION_OPENTS_INVALID_PARAMETER, message);
}


/// <summary>
/// Removes everything below a directory, then the directory itself.
/// </summary>
/// <param name="path">The directory to remove.</param>
/// <param name="depth">Recursion budget, so that a link loop cannot walk forever.</param>
static void Remove_Directory_Tree(char const * path, unsigned depth)
{
	if (depth == 0) {
		return;
	}

	char pattern[MAX_PATH];
	snprintf(pattern, sizeof(pattern), "%s\\*", path);

	WIN32_FIND_DATAA found;
	HANDLE const search = FindFirstFileA(pattern, &found);

	if (search != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(found.cFileName, ".") == 0 || strcmp(found.cFileName, "..") == 0) {
				continue;
			}

			char child[MAX_PATH];
			snprintf(child, sizeof(child), "%s\\%s", path, found.cFileName);

			if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				Remove_Directory_Tree(child, depth - 1);
			} else {
				SetFileAttributesA(child, FILE_ATTRIBUTE_NORMAL);
				DeleteFileA(child);
			}
		} while (FindNextFileA(search, &found));

		FindClose(search);
	}

	RemoveDirectoryA(path);
}


/// <summary>
/// Deletes artifact folders older than the retention period.
/// </summary>
/// <remarks>
/// The logger's own pruning only considers files, so the folders this subsystem writes need a
/// sweep of their own.
/// </remarks>
static void Prune_Old_Artifact_Folders(void)
{
	SYSTEMTIME now;
	FILETIME now_stamp;

	GetSystemTime(&now);
	if (!SystemTimeToFileTime(&now, &now_stamp)) {
		return;
	}

	unsigned __int64 const stamp = ((unsigned __int64)now_stamp.dwHighDateTime << 32) | now_stamp.dwLowDateTime;
	unsigned __int64 const span = 10000000ULL * 60 * 60 * 24 * EXCEPTION_FOLDER_DAYS;
	if (stamp <= span) {
		return;
	}

	unsigned __int64 const cutoff = stamp - span;

	char root[MAX_PATH];
	snprintf(root, sizeof(root), "%s\\Exceptions", ExecutableDirectory);

	char pattern[MAX_PATH];
	snprintf(pattern, sizeof(pattern), "%s\\exception-*", root);

	WIN32_FIND_DATAA found;
	HANDLE const search = FindFirstFileA(pattern, &found);
	if (search == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			continue;
		}
		if (strcmp(found.cFileName, ".") == 0 || strcmp(found.cFileName, "..") == 0) {
			continue;
		}

		unsigned __int64 const written = ((unsigned __int64)found.ftLastWriteTime.dwHighDateTime << 32)
					| found.ftLastWriteTime.dwLowDateTime;
		if (written >= cutoff) {
			continue;
		}

		char folder[MAX_PATH];
		snprintf(folder, sizeof(folder), "%s\\%s", root, found.cFileName);
		Remove_Directory_Tree(folder, 8);
	} while (FindNextFileA(search, &found));

	FindClose(search);
}


/// <summary>
/// Waits for a crash, then writes the artifacts and presents the report.
/// </summary>
/// <remarks>
/// The report runs here rather than on the thread that crashed because a fault taken inside an
/// exception filter cannot be caught by anything the filter goes on to call. On its own thread
/// every guard in the report works, and its untouched stack survives a stack overflow.
/// </remarks>
static DWORD WINAPI Dumper_Thread_Proc(LPVOID)
{
	// Published before anything that can fault, so that a crash in this thread's own setup is
	// still recognized as the dumper crashing rather than handed to it.
	DumperThreadId.store(GetCurrentThreadId());

	ULONG guarantee = EXCEPTION_STACK_GUARANTEE;
	SetThreadStackGuarantee(&guarantee);

	if (TestDumperFault) {
		*(volatile int *)16 = 1;
	}

	Prune_Old_Artifact_Folders();

	for (;;) {
		if (WaitForSingleObject(DumperWakeEvent, INFINITE) != WAIT_OBJECT_0) {
			continue;
		}

		__try {
			Write_Crash_Artifacts(&SavedPointers, CrashedThreadId);
		} __except (EXCEPTION_EXECUTE_HANDLER) {
		}

		SetEvent(ArtifactsWrittenEvent);
		Show_Exception_Dialog();
	}
}


/// <summary>
/// Prepares the symbol handler while the process is still healthy.
/// </summary>
/// <remarks>
/// Doing this at crash time would need the loader lock, which the crash may have interrupted
/// another thread holding. The search path is the executable's own directory because the
/// working directory changes during startup and the player chooses where to launch from.
/// </remarks>
static void Initialize_Symbols(void)
{
	SymbolState.store(SymbolStateType::Initializing);

	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME
				| SYMOPT_OMAP_FIND_NEAREST | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_NO_PROMPTS);

	if (!SymInitialize(GetCurrentProcess(), ExecutableDirectory, TRUE)) {
		SymbolState.store(SymbolStateType::Uninitialized);
		return;
	}

	// Deferred loading means the executable's own symbols are not read until something asks
	// for them, and only a lookup asks. Resolving one address now both loads them off the
	// crash path and settles whether they are there at all.
	DWORD_PTR const probe = (DWORD_PTR)&Initialize_Symbols;

	char storage[sizeof(SYMBOL_INFO) + 256];
	memset(storage, 0, sizeof(storage));

	SYMBOL_INFO * const symbol = (SYMBOL_INFO *)storage;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = 255;

	DWORD64 offset = 0;
	SymFromAddr(GetCurrentProcess(), (DWORD64)probe, &offset, symbol);

	IMAGEHLP_MODULE64 module;
	memset(&module, 0, sizeof(module));
	module.SizeOfStruct = sizeof(module);

	if (SymGetModuleInfo64(GetCurrentProcess(), (DWORD64)probe, &module)) {
		SymbolsVerified = (module.SymType == SymPdb);
	}

	SymbolState.store(SymbolStateType::Ready);
}


/// <summary>
/// Records the paths the exception path is not allowed to look up for itself.
/// </summary>
static void Cache_Executable_Paths(void)
{
	if (GetModuleFileNameA(NULL, ExecutablePath, sizeof(ExecutablePath)) == 0) {
		ExecutablePath[0] = '\0';
	}

	strncpy(ExecutableDirectory, ExecutablePath, sizeof(ExecutableDirectory) - 1);
	ExecutableDirectory[sizeof(ExecutableDirectory) - 1] = '\0';

	char * const separator = strrchr(ExecutableDirectory, '\\');
	if (separator != NULL) {
		*separator = '\0';
	} else {
		strcpy(ExecutableDirectory, ".");
	}
}


/// <summary>
/// Installs the crash reporting subsystem. Call this first, before anything else in WinMain.
/// </summary>
void Install_Exception_Handler(void)
{
	// The handler chain goes on before anything that can fail, so that the rest of this
	// function is covered by it. Until the symbol handler and the dumper below are up, a crash
	// is reported without them rather than not reported at all.
	SetUnhandledExceptionFilter(Exception_Filter);
	_set_purecall_handler(Purecall_Handler);
	_set_invalid_parameter_handler(Invalid_Parameter_Handler);
	std::set_terminate(Terminate_Handler);

	// Left alone, abort ends the process through a fast fail that is designed to bypass every
	// user mode handler, so an aborted run would report nothing at all. Turning that off makes
	// it raise the signal instead, which this subsystem can report on.
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	signal(SIGABRT, Abort_Handler);

	MainThreadId = GetCurrentThreadId();

	InitializeCriticalSection(&DbgHelpLock);
	DbgHelpLockReady = true;

	Cache_Executable_Paths();

	// A stack overflow is reported from the little that is left of the stack that overflowed,
	// so the handler needs guaranteed room to run in. Only threads that ask for it get it,
	// which is why an overflow on a worker or an operating system callback thread is reported
	// on a best effort basis.
	ULONG guarantee = EXCEPTION_STACK_GUARANTEE;
	SetThreadStackGuarantee(&guarantee);

	char test[32];
	if (GetEnvironmentVariableA("OPENTS_EXCEPTION_TEST", test, sizeof(test)) != 0) {
		TestDumperFault = (strcmp(test, "dumperinit") == 0);

		if (strcmp(test, "syminit") == 0) {
			// Faulting here proves the filter reports without the symbol handler, which is
			// the state every crash during this function is reported in.
			SymbolState.store(SymbolStateType::Initializing);
			*(volatile int *)16 = 1;
		}
	}

	Initialize_Symbols();

	// Primed because the first call allocates, and the report cannot afford to.
	Version_Name();

	// Both events exist before the thread does, so that a published thread id also means its
	// handoff is ready to use.
	DumperWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ArtifactsWrittenEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (DumperWakeEvent != NULL && ArtifactsWrittenEvent != NULL) {
		DumperThread = CreateThread(NULL, 256 * 1024, Dumper_Thread_Proc, NULL, 0, NULL);
	}
}


/// <summary>
/// Records where the debug log is, so that a crash can copy it without asking the logger.
/// </summary>
/// <param name="path">The log file's full path.</param>
void Exception_Register_Log_File(char const * path)
{
	if (path == NULL) {
		RegisteredLogFile[0] = '\0';
		return;
	}

	strncpy(RegisteredLogFile, path, sizeof(RegisteredLogFile) - 1);
	RegisteredLogFile[sizeof(RegisteredLogFile) - 1] = '\0';
}


/// <summary>
/// Records which test fault the launch options asked for.
/// </summary>
/// <param name="mode">The name of the fault to raise.</param>
void Exception_Set_Test_Mode(char const * mode)
{
	if (mode == NULL) {
		TestMode[0] = '\0';
		return;
	}

	strncpy(TestMode, mode, sizeof(TestMode) - 1);
	TestMode[sizeof(TestMode) - 1] = '\0';
}


static DWORD WINAPI Test_Worker_Thread(LPVOID)
{
	*(volatile int *)16 = 1;
	return(0);
}


static void CALLBACK Test_Timer_Callback(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR)
{
	*(volatile int *)16 = 1;
}


// Exhausting the stack is the whole point of this one, so the compiler's warning about it is
// not news here.
#pragma warning(push)
#pragma warning(disable : 4717)
static unsigned Test_Recurse(unsigned depth)
{
	// Volatile so that the recursion survives optimization and actually consumes the stack.
	volatile char block[4096];
	block[0] = (char)depth;

	return(Test_Recurse(depth + 1) + block[0]);
}
#pragma warning(pop)


class TestPureBaseClass
{
	public:
		TestPureBaseClass(void);
		virtual ~TestPureBaseClass(void) {}
		virtual void Call_It(void) = 0;
};


// Out of line and never inlined, so that the call has to go through the virtual table. While
// the base constructor runs, that slot still holds the runtime's pure call handler.
static __declspec(noinline) void Invoke_Pure_Call(TestPureBaseClass * object)
{
	object->Call_It();
}


TestPureBaseClass::TestPureBaseClass(void)
{
	Invoke_Pure_Call(this);
}


class TestPureDerivedClass : public TestPureBaseClass
{
	public:
		virtual void Call_It(void) override {}
};


/// <summary>
/// Raises the requested test fault, for the faults that need nothing but a running process.
/// </summary>
void Exception_Run_Immediate_Test(void)
{
	if (TestMode[0] == '\0') {
		return;
		}

	if (stricmp(TestMode, "av-read") == 0) {
		volatile int const value = *(volatile int *)16;
		(void)value;

	} else if (stricmp(TestMode, "av-write") == 0) {
		*(volatile int *)16 = 1;

	} else if (stricmp(TestMode, "stack") == 0) {
		Test_Recurse(0);

	} else if (stricmp(TestMode, "purecall") == 0) {
		TestPureDerivedClass instance;

	} else if (stricmp(TestMode, "terminate") == 0) {
		std::terminate();

	} else if (stricmp(TestMode, "invalidparam") == 0) {
		char target[4];
		strcpy_s(target, 0, "longer than the buffer");

	} else if (stricmp(TestMode, "fatal") == 0) {
		Fatal("Requested test failure %d.", 1);

	} else if (stricmp(TestMode, "worker") == 0) {
		HANDLE const thread = CreateThread(NULL, 0, Test_Worker_Thread, NULL, 0, NULL);
		if (thread != NULL) {
			WaitForSingleObject(thread, INFINITE);
	}

	} else if (stricmp(TestMode, "sectionfault") == 0) {
		TestSectionFault = true;
		*(volatile int *)16 = 1;
	}
}


/// <summary>
/// Raises the requested test fault, for the faults that need a window to happen inside.
/// </summary>
void Exception_Run_Post_Window_Test(void)
{
	if (stricmp(TestMode, "wndproc") == 0) {
		PostMessage(MainWindow, WM_EXCEPTION_TEST, 0, 0);

	} else if (stricmp(TestMode, "timer") == 0) {
		timeSetEvent(200, 10, Test_Timer_Callback, 0, TIME_ONESHOT);
	}
}


/// <summary>
/// Faults inside window procedure dispatch, which the operating system unwinds differently.
/// </summary>
void Exception_Wndproc_Test_Fault(void)
{
	*(volatile int *)16 = 1;
}
