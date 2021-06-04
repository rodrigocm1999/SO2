#include "UIDrawFunctions.h"


using namespace std;


DWORD WINAPI draw_map(HBITMAP h_bitmap, HDC dc, ControlMain* control) {

	for (auto airport : control->airports) {
		draw_airport(h_bitmap, dc, airport.second->position);
	}

	for (int i = 0; i < control->shared_control->max_plane_amount; i++) {
		auto plane = control->get_plane(i);
		if (plane->in_use && plane->is_flying) {
			draw_plane(h_bitmap, dc, plane->position);
		}
	}
	return 0;
}

void draw_plane(HBITMAP hbitmap, HDC dc, const Position& pos) {
	//TODO
}

void draw_airport(HBITMAP h_bitmap, HDC dc, const Position& pos) {
	//TODO
}

TSTRING print_airports(ControlMain* control) {
	tstringstream stream;

	for (auto pair : control->airports) {
		Airport* airport = pair.second;
		stream << _T("Airport -> name : ") << airport->name << _T("\tposition : ") << airport->position.x << _T(", ") << airport->position.y << END_LINE;

		for (Plane* plane : airport->planes) {
			auto list = control->get_passengers_on_plane(plane->offset);
			stream << _T("\r\tPlane -> offset : ") << plane->offset <<
				_T(" , max passangers : ") << plane->max_passengers <<
				_T(" , velocity : ") << plane->velocity <<
				_T(" , passagers : ") << (list == nullptr ? 0 : list->size()) << END_LINE;
		}


	}

	return stream.str();
}

TSTRING print_planes(ControlMain* control) {
	tstringstream stream;
	for (int i = 0; i < control->shared_control->max_plane_amount; i++) {
		Plane* plane = &control->planes[i];

		const auto list = control->get_passengers_on_plane(plane->offset);

		if (plane->in_use) {
			//stream << _T("\t");
			stream << _T("\n\tPlane -> offset : ") << plane->offset <<
				_T(" , max passangers : ") << plane->max_passengers <<
				_T(" , velocity : ") << plane->velocity <<
				_T(" , passagers : ") << (list == nullptr ? 0 : list->size());

			if (plane->is_flying) {
				stream << _T(", the plane is flying from '") << control->get_airport(plane->origin_airport_id)->name <<
					_T("', to '") << control->get_airport(plane->destiny_airport_id)->name <<
					_T("', current position : ") << plane->position.x << _T(",") << plane->position.y << END_LINE;

			} else {
				stream << _T(", airport: ") << control->get_airport(plane->origin_airport_id)->name << END_LINE;
			}
		}
	}
	return stream.str();
}

TSTRING print_passengers(ControlMain* control) {
	tstringstream stream;
	for (auto passenger : control->all_passengers) {
		stream << _T("id: ") << passenger.second->id
			<< _T(", name: ") << passenger.second->name
			<< _T(", origin: ") << passenger.second->origin->name
			<< _T(", destiny:") << passenger.second->destiny->name << END_LINE;
	}

	return stream.str();
}