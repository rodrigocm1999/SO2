#pragma once
#include "SharedStructContents.h"
#include <string>
#include <unordered_map>
#include <vector>

#define TSTRING std::basic_string<TCHAR>

class Airport;

class Passenger {

public:
	Airport* origin;
	Airport* destiny;
	TSTRING name;

	Passenger(Airport* origin, Airport* destiny, const TSTRING& name);
};

class Airport {

public:
	const unsigned int id;
	const TSTRING name;
	Position position;

	std::vector<Plane*> planes; // Plane * in shared memory, do NOT DELETE
	std::unordered_map<unsigned int, std::vector<Passenger*>*> passengers; // key = destiny airport , value = vector<Passanger*>


	Airport(int id, const TCHAR* name, int x, int y);
	~Airport();
	
	void add_plane(Plane* plane);

	void add_passenger(Passenger* passenger);
};
