#include "CircularBuffer.h"

#include <iostream>

template<typename T>
CircularBuffer<T>::CircularBuffer(struct CircBuffer<T>* data) {
	mutex_get = CreateMutex(NULL, FALSE, NULL);
	mutex_set = CreateMutex(NULL, FALSE, NULL);//TODO put names on these shits
	mutex_empty_spot = CreateMutex(NULL, FALSE, NULL);
	mutex_available_item = CreateMutex(NULL, FALSE, NULL);

	if (mutex_get == NULL || mutex_set == NULL || mutex_empty_spot == NULL || mutex_available_item == NULL) {
		printf("CreateMutex error: %d\n", GetLastError());
		throw std::exception();
	}

	this->data = data;
}

template<typename T>
CircularBuffer<T>::~CircularBuffer() {
	CloseHandle(mutex_get);
	CloseHandle(mutex_set);
	CloseHandle(mutex_empty_spot);
	CloseHandle(mutex_available_item);
}


template<typename T>
T CircularBuffer<T>::get_next_element() {

	WaitForSingleObject(mutex_available_item, INFINITE);
	WaitForSingleObject(mutex_get, INFINITE);

	T item = data->buffer[data->out];
	data->out = (data->out + 1) % data->buffer_size;

	ReleaseMutex(mutex_get);
	ReleaseMutex(mutex_empty_spot);

	return item;
}

template<typename T>
void CircularBuffer<T>::set_next_element(T& element) {

	WaitForSingleObject(mutex_empty_spot, INFINITE);
	WaitForSingleObject(mutex_set, INFINITE);

	data->buffer[data->in] = element;
	data->in = (data->in + 1) % data->buffer_size;

	ReleaseMutex(mutex_set);
	ReleaseMutex(mutex_available_item);
}