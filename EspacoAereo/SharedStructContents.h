#pragma once
#include <tchar.h>

#define SEMAPHORE_MAX_PLANES _T("Airport_max_planes")
#define MAPPED_MEMORY_IDENTIFIER _T("Airport_shared_memory")

#define TYPE_NEW_PLANE 1
#define TYPE_PLANE_NOT_ALLOWED 2

#define TYPE_CONTROL_EXITING 40

#define TYPE_NEXT_DESTINY 10
#define TYPE_PLANE_OK_DESTINY 11
#define TYPE_PLANE_BAD_DESTINY 12

#define TYPE_TO_BOARD 20
#define TYPE_START_TRIP 21
#define TYPE_FINISHED_TRIP 22
#define TYPE_PLANE_MOVED 23

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

#define MAP_EMPTY 255
#define MAP_AIRPORT 254

#define BUFFER_SIZE 30
#define CIRC_BUFFER_SIZE 5

#define NOT_DEFINED_AIRPORT -1

typedef struct {
	int x; int y;
}Position;


typedef union { // check if needs more stuff to send
	TCHAR airport_name[BUFFER_SIZE];
	Position position;
}Data;

typedef struct {
	unsigned char plane_offset;
	int type;
	Data data;
}PlaneControlMessage;


typedef struct {
	unsigned char in;
	unsigned char out;
	PlaneControlMessage buffer[CIRC_BUFFER_SIZE];
}CircBuffer;

typedef struct {
	CircBuffer circular_buffer;
	unsigned char map[MAP_SIZE][MAP_SIZE];

	unsigned char max_plane_amount;
}SharedControl;

typedef struct {
	unsigned char offset;
	int max_passengers;
	unsigned char velocity;

	bool is_flying;
	Position position;
	
	bool in_use;
	bool heartbeat;

	unsigned int origin_airport_id;
	unsigned int destiny_airport_id;

	CircBuffer buffer;
}Plane;