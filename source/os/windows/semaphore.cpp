/* Copyright(C) 2018 Michael Fabian Dirks <info@xaymar.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <codecvt>
#include <locale>
#include <string>
#include "semaphore.hpp"

inline std::wstring make_wide_string(std::string name) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(name);
}

inline void validate_params(std::wstring name, int32_t initial_count, int32_t maximum_count) {
	if (initial_count > maximum_count) {
		throw std::invalid_argument("'initial_count' can't be larger than 'maximum_count'.");
	} else if (initial_count < 0) {
		throw std::invalid_argument("'initial_count' can't be negative.");
	} else if (maximum_count == 0) {
		throw std::invalid_argument("'maximum_count' can't be 0.");
	} else if (maximum_count < 0) {
		throw std::invalid_argument("'maximum_count' can't be negative.");
	} else if (name.length() > 0) {
		if (name.length() >= MAX_PATH) {
			std::vector<char> msg(2048);
			sprintf_s(msg.data(), msg.size(), "'name' can't be longer than %lld characters.\0", uint64_t(MAX_PATH));
			throw std::invalid_argument(msg.data());
		}
	}
}

inline void create_semaphore_impl(HANDLE &handle, std::wstring name, DWORD initial_count, DWORD maximum_count) {
	SetLastError(ERROR_SUCCESS);
	handle = CreateSemaphoreW(NULL, initial_count, maximum_count, name.data());
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Semaphore creation failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

inline void open_semaphore_impl(HANDLE &handle, std::wstring name) {
	SetLastError(ERROR_SUCCESS);
	handle = OpenSemaphoreW(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, name.data());
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Opening Semaphore failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

os::windows::semaphore::semaphore(int32_t initial_count /*= 0*/, int32_t maximum_count /*= UINT32_MAX*/) {
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

os::windows::semaphore::semaphore(os::create_only_t, std::string name, int32_t initial_count /*= 0*/,
								  int32_t maximum_count /*= UINT32_MAX*/) {
	std::wstring wide_name = make_wide_string(name + '\0');
	validate_params(wide_name, initial_count, maximum_count);
	create_semaphore_impl(handle, wide_name, initial_count, maximum_count);
}

os::windows::semaphore::semaphore(os::create_or_open_t, std::string name, int32_t initial_count /*= 0*/,
								  int32_t maximum_count /*= UINT32_MAX*/) {
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
	if (!result) {
		DWORD err = GetLastError();
		if (err == ERROR_TOO_MANY_POSTS) {
			return os::error::TooMuchData;
		}
		return os::error::Error;
	}
	return os::error::Success;
}

void *os::windows::semaphore::get_waitable() {
	return (void *)handle;
}
