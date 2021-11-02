# TinyChat
This is a small chat using Berkeley sockets created in Microsoft Visual Studio.
It consists of a server program and a client program. 
Server takes a port number as a command line argument. Client takes a port number and host IP-address as command line arguments. 
One server can process many clients requests. 
Messages are sent in json format. A few message types are used for exchange of information.
There is a json parser - parser.h - that is used to parse and unparse messages that are sent and recevied.
Mixed С/С++ code is used in programs.
