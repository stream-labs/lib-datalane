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

void os::windows::async_request::set_handle(HANDLE handle) {
	this->handle = handle;
	this->valid = false;
}

void os::windows::async_request::set_valid(bool valid) {
	this->valid = valid;
}

os::windows::async_request::async_request(HANDLE handle) {
	this->handle = handle;
	this->valid = false;
}

os::windows::async_request::~async_request() {
	if (is_valid()) {
		cancel();
	}
}

bool os::windows::async_request::is_valid() {
	return this->valid;
}

bool os::windows::async_request::is_complete() {
	if (!is_valid()) {
		throw std::runtime_error("Asynchronous request is invalid.");
	}

	return HasOverlappedIoCompleted(this->get_overlapped_pointer());
}

size_t os::windows::async_request::get_bytes_transferred() {
	if (!is_valid()) {
		throw std::runtime_error("Asynchronous request is invalid.");
	}

	DWORD bytes = 0;
	SetLastError(ERROR_SUCCESS);
	GetOverlappedResult(handle, this->get_overlapped_pointer(), &bytes, false);
	if (GetLastError() == ERROR_IO_INCOMPLETE) {
		throw std::runtime_error("Asynchronous request is not yet complete.");
	}
	return bytes;
}

bool os::windows::async_request::cancel() {
	if (!is_valid()) {
		throw std::runtime_error("Asynchronous request is invalid.");
	}

	if (!is_complete()) {
		return CancelIoEx(handle, this->get_overlapped_pointer());
	}
	return true;
}

void os::windows::async_request::invalidate() {
	valid = false;
}

