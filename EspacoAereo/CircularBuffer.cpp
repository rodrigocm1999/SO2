#include "CircularBuffer.h"
#include "Utils.h"

#include <tchar.h>
#include <sstream>


#ifdef UNICODE
#define tstringstream std::wstringstream
#else
#define tstringstream std::stringstream
#endif

CircularBuffer::CircularBuffer(CircBuffer* data, int mutex_number) {

	tstringstream stream;
	stream << mutex_number;
	TSTRING prefix = stream.str();
	
	mutex_get = CreateMutex(nullptr, FALSE, (prefix + MUTEX_GET).c_str());
	mutex_set = CreateMutex(nullptr, FALSE, (prefix + MUTEX_SET).c_str());
	mutex_empty_spot = CreateSemaphore(nullptr, CIRC_BUFFER_SIZE, CIRC_BUFFER_SIZE, (prefix + MUTEX_EMPTY_SPOT).c_str());
	mutex_available_item = CreateSemaphore(nullptr, 0, CIRC_BUFFER_SIZE, (prefix + MUTEX_AVAILABLE_ITEM).c_str());

	
	if (mutex_get == nullptr || mutex_set == nullptr || mutex_empty_spot == nullptr || mutex_available_item == nullptr) {
		printf("CreateMutex error: %lu\n", GetLastError());
		throw std::exception();
	}

	this->data = data;
}


CircularBuffer::~CircularBuffer() {
	CloseHandle(mutex_get);
	CloseHandle(mutex_set);
	CloseHandle(mutex_empty_spot);
	CloseHandle(mutex_available_item);
}


PlaneControlMessage CircularBuffer::get_next_element() {

	WaitForSingleObject(mutex_available_item, INFINITE);
	WaitForSingleObject(mutex_get, INFINITE);

	PlaneControlMessage item = data->buffer[data->out];
	data->out = (data->out + 1) % CIRC_BUFFER_SIZE;

	ReleaseMutex(mutex_get);
	ReleaseSemaphore(mutex_empty_spot, 1, nullptr);

	return item;
}

void CircularBuffer::set_next_element(PlaneControlMessage& element) {

	WaitForSingleObject(mutex_empty_spot, INFINITE);
	WaitForSingleObject(mutex_set, INFINITE);

	data->buffer[data->in] = element;
	data->in = (data->in + 1) % CIRC_BUFFER_SIZE;

	ReleaseMutex(mutex_set);
	ReleaseSemaphore(mutex_available_item, 1, nullptr);
}