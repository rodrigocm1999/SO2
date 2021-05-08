#pragma once
#include <Windows.h>
#include <vector>
#include <string>

#include "SharedStructContents.h"

#define TSTRING std::basic_string<TCHAR>

std::vector<TSTRING> string_split(const TSTRING& ori_str, const TSTRING delimiter);

void* allocate_shared_memory(HANDLE& mapped_file,DWORD size);

HANDLE create_thread(LPTHREAD_START_ROUTINE function_to_run, LPVOID argument);

int grid_distance(const Position& o1, const Position& o2);