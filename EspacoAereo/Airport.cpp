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
		auto* new_vector = new vector<Passenger*>;
		passengers[destiny_id] = new_vector;
		passenger_list = new_vector;
	}

	passenger_list->push_back(passenger);
}

void Airport::add_passengers_list(vector<Passenger*>* passengers_list, AIRPORT_ID destiny_id) {
	vector<Passenger*>* already_created = passengers[destiny_id];

	if (already_created == nullptr) {
		passengers.insert(make_pair(destiny_id, passengers_list));
	} else {
		already_created->insert(already_created->end(), passengers_list->begin(), passengers_list->end());
		delete passengers_list;
	}
}


vector<Passenger*>* Airport::remove_passengers(AIRPORT_ID destiny_id) {
	vector<Passenger*>* list = passengers[destiny_id];
	passengers.erase(destiny_id);
	return list;
}

vector<Passenger*>* Airport::give_passengers_to_plane(AIRPORT_ID destiny_id, int plane_capacity) {
	vector<Passenger*>* list = passengers[destiny_id];
	if (list->size() <= plane_capacity) {
		passengers.erase(destiny_id);
		return list;

	} else {
		auto into_the_plane = new vector<Passenger*>(plane_capacity);

		into_the_plane->insert(into_the_plane->begin(), list->begin(), list->begin() + plane_capacity);
		list->erase(list->begin(), list->begin() + plane_capacity);

		return into_the_plane;
	}
}
