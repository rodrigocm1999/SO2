#include "Passenger.h"

#include <sstream>

#ifdef UNICODE
#define tstringstream std::wstringstream
#else
#define tstringstream std::stringstream
#endif


Passenger::Passenger(const DWORD id, Airport* origin, Airport* destiny, const TSTRING& name)
	: id(id), origin(origin), destiny(destiny), name(name) {
	
	TSTRING pipe_name = PIPE_NAME_PREFIX;

	tstringstream oss;
	oss << id;
	pipe_name.append(oss.str());

	pipe = CreateFile(pipe_name.c_str(), GENERIC_WRITE, 0, nullptr,
					  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (pipe == INVALID_HANDLE_VALUE)
		throw std::exception();

}

Passenger::~Passenger() {
	CloseHandle(pipe);
}

bool Passenger::send_message(PassengerMessage& message) {
	DWORD bytes_written;
	return WriteFile(pipe, &message, sizeof(message), &bytes_written, nullptr) && bytes_written != 0;
}

bool Passenger::send_message(int type) {
	PassengerMessage message;
	message.type = type;
	return send_message(message);
}
