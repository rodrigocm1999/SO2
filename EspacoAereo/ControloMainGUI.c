#include <windows.h>
#include <tchar.h>
#include "resource.h" //1º passo

#define ADD_AIRPORT 1
#define LIST_AIRPORTS 2
#define LIST_PLANES 3
#define LIST_PASSANGERS 4
#define ACCEPT 5


//LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM)

typedef struct {
	HINSTANCE hInstance;
	HWND airport_name_text_field;
} HANDLES_N_STUFF;


HANDLES_N_STUFF stuff;

LRESULT CALLBACK window_procedure(HWND, UINT, WPARAM, LPARAM);

void AddControls(HWND, HANDLES_N_STUFF*);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG msg = { 0 };
	WNDCLASSA window_app = { 0 };

	TCHAR* window_name = _T("Controlo Aeroporto");
	
	//window_app.cbSize = sizeof(WNDCLASSEX);
	
	window_app.hbrBackground = (HBRUSH)COLOR_WINDOW;
	window_app.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_app.hInstance = hInst;
	window_app.lpszClassName = window_name;
	window_app.lpfnWndProc = window_procedure;
	
	window_app.cbWndExtra = sizeof(HANDLES_N_STUFF*);

	if (!RegisterClassA(&window_app))
		return GetLastError();
	
	hWnd = CreateWindow(window_name, _T("Controlo"), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
						0, 0, 1500, 1050, HWND_DESKTOP, NULL, hInst, 0);
	
	int error_debug = GetLastError();

	HANDLES_N_STUFF structure;
	if (SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&structure) == 0) {
		return GetLastError();
	}
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK window_procedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HANDLES_N_STUFF* main_struct = NULL;
	main_struct = &stuff;
	//main_struct = (HANDLES_N_STUFF*)GetWindowLongPtr(hWnd, GWLP_USERDATA); // TODO ERROR FIX - o GetWindowLongPtr está a retornar 
	main_struct->hInstance = GetModuleHandle(NULL);
	

	switch (msg) {
		case WM_COMMAND:
			switch (wParam) {
				case ADD_AIRPORT: {
					TCHAR text[100];
					GetWindowTextW(main_struct->airport_name_text_field, text, sizeof(text));
					SetWindowTextW(hWnd, text);
					break;
				}
			}

			break;
		case WM_CLOSE:
			if (MessageBox(hWnd, _T("Será?"), _T("sike"), MB_YESNO) == IDYES) {
				PostQuitMessage(0);
			}
			break;

		case WM_CREATE:
			AddControls(hWnd, main_struct);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void AddControls(HWND hWnd, HANDLES_N_STUFF* main_struct) {
	HINSTANCE hInstance = main_struct->hInstance;

	//TODO adicionar os handles e merdas na struct e meter pa dentro
	CreateWindowW(_T("static"), _T("isto vai ser o mapa e esta feito 1000x1000 "), WS_VISIBLE | WS_CHILD | WS_BORDER,
				  20, 1, 1000, 1000, hWnd, NULL, NULL, NULL);

	main_struct->airport_name_text_field =
		CreateWindowW(_T("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER, 1172, 10, 250, 30, hWnd, NULL, hInstance, NULL);

	CreateWindowW(_T("Button"), _T("Add Airport"), WS_VISIBLE | WS_CHILD, 1050, 10, 120, 30, hWnd, NULL, NULL, NULL);

	CreateWindowW(_T("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER, 1050, 50, 380, 440, hWnd, NULL, hInstance, NULL);

	CreateWindowW(_T("Button"), _T("List Airports"), WS_VISIBLE | WS_CHILD, 1050, 500, 120, 30, hWnd, NULL, NULL, NULL);
	CreateWindowW(_T("Button"), _T("List Planes"), WS_VISIBLE | WS_CHILD, 1172, 500, 120, 30, hWnd, NULL, NULL, NULL);
	CreateWindowW(_T("Button"), _T("List Passangers"), WS_VISIBLE | WS_CHILD, 1294, 500, 120, 30, hWnd, NULL, NULL,
				  NULL);
	CreateWindowW(_T("Button"), _T("Accept"), WS_VISIBLE | WS_CHILD, 1050, 532, 120, 30, hWnd, NULL, NULL, NULL);
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
