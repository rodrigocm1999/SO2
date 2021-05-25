#pragma once

#include <string>

#include "Windows.h"

#include "SharedPassagStruct.h"

#define TSTRING std::basic_string<TCHAR>

class Airport;

class Passenger {

public:

	const DWORD id;
	Airport* const origin;
	Airport* const destiny;
	const std::basic_string<TCHAR> name;

	HANDLE pipe;

	Passenger(const DWORD id, Airport* origin, Airport* destiny, const TSTRING& name);
	~Passenger();

	bool send_message(PassengerMessage& message);
	bool send_message(int type);

};

