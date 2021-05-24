#pragma once
#include "SharedStructContents.h"


struct WhereTo {
	TCHAR origin[BUFFER_SIZE];
	TCHAR destiny[BUFFER_SIZE];
	TCHAR name[BUFFER_SIZE];
};

union PassagControlData {
	struct WhereTo where_to;
};


typedef struct {
	
	union PassagControlData data;
}PassagControlMessage;