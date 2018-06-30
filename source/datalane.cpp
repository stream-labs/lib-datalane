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

#include "datalane.hpp"
#include "datalane-socket-server.hpp"
#include "datalane-socket-client.hpp"

std::shared_ptr<datalane::socket> datalane::listen(std::string socket, size_t backlog /*= -1*/) {
	std::shared_ptr<datalane::server_socket> sock = std::make_shared<datalane::server_socket>(socket, backlog);
	return std::dynamic_pointer_cast<datalane::socket>(sock);
}

std::shared_ptr<datalane::socket> datalane::connect(std::string socket) {
	std::shared_ptr<datalane::client_socket> sock = std::make_shared<datalane::client_socket>(socket);
	return std::dynamic_pointer_cast<datalane::socket>(sock);
}
