#include "ControlMain.h"

#include <iostream>

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

	for (auto passenger : all_passengers) {
		delete passenger.second;
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

	if (plane == nullptr || airport == nullptr) return; // pode acontecer se o controlo crashar e depois aberto sem terminar os aviões
	
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
	all_passengers.insert(make_pair(passenger->id, passenger));
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

	auto list = origin->give_passengers_to_plane(destiny->id, plane->max_passengers);

	for (auto passenger_id : *list) {
		auto passenger = get_passenger_by_id(passenger_id);
		passenger->boarded = true;
		passenger->flying_plane_id = plane->offset;
	}

	boarded_passengers_map[plane_offset] = list;
}

void ControlMain::ended_trip(PLANE_ID plane_offset, int message_type) {
	PassengerMessage message;
	message.type = message_type;
	ended_trip(plane_offset, message);
}

void ControlMain::ended_trip(unsigned char plane_offset, PassengerMessage& message) {
	const auto list = boarded_passengers_map[plane_offset];

	if (list != nullptr) {
		for (auto passenger_id : *list) {
			auto passenger = get_passenger_by_id(passenger_id);
			send_message_to_passenger(passenger, message);
			all_passengers.erase(passenger->id);
			delete passenger;
		}
		boarded_passengers_map.erase(plane_offset);
		delete list;
	}
}

std::vector<PASSENGER_ID>* ControlMain::get_passengers_on_plane(PLANE_ID plane_offset) {
	vector<PASSENGER_ID>* temp = boarded_passengers_map[plane_offset];
	if (temp == nullptr) {
		temp = new vector<PASSENGER_ID>;
		boarded_passengers_map[plane_offset] = temp;
	}
	return temp;
}

std::vector<Passenger*> ControlMain::get_passengers_object_on_plane(PLANE_ID plane_id) {
	auto plane_passengers = get_passengers_on_plane(plane_id);

	vector<Passenger*> list;
	if (plane_passengers != nullptr && !plane_passengers->empty()) {
		list.reserve(plane_passengers->size());

		for (auto passenger : *plane_passengers)
			list.push_back(get_passenger_by_id(passenger));
	}
	return list;
}

bool ControlMain::send_message_to_passenger(Passenger* passenger, const PassengerMessage& message) {
	if (!_send_passenger_message(passenger, message)) {
		remove_passenger(passenger);
		return false;
	}
	return true;
}

bool ControlMain::send_message_to_passenger(Passenger* passenger, int type) {
	PassengerMessage message;
	message.type = type;
	return send_message_to_passenger(passenger, message);
}

Passenger* ControlMain::get_passenger_by_id(PASSENGER_ID id) {
	return all_passengers[id];
}

bool ControlMain::_send_passenger_message(Passenger* passenger, const PassengerMessage& message) {
	DWORD bytes_written;
	return WriteFile(passenger->pipe, &message, sizeof(message), &bytes_written, nullptr) && bytes_written != 0;
}

void ControlMain::remove_passenger(Passenger* passenger) {
	if (passenger == nullptr) return;
	
	all_passengers.erase(passenger->id);

	vector<PASSENGER_ID>* list = nullptr;

	if (passenger->boarded)
		list = get_passengers_on_plane(passenger->flying_plane_id);
	else
		list = passenger->origin->passengers[passenger->destiny->id];

	for (auto it = list->begin(); it < list->end(); ++it) {

		if (*it == passenger->id) {
			list->erase(it);
			break;
		}
	}

	delete passenger;
}

bool ControlMain::change_accept_state()
{
	HANDLE mutex = plane_entering_lock;

	if (accept_state) {
		DWORD result = WaitForSingleObject(mutex, 2000);
		if (result != WAIT_OBJECT_0) {
			//continue;
			//SetWindowTextW(main_struct->accept_window, _T("Something went wrong locking plane entering"));
			return false;
			
		}
		//SetWindowTextW(main_struct->accept_window, _T("New Planes: off"));

	}
	else {
		ReleaseMutex(mutex);
		//SetWindowTextW(main_struct->accept_window, _T("New Planes: on"));

	}
	accept_state = !accept_state;
	return true;
}
