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
