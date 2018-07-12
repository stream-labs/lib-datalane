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
#include "named-pipe.hpp"
#include "utility.hpp"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DEFAULT_BUFFER_SIZE 16 * 1024 * 1024
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

inline std::string make_windows_compatible(std::string &name) {
	std::string out = name;
	for (char &v : out) {
		if (v == '\\') {
			v = '/';
		}
	}
	return {"\\\\.\\pipe\\" + out};
}

inline void create_logic(HANDLE &handle, std::wstring name, size_t max_instances, os::windows::pipe_type type,
						 os::windows::pipe_read_mode mode, bool is_unique) {
	DWORD pipe_open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OVERLAPPED;
	DWORD pipe_type      = 0;
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
							  DWORD(max_instances), DEFAULT_BUFFER_SIZE, DEFAULT_BUFFER_SIZE, DEFAULT_WAIT_TIME, NULL);
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Creating Named Pipe failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}
}

inline void open_logic(HANDLE &handle, std::wstring name, os::windows::pipe_read_mode mode) {
	SetLastError(ERROR_SUCCESS);
	handle = CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
						 FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);
	if (!handle || (GetLastError() != ERROR_SUCCESS)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Opening Named Pipe failed with error code %lX.\0", GetLastError());
		throw std::runtime_error(msg.data());
	}

	DWORD pipe_read_mode = PIPE_WAIT;
	switch (mode) {
	case os::windows::pipe_read_mode::Message:
		pipe_read_mode |= PIPE_READMODE_MESSAGE;
		break;
	default:
		pipe_read_mode |= PIPE_READMODE_BYTE;
		break;
	}

	SetLastError(ERROR_SUCCESS);
	if (!SetNamedPipeHandleState(handle, &pipe_read_mode, NULL, NULL)) {
		std::vector<char> msg(2048);
		sprintf_s(msg.data(), msg.size(), "Changing Named Pipe read mode failed with error code %lX.\0",
				  GetLastError());
		throw std::runtime_error(msg.data());
	}
}

os::windows::named_pipe::named_pipe(os::create_only_t, std::string name,
									size_t         max_instances /*= PIPE_UNLIMITED_INSTANCES*/,
									pipe_type      type /*= pipe_type::Message*/,
									pipe_read_mode mode /*= pipe_read_mode::Message*/, bool is_unique /*= false*/) {
	validate_create_param(name, max_instances);

	std::wstring wide_name = make_wide_string(make_windows_compatible(name + '\0'));
	create_logic(handle, wide_name, max_instances, type, mode, is_unique);
	created = true;
}

os::windows::named_pipe::named_pipe(os::create_or_open_t, std::string name,
									size_t         max_instances /*= PIPE_UNLIMITED_INSTANCES*/,
									pipe_type      type /*= pipe_type::Message*/,
									pipe_read_mode mode /*= pipe_read_mode::Message*/, bool is_unique /*= false*/) {
	validate_create_param(name, max_instances);

	std::wstring wide_name = make_wide_string(make_windows_compatible(name + '\0'));
	try {
		create_logic(handle, wide_name, max_instances, type, mode, is_unique);
		created = true;
	} catch (...) {
		open_logic(handle, wide_name, mode);
	}
}

os::windows::named_pipe::named_pipe(os::open_only_t, std::string name,
									pipe_read_mode mode /*= pipe_read_mode::Message*/) {
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

os::error os::windows::named_pipe::available(size_t &avail) {
	DWORD bytes = 0;
	SetLastError(ERROR_SUCCESS);
	if (!PeekNamedPipe(handle, NULL, NULL, NULL, NULL, &bytes) || (GetLastError() != ERROR_SUCCESS)) {
		switch (GetLastError()) {
		case ERROR_BROKEN_PIPE:
			return os::error::Disconnected;
		default:
			return os::error::Error;
		}
	}
	avail = bytes;
	return os::error::Success;
}

os::error os::windows::named_pipe::total_available(size_t &avail) {
	DWORD bytes = 0;
	SetLastError(ERROR_SUCCESS);
	if (!PeekNamedPipe(handle, NULL, NULL, NULL, &bytes, NULL) || (GetLastError() != ERROR_SUCCESS)) {
		switch (GetLastError()) {
		case ERROR_BROKEN_PIPE:
			return os::error::Disconnected;
		default:
			return os::error::Error;
		}
	}
	avail = bytes;
	return os::error::Success;
}

os::error os::windows::named_pipe::read(std::unique_ptr<os::windows::async_request> &request, char *buffer,
										size_t buffer_length) {
	if (!request) {
		request = std::make_unique<os::windows::async_request>();
	}
	request->set_handle(handle);

	SetLastError(ERROR_SUCCESS);
	if (!ReadFileEx(handle, buffer, DWORD(buffer_length), request->get_overlapped_pointer(),
					os::windows::async_request::completion_routine)
		|| (GetLastError() != ERROR_SUCCESS)) {
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

os::error os::windows::named_pipe::write(std::unique_ptr<os::windows::async_request> &request, const char *buffer,
										 size_t buffer_length) {
	if (!request) {
		request = std::make_unique<os::windows::async_request>();
	}
	request->set_handle(handle);

	SetLastError(ERROR_SUCCESS);
	if (!WriteFileEx(handle, buffer, DWORD(buffer_length), request->get_overlapped_pointer(),
					 os::windows::async_request::completion_routine)
		|| (GetLastError() != ERROR_SUCCESS)) {
		DWORD error = GetLastError();
		if (error == ERROR_MORE_DATA) {
			return os::error::MoreData;
		} else if (error == ERROR_BROKEN_PIPE) {
			return os::error::Disconnected;
		} else if (error != ERROR_IO_PENDING) {
			return os::error::Error;
		}
	}

	request->set_valid(true);
	return os::error::Success;
}

os::error os::windows::named_pipe::read(char *buffer, size_t buffer_length, std::shared_ptr<os::async_op> &op,
										os::async_op_cb_t cb) {
	os::error ec;

	std::shared_ptr<os::windows::async_request> ar = std::static_pointer_cast<os::windows::async_request>(op);
	if (!ar) {
		ar = std::make_shared<os::windows::async_request>();
	}
	op = std::static_pointer_cast<os::async_op>(ar);
	ar->set_callback(cb);
	ar->set_handle(handle);

	SetLastError(ERROR_SUCCESS);
	BOOL  suc   = ReadFileEx(handle, buffer, DWORD(buffer_length), ar->get_overlapped_pointer(),
                          os::windows::async_request::completion_routine);
	DWORD error = GetLastError();
	ec          = utility::translate_error(error);

	if (suc == 0) {
		ar->call_callback(ec, buffer_length);
		ar->cancel();
		return ec;
	}

	ar->set_valid(true);
	return ec;
}

os::error os::windows::named_pipe::write(const char *buffer, size_t buffer_length, std::shared_ptr<os::async_op> &op,
										 os::async_op_cb_t cb) {
	os::error ec;

	std::shared_ptr<os::windows::async_request> ar = std::static_pointer_cast<os::windows::async_request>(op);
	if (!ar) {
		ar = std::make_shared<os::windows::async_request>();
	}
	op = std::static_pointer_cast<os::async_op>(ar);
	ar->set_callback(cb);
	ar->set_handle(handle);

	SetLastError(ERROR_SUCCESS);
	BOOL  suc   = WriteFileEx(handle, buffer, DWORD(buffer_length), ar->get_overlapped_pointer(),
                           os::windows::async_request::completion_routine);
	DWORD error = GetLastError();
	ec          = utility::translate_error(error);

	if (suc == 0) {
		ar->call_callback(ec, buffer_length);
		ar->cancel();
		return ec;
	}

	ar->set_valid(true);
	return ec;
}

bool os::windows::named_pipe::is_created() {
	return created;
}

bool os::windows::named_pipe::is_connected() {
	size_t avail;
	return available(avail) == os::error::Success;
}

os::error os::windows::named_pipe::accept(std::unique_ptr<os::windows::async_request> &request) {
	std::shared_ptr<os::windows::async_request> ars = std::move(request);
	os::error                                   ec  = accept(std::static_pointer_cast<os::async_op>(ars), nullptr);
	request.reset(ars.get());
	return ec;
}

os::error os::windows::named_pipe::accept(std::shared_ptr<os::async_op> &op, os::async_op_cb_t cb) {
	os::error ec;

	if (!is_created()) {
		return os::error::Error;
	}

	std::shared_ptr<os::windows::async_request> ar = std::static_pointer_cast<os::windows::async_request>(op);
	if (!ar) {
		ar = std::make_shared<os::windows::async_request>();
	}
	op = std::static_pointer_cast<os::async_op>(ar);
	ar->set_callback(cb);
	ar->set_handle(handle);

	SetLastError(ERROR_SUCCESS);
	BOOL suc = ConnectNamedPipe(handle, ar->get_overlapped_pointer());
	ec       = utility::translate_error(GetLastError());

	if (ec != os::error::Pending && ec != os::error::Connected) {
		ar->call_callback(ec, 0);
		ar->cancel();
		return ec;
	}

	ar->set_valid(true);
	if (ec == os::error::Connected) {
		ar->call_callback(ec, 0);
	}

	return ec;
}
