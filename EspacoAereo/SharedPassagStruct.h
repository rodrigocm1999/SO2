#pragma once

#include "SharedStructContents.h"

#define CONTROL_PIPE_MAIN _T("\\\\.\\pipe\\CONTROL_PIPE_MAIN")

#define PASSENGER_TYPE_NEW_PASSENGER 1

struct WhereTo {
	TCHAR origin[BUFFER_SIZE];
	TCHAR destiny[BUFFER_SIZE];
	TCHAR name[BUFFER_SIZE];
};

union PassagControlData {
	struct WhereTo where_to;
};


typedef struct {
	unsigned int id;
	int type;
	union PassagControlData data;
}PassagControlMessage;