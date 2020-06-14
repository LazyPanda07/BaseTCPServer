#include "BaseTCPServer.h"

#include <thread>

#pragma comment (lib,"ws2_32.lib")

using namespace std;

namespace web
{
	void BaseTCPServer::receiveConnections()
	{
		while (isRunning)
		{
			sockaddr addr;

			SOCKET clientSocket = accept(listenSocket, &addr, nullptr);

			if (isRunning && clientSocket != INVALID_SOCKET)
			{
				thread(&BaseTCPServer::clientConnection, this, clientSocket, addr).detach();
			}
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
	}
}

