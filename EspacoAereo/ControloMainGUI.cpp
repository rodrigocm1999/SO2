#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <sstream>

#include "ControlFunctions.h"
#include "MainBreakdown.h"
#include "resource.h" //1º passo
#include "StartException.h"
#include "Utils.h"

#define ADD_AIRPORT 1
#define LIST_AIRPORTS 2
#define LIST_PLANES 3
#define LIST_PASSANGERS 4
#define ACCEPT 5


#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tstringstream std::wstringstream
#else
#define tstringstream std::stringstream
#endif

using namespace std;

//LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM)

typedef struct {
	HINSTANCE hInstance;

	HWND airport_name_text_field;
	HWND map_area;
	HWND list_info_text_field;

	HDC double_buffer_dc;
	HBITMAP double_buffer_bitmap;

	ControlMain* control;
} HANDLES_N_STUFF;


HANDLES_N_STUFF stuff;

LRESULT CALLBACK window_procedure(HWND, UINT, WPARAM, LPARAM);

void AddControls(HWND, HANDLES_N_STUFF*);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG msg = { 0 };
	WNDCLASSEX window_app = { 0 };

	window_app.cbSize = sizeof(WNDCLASSEX);
	window_app.hbrBackground = (HBRUSH)COLOR_WINDOW;
	window_app.hCursor = LoadCursor(nullptr, IDC_ARROW);
	window_app.hInstance = hInst;
	window_app.lpszClassName = _T("windowClass");
	window_app.lpfnWndProc = window_procedure;

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
	}
	catch (StartException* e) {
		MessageBox(hWnd, e->get_message(), _T("Error"), MB_OK);
		delete e;
	}

	CloseHandle(process_lock_mutex);

	return 0;
}

LRESULT CALLBACK window_procedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	HANDLES_N_STUFF* main_struct = nullptr;

	static HDC double_buffer_dc;
	static HBITMAP double_buffer_bitmap;

	//main_struct = (HANDLES_N_STUFF*)GetWindowLongPtr(hWnd, 0); // TODO ERROR FIX - o GetWindowLongPtr está a retornar NULL
	main_struct = &stuff;
	main_struct->hInstance = GetModuleHandle(NULL);

	switch (msg) {
	case WM_COMMAND:
		switch (wParam) {
		case ADD_AIRPORT:
		{
			TCHAR text[100];
			GetWindowText(main_struct->airport_name_text_field, text, sizeof(text));
			//SetWindowTextW(main_struct->map_area, text);
			//draw on map but if not correct show output on list_text_field
			vector<TSTRING> input_parts = string_split(text, _T(" "));
			const int pos_x = _ttoi(input_parts[1].c_str());
			const int pos_y = _ttoi(input_parts[2].c_str());
			main_struct->control->add_airport(input_parts[0].c_str(), pos_x, pos_y);
			break;
		}

		case LIST_AIRPORTS:
		{
			tstringstream stream;
			for (auto pair : main_struct->control->airports) {
				Airport* airport = pair.second;


				stream << _T("Airport -> name : ") << airport->name << _T("\tposition : ") << airport->position.x << _T(", ") << airport->position.y;
				//std::wcin >> _T("Airport -> name : ") >> airport->name >> _T("\tposition : ") >> airport->position.x >> _T(", ") >> airport->position.y >> text;

			}

			TSTRING str = stream.str();
			SetWindowText(main_struct->list_info_text_field, str.c_str());
			break;
		}
		case LIST_PLANES:

			break;
		case LIST_PASSANGERS:

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
	case WM_CREATE:
		AddControls(hWnd, main_struct);
		break;

	case WM_PAINT:
		PAINTSTRUCT paint_struct;
		HDC hdc = BeginPaint(hWnd, &paint_struct);

		BitBlt(hdc, 0, 0, 1000, 1000, double_buffer_dc, 0, 0, SRCCOPY);

		EndPaint(hWnd, &paint_struct);

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

void AddControls(HWND hWnd, HANDLES_N_STUFF* main_struct) {
	HINSTANCE hInstance = main_struct->hInstance;

	main_struct->map_area =
		CreateWindowW(_T("static"), _T("isto vai ser o mapa e esta feito 1000x1000 "), WS_VISIBLE | WS_CHILD | WS_BORDER,
			20, 1, 1000, 1000, hWnd, NULL, NULL, NULL);

	main_struct->airport_name_text_field =
		CreateWindowW(_T("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER, 1172, 10, 250, 30, hWnd, NULL, hInstance, NULL);

	CreateWindowW(_T("Button"), _T("Add Airport"), WS_VISIBLE | WS_CHILD, 1050, 10, 120, 30, hWnd, (HMENU)ADD_AIRPORT, NULL, NULL);

	main_struct->list_info_text_field =
		CreateWindowW(_T("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER, 1050, 50, 380, 440, hWnd, NULL, hInstance, NULL);

	CreateWindowW(_T("Button"), _T("List Airports"), WS_VISIBLE | WS_CHILD, 1050, 500, 120, 30, hWnd, (HMENU)LIST_AIRPORTS, NULL, NULL);
	CreateWindowW(_T("Button"), _T("List Planes"), WS_VISIBLE | WS_CHILD, 1172, 500, 120, 30, hWnd, (HMENU)LIST_PLANES, NULL, NULL);
	CreateWindowW(_T("Button"), _T("List Passangers"), WS_VISIBLE | WS_CHILD, 1294, 500, 120, 30, hWnd, (HMENU)LIST_PASSANGERS, NULL, NULL);
	CreateWindowW(_T("Button"), _T("Accept"), WS_VISIBLE | WS_CHILD, 1050, 532, 120, 30, hWnd, (HMENU)ACCEPT, NULL, NULL);
}



//
//LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
//	static TCHAR letra = 'a';
//	HDC hdc, auxdc;
//	static HBITMAP bmp, bmp2, foto, double_dc; 
//	PAINTSTRUCT paint;
//	switch (messg) {
//	case WM_DESTROY:
//		DeleteObject(bmp);
//		PostQuitMessage(0);
//		break;
//
//	case WM_PAINT:
//
//		
//		hdc = BeginPaint(hWnd, &paint);
//		
//		auxdc = CreateCompatibleDC(hdc);
//		
//		SelectObject(auxdc, bmp);
//		
//		BitBlt(hdc, 10, 10, 100, 100, auxdc, 0, 0, SRCCOPY);
//		DeleteDC(auxdc);
//		EndPaint(hWnd, &paint);
//		break;
//	case WM_CREATE:
//		
//		bmp = LoadBitmap(hInstanceGlobal, MAKEINTRESOURCE(IDB_BITMAP1));
//
//		hdc = GetDC(hWnd);
//		double_dc = CreateCompatibleDC(hdc);
//		foto = CreateCompatibleBitmap(hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
//		SelectObject(double_dc, foto);
//		ReleaseDC(hWnd, hdc);
//		DeleteObject(foto);
//		break;
//	case WM_KEYDOWN:
//		switch (wParam) {
//		case VK_UP:
//			letra = 'c';
//			break;
//
//		case VK_DOWN:
//			letra = 'b';
//			break;
//		}
//		break;
//
//	case WM_CHAR:
//		letra = wParam;
//		break;
//
//	case WM_LBUTTONDOWN:
//		hdc = GetDC(hWnd);
//		TextOut(hdc, LOWORD(lParam), HIWORD(lParam), &letra, 1);
//		ReleaseDC(hWnd, hdc);
//		break;
//	case WM_COMMAND:
//
//		break;
//	default:
//		
//		return(DefWindowProc(hWnd, messg, wParam, lParam));
//		break; 
//	}
//	return(0);
//}
