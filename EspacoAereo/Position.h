#pragma once
class Position {
public:

	int x;
	int y;

	Position();
	Position(int x, int y);

	int grid_distance(const Position & other);
	float distance(const Position& other);
};
