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

#ifndef OS_ASYNC_OP_HPP
#define OS_ASYNC_OP_HPP

#include <chrono>
#include <vector>
#include <functional>
#include "error.hpp"
#include "waitable.hpp"

namespace os {
	typedef std::function<void(bool success, size_t length)> async_op_cb_t;

	class async_op : public os::waitable {
		bool valid = false;
		async_op_cb_t callback;
		
		public:
		async_op() {};
		async_op(async_op_cb_t u_callback) : callback(u_callback) {};
		virtual ~async_op() {};

		virtual bool is_valid() = 0;

		virtual bool invalidate() = 0;

		virtual bool is_complete() = 0;

		virtual size_t get_bytes_transferred() = 0;

		virtual bool cancel() = 0;

	};
}

#endif