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
