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

	TinyParser(char* str) {
		Parse(str);
	}
	~TinyParser() {
		delete(str);
	}
private:
	vector<pair<string, string>> data;
	char* str;
};