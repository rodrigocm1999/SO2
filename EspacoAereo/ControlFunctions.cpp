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

		switch (message.type) {
		case TYPE_NEW_PLANE: {
			int offset = message.plane_offset;
			TSTRING airport_name = message.data.airport_name;
			Plane* new_plane = &control->planes[offset];

			tcout << _T("New Plane -> ")
				<< _T("offset : ") << offset
				<< _T(", airport : ") << airport_name
				<< _T(", max capacity : ") << new_plane->max_passengers
				<< _T(", velocity : ") << new_plane->velocity << endl;


			Airport* airport = control->get_airport(airport_name);
			if (airport != nullptr) {
				new_plane->position = airport->position;
			} else {
				CircularBuffer buffer(&new_plane->buffer, offset);
				message.type = TYPE_PLANE_NOT_ALLOWED;
				buffer.set_next_element(message);

				//TODO mutex maybe
				new_plane->in_use = false;
			}
			break;
		}
		case TYPE_NEXT_DESTINY: {// maybe there is no need for this

			int offset = message.plane_offset;
			TSTRING airport_name = message.data.airport_name;
			Plane* new_plane = &control->planes[offset];

			CircularBuffer* buffer = control->get_plane_buffer(offset);

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
		case TYPE_TO_BOARD: {
			break;
		}
		case TYPE_PLANE_LEAVES: {
			tcout << _T("He gone\n"); //TODO actually do the shits
			break;
		}
		case TYPE_PLANE_CRASHES: {
			break;
		}case TYPE_FINISHED_TRIP: {
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