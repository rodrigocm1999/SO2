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
	vector<PASSENGER_ID>* passenger_list = passengers[destiny_id];

	if (passenger_list == nullptr) {
		auto* new_vector = new vector<PASSENGER_ID>;
		passengers[destiny_id] = new_vector;
		passenger_list = new_vector;
	}

	passenger_list->push_back(passenger->id);
}

void Airport::add_passengers_list(vector<PASSENGER_ID>* passengers_list, AIRPORT_ID destiny_id) {
	vector<PASSENGER_ID>* already_created = passengers[destiny_id];

	if (already_created == nullptr) {
		passengers.insert(make_pair(destiny_id, passengers_list));
	} else {
		already_created->insert(already_created->end(), passengers_list->begin(), passengers_list->end());
		delete passengers_list;
	}
}


vector<PASSENGER_ID>* Airport::remove_passengers(AIRPORT_ID destiny_id) {
	auto list = passengers[destiny_id];
	passengers.erase(destiny_id);
	return list;
}

vector<PASSENGER_ID>* Airport::give_passengers_to_plane(AIRPORT_ID destiny_id, int plane_capacity) {
	auto list = passengers[destiny_id];
	if (list->size() <= plane_capacity) {
		passengers.erase(destiny_id);
		return list;
	}

	auto into_the_plane = new vector<PASSENGER_ID>(plane_capacity);

	const auto last_element = list->begin() + plane_capacity;

	into_the_plane->insert(into_the_plane->begin(), list->begin(), last_element);
	list->erase(list->begin(), last_element);

	return into_the_plane;
}
