#include "PlaneFunctions.h"

#include <iostream>

#include "SO2_TP_DLL_2021.h"
#include "SharedStructContents.h"
#include "TextInterface.h"


using namespace std;

#define PLANE_ARRIVED 0
#define PLANE_MOVED 1

DWORD WINAPI receive_updates(LPVOID param) {
	PlaneMain* plane_main = (PlaneMain*)param;

	while (true) {
		const PlaneControlMessage message = plane_main->receiving_buffer->get_next_element();

		bool to_exit = false;

		switch (message.type) {
			case TYPE_CONTROL_EXITING:
				tcout << _T("Control Exiting") << endl;
				plane_main->exit = true;
				to_exit = true;
				break;

			case TYPE_PLANE_NOT_ALLOWED:
				tcout << _T("Indicated airport doesn't exist") << endl;
				plane_main->exit = true;
				to_exit = true;
				break;

			case TYPE_PLANE_OK_DESTINY:
				plane_main->destiny_position = message.data.position;
				plane_main->this_plane->flight_ready = true;
				tcout << _T("You are now allowed to fly to your destiny") << endl;
				break;

			case TYPE_PLANE_BAD_DESTINY:
				plane_main->this_plane->flight_ready = false;
				plane_main->this_plane->destiny_airport_id = NOT_DEFINED_AIRPORT;
				tcout << _T("Invalid destiny") << endl;
				break;

			case TYPE_ERROR:
				tcout << _T("Error from control -> ") << message.data.error_message << endl;
				break;

			default:
				tcout << _T("Invalid type received from control :") << message.type << endl;
		}

		if (to_exit) {
			exit_everything(plane_main);
		}
	}
}

DWORD WINAPI fly_plane(LPVOID param) {
	PlaneMain* plane_main = (PlaneMain*)param;

	Plane* plane = plane_main->this_plane;
	int result = -1;
	const DWORD sleep_time = 1000 / plane->velocity;

	while (true) {

		for (int i = 0; i < plane->velocity; ++i) {

			Sleep(sleep_time);

			Position next_pos = plane->position;

			result = move(plane->position.x, plane->position.y,
						  plane_main->destiny_position.x, plane_main->destiny_position.y,
						  &next_pos.x, &next_pos.y);

			if (result == PLANE_MOVED) {
				plane->position = next_pos;

			} else if (result == PLANE_ARRIVED) {
				plane->position = next_pos;
				break;
			} else {
				tcout << _T("What the heck happened. Error when flying. move() returned error") << endl;
				exit_everything(plane_main);
			}
		}

		if (result == PLANE_MOVED) {
			auto message = ready_message(plane_main, TYPE_PLANE_MOVED);
			message.data.position = plane->position;
			plane_main->control_buffer->set_next_element(message);
			tcout << _T("Plane moved to : ") << plane->position.x << _T(",") << plane->position.y << endl;
		} else if (result == PLANE_ARRIVED) {
			plane->is_flying = false;
			plane->flight_ready = false;
			plane->already_boarded = false;

			auto message = ready_message(plane_main, TYPE_FINISHED_TRIP);
			plane_main->control_buffer->set_next_element(message);
			tcout << _T("Plane arrived at destination") << endl;
			break;
		}
	}

	return 0;
}

DWORD WINAPI heartbeat(LPVOID param) {

	PlaneMain* plane = (PlaneMain*)param;

	while (!plane->exit) {

		Sleep(HEARTBEAT_TIME_PLANE);

		plane->this_plane->heartbeat = true;
	}
	return -1;
}

void exit_everything(PlaneMain* plane_main) {
	plane_main->exit = true;
	plane_main->this_plane->in_use = false;

	PlaneControlMessage message;
	if (plane_main->this_plane->is_flying)
		message = ready_message(plane_main, TYPE_PLANE_CRASHES);
	else
		message = ready_message(plane_main, TYPE_PLANE_LEAVES);
	plane_main->control_buffer->set_next_element(message);

	UnmapViewOfFile(plane_main->shared_control);
	CloseHandle(plane_main->handle_mapped_file);
	CloseHandle(plane_main->receiving_thread);
	ReleaseSemaphore(plane_main->semaphore_plane_counter, 1, nullptr);
	CloseHandle(plane_main->heartbeat_thread);

	delete(plane_main);

	exit(0);
}
