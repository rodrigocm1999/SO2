#pragma once
#include <unordered_map>
#include "Airport.h"
#include "CircularBuffer.h"
#include "SharedStructContents.h"

#define TSTRING std::basic_string<TCHAR>

class ControlMain {

	CircularBuffer** buffer_planes;

public:

	bool exit = false;

	unsigned int airport_counter = 0;
	std::unordered_map<unsigned int, Airport*> airports;

	std::unordered_map<unsigned char, std::vector<Passenger*>> flying_passengers_map; // key = plane offset , value = all passengers

	CircularBuffer* const receiving_buffer;

	// points directly to shared memory
	SharedControl* const shared_control;
	Plane* const planes;
	// --------------------------------
	HANDLE const handle_mapped_file;
	HANDLE const handle_control_named_pipe;
	HANDLE receiving_thread;
	HANDLE heartbeat_thread;
	HANDLE interface_thread;
	HANDLE shutdown_event;
	HANDLE plane_entering_lock;
	HANDLE passenger_receiver;
	

	ControlMain(SharedControl* shared_control, Plane* planes, HANDLE handle_mapped_file, HANDLE handle_control_named_pipe);
	~ControlMain();

	bool add_airport(const TCHAR* name, int x, int y);
	Airport* get_airport(const TSTRING& name);
	Airport* get_airport(unsigned int id);
	void plane_left_airport(unsigned char plane_offset);
	Plane* get_plane(unsigned char plane_offset);

	CircularBuffer* get_plane_buffer(int offset);

};