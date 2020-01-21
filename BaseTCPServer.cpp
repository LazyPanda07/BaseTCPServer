#include "BaseTCPServer.h"

#include <thread>
#include <stdexcept>

#pragma comment (lib,"ws2_32.lib")

using namespace std;

#define CREATE_EXCEPTION(message) string(message + to_string(WSAGetLastError())).data()
#define WINDOWS_SOCKETS_VERSION MAKEWORD(2,2)

namespace web
{
	namespace ServerExceptions
	{
		static std::string wsaStartup			= "WSAStartup failed ";
		static std::string getAddr				= "getaddrinfo failed ";
		static std::string createSocket			= "create socket failed ";
		static std::string bindSocket			= "bind failed ";
		static std::string createListenSocket	= "listen failed ";
	}

	void BaseTCPServer::receiveConnections()
	{
		while (isRunning)
		{
			SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

			if (isRunning && clientSocket != INVALID_SOCKET)
			{
				thread(&BaseTCPServer::clientConnection, this, clientSocket).detach();
			}
		}
	}

	BaseTCPServer::BaseTCPServer(const string& port, bool freeDLL) : port(port), freeDLL(freeDLL)
	{
		if (WSAStartup(WINDOWS_SOCKETS_VERSION, &wsaData))
		{
			throw runtime_error(CREATE_EXCEPTION(ServerExceptions::wsaStartup));
		}

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_socktype = SOCK_STREAM;

		if (getaddrinfo(nullptr, port.data(), &hints, &info))
		{
			throw runtime_error(CREATE_EXCEPTION(ServerExceptions::getAddr));
		}

		if ((listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == INVALID_SOCKET)
		{
			throw runtime_error(CREATE_EXCEPTION(ServerExceptions::createSocket));
		}

		if (bind(listenSocket, info->ai_addr, info->ai_addrlen) == SOCKET_ERROR)
		{
			throw runtime_error(CREATE_EXCEPTION(ServerExceptions::bindSocket));
		}

		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			throw runtime_error(CREATE_EXCEPTION(ServerExceptions::createListenSocket));
		}
	}

	void BaseTCPServer::start()
	{
		isRunning = true;

		thread(&BaseTCPServer::receiveConnections, this).detach();
	}

	void BaseTCPServer::stop()
	{
		isRunning = false;
	}

	bool BaseTCPServer::serverState() const
	{
		return isRunning;
	}

	BaseTCPServer::~BaseTCPServer()
	{
		isRunning = false;

		if (freeDLL)
		{
			WSACleanup();
		}

		if (info)
		{
			freeaddrinfo(info);
			info = nullptr;
		}
	}
}

