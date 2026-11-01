/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "contentnet.h"

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <winhttp.h>
#include <shobjidl.h>
#include <objbase.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <string>


namespace
{

std::wstring Widen(std::string const & text)
{
	if (text.empty()) {
		return(std::wstring());
	}
	int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0);
	std::wstring wide((std::size_t)length, L'\0');
	if (length > 0) {
		MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (int)text.size(), wide.data(), length);
	}
	return(wide);
}


std::string Narrow(wchar_t const * text)
{
	if (text == nullptr || text[0] == L'\0') {
		return(std::string());
	}
	int length = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
	if (length <= 1) {
		return(std::string());
	}
	std::string narrow((std::size_t)length - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, text, -1, narrow.data(), length, nullptr, nullptr);
	return(narrow);
}


bool Parse_Content_Range(std::wstring const & value, std::uint64_t & start, std::uint64_t & total)
{
	if (value.rfind(L"bytes ", 0) != 0) {
		return(false);
	}
	wchar_t const * text = value.c_str() + 6;
	if (*text == L'*') {
		return(false);
	}
	wchar_t * end = nullptr;
	start = std::wcstoull(text, &end, 10);
	if (end == text || *end != L'-') {
		return(false);
	}
	wchar_t const * slash = std::wcschr(end, L'/');
	if (slash == nullptr || slash[1] == L'*') {
		return(false);
	}
	text = slash + 1;
	total = std::wcstoull(text, &end, 10);
	return(end != text);
}

}


struct ContentHttpGet::State
{
	HINTERNET Session = nullptr;
	HINTERNET Connect = nullptr;
	HINTERNET Request = nullptr;
	std::uint64_t TotalBytes = 0;
};


ContentHttpGet::ContentHttpGet(void) :
	Data(std::make_unique<State>())
{
}


ContentHttpGet::~ContentHttpGet(void)
{
	Close();
}


bool ContentHttpGet::Open(char const * url, char const * user_agent, char const * key, std::uint64_t offset, std::string & error)
{
	Close();
	if (url == nullptr || url[0] == '\0') {
		error = "The download address is empty.";
		return(false);
	}

	std::wstring wide_url = Widen(url);
	std::wstring agent = Widen(user_agent != nullptr ? user_agent : "siliconsun");
	Data->Session = WinHttpOpen(agent.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (Data->Session == nullptr) {
		error = "The download connection could not be opened.";
		return(false);
	}
	WinHttpSetTimeouts(Data->Session, 30000, 30000, 30000, 60000);

	URL_COMPONENTS parts;
	std::memset(&parts, 0, sizeof(parts));
	parts.dwStructSize = sizeof(parts);
	wchar_t host[256];
	wchar_t path[2048];
	parts.lpszHostName = host;
	parts.dwHostNameLength = (DWORD)(sizeof(host) / sizeof(host[0]));
	parts.lpszUrlPath = path;
	parts.dwUrlPathLength = (DWORD)(sizeof(path) / sizeof(path[0]));
	if (!WinHttpCrackUrl(wide_url.c_str(), 0, 0, &parts)) {
		error = "The download address could not be read.";
		Close();
		return(false);
	}

	bool secure = parts.nScheme == INTERNET_SCHEME_HTTPS;
	Data->Connect = WinHttpConnect(Data->Session, host, parts.nPort, 0);
	if (Data->Connect == nullptr) {
		error = "The download server could not be reached.";
		Close();
		return(false);
	}

	Data->Request = WinHttpOpenRequest(Data->Connect, L"GET", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
	if (Data->Request == nullptr) {
		error = "The download request could not be opened.";
		Close();
		return(false);
	}

	if (key != nullptr && key[0] != '\0') {
		std::wstring key_header = L"X-Siliconsun-Key: " + Widen(key);
		WinHttpAddRequestHeaders(Data->Request, key_header.c_str(), (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
	}

	if (offset > 0) {
		wchar_t range[80];
		std::swprintf(range, sizeof(range) / sizeof(range[0]), L"Range: bytes=%llu-", (unsigned long long)offset);
		WinHttpAddRequestHeaders(Data->Request, range, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
	}

	if (!WinHttpSendRequest(Data->Request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
		|| !WinHttpReceiveResponse(Data->Request, nullptr)) {
		error = "The download connection failed.";
		Close();
		return(false);
	}

	DWORD status = 0;
	DWORD status_size = sizeof(status);
	if (!WinHttpQueryHeaders(Data->Request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size, WINHTTP_NO_HEADER_INDEX)) {
		error = "The download server sent no status.";
		Close();
		return(false);
	}

	if (status == 429) {
		error = "The download server asked to wait. Try again shortly.";
		Close();
		return(false);
	}
	if (status == 416) {
		error = "The download could not be resumed.";
		Close();
		return(false);
	}
	if (status != 200 && status != 206) {
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "The download server returned %lu.", (unsigned long)status);
		error = buffer;
		Close();
		return(false);
	}

	if (status == 206) {
		wchar_t range_header[128];
		DWORD range_size = sizeof(range_header);
		DWORD index = WINHTTP_NO_HEADER_INDEX;
		if (!WinHttpQueryHeaders(Data->Request, WINHTTP_QUERY_CUSTOM, L"Content-Range", range_header, &range_size, &index)) {
			error = "The download server resumed without a range.";
			Close();
			return(false);
		}
		std::uint64_t start = 0;
		std::uint64_t total = 0;
		if (!Parse_Content_Range(range_header, start, total) || start != offset) {
			error = "The download server resumed at the wrong position.";
			Close();
			return(false);
		}
		Data->TotalBytes = total;
	} else if (offset > 0) {
		error = "The download server did not resume.";
		Close();
		return(false);
	} else {
		wchar_t length_header[64];
		DWORD length_size = sizeof(length_header);
		DWORD index = WINHTTP_NO_HEADER_INDEX;
		if (!WinHttpQueryHeaders(Data->Request, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX, length_header, &length_size, &index)) {
			error = "The download server did not say how large the file is.";
			Close();
			return(false);
		}
		Data->TotalBytes = std::wcstoull(length_header, nullptr, 10);
	}

	return(true);
}


int ContentHttpGet::Read(void * buffer, int capacity, std::string & error)
{
	if (Data->Request == nullptr || buffer == nullptr || capacity <= 0) {
		error = "The download connection failed.";
		return(-1);
	}
	DWORD got = 0;
	if (!WinHttpReadData(Data->Request, buffer, (DWORD)capacity, &got)) {
		error = "The download connection failed.";
		return(-1);
	}
	return((int)got);
}


void ContentHttpGet::Close(void)
{
	if (Data == nullptr) {
		return;
	}
	if (Data->Request != nullptr) {
		WinHttpCloseHandle(Data->Request);
		Data->Request = nullptr;
	}
	if (Data->Connect != nullptr) {
		WinHttpCloseHandle(Data->Connect);
		Data->Connect = nullptr;
	}
	if (Data->Session != nullptr) {
		WinHttpCloseHandle(Data->Session);
		Data->Session = nullptr;
	}
}


std::uint64_t ContentHttpGet::Total(void) const
{
	return(Data->TotalBytes);
}


bool Content_Pick_Folder(std::string & path)
{
	HRESULT started = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	bool release = SUCCEEDED(started);

	IFileDialog * dialog = nullptr;
	HRESULT result = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
	if (FAILED(result)) {
		if (release) {
			CoUninitialize();
		}
		return(false);
	}

	DWORD options = 0;
	dialog->GetOptions(&options);
	dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
	dialog->SetTitle(L"Tiberian Sun files");
	result = dialog->Show(nullptr);

	bool picked = false;
	if (SUCCEEDED(result)) {
		IShellItem * item = nullptr;
		if (SUCCEEDED(dialog->GetResult(&item))) {
			PWSTR display = nullptr;
			if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &display))) {
				path = Narrow(display);
				picked = !path.empty();
				CoTaskMemFree(display);
			}
			item->Release();
		}
	}
	dialog->Release();
	if (release) {
		CoUninitialize();
	}
	return(picked);
}

#endif
