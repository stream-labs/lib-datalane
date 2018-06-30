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

#ifndef DATALANE_SOCKET_CLIENT_HPP
#define DATALANE_SOCKET_CLIENT_HPP

#include "datalane-socket.hpp"
#include "os-namedsocket.hpp"

namespace datalane {
	class client_socket : public socket {
		std::shared_ptr<os::named_socket> socket;
		std::shared_ptr<os::named_socket_connection> connection;

		public:
		client_socket(std::string name);
		client_socket(std::shared_ptr<os::named_socket_connection> connection);

		virtual size_t avail() override;
		virtual size_t avail_total() override;

		virtual error write(void* buffer, size_t length, size_t& write_length) override;
		virtual error read(void* buffer, size_t max_length, size_t& read_length) override;
		
		virtual bool connected() override;
		virtual error disconnect() override;
		
		virtual bool good() override;
		
		virtual bool pending() override;
		virtual error accept(std::shared_ptr<datalane::socket>& socket) override;
	};
}

#endif
