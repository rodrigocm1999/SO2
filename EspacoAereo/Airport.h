#pragma once
#include "SharedStructContents.h"
#include <string>
#include <vector>

#define TSTRING std::basic_string<TCHAR>


typedef struct {
	TCHAR name[BUFFER_SIZE];
	//TCHAR origin[BUFFER_SIZE];
	TCHAR destiny[BUFFER_SIZE];
}Passenger;

class Airport{

public:
	const unsigned int id;
	const TSTRING name;
	Position position;

	std::vector<Plane*> planes;
	std::vector<Passenger*> passengers;
	
	
	Airport(int id,const TCHAR *name,int x ,int y);
	
	void add_plane(Plane * plane);
};

