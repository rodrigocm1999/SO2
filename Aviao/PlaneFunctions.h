#pragma once
#include "PlaneMain.h"

DWORD WINAPI receive_updates(LPVOID param);

DWORD WINAPI heartbeat(LPVOID param);

void exit_everything(PlaneMain* plane_main);

DWORD WINAPI fly_plane(LPVOID param);