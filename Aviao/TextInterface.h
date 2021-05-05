#pragma once
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include "PlaneMain.h"

using namespace std;

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tcout wcout
#define tcin wcin
#define tstringstream wstringstream
#else
#define tcout cout
#define tcin cin
#define tstringstream stringstream
#endif

PlaneControlMessage ready_message(PlaneMain* plane_main, int type);
void fly_command(PlaneMain* plane_main);
DWORD WINAPI fly_plane(LPVOID param);
void board_command(PlaneMain* plane_main);
void destiny_command(PlaneMain* plane_main, vector<TSTRING> input_parts);
void exit_command(PlaneMain* plane_main);

void enter_text_interface_plane(PlaneMain* plane_main);
