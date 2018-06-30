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

#include "os-windows-overlapped-manager.hpp"

void os::windows::overlapped_manager::create_overlapped(std::shared_ptr<OVERLAPPED>& ov) {
	ov = std::make_shared<OVERLAPPED>();
	ov->Internal = ov->InternalHigh = ov->Offset = ov->OffsetHigh = 0;
	ov->Pointer = nullptr;
	ov->hEvent = CreateEventW(NULL, false, false, NULL);
}

void os::windows::overlapped_manager::destroy_overlapped(std::shared_ptr<OVERLAPPED>& ov) {
	CloseHandle(ov->hEvent);
	ov = nullptr;
}

void os::windows::overlapped_manager::append_create_overlapped() {
	std::shared_ptr<OVERLAPPED> ov = nullptr;
	create_overlapped(ov);
	free_objects.push(ov);
}

bool os::windows::overlapped_manager::create_security_attributes() {
	DWORD dwRes;

	// Create a well-known SID for the Everyone group.
	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
		SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0,
		&pEveryoneSID)) {
		return false;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE.
	// The ACE will allow Everyone read access to the key.
	ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions = KEY_READ;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSID;

	// Create a SID for the BUILTIN\Administrators group.
	if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdminSID)) {
		return false;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE.
	// The ACE will allow the Administrators group full access to
	// the key.
	ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
	ea[1].grfAccessMode = SET_ACCESS;
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName = (LPTSTR)pAdminSID;

	// Create a new ACL that contains the new ACEs.
	dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes) {
		return false;
	}

	// Initialize a security descriptor.  
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (NULL == pSD) {
		return false;
	}

	if (!InitializeSecurityDescriptor(pSD,
		SECURITY_DESCRIPTOR_REVISION)) {
		return false;
	}

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,     // bDaclPresent flag   
		pACL,
		FALSE))   // not a default DACL 
	{
		return false;
	}

	// Initialize a security attributes structure.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;
	return true;
}

void os::windows::overlapped_manager::destroy_security_attributes() {
	if (pEveryoneSID)
		FreeSid(pEveryoneSID);
	if (pAdminSID)
		FreeSid(pAdminSID);
	if (pACL)
		LocalFree(pACL);
	if (pSD)
		LocalFree(pSD);
}

os::windows::overlapped_manager::overlapped_manager() {
	if (!create_security_attributes()) {
		destroy_security_attributes();
		throw std::runtime_error("can't create security attributes");
	}

	// Prepare for 8 queued writes.
	for (size_t idx = 0; idx < 8; idx++) {
		append_create_overlapped();
	}
}

os::windows::overlapped_manager::~overlapped_manager() {
	std::shared_ptr<OVERLAPPED> ov = nullptr;
	while (free_objects.size() > 0) {
		ov = free_objects.front();
		free_objects.pop();
		destroy_overlapped(ov);
	}

	while (used_objects.size() > 0) {
		ov = used_objects.front();
		used_objects.pop_front();
		destroy_overlapped(ov);
	}

	destroy_security_attributes();
}

std::shared_ptr<OVERLAPPED> os::windows::overlapped_manager::alloc() {
	std::unique_lock<std::mutex> ulock(objects_mtx);
	if (free_objects.size() == 0) {
		append_create_overlapped();
	}

	std::shared_ptr<OVERLAPPED> ov = free_objects.front();
	free_objects.pop();
	used_objects.push_back(ov);
	return ov;
}

void os::windows::overlapped_manager::free(std::shared_ptr<OVERLAPPED> ov) {
	std::unique_lock<std::mutex> ulock(objects_mtx);
	for (auto itr = used_objects.begin(); itr != used_objects.end(); itr++) {
		if (*itr == ov) {
			used_objects.erase(itr);
		}
	}
	ResetEvent(ov->hEvent);
	free_objects.push(ov);
}

