/* Copyright(C) 2018 Michael Fabian Dirks <info@xaymar.com>
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files(the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
*/

#include "semaphore.hpp"
#include <locale>
#include <codecvt>
#include <string>

os::windows::semaphore::semaphore(uint32_t initial_count /*= 0*/, uint32_t maximum_count /*= UINT32_MAX*/) {
	if (initial_count > maximum_count) {
		throw std::invalid_argument("initial_count can't be larger than maximum_count");
	} else if (maximum_count == 0) {
		throw std::invalid_argument("maximum_count can't be 0");
	}

	SetLastError(ERROR_SUCCESS);
	handle = CreateSemaphoreW(NULL, initial_count, maximum_count, NULL);
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Semaphore creation failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

inline std::wstring make_wide_string(std::string name) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(name);
}

inline void validate_params(std::wstring name, DWORD initial_count, DWORD maximum_count) {
	if (initial_count > maximum_count) {
		throw std::invalid_argument("'initial_count' can't be larger than 'maximum_count'.");
	} else if (maximum_count == 0) {
		throw std::invalid_argument("'maximum_count' can't be 0.");
	} else if (name.length() > 0) {
		if (name.length() >= MAX_PATH) {
			std::vector<char> msg(2048);
			sprintf_s(msg.data(), msg.size(), "'name' can't be longer than %lld characters.\0", uint64_t(MAX_PATH));
			throw std::invalid_argument(msg.data());
		}		
	}
}

inline void create_semaphore_impl(HANDLE& handle, std::wstring name, DWORD initial_count, DWORD maximum_count) {
	SetLastError(ERROR_SUCCESS);
	handle = CreateSemaphoreW(NULL, initial_count, maximum_count, name.data());
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Semaphore creation failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

inline void open_semaphore_impl(HANDLE& handle, std::wstring name) {
	SetLastError(ERROR_SUCCESS);
	handle = OpenSemaphoreW(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, name.data());
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Opening Semaphore failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

os::windows::semaphore::semaphore(os::create_only_t, std::string name, uint32_t initial_count /*= 0*/, uint32_t maximum_count /*= UINT32_MAX*/) {
	std::wstring wide_name = make_wide_string(name + '\0');
	validate_params(wide_name, initial_count, maximum_count);
	create_semaphore_impl(handle, wide_name, initial_count, maximum_count);
}

os::windows::semaphore::semaphore(os::create_or_open_t, std::string name, uint32_t initial_count /*= 0*/, uint32_t maximum_count /*= UINT32_MAX*/) {
	std::wstring wide_name = make_wide_string(name + '\0');
	validate_params(wide_name, initial_count, maximum_count);
	try {
		create_semaphore_impl(handle, wide_name, initial_count, maximum_count);
	} catch (...) {
		// There's technically two errors here, but the latter is likely to be more interesting.
		open_semaphore_impl(handle, wide_name);
	}
}

os::windows::semaphore::semaphore(os::open_only_t, std::string name) {
	std::wstring wide_name = make_wide_string(name + '\0');
	validate_params(wide_name, 0, 1);
	open_semaphore_impl(handle, wide_name);
}

os::windows::semaphore::~semaphore() {
	if (handle) {
		CloseHandle(handle);
	}
}

os::error os::windows::semaphore::signal(uint32_t count /*= 1*/) {
	SetLastError(ERROR_SUCCESS);
	DWORD result = ReleaseSemaphore(handle, count, NULL);
	if (result) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Semaphore release failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

void* os::windows::semaphore::get_waitable() {
	return (void*)handle;
}
