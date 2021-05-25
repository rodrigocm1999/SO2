#pragma once
#include "SharedPassagStruct.h"

#include "Windows.h"
#include <string>

#define TSTRING std::basic_string<TCHAR>

bool read_pipe(HANDLE pipe, PassengerMessage& message);

void receive_control_updates(HANDLE passenger_pipe, TSTRING& destiny_port);
