#include "CircularBuffer.h"

#include <iostream>

template<typename T>
CircularBuffer<T>::CircularBuffer(int size_in_elements) {
	mutex_get = CreateMutex(NULL, FALSE, NULL);
	mutex_set = CreateMutex(NULL, FALSE, NULL);
	mutex_empty_spot = CreateMutex(NULL, FALSE, NULL);
	mutex_available_item = CreateMutex(NULL, FALSE, NULL);

	if (mutex_get == NULL || mutex_set == NULL || mutex_empty_spot == NULL || mutex_available_item == NULL) {
		printf("CreateMutex error: %d\n", GetLastError());
		throw std::exception();
	}

	this->buffer_size = size_in_elements;
	this->buffer = malloc(sizeof(T) * buffer_size); //TODO cant do this shit
}
//TODO change the mutexeress to the right place as in this moment they are 
// created in the process they are called from and stay with pointers in sharred memorry and erros

template<typename T>
CircularBuffer<T>::CircularBuffer() : CircularBuffer(10) {}

template<typename T>
CircularBuffer<T>::~CircularBuffer() {
	CloseHandle(mutex_get);
	CloseHandle(mutex_set);
	CloseHandle(mutex_empty_spot);
	CloseHandle(mutex_available_item);
	free(buffer);
}


template<typename T>
T CircularBuffer<T>::get_next_element() {

	WaitForSingleObject(mutex_available_item, INFINITE);
	WaitForSingleObject(mutex_get, INFINITE);

	T item = buffer[out];
	out = (out + 1) % buffer_size;

	ReleaseMutex(mutex_get);
	ReleaseMutex(mutex_empty_spot);

	return item;
}

template<typename T>
void CircularBuffer<T>::set_next_element(T &element) {

	WaitForSingleObject(mutex_empty_spot, INFINITE);
	WaitForSingleObject(mutex_set, INFINITE);

	buffer[in] = element;
	in = (in + 1) % buffer_size;

	ReleaseMutex(mutex_set);
	ReleaseMutex(mutex_available_item);
}