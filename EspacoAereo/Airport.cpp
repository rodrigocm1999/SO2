#include "Airport.h"


Airport::Airport(const TCHAR* name, int x, int y) : name(name) {
	position.x = x;
	position.y = y;
}
