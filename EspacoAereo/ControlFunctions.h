#pragma once
#include "ControlMain.h"


DWORD WINAPI receive_updates(LPVOID param);

void exit_everything(ControlMain* control_main);

DWORD WINAPI heartbeat_checker(LPVOID param);

DWORD WINAPI passenger_piper_receiver(LPVOID param);