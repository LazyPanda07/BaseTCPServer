#include "BaseTCPServer.h"

#include <thread>

#pragma comment (lib,"ws2_32.lib")

using namespace std;

namespace web
{
	void BaseTCPServer::createListenSocket()
	{
		addrinfo* info = nullptr;
		addrinfo hints = {};

		hints.ai_family = AF_INET;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_socktype = SOCK_STREAM;

		if (getaddrinfo(ip.data(), port.data(), &hints, &info))
		{
			WSACleanup();

			throw exceptions::WebException();
		}

		if ((listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == INVALID_SOCKET)
		{
			freeaddrinfo(info);

			WSACleanup();

			throw exceptions::WebException();
		}

		ioctlsocket(listenSocket, FIONBIO, &listenSocketBlockingMode);

		if (::bind(listenSocket, info->ai_addr, static_cast<int>(info->ai_addrlen)) == SOCKET_ERROR)
		{
			freeaddrinfo(info);

			WSACleanup();

			throw exceptions::WebException();
		}

		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			freeaddrinfo(info);

			WSACleanup();

			throw exceptions::WebException();
		}

		freeaddrinfo(info);
	}

	void BaseTCPServer::receiveConnections()
	{
		int addrlen = sizeof(sockaddr);

		while (isRunning)
		{
			sockaddr addr;
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

		for (const auto& [ip, _] : data.getClients())
		{
			this->pubDisconnect(ip);
		}
	}

	void BaseTCPServer::disconnect(const string& ip)
	{
		try
		{
			vector<SOCKET> sockets = data[ip];

			for (SOCKET socket : sockets)
			{
				this->onDisconnect(socket, ip);

				closesocket(socket);
			}
		}
		catch (const exception&)
		{

		}
	}

	void BaseTCPServer::onConnectionReceive(SOCKET clientSocket, sockaddr addr)
	{

	}

	void BaseTCPServer::onDisconnect(SOCKET clientSocket, const string& ip)
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

	BaseTCPServer::BaseTCPServer(const string& port, const string& ip, DWORD timeout, bool multiThreading, u_long listenSocketBlockingMode, bool freeDLL) :
		ip(ip),
		port(port),
		listenSocket(INVALID_SOCKET),
		blockingMode(0),
		listenSocketBlockingMode(listenSocketBlockingMode),
		timeout(timeout),
		freeDLL(freeDLL),
		isRunning(false),
		multiThreading(multiThreading)
	{
		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			throw exceptions::WebException();
		}
	}

	void BaseTCPServer::start()
	{
		this->createListenSocket();

		isRunning = true;

		handle = async(&BaseTCPServer::receiveConnections, this);
	}

	void BaseTCPServer::stop(bool wait)
	{
		isRunning = false;

		closesocket(listenSocket);

		if (wait)
		{
			handle.wait();
		}
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
