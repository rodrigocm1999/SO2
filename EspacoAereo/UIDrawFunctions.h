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


DWORD WINAPI draw_map(HBITMAP h_bitmap, HDC dc, ControlMain* control);
void draw_plane(HBITMAP h_bitmap, HDC dc, const Position& pos);
void draw_airport(HBITMAP h_bitmap, HDC dc, const Position& pos);

TSTRING print_airports(ControlMain* control);
TSTRING print_planes(ControlMain* control);
TSTRING print_passengers(ControlMain* control);