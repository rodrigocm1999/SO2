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
				memcpy(plane_main->this_plane->destiny, destiny.c_str(), destiny_bytes);

				PlaneControlMessage message;
				message.plane_offset = plane_main->this_plane->offset;
				message.type = TYPE_NEXT_DESTINY;
				memcpy(message.data.airport_name, plane_main->this_plane->destiny, destiny_bytes);
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
				Plane* plane = plane_main->this_plane;
				int result = 0;
				DWORD sleep_time = 1000 / plane->velocity;

				for (int i = 0; i < plane->velocity; ++i) {

					Sleep(sleep_time);

					Position position;

					result = move(plane->position.x, plane->position.y,
						plane_main->destiny_position.x, plane_main->destiny_position.y,
						&position.x, &position.y);



					if (result == PLANE_MOVED) {
						plane->position = position;

					} else if (result == PLANE_ARRIVED) {
						plane->position = position;
						break;
					} else {
						tcout << _T("What the heck happened. Error when flying. move() returned error\n");
						break;
					}

				}

				PlaneControlMessage message;
				message.plane_offset = plane_main->this_plane->offset;

				if (result == PLANE_MOVED) {
					message.type = TYPE_PLANE_MOVED;
					message.data.position = plane->position;
				} else if (result == PLANE_ARRIVED) {
					message.type = TYPE_FINISHED_TRIP;
				}

				plane_main->control_buffer->set_next_element(message);
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
