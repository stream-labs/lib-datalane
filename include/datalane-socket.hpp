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

#ifndef DATALANE_SOCKET_HPP
#define DATALANE_SOCKET_HPP

#include <memory>
#include "datalane-error.hpp"

namespace datalane {
	class socket {
		public:
		virtual size_t avail()       = 0;
		virtual size_t avail_total() = 0;

		virtual error write(void *buffer, size_t length, size_t &write_length)   = 0;
		virtual error read(void *buffer, size_t max_length, size_t &read_length) = 0;

		virtual bool  connected()  = 0;
		virtual error disconnect() = 0;

		virtual bool good() = 0;
		virtual bool bad();

		public: // listen() only
		virtual bool  pending()                                         = 0;
		virtual error accept(std::shared_ptr<datalane::socket> &socket) = 0;
	};
} // namespace datalane

#endif // DATALANE_SOCKET_HPP
