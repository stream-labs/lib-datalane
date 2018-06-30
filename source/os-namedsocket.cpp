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

#include "os-namedsocket.hpp"

#define MINIMUM_TIMEOUT 1000
#define MINIMUM_BUFFER_SIZE 32767

#pragma region Named Socket
os::named_socket::named_socket() {
	// Socket is neither initialized or listening.
	m_isInitialized =
		m_isListening = false;

	// Timing out defaults to 5ms.
	m_timeOutWait = std::chrono::milliseconds(5);
	m_timeOutReceive = std::chrono::milliseconds(5);
	m_timeOutSend = std::chrono::milliseconds(5);

	// Buffers default to 1 MB Size.
	m_bufferReceiveSize = MINIMUM_BUFFER_SIZE;
	m_bufferSendSize = MINIMUM_BUFFER_SIZE;
}

os::named_socket::~named_socket() {
	close();
}

#pragma region Options
bool os::named_socket::set_receive_buffer_size(size_t size) {
	if (m_isInitialized)
		return false;
	if (size < MINIMUM_BUFFER_SIZE)
		return false;
	m_bufferReceiveSize = size;
	return true;
}

size_t os::named_socket::get_receive_buffer_size() {
	return m_bufferReceiveSize;
}

bool os::named_socket::set_send_buffer_size(size_t size) {
	if (m_isInitialized)
		return false;
	if (size < MINIMUM_BUFFER_SIZE)
		return false;
	m_bufferSendSize = size;
	return true;
}

size_t os::named_socket::get_send_buffer_size() {
	return m_bufferSendSize;
}

bool os::named_socket::set_wait_timeout(std::chrono::nanoseconds time) {
	if (m_isInitialized)
		return false;
	if (time < std::chrono::nanoseconds(MINIMUM_TIMEOUT))
		return false;
	m_timeOutWait = time;
	return true;
}

std::chrono::nanoseconds os::named_socket::get_wait_timeout() {
	return m_timeOutWait;
}

bool os::named_socket::set_receive_timeout(std::chrono::nanoseconds time) {
	if (m_isInitialized)
		return false;
	if (time < std::chrono::nanoseconds(MINIMUM_TIMEOUT))
		return false;
	m_timeOutReceive = time;
	return true;
}

std::chrono::nanoseconds os::named_socket::get_receive_timeout() {
	return m_timeOutReceive;
}

bool os::named_socket::set_send_timeout(std::chrono::nanoseconds time) {
	if (m_isInitialized)
		return false;
	if (time < std::chrono::nanoseconds(MINIMUM_TIMEOUT))
		return false;
	m_timeOutSend = time;
	return true;
}

std::chrono::nanoseconds os::named_socket::get_send_timeout() {
	return m_timeOutSend;
}
#pragma endregion Options

#pragma region Listen/Connect/Close
bool os::named_socket::listen(std::string path, size_t backlog) {
	if (m_isInitialized)
		return false;

	if (backlog == 0)
		backlog = 1;

	if (!_listen(path, backlog)) {
		_close();
		return false;
	}

	m_isInitialized = true;
	m_isListening = true;
	return true;
}

bool os::named_socket::connect(std::string path) {
	if (m_isInitialized)
		return false;

	if (!_connect(path)) {
		_close();
		return false;
	}

	m_isInitialized = true;
	m_isListening = false;
	return true;
}

bool os::named_socket::close() {
	if (!m_isInitialized)
		return false;

	if (!_close())
		return false;

	m_isInitialized = false;
	return true;
}
#pragma endregion Listen/Connect/Close

#pragma region Server & Client
bool os::named_socket::is_initialized() {
	return m_isInitialized;
}

bool os::named_socket::is_server() {
	return m_isInitialized && m_isListening;
}

bool os::named_socket::is_client() {
	return m_isInitialized && !m_isListening;
}

std::weak_ptr<os::named_socket_connection> os::named_socket::accept() {
	for (auto frnt = m_connections.begin(); frnt != m_connections.end(); frnt++) {
		if ((*frnt)->is_waiting()) {
			return std::weak_ptr<os::named_socket_connection>(*frnt);
		}
	}
	return {};
}

bool os::named_socket::accept(std::shared_ptr<os::named_socket_connection>& conn) {
	for (auto frnt = m_connections.begin(); frnt != m_connections.end(); frnt++) {
		if ((*frnt)->is_waiting()) {
			conn = *frnt;
			return true;
		}
	}
	return false;

}

#pragma endregion Server & Client

#pragma region Client Only
std::shared_ptr<os::named_socket_connection> os::named_socket::get_connection() {
	return m_connections.front();
}
#pragma endregion Client Only
#pragma endregion Named Socket

#pragma region Named Socket Connection
bool os::named_socket_connection::bad() {
	return !good();
}
#pragma endregion Named Socket Connection
