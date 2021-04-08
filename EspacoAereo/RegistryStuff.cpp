#include "RegistryStuff.h"
#include <Windows.h>
#include <tchar.h>
#include <io.h>
#include <iostream>

#ifdef UNICODE
#define tcout std::wcout
#define tcin std::wcin
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tcout std::cout
#define tcin std::cin
#define tstring std::string
#define tstringstream std::stringstream
#endif

DWORD registry_open_handle(HKEY* registry_handle, TCHAR* key_path) {
	DWORD result;
	LSTATUS what_happened = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		key_path,
		0, NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		registry_handle, &result);

	if (what_happened != ERROR_SUCCESS)
		return -1;
	return what_happened;
}

DWORD registry_get_value(HKEY* registry_handle, TCHAR* key_name, DWORD* value) {
	DWORD var_size;
	LSTATUS what_happened = RegQueryValueEx(*registry_handle, key_name, 0, NULL,
		(LPBYTE)value, &var_size);

	if (what_happened != ERROR_SUCCESS) // Key doesnt exist
		return -1;
	return what_happened;
}

DWORD registry_set_value(HKEY* registry_handle, TCHAR* key_path, int new_value) {
	DWORD var_size = sizeof(new_value);

	LSTATUS what_happened = RegSetValueEx(*registry_handle, key_path, 0, REG_DWORD,
		(const BYTE*)&new_value, var_size);

	if (what_happened != ERROR_SUCCESS)
		return -1;
	return what_happened;
}

#define TAM 40

int get_max_planes_from_registry() {
	HKEY registry_handle;
	TCHAR key_path[TAM] = _T("SOFTWARE\\Airport");
	TCHAR key_name[TAM] = _T("MaximumPlanesAmount");

	DWORD result = registry_open_handle(&registry_handle, key_path);

	if (result != ERROR_SUCCESS) {
		tcout << _T("Key wasn´t created or opened! ERROR!\n");
		return -1;
	}

	DWORD max_planes = DEFAULT_MAX_PLANES;
	result = registry_get_value(&registry_handle, key_name, &max_planes);

	if (result == -1 || max_planes <= 0) {
		result = registry_set_value(&registry_handle, key_name, DEFAULT_MAX_PLANES);
		if (result != ERROR_SUCCESS) {
			tcout << _T("Error setting default MAX_PLANES value \n");
			RegCloseKey(registry_handle);
			return -1;
		}
	}

	RegCloseKey(registry_handle);
	return max_planes;
}