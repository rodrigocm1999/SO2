#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <sstream>

#include "ControlFunctions.h"
#include "MainBreakdown.h"
#include "resource.h" //1º passo
#include "StartException.h"
#include "UIDrawFunctions.h"
#include "Utils.h"

#define ADD_AIRPORT 1
#define LIST_AIRPORTS 2
#define LIST_PLANES 3
#define LIST_PASSANGERS 4
#define ACCEPT 5



#define END_LINE _T("\r\n")


#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tstringstream std::wstringstream
#else
#define tstringstream std::stringstream
#endif

using namespace std;

typedef struct {
	HINSTANCE hInstance;

	HWND airport_name_text_field;
	HWND map_area;
	HWND list_info_text_field;

	ControlMain* control;
} HANDLES_N_STUFF;


HANDLES_N_STUFF stuff;

LRESULT CALLBACK window_event_handler(HWND, UINT, WPARAM, LPARAM);

void AddControls(HWND, HINSTANCE, HANDLES_N_STUFF*);

TSTRING print_airports(ControlMain*);
TSTRING print_planes(ControlMain*);
TSTRING print_passengers(ControlMain*);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG msg = { 0 };
	WNDCLASSEX window_app = { 0 };

	window_app.cbSize = sizeof(WNDCLASSEX);
	window_app.hbrBackground = (HBRUSH)COLOR_WINDOW;
	window_app.hCursor = LoadCursor(nullptr, IDC_ARROW);
	window_app.hInstance = hInst;
	window_app.lpszClassName = _T("windowClass");
	window_app.lpfnWndProc = window_event_handler;
	window_app.cbWndExtra = sizeof(HANDLES_N_STUFF*);

	if (!RegisterClassEx(&window_app))
		return -1;

	hWnd = CreateWindow(_T("windowClass"), _T("Controlo"), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
						-10, 0, 1500, 1050, HWND_DESKTOP, NULL, hInst, 0);

	HANDLE process_lock_mutex = nullptr;
	try {
		HANDLE process_lock_mutex = lock_process();

		ControlMain* control_main = main_start();

		startAllThreads(control_main);


		if (hWnd != nullptr) {
			//SetWindowLongPtr(hWnd, 0, (LONG_PTR)&structure); // TODO make this works
			stuff.control = control_main;

			while (GetMessage(&msg, NULL, NULL, NULL)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		exit_everything(control_main);
		waitForThreadsToFinish(control_main);

		exitAndSendSentiment(control_main);

		delete control_main;
	} catch (StartException* e) {
		MessageBox(hWnd, e->get_message(), _T("Error"), MB_OK);
		delete e;
	}

	CloseHandle(process_lock_mutex);

	return 0;
}


DWORD WINAPI draw_map(HBITMAP h_bitmap, HDC dc, ControlMain* control) {
	SelectObject(dc, h_bitmap);

	TextOut(dc, 10, 10, _T("Eyeedsasdasdasdasdasdaasdasdasd"), 1);
	
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


LRESULT CALLBACK window_event_handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	HANDLES_N_STUFF* main_struct = nullptr;

	static HDC double_buffer_dc;
	static HBITMAP double_buffer_bitmap;

	//main_struct = (HANDLES_N_STUFF*)GetWindowLongPtr(hWnd, 0); // TODO ERROR FIX - o GetWindowLongPtr está a retornar NULL
	main_struct = &stuff;
	main_struct->hInstance = GetModuleHandle(NULL);

	switch (msg) {
		case WM_COMMAND:
			switch (wParam) {
				case ADD_AIRPORT: {
					InvalidateRect(hWnd, nullptr, false);

					TCHAR text[100];
					GetWindowText(main_struct->airport_name_text_field, text, sizeof(text));
					//SetWindowTextW(main_struct->map_area, text);
					//draw on map but if not correct show output on list_text_field
					vector<TSTRING> input_parts = string_split(text, _T(" "));
					if (input_parts.size() != 3) {
						SetWindowText(main_struct->list_info_text_field, _T("Invalid input"));
						break;
					}
					const int pos_x = _ttoi(input_parts[1].c_str());
					const int pos_y = _ttoi(input_parts[2].c_str());
					auto success = main_struct->control->add_airport(input_parts[0].c_str(), pos_x, pos_y);
					if (!success) {
						SetWindowText(main_struct->list_info_text_field, _T("There is either an airport with that name or there is one too close (10 grid units)"));
						break;
					}
					SetWindowText(main_struct->list_info_text_field, _T(""));
					break;
				}

				case LIST_AIRPORTS: {
					TSTRING str = print_airports(main_struct->control);
					SetWindowText(main_struct->list_info_text_field, str.c_str());
					break;
				}
				case LIST_PLANES:
					{
						TSTRING str = print_planes(main_struct->control);
						SetWindowText(main_struct->list_info_text_field, str.c_str());
						break;
					}

					break;
				case LIST_PASSANGERS:
					{
						TSTRING str = print_passengers(main_struct->control);
						SetWindowText(main_struct->list_info_text_field, str.c_str());
						break;
					}

					break;
				case ACCEPT:

					break;
			}
			break;
		case WM_CLOSE:
			if (MessageBox(hWnd, _T("Are you sure you want to exit?"), _T("Exit"), MB_YESNO) == IDYES) {
				PostQuitMessage(0);
			}
			break;
		case WM_DESTROY:
			DeleteObject(double_buffer_bitmap);
			DeleteDC(double_buffer_dc);
			break;
		case WM_CREATE: {
			AddControls(hWnd, main_struct->hInstance, main_struct);

			HDC dc = GetDC(hWnd);
			double_buffer_dc = CreateCompatibleDC(dc);
			double_buffer_bitmap = CreateCompatibleBitmap(dc, MAP_SIZE, MAP_SIZE);
			SelectObject(double_buffer_dc, double_buffer_bitmap);

			SelectObject(double_buffer_dc, GetStockObject(BLACK_BRUSH));
			PatBlt(double_buffer_dc, 0, 0, MAP_SIZE, MAP_SIZE, PATCOPY);

			break;
		}
		case WM_ERASEBKGND:
			break;
		case WM_PAINT: {
			PAINTSTRUCT paint_struct;
			const HDC hdc = BeginPaint(hWnd, &paint_struct);

			//TODO make this be called from somewhere else
			draw_map(double_buffer_bitmap, double_buffer_dc, main_struct->control);

			BitBlt(hdc, 0, 0, 1000, 1000, double_buffer_dc, 0, 0, SRCCOPY);

			EndPaint(hWnd, &paint_struct);
			break;
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

void AddControls(HWND hWnd, HINSTANCE hInstance, HANDLES_N_STUFF* main_struct) {

	main_struct->map_area =
		CreateWindowW(_T("Static"), _T(""), WS_VISIBLE | WS_CHILD | WS_BORDER,
					  0, 0, MAP_SIZE, MAP_SIZE, hWnd, NULL, NULL, NULL);

	main_struct->airport_name_text_field =
		CreateWindowW(_T("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER, 1172, 10, 250, 30, hWnd, NULL, hInstance, NULL);

	CreateWindowW(_T("Button"), _T("Add Airport"), WS_VISIBLE | WS_CHILD , 1050, 10, 120, 30, hWnd, (HMENU)ADD_AIRPORT, NULL, NULL);

	main_struct->list_info_text_field =
		CreateWindowW(_T("Edit"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_HSCROLL |
			WS_VSCROLL | ES_LEFT | ES_MULTILINE |
			ES_AUTOHSCROLL | ES_AUTOVSCROLL, 1050, 50, 380, 440, hWnd, NULL, hInstance, NULL);

	CreateWindowW(_T("Button"), _T("List Airports"), WS_VISIBLE | WS_CHILD, 1050, 500, 120, 30, hWnd, (HMENU)LIST_AIRPORTS, NULL, NULL);
	CreateWindowW(_T("Button"), _T("List Planes"), WS_VISIBLE | WS_CHILD, 1172, 500, 120, 30, hWnd, (HMENU)LIST_PLANES, NULL, NULL);
	CreateWindowW(_T("Button"), _T("List Passangers"), WS_VISIBLE | WS_CHILD, 1294, 500, 120, 30, hWnd, (HMENU)LIST_PASSANGERS, NULL, NULL);
	CreateWindowW(_T("Button"), _T("Accept"), WS_VISIBLE | WS_CHILD, 1050, 532, 120, 30, hWnd, (HMENU)ACCEPT, NULL, NULL);
}

TSTRING print_airports(ControlMain* control)
{
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

TSTRING print_planes(ControlMain* control)
{
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

			}
			else {
				stream << _T(", airport: ") << control->get_airport(plane->origin_airport_id)->name << END_LINE;
			}
		}
	}
	return stream.str();
}
TSTRING print_passengers(ControlMain* control)
{
	tstringstream stream;
	for (auto passenger : control->all_passengers)
	{
		stream << _T("id: ") << passenger.second->id
			<< _T(", name: ") << passenger.second->name
			<< _T(", origin: ") << passenger.second->origin->name
			<< _T(", destiny:") << passenger.second->destiny->name << END_LINE;
	}

	return stream.str();
}