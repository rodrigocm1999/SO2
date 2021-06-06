#pragma once

#include <Windows.h>
#include <sstream>

#include "Airport.h"
#include "SharedStructContents.h"
#include "ControlMain.h"

#define END_LINE _T("\r\n")


#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tstringstream std::wstringstream
#else
#define tstringstream std::stringstream
#endif


typedef struct {
	HINSTANCE hInstance;

	HWND airport_name_text_field;
	HWND map_area;
	HWND list_info_text_field;

	HBITMAP airport_icon;
	HBITMAP plane_icon;
	
	ControlMain* control;
} HANDLES_N_STUFF;

typedef struct {
	HDC bitmap_dc;
	HANDLES_N_STUFF* handles;
}ToDrawThread;


#define ICON_SIZE 28
#define REFRESH_WAIT 1000


DWORD WINAPI draw_map(HDC bitmap_dc, HANDLES_N_STUFF* handles);
DWORD WINAPI draw_map_thread(LPVOID param);

void draw_img(HBITMAP h_bitmap, HDC bitmap_dc, HDC aux_dc, const Position& pos);

TSTRING print_airports(ControlMain* control);
TSTRING print_planes(ControlMain* control);
TSTRING print_passengers(ControlMain* control);