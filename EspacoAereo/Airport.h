#pragma once
#include "PVector.h"
#include <string>
#include <tchar.h>

#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

class Airport{

	PVector position;
	tstring name;

public:
	

	Airport(TCHAR *name);


};

