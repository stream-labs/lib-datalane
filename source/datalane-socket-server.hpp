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

#ifndef DATALANE_SOCKET_SERVER_HPP
#define DATALANE_SOCKET_SERVER_HPP

#include "datalane-socket.hpp"
#include "os-namedsocket.hpp"

namespace datalane {
	class server_socket : public socket {
		std::shared_ptr<os::named_socket> named_socket;

		public:
		server_socket(std::string name, size_t backlog = -1);
		~server_socket();

		public:
		virtual error write(void* buffer, size_t length, size_t& write_length) override;
		virtual error read(void* buffer, size_t max_length, size_t& read_length) override;
		virtual error disconnect() override;
		virtual bool connected() override;
		virtual bool good() override;
		virtual bool pending() override;
		virtual error accept(std::shared_ptr<datalane::socket>& socket) override;
	};
}

#endif
