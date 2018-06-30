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

#ifndef OS_WINDOWS_OVERLAPPED_MANAGER_HPP
#define OS_WINDOWS_OVERLAPPED_MANAGER_HPP

extern "C" { // clang++ compatible
#define NOMINMAX
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <windows.h>
#include <AclAPI.h>
#include <AccCtrl.h>
}

#include <queue>
#include <list>
#include <mutex>
#include <memory>

namespace os {
	namespace windows {
		class overlapped_manager {
			// Security Descriptor Stuff
			SECURITY_ATTRIBUTES sa;
			PSECURITY_DESCRIPTOR pSD = NULL;
			PSID pEveryoneSID = NULL, pAdminSID = NULL;
			PACL pACL = NULL;
			EXPLICIT_ACCESS ea[2];
			SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
			SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;

			// Overlapped Objects
			std::queue<std::shared_ptr<OVERLAPPED>> free_objects;
			std::list<std::shared_ptr<OVERLAPPED>> used_objects;
			std::mutex objects_mtx;

			void create_overlapped(std::shared_ptr<OVERLAPPED>& ov);
			void destroy_overlapped(std::shared_ptr<OVERLAPPED>& ov);
			void append_create_overlapped();

			bool create_security_attributes();
			void destroy_security_attributes();

			public:
			overlapped_manager();
			~overlapped_manager();

			std::shared_ptr<OVERLAPPED> alloc();
			void free(std::shared_ptr<OVERLAPPED> ov);
		};
	}
}

#endif
