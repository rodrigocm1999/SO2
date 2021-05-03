#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <string>
#include <vector>
#include "SO2_TP_DLL_2021.h"
#include "SharedStructContents.h"
#include "Utils.h"
#include "CircularBuffer.h"
#include "PlaneMain.h"
#include "TextInterface.h"

using namespace std;

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tcout std::wcout
#define tcin std::wcin
#define tstringstream std::wstringstream
#else
#define tcout std::cout
#define tcin std::cin
#define tstringstream std::stringstream
#endif

void exit_everything(PlaneMain* plane_main);

DWORD WINAPI receive_updates(LPVOID param) {
	PlaneMain* plane_main = (PlaneMain*)param;

	while (1) {
		const PlaneControlMessage message = plane_main->receiving_buffer->get_next_element();

		switch (message.type) {
		case TYPE_START_TRIP:
			break;
		case TYPE_CONTROL_EXITING:
			tcout << _T("Control Exiting\n");
			plane_main->exit = true;
			exit_everything(plane_main);
			break;
		case TYPE_PLANE_NOT_ALLOWED:
			tcout << _T("Indicated airport doesn't exist\n");
			plane_main->exit = true;
			exit_everything(plane_main);
			break;
		default:
			tcout << _T("Invalid type received from control :") << message.type << endl;
		}
	}
}

DWORD WINAPI heartbeat(LPVOID param) {
	bool* b = (bool*)param;
	*b = true;
	return -1;
}

void exit_everything(PlaneMain* plane_main) {
	tcout << _T("Exiting\n-----------------------------");

	plane_main->this_plane->in_use = false;
	PlaneControlMessage message;
	message.plane_offset = plane_main->this_plane->offset;
	message.type = TYPE_PLANE_LEAVES;
	plane_main->control_buffer->set_next_element(message);

	UnmapViewOfFile(plane_main->shared_control);
	CloseHandle(plane_main->handle_mapped_file);
	CloseHandle(plane_main->receiving_thread);
	ReleaseSemaphore(plane_main->semaphore_plane_counter, 1, nullptr);

	delete(plane_main);

	exit(0);
}

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
				memcpy(this_plane->origin, starting_port.c_str(), sizeof(TCHAR) * (starting_port.size() + 1));

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

	PlaneControlMessage message;
	message.plane_offset = plane_main->this_plane->offset;
	message.type = TYPE_NEW_PLANE;
	memcpy(message.data.airport_name, starting_port.c_str(), sizeof(TCHAR) * (starting_port.size() + 1));
	plane_main->control_buffer->set_next_element(message);


	//TODO heartbeat
	int dsa = 2;
	int* asd = &dsa;
	//move(1, 2, 3, 4, asd, asd); // TODO fix this

	enter_text_interface_plane(plane_main);

	exit_everything(plane_main);

	//TODO Interação / Comandos:
	//	Definir o próximo destino(depois de iniciada a viagem este não pode ser alterado).
	//	Embarcar passageiros(todos os que estiverem nesse aeroporto para esse destino) e que caibam no avião.
	//	Iniciar viagem(pode iniciar viagem sem embarcar ninguém).
	//	Quando em viagem, o piloto não pode fazer nada a não ser terminar o programa.
	//	O piloto pode terminar o programa a qualquer altura
	//		Se o fizer a meio do voo, considera-se que houve um acidente e os passageiros perdem-se.
	//		Se o fizer num aeroporto, considera-se que o piloto se reformou.
	//		Em ambos os casos, o avião deixa de existir no sistema.


	//TODO Funcionalidades principais:
	//	Movimenta-se no espaço aéreo, partindo sempre de um aeroporto em direção a outro.A lógica desta
	//		movimentação não é aqui apresentada, já que será fornecida uma DLL com a sua implementação feita
	//	Quando chega a um novo aeroporto, deve comunicar a sua chegada ao controlador aéreo.

}