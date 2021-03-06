#include "TextInterface.h"

#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>

#include "ControlFunctions.h"
#include "Utils.h"

using namespace std;

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tcout wcout
#define tcin wcin
#else
#define tcout cout
#define tcin cin
#endif

void print_passenger(Passenger* passenger) {
	tcout << _T("id: ") << passenger->id
		<< _T(", name: ") << passenger->name
		<< _T(", origin: ") << passenger->origin->name
		<< _T(", destiny:") << passenger->destiny->name << endl;
}

void print_plane(Plane* plane, ControlMain* control) {
	auto list = control->get_passengers_on_plane(plane->offset);

	tcout << _T("\tPlane -> offset : ") << plane->offset <<
		_T(" , max passangers : ") << plane->max_passengers <<
		_T(" , velocity : ") << plane->velocity <<
		_T(" , passagers : ") << (list == nullptr ? 0 : list->size());

}

DWORD WINAPI enter_text_interface(LPVOID param) {
	ControlMain* control_main = (ControlMain*)param;

	bool accept_state = true;

	while (!control_main->exit) {

		tcout << _T("Menu -------------\n");
		TSTRING input;
		getline(tcin, input);
		tcout << endl;
		vector<TSTRING> input_parts = string_split(input, _T(" "));
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
						tcout << _T("Airport with the same name or closer than 10 units already exists") << endl;
					}
				}
			} else {
				tcout << _T("Invalid Syntax -> new_airport <name> <posX> <posY>\n");
			}
		} else if (command == _T("accept")) {
			HANDLE mutex = control_main->plane_entering_lock;

			if (accept_state) {
				DWORD result = WaitForSingleObject(mutex, 2000);
				if (result != WAIT_OBJECT_0) {
					tcout << _T("Something went wrong locking plane entering");
					continue;
				}
				tcout << _T("Planes can now not enter") << endl;
			} else {
				ReleaseMutex(mutex);
				tcout << _T("Planes can enter again") << endl;
			}
			accept_state = !accept_state;

		} else if (command == _T("list")) {
			if (input_parts.size() == 2) {
				TSTRING type = input_parts[1];

				if (type == _T("airports")) {

					for (auto pair : control_main->airports) {
						Airport* airport = pair.second;

						tcout << _T("Airport -> name : ") << airport->name << _T("\tposition : ") << airport->position.x << _T(", ") << airport->position.y << endl;

						for (Plane* plane : airport->planes) {
							tcout << _T("\t");
							print_plane(plane, control_main);
						}
						tcout << endl;

						for (auto destiny_pair : airport->passengers) {
							TSTRING airport_name = control_main->get_airport(destiny_pair.first)->name;
							tcout << _T("\tDestiny : ") << airport_name << endl;
							for (auto passenger : *destiny_pair.second) {
								tcout << "\t\t";
								print_passenger(control_main->get_passenger_by_id(passenger));
							}
						}
					}
				} else if (type == _T("planes")) {
					for (int i = 0; i < control_main->shared_control->max_plane_amount; i++) {
						Plane* plane = &control_main->planes[i];

						const auto list = control_main->get_passengers_on_plane(plane->offset);

						if (plane->in_use) {
							print_plane(plane, control_main);

							if (plane->is_flying) {
								tcout << _T(", the plane is flying from '") << control_main->get_airport(plane->origin_airport_id)->name <<
									_T("', to '") << control_main->get_airport(plane->destiny_airport_id)->name <<
									_T("', current position : ") << plane->position.x << _T(",") << plane->position.y << endl;

								for (auto passenger : *list) {
									tcout << "\t\t";
									print_passenger(control_main->get_passenger_by_id(passenger));
								}
							} else {
								tcout << _T(", airport: ") << control_main->get_airport(plane->origin_airport_id)->name << endl;
							}
						}
					}
				} else if (type == _T("passengers")) {
					for (auto passenger : control_main->all_passengers)
						print_passenger(passenger.second);
				}

			} else {
				tcout << _T("Invalid Syntax -> list (airports|planes|passengers)\n");
			}
		} else if (command == _T("exit")) {
			exit_everything(control_main);
		} else {
			tcout << _T("Invalid Syntax -> new_airport|accept|list\n Introduced -> ") << input << endl;
		}
	}

	return 0;
}
