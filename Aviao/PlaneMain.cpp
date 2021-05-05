#include "PlaneMain.h"

#include "SharedStructContents.h"

PlaneMain::PlaneMain(SharedControl* shared_control, Plane* planes, Plane* this_plane, int offset, HANDLE semaphore_plane_counter, HANDLE handle_mapped_file) :
	shared_control(shared_control), planes(planes), this_plane(this_plane), semaphore_plane_counter(semaphore_plane_counter), handle_mapped_file(handle_mapped_file) {

	receiving_buffer = new CircularBuffer(&planes[offset].buffer, offset);
	control_buffer = new CircularBuffer(&shared_control->circular_buffer, CONTROL_MUTEX_PREFIX);
}

PlaneMain::~PlaneMain() {
	delete(this->receiving_buffer);
	delete(this->control_buffer);

	
}
