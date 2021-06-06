#include "StartException.h"

StartException::StartException(tstringstream* stream) {
	error_string = stream->str();
}

StartException::StartException(const TCHAR* str) {
	error_string = str;
}

const TCHAR* StartException::get_message() {
	return error_string.c_str();
}
