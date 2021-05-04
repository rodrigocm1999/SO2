#include "Airport.h"


Airport::Airport(int id, const TCHAR* name, int x, int y) : name(name), id(id) {
	position.x = x;
	position.y = y;
}
