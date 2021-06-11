#pragma once
#include <unordered_map>
#include <unordered_set>

#include "Airport.h"
#include "CircularBuffer.h"
#include "SharedStructContents.h"

#define TSTRING std::basic_string<TCHAR>

class ControlMain {

	CircularBuffer** buffer_planes;

public:

	bool exit = false;

	bool accept_state = true;

	unsigned int airport_counter = 0;
	std::unordered_map<AIRPORT_ID, Airport*> airports;

	std::unordered_map<PLANE_ID, std::vector<PASSENGER_ID>*> boarded_passengers_map; // key = plane offset , value = all passengers

	std::unordered_map<PASSENGER_ID, Passenger*> all_passengers;
	
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

	static LPVOID temp_ptr;


	ControlMain(SharedControl* shared_control, Plane* planes, HANDLE handle_mapped_file, HANDLE handle_control_named_pipe);
	~ControlMain();

	bool add_airport(const TCHAR* name, int x, int y);
	Airport* get_airport(const TSTRING& name);
	Airport* get_airport(AIRPORT_ID id);
	void plane_left_airport(PLANE_ID plane_offset);
	Plane* get_plane(PLANE_ID plane_offset);
	std::vector<Plane*> get_current_planes();
	
	Airport* get_closest_airport(const Position& position, int threshold);
	Plane* get_closest_plane(const Position& position, int threshold);

	bool add_passenger(Passenger* passenger);
	CircularBuffer* get_plane_buffer(PLANE_ID offset) const;
	void board_people(PLANE_ID plane_offset, AIRPORT_ID origin_airport_id, AIRPORT_ID destiny_airport_id);
	void ended_trip(PLANE_ID plane_offset, int message_type);
	void ended_trip(PLANE_ID plane_offset, PassengerMessage& message);
	
	std::vector<PASSENGER_ID>* get_passengers_on_plane(PLANE_ID offset);
	std::vector<Passenger*> get_passengers_object_on_plane(PLANE_ID plane_id);

	bool send_message_to_passenger(Passenger* passenger, const PassengerMessage& message);
	bool send_message_to_passenger(Passenger* passenger, int type);

	Passenger* get_passenger_by_id(PASSENGER_ID id);

	void remove_passenger(Passenger* passenger);

	bool change_accept_state();


private:
	static bool _send_passenger_message(Passenger* passenger, const PassengerMessage& message);
};