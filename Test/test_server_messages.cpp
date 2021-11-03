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

//Server messages test

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include "parser.h"
#include <cassert>

#pragma comment(lib,"ws2_32.lib") 

using namespace std;

#define VMAX 20
#define TINYMES 100
#define MSG 1000

SOCKET sock_h, consock;
struct sockaddr_in serv_addr, cli;
SOCKET cl_handles[VMAX] = { 0 }; //clients sockets
int cli_count;  //clients counter 
int flag_auth;
int client_id; //client's message id
int port;
TinyParser d;
TinyParser sender;

HANDLE thr[VMAX] = { nullptr };  //clients threads handles
HANDLE sender_thr[VMAX] = { nullptr };  //clients threads handles
HANDLE sem;
vector<pair<string, string>> LogsPassws = { { "\"bob\"", "\"123\"" }, { "\"paul\"", "\"456\"" }, 
                                            { "\"jim\"", "\"diam01\"" }, { "\"storm\"", "\"qw123\"" }, { "\"guest\"", "\"passw\"" } };

vector<pair<string, string>> make_answer(int port,  char* msg, int socket) {

	string str, str1, str2;
	d.Parse(msg);
	vector<pair<string, string>> message = d.GetData(); //received data
	vector<pair<string, string>> reply;                 //server reply
	reply.clear();
	if (message.size() < 2)  return reply;
	reply.push_back({ "\"id\"", message[0].second});
	if (message[1].second == "\"HELLO\"") {
		reply.push_back({ "\"command\"", "\"HELLO\"" });
		reply.push_back({ "\"auth_method\"", "\"plain-text\"" });
	}
	
	else if (message[1].second == "\"login\"") {
		reply.push_back({ "\"command\"", "\"login\"" });
		
		flag_auth = 0;
		for (size_t r = 0; r < LogsPassws.size(); ++r) 
			if (LogsPassws[r].first == message[2].second && LogsPassws[r].second == message[3].second) {
				flag_auth = 1;
				break;
			}
		
		if (flag_auth) {
			reply.push_back({ "\"status\"", "\"ok\"" });
			reply.push_back({ "\"session\"", "\""+ to_string(port) + to_string(socket) + "\"" });
		}
		else {
			reply.push_back({ "\"status\"", "\"failed\"" });
			reply.push_back({ "\"message\"", "\"No such login or password, try again.\"" });
		}
	}

	else if (message[1].second == "\"message\"") {
		if (flag_auth) {
			reply.push_back({ "\"command\"", "\"message_reply\"" });
			reply.push_back({ "\"status\"", "\"ok\"" });
			reply.push_back({ "\"client_id\"", "\"" + to_string(client_id) + "\"" });
		}
		else {
			reply.push_back({ "\"command\"", "\"message_reply\"" });
			reply.push_back({ "\"status\"", "\"failed\"" });
			reply.push_back({ "\"message\"", "\"Authentication is not passed yet.\"" });
		}
	}
	else if (message[1].second == "\"ping\"") {
		if (flag_auth) {
			reply.push_back({ "\"command\"", "\"ping_reply\"" });
			reply.push_back({ "\"status\"", "\"ok\"" });
		}
		else {
			reply.push_back({ "\"command\"", "\"ping_reply\"" });
			reply.push_back({ "\"status\"", "\"failed\"" });
			reply.push_back({ "\"message\"", "\"Authentication is not passed yet.\"" });
		}
	}
	
	else if (message[1].second == "\"logout\"") {
		reply.push_back({ "\"command\"", "\"logout\"" });
		reply.push_back({ "\"status\"", "\"ok\"" });
	}
	
	return reply;
}



DWORD WINAPI job(LPVOID arg) {
	char* message; 
	char* cl_mes = new char [MSG];
	vector<pair<string, string>> answer;
	int num = cli_count;
	client_id = 0;
	cl_handles[num] = consock;
	SOCKET my_sock = consock;
	int numbytes, n = 0, last_msg_len = MSG, last_cl_len = MSG;
	printf("Server\n");

	while (1) {
	
		memset(cl_mes, '\0', last_cl_len);
	    numbytes = recv(my_sock, cl_mes, MSG, 0);
		if (numbytes == SOCKET_ERROR || numbytes == 0) {

			closesocket(my_sock);
			my_sock = 0;
			cl_handles[num] = 0;
			break;
		}
		else if (numbytes > 0) {

			cout << cl_mes << "\n";
			last_cl_len = numbytes;		
			for (int i = 0; i <= cli_count; i++) {   //broadcast
				if (cl_handles[i] != NULL && cl_handles[i] != my_sock) {
					send(cl_handles[i], cl_mes, numbytes, 0);
				}
			}
			if (strcmp(cl_mes, "\"logout\"") == 0) n = 1;
			d = make_answer(port, cl_mes, my_sock);
			message = d.UnParse();
			if (message == nullptr) continue;
			++client_id; //number of client messages
			last_msg_len = strlen(message);
			send(my_sock, message, last_msg_len, 0); //sending reply
			if (n) {
				closesocket(my_sock);
				my_sock = 0;
				cl_handles[num] = 0;
				break;
			}
		}
	}
	return 0;
}

int start_work(int argc, char* argv[]) {
	WSADATA ws;
	WSAStartup(MAKEWORD(2, 2), &ws);
	//port = atoi(argv[1]);
	port = 84800;
	int cli_len;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	sock_h = socket(AF_INET, SOCK_STREAM, 0);
	bind(sock_h, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	listen(sock_h, VMAX);

	cout << "Server ready\n";
	cli_count = -1;
	
	while (1)
	{
		cli_len = sizeof(struct sockaddr);
		consock = accept(sock_h, (struct sockaddr*)&cli, &cli_len);
		++cli_count;
		thr[cli_count] = CreateThread(NULL, 0, job, NULL, 0, NULL);
	}
	for (int i = cli_count; i >= 0; --i) {
		CloseHandle(thr[i]);
	}
	shutdown(sock_h, 2);
	WSACleanup();
	return 0;
}

void TestMakeAnswer() {

	int port = 84800, socket = 250;
	char *msg = new char[MSG];
	int len;
	//bad login
	memset(msg, 0, MSG);
	strcpy(msg, "{\"id\":25,\"command\":\"login\",\"login\":\"first\",\"password\":\"mypassw5\"}");
	len = strlen(msg);
	vector<pair<string, string>> answer = make_answer(84800, msg, socket);
	vector<pair<string, string>> res = { {"\"id\"", "25"}, { "\"command\"", "\"login\"" },
					  { "\"status\"", "\"failed\"" }, { "\"message\"", "\"No such login or password, try again.\"" } };
	assert(answer == res);
	//good login
	memset(msg, 0, len);
	strcpy(msg, "{\"id\":25,\"command\":\"login\",\"login\":\"bob\",\"password\":\"123\"}");
	len = strlen(msg);
	answer = make_answer(84800, msg, socket);
	res = { {"\"id\"", "25"}, { "\"command\"", "\"login\"" },
					  { "\"status\"", "\"ok\"" }, { "\"session\"", "\""+to_string(port) + to_string(socket) + "\"" } };
	assert(answer == res);
	
	//message when auth = 1
	flag_auth = 1;
	client_id = 2;
	memset(msg, 0, MSG);
	char tmp[30];
	sprintf(tmp, "%d%d\"}\0", port, socket);
	strcpy(msg, "{\"id\":25,\"command\":\"message\",\"body\":\"test\",\"sender_login\":bob,\"session\":\"");
	strcat(msg, tmp);
	answer = make_answer(84800, msg, socket);
	res = { {"\"id\"", "25"}, { "\"command\"", "\"message_reply\"" },
					  { "\"status\"", "\"ok\"" }, { "\"client_id\"", "\"" + to_string(client_id) + "\"" } };
	assert(answer == res);
	//message when auth = 0
	flag_auth = 0;
	memset(msg, 0, MSG);
	strcpy(msg, "{\"id\":25,\"command\":\"message\",\"body\":\"test\",\"sender_login\":bob,\"session\":\"");
	strcat(msg, tmp);
	answer = make_answer(84800, msg, socket);
	res = { {"\"id\"", "25"}, { "\"command\"", "\"message_reply\"" },
					  { "\"status\"", "\"failed\"" }, { "\"message\"", "\"Authentication is not passed yet.\"" } };
	assert(answer == res);

	//and so on...
	cout << "Make_Answer OK" << endl;
}

int main(int argc, char *argv[]) {
	
	TestMakeAnswer();
	//start_work(argc, argv);
	return 0;

}
