#pragma once

#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include "Utils.h"
#include "ControlMain.h"

using namespace std;

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tcout wcout
#define tcin wcin
#else
#define tcout cout
#define tcin cin
#endif

void enter_text_interface(ControlMain* control_main, bool* exit) {
	while (!*exit) {
		tcout << _T("Menu -------------\n");
		TSTRING input;
		getline(tcin, input);
		vector<TSTRING> input_parts = stringSplit(input, _T(" "));
		auto command = input_parts[0];

		if (command == _T("new_airport")) {
			if (input_parts.size() == 4) {
				const TCHAR* name = input_parts[1].c_str();
				int pos_x = _ttoi(input_parts[2].c_str());
				int pos_y = _ttoi(input_parts[3].c_str());

				if (pos_x < 0 || pos_x >= MAP_SIZE || pos_y < 0 || pos_y >= MAP_SIZE) {
					tcout << _T("Invalid position entered\n");
				} else {
					if (control_main->add_airport(name, pos_x, pos_y)) {
						tcout << _T("Added airport") << endl;
					} else {
						tcout << _T("Airport with the same name or in the same position already exists") << endl;
					}
				}
			} else {
				tcout << _T("Invalid Syntax -> new_airport <name> <posX> <posY>\n");
			}
		} else if (command == _T("accept")) { // WTF -> Suspender / ativar a aceitação de novos aviões por parte dos utilizadores.

		} else if (command == _T("list")) {
			if (input_parts.size() == 2) {
				TSTRING type = input_parts[1];

				if (type == _T("airports")) {
					for (pair<const int, Airport*> pair : control_main->airports) {
						Airport* airport = pair.second;

						tcout << _T("Airport -> name : ") << airport->name << _T("\tposition : ") << airport->position.x << _T(", ") << airport->position.y << endl;

						for (Plane* plane : airport->planes) {

							tcout << _T("\tPlane -> offset : ") << plane->offset << _T(", max passangers : ") << plane->max_passengers << _T(", passagers : ") << endl;
							//TODO put passengers
						}
					}
				} else if (type == _T("planes")) {

				} else if (type == _T("passengers")) {

				}

			} else {
				tcout << _T("Invalid Syntax -> list (airports|planes|passengers)\n");
			}
		} else if (command == _T("exit")) {
			*exit = true;
			control_main->exit = true;
		} else {
			tcout << _T("Invalid Syntax -> new_airport|accept|list\n Introduced -> ") << input << endl;
		}
	}
}