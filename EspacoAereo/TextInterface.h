#pragma once

#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <vector>
#include <string>
#include "Utils.h"
#include "ControlStructs.h"

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


void enter_text_interface(ControlMain* control_main, bool* exit) {
	while (!*exit) {
		tcout << _T("Menu -------------\n");
		TSTRING input;
		tcin >> input;
		vector<TSTRING> input_parts = stringSplit(input, _T(" "));
		auto command = input_parts[0];

		if (command == _T("new_airport")) {
			if (input_parts.size() == 4) {
				const TCHAR* name = input_parts[1].c_str();
				int pos_x = _ttoi(input_parts[2].c_str());
				int pos_y = _ttoi(input_parts[3].c_str());

				auto airport = new Airport(name, pos_x, pos_y);
				control_main->add_airport(airport);
			} else {
				tcout << _T("Invalid Syntax -> new_airport <name> <posX> <posY>\n");
			}
		} else if (command == _T("accept")) { // WTF -> Suspender / ativar a aceitação de novos aviões por parte dos utilizadores.

		} else if (command == _T("list")) {
			if (input_parts.size() == 2) {
				TSTRING type = input_parts[1];

				if (type == _T("airports")) {
					for (Airport* airport : control_main->airports) {
						tcout << _T("\tname : ") << airport->name << _T("\n\tposition : ") << airport->position.x << _T(", ") << airport->position.y << endl;
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
			tcout << _T("Invalid Syntax -> new_airport|accept|list\n");
		}
	}
}