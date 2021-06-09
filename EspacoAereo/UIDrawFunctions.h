#pragma once

#include <Windows.h>
#include <sstream>

#include "Airport.h"
#include "SharedStructContents.h"
#include "ControlMain.h"

#define END_LINE _T("\r\n")

#define WINDOW_BAR_HEIGHT 32

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tstringstream std::wstringstream
#else
#define tstringstream std::stringstream
#endif


typedef struct {
	HINSTANCE hInstance;

	HWND window;
	HWND map_area;

	HWND airport_name_text_field;
	HWND list_info_text_field;
	HWND accept_window;

	HBITMAP airport_icon;
	HBITMAP plane_icon;

	BOOL mouse_hover;
	BOOL mouse_click;

	Position mouse_pos;

	HANDLE draw_thread_handle;
	HDC double_buffer_dc;
	HBITMAP double_buffer_bitmap;

	ControlMain* control;
} HANDLES_N_STUFF;


#define ICON_SIZE 28
#define REFRESH_WAIT 1000

DWORD WINAPI draw_map_thread(LPVOID param);

void update_map_area(HANDLES_N_STUFF* stuff);
void draw_map(HDC bitmap_dc, HANDLES_N_STUFF* handles);
void draw_img(HBITMAP h_bitmap, HDC bitmap_dc, HDC aux_dc, const Position& pos);

TSTRING print_airports(ControlMain* control);
TSTRING print_planes(ControlMain* control);
TSTRING print_passengers(ControlMain* control);

void set_accept_state(HANDLES_N_STUFF* handles);


void get_cursor_pos(Position* position);