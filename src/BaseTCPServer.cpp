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
			int addrlen = sizeof(addr);

			SOCKET clientSocket = accept(listenSocket, &addr, &addrlen);

			if (isRunning && clientSocket != INVALID_SOCKET)
			{
				setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

				ioctlsocket(clientSocket, FIONBIO, &blockingMode);

				data.insert(getClientIpV4(addr), clientSocket);

				if (multiThreading)
				{
					thread(&BaseTCPServer::clientConnection, this, clientSocket, addr).detach();
				}
				else
				{
					this->clientConnection(clientSocket, addr);
				}

				this->onConnectionReceive(clientSocket, addr);
			}
		}
	}

	void BaseTCPServer::disconnect(const string& ip)
	{
		try
		{
			auto ipAddresses = data[ip];

			for (const auto& i : ipAddresses)
			{
				closesocket(i);
			}
		}
		catch (const exception&)
		{

		}
	}

	void BaseTCPServer::onConnectionReceive(SOCKET clientSocket, sockaddr addr)
	{

	}

	string BaseTCPServer::getClientIpV4(sockaddr& addr)
	{
		string ip;

		ip.resize(16);

		inet_ntop(AF_INET, reinterpret_cast<const char*>(&reinterpret_cast<sockaddr_in*>(&addr)->sin_addr), ip.data(), ip.size());

		while (ip.back() == '\0')
		{
			ip.pop_back();
		}

		return ip;
	}

	string BaseTCPServer::getServerIpV4() const
	{
		string ip;
		sockaddr_in serverInfo = {};
		int len = sizeof(serverInfo);

		ip.resize(16);

		getsockname(listenSocket, reinterpret_cast<sockaddr*>(&serverInfo), &len);

		inet_ntop(AF_INET, &serverInfo.sin_addr, ip.data(), ip.size());

		while (ip.back() == '\0')
		{
			ip.pop_back();
		}

		return ip;
	}

	uint16_t BaseTCPServer::getClientPortV4(sockaddr& addr)
	{
		return ntohs(reinterpret_cast<sockaddr_in&>(addr).sin_port);
	}

	uint16_t BaseTCPServer::getServerPortV4() const
	{
		sockaddr_in serverInfo = {};
		int len = sizeof(serverInfo);

		getsockname(listenSocket, reinterpret_cast<sockaddr*>(&serverInfo), &len);

		return ntohs(serverInfo.sin_port);
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

	void BaseTCPServer::pubDisconnect(const string& ip)
	{
		this->disconnect(ip);

		data.erase(ip);
	}

	vector<pair<string, SOCKET>> BaseTCPServer::getClients()
	{
		return data.getClients();
	}

	u_long& BaseTCPServer::blockingModeForOtherConnections()
	{
		return blockingMode;
	}

	const u_long& BaseTCPServer::blockingModeForOtherConnections() const
	{
		return blockingMode;
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
