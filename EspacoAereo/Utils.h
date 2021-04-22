#pragma once
#include <vector>
#include <string>

#ifdef UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif


std::vector<tstring> stringSplit(const tstring& oriStr, const tstring delimiter) {
	std::vector<tstring> list;
	tstring str(oriStr);
	size_t pos;
	tstring token;
	while ((pos = str.find(delimiter)) != tstring::npos) {
		token = str.substr(0, pos);
		list.push_back(token);
		str.erase(0, pos + delimiter.length());
	}
	list.push_back(str);
	return list;
}
