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

#include "../waitable.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

os::error os::waitable::wait(waitable *item, std::chrono::nanoseconds timeout) {
	int64_t ms_timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
	HANDLE  handle     = (HANDLE)item->get_waitable();

	SetLastError(ERROR_SUCCESS);
	DWORD result = WaitForSingleObjectEx(handle, DWORD(ms_timeout), TRUE);
	if (result == WAIT_OBJECT_0) {
		return os::error::Success;
	} else if (result = WAIT_TIMEOUT) {
		return os::error::TimedOut;
	} else if (result = WAIT_ABANDONED) {
		return os::error::Disconnected; // Disconnected Semaphore from original Owner
	} else if (result = WAIT_IO_COMPLETION) {
		SetLastError(ERROR_SUCCESS);
		result = WaitForSingleObjectEx(handle, DWORD(ms_timeout), TRUE);
		if (result == WAIT_OBJECT_0) {
			return os::error::Success;
		} else if (result = WAIT_TIMEOUT) {
			return os::error::TimedOut;
		} else if (result = WAIT_ABANDONED) {
			return os::error::Disconnected; // Disconnected Semaphore from original Owner
		}
	}
	return os::error::Error;
}

os::error os::waitable::wait(waitable *item) {
	return wait(item, std::chrono::milliseconds(INFINITE));
}

os::error os::waitable::wait_any(waitable **items, size_t items_count, size_t &signalled_index,
								 std::chrono::nanoseconds timeout) {
	if (items == nullptr) {
		throw std::invalid_argument("'items' can't be nullptr.");
	} else if (items_count >= MAXIMUM_WAIT_OBJECTS) {
		throw std::invalid_argument("Too many items to wait for.");
	}

	// Need to create a sequential array of HANDLEs here.
	std::vector<HANDLE> handles(items_count);
	for (size_t idx = 0, eidx = items_count; idx < eidx; idx++) {
		waitable *obj = items[idx];
		handles[idx]  = (HANDLE)obj->get_waitable();
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

os::error os::waitable::wait_any(waitable **items, size_t items_count, size_t &signalled_index) {
	return wait_any(items, items_count, signalled_index, std::chrono::milliseconds(INFINITE));
}

os::error os::waitable::wait_any(std::vector<waitable *> items, size_t &signalled_index,
								 std::chrono::nanoseconds timeout) {
	return wait_any(items.data(), items.size(), signalled_index, timeout);
}

os::error os::waitable::wait_any(std::vector<waitable *> items, size_t &signalled_index) {
	return wait_any(items.data(), items.size(), signalled_index);
}

os::error os::waitable::wait_all(waitable **items, size_t items_count, size_t &signalled_index,
								 std::chrono::nanoseconds timeout) {
	if (items == nullptr) {
		throw std::invalid_argument("'items' can't be nullptr.");
	} else if (items_count >= MAXIMUM_WAIT_OBJECTS) {
		throw std::invalid_argument("Too many items to wait for.");
	}

	// Need to create a sequential array of HANDLEs here.
	std::vector<HANDLE> handles(items_count);
	for (size_t idx = 0, eidx = items_count; idx < eidx; idx++) {
		waitable *obj = items[idx];
		handles[idx]  = (HANDLE)obj->get_waitable();
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

os::error os::waitable::wait_all(waitable **items, size_t items_count, size_t &signalled_index) {
	return wait_all(items, items_count, signalled_index, std::chrono::milliseconds(INFINITE));
}

os::error os::waitable::wait_all(std::vector<waitable *> items, size_t &signalled_index,
								 std::chrono::nanoseconds timeout) {
	return wait_all(items.data(), items.size(), signalled_index, timeout);
}

os::error os::waitable::wait_all(std::vector<waitable *> items, size_t &signalled_index) {
	return wait_all(items.data(), items.size(), signalled_index);
}
