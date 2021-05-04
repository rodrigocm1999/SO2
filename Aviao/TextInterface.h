#pragma once
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include "Utils.h"
#include "PlaneMain.h"
#include "SO2_TP_DLL_2021.h"

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

#define PLANE_ARRIVED 0
#define PLANE_MOVED 1

// TODO move the function to a proper place
void exit_everything(PlaneMain* plane_main);

PlaneControlMessage ready_message(PlaneMain* plane_main, int type) {
	PlaneControlMessage message;
	message.plane_offset = plane_main->this_plane->offset;
	message.type = type;
	return message;
}

void enter_text_interface_plane(PlaneMain* plane_main) {

	while (!plane_main->exit) {

		tcout << _T("> ");
		TSTRING input;
		getline(tcin, input);
		vector<TSTRING> input_parts = stringSplit(input, _T(" "));
		auto command = input_parts[0];

		if (command == _T("destiny")) {
			if (input_parts.size() == 2) {
				TSTRING destiny = input_parts[1];
				int destiny_bytes = sizeof(TCHAR) * (destiny.size() + 1);

				PlaneControlMessage message = ready_message(plane_main, TYPE_NEXT_DESTINY);
				memcpy(message.data.airport_name, destiny.c_str(), destiny_bytes);
				plane_main->control_buffer->set_next_element(message);

			} else
				tcout << _T("Invalid Syntax -> destiny <name>") << endl;
		} else if (command == _T("board")) {
			if (plane_main->flight_ready) {

			} else {
				tcout << _T("No defined destiny\n");
			}
		} else if (command == _T("fly")) {
			if (plane_main->flight_ready) {
				PlaneControlMessage message = ready_message(plane_main, TYPE_START_TRIP);
				plane_main->control_buffer->set_next_element(message);

				Plane* plane = plane_main->this_plane;
				int result = -1;
				DWORD sleep_time = 1000 / plane->velocity;

				while (true) {

					for (int i = 0; i < plane->velocity; ++i) {

						Sleep(sleep_time);

						Position next_pos;
						next_pos = plane->position;

						//TODO remove this
						// -------------------------------------------------------------------
						Position& cur_pos = plane->position;
						Position& des_pos = plane_main->destiny_position;

						if (cur_pos.x < des_pos.x) next_pos.x = cur_pos.x + 1;
						else if (cur_pos.x > des_pos.x) next_pos.x = cur_pos.x - 1;
						if (cur_pos.y < des_pos.y) next_pos.y = cur_pos.y + 1;
						else if (cur_pos.y > des_pos.y) next_pos.y = cur_pos.y - 1;

						result = PLANE_MOVED;

						if (cur_pos.x == des_pos.x && cur_pos.y == des_pos.y)
							result = PLANE_ARRIVED;
						// -------------------------------------------------------------------

						/*result = move(plane->position.x, plane->position.y,
							plane_main->destiny_position.x, plane_main->destiny_position.y,
							&position.x, &position.y);*/


						if (result == PLANE_MOVED) {
							plane->position = next_pos;

						} else if (result == PLANE_ARRIVED) {
							plane->position = next_pos;
							break;
						} else {
							tcout << _T("What the heck happened. Error when flying. move() returned error\n");
							exit_everything(plane_main);
							exit(-1);
						}
					}

					if (result == PLANE_MOVED) {
						message = ready_message(plane_main, TYPE_PLANE_MOVED);
						message.data.position = plane->position;
						plane_main->control_buffer->set_next_element(message);
						tcout << _T("Plane moved to : ") << plane->position.x << _T(",") << plane->position.y << endl;
					} else if (result == PLANE_ARRIVED) {
						message = ready_message(plane_main, TYPE_FINISHED_TRIP);
						plane_main->control_buffer->set_next_element(message);
						tcout << _T("Plane arrived at destination") << endl;
						break;
					}
				}
			} else {
				tcout << _T("No defined destiny\n");
			}
		} else if (command == _T("exit")) {
			plane_main->exit = true;
			break;
		} else {
			tcout << _T("----- Comands available ----- \n destiny <name>\n board\n fly\n exit") << endl;
		}
	}
}
