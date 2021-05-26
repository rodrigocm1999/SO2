#include "ControlMain.h"

#include "Utils.h"

using namespace std;

ControlMain::ControlMain(SharedControl* shared_control, Plane* planes, HANDLE handle_mapped_file,
						 HANDLE handle_control_named_pipe) :
	shared_control(shared_control), planes(planes),
	handle_mapped_file(handle_mapped_file), handle_control_named_pipe(handle_control_named_pipe),
	receiving_buffer(new CircularBuffer(&shared_control->circular_buffer, CONTROL_MUTEX_PREFIX)) {
	buffer_planes = new CircularBuffer * [shared_control->max_plane_amount];
	ZeroMemory(buffer_planes, sizeof(CircularBuffer*) * shared_control->max_plane_amount);

	shutdown_event = CreateEvent(nullptr, true, false, nullptr);

	plane_entering_lock = CreateMutex(nullptr, FALSE, PLANE_LOCK_MUTEX);
}

ControlMain::~ControlMain() {
	delete(receiving_buffer);

	for (int i = 0; i < shared_control->max_plane_amount; ++i) {
		if (buffer_planes[i] != nullptr)
			delete(buffer_planes[i]);
	}
	delete(buffer_planes);

	for (auto pair : this->airports) {
		delete pair.second;
	}

	CloseHandle(receiving_thread);
	UnmapViewOfFile(shared_control);
	CloseHandle(handle_mapped_file);
	CloseHandle(heartbeat_thread);
	CloseHandle(shutdown_event);
	CloseHandle(plane_entering_lock);
	CloseHandle(handle_control_named_pipe);
}

bool ControlMain::add_airport(const TCHAR* name, int x, int y) {
	Position position;
	position.x = x;
	position.y = y;

	for (auto pair : this->airports) {
		Airport* airport = pair.second;
		if (airport->name == name || airport->position.x == x && airport->position.y == y)
			return false;

		if (grid_distance(airport->position, position) <= MINIMUM_AIRPORT_GRID_DISTANCE)
			return false;
	}

	Airport* airport = new Airport(this->airport_counter, name, x, y);
	this->airport_counter++;

	this->airports[airport->id] = airport;
	this->shared_control->map[airport->position.x][airport->position.y] = MAP_AIRPORT;
	return true;
}

Airport* ControlMain::get_airport(const TSTRING& name) {
	for (auto pair : this->airports)
		if (pair.second->name == name)
			return pair.second;
	return nullptr;
}

Airport* ControlMain::get_airport(AIRPORT_ID id) {
	return airports[id];
}

void ControlMain::plane_left_airport(PLANE_ID plane_offset) {
	Plane* plane = get_plane(plane_offset);
	Airport* airport = get_airport(plane->origin_airport_id);

	for (unsigned int i = 0; i < airport->planes.size(); ++i) {
		Plane* cur_plane = airport->planes[i];
		if (cur_plane == plane) {
			airport->planes.erase(airport->planes.begin() + i);
			break;
		}
	}
}

Plane* ControlMain::get_plane(PLANE_ID plane_offset) {
	return &planes[plane_offset];
}

bool ControlMain::add_passenger(Passenger* passenger) {
	all_passengers.insert(passenger);
	passenger->origin->add_passenger(passenger, passenger->destiny->id);
	return true;
}

CircularBuffer* ControlMain::get_plane_buffer(PLANE_ID offset) const {
	if (buffer_planes[offset] == nullptr)
		buffer_planes[offset] = new CircularBuffer(&planes[offset].buffer, offset);
	return buffer_planes[offset];
}

void ControlMain::board_people(PLANE_ID plane_offset, AIRPORT_ID origin_airport_id, AIRPORT_ID destiny_airport_id) {
	Airport* origin = get_airport(origin_airport_id);
	Airport* destiny = get_airport(destiny_airport_id);

	Plane* plane = get_plane(plane_offset);

	std::vector<Passenger*>* list = origin->give_passengers_to_plane(destiny->id, plane->max_passengers);
	
	boarded_passengers_map[plane_offset] = list;
}

void ControlMain::ended_trip(PLANE_ID plane_offset, int message_type) {
	PassengerMessage message;
	message.type = message_type;
	ended_trip(plane_offset, message);
}

void ControlMain::ended_trip(unsigned char plane_offset, PassengerMessage& message) {
	const auto list = boarded_passengers_map[plane_offset];

	for (Passenger* passenger : *list) {
		passenger->send_message(message);
		all_passengers.erase(passenger);
		delete passenger;
	}
	boarded_passengers_map.erase(plane_offset);
	delete list;
}

std::vector<Passenger*>* ControlMain::get_passengers_on_plane(PLANE_ID plane_offset) {
	//TODO might cause problems with empty pairs on the map
	return boarded_passengers_map[plane_offset];
}
