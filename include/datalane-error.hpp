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

#ifndef DATALANE_ERROR_HPP
#define DATALANE_ERROR_HPP

namespace datalane {
	enum class error {
		// Everything is fine.
		Ok,
		Success = Ok,

		// A generic error occurred.
		Error,

		// A critical error occurred (unrecoverable, abort program).
		CriticalError,

		// Disconnected
		Disconnected,

		// Timed Out
		TimedOut,
	};
}

#endif // DATALANE_ERROR_HPP
