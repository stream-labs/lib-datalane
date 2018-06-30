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

#ifndef OS_WAITABLE
#define OS_WAITABLE

#include <chrono>
#include <vector>
#include "error.hpp"

namespace os {
	class waitable {
		protected:

		virtual void* get_waitable() = 0;

		public:

		inline os::error wait() {
			return os::waitable::wait(this);
		};
		inline os::error wait(std::chrono::nanoseconds timeout) {
			return os::waitable::wait(this, timeout);
		};

		static os::error wait(waitable* item);
		static os::error wait(waitable* item, std::chrono::nanoseconds timeout);

		static os::error wait_any(waitable** items, size_t items_count, size_t& signalled_index);
		static os::error wait_any(waitable** items, size_t items_count, size_t& signalled_index, std::chrono::nanoseconds timeout);

		static os::error wait_any(std::vector<waitable*> items, size_t& signalled_index);
		static os::error wait_any(std::vector<waitable*> items, size_t& signalled_index, std::chrono::nanoseconds timeout);

		static os::error wait_all(waitable** items, size_t items_count, size_t& signalled_index);
		static os::error wait_all(waitable** items, size_t items_count, size_t& signalled_index, std::chrono::nanoseconds timeout);

		static os::error wait_all(std::vector<waitable*> items, size_t& signalled_index);
		static os::error wait_all(std::vector<waitable*> items, size_t& signalled_index, std::chrono::nanoseconds timeout);

	};
}

#endif