#pragma once
#include "CircularBuffer.h"
#include "SharedStructContents.h"

class PlaneMain {

public:

	bool exit = false;

	bool flight_ready = false;
	Position destiny_position;
	
	CircularBuffer* receiving_buffer;
	CircularBuffer* control_buffer;

	SharedControl* const shared_control;
	Plane* const planes;
	Plane* const this_plane;
	HANDLE const semaphore_plane_counter;
	HANDLE const handle_mapped_file;
	HANDLE receiving_thread;

	PlaneMain(SharedControl* shared_control, Plane* planes, Plane* this_plane, int offset,HANDLE semaphore_plane_counter, HANDLE handle_mapped_file);

	~PlaneMain();
};
