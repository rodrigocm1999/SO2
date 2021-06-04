#include "Utils.h"

using namespace std;

vector<TSTRING> string_split(const TSTRING& ori_str, const TSTRING delimiter) {
	vector<TSTRING> list;
	TSTRING str(ori_str);
	size_t pos;
	while ((pos = str.find(delimiter)) != TSTRING::npos) {
		TSTRING token = str.substr(0, pos);
		list.push_back(token);
		str.erase(0, pos + delimiter.length());
	}
	list.push_back(str);
	return list;
}

HANDLE create_thread(const LPTHREAD_START_ROUTINE function_to_run, const LPVOID argument) {
	return CreateThread(nullptr, 0, function_to_run, argument, 0, nullptr);
}

int grid_distance(const Position& o1, const Position& o2) {
	return  abs(o1.x - o2.x) + abs(o1.y - o2.y);
}
