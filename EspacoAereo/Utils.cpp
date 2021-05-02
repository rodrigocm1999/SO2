#include "Utils.h"
#include "SharedStructContents.h"
#include <tchar.h>

using namespace std;

vector<TSTRING> stringSplit(const TSTRING& ori_str, const TSTRING delimiter) {
	vector<TSTRING> list;
	TSTRING str(ori_str);
	size_t pos;
	while ((pos = str.find(delimiter)) != TSTRING::npos) {
		TSTRING token = str.substr(0, pos);
		list.push_back(token);
		str.erase(0, pos + delimiter.length());
	}
	list.push_back(str);
	return list;
}


void* allocate_shared_memory(HANDLE& mapped_file, DWORD size) {
	mapped_file = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, MAPPED_MEMORY_IDENTIFIER);

	if (mapped_file == nullptr) {
		_tprintf(_T("Could not create file mapping object (%d).\n"), GetLastError());
		return nullptr;
	}
	void* shared_mem_pointer = MapViewOfFile(mapped_file, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if (shared_mem_pointer == nullptr) {
		_tprintf(_T("Could not map view of file (%d).\n"), GetLastError());
		CloseHandle(mapped_file);
		return nullptr;
	}

	return shared_mem_pointer;
}

HANDLE create_thread(const LPTHREAD_START_ROUTINE function_to_run, const LPVOID argument) {
	return CreateThread(nullptr, 0, function_to_run, argument, 0, nullptr);
}