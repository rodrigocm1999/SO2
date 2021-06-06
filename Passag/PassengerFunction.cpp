#include "PassengerFunction.h"

#include <iostream>

using namespace std;


#ifdef UNICODE
#define tcout std::wcout
#define tcin std::wcin
#define tstringstream std::wstringstream
#else
#define tcout std::cout
#define tcin std::cin
#define tstringstream std::stringstream
#endif


bool read_pipe(HANDLE pipe, PassengerMessage& message) {
	bool connected = ConnectNamedPipe(pipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	if (!connected) return false;

	DWORD bytes_read;
	bool to_return = ReadFile(pipe, &message, sizeof(message),
							  &bytes_read, nullptr) && bytes_read != 0;

	return to_return;
}

DWORD WINAPI max_wait_timer(LPVOID param) {
	auto main_struct = (PassagStruct*)param;

	DWORD result = WaitForSingleObject(main_struct->stop_wait_event, main_struct->max_wait_time * 1000);
	if (result == WAIT_TIMEOUT) {

		EnterCriticalSection(&main_struct->critical_section);
		const bool still_waiting = main_struct->still_waiting;
		LeaveCriticalSection(&main_struct->critical_section);

		if (still_waiting) {
			main_struct->timeout = true;

			exit_passenger(main_struct);

			tcout << _T("Timed out") << endl;
		}

	} else if (result == WAIT_OBJECT_0) {
	} else {
		tcout << _T("Error waiting for shutdown event -> ") << GetLastError() << endl;
	}
	
	return 0;
}

DWORD WINAPI receive_control_updates(LPVOID param) {
	auto main_struct = (PassagStruct*)param;

	while (!main_struct->exit) {
		PassengerMessage message;
		if (!read_pipe(main_struct->this_pipe, message)) {
			if (GetLastError() != ERROR_OPERATION_ABORTED) {
				tcout << _T("Error reading pipe -> ") << GetLastError() << endl;
				exit_passenger(main_struct);
			}
			break;
		}

		EnterCriticalSection(&main_struct->critical_section);
		main_struct->still_waiting = false;
		SetEvent(main_struct->stop_wait_event);
		LeaveCriticalSection(&main_struct->critical_section);

		switch (message.type) {
			case PASSENGER_TYPE_BOARDED: {
				tcout << _T("Buckle up! The plane will leave soon!") << endl;
				break;
			}
			case PASSENGER_TYPE_STARTED_FLYING: {
				tcout << _T("And we have lift off!!") << endl;
				break;
			}
			case PASSENGER_TYPE_MOVED: {
				tcout << _T("The plane moved to pos : (") << message.data.pos.x << _T(", ") << message.data.pos.y << _T(")") << endl;
				break;
			}
			case PASSENGER_TYPE_PLANE_ARRIVED: {
				tcout << _T("You have arrived to your destination: ") << main_struct->destiny_port << _T("\nNow you must leave the airport") << endl;
				exit_passenger(main_struct);
				break;
			}
			case PASSENGER_TYPE_PLANE_CRASHED: {
				tcout << _T("We hope you had your seatbelt fastened... The plane is going down ... And it has crashed") << endl;
				exit_passenger(main_struct);
				break;
			}
			case PASSENGER_TYPE_CONTROL_EXITING: {
				tcout << _T("Something went wrong and you will be escorted out of the airport") << endl;
				exit_passenger(main_struct);
				break;
			}
			default: tcout << _T("Invalid type received -> ") << message.type << endl;
		}
	}
	
	return 0;
}


DWORD WINAPI receive_exit_command(LPVOID param) {
	PassagStruct* passag_struct = (PassagStruct*)param;

	while (!passag_struct->exit) {
		TSTRING str;
		tcin >> str;
		if (str == _T("exit")) {
			passag_struct->timeout = true;
			exit_passenger(passag_struct);
		}
	}
	
	return 0;
}


void exit_passenger(PassagStruct* main_struct) {
	main_struct->exit = true;
	SetEvent(main_struct->stop_wait_event);
	CancelSynchronousIo(main_struct->pipe_updates_thread);
	CancelSynchronousIo(main_struct->exit_command_thread);
}