# TinyChat
This is a small chat using Berkeley sockets created in Microsoft Visual Studio.\r\n
It consists of a server program and a client program. \r\n
Server takes a port number as a command line argument. Client takes a port number and host IP-address as command line arguments. \r\n
One server can process many clients requests. \r\n
Messages are sent in json format. A few message types are used for chatting. \r\n
There is a json parser - parser.h - that is used to parse and unparse messages that are sent and recevied. \r\n
Mixed С/С++ code is used in programs (included string & string.h - must be rewritten). \r\n
