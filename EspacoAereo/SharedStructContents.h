#pragma once
#include <Windows.h>

#include "CircularBuffer.h"

#define SEMAPHORE_NAME_MAX_PLANES _T("Airport_MaxPlanes")
#define MAPPED_MEMORY_IDENTIFIER _T("Airport_shared_memory")

#define BUFFER_SIZE 100

#define NEXT_DESTINY 10
#define TO_BOARD 20
#define START_TRIP 21
#define FINISHED_TRIP 22
#define PLANE_LEAVES 30
#define PLANE_CRASHES 31

#define MAX_PLANES 10


typedef union {
	TCHAR airport_name[BUFFER_SIZE];
}Data;

typedef struct {
	int type;
	Data data;
}Message;


typedef struct {
	int id;
	int max_passag;
	int velocity;

	TCHAR origin[BUFFER_SIZE];
	TCHAR destiny[BUFFER_SIZE];

}Plane;


typedef struct {


	Plane planes[10];


	//CircularBuffer<Message> control_read[MAX_PLANES];
	//CircularBuffer<Message> control_write[MAX_PLANES];

}SharedStructContents;