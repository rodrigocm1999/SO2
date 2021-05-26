#pragma once
#include "SharedPassagStruct.h"

#include "Windows.h"
#include <string>

#define TSTRING std::basic_string<TCHAR>

typedef struct {
	HANDLE shutdown_event;
	HANDLE wait_thread;
	HANDLE main_thread;
	HANDLE control_pipe;
	HANDLE this_pipe;
	
	TSTRING destiny_port;
	TSTRING origin_port;
	
	int max_wait_time;
	bool exit;
	bool timeout;
} TimerThread;

bool read_pipe(HANDLE pipe, PassengerMessage& message);

DWORD WINAPI max_wait_timer(LPVOID param);

void receive_control_updates(TimerThread* timer_thread);