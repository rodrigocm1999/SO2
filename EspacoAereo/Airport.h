#pragma once
#include "SharedStructContents.h"
#include <string>
#include <unordered_map>
#include <vector>

#include "Passenger.h"

#define TSTRING std::basic_string<TCHAR>

class Airport {

public:
	const AIRPORT_ID id;
	const TSTRING name;
	Position position;

	std::vector<Plane*> planes; // Plane * in shared memory, do NOT DELETE

	std::unordered_map<AIRPORT_ID, std::vector<PASSENGER_ID>*> passengers; // key = destiny airport , value = vector<Passanger*>*


	Airport(int id, const TCHAR* name, int x, int y);
	~Airport();

	void add_plane(Plane* plane);

	void add_passenger(Passenger* passenger, AIRPORT_ID destiny_id);
	void add_passengers_list(std::vector<PASSENGER_ID>* passengers_list, AIRPORT_ID destiny_id);

	std::vector<PASSENGER_ID>* remove_passengers(AIRPORT_ID destiny_id);
	std::vector<PASSENGER_ID>* give_passengers_to_plane(AIRPORT_ID destiny_id, int plane_capacity);
};
