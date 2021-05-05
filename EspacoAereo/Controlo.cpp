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


bool exit_bool = false;

BOOL WINAPI console_handler(DWORD dwType) {
	switch (dwType) {
	case CTRL_C_EVENT:
		tcout << _T("Ctrl + C\n");
		exit_bool = true;
		//TODO make da ting go down when it should
		break;
	case CTRL_BREAK_EVENT:
		tcout << _T("Ctrl + break\n");
		break;
	default:
		tcout << _T("Non Defined Event\n");
	}
	return false;
}



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


	if (!SetConsoleCtrlHandler(console_handler, true)) {
		tcout << _T("Unable to set console handler!\n");
		return -1;
	}

	//Create Shared Memory ---------------------------------------------------
	const DWORD shared_memory_size = sizeof(SharedControl) + sizeof(Plane) * max_planes; //Soma do espaço necessário a alocar

	HANDLE handle_mapped_file;
	void* shared_mem_pointer = allocate_shared_memory(handle_mapped_file, shared_memory_size);
	if (shared_mem_pointer == NULL) {
		return -1;
	}


	ControlMain* control_main = nullptr;
	{
		SharedControl* shared_control = (SharedControl*)shared_mem_pointer;
		Plane* planes_start = (Plane*)&shared_control[1];


		shared_control->max_plane_amount = max_planes;
		control_main = new ControlMain(shared_control, planes_start, handle_mapped_file);

		memset(control_main->shared_control->map, MAP_EMPTY, sizeof(control_main->shared_control->map));
	}


	control_main->receiving_thread = create_thread(receive_updates, control_main);
	control_main->heartbeat_thread = create_thread(heartbeat_checker, control_main);


	enter_text_interface(control_main, &exit_bool);

	exit_everything(control_main);

	//TODO Pode assumir que existe um número máximo tanto de aeroportos como de aviões. 
	// Estas quantidades deverão estar definidas no Registry.Quando os valores máximos são atingidos, os
	// novos aviões que se tentem ligar ao sistema serão ignorados até que este tenha espaço para eles.

	//TODO Interface com utilizador: interface gráfica Win32 que apresenta todo o espaço aéreo e os seus elementos. Os 
	//	aeroportos e os aviões são representados graficamente de forma distinta.Esta informação estará permanentemente visível e sempre atualizada
	//		
	//		TODO Numa versão inicial (Meta 1) será aceite uma versão simplificada da interface segundo o 
	//			paradigma consola.Não é para desenhar uma matriz, mas sim para apresentar mensagens de texto tais como “avião ID na posição x, y”.

	//TODO O controlador aéreo considerará que um avião que não dá sinal de vida durante 3 segundos deixou de existir
	//	quer esteja em voo ou parado no aeroporto.O avião é identificado perante o controlador aéreo através do seu ID de processo

	//TODO Comandos:
	//	Encerrar todo o sistema (todas as aplicações são notificadas).
	//	Suspender / ativar a aceitação de novos aviões por parte dos utilizadores.
	//	Listar todos os aeroportos, aviões e passageiros existentes no sistema(com os detalhes de cada um, por	exemplo, destino, no caso dos passageiros).
}
