#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <string>
#include <vector>
#include "SharedStructContents.h"
#include "Utils.h"
#include "CircularBuffer.h"
#include "PlaneMain.h"
#include "TextInterface.h"
#include "PlaneFunctions.h"

using namespace std;

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tcout std::wcout
#define tcin std::wcin
#else
#define tcout std::cout
#define tcin std::cin
#endif


int _tmain(int argc, TCHAR** argv) {

#ifdef UNICODE
	int val = _setmode(_fileno(stdin), _O_WTEXT);
	val = _setmode(_fileno(stdout), _O_WTEXT);
	val = _setmode(_fileno(stderr), _O_WTEXT);
#endif

	if (argc < 4) {
		tcout << _T("Missing arguments\n");
		return -1;
	}
	int capacity = _ttoi(argv[1]);
	int velocity = _ttoi(argv[2]);
	TSTRING starting_port = argv[3];

	if (capacity <= 0) {
		tcout << _T("Invalid capacity (capacity > 0)\n");
		return -1;
	}
	if (velocity <= 0 || velocity >= 1000) {
		tcout << _T("Invalid velocity (0 < velocity < 1000)\n");
		return -1;
	}

	PlaneMain* plane_main = nullptr;
	{
		const HANDLE semaphore_plane_counter = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_MAX_PLANES);
		{
			if (semaphore_plane_counter == NULL) {
				tcout << _T("Error opening semaphore -> ") << GetLastError() << endl;
				return -1;
			}
			const DWORD result = WaitForSingleObject(semaphore_plane_counter, 2000);
			if (result == WAIT_TIMEOUT) {
				tcout << _T("The control is full and doesnt allow more planes\n");
				return -1;
			}
			if (result != WAIT_OBJECT_0) {
				tcout << _T("Semaphore wait error : ") << GetLastError() << endl;
				return -1;
			}
		}

		const HANDLE handle_mapped_file = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MAPPED_MEMORY_IDENTIFIER);
		if (handle_mapped_file == NULL) {
			tcout << _T("Could not open file mapping object (") << GetLastError() << _T(").\n");
			ReleaseSemaphore(semaphore_plane_counter, 1, nullptr);
			return -1;
		}
		void* shared_mem_pointer = MapViewOfFile(handle_mapped_file, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (shared_mem_pointer == NULL) {
			tcout << _T("Could not map view of file (") << GetLastError() << _T(").\n");
			CloseHandle(handle_mapped_file);
			ReleaseSemaphore(semaphore_plane_counter, 1, nullptr);
			return -1;
		}

		auto shared_control = (SharedControl*)shared_mem_pointer;
		auto planes = (Plane*)&shared_control[1];

		Plane* this_plane = nullptr;

		for (int i = 0; i < shared_control->max_plane_amount; ++i) {
			if (!planes[i].in_use) {
				this_plane = &planes[i];

				this_plane->in_use = true;
				this_plane->offset = i;
				this_plane->heartbeat = true;
				this_plane->velocity = velocity;
				this_plane->max_passengers = capacity;
				this_plane->is_flying = false;
				this_plane->destiny_airport_id = NOT_DEFINED_AIRPORT;
				this_plane->origin_airport_id = NOT_DEFINED_AIRPORT;
				break;
			}
		}

		if (this_plane == nullptr) {
			tcout << _T("Critical Error. Should never happen. Plane did not find free spot\n");
			UnmapViewOfFile(shared_mem_pointer);
			CloseHandle(handle_mapped_file);
			ReleaseSemaphore(semaphore_plane_counter, 1, nullptr);
			return -1;
		}

		plane_main = new PlaneMain(shared_control, planes, this_plane, this_plane->offset, semaphore_plane_counter, handle_mapped_file);
	}


	plane_main->receiving_thread = create_thread(receive_updates, plane_main);
	plane_main->heartbeat_thread = create_thread(heartbeat, plane_main);


	PlaneControlMessage message;
	message.plane_offset = plane_main->this_plane->offset;
	message.type = TYPE_NEW_PLANE;
	memcpy(message.data.airport_name, starting_port.c_str(), sizeof(TCHAR) * (starting_port.size() + 1));
	plane_main->control_buffer->set_next_element(message);

	enter_text_interface_plane(plane_main);

	exit_everything(plane_main);
}