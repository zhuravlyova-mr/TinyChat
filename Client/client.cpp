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

//CLient
//Command args: <port> <IP-address>
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include "parser.h"

using namespace std;

#pragma comment (lib, "ws2_32.lib")
#define MSG 1000

SOCKET cli_sock;

HANDLE thr; //thread handle
DWORD thrID;

string uuid, login;
TinyParser d;
TinyParser sender;
DWORD client_id;
DWORD id;
int flag; //is_logout

vector<pair<string, string>> make_message(int port, int& flag) {

	string str, str1, str2;
	vector<pair<string, string>> message;
	message.clear();
	cin >> str;
	message.push_back({ "\"id\"", to_string(cli_sock) + to_string(thrID) });
	if (str == "HELLO") {
		message.push_back({ "\"command\"", "\"HELLO\"" });
	}
	else if (str == "login") {
		message.push_back({ "\"command\"", "\"login\"" });
		cout << "login: ";
		cin >> login;
		message.push_back({ "\"login\"", "\"" + login + "\"" });
		cout << "password: ";
		cin >> str1;
		message.push_back({ "\"password\"", "\"" + str1 + "\"" });
	}
	else if (str == "message") {
		message.push_back({ "\"command\"", "\"message\"" });
		cout << "body: ";
		cin >> str2;
		message.push_back({ "\"body\"", str2 });
		message.push_back({ "\"sender_login\"", "\"" + login + "\"" });
		message.push_back({ "\"session\"", uuid });
	}
	else if (str == "ping") {
		message.push_back({ "\"command\"", "\"ping\"" });
		message.push_back({ "\"session\"", uuid });
	}
	else if (str == "logout") {
		message.push_back({ "\"command\"", "\"logout\"" });
		message.push_back({ "\"session\"", uuid });
		flag = 1;
	}
	else {
		message.clear();
		return message;
	}
	return message;
}

DWORD WINAPI from_server(PVOID arg) {

	char* msg = new char[MSG];
	char* msg_to_parse = new char[MSG];
	char* serv_repl = nullptr;
	vector<pair<string, string>> message;
	int session = 0;

	while (1)
	{
		memset(msg, '\0', MSG);
		memset(msg_to_parse, '\0', MSG);
		int numbytes = recv(cli_sock, msg, MSG, 0);

		if (!numbytes or numbytes == SOCKET_ERROR) {
			cout << "\nÑlient leaves (may be server not found).\n";
			CloseHandle(thr);
			closesocket(cli_sock);
			break;
		}
		else if (numbytes > 0) {
			strcpy(msg_to_parse, msg);
			d.Parse(msg_to_parse);
			message = d.GetData();
			if (!session && message.size() >= 4 && message[3].first == "\"session\"") {
				uuid = message[3].second; session = 1;
			}
			cout << msg << "\n";
			if (flag) break;
			cout << "command: ";
		}
	}
	return 0;
}

int start_client(int argc, char* argv[]) {

	struct  sockaddr_in addr;
	WSADATA ws;
	id = time(0);
	char* mess = nullptr;
	int last_len = MSG;
	WSAStartup(MAKEWORD(2, 2), &ws);
	int port = atoi(argv[1]);
	uuid = "\"\"";
	login = "\"\"";
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(argv[2]);

	cli_sock = socket(AF_INET, SOCK_STREAM, 0);

	connect(cli_sock, (struct sockaddr *)&addr, sizeof(addr));

	thr = CreateThread(NULL, 0, from_server, NULL, 0, &thrID);
	cout << "Client " << to_string(cli_sock) + to_string(thrID) << "\n";

	int res = 0;
	client_id = 0;

	while (1) {
		try {
			sender = make_message(port, flag);
			mess = sender.UnParse();
			if (mess == nullptr) {
				cout << "\ncommand: ";
				continue;
			}
			last_len = strlen(mess);
			send(cli_sock, mess, last_len, 0);
			++client_id;
			if (WaitForSingleObject(thr, 1) == WAIT_OBJECT_0) break; //when logout thread becomes signaled
		}
		catch (...) {
			cout << "Unknown command\n";
		}
	}
	shutdown(cli_sock, 2);
	WSACleanup();
	fflush(stdin);
	getchar();
	return 0;
}

int main(int argc, char *argv[]) {

	start_client(argc, argv);
	return 0;
}
