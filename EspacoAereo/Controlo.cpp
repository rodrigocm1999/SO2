#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <synchapi.h>
#include <iostream>
#include <string>
#include "RegistryStuff.h"
#include "Controlo.h"
#include "Utils.h"
#include "SharedStructContents.h"
#include "TextInterface.h"
#include "ControlMain.h"
#include "ControlFunctions.h"

using namespace std;

#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif


int _tmain(int argc, TCHAR** argv) {

#ifdef UNICODE
	int val = _setmode(_fileno(stdin), _O_WTEXT);
	val = _setmode(_fileno(stdout), _O_WTEXT);
	val = _setmode(_fileno(stderr), _O_WTEXT);
#endif

	//Check if is already running --------------------------------------------
	HANDLE process_lock_mutex = CreateMutexW(0, FALSE, _T("Airport_Control"));
	// Tries to create a mutex with the specified name
	// If the application is already running it cant create another mutex with the same name
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		tcout << _T("Process already running\n");
		return -1;
	}
	// -----------------------------------------------------------------------


	//Get max planes amount from registry ------------------------------------
	int max_planes = get_max_planes_from_registry();
	tcout << _T("Max planes from registry : ") << max_planes << endl;
	//Create Semaphore that control max planes at a moment
	HANDLE planes_semaphore = CreateSemaphoreW(nullptr, max_planes, max_planes, SEMAPHORE_MAX_PLANES);
	if (planes_semaphore == nullptr) {
		tcout << _T("Semaphore already exists -> ") << GetLastError() << endl;
		return -1;
	}
	// -----------------------------------------------------------------------


	//Create Shared Memory ---------------------------------------------------
	const DWORD shared_memory_size = sizeof(SharedControl) + sizeof(Plane) * max_planes; //Soma do espaço necessário a alocar

	HANDLE handle_mapped_file;
	void* shared_mem_pointer = allocate_shared_memory(handle_mapped_file, shared_memory_size);
	if (shared_mem_pointer == NULL) {
		return -1;
	}
	//------------------------------------------------------------------------------------

	ControlMain* control_main = nullptr;
	{
		SharedControl* shared_control = (SharedControl*)shared_mem_pointer;
		Plane* planes_start = (Plane*)&shared_control[1];


		shared_control->max_plane_amount = max_planes;
		control_main = new ControlMain(shared_control, planes_start, handle_mapped_file);

		memset(control_main->shared_control->map, MAP_EMPTY, sizeof(control_main->shared_control->map));
	}
	

	// Start all threads -----------------------------------------------------------------
	control_main->receiving_thread = create_thread(receive_updates, control_main);
	control_main->heartbeat_thread = create_thread(heartbeat_checker, control_main);
	control_main->interface_thread = create_thread(enter_text_interface, control_main);
	//------------------------------------------------------------------------------------


	// Wait for them to finish -----------------------------------------------------------
	const int handle_amount = 3;
	HANDLE handles[] = { control_main->receiving_thread ,control_main->heartbeat_thread ,control_main->interface_thread };
	WaitForMultipleObjects(handle_amount, handles, true, INFINITE);
	//------------------------------------------------------------------------------------

	
	//Exiting ----------------------------------------------------------------------------
	tcout << _T("Exiting\n-----------------------------");

	for (int i = 0; i < control_main->shared_control->max_plane_amount; ++i) {
		if (control_main->planes[i].in_use) {
			CircularBuffer* buffer = control_main->get_plane_buffer(i);

			PlaneControlMessage message;
			message.type = TYPE_CONTROL_EXITING;
			buffer->set_next_element(message);
		}
	}
	delete(control_main);
	//------------------------------------------------------------------------------------


	//TODO Interface com utilizador: interface gráfica Win32 que apresenta todo o espaço aéreo e os seus elementos. Os 
	//	aeroportos e os aviões são representados graficamente de forma distinta.Esta informação estará permanentemente visível e sempre atualizada
}
