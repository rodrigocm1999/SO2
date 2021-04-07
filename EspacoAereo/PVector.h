#pragma once
class PVector {
public:

	int x;
	int y;

	PVector();
	PVector(int x, int y);

	int grid_distance(const PVector & other);
	float distance(const PVector& other);
};
