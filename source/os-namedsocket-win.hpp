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

#ifndef OS_NAMEDSOCKET_WIN_HPP
#define OS_NAMEDSOCKET_WIN_HPP

#include "os-namedsocket.hpp"
#include <map>
#include <mutex>
#include <thread>
#include <queue>
#include <list>
extern "C" { // clang++ compatible
#define NOMINMAX
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <windows.h>
#include <AclAPI.h>
#include <AccCtrl.h>
}

#include "os-windows-overlapped-manager.hpp"

namespace os {

	class named_socket_win : public named_socket {
		public:
		named_socket_win();
		virtual ~named_socket_win();

		protected:
	#pragma region Listen/Connect/Close
		virtual bool _listen(std::string path, size_t backlog) override;
		virtual bool _connect(std::string path) override;
		virtual bool _close() override;
	#pragma endregion Listen/Connect/Close

		// Private Memory
		private:
		std::string m_pipeName;
		DWORD m_openMode;
		DWORD m_pipeMode;

		protected:
		os::windows::overlapped_manager ovm;

		friend class named_socket_connection_win;
	};

	class named_socket_connection_win : public named_socket_connection {
		public:
		named_socket_connection_win(os::named_socket* parent, std::string path, DWORD openFlags, DWORD pipeFlags);
		named_socket_connection_win(os::named_socket* parent, std::string path);
		virtual ~named_socket_connection_win();
		
		// Status
		virtual bool is_waiting() override;
		virtual bool is_connected() override;
		virtual bool connect() override;
		virtual bool disconnect() override;
		virtual bool eof() override;
		virtual bool good() override;

		virtual size_t read_avail() override;
		virtual size_t read_total() override;
		virtual os::error read(char* buffer, size_t length, size_t& read_length) override;
		virtual os::error write(char const* buffer, size_t const length, size_t& write_length) override;

		// Info
		virtual ClientId_t get_client_id() override;

		private:
		static void thread_main(void* ptr);
		void threadlocal();
		static void create_overlapped(OVERLAPPED& ov);
		static void destroy_overlapped(OVERLAPPED& ov);

		private:
		os::named_socket* m_parent;
		HANDLE m_handle;

		bool m_stopWorkers = false;
		std::thread m_managerThread;

		std::mutex m_writeLock, m_readLock;
		std::queue<std::vector<char>>
			m_writeQueue, m_readQueue;

		// Status
		bool m_isServer = false;
		enum class state {
			Sleeping,
			Waiting,
			Connected,
			Disconnected
		};
		state m_state = state::Sleeping;
	};
}

#endif
