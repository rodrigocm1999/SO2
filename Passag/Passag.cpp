#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <sstream>
#include <string>

#include "SharedPassagStruct.h"

using namespace std;

#define TSTRING std::basic_string<TCHAR>

#ifdef UNICODE
#define tcout wcout
#define tcin wcin
#define tstringstream wstringstream
#else
#define tcout cout
#define tcin cin
#define tstringstream stringstream
#endif

int _tmain(int argc, TCHAR** argv) {
#ifdef UNICODE
	int val = _setmode(_fileno(stdin), _O_WTEXT);
	val = _setmode(_fileno(stdout), _O_WTEXT);
	val = _setmode(_fileno(stderr), _O_WTEXT);
#endif

	if (argc < 4) {
		tcout << _T("Invalid Arguments : person_name origin_port destiny_port (optional)max_wait_time_seconds") << endl;
		return -1;
	}

	TSTRING person_name = argv[1];
	TSTRING origin_port = argv[2];
	TSTRING destiny_port = argv[3];
	int max_waiting_time_in_seconds = -1;
	if (argc > 4) {
		max_waiting_time_in_seconds = _ttoi(argv[4]);
	}

	DWORD process_id = GetProcessId(GetCurrentProcess());

	TSTRING pipe_name(_T("\\\\.\\pipe\\"));
	{
		tstringstream oss;
		oss << process_id;
		pipe_name.append(oss.str());
	}

	const HANDLE passenger_named_pipe = CreateNamedPipe(pipe_name.c_str(), PIPE_ACCESS_INBOUND,
														PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
														1, 0, sizeof(PassagControlMessage), 1000, nullptr);
	if (passenger_named_pipe == INVALID_HANDLE_VALUE) {
		tcout << _T("Error creating named pipe -> ") << GetLastError() << endl;
		return -1;
	}

	const HANDLE control_named_pipe = CreateFile(CONTROL_PIPE_MAIN, GENERIC_READ, 0, 0,
												 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (control_named_pipe == INVALID_HANDLE_VALUE) {
		tcout << _T("Error opening control named pipe -> ") << GetLastError() << endl;
		CloseHandle(passenger_named_pipe);
		return -1;
	}

	PassagControlMessage message;
	message.id = process_id;
	message.type = PASSENGER_TYPE_NEW_PASSENGER;
	memcpy(message.data.where_to.name, person_name.c_str(), person_name.size() * sizeof(TCHAR));
	memcpy(message.data.where_to.origin, origin_port.c_str(), origin_port.size() * sizeof(TCHAR));
	memcpy(message.data.where_to.destiny, destiny_port.c_str(), destiny_port.size() * sizeof(TCHAR));

	DWORD bytes_written;
	WriteFile(control_named_pipe, &message, sizeof(message), &bytes_written, nullptr);


	//TODO the menu thing part stuff


	TSTRING asd;
	tcin >> asd;


	if (!DisconnectNamedPipe(control_named_pipe)) {
		tcout << _T("Error closing the control named pipe -> ") << GetLastError() << endl;
	}
	if (!DisconnectNamedPipe(passenger_named_pipe)) {
		tcout << _T("Error closing the passenger named pipe -> ") << GetLastError() << endl;
	}

	//TODO Funcionamento: 
	//	O passageiro é atribuído ao aeroporto origem, ficando a aguardar que exista um avião disponível para o aeroporto destino. 
	//		Quando tal avião existir, o passageiro embarca automaticamente e, ao chegar ao aeroporto destino, desembarca e o programa termina.
	//		Caso tenha sido indicado um tempo de espera máximo, o passageiro desiste automaticamente de viajar se o tempo indicado passar 
	//			e não for atribuído a nenhum avião, sai do aeroporto e o programa termina.

	//TODO Interação:
	//	O utilizador é automaticamente informado de quando embarca, da posição em que está quando está em voo, e quando chega
	//	O utilizador pode sempre interagir com esta aplicação para a terminar. Se o fizer, considera - se que o passageiro deixou de existir


	//TODO Comandos: 
	//	Deve adicionar os comandos que considerar necessários para a utilização do programa tendo em vista a funcionalidade requerida
}
