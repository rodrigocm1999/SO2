#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <sstream>
#include <string>

#include "SharedPassagStruct.h"
#include "PassengerFunction.h"
#include "Utils.h"

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

#define THREADS_AMOUNT 3

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
		if (max_waiting_time_in_seconds <= 0) {
			tcout << _T("Invalid wait time") << endl;
			return 0;
		}
	}

	DWORD process_id = GetProcessId(GetCurrentProcess());

	tcout << _T("Process Id : ") << process_id << endl;

	TSTRING pipe_name(PIPE_NAME_PREFIX);
	{
		tstringstream oss;
		oss << process_id;
		pipe_name.append(oss.str());
	}

	HANDLE shutdown_event = CreateEvent(nullptr, true, false, nullptr);
	if (shutdown_event == nullptr) {
		tcout << _T("Handle creation error") << endl;
		return -1;
	}

	const HANDLE passenger_named_pipe = CreateNamedPipe(pipe_name.c_str(), PIPE_ACCESS_INBOUND,
														PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
														1, 0, sizeof(PassengerMessage), 1000, nullptr);
	if (passenger_named_pipe == INVALID_HANDLE_VALUE) {
		tcout << _T("Error creating named pipe -> ") << GetLastError() << endl;
		CloseHandle(shutdown_event);
		return -1;
	}

	HANDLE mutex_new_passenger = CreateMutex(nullptr, FALSE, MUTEX_NEW_PASSENGER);
	if (mutex_new_passenger == nullptr) {
		tcout << _T("Error creating/opening mutex -> ") << GetLastError() << endl;
		CloseHandle(shutdown_event);
		return 1;
	}

	DWORD result = WaitForSingleObject(mutex_new_passenger, 3000);
	if (result != WAIT_OBJECT_0) {
		tcout << _T("Error Waiting for mutex to connect to control -> ") << GetLastError() << endl;
		CloseHandle(passenger_named_pipe);
		CloseHandle(shutdown_event);
		return -1;
	}

	bool success = WaitNamedPipe(CONTROL_PIPE_MAIN, INFINITE);
	if (!success) {
		tcout << _T("Error Waiting for control named pipe -> ") << GetLastError() << endl
			<< _T("Control might not be running") << endl;
		CloseHandle(passenger_named_pipe);
		CloseHandle(shutdown_event);
		return -1;
	}

	HANDLE control_named_pipe = CreateFile(CONTROL_PIPE_MAIN, GENERIC_WRITE, 0, nullptr,
										   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (control_named_pipe == INVALID_HANDLE_VALUE) {
		tcout << _T("Error opening control named pipe -> ") << GetLastError() << endl;
		CloseHandle(passenger_named_pipe);
		CloseHandle(shutdown_event);
		return -1;
	}

	PassengerMessage message;
	message.id = process_id;
	message.type = PASSENGER_TYPE_NEW_PASSENGER;
	memcpy(message.data.new_passenger.name, person_name.c_str(), (person_name.size() + 1) * sizeof(TCHAR));
	memcpy(message.data.new_passenger.origin, origin_port.c_str(), (origin_port.size() + 1) * sizeof(TCHAR));
	memcpy(message.data.new_passenger.destiny, destiny_port.c_str(), (destiny_port.size() + 1) * sizeof(TCHAR));

	DWORD bytes_written;
	WriteFile(control_named_pipe, &message, sizeof(message), &bytes_written, nullptr);

	if (!read_pipe(passenger_named_pipe, message)) {
		tcout << _T("Error connecting to pipe reader -> ") << GetLastError() << endl;
		CloseHandle(passenger_named_pipe);
		CloseHandle(control_named_pipe);
		CloseHandle(shutdown_event);
		exit(-1);
	}

	ReleaseMutex(mutex_new_passenger);

	PassagStruct passag_struct;
	passag_struct.exit = false;
	passag_struct.control_pipe = control_named_pipe;
	passag_struct.this_pipe = passenger_named_pipe;
	passag_struct.destiny_port = destiny_port;
	passag_struct.origin_port = origin_port;
	passag_struct.max_wait_time = max_waiting_time_in_seconds;
	passag_struct.stop_wait_event = shutdown_event;
	passag_struct.pipe_updates_thread = nullptr;
	passag_struct.timeout = false;
	InitializeCriticalSection(&passag_struct.critical_section);

	int amount_of_threads = THREADS_AMOUNT;
	HANDLE handles[THREADS_AMOUNT] = { nullptr };

	if (message.type == PASSENGER_TYPE_GOOD_AIRPORTS) {
		tcout << _T("You are now waiting to be boarded on the plane \nWrite 'exit' to leave") << endl;

		passag_struct.pipe_updates_thread = CreateThread(nullptr, 0, receive_control_updates, &passag_struct, 0, nullptr);
		passag_struct.exit_command_thread = CreateThread(nullptr, 0, receive_exit_command, &passag_struct, 0, nullptr);

		handles[0] = passag_struct.pipe_updates_thread;
		handles[1] = passag_struct.exit_command_thread;

		if (passag_struct.max_wait_time >= 0) {
			passag_struct.timeout_thread = CreateThread(nullptr, 0, max_wait_timer, &passag_struct, 0, nullptr);
			handles[2] = passag_struct.timeout_thread;
		} else {
			amount_of_threads--;
		}

	} else if (message.type == PASSENGER_TYPE_BAD_AIRPORTS) {
		tcout << _T("Invalid airports!") << endl;
	} else {
		tcout << _T("Invalid Message from control! type -> ") << message.type << endl;
	}

	const DWORD wait_result = WaitForMultipleObjects(amount_of_threads, handles, true, INFINITE);
	if (wait_result == WAIT_TIMEOUT || wait_result == WAIT_ABANDONED || wait_result == WAIT_FAILED)
		tcout << _T("Error waiting for threads to finish -> ") << GetLastError() << endl;

	// In case of timeout ------------------------------------------------------------------------------------------
	if (passag_struct.timeout) {
		DWORD result = WaitForSingleObject(mutex_new_passenger, 3000);
		if (result != WAIT_OBJECT_0) {
			tcout << _T("Error Waiting for mutex to connect to control -> ") << GetLastError() << endl;
		} else {

			HANDLE control_named_pipe = CreateFile(CONTROL_PIPE_MAIN, GENERIC_WRITE, 0, nullptr,
												   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (control_named_pipe == INVALID_HANDLE_VALUE) {
				tcout << _T("Error opening control named pipe to send PASSENGER_TYPE_GAVE_UP message -> ") << GetLastError() << endl;
			} else {
				message.id = process_id;
				message.type = PASSENGER_TYPE_GAVE_UP;
				if (!WriteFile(control_named_pipe, &message, sizeof(message), &bytes_written, nullptr)) {
					tcout << _T("Error sending timeout alert to control -> ") << GetLastError() << endl;
				}
			}
			tcout << _T("Person Gave up waiting for a plane") << endl;
		}
		ReleaseMutex(mutex_new_passenger);
	} else {
		tcout << _T("Exiting...") << endl;
	}
	//---------------------------------------------------------------------------------------------------------------

	if (!CloseHandle(control_named_pipe)) {
		tcout << _T("Error closing the control named pipe -> ") << GetLastError() << endl;
	}
	if (!CloseHandle(passenger_named_pipe)) {
		tcout << _T("Error closing the passenger named pipe -> ") << GetLastError() << endl;
	}
	CloseHandle(passag_struct.pipe_updates_thread);
	CloseHandle(passag_struct.exit_command_thread);
	CloseHandle(passag_struct.timeout_thread);
	CloseHandle(mutex_new_passenger);
	CloseHandle(shutdown_event);
}