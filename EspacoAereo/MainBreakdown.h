#pragma once

#include <Windows.h>
#include "ControlMain.h"


void waitForThreadsToFinish(ControlMain* control_main);


void exitAndSendSentiment(ControlMain* control_main);

void startAllThreads(ControlMain* control_main);

ControlMain* main_start();

HANDLE lock_process();

void* allocate_shared_memory(HANDLE& mapped_file, DWORD size);