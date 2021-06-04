#pragma once

#include <Windows.h>

#include "Airport.h"
#include "SharedStructContents.h"

void draw_plane(HBITMAP h_bitmap, HDC dc, const Position& pos);

void draw_airport(HBITMAP h_bitmap, HDC dc, const Position& pos);