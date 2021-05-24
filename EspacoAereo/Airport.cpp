#include "Airport.h"

using namespace  std;

Passenger::Passenger(Airport* origin, Airport* destiny, const TSTRING& name)
	: origin(origin), destiny(destiny), name(name) {}

Airport::Airport(int id, const TCHAR* name, int x, int y) : name(name), id(id) {
	position.x = x;
	position.y = y;
}

Airport::~Airport() {
	for (auto pair : passengers) {
		delete pair.second;
	}
}

void Airport::add_plane(Plane* plane) {
	planes.push_back(plane);
}

void Airport::add_passenger(Passenger* passenger) {

	vector<Passenger*>* passenger_list = passengers[passenger->destiny->id];

	if (passenger_list == nullptr) {
		auto new_vector = new vector<Passenger*>;
		pair<unsigned int, vector<Passenger*>*> pair(passenger->destiny->id, new_vector);
		passengers.insert(pair);
		passenger_list = new_vector;
	}

	passenger_list->push_back(passenger);
}
