#pragma once

#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <vector>
#include <string>
#include "Utils.h"

using namespace std;

#ifdef UNICODE
#define tcout wcout
#define tcin wcin
#define tstring wstring
#define tstringstream wstringstream
#else
#define tcout cout
#define tcin cin
#define tstring string
#define tstringstream stringstream
#endif


void start_interface() {
	while (1) {
		tcout << _T("Menu -------------\n");
		tstring input;
		tcin >> input;
		vector<tstring> input_parts = stringSplit(input, _T(" "));
		auto command = input_parts[0];

		if (command == _T("new_airport")) {

		}
		else if (command == _T("accept")) { // WTF -> Suspender / ativar a aceitação de novos aviões por parte dos utilizadores.

		}
		else if (command == _T("list")) {
			if (input_parts.size() == 2) {

			}
			else
				tcout << "Invalid Syntax -> list (airports|planes|passangers)\n";
		}
		else if (command == _T("exit")) {

		}
	}
}