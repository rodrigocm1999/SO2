#pragma once
#include "SharedPassagStruct.h"

#include "Windows.h"
#include <string>

#define TSTRING std::basic_string<TCHAR>

typedef struct {
	HANDLE stop_wait_event;
	HANDLE pipe_updates_thread;
	HANDLE control_pipe;
	HANDLE this_pipe;
	HANDLE timeout_thread;
	HANDLE exit_command_thread;

	CRITICAL_SECTION critical_section;

	TSTRING destiny_port;
	TSTRING origin_port;

	int max_wait_time;
	bool exit;
	bool timeout;
	bool still_waiting;

} PassagStruct;

bool read_pipe(HANDLE pipe, PassengerMessage& message);

DWORD WINAPI max_wait_timer(LPVOID param);
DWORD WINAPI receive_control_updates(LPVOID param);
DWORD WINAPI receive_exit_command(LPVOID param);

void exit_passenger(PassagStruct* main_struct);