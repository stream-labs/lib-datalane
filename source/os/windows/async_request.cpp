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

#include "async_request.hpp"
#include "utility.hpp"

void os::windows::async_request::set_handle(HANDLE handle) {
	this->handle = handle;
	this->valid  = false;
}

void os::windows::async_request::set_valid(bool valid) {
	this->valid = valid;
}

void os::windows::async_request::completion_routine(DWORD dwErrorCode,
                                                    DWORD dwBytesTransmitted,
                                                    OVERLAPPED *ov) {
	os::windows::overlapped *ovp =
	 reinterpret_cast<os::windows::overlapped *>(
	  reinterpret_cast<char *>(ov) + sizeof(OVERLAPPED));

	os::windows::async_request *ar =
	 static_cast<os::windows::async_request *>(ovp);

	if (ar) {
		if (ar->callback) {
			ar->callback(
			 os::windows::utility::translate_error(dwErrorCode),
			 dwBytesTransmitted);
		}
	}

	os::windows::overlapped::completion_routine(
	 dwErrorCode, dwBytesTransmitted, ov);
}

void *os::windows::async_request::get_waitable() {
	return os::windows::overlapped::get_waitable();
}

os::windows::async_request::~async_request() {
	if (is_valid()) {
		cancel();
	}
}

bool os::windows::async_request::is_valid() {
	return this->valid;
}

void os::windows::async_request::invalidate() {
	valid = false;
}

bool os::windows::async_request::is_complete() {
	if (!is_valid()) {
		return false;
	}

	return HasOverlappedIoCompleted(this->get_overlapped_pointer());
}

size_t os::windows::async_request::get_bytes_transferred() {
	if (!is_valid()) {
		return 0;
	}

	DWORD bytes = 0;
	SetLastError(ERROR_SUCCESS);
	GetOverlappedResult(
	 handle, this->get_overlapped_pointer(), &bytes, false);
	if (GetLastError() == ERROR_IO_INCOMPLETE) {
		return 0;
	}
	return bytes;
}

bool os::windows::async_request::cancel() {
	if (!is_valid()) {
		return false;
	}

	if (!is_complete()) {
		return CancelIoEx(handle, this->get_overlapped_pointer());
	}
	return true;
}
