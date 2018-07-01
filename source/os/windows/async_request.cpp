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

	CancelIoEx(handle, this->get_overlapped_pointer());
}

