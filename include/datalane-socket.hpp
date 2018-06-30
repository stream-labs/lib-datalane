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

#ifndef DATALANE_SOCKET_HPP
#define DATALANE_SOCKET_HPP

#include "datalane.hpp"
#include "datalane-error.hpp"
#include <memory>

namespace datalane {
	class socket {
		public:
		virtual size_t avail() = 0;
		virtual size_t avail_total() = 0;

		virtual error write(void* buffer, size_t length, size_t& write_length) = 0;
		virtual error read(void* buffer, size_t max_length, size_t& read_length) = 0;

		virtual bool connected() = 0;
		virtual error disconnect() = 0;
		
		virtual bool good() = 0;
		virtual bool bad();

		public: // listen() only
		virtual bool pending() = 0;
		virtual error accept(std::shared_ptr<datalane::socket>& socket) = 0;

	};
}

#endif
