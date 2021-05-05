#include "ControlFunctions.h"
#include "SharedStructContents.h"

#include <iostream>


#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

using namespace std;

DWORD WINAPI receive_updates(LPVOID param) {
	ControlMain* control = (ControlMain*)param;

	while (!control->exit) {
		PlaneControlMessage message = control->receiving_buffer->get_next_element();
		const int plane_offset = message.plane_offset;
		Plane* plane = &control->planes[plane_offset];

		switch (message.type) {
		case TYPE_NEW_PLANE: {
			TSTRING airport_name = message.data.airport_name;

			tcout << _T("New Plane -> ")
				<< _T("offset : ") << plane_offset
				<< _T(", airport : ") << airport_name
				<< _T(", max capacity : ") << plane->max_passengers
				<< _T(", velocity : ") << plane->velocity << endl;


			Airport* airport = control->get_airport(airport_name);
			if (airport != nullptr) {
				plane->position = airport->position;
				plane->origin_airport_id = airport->id;
				airport->add_plane(plane);
			} else {
				// buffer mutexes get created/opened and closed in this scope
				// but it doesn't matter because this plane is leaving
				CircularBuffer buffer(&plane->buffer, plane_offset);
				message.type = TYPE_PLANE_NOT_ALLOWED;
				buffer.set_next_element(message);

				plane->in_use = false;
			}
			break;
		}
		case TYPE_NEXT_DESTINY: {
			TSTRING airport_name = message.data.airport_name;

			CircularBuffer* buffer = control->get_plane_buffer(plane_offset);
			Airport* destiny = control->get_airport(airport_name);

			if (destiny != nullptr && destiny->id != plane->origin_airport_id) {
				message.type = TYPE_PLANE_OK_DESTINY;
				message.data.position = destiny->position;
				plane->destiny_airport_id = destiny->id;
				buffer->set_next_element(message);

				tcout << _T("Plane offset -> ") << plane_offset << _T(", changed destiny to : ") << airport_name << endl;

				//TODO later change the peoples maybe
			} else {
				message.type = TYPE_PLANE_BAD_DESTINY;
				buffer->set_next_element(message);
			}
			break;
		}
		case TYPE_START_TRIP: {
			control->plane_left_airport(plane_offset);
			tcout << _T("Plane offset -> ") << plane_offset << _T(", left the airport") << endl;
			break;
		}

		case TYPE_PLANE_MOVED: {
			Position& position = message.data.position;
			tcout << _T("Plane offset -> ") << plane->offset << _T(", moved to : ") << position.x << _T(",") << position.y << endl;
			break;
		}

		case TYPE_TO_BOARD: {
			tcout << _T("Plane offset -> ") << plane->offset << _T(", boarding") << endl;
			plane->flight_ready = false;
			//TODO actually move the people
			plane->flight_ready = true;
			message.type = TYPE_PLANE_FINISHED_BOARDING;
			CircularBuffer* buffer = control->get_plane_buffer(plane_offset);
			break;
		}

		case TYPE_PLANE_LEAVES: {
			tcout << _T("Plane offset -> ") << plane->offset << _T(", left") << endl;

			if (plane->origin_airport_id != NOT_DEFINED_AIRPORT) {
				control->plane_left_airport(plane_offset);
			}
			break;
		}

		case TYPE_PLANE_CRASHES: {
			tcout << _T("Plane offset -> ") << plane->offset << _T(", crashed") << endl;
			//TODO tell the people to shutdown
			break;
		}

		case TYPE_FINISHED_TRIP: {
			tcout << _T("Plane offset -> ") << plane->offset
				<< _T(", arrived at : ") << control->get_airport(plane->destiny_airport_id)->name
				<< _T(", from : ") << control->get_airport(plane->origin_airport_id)->name << endl;

			Airport* airport = control->get_airport(plane->destiny_airport_id);
			airport->add_plane(plane);

			plane->origin_airport_id = plane->destiny_airport_id;
			plane->destiny_airport_id = NOT_DEFINED_AIRPORT;

			//TODO tell the people to shtudown
			break;
		}default:
			tcout << _T("Invalid type received from planes : ") << message.plane_offset << endl;
		}
	}
	return 0;
}

void exit_everything(ControlMain* control_main) {

	tcout << _T("Exiting\n-----------------------------");

	for (int i = 0; i < control_main->shared_control->max_plane_amount; ++i) {
		Plane* current_plane = &control_main->planes[i];
		if (current_plane->in_use) {

			CircularBuffer* buffer = control_main->get_plane_buffer(i);

			PlaneControlMessage message;
			message.type = TYPE_CONTROL_EXITING;
			buffer->set_next_element(message);
		}
	}

	CloseHandle(control_main->receiving_thread);
	UnmapViewOfFile(control_main->shared_control);
	CloseHandle(control_main->handle_mapped_file);
	CloseHandle(control_main->heartbeat_thread);

	delete(control_main);

	exit(0);
}

DWORD WINAPI heartbeat_checker(LPVOID param) {

	ControlMain* control = (ControlMain*)param;

	while (!control->exit) {

		Sleep(HEARTBEAT_TIME_CONTROL);

		for (int i = 0; i < control->shared_control->max_plane_amount; ++i) {
			Plane* plane = control->get_plane(i);

			if (plane->in_use) {

				if (!plane->heartbeat) {
					plane->in_use = false;
					tcout << _T("Plane offset -> ") << i << _T(", disconnected") << endl;

					if (plane->is_flying) {

						PlaneControlMessage message;
						message.plane_offset = i;
						message.type = TYPE_PLANE_CRASHES;
						control->receiving_buffer->set_next_element(message);

						//TODO warn passengers if mid flight	
					}

				} else {
					plane->heartbeat = false;
				}
			}

		}
	}

	return 0;
}
