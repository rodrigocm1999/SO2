#include "ControlStructs.h"


ControlMain::ControlMain(SharedControl* shared_control, Plane* planes, HANDLE handle_mapped_file) :
	shared_control(shared_control),
	planes(planes),
	handle_mapped_file(handle_mapped_file),
	receiving_buffer(new CircularBuffer(&shared_control->circular_buffer, CONTROL_MUTEX_PREFIX)) {}

bool ControlMain::add_airport(Airport* new_one) {

	for (Airport* airport : this->airports)
		if (airport->name == new_one->name || airport->position.x == new_one->position.x && airport->position.y == new_one->position.y)
			return false;

	this->airports.push_back(new_one);
	return true;
}

