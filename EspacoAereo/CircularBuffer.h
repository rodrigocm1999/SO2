#pragma once
#include<Windows.h>

// TODO test all of this

template <typename T>
class CircularBuffer {
	
	T* buffer;
	unsigned int buffer_size;
	unsigned int in;
	unsigned int out;

	HANDLE mutex_get;
	HANDLE mutex_set;
	HANDLE mutex_empty_spot;
	HANDLE mutex_available_item;

public:

	CircularBuffer(int size_in_elements);
	CircularBuffer();
	~CircularBuffer();

	T get_next_element();
	void set_next_element(T &element);

};

