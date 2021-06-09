#include "MainBreakdown.h"

#include <iostream>
#include <sstream>

#include "ControlFunctions.h"
#include "RegistryStuff.h"
#include "StartException.h"
#include "Utils.h"

using namespace std;

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tcout wcout
#define tcin wcin
#define tstringstream wstringstream
#else
#define tcout cout
#define tcin cin
#define tstringstream stringstream
#endif

void wait_for_threads_to_finish(ControlMain* control_main) {
	// Wait for them to finish -----------------------------------------------------------
	const int handle_amount = 3;
	HANDLE handles[] = { control_main->receiving_thread ,control_main->heartbeat_thread ,control_main->passenger_receiver };
	WaitForMultipleObjects(handle_amount, handles, true, INFINITE);
	//------------------------------------------------------------------------------------	
}


void exit_and_send_sentiment(ControlMain* control_main) {
	//Exiting ----------------------------------------------------------------------------
	PassengerMessage passenger_message;
	passenger_message.type = PASSENGER_TYPE_PLANE_CRASHED;

	unordered_set<Passenger*> already_left_passengers;

	for (int i = 0; i < control_main->shared_control->max_plane_amount; ++i) {
		Plane* plane = &control_main->planes[i];
		if (plane->in_use) {
			CircularBuffer* buffer = control_main->get_plane_buffer(i);

			PlaneControlMessage message;
			message.type = TYPE_CONTROL_EXITING;
			buffer->set_next_element(message);

			// if Plane flying warn all passengers that it went down
			if (plane->is_flying) {
				const auto passengers = control_main->get_passengers_on_plane(plane->offset);

				for (auto passenger_id : *passengers) {
					auto passenger = control_main->get_passenger_by_id(passenger_id);
					if (control_main->send_message_to_passenger(passenger, passenger_message))
						already_left_passengers.insert(passenger);
				}
			}
		}
	}
	passenger_message.type = PASSENGER_TYPE_CONTROL_EXITING;
	// Tell Passengers to leave, if they haven't left yet
	for (auto p : control_main->all_passengers) {
		auto passenger = p.second;
		if (already_left_passengers.find(passenger) != already_left_passengers.end())
			continue;

		if (!control_main->send_message_to_passenger(passenger, passenger_message))
			tcout << _T("Error Telling passenger to leave, id -> ") << passenger->id << endl;
	}
}

void start_all_threads(ControlMain* control_main) {
	// Start all threads -----------------------------------------------------------------
	control_main->passenger_receiver = create_thread(passenger_pipe_receiver, control_main);
	control_main->receiving_thread = create_thread(receive_updates, control_main);
	control_main->heartbeat_thread = create_thread(heartbeat_checker, control_main);
	//------------------------------------------------------------------------------------
}

ControlMain* main_start() {
	tstringstream stream;
	//Get max planes amount from registry ------------------------------------
	int max_planes = get_max_planes_from_registry();
	//Create Semaphore that control max planes at a moment
	HANDLE planes_semaphore = CreateSemaphoreW(nullptr, max_planes, max_planes, SEMAPHORE_MAX_PLANES);
	if (planes_semaphore == nullptr) {
		stream << _T("Semaphore already exists -> ") << GetLastError() << endl;
		throw new StartException(&stream);
	}
	// -----------------------------------------------------------------------

	//Create Pipe to receive new passangers ----------------------------------
	const HANDLE handle_control_named_pipe = CreateNamedPipe(CONTROL_PIPE_MAIN, PIPE_ACCESS_INBOUND,
															 PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES,
															 0, sizeof(PassengerMessage), 1000, nullptr);
	if (handle_control_named_pipe == INVALID_HANDLE_VALUE) {
		stream << _T("Error creating named pipe -> ") << GetLastError() << endl;
		CloseHandle(planes_semaphore);
		throw new StartException(&stream);
	}
	// -----------------------------------------------------------------------

	//Create Shared Memory ---------------------------------------------------
	const DWORD shared_memory_size = sizeof(SharedControl) + sizeof(Plane) * max_planes; //Soma do espaço necessário a alocar

	HANDLE handle_mapped_file;
	void* shared_mem_pointer;

	try {
		shared_mem_pointer = allocate_shared_memory(handle_mapped_file, shared_memory_size);
	} catch (StartException* e) {
		CloseHandle(planes_semaphore);
		CloseHandle(handle_control_named_pipe);
		throw e;
	}
	//------------------------------------------------------------------------------------

	ControlMain* control_main;

	SharedControl* shared_control = (SharedControl*)shared_mem_pointer;
	Plane* planes_start = (Plane*)&shared_control[1];


	shared_control->max_plane_amount = max_planes;
	control_main = new ControlMain(shared_control, planes_start, handle_mapped_file, handle_control_named_pipe);

	memset(control_main->shared_control->map, MAP_EMPTY, sizeof(control_main->shared_control->map));

	return control_main;
}

HANDLE lock_process() {
	tstringstream stream;
	//Check if is already running --------------------------------------------
	HANDLE process_lock_mutex = CreateMutexW(0, FALSE, _T("Airport_Control"));
	// Tries to create a mutex with the specified name
	if (process_lock_mutex == nullptr) {
		stream << _T("Process lock mutex error creating : ") << GetLastError() << endl;
		throw new StartException(&stream);
	}
	// If the application is already running it cant create another mutex with the same name
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		stream << _T("Process already running\n");
		throw new StartException(&stream);
	}
	return process_lock_mutex;
}

void* allocate_shared_memory(HANDLE& mapped_file, DWORD size) {
	tstringstream stream;
	mapped_file = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, MAPPED_MEMORY_IDENTIFIER);

	if (mapped_file == nullptr) {
		stream << _T("Could not create file mapping object -> ") << GetLastError();
		throw new StartException(&stream);
	}
	void* shared_mem_pointer = MapViewOfFile(mapped_file, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if (shared_mem_pointer == nullptr) {
		stream << _T("Could not map view of file ->") << GetLastError() << endl;
		CloseHandle(mapped_file);
		throw new StartException(&stream);
	}

	return shared_mem_pointer;
}