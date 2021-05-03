#pragma once
#include <tchar.h>

#define SEMAPHORE_MAX_PLANES _T("Airport_max_planes")
#define MAPPED_MEMORY_IDENTIFIER _T("Airport_shared_memory")

#define BUFFER_SIZE 40


#define TYPE_NEW_PLANE 1
#define TYPE_PLANE_NOT_ALLOWED 2

#define TYPE_CONTROL_EXITING 40

#define TYPE_NEXT_DESTINY 10
#define TYPE_PLANE_OK_DESTINY 11
#define TYPE_PLANE_BAD_DESTINY 12

#define TYPE_TO_BOARD 20
#define TYPE_START_TRIP 21
#define TYPE_FINISHED_TRIP 22

#define TYPE_PLANE_LEAVES 30
#define TYPE_PLANE_CRASHES 31


#define CONTROL_MUTEX_PREFIX -99

#define MUTEX_GET _T("_AIRPORT_MUTEX_GET")
#define MUTEX_SET _T("_AIRPORT_MUTEX_SET")
#define MUTEX_EMPTY_SPOT _T("_AIRPORT_MUTEX_EMPTY_SPOT")
#define MUTEX_AVAILABLE_ITEM _T("_AIRPORT_MUTEX_AVAILABLE_ITEM")

#define CONTROL_WROTE_EVENT _T("_AIRPORT_CONTROL_WROTE_EVENT")
#define PLANE_WROTE_EVENT _T("_AIRPORT_PLANE_WROTE_EVENT")

#define MAP_SIZE 1000

#define MAP_EMPTY 0
#define MAP_AIRPORT 1
#define MAP_PLANE 2

#define CIRC_BUFFER_SIZE 5

typedef struct {
	int x; int y;
}Position;


typedef union { // check if needs more stuff to send
	TCHAR airport_name[BUFFER_SIZE];
	Position position;
}Data;

typedef struct {
	int plane_offset;
	int type;
	Data data;
}PlaneControlMessage;


typedef struct {
	unsigned int in;
	unsigned int out;
	PlaneControlMessage buffer[CIRC_BUFFER_SIZE];
}CircBuffer;

typedef struct {
	CircBuffer circular_buffer;
	char map[MAP_SIZE][MAP_SIZE];

	int max_plane_amount;
}SharedControl;

typedef struct {
	int offset;
	int max_passengers;
	int velocity;
	bool is_flying;

	Position position;

	bool in_use;
	bool heartbeat;

	TCHAR origin[BUFFER_SIZE];
	TCHAR destiny[BUFFER_SIZE];

	CircBuffer buffer;
}Plane;
