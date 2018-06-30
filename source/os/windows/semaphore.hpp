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

#ifndef OS_WINDOWS_SEMAPHORE
#define OS_WINDOWS_SEMAPHORE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <inttypes.h>
#include "../waitable.hpp"
#include "../tags.hpp"

namespace os {
	namespace windows {
		class semaphore : os::waitable {
			HANDLE handle;

			public:
			semaphore(uint32_t initial_count = 0, uint32_t maximum_count = UINT32_MAX);
			semaphore(os::create_only_t, std::string name, uint32_t initial_count = 0, uint32_t maximum_count = UINT32_MAX);
			semaphore(os::create_or_open_t, std::string name, uint32_t initial_count = 0, uint32_t maximum_count = UINT32_MAX);
			semaphore(os::open_only_t, std::string name);
			~semaphore();
			
			os::error signal(uint32_t count = 1);

			// os::waitable
			protected:
			virtual void* get_waitable() override;

		};
	}
}

#endif