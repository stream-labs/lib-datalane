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

#ifndef OS_WINDOWS_OVERLAPPED
#define OS_WINDOWS_OVERLAPPED

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "../waitable.hpp"

namespace os {
	namespace windows {
		class overlapped : public os::waitable {
			OVERLAPPED data;

			public:
			overlapped();
			~overlapped();

			OVERLAPPED* get_overlapped_pointer();

			static void completion_routine(DWORD dwErrorCode, DWORD dwBytesTransmitted, OVERLAPPED* ov);

			// os::waitable
			protected:
			virtual void* get_waitable() override;

		};
	}
}

#endif
