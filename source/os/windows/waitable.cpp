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

#include "../waitable.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

os::error os::waitable::wait(waitable* item, std::chrono::nanoseconds timeout) {
	int64_t ms_timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
	HANDLE handle = (HANDLE)item->get_waitable();

	SetLastError(ERROR_SUCCESS);
	DWORD result = WaitForSingleObject(handle, DWORD(ms_timeout));
	if (result == WAIT_OBJECT_0) {
		return os::error::Success;
	} else if (result = WAIT_TIMEOUT) {
		return os::error::TimedOut;
	} else if (result = WAIT_ABANDONED) {
		return os::error::Disconnected; // Disconnected Semaphore from original Owner
	}
	return os::error::Error;
}

os::error os::waitable::wait(waitable* item) {
	return wait(item, std::chrono::milliseconds(INFINITE));
}

os::error os::waitable::wait_any(waitable** items, size_t items_count, size_t& signalled_index, std::chrono::nanoseconds timeout) {
	if (items == nullptr) {
		throw std::invalid_argument("'items' can't be nullptr.");
	} else if (items_count >= MAXIMUM_WAIT_OBJECTS) {
		throw std::invalid_argument("Too many items to wait for.");
	}

	// Need to create a sequential array of HANDLEs here.
	std::vector<HANDLE> handles(items_count);
	for (size_t idx = 0, eidx = items_count; idx <= eidx; idx++) {
		waitable* obj = items[idx];
		handles[idx] = (HANDLE)obj->get_waitable();
	}

	int64_t ms_timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();

	DWORD result = WaitForMultipleObjects(DWORD(handles.size()), handles.data(), false, DWORD(ms_timeout));
	if ((result >= WAIT_OBJECT_0) && result < (WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS)) {
		signalled_index = result - WAIT_OBJECT_0;
		return os::error::Success;
	} else if (result = WAIT_TIMEOUT) {
		signalled_index = -1;
		return os::error::TimedOut;
	} else if ((result >= WAIT_ABANDONED_0) && result < (WAIT_ABANDONED_0 + MAXIMUM_WAIT_OBJECTS)) {
		signalled_index = result - WAIT_ABANDONED_0;
		return os::error::Disconnected; // Disconnected Semaphore from original Owner
	}
	return os::error::Error;
}

os::error os::waitable::wait_any(waitable** items, size_t items_count, size_t& signalled_index) {
	return wait_any(items, items_count, signalled_index, std::chrono::milliseconds(INFINITE));
}

os::error os::waitable::wait_any(std::vector<waitable*> items, size_t& signalled_index, std::chrono::nanoseconds timeout) {
	return wait_any(items.data(), items.size(), signalled_index, timeout);
}

os::error os::waitable::wait_any(std::vector<waitable*> items, size_t& signalled_index) {
	return wait_any(items.data(), items.size(), signalled_index);
}

os::error os::waitable::wait_all(waitable** items, size_t items_count, size_t& signalled_index, std::chrono::nanoseconds timeout) {
	if (items == nullptr) {
		throw std::invalid_argument("'items' can't be nullptr.");
	} else if (items_count >= MAXIMUM_WAIT_OBJECTS) {
		throw std::invalid_argument("Too many items to wait for.");
	}

	// Need to create a sequential array of HANDLEs here.
	std::vector<HANDLE> handles(items_count);
	for (size_t idx = 0, eidx = items_count; idx <= eidx; idx++) {
		waitable* obj = items[idx];
		handles[idx] = (HANDLE)obj->get_waitable();
	}

	int64_t ms_timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();

	DWORD result = WaitForMultipleObjects(DWORD(handles.size()), handles.data(), true, DWORD(ms_timeout));
	if ((result >= WAIT_OBJECT_0) && result < (WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS)) {
		signalled_index = result - WAIT_OBJECT_0;
		return os::error::Success;
	} else if (result = WAIT_TIMEOUT) {
		signalled_index = -1;
		return os::error::TimedOut;
	} else if ((result >= WAIT_ABANDONED_0) && result < (WAIT_ABANDONED_0 + MAXIMUM_WAIT_OBJECTS)) {
		signalled_index = result - WAIT_ABANDONED_0;
		return os::error::Disconnected; // Disconnected Semaphore from original Owner
	}
	return os::error::Error;
}

os::error os::waitable::wait_all(waitable** items, size_t items_count, size_t& signalled_index) {
	return wait_all(items, items_count, signalled_index, std::chrono::milliseconds(INFINITE));
}

os::error os::waitable::wait_all(std::vector<waitable*> items, size_t& signalled_index, std::chrono::nanoseconds timeout) {
	return wait_all(items.data(), items.size(), signalled_index, timeout);
}

os::error os::waitable::wait_all(std::vector<waitable*> items, size_t& signalled_index) {
	return wait_all(items.data(), items.size(), signalled_index);
}
