#include "ControlFunctions.h"
#include "SharedStructContents.h"

#include <iostream>


#ifdef UNICODE
#define tcout std::wcout
#define tcin std::wcin
#define tstringstream std::wstringstream
#else
#define tcout std::cout
#define tcin std::cin
#define tstringstream std::stringstream
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
			} else {
				CircularBuffer buffer(&plane->buffer, plane_offset);
				message.type = TYPE_PLANE_NOT_ALLOWED;
				buffer.set_next_element(message);

				//TODO mutex maybe
				plane->in_use = false;
			}
			break;
		}
		case TYPE_NEXT_DESTINY: {// maybe there is no need for this

			TSTRING airport_name = message.data.airport_name;
			Plane* new_plane = &control->planes[plane_offset];

			CircularBuffer* buffer = control->get_plane_buffer(plane_offset);

			Airport* airport = control->get_airport(airport_name);
			if (airport != nullptr) {
				message.type = TYPE_PLANE_OK_DESTINY;
				message.data.position = airport->position;
			} else {
				message.type = TYPE_PLANE_BAD_DESTINY;
			}
			buffer->set_next_element(message);
			break;
		}
		case TYPE_START_TRIP: {
			break;
		}
		case TYPE_PLANE_MOVED: {
			Position& position = message.data.position;
			tcout << _T("Plane offset -> ") << plane->offset << _T(", moved to : ") << position.x << _T(",") << position.y << endl;
			break;
		}
		case TYPE_TO_BOARD: {
			break;
		}
		case TYPE_PLANE_LEAVES: {
			tcout << _T("Plane offset -> ") << plane->offset << _T(", left") << endl;
			break;
		}
		case TYPE_PLANE_CRASHES: {
			break;
		}case TYPE_FINISHED_TRIP: {
			tcout << _T("Plane offset -> ") << plane->offset << _T(", arrived at : ") << plane->destiny << _T(", from : ") << plane->origin << endl;
			break;
		}default:
			tcout << _T("Invalid type received from plane : ") << message.plane_offset << endl;
		}
	}
	return 0;
}

void exit_everything(ControlMain* control_main) {

	tcout << _T("Exiting\n-----------------------------");

	for (int i = 0; i < control_main->shared_control->max_plane_amount; ++i) {
		Plane* current_plane = &control_main->planes[i];
		if (current_plane->in_use) {

			TCHAR buf[8];
			_stprintf_s(buf, 8, _T("%d"), i);
			CircularBuffer buffer(&current_plane->buffer, i);

			PlaneControlMessage message;
			message.type = TYPE_CONTROL_EXITING;
			buffer.set_next_element(message);
		}
	}

	CloseHandle(control_main->receiving_thread);
	UnmapViewOfFile(control_main->shared_control);
	CloseHandle(control_main->handle_mapped_file);

	delete(control_main);

	exit(0);
}