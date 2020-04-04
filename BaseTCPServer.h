#pragma once

#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>

namespace web
{
	class BaseTCPServer
	{
	protected:
		WSADATA wsaData;

		SOCKET listenSocket;

		addrinfo* info;
		addrinfo hints;

		const std::string port;
		bool freeDLL;
		bool isRunning;

	protected:
		virtual void receiveConnections();

		virtual void clientConnection(SOCKET clientSocket, sockaddr addr) = 0;

	public:
		BaseTCPServer(const std::string& port, bool freeDLL = true);

		virtual void start();

		virtual void stop();

		virtual bool serverState() const;

		virtual ~BaseTCPServer();
	};
}