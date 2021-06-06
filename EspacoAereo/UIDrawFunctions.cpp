#include "UIDrawFunctions.h"

#include "resource1.h"


using namespace std;


DWORD WINAPI draw_map(HDC bitmap_dc, HANDLES_N_STUFF* handles) {

	ControlMain* control = handles->control;

	HDC aux_dc = CreateCompatibleDC(bitmap_dc);

	HBITMAP airport_icon = handles->airport_icon;

	for (auto airport : control->airports) {
		draw_img(airport_icon, bitmap_dc, aux_dc, airport.second->position);
	}

	HBITMAP plane_icon = handles->plane_icon;

	for (int i = 0; i < control->shared_control->max_plane_amount; i++) {
		auto plane = control->get_plane(i);
		if (plane->in_use && plane->is_flying) {
			draw_img(plane_icon, bitmap_dc, aux_dc, plane->position);
		}
	}

	DeleteDC(aux_dc);

	return 0;
}

DWORD WINAPI draw_map_thread(LPVOID param) {
	auto stuff = (ToDrawThread*)param;
	auto control = stuff->handles->control;

	while (!control->exit) {

		DWORD result = WaitForSingleObject(control->shutdown_event, REFRESH_WAIT);
		if (result != WAIT_OBJECT_0 && result != WAIT_TIMEOUT) {
			break;
		}
		//TODO make sure planes show up
		draw_map(stuff->bitmap_dc, stuff->handles);
		InvalidateRect(stuff->handles->map_area, nullptr, false);
	}
	return 0;
}

void draw_img(HBITMAP h_bitmap, HDC bitmap_dc, HDC aux_dc, const Position& pos) {
	SelectObject(aux_dc, h_bitmap);
	BitBlt(bitmap_dc, pos.x - (ICON_SIZE / 2), pos.y - -(ICON_SIZE / 2), ICON_SIZE, ICON_SIZE, aux_dc, 0, 0, SRCCOPY);
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