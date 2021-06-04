#include "StartException.h"

StartException::StartException(tstringstream* stream) {
	str = stream->str();
}

const TCHAR* StartException::get_message() {
	return str.c_str();
}