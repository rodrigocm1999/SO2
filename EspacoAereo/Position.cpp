#include "Position.h"

#include <math.h>

Position::Position(int x, int y){
	this->x = x;
	this->y = y;
}

int Position::grid_distance(const Position& other)
{
	return  abs(this->x - other.x) + abs(this->y - other.y);
}

float Position::distance(const Position& other)
{
	float dx = x - other.x;
	float dy = y - other.y;
	return (float)sqrt(dx * dx + dy * dy);
}

Position::Position() : Position(0, 0){
}
