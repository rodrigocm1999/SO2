#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <sstream>

#include "ControlFunctions.h"
#include "MainBreakdown.h"
#include "resource.h"
#include "resource1.h"
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

void AddControls(HWND hWnd, HINSTANCE hInstance, ControlUIHandles* main_struct);

LRESULT CALLBACK window_event_handler(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK map_event_handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
	window_app.cbWndExtra = sizeof(ControlUIHandles*);

	if (!RegisterClassEx(&window_app))
		return -1;

	ControlUIHandles vars;
	vars.hInstance = hInst;
	ControlMain::temp_ptr = &vars;

	hWnd = CreateWindow(_T("windowClass"), _T("Controlo"), WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_BORDER | WS_MAXIMIZE,
						0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), HWND_DESKTOP, NULL, hInst, 0);
	vars.window = hWnd;

	if (hWnd != nullptr) {

		HANDLE process_lock_mutex = nullptr;
		try {
			process_lock_mutex = lock_process();

			ControlMain* control_main = main_start();

			start_all_threads(control_main);

			vars.plane_icon = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PLANEICON));
			vars.airport_icon = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRPORTICON));

			vars.control = control_main;
			vars.draw_thread_handle = create_thread(draw_map_thread, &vars);


			SetWindowLongPtr(hWnd, 0, (LONG_PTR)&vars); // TODO make this works

			while (GetMessage(&msg, nullptr, NULL, NULL)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			exit_everything(control_main);
			wait_for_threads_to_finish(control_main);

			exit_and_send_sentiment(control_main);

			delete control_main;

		} catch (StartException* e) {
			MessageBox(hWnd, e->get_message(), _T("Error"), MB_OK);
			delete e;
		}

		CloseHandle(process_lock_mutex);
	}
	DeleteObject(vars.plane_icon);
	DeleteObject(vars.airport_icon);

	return (int)msg.wParam;
}


LRESULT CALLBACK window_event_handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	ControlUIHandles* main_struct = (ControlUIHandles*)GetWindowLongPtr(hWnd, 0);

	static bool already_created = false;

	if (main_struct != nullptr && !already_created) {
		//AddControls(hWnd, main_struct->hInstance, main_struct); // Não funciona, ao criar os controlos aqui,
		//		embora apenas corra 1 vez, os controlos ficam com problemas
		already_created = true; // Já não é necessário
	}

	switch (msg) {
		case WM_COMMAND:
			switch (wParam) {

				case ADD_AIRPORT: {
					update_map_area(main_struct);

					TCHAR text[100];
					GetWindowText(main_struct->airport_name_text_field, text, sizeof(text) - 1);
					vector<TSTRING> input_parts = string_split(text, _T(" "));
					if (input_parts.size() != 3) {
						SetWindowText(main_struct->list_info_text_field, _T("Invalid input -> <airport_name> <posX> <poxY>"));
						break;
					}
					const int pos_x = _ttoi(input_parts[1].c_str());
					const int pos_y = _ttoi(input_parts[2].c_str());

					if (pos_x >= MAP_SIZE || pos_y >= MAP_SIZE) {
						SetWindowText(main_struct->list_info_text_field, _T("Position exceds limit"));
						break;
					}

					auto success = main_struct->control->add_airport(input_parts[0].c_str(), pos_x, pos_y);
					if (!success) {
						SetWindowText(main_struct->list_info_text_field, _T("There is either an airport with that name or there is one too close (10 grid units)"));
						break;
					}
					SetWindowText(main_struct->list_info_text_field, _T(""));
					SetWindowText(main_struct->airport_name_text_field, _T(""));
					break;
				}

				case LIST_AIRPORTS: {
					TSTRING str = print_airports(main_struct->control);
					SetWindowText(main_struct->list_info_text_field, str.c_str());
					break;
				}

				case LIST_PLANES: {
					TSTRING str = print_planes(main_struct->control);
					SetWindowText(main_struct->list_info_text_field, str.c_str());
					break;
				}

				case LIST_PASSANGERS: {
					TSTRING str = print_passengers(main_struct->control);
					SetWindowText(main_struct->list_info_text_field, str.c_str());
					break;
				}

				case ACCEPT:
					set_accept_state(main_struct);
					break;
			}
			break;
		case WM_CLOSE:
			if (MessageBox(hWnd, _T("Are you sure you want to exit?"), _T("Exit"), MB_YESNO) == IDYES) {
				PostQuitMessage(0);
			}
			break;
		case WM_CREATE: {
			auto handles = (ControlUIHandles*) ControlMain::temp_ptr;
			AddControls(hWnd, handles->hInstance, handles);
			ControlMain::temp_ptr = nullptr;
			// Se não forem adicionados na mensagem do create ficam a piscar e com comportamento estranho
			// Por isso foi criada este ponteiro global apenas para poder chamar a função, já que o create
			//		corre antes de ser possível chamar o SetWindowLongPtr()
			break;
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return TRUE;
}

void AddControls(HWND hWnd, HINSTANCE hInstance, ControlUIHandles* main_struct) {
	main_struct->map_area =
		CreateWindowW(_T("Static"), _T(""), WS_VISIBLE | WS_CHILD | WS_BORDER,
					  0, 0, MAP_SIZE, MAP_SIZE, hWnd, NULL, hInstance, NULL);
	SetWindowLongPtr(main_struct->map_area, GWLP_USERDATA, (LONG_PTR)main_struct);
	SetWindowLongPtr(main_struct->map_area, GWLP_WNDPROC, (LONG_PTR)map_event_handler);
	InvalidateRect(main_struct->map_area, nullptr, false);


	main_struct->airport_name_text_field =
		CreateWindowW(_T("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER, 1172, 10, 400, 30, hWnd, NULL, hInstance, NULL);

	CreateWindowW(_T("Button"), _T("Add Airport"), WS_VISIBLE | WS_CHILD, 1050, 10, 120, 30, hWnd, (HMENU)ADD_AIRPORT, hInstance, NULL);

	main_struct->list_info_text_field =
		CreateWindowW(_T("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_HSCROLL |
					  WS_VSCROLL | ES_LEFT | ES_MULTILINE |
					  ES_AUTOHSCROLL | ES_AUTOVSCROLL, 1050, 50, 550, 450, hWnd, NULL, hInstance, NULL);

	CreateWindowW(_T("Button"), _T("List Airports"), WS_VISIBLE | WS_CHILD, 1050, 500, 120, 30, hWnd, (HMENU)LIST_AIRPORTS, hInstance, NULL);
	CreateWindowW(_T("Button"), _T("List Planes"), WS_VISIBLE | WS_CHILD, 1265, 500, 120, 30, hWnd, (HMENU)LIST_PLANES, hInstance, NULL);
	CreateWindowW(_T("Button"), _T("List Passangers"), WS_VISIBLE | WS_CHILD, 1480, 500, 120, 30, hWnd, (HMENU)LIST_PASSANGERS, hInstance, NULL);
	CreateWindowW(_T("Button"), _T("Accept"), WS_VISIBLE | WS_CHILD, 1050, 650, 120, 30, hWnd, (HMENU)ACCEPT, hInstance, NULL);
	main_struct->accept_window = CreateWindowW(_T("Static"), _T("New Planes: on"), WS_VISIBLE | WS_CHILD,
											   1172, 650, 120, 35, hWnd, NULL, hInstance, NULL);
}

LRESULT CALLBACK map_event_handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT paint_struct;
	ControlUIHandles* main_struct = (ControlUIHandles*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	static bool already_created = false;

	if (main_struct == nullptr) {
		DefWindowProc(hWnd, msg, wParam, lParam);
		return TRUE;
	}

	if (main_struct != nullptr && !already_created) {
		already_created = true;

		HDC hdc = BeginPaint(hWnd, &paint_struct);
		main_struct->double_buffer_dc = CreateCompatibleDC(hdc);
		main_struct->double_buffer_bitmap = CreateCompatibleBitmap(hdc, MAP_SIZE, MAP_SIZE);
		SelectObject(main_struct->double_buffer_dc, main_struct->double_buffer_bitmap);

		DeleteDC(hdc);
		EndPaint(hWnd, &paint_struct);
	}

	switch (msg) {
		case WM_MOUSEMOVE:
		{ // Mostrar informação do avião
			if (main_struct != nullptr) {
				main_struct->mouse_hover = true;
				main_struct->mouse_click = false;
				get_cursor_pos(&main_struct->mouse_pos);

				ControlMain* control = main_struct->control;

				Plane* plane = control->get_closest_plane(main_struct->mouse_pos, ICON_SIZE / 2);
				if (plane != nullptr && plane->is_flying) {
					const auto list = control->get_passengers_on_plane(plane->offset);
					tstringstream stream;

					stream << _T("\nPlane-> id: ") << plane->offset << END_LINE <<
						_T("\r\t Passengers: ") << (list == nullptr ? 0 : list->size()) <<
						_T("\r\t Flying from '") << control->get_airport(plane->origin_airport_id)->name <<
						_T("'\r\t To '") << control->get_airport(plane->destiny_airport_id)->name << _T("'") << END_LINE;

					SetWindowTextW(main_struct->list_info_text_field, stream.str().c_str());
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{ // Mostrar informação de um aeroporto
			if (main_struct != nullptr) {
				main_struct->mouse_click = true;
				main_struct->mouse_hover = false;
				get_cursor_pos(&main_struct->mouse_pos);


				Airport* airport = main_struct->control->get_closest_airport(main_struct->mouse_pos, ICON_SIZE / 2);
				if (airport != nullptr) {
					tstringstream stream;
					stream << _T("Airport-> name: ") << airport->name << END_LINE <<
						_T("\r\tPlanes quantity: ") << airport->planes.size() <<
						_T("\r\tPeople quantity: ") << airport->get_passengers_count() << END_LINE;

					SetWindowTextW(main_struct->list_info_text_field, stream.str().c_str());
				}
			}
			break;
		}

		case WM_ERASEBKGND:
			break;

		case WM_NCPAINT: {
			if (main_struct != nullptr) {
				HDC hdc = BeginPaint(hWnd, &paint_struct);

				BitBlt(hdc, 0, 0, MAP_SIZE, MAP_SIZE, main_struct->double_buffer_dc, 0, 0, SRCCOPY);

				DeleteDC(hdc);
				EndPaint(hWnd, &paint_struct);
			}
			break;
		}
		case WM_NCDESTROY:
			DeleteDC(main_struct->double_buffer_dc);
			DeleteObject(main_struct->double_buffer_bitmap);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return TRUE;
}
