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

#include "overlapped.hpp"

os::windows::overlapped::overlapped() {
	data.Internal = data.InternalHigh = data.Offset = data.OffsetHigh = 0;
	data.Pointer = nullptr;
	data.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
}

os::windows::overlapped::~overlapped() {
	CloseHandle(data.hEvent);
}

OVERLAPPED* os::windows::overlapped::get_overlapped_pointer() {
	return &data;
}

void os::windows::overlapped::completion_routine(DWORD dwErrorCode, DWORD dwBytesTransmitted, OVERLAPPED* ov) {
	if (dwErrorCode == ERROR_SUCCESS) {
		SetEvent(ov->hEvent);
	}
}

void* os::windows::overlapped::get_waitable() {
	return (void*)data.hEvent;
}
