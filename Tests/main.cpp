#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>

#include <BaseTCPServer.h>

class EchoServer : public web::BaseTCPServer
{
private:
	void clientConnection(const std::string& ip, SOCKET clientSocket, sockaddr address, std::function<void()>& cleanup) override try
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
		BaseTCPServer("8080")
	{

	}
};

int main(int argc, char** argv) try
{
	EchoServer server;

	server.start(false, []() { std::ofstream("run.txt"); });

	int errorCode = 0;

#ifdef __LINUX__
	errorCode = std::system("python3 -u tests.py");
#else
	errorCode = std::system("python -u tests.py");
#endif

	while (true)
	{
		if (std::filesystem::exists("finish.txt"))
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return errorCode;
}
catch (const web::exceptions::WebServerException& e)
{
	std::cout << e.getErrorCode() << ' ' << e.what() << ' ' << e.getFile() << ' ' << e.getLine() << std::endl;

	return -1;
}
