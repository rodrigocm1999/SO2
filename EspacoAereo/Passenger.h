#pragma once

#include <string>

#include "Windows.h"

#include "SharedPassagStruct.h"

#define TSTRING std::basic_string<TCHAR>

#define PASSENGER_ID DWORD

class Airport;

class Passenger {
	
public:

	PASSENGER_ID id;
	Airport* const origin;
	Airport* const destiny;
	const std::basic_string<TCHAR> name;

	bool boarded;
	PLANE_ID flying_plane_id;
	
	HANDLE pipe;

	Passenger(PASSENGER_ID id, Airport* origin, Airport* destiny, const TSTRING& name);
	~Passenger();


private:
	bool send_message(PassengerMessage& message);
};

