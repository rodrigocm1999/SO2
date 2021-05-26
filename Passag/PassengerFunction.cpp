#include "PassengerFunction.h"

#include <iostream>

#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

using namespace std;

bool read_pipe(HANDLE pipe, PassengerMessage& message) {
	bool connected = ConnectNamedPipe(pipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	if (!connected) return false;

	DWORD bytes_read;
	bool to_return = ReadFile(pipe, &message, sizeof(message),
							  &bytes_read, nullptr) && bytes_read != 0;
	//DisconnectNamedPipe(pipe);

	return to_return;
}

DWORD WINAPI max_wait_timer(LPVOID param) {
	auto timer_thread = (TimerThread*)param;

	DWORD result = WaitForSingleObject(timer_thread->stop_wait_event, timer_thread->max_wait_time * 1000);
	if (result == WAIT_TIMEOUT) {

		EnterCriticalSection(&timer_thread->critical_section);
		const bool still_waiting = timer_thread->still_waiting;
		LeaveCriticalSection(&timer_thread->critical_section);

		if (still_waiting) {
			timer_thread->timeout = true;
			timer_thread->exit = true;

			CancelSynchronousIo(timer_thread->pipe_updates_thread);

			tcout << _T("Timed out") << endl;
		}

	} else if (result == WAIT_OBJECT_0) {
	} else {
		tcout << _T("Error waiting for shutdown event -> ") << GetLastError() << endl;
	}

	return 0;
}

DWORD WINAPI receive_control_updates(LPVOID param) {
	auto timer_thread = (TimerThread*)param;

	while (!timer_thread->exit) {
		PassengerMessage message;
		if (!read_pipe(timer_thread->this_pipe, message)) {
			if (GetLastError() != ERROR_OPERATION_ABORTED) {
				tcout << _T("Error reading pipe -> ") << GetLastError() << endl;
			}
			timer_thread->exit = true;
			continue;
		}

		EnterCriticalSection(&timer_thread->critical_section);
		timer_thread->still_waiting = false;
		SetEvent(timer_thread->stop_wait_event);
		LeaveCriticalSection(&timer_thread->critical_section);

		switch (message.type) {
			case PASSENGER_TYPE_BOARDED:
			{
				tcout << _T("Buckle up! The plane will leave soon!") << endl;
				break;
			}
			case PASSENGER_TYPE_STARTED_FLYING:
			{
				tcout << _T("And we have lift off!!") << endl;
				break;
			}
			case PASSENGER_TYPE_MOVED:
			{
				tcout << _T("The plane moved to pos : (") << message.data.pos.x << _T(", ") << message.data.pos.y <<
					_T(")") << endl;
				break;
			}
			case PASSENGER_TYPE_PLANE_ARRIVED:
			{
				tcout << _T("You have arrived to your destination: ") << timer_thread->destiny_port
					<< _T("\nNow you must leave the airport") << endl;
				timer_thread->exit = true;
				break;
			}
			case PASSENGER_TYPE_PLANE_CRASHED:
			{
				tcout << _T("We hope you had your seatbelt fastened... The plane is going down ... And it has crashed")
					<< endl;
				timer_thread->exit = true;
				break;
			}
			case PASSENGER_TYPE_CONTROL_EXITING:
			{
				tcout << _T("Something went wrong and you will be escorted out of the airport") << endl;
				timer_thread->exit = true;
				break;
			}
			default: tcout << _T("Invalid type received -> ") << message.type << endl;
		}
	}

	return 0;
}
