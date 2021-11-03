//This is free and unencumbered software released into the public domain.

//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non-commercial, and by any
//means.

//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain. We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors. We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.

//For more information, please refer to <http://unlicense.org/>


#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <string.h> 

using namespace std;

#define MSG 1000

class TinyParser {

public:
	TinyParser() {
		data.clear();
		str = new char[MSG];
	}
	TinyParser(vector<pair<string, string>> _data) : data(_data) {
		str = new char[MSG];
	}

	TinyParser(char* str) {
		Parse(str);
	}

	TinyParser& operator=(vector<pair<string, string>> _data) {

		data.clear();
		data.resize(_data.size());
		for (auto r = 0; r < _data.size(); ++r) {
			data[r] = _data[r];
		}
		return *this;
	}
	vector<pair<string, string>> GetData() const {
		return data;
	}

	int Parse(char* str) {

		data.clear();
		if (str == nullptr) return 0;
		int yes = 1;
		string s;
		char * tokens = strtok(str, " {},:");
		while (tokens != nullptr) {
			s = tokens;
			tokens = strtok(nullptr, " {},:");
			data.push_back({ s, tokens });
			yes = yes && tokens;
			tokens = strtok(nullptr, " {},:");
		}
		return yes;
	}
	char* UnParse() {

		if (data.size() == 0) return nullptr;
		string f = "{";
		for (auto r = 0; r < data.size(); ) {
			f += data[r].first + ":" + data[r].second;
			++r;
			if (r == data.size()) {
				f += '}';
				break;
			}
			else f += ',';
		}
		f += '\0';
		memset(str, 0, f.length() + 1);
		strcpy(str, f.c_str());
		return str;
	}

	~TinyParser() {
		delete(str);
	}
private:
	vector<pair<string, string>> data;
	char* str;
};
