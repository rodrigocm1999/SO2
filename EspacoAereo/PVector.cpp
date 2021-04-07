#include "PVector.h"

#include <math.h>

PVector::PVector(int x, int y){
	this->x = x;
	this->y = y;
}

int PVector::grid_distance(const PVector& other)
{
	return  abs(this->x - other.x) + abs(this->y - other.y);
}

float PVector::distance(const PVector& other)
{
	float dx = x - other.x;
	float dy = y - other.y;
	return (float)sqrt(dx * dx + dy * dy);
}

PVector::PVector() : PVector(0, 0){
}
