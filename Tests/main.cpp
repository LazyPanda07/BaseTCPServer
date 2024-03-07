#include <iostream>

#include "BaseTCPServer.h"

class EchoServer : public web::BaseTCPServer
{
private:
	void clientConnection(SOCKET clientSocket, sockaddr addr) override try
	{
		int length = 0;
		std::string message;

		this->receiveBytes(clientSocket, &length, sizeof(int));

		message.resize(length);
		
		this->receiveBytes(clientSocket, message.data(), length);

		message += " from echo server";

		length = message.size();

		this->sendBytes(clientSocket, &length, sizeof(int));

		this->sendBytes(clientSocket, message.data(), length);
	}
	catch (const std::exception& e)
	{
		printf("Exception: %s\n", e.what());
	}

public:
	EchoServer() :
		BaseTCPServer("80")
	{

	}
};

int main(int argc, char** argv) try
{
	EchoServer server;

	server.start(true);

	return 0;
}
catch (const web::exceptions::WebException& e)
{
	std::cout << e.what() << ' ' << e.getFile() << ' ' << e.getLine() << std::endl;

	return -1;
}