#pragma once
#include<Windows.h>

template <typename T>
struct CircBuffer {
	T* buffer;
	unsigned int buffer_size;
	unsigned int in;
	unsigned int out;
};

template <typename T>
class CircularBuffer {

	struct CircBuffer<T>* data;

	HANDLE mutex_get;
	HANDLE mutex_set;
	HANDLE mutex_empty_spot;
	HANDLE mutex_available_item;

public:

	CircularBuffer(struct CircBuffer<T>* data);
	~CircularBuffer();

	T get_next_element();
	void set_next_element(T& element);

};

