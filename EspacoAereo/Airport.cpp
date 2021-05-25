#include "Airport.h"


using namespace  std;

Airport::Airport(int id, const TCHAR* name, int x, int y) : name(name), id(id) {
	position.x = x;
	position.y = y;
}

Airport::~Airport() {
	for (auto pair : passengers) {
		delete pair.second; // delete vector<Passenger*>*
	}
}

void Airport::add_plane(Plane* plane) {
	planes.push_back(plane);
}

void Airport::add_passenger(Passenger* passenger, AIRPORT_ID destiny_id) {
	vector<Passenger*>* passenger_list = passengers[destiny_id];

	if (passenger_list == nullptr) {
		auto new_vector = new vector<Passenger*>;
		pair<AIRPORT_ID, vector<Passenger*>*> pair(destiny_id, new_vector);
		passengers.insert(pair);
		passenger_list = new_vector;
	}

	passenger_list->push_back(passenger);
}

void Airport::add_passengers_list(vector<Passenger*>* passengers_list, AIRPORT_ID destiny_id) {
	vector<Passenger*>* already_created = passengers[destiny_id];

	if (already_created == nullptr) {
		pair<AIRPORT_ID, vector<Passenger*>*> pair(destiny_id, passengers_list);
		passengers.insert(pair);
	} else {
		already_created->insert(already_created->end(), passengers_list->begin(), passengers_list->end());
		delete passengers_list;
	}
}


vector<Passenger*>* Airport::remove_passengers(AIRPORT_ID destiny_id) {
	vector<Passenger*>* temp = passengers[destiny_id];
	passengers.erase(destiny_id);
	return temp;
}
