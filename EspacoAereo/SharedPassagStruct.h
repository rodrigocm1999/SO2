#pragma once

#define CONTROL_PIPE_MAIN _T("\\\\.\\pipe\\CONTROL_PIPE_MAIN")

#define PIPE_NAME_PREFIX _T("\\\\.\\pipe\\")

#define PASSENGER_TYPE_NEW_PASSENGER 1
#define PASSENGER_TYPE_GOOD_AIRPORTS 2
#define PASSENGER_TYPE_BAD_AIRPORTS 3

#define PASSENGER_TYPE_BOARDED 10
#define PASSENGER_TYPE_STARTED_FLYING 11
#define PASSENGER_TYPE_MOVED 12
#define PASSENGER_TYPE_PLANE_ARRIVED 15
#define PASSENGER_TYPE_PLANE_CRASHED 16

#define PASSENGER_TYPE_GAVE_UP 20

#define PASSENGER_TYPE_CONTROL_EXITING 30

#include "SharedStructContents.h"

struct new_passenger_struct {
	TCHAR origin[BUFFER_SIZE];
	TCHAR destiny[BUFFER_SIZE];
	TCHAR name[BUFFER_SIZE];
};

union passenger_control_data {
	struct new_passenger_struct new_passenger;
	struct position pos;
};

typedef struct {
	unsigned int id;
	int type;
	union passenger_control_data data;
}PassengerMessage;