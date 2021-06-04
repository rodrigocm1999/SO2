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
	TSTRING str;

public:
	StartException(tstringstream* stream);

	const TCHAR* get_message();
};