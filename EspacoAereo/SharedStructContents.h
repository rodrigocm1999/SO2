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
#define TYPE_START_TRIP 23
#define TYPE_FINISHED_TRIP 24
#define TYPE_PLANE_MOVED 25

#define TYPE_PLANE_LEAVES 30
#define TYPE_PLANE_CRASHES 31

#define TYPE_ERROR 999


#define CONTROL_MUTEX_PREFIX -99

#define MUTEX_GET _T("_AIRPORT_MUTEX_GET")
#define MUTEX_SET _T("_AIRPORT_MUTEX_SET")
#define MUTEX_EMPTY_SPOT _T("_AIRPORT_MUTEX_EMPTY_SPOT")
#define MUTEX_AVAILABLE_ITEM _T("_AIRPORT_MUTEX_AVAILABLE_ITEM")
#define PLANE_LOCK_MUTEX _T("PLANE_LOCK_MUTEX")

#define CONTROL_WROTE_EVENT _T("_AIRPORT_CONTROL_WROTE_EVENT")
#define PLANE_WROTE_EVENT _T("_AIRPORT_PLANE_WROTE_EVENT")

#define MAP_SIZE 1000

#define MAP_EMPTY 255
#define MAP_AIRPORT 254

#define MINIMUM_AIRPORT_GRID_DISTANCE 10

#define BUFFER_SIZE 30
#define CIRC_BUFFER_SIZE 5

#define NOT_DEFINED_AIRPORT (-1)


#define HEARTBEAT_TIME_CONTROL 3000L
#define HEARTBEAT_TIME_PLANE 2000L

#define AIRPORT_ID unsigned int
#define PLANE_ID unsigned char


typedef struct position {
	int x;
	int y;
}Position;


typedef union { // check if needs more stuff to send
	TCHAR airport_name[BUFFER_SIZE];
	Position position;
	TCHAR error_message[BUFFER_SIZE];
}Data;

typedef struct PlaneControlMessage {
	PLANE_ID plane_offset;
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
	PLANE_ID offset;
	int max_passengers;
	unsigned char velocity;

	bool flight_ready;
	bool is_flying;
	bool already_boarded;
	Position position;

	bool in_use;
	bool heartbeat;

	AIRPORT_ID origin_airport_id;
	AIRPORT_ID destiny_airport_id;

	CircBuffer buffer;
}Plane;