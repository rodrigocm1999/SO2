#pragma once
#include "Position.h"
#include "SharedStructContents.h"
#include <string>
#include <vector>

#define TSTRING std::basic_string<TCHAR>

class Airport{
public:
	TSTRING name;
	Position position;
	std::vector<Plane> plane;
	//std::vector<People> people;
	
	Airport(const TCHAR *name,int x ,int y);
};

