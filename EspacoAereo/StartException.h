#pragma once
#include <exception>
#include <sstream>
#include <string>
#include <tchar.h>

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tstringstream std::wstringstream
#else
#define tstringstream std::stringstream
#endif


class StartException : std::exception {
	TSTRING error_string;

public:
	StartException(tstringstream* stream);
	StartException(const TCHAR* str);

	const TCHAR* get_message();
};