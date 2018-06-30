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

#include "datalane-socket-client.hpp"

datalane::client_socket::client_socket(std::string name) {
	this->socket = std::make_shared<os::named_socket>();
	this->socket->connect(name);
	this->connection = this->socket->get_connection();
}

datalane::client_socket::client_socket(std::shared_ptr<os::named_socket_connection> connection) {
	this->connection = connection;
}

size_t datalane::client_socket::avail() {
	return this->connection->avail();
}

size_t datalane::client_socket::avail_total() {
	return this->connection->avail_total();
}

datalane::error datalane::client_socket::write(void* buffer, size_t length, size_t& write_length) {
	os::error err = this->connection->write((const char*)buffer, length, write_length);
	switch (err) {
		case os::error::Ok:
			return datalane::error::Ok;
		case os::error::Error:
			return datalane::error::Error;
		case os::error::TimedOut:
			return datalane::error::TimedOut;
		case os::error::Disconnected:
			return datalane::error::Disconnected;
	}
	return datalane::error::CriticalError;
}

datalane::error datalane::client_socket::read(void* buffer, size_t max_length, size_t& read_length) {
	throw std::logic_error("The method or operation is not implemented.");
}

datalane::error datalane::client_socket::disconnect() {
	throw std::logic_error("The method or operation is not implemented.");
}

bool datalane::client_socket::connected() {
	
}

bool datalane::client_socket::good() {
	throw std::logic_error("The method or operation is not implemented.");
}

bool datalane::client_socket::pending() {
	throw std::logic_error("The method or operation is not implemented.");
}

datalane::error datalane::client_socket::accept(std::shared_ptr<datalane::socket>& socket) {
	throw std::logic_error("The method or operation is not implemented.");
}
