#include "ControlMain.h"

#include "Utils.h"

using namespace std;

ControlMain::ControlMain(SharedControl* shared_control, Plane* planes, HANDLE handle_mapped_file) :
	shared_control(shared_control),
	planes(planes),
	handle_mapped_file(handle_mapped_file),
	receiving_buffer(new CircularBuffer(&shared_control->circular_buffer, CONTROL_MUTEX_PREFIX)) {

	buffer_planes = new CircularBuffer * [shared_control->max_plane_amount];
	ZeroMemory(buffer_planes, sizeof(CircularBuffer*) * shared_control->max_plane_amount);

	shutdown_event = CreateEvent(nullptr, true, false, nullptr);
}

ControlMain::~ControlMain() {
	delete(receiving_buffer);

	for (int i = 0; i < shared_control->max_plane_amount; ++i) {
		if (buffer_planes[i] != nullptr)
			delete(buffer_planes[i]);
	}
	delete(buffer_planes);

	for (auto pair : this->airports) {
		delete(pair.second);
	}

	CloseHandle(receiving_thread);
	UnmapViewOfFile(shared_control);
	CloseHandle(handle_mapped_file);
	CloseHandle(heartbeat_thread);
}

bool ControlMain::add_airport(const TCHAR* name, int x, int y) {
	Position position;
	position.x = x;
	position.y = y;

	for (pair<const int, Airport*> pair : this->airports) {
		Airport* airport = pair.second;
		if (airport->name == name || airport->position.x == x && airport->position.y == y)
			return false;

		if (grid_distance(airport->position, position) <= MINIMUM_AIRPORT_GRID_DISTANCE)
			return  false;
	}

	Airport* airport = new Airport(this->airport_counter, name, x, y);
	this->airport_counter++;

	pair<const int, Airport*> pair(airport->id, airport);
	this->airports.insert(pair);
	this->shared_control->map[airport->position.x][airport->position.y] = MAP_AIRPORT;
	return true;
}

Airport* ControlMain::get_airport(const std::basic_string<TCHAR> name) {
	for (pair<const int, Airport*> pair : this->airports)
		if (pair.second->name == name)
			return pair.second;
	return nullptr;
}

Airport* ControlMain::get_airport(unsigned int id) {
	return airports[id];
}

void ControlMain::plane_left_airport(unsigned char plane_offset) {
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

Plane* ControlMain::get_plane(unsigned char plane_offset) {
	return &planes[plane_offset];
}

CircularBuffer* ControlMain::get_plane_buffer(int offset) {
	if (buffer_planes[offset] == nullptr)
		buffer_planes[offset] = new CircularBuffer(&planes[offset].buffer, offset);
	return buffer_planes[offset];
}

