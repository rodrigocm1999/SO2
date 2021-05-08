#include "TextInterface.h"

#include "Utils.h"


void enter_text_interface_plane(PlaneMain* plane_main) {
	tcout << _T("Ready to receive commands") << endl;

	while (!plane_main->exit) {

		tcout << _T("> ");
		TSTRING input;
		getline(tcin, input);
		vector<TSTRING> input_parts = string_split(input, _T(" "));
		auto command = input_parts[0];

		if (command == _T("exit")) {
			exit_command(plane_main);
			break;
		}
		if (!plane_main->this_plane->is_flying) {
			if (command == _T("destiny")) {
				destiny_command(plane_main, input_parts);

			} else if (command == _T("board")) {
				board_command(plane_main);

			} else if (command == _T("fly")) {
				fly_command(plane_main);

			} else {
				tcout << _T("----- Comands available ----- \n destiny <name>\n board\n fly\n exit") << endl;
			}
		} else {
			tcout << _T("Can only exit while flying") << endl;
		}
	}
}


void fly_command(PlaneMain* plane_main) {
	if (!plane_main->this_plane->flight_ready) {
		tcout << _T("No defined destiny\n");
		return;
	}

	PlaneControlMessage message = ready_message(plane_main, TYPE_START_TRIP);
	plane_main->control_buffer->set_next_element(message);

	plane_main->this_plane->is_flying = true;

	create_thread(fly_plane, plane_main);
}

void board_command(PlaneMain* plane_main) {
	if (!plane_main->this_plane->flight_ready) {
		tcout << _T("No defined destiny") << endl;
		return;
	}

	plane_main->this_plane->flight_ready = false;

	auto message = ready_message(plane_main, TYPE_TO_BOARD);
	plane_main->control_buffer->set_next_element(message);
	tcout << _T("Boarding") << endl;
}

void destiny_command(PlaneMain* plane_main, vector<TSTRING> input_parts) {
	if (input_parts.size() != 2) {
		tcout << _T("Invalid Syntax -> destiny <name>") << endl;
		return;
	}

	TSTRING destiny = input_parts[1];
	const int destiny_bytes = sizeof(TCHAR) * (destiny.size() + 1);

	auto message = ready_message(plane_main, TYPE_NEXT_DESTINY);
	memcpy(message.data.airport_name, destiny.c_str(), destiny_bytes);
	plane_main->control_buffer->set_next_element(message);
}

void exit_command(PlaneMain* plane_main) {
	plane_main->exit = true;
}

PlaneControlMessage ready_message(PlaneMain* plane_main, int type) {
	PlaneControlMessage message;
	message.plane_offset = plane_main->this_plane->offset;
	message.type = type;
	return message;
}
