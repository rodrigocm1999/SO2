#include "CircularBuffer.h"
#include "Utils.h"

#include <tchar.h>
#include <iostream>


#define join_tchars(buffer, prefix, suffix) _sntprintf_s(buffer, BUFFER_SIZE - 1, BUFFER_SIZE, _T("%s%s"), prefix, suffix)

CircularBuffer::CircularBuffer(CircBuffer* data, const TCHAR* mutex_prefix) {

	TCHAR temp[BUFFER_SIZE];

	join_tchars(temp, mutex_prefix, MUTEX_GET);
	mutex_get = CreateMutex(nullptr, FALSE, temp);

	join_tchars(temp, mutex_prefix, MUTEX_SET);
	mutex_set = CreateMutex(nullptr, FALSE, temp);

	join_tchars(temp, mutex_prefix, MUTEX_EMPTY_SPOT);
	mutex_empty_spot = CreateSemaphore(nullptr, CIRC_BUFFER_SIZE, CIRC_BUFFER_SIZE, temp);

	join_tchars(temp, mutex_prefix, MUTEX_AVAILABLE_ITEM);
	mutex_available_item = CreateSemaphore(nullptr, 0, CIRC_BUFFER_SIZE, temp);

	if (mutex_get == nullptr || mutex_set == nullptr || mutex_empty_spot == nullptr || mutex_available_item == nullptr) {
		printf("CreateMutex error: %lu\n", GetLastError());
		throw std::exception();
	}

	this->data = data;
}


CircularBuffer::~CircularBuffer() {
	CloseHandle(mutex_get);
	CloseHandle(mutex_set); // TODO se fizer close será que rebenta com o outro???????
	CloseHandle(mutex_empty_spot);
	CloseHandle(mutex_available_item);
}


PlaneControlMessage CircularBuffer::get_next_element() {

	WaitForSingleObject(mutex_available_item, INFINITE);
	WaitForSingleObject(mutex_get, INFINITE);

	PlaneControlMessage item = data->buffer[data->out];
	data->out = (data->out + 1) % CIRC_BUFFER_SIZE;

	ReleaseMutex(mutex_get);
	ReleaseSemaphore(mutex_empty_spot,1 ,nullptr);

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