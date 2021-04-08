#pragma once
#include <Windows.h>


#define DEFAULT_MAX_PLANES 10

DWORD registry_open_handle(HKEY* registry_handle, TCHAR* key_path);

DWORD registry_get_value(HKEY* registry_handle, TCHAR* key_name, int* value);

DWORD registry_set_value(HKEY* registry_handle, TCHAR* key_path, int new_value);

int get_max_planes_from_registry();