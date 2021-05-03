#pragma once
#include "PlaneMain.h"

#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <vector>
#include <string>
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

		} else if (command == _T("fly")) {

			//int amen = move(plain_main->this_plane->position.x, 2, 3, 4, int* asd, int* asd2);
			// put result[1] on 
		} else if (command == _T("exit")) {
			plane_main->exit = true;
			break;
		} else {
			tcout << _T("----- Comands available ----- \n destiny <name>\n board\n fly\n exit") << endl;
		}
	}
}
