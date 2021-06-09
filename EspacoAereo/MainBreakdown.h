#pragma once

#include <Windows.h>
#include "ControlMain.h"


void wait_for_threads_to_finish(ControlMain* control_main);


void exit_and_send_sentiment(ControlMain* control_main);

void start_all_threads(ControlMain* control_main);

ControlMain* main_start();

HANDLE lock_process();

void* allocate_shared_memory(HANDLE& mapped_file, DWORD size);