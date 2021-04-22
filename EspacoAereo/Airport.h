#pragma once
#include "Position.h"
#include <string>
#include <tchar.h>


#ifdef UNICODE
#define tcout wcout
#define tcin wcin
#define tstring wstring
#define tstringstream wstringstream
#else
#define tcout cout
#define tcin cin
#define tstring string
#define tstringstream stringstream
#endif


class Airport{

	Position position;
	std::tstring name;

public:
	
	Airport(TCHAR *name);

};

