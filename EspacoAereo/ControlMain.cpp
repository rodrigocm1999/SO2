#include "ControlMain.h"


ControlMain::ControlMain(SharedControl* shared_control, Plane* planes, HANDLE handle_mapped_file) :
	shared_control(shared_control),
	planes(planes),
	handle_mapped_file(handle_mapped_file),
	receiving_buffer(new CircularBuffer(&shared_control->circular_buffer, CONTROL_MUTEX_PREFIX)) {

	buffer_planes = new CircularBuffer * [shared_control->max_plane_amount];
}

ControlMain::~ControlMain() {
	delete(this->receiving_buffer);

	for (int i = 0; i < shared_control->max_plane_amount; ++i) {
		if (buffer_planes[i] != nullptr)
			delete(buffer_planes[i]);
	}
}

bool ControlMain::add_airport(Airport* new_one) {

	for (Airport* airport : this->airports)
		if (airport->name == new_one->name || airport->position.x == new_one->position.x && airport->position.y == new_one->position.y)
			return false;

	this->airports.push_back(new_one);
	this->shared_control->map[new_one->position.x][new_one->position.y] = MAP_AIRPORT;
	return true;
}

Airport* ControlMain::get_airport(const std::basic_string<TCHAR> name) {
	for (Airport* airport : this->airports)
		if (airport->name == name)
			return airport;
	return nullptr;
}

CircularBuffer* ControlMain::get_plane_buffer(int offset) {
	if (buffer_planes[offset] == nullptr) {
		buffer_planes[offset] = new CircularBuffer(&planes[offset].buffer, offset);
	}
	return	buffer_planes[offset];
}

