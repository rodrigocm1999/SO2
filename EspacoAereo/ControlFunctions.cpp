#include "ControlFunctions.h"
#include "SharedStructContents.h"
#include "SharedPassagStruct.h"

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
			case TYPE_NEW_PLANE:
			{
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
					auto buffer = control->get_plane_buffer(plane_offset);
					message.type = TYPE_PLANE_NOT_ALLOWED;
					buffer->set_next_element(message);

					plane->in_use = false;
				}
				break;
			}
			case TYPE_NEXT_DESTINY:
			{
				TSTRING airport_name = message.data.airport_name;
				CircularBuffer* buffer = control->get_plane_buffer(plane_offset);

				if (plane->already_boarded) {
					message.type = TYPE_ERROR;
					memcpy(message.data.error_message, _T("Plane not ready to fly"), BUFFER_SIZE * sizeof(TCHAR));
					buffer->set_next_element(message);
				}

				Airport* destiny = control->get_airport(airport_name);

				if (destiny != nullptr && destiny->id != plane->origin_airport_id) {
					message.type = TYPE_PLANE_OK_DESTINY;
					message.data.position = destiny->position;
					plane->destiny_airport_id = destiny->id;
					buffer->set_next_element(message);

					tcout << _T("Plane offset -> ") << plane_offset
						<< _T(", changed destiny to : ") << airport_name << endl;

				} else {
					message.type = TYPE_PLANE_BAD_DESTINY;
					buffer->set_next_element(message);
				}
				break;
			}
			case TYPE_START_TRIP:
			{
				control->plane_left_airport(plane_offset);
				tcout << _T("Plane offset -> ") << plane_offset << _T(", left the airport") << endl;

				PassengerMessage message;
				message.type = PASSENGER_TYPE_STARTED_FLYING;
				for (auto passenger : control->get_passengers_object_on_plane(plane_offset)) {
					control->send_message_to_passenger(passenger, message);
				}
				break;
			}

			case TYPE_PLANE_MOVED:
			{
				Position& position = message.data.position;
				tcout << _T("Plane offset -> ") << plane->offset << _T(", moved to : ") << position.x << _T(",") <<
					position.y << endl;

				PassengerMessage message;
				message.type = PASSENGER_TYPE_MOVED;
				message.data.pos = position;
				for (auto passenger : control->get_passengers_object_on_plane(plane_offset)) {
					control->send_message_to_passenger(passenger, message);
				}

				break;
			}

			case TYPE_TO_BOARD:
			{
				tcout << _T("Plane offset -> ") << plane->offset << _T(", boarding") << endl;

				control->board_people(plane->offset, plane->origin_airport_id, plane->destiny_airport_id);
				plane->flight_ready = true;
				plane->already_boarded = true;

				PassengerMessage message;
				message.type = PASSENGER_TYPE_BOARDED;
				for (auto passenger : control->get_passengers_object_on_plane(plane_offset)) {
					tcout << _T("pasenger id -> ") << passenger->id << endl;
					control->send_message_to_passenger(passenger, message);
				}

				break;
			}

			case TYPE_PLANE_LEAVES:
			{
				tcout << _T("Plane offset -> ") << plane->offset << _T(", left") << endl;

				if (plane->origin_airport_id != NOT_DEFINED_AIRPORT) {
					control->plane_left_airport(plane_offset);
				}
				break;
			}

			case TYPE_PLANE_CRASHES:
			{
				tcout << _T("Plane offset -> ") << plane->offset << _T(", crashed") << endl;

				control->ended_trip(plane_offset, PASSENGER_TYPE_PLANE_CRASHED);

				break;
			}

			case TYPE_FINISHED_TRIP:
			{
				tcout << _T("Plane offset -> ") << plane->offset
					<< _T(", arrived at : ") << control->get_airport(plane->destiny_airport_id)->name
					<< _T(", from : ") << control->get_airport(plane->origin_airport_id)->name << endl;

				Airport* airport = control->get_airport(plane->destiny_airport_id);
				airport->add_plane(plane);

				plane->origin_airport_id = plane->destiny_airport_id;
				plane->destiny_airport_id = NOT_DEFINED_AIRPORT;

				control->ended_trip(plane_offset, PASSENGER_TYPE_PLANE_ARRIVED);

				break;
			}
			case TYPE_CONTROL_EXITING:
			{
				// no need to do anything, just here to not go to the default
				break;
			}
			default:
				tcout << _T("Invalid type received from planes : ") << message.plane_offset << endl;
		}
	}
	return 0;
}

void exit_everything(ControlMain* control_main) {
	control_main->exit = true;

	SetEvent(control_main->shutdown_event);

	PlaneControlMessage message;
	message.plane_offset = 0;
	message.type = TYPE_CONTROL_EXITING;
	control_main->receiving_buffer->set_next_element(message);

	CancelSynchronousIo(control_main->passenger_receiver);
}

DWORD WINAPI heartbeat_checker(LPVOID param) {
	ControlMain* control = (ControlMain*)param;

	while (!control->exit) {
		const DWORD result = WaitForSingleObject(control->shutdown_event, HEARTBEAT_TIME_CONTROL);
		if (result != WAIT_TIMEOUT)
			break;


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
					}
				} else {
					plane->heartbeat = false;
				}
			}
		}
	}

	return 0;
}


DWORD WINAPI passenger_pipe_receiver(LPVOID param) {
	auto control_main = (ControlMain*)param;
	const HANDLE control_pipe = control_main->handle_control_named_pipe;
	DWORD bytesRead;
	PassengerMessage message;

	while (!control_main->exit) {

		bool connected = ConnectNamedPipe(control_pipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (!connected) {
			if (GetLastError() != ERROR_OPERATION_ABORTED) {
				tcout << _T("Error connecting to pipe reader -> ") << GetLastError() << endl;
				exit_everything(control_main);
			}
			continue;
		}

		const bool result = ReadFile(control_pipe, &message, sizeof(PassengerMessage), &bytesRead, nullptr);
		if (result == false) {
			tcout << _T("Shouldnt happen ReadFile(control_pipe) -> ") << GetLastError() << endl;
			if (GetLastError() == 109) {
				tcout << _T("Pipe Error open") << endl;
			}
			DisconnectNamedPipe(control_pipe);
			continue;
		}

		switch (message.type) {
			case PASSENGER_TYPE_NEW_PASSENGER: {
				Airport* origin = control_main->get_airport(message.data.new_passenger.origin);
				Airport* destiny = control_main->get_airport(message.data.new_passenger.destiny);

				Passenger* passenger = new Passenger(message.id, origin, destiny, message.data.new_passenger.name);

				if (origin == nullptr || destiny == nullptr) {

					if (!control_main->send_message_to_passenger(passenger, PASSENGER_TYPE_BAD_AIRPORTS))
						tcout << _T("Error sending message to client") << endl;
					delete passenger;
					break;
				}


				if (control_main->send_message_to_passenger(passenger, PASSENGER_TYPE_GOOD_AIRPORTS)) {
					tcout << _T("Received new passenger -> id : ") << message.id << _T("\tname : ") << message.data.new_passenger.name
						<< _T("\torigin : ") << message.data.new_passenger.origin << _T("\tDestiny : ") << message.data.new_passenger.destiny << endl;
				} else {
					tcout << _T("Error sending message to client") << endl;
				}

				control_main->add_passenger(passenger);
				break;
			}
			case PASSENGER_TYPE_GAVE_UP:
				control_main->remove_passenger(control_main->get_passenger_by_id(message.id));
				break;
		}

		DisconnectNamedPipe(control_pipe);
	}
	return 0;
}
