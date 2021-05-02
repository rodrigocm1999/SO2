#pragma once
#include <vector>
#include "SharedStructContents.h"
#include "CircularBuffer.h"
#include "Airport.h"

class ControlMain {

public:

	bool exit = false;

	std::vector<Airport*> airports;

	CircularBuffer* const receiving_buffer;

	// points directly to shared memory
	SharedControl* const shared_control;
	Plane* const planes;
	HANDLE const handle_mapped_file;
	HANDLE receiving_thread;
	// --------------------------------

	
	ControlMain(SharedControl* shared_control, Plane* planes,HANDLE handle_mapped_file);

	bool add_airport(Airport* airport);

};