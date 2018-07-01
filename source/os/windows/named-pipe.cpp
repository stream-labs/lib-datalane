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

#include "named-pipe.hpp"
#include <locale>
#include <codecvt>
#include <string>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DEFAULT_BUFFER_SIZE 65535
#define DEFAULT_WAIT_TIME 100

#define MAX_PATH_MINUS_PREFIX (MAX_PATH - 9)

inline std::wstring make_wide_string(std::string text) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(text);
}

inline void validate_create_param(std::string name, size_t max_instances) {
	if (name.length() == 0) {
		throw std::invalid_argument("'name' can't be empty.");
	} else if (name.length() >= MAX_PATH_MINUS_PREFIX) {
		throw std::invalid_argument("'name' can't be longer than " TOSTRING(MAX_PATH_MINUS_PREFIX) " characters.");
	} else if (max_instances == 0) {
		throw std::invalid_argument("'max_instances' can't be zero.");
	} else if (max_instances > PIPE_UNLIMITED_INSTANCES) {
		throw std::invalid_argument("'max_instances' can't be greater than " TOSTRING(PIPE_UNLIMITED_INSTANCES));
	}
}

inline void validate_open_param(std::string name) {
	if (name.length() == 0) {
		throw std::invalid_argument("'name' can't be empty.");
	} else if (name.length() >= MAX_PATH_MINUS_PREFIX) {
		throw std::invalid_argument("'name' can't be longer than " TOSTRING(MAX_PATH_MINUS_PREFIX) " characters.");
	}
}

inline std::string make_windows_compatible(std::string& name) {
	std::string out = name;
	for (char& v : out) {
		if (v == '\\') {
			v = '/';
		}
	}
	return { "\\\\.\\pipe\\" + out };
}

inline bool create_logic(HANDLE& handle, std::wstring name, size_t max_instances, os::windows::pipe_type type, os::windows::pipe_read_mode mode, bool is_unique) {
	DWORD pipe_open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OVERLAPPED;
	DWORD pipe_type = 0;
	DWORD pipe_read_mode = 0;

	if (is_unique) {
		pipe_open_mode |= FILE_FLAG_FIRST_PIPE_INSTANCE;
	}

	switch (type) {
		case os::windows::pipe_type::Message:
			pipe_type = PIPE_TYPE_MESSAGE;
			break;
		default:
			pipe_type = PIPE_TYPE_BYTE;
			break;
	}
	
	switch (mode) {
		case os::windows::pipe_read_mode::Message:
			pipe_read_mode = PIPE_READMODE_MESSAGE;
			break;
		default:
			pipe_read_mode = PIPE_READMODE_BYTE;
			break;
	}
	
	SetLastError(ERROR_SUCCESS);
	handle = CreateNamedPipeW(name.c_str(), pipe_open_mode, pipe_type | pipe_read_mode | PIPE_WAIT,
		max_instances, DEFAULT_BUFFER_SIZE, DEFAULT_BUFFER_SIZE, DEFAULT_WAIT_TIME, NULL);
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Creating Named Pipe failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

inline bool open_logic(HANDLE& handle, std::wstring name, os::windows::pipe_read_mode mode) {
	SetLastError(ERROR_SUCCESS);
	handle = CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Opening Named Pipe failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

os::windows::named_pipe::named_pipe(os::create_only_t, std::string name, size_t max_instances /*= PIPE_UNLIMITED_INSTANCES*/, pipe_type type /*= pipe_type::Message*/, pipe_read_mode mode /*= pipe_read_mode::Message*/, bool is_unique /*= false*/) {
	validate_create_param(name, max_instances);

	std::wstring wide_name = make_wide_string(make_windows_compatible(name + '\0'));
	create_logic(handle, wide_name, max_instances, type, mode, is_unique);
}

os::windows::named_pipe::named_pipe(os::create_or_open_t, std::string name, size_t max_instances /*= PIPE_UNLIMITED_INSTANCES*/, pipe_type type /*= pipe_type::Message*/, pipe_read_mode mode /*= pipe_read_mode::Message*/, bool is_unique /*= false*/) {
	validate_create_param(name, max_instances);

	std::wstring wide_name = make_wide_string(make_windows_compatible(name + '\0'));
	if (!create_logic(handle, wide_name, max_instances, type, mode, is_unique)) {
		open_logic(handle, wide_name, mode);
	}
}

os::windows::named_pipe::named_pipe(os::open_only_t, std::string name, pipe_read_mode mode /*= pipe_read_mode::Message*/) {
	validate_open_param(name);

	std::wstring wide_name = make_wide_string(make_windows_compatible(name + '\0'));
	open_logic(handle, wide_name, mode);
}

os::windows::named_pipe::~named_pipe() {
	if (handle) {
		DisconnectNamedPipe(handle);
		CloseHandle(handle);
	}
}

os::error os::windows::named_pipe::read(std::unique_ptr<os::windows::async_request>& request, char* buffer, size_t buffer_length) {
	if (!request) {
		request = std::make_unique<os::windows::async_request>(handle);
	}
	request->set_handle(handle);

	SetLastError(ERROR_SUCCESS);
	if (!ReadFileEx(handle, buffer, buffer_length, request->get_overlapped_pointer(), NULL) || (GetLastError() != ERROR_SUCCESS)) {
		DWORD error = GetLastError();
		if (error == ERROR_MORE_DATA) {
			return os::error::MoreData;
		} else if (error == ERROR_BROKEN_PIPE) {
			return os::error::Disconnected;
		} else if (error != ERROR_IO_PENDING) {
			request->cancel();
			return os::error::Error;
		}
	}

	request->set_valid(true);
	return os::error::Success;
}

os::error os::windows::named_pipe::write(std::unique_ptr<os::windows::async_request>& request, const char* buffer, size_t buffer_length) {
	if (!request) {
		request = std::make_unique<os::windows::async_request>(handle);
	}
	request->set_handle(handle);

	SetLastError(ERROR_SUCCESS);
	if (!WriteFileEx(handle, buffer, buffer_length, request->get_overlapped_pointer(), NULL) || (GetLastError() != ERROR_SUCCESS)) {
		DWORD error = GetLastError();
		if (error == ERROR_MORE_DATA) {
			return os::error::MoreData;
		} else if (error == ERROR_BROKEN_PIPE) {
			return os::error::Disconnected;
		} else if (error != ERROR_IO_PENDING) {
			request->cancel();
			return os::error::Error;
		}
	}

	request->set_valid(true);
	return os::error::Success;
}
