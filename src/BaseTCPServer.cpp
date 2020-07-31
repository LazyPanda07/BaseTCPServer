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

				data.insert(getIpV4(addr), clientSocket);

				thread(&BaseTCPServer::clientConnection, this, clientSocket, addr).detach();
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
		catch (const std::exception&)
		{

		}
	}

	string BaseTCPServer::getIpV4(sockaddr& addr)
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

	BaseTCPServer::~BaseTCPServer()
	{
		isRunning = false;

		if (freeDLL)
		{
			WSACleanup();
		}
	}
}