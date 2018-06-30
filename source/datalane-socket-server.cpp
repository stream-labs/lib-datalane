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

#include "datalane-socket-server.hpp"

datalane::server_socket::server_socket(std::string name, size_t backlog /*= -1*/) {
	named_socket = std::make_shared<os::named_socket>();
	named_socket->listen(name, backlog);
}

datalane::server_socket::~server_socket() {
	if (named_socket) {
		disconnect();
	}
}

datalane::error datalane::server_socket::write(void* buffer, size_t length, size_t& write_length) {
	throw std::logic_error("The method or operation is not implemented.");
}

datalane::error datalane::server_socket::read(void* buffer, size_t max_length, size_t& read_length) {
	throw std::logic_error("The method or operation is not implemented.");
}

datalane::error datalane::server_socket::disconnect() {
	named_socket->close();
}

bool datalane::server_socket::connected() {
	return (named_socket != nullptr);
}

bool datalane::server_socket::good() {
	return connected();
}

bool datalane::server_socket::pending() {
	std::shared_ptr<os::named_socket_connection> connection = nullptr;
	return named_socket->accept(connection);
}

datalane::error datalane::server_socket::accept(std::shared_ptr<datalane::socket>& out_socket) {
	std::shared_ptr<os::named_socket_connection> connection = nullptr;
	if (named_socket->accept(connection)) {
		out_socket = std::make_shared<datalane::client_socket>(connection);
		return datalane::error::Ok;
	}
	return datalane::error::Error;
}
