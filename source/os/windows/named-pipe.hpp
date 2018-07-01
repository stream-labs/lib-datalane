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

#ifndef OS_WINDOWS_NAMED_PIPE_HPP
#define OS_WINDOWS_NAMED_PIPE_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <inttypes.h>
#include <string>
#include <memory>
#include "../error.hpp"
#include "../tags.hpp"
#include "async_request.hpp"

namespace os {
	namespace windows {
		enum class pipe_type : int8_t {
			Byte = 0x00,
			//reserved = 0x01,
			//reserved = 0x02,
			//reserved = 0x03,
			Message = 0x04,
		};

		enum class pipe_read_mode : int8_t {
			Byte = 0x00,
			//reserved = 0x01,
			Message = 0x02,
		};

		class named_pipe {
			HANDLE handle;
			
			public:
			named_pipe(os::create_only_t, std::string name, size_t max_instances = PIPE_UNLIMITED_INSTANCES, pipe_type type = pipe_type::Message, pipe_read_mode mode = pipe_read_mode::Message, bool is_unique = false);
			named_pipe(os::create_or_open_t, std::string name, size_t max_instances = PIPE_UNLIMITED_INSTANCES, pipe_type type = pipe_type::Message, pipe_read_mode mode = pipe_read_mode::Message, bool is_unique = false);
			named_pipe(os::open_only_t, std::string name, pipe_read_mode mode = pipe_read_mode::Message);
			~named_pipe();

			os::error read(std::unique_ptr<os::windows::async_request>& request, char* buffer, size_t buffer_length);

			os::error write(std::unique_ptr<os::windows::async_request>& request, const char* buffer, size_t buffer_length);

		};
	}
}

#endif
