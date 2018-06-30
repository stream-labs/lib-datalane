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

#ifndef OS_NAMEDSOCKET_HPP
#define OS_NAMEDSOCKET_HPP

#include <chrono>
#include <memory>
#include <thread>
#include <string>
#include <vector>
#include <list>
#include "os-error.hpp"

namespace os {
	typedef int64_t ClientId_t;

	class named_socket_connection;
	class named_socket {
		bool is_ready;


		public:





		public:
		static std::unique_ptr<os::named_socket> create();
		typedef void(*connect_handler_t)(std::shared_ptr<os::named_socket_connection> conn);
		typedef void(*disconnect_handler_t)(std::shared_ptr<os::named_socket_connection> conn);

		named_socket();
		virtual ~named_socket();

	#pragma region Options
		/// Adjust the incoming(receive) buffer size.
		bool set_receive_buffer_size(size_t size);
		size_t get_receive_buffer_size();

		/// Adjust the outgoing(send) buffer size.
		bool set_send_buffer_size(size_t size);
		size_t get_send_buffer_size();
		
		/// Adjust the timeout for waiting.
		bool set_wait_timeout(std::chrono::nanoseconds time);
		std::chrono::nanoseconds get_wait_timeout();

		/// Adjust the timeout for receiving data.
		bool set_receive_timeout(std::chrono::nanoseconds time);
		std::chrono::nanoseconds get_receive_timeout();

		/// Adjust the timeout for sending data.
		bool set_send_timeout(std::chrono::nanoseconds time);
		std::chrono::nanoseconds get_send_timeout();
	#pragma endregion Options

	#pragma region Listen/Connect/Close
		// Listen to a Named Socket.
		/// Listens on the specified path for connections of clients. These clients can be local or
		///  on the network depending on what platform this is run on.
		/// It will also attempt to keep a set amount of connections waiting for more clients, also
		///  known as the backlog. A larger backlog can negatively impact performance while a lower
		///  one means that less clients can connect simultaneously, resulting in delays.
		bool listen(std::string path, size_t backlog);

		// Connect to a Named Socket.
		/// Connects to an existing named socket (if possible), otherwise immediately returns false.
		bool connect(std::string path);

		// Close the Named Socket.
		/// Different behavior depending on Initialized mode:
		/// - Create disconnects all clients and closes the socket.
		/// - Connect just disconnects from the socket and closes it.
		bool close();
	#pragma endregion Listen/Connect/Close

	#pragma region Server & Client
		bool is_initialized();
		bool is_server();
		bool is_client();
	#pragma endregion Server & Client

	#pragma region Server Only
		std::weak_ptr<os::named_socket_connection> accept();
		bool accept(std::shared_ptr<os::named_socket_connection>& conn);
	#pragma endregion Server Only

	#pragma region Client Only
		std::shared_ptr<os::named_socket_connection> get_connection();
	#pragma endregion Client Only

		protected:
		virtual bool _listen(std::string path, size_t backlog) = 0;
		virtual bool _connect(std::string path) = 0;
		virtual bool _close() = 0;

		private:
		// Flags
		bool m_isInitialized;
		bool m_isListening;

		// Times for timing out.
		std::chrono::nanoseconds m_timeOutWait;
		std::chrono::nanoseconds m_timeOutReceive;
		std::chrono::nanoseconds m_timeOutSend;

		// Buffers
		size_t m_bufferReceiveSize;
		size_t m_bufferSendSize;

		// IO
		protected:
		std::list<std::shared_ptr<named_socket_connection>> m_connections;
	};

	class named_socket_connection {
		public:
		
		// Status
		virtual bool is_waiting() = 0;
		virtual bool is_connected() = 0;
		virtual bool connect() = 0;
		virtual bool disconnect() = 0;
		virtual bool eof() = 0;
		virtual bool good() = 0;
		virtual bool bad();

		// Reading
		virtual size_t read_avail() = 0;
		virtual size_t read_total() = 0;
		virtual os::error read(char* buffer, size_t length, size_t& read_length) = 0;		
		virtual os::error write(char const* buffer, size_t const length, size_t& write_length) = 0;

		// Info
		virtual ClientId_t get_client_id() = 0;
	};
}

#endif
