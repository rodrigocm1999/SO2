#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <synchapi.h>
#include <iostream>
#include <string>
#include "RegistryStuff.h"
#include "Controlo.h"

#include <unordered_set>

#include "Utils.h"
#include "SharedStructContents.h"
#include "SharedPassagStruct.h"
#include "TextInterface.h"
#include "ControlMain.h"
#include "ControlFunctions.h"
#include "MainBreakdown.h"
#include "StartException.h"

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

	HANDLE process_lock_mutex = nullptr;

	try {
		process_lock_mutex = lock_process();

		ControlMain* control_main = main_start();

		tcout << _T("Max planes from registry : ") << control_main->shared_control->max_plane_amount << endl;

		startAllThreads(control_main);
		control_main->interface_thread = create_thread(enter_text_interface, control_main);

		waitForThreadsToFinish(control_main);
		WaitForSingleObject(control_main->interface_thread, INFINITE);

		exitAndSendSentiment(control_main);

		delete control_main;

	} catch (StartException* e) {
		tcout << _T("ERROR -> ") << e->get_message() << endl;
		delete e;
	}
	

	CloseHandle(process_lock_mutex);

	//TODO Interface com utilizador: interface gráfica Win32 que apresenta todo o espaço aéreo e os seus elementos. Os 
	//	aeroportos e os aviões são representados graficamente de forma distinta.Esta informação estará permanentemente visível e sempre atualizada
}
