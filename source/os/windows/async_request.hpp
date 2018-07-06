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

#ifndef OS_WINDOWS_ASYNC_REQUEST
#define OS_WINDOWS_ASYNC_REQUEST

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "../async_op.hpp"
#include "overlapped.hpp"

namespace os {
	namespace windows {
		class named_pipe;

		class async_request : public os::windows::overlapped,
		                      public os::async_op {
			HANDLE handle = {0};

			protected:
			// os::windows::overlapped
			void set_handle(HANDLE handle);

			void set_valid(bool valid);

			static void completion_routine(DWORD dwErrorCode,
			                               DWORD dwBytesTransmitted,
			                               OVERLAPPED *ov);

			// os::waitable
			virtual void *get_waitable() override;

			public:
			~async_request();

			virtual bool is_valid() override;

			virtual void invalidate() override;

			virtual bool is_complete() override;

			virtual size_t get_bytes_transferred() override;

			virtual bool cancel() override;

			public:
			friend class named_pipe;
		};
	} // namespace windows
} // namespace os

#endif // OS_WINDOWS_ASYNC_REQUEST
