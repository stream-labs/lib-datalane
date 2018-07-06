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
#include "datalane-socket-server.hpp"
#include "datalane.hpp"

std::shared_ptr<datalane::socket> datalane::listen(std::string socket,
                                                   size_t backlog /*= -1*/) {
	throw std::exception("Not implemented yet.");
	//std::shared_ptr<datalane::server_socket> sock =
	// std::make_shared<datalane::server_socket>(socket, backlog);
	//return std::dynamic_pointer_cast<datalane::socket>(sock);
}

std::shared_ptr<datalane::socket> datalane::connect(std::string socket) {
	throw std::exception("Not implemented yet.");
	//std::shared_ptr<datalane::client_socket> sock =
	// std::make_shared<datalane::client_socket>(socket);
	//return std::dynamic_pointer_cast<datalane::socket>(sock);
}
